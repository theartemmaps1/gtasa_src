//
// name:		MemoryMgr.cpp
// description:	Interface to all dyanmic memory management functions
// written by:	Adam Fowler
//
// Game headers
#include "MemoryMgr.h"
#include "MemoryHeap.h"
#include "MemInfo.h"
#include "Streaming.h"
#include "StreamingDebug.h"
#include "Game.h"
#include "CutsceneMgr.h"
#ifdef GTA_PS2
	#include "eemacros.h"
#else
	#include "fakeScratchpad.h"
#endif //GTA_PS2

#ifndef MASTER
#define HEAP_RECORDFREQ
//#define STACKOVERFLOW_CHECK
#endif

#if defined (GTA_PS2)
#define SMALLHEAP_SIZE		(2300*1024)
#else //GTA_PS2
#define MAINHEAP_SIZE		(50000*1024)
#define SMALLHEAP_SIZE		(6000*1024)
#endif //GTA_PS2

#define SMALLHEAP_MAX_ALLOC	(2048)

static CMemoryHeap gMainHeap;
static CMemoryHeap gSmallHeap;
static CMemoryHeap gScratchPadHeap;

int32 CMemoryMgr::m_memUsed;
bool CMemoryMgr::m_bMallocHintDebugging;



// static member variables
uint32 CMemoryMgr::m_minLargeAllocation;
uint32 CMemoryMgr::m_largeAllocation;
CStack<int32, 16> CMemoryMgr::m_idStack;
int32 CMemoryMgr::m_id;
int32* CMemoryMgr::m_pMemUsedArray;
int32 CMemoryMgr::m_blocksUsed;
int32* CMemoryMgr::m_pBlockArray;
int32 CMemoryMgr::m_idCapture;



// size frequency recording
#ifdef HEAP_RECORDFREQ
static int32 gFreq[4096];
#define INITFREQ()			{for(int32 i=0; i<4096; i++) gFreq[i] = 0;}
#define RECORDFREQ(size) 	{if(size < 65536) gFreq[size>>4]++;}
#else
#define INITFREQ()
#define RECORDFREQ(size)
#endif


/*#ifdef CALC_MEMORY_TOP
char* pMemoryTop = 0;
#define STOREMEMORYTOP(p, size) if(((char*)pMem)+size > pMemoryTop) {pMemoryTop = ((char*)pMem)+size;}
#else
#define STOREMEMORYTOP(p, size)
#endif*/

#define DUMMY_PAGE 0x1000  /* 4kb dummy page  */
// DTS libraries allocate 22K with malloc so need to leave space for this
//#define USING_DTS
#define DTS_MEMORY		(22 * 1024)
#ifdef USING_DTS
#define OTHER_MALLOCS (0x800 + 0x5800)
#else
#define OTHER_MALLOCS (0x800/* + 0x300000*/)
#endif

#define HEADER_SIZE 0x10  /* malloc header  */

#if defined (GTA_PS2)
static UInt32 memMgrSema;
static UInt32 scratchPadSema;
#elif !defined(OSW) //GTA_PS2
static HANDLE memMgrSema;
static HANDLE scratchPadSema;
#endif //GTA_PS2

#ifdef STACKOVERFLOW_CHECK
#define STACK_TOP_ID1	0x63617473
#define STACK_TOP_ID2	0x6b6f5f6b

#if defined (GTA_PS2)
#ifndef MASTER
//
// name:		LowestStackAddress
// description:	Resturns the lowest address in the stack (for debug purposes)
void *CMemoryMgr::LowestStackAddress()
{
   __declspec(data) extern void *_stack_size;
    int32 stackSize = (int)&_stack_size + DUMMY_PAGE;
	void* pStackTop = (void*)((char*)EE_MEMORY_SIZE-stackSize);
	
	return (pStackTop);
}
#endif

//
// name:		InitStackOverflowCheck
// description:	Initialises the stack overflow check. It writes 2 integers into the top
//				8 byts of the stack. If these are changed then the stack has overflowed
void InitStackOverflowCheck()
{
    __declspec(data) extern void *_stack_size;
    int32 stackSize = (int)&_stack_size + DUMMY_PAGE;
	uint32* pStackTop = (uint32*)((char*)EE_MEMORY_SIZE-stackSize);

	*pStackTop = STACK_TOP_ID1;	//"stac"
	*(pStackTop+1) = STACK_TOP_ID2;//"k_ok";
}

//
// name:		CheckStackOverflow
// description:	Check for stack overflows. Check to see if two integers at top of
//				stack have changed
void CheckStackOverflow()
{
    __declspec(data) extern void *_stack_size;
    int32 stackSize = (int)&_stack_size + DUMMY_PAGE;
	uint32* pStackTop = (uint32*)((char*)EE_MEMORY_SIZE-stackSize);

	ASSERTMSG(*pStackTop == STACK_TOP_ID1 && *(pStackTop+1) == STACK_TOP_ID2,
				"The stack has overflowed");
}
#endif
#endif //GTA_PS2

//semaphore stuff - needs rewritten for PC
#if defined (GTA_PS2)
//
// name:		CreateCriticalCodeLock
// description:	
uint32 CreateCriticalCodeLock()
{
	uint32 id;
	struct SemaParam sp;
	sp.maxCount = 1;
	sp.initCount = 1;
	
	id = CreateSema(&sp);
	return id;
}
#pragma dont_inline on
//
// name:		LockCriticalCode
// description: Only allow one thread to use this code
void LockCriticalCode(uint32 sema)
{
	if(PollSema(sema) == -1)
		printf("Locked critical code %d\n", sema);
	WaitSema(sema);
}

//
// name:		ReleaseCriticalCode
// description: Allow other threads to use code
void ReleaseCriticalCode(uint32 sema)
{
	SignalSema(sema);
}
#pragma dont_inline reset

#elif !defined(OSW) //GTA_PS2 - semaphore stuff
// name:		CreateCriticalCodeLock
// description:	
HANDLE CreateCriticalCodeLock()
{
	HANDLE id;
	id = CreateSemaphore(NULL, 1, 1, NULL);

	ASSERT(id);
	return id;
}

//
// name:		LockCriticalCode
// description: Only allow one thread to use this code
void LockCriticalCode(HANDLE sema)
{
//	UInt32 ret = WaitForSingleObject(sema, INFINITE);
//	ASSERT((ret==WAIT_OBJECT_0));
}

//
// name:		ReleaseCriticalCode
// description: Allow other threads to use code
void ReleaseCriticalCode(HANDLE sema)
{
//	Bool ret = ReleaseSemaphore(sema, 1, NULL);
//	ASSERT(ret);
}
#endif //GTA_PS2 - semaphore stuff

//
// name:		CMemoryMgr::Init
// description:	Calculate size of heap and allocate memory
//
void CMemoryMgr::Init()
{
#ifdef USE_STD_MALLOC
	return;
#else
#ifdef STACKOVERFLOW_CHECK
	InitStackOverflowCheck();
#endif

    int heap_size, end, stack_size;
#if defined (GTA_PS2)
   __declspec(data) extern void *_end, *_stack_size;

    end = (int)&_end;
    stack_size = (int)&_stack_size + DUMMY_PAGE;

    heap_size = EE_MEMORY_SIZE - (end + stack_size + OTHER_MALLOCS);
#else //GTA_PS2
	heap_size = (MAINHEAP_SIZE);		//80Meg heap?!?
#endif //GTA_PS2

	// allocate mainheap using standard malloc
	void* pBuffer = malloc(heap_size);
	gMainHeap.Init(pBuffer, heap_size);//heap_size - HEADER_SIZE);

	// Create critical code semaphores
	memMgrSema = CreateCriticalCodeLock();
    scratchPadSema = CreateCriticalCodeLock();

	// allocate small heap from the main heap
	pBuffer = gMainHeap.Malloc(SMALLHEAP_SIZE);
	gSmallHeap.Init(pBuffer, SMALLHEAP_SIZE, true);

#ifndef MASTER
	m_minLargeAllocation = 0;
	m_idCapture = -1;
	m_bMallocHintDebugging = false;
#endif
	INITFREQ();

	InitMemoryTracking(MAX_MEM_ID);
	
	InitScratchPad();
#endif
}

// name:		CMemoryMgr::Shutdown
// description:	cleanup ready to shutdown - check what is still allocated
//
void CMemoryMgr::Shutdown()
{
#ifdef USE_STD_MALLOC
	return;
#else
//	gSmallHeap.Shutdown();
	gMainHeap.Shutdown();
#endif
}

//
// name:		MakeSpaceForAllocation
// description:	Get game to remove stuff so that it can allocate
static void MakeSpaceForAllocation(int32 size, int32 attempts)
{
	// We have failed to find space in the memory heap. We need
	// to free up some space
	uint32 memUsed = CStreaming::GetMemoryUsed();

	CStreaming::MakeSpaceFor(256000 + STREAMING_MAX_MEMORY - CStreaming::GetMemoryUsed());

//	CMemoryMgr::PrintFrequencies();
//	gMainHeap.ParseHeap();

	if(attempts > 10)
		CGame::TidyUpMemory(true, false);
	else if(attempts > 6)
		CGame::TidyUpMemory(false);

	// If no memory was freed up and textures have been moved about
	if(memUsed == CStreaming::GetMemoryUsed() &&
		attempts > 12 &&
		CMemoryMgr::GetLargestFreeBlock() < size)
	{
		CGame::TidyUpMemory(true, true);

		// If in the process of loading a cutscene
		if(CCutsceneMgr::IsCutsceneProcessing())
		{
			CCutsceneMgr::RemoveEverythingBecauseCutsceneDoesntFitInMemory();
			return;
		}	
#ifndef MASTER
		if(attempts > 13 && CMemoryMgr::GetLargestFreeBlock() < size)
		{
			CStreamingDebug::PrintStreamingBufferState();
//			asm volatile ("breakc 1");
			ASSERT(FALSE);
		}
#endif
	}
}

//
// name:		InHeap
// description:	Return if memory address is inside a heap
inline bool InHeap(void* pAddr, const CMemoryHeap& heap)
{
	return (pAddr >= heap.GetHeapBase() && pAddr < heap.GetHeapEnd());
}
//
// name:		InternalMalloc
// description:	Attempt to malloc memory. This version can fail
void* CMemoryMgr::InternalMalloc(size_t size)
{
	void* pMem;
	if(size <= SMALLHEAP_MAX_ALLOC)
	{
		pMem = gSmallHeap.Malloc(size);
		if(pMem)
			return pMem;
	}
	return gMainHeap.Malloc(size);
}

//
// name:		InternalRealloc
// description:	Attempt to realloc memory. This version can fail
void* CMemoryMgr::InternalRealloc(void* pMem, size_t size)
{
	void* pNewMem;
	// If memory is in scratch pad
	if(InHeap(pMem, gScratchPadHeap))
	{
		pNewMem = gScratchPadHeap.Realloc(pMem, size);
		ASSERTMSG(pNewMem, "Not enough memory available for scratchpad realloc");
	}
	// If memory is in small heap. Have to check small allocation heap first as it is in 
	// the main heap
	else if(InHeap(pMem, gSmallHeap))
	{
		// if size is still small enough to be in small heap
		if(size <= SMALLHEAP_MAX_ALLOC)
			pNewMem = gSmallHeap.Realloc(pMem, size);
		// otherwise allocate space in large heap and copy data across
		else
		{
			pNewMem = InternalMalloc(size);
			if(pNewMem)
			{
				memcpy(pNewMem, pMem, CMaths::Min(size, CMemoryHeap::GetMemorySize(pMem)));
				Free(pMem);
			}	
		}	
	}
	// otherwise allocation is in main heap
	else
	{
		// if size is still large enough for memory block to be in large heap
		if(size > SMALLHEAP_MAX_ALLOC)
			pNewMem = gMainHeap.Realloc(pMem, size);
		// otherwise allocate space in small heap and copy data across
		else
		{
			pNewMem = InternalMalloc(size);
			if(pNewMem)
			{
				memcpy(pNewMem, pMem, CMaths::Min(size, CMemoryHeap::GetMemorySize(pMem)));
				Free(pMem);
			}	
		}		
	}	
	return pNewMem;
}

//
// name:		CMemoryMgr::Malloc
// description:	Allocate memory
#ifdef OSW
void* CMemoryMgr::Malloc(size_t size)
#else
void* CMemoryMgr::Malloc(size_t size, uint32 hint)
#endif
{
#ifdef USE_STD_MALLOC
	return malloc(size);
#else
	LockCriticalCode(memMgrSema);

	ASSERTMSG(size > 0, "Trying to allocate zero or negative amount of bytes");

	RECORDFREQ(size);

#ifndef MASTER
	if(m_bMallocHintDebugging)
		printf("0x%x allocating %d\n", hint&0xffff, ((size+15)/16)*16);

	// Check for large allocations
	if(m_minLargeAllocation == 0)
	{
		if(size > 90*1024)
			printf("Allocating %dK\n", size>>10);
	}
	else if(size > m_minLargeAllocation)
		m_largeAllocation = size;
#endif

	void* pMem = InternalMalloc(size);
	int32 attempts = 0;

	while(pMem == NULL)
	{
		ReleaseCriticalCode(memMgrSema);
		
		MakeSpaceForAllocation(size, ++attempts);

		LockCriticalCode(memMgrSema);
		
		pMem = InternalMalloc(size);
	}

	RegisterMalloc(pMem);

#ifdef STACKOVERFLOW_CHECK
	CheckStackOverflow();
#endif
	
	CMemoryHeap::SetDebugInfo(pMem, hint);
	
	ReleaseCriticalCode(memMgrSema);

// stuff for trapping allocations which are leaking on shutdown...
	static void* Addr1 = (void*)0x32b1970;
	static void* Addr2 = (void*)0x29c7800;
	static void* Addr3 = (void*)0x29c82c0;
	static void* Addr4 = (void*)0x2a3ab00;

	if ( (pMem == Addr1) || (pMem == Addr2) || (pMem == Addr3) || (pMem == Addr4))
	{
		//want to breakpoint here to trap the memory leak...
		DEBUGLOG("Malloc trap hit...\n");
	}

	return pMem;
#endif
}

//
// name:		CMemoryMgr::Free
// description:	free up memory
void CMemoryMgr::Free(void* pMem)
{
#ifdef USE_STD_MALLOC
	free(pMem);
#else
	LockCriticalCode(memMgrSema);

	ASSERT(m_idCapture == -1 || m_idCapture == CMemoryHeap::GetBlockId(pMem));

#ifndef MASTER
	if(m_bMallocHintDebugging)
		printf("0x%x freeing %d\n", 
			CMemoryHeap::GetDebugInfo(pMem) & 0xffff, 
			CMemoryHeap::GetMemorySize(pMem));
#endif
	RegisterFree(pMem);
	
	// If memory is in scratch pad
	if(InHeap(pMem, gScratchPadHeap))
		gScratchPadHeap.Free(pMem);
	// If memory is in small heap. Have to check small allocation heap first as it is in 
	// the main heap
	else if(InHeap(pMem, gSmallHeap))
		gSmallHeap.Free(pMem);
	else
		gMainHeap.Free(pMem);	

#ifdef STACKOVERFLOW_CHECK
	CheckStackOverflow();
#endif

	ReleaseCriticalCode(memMgrSema);
#endif
}

//
// name:		CMemoryMgr::Realloc
// description:	Allocate a new memory block but keep data from the current block
#ifdef OSW
void* CMemoryMgr::Realloc(void* pMem, size_t size)
#else
void* CMemoryMgr::Realloc(void* pMem, size_t size, uint32 hint)
#endif
{
#ifdef USE_STD_MALLOC
	return realloc(pMem, size);
#else
	// If they send NULL to Realloc it should just do a malloc
	if(pMem == NULL)
		return Malloc(size, hint);
		
	LockCriticalCode(memMgrSema);

	RegisterFree(pMem);

	void* pNewMem = InternalRealloc(pMem, size);
	int32 attempts = 0;

	while(pNewMem == NULL)
	{
		MakeSpaceForAllocation(size, ++attempts);

		pNewMem = InternalRealloc(pMem, size);
	}

	RegisterMalloc(pNewMem);

#ifndef MASTER
	if(m_bMallocHintDebugging)
		printf("0x%x reallocating %d\n", hint&0xffff, size);
#endif
		
	CMemoryHeap::SetDebugInfo(pNewMem, hint);

	ReleaseCriticalCode(memMgrSema);

	return pNewMem;
#endif
}

//
// name:		CMemoryMgr::Malloc
// description:	Allocate memory for a number of objects of a defined size
#ifdef OSW
void* CMemoryMgr::Calloc(size_t numObj, size_t sizeObj)
#else
void* CMemoryMgr::Calloc(size_t numObj, size_t sizeObj, uint32 hint)
#endif
{
#ifdef USE_STD_MALLOC
	return calloc(numObj, sizeObj);
#else
	void* pMem = Malloc(numObj * sizeObj, hint);
	return pMem;
#endif
}

//
// name:		InitScratchPad
// description:	Initialise scratch pad memory heap
void CMemoryMgr::InitScratchPad()
{
	gScratchPadHeap.Init(SPR_PTR(void, 0), SPR_SIZE);
}

//
// name:		MallocFromScratchPad
// description:	Allocate memory from the scratch pad
void* CMemoryMgr::MallocFromScratchPad(size_t size)
{
	ASSERTMSG(size > 0, "Trying to allocate zero or negative amount of bytes");
	void* pMem = gScratchPadHeap.Malloc(size);
	RegisterMalloc(pMem);
	
	ASSERTMSG(pMem, "No more memory left in scratch pad");
	return pMem;
}

//
// name:		MallocAlignFromScratchPad
// description:	Allocate memory that is aligned to a certain value from the 
//				scratch pad
void* CMemoryMgr::MallocAlignFromScratchPad(size_t size, size_t align)
{
	ASSERTMSG(size > 0, "Trying to allocate zero or negative amount of bytes");
	void* memAddr = MallocFromScratchPad(size + align);

	void *pAlignedMem = (void*)((((uint32)memAddr) + align) & ~(align-1));
	*(((void**)pAlignedMem)-1) = memAddr;

	return pAlignedMem;
}

//
// name:		LockScratchPad
// description:	Only allow this thread access to the scratch pad
void CMemoryMgr::LockScratchPad()
{
#ifndef OSW
	LockCriticalCode(scratchPadSema);
#endif
}

void CMemoryMgr::ReleaseScratchPad()
{
#ifndef OSW
	ReleaseCriticalCode(scratchPadSema);
#endif
}

//
// name:		CMemoryMgr::MallocAlign
// description:	Allocate memory that is aligned to a certain value
#ifdef OSW
void* CMemoryMgr::MallocAlign(size_t size, size_t align)
#else
void* CMemoryMgr::MallocAlign(size_t size, size_t align, uint32 hint)
#endif
{
#ifdef USE_STD_MALLOC
	void* memAddr = ::malloc(size + align);
#else
	void* memAddr = Malloc(size + align, hint);
#endif

	void *pAlignedMem = (void*)((((uint32)memAddr) + align) & ~(align-1));
	*(((void**)pAlignedMem)-1) = memAddr;

	return pAlignedMem;
}

//
// name:		CMemoryMgr::FreeAlign
// description:	Free memory that was aligned to a certain value
void CMemoryMgr::FreeAlign(void* pMem)
{
	void* pAddr = *(((void**)pMem)-1);
#ifdef USE_STD_MALLOC
	::free(pAddr);
#else
	Free(pAddr);
#endif
}

//
// name:		CMemoryMgr::MoveMemory
// description:	Attempt to move memory to reduce fragmentation
void* CMemoryMgr::MoveMemory(void* pMem)
{
#ifdef USE_STD_MALLOC
	return pMem;
#else
	void* pNewMem;
	LockCriticalCode(memMgrSema);

	if(InHeap(pMem, gSmallHeap))
		pNewMem = gSmallHeap.MoveMemory(pMem);
	else	
		pNewMem = gMainHeap.MoveMemory(pMem);

	ReleaseCriticalCode(memMgrSema);
	
	return pNewMem;
#endif
}

//
// name:		CMemoryMgr::MoveMemory
// description:	Attempt to move aligned memory to reduce fragmentation
void* CMemoryMgr::MoveMemory(void* pMem, void** pMemAlign, int32 align)
{
#ifdef USE_STD_MALLOC
	return pMem;
#else
	void* pNewMem;
	LockCriticalCode(memMgrSema);
	
	if(InHeap(pMem, gSmallHeap))
		pNewMem = gSmallHeap.MoveMemory(pMem, pMemAlign, align);
	else	
		pNewMem = gMainHeap.MoveMemory(pMem, pMemAlign, align);
		
	ReleaseCriticalCode(memMgrSema);
	
	return pNewMem;
#endif
}

//
// name:		CMemoryMgr::SetRestrictMemoryMove
// description:	Restrict memory movement
void CMemoryMgr::SetRestrictMemoryMove(bool bRestrict)
{
	gMainHeap.SetRestrictMemoryMove(bRestrict);
	gSmallHeap.SetRestrictMemoryMove(bRestrict);
}

//
// name:		CMemoryMgr::SetRestrictMemoryMove
// description:	Return the largest free block available
uint32 CMemoryMgr::GetLargestFreeBlock()
{
	return gMainHeap.GetLargestFreeBlock();
}

//
// name:		CMemoryMgr::GetSizeOfHoles
// description:	Returns the size of all the holes in the heap
uint32 CMemoryMgr::GetSizeOfHoles()
{
	return gMainHeap.GetSizeOfHoles() + gSmallHeap.GetSizeOfHoles();
}

//
// name:		PrintFrequencies()
// description:	Print up the frequency that different sizes are malloc'ed
//
void CMemoryMgr::PrintFrequencies()
{
#ifdef HEAP_RECORDFREQ
	int32 i,j;
	for(i=0; i<64; i++)
	{
		DEBUGLOG1("%d: ", i*16);
		for(j=0; j<gFreq[i]/100; j++)
			DEBUGLOG("*");
		DEBUGLOG("\n");
	}
	for(i=0; i<64; i++)
	{
		DEBUGLOG1("%d: ", i*1024);
		for(j=0; j<gFreq[i*64]/100; j++)
			DEBUGLOG("*");
		DEBUGLOG("\n");
	}
#endif
}

//
// name:		InitMemoryTracking
// description:	Initialise the memory tracking. Enables us to indentify which module allocated memory
void CMemoryMgr::InitMemoryTracking(int32 numIds)
{
	m_memUsed = 0;
	
	PushMemId(1);
	PushMemId(-1);
	
#ifdef OSW
	m_pMemUsedArray = (int32*)Malloc(numIds * sizeof(int32));
	m_pBlockArray = (int32*)Malloc(numIds * sizeof(int32));
#else
	m_pMemUsedArray = (int32*)Malloc(numIds * sizeof(int32), 0);
	m_pBlockArray = (int32*)Malloc(numIds * sizeof(int32), 0);
#endif
	
	PopMemId();
	//RegisterMalloc(HEAPBLOCK_FROM_MEMPTR(m_pMemUsedArray));
	//RegisterMalloc(HEAPBLOCK_FROM_MEMPTR(m_pBlockArray));

	for(int32 i=0; i<numIds; i++)
	{
		m_pMemUsedArray[i] = 0;
		m_pBlockArray[i] = 0;
	}
}

//
// name:		RegisterMalloc
// description:	Register malloc with the memory tracking system
void CMemoryMgr::RegisterMalloc(void* pMem)
{
	int32 size = CMemoryHeap::GetBlockSize(pMem);
	int32 id = CMemoryHeap::GetBlockId(pMem);
//	CMemoryHeap::SetBlockId(pMem, m_id);

	m_memUsed += size;
	m_blocksUsed++;

	if(id != -1)
	{
		m_pMemUsedArray[id] += size;
		m_pBlockArray[id]++;
	}
}

//
// name:		RegisterFree
// description:	Register free with the memory tracking system
void CMemoryMgr::RegisterFree(void* pMem)
{
	// some low level DX stuff is trying to free NULL when assembling a shader...
	if (pMem == NULL)
		return;

	int32 size = CMemoryHeap::GetBlockSize(pMem);
	int32 id = CMemoryHeap::GetBlockId(pMem);

	m_memUsed -= size;
	m_blocksUsed--;
	if(id != -1)
	{
		m_pMemUsedArray[id] -= size;
		m_pBlockArray[id]--;
	}
}

//
// name:		GetMemoryUsed
// description:	Get amount of memory used by a certain module
int32 CMemoryMgr::GetMemoryUsed(int32 id)
{
	return m_pMemUsedArray[id];
}
//
// name:		GetBlocksUsed
// description:	Get number of blocks allocated by a certain module
int32 CMemoryMgr::GetBlocksUsed(int32 id)
{
	return m_pBlockArray[id];
}

//
// name:		PushMemId
// description:	Set the current module that is allocating memory
void CMemoryMgr::PushMemId(int32 id)
{
	ASSERT(id != 0);	// can't use zero as an ID
	m_idStack.push(m_id);
	m_id = id;

	gMainHeap.SetMemId(m_id);
	gSmallHeap.SetMemId(m_id);
}
//
// name:		PopMemId
// description:	Go back to previous module that is allocating memory
void CMemoryMgr::PopMemId()
{
	m_id = m_idStack.pop();

	gMainHeap.SetMemId(m_id);
	gSmallHeap.SetMemId(m_id);
}

#ifndef USE_STD_MALLOC
//
// New and delete handlers. Use the RenderWare memory allocation
//
void* operator new(size_t size)
{
	return CMemoryMgr::Malloc(size, 0);
}

void operator delete(void* pMem)
{
	CMemoryMgr::Free(pMem);
}

void* operator new[](size_t size)
{
	return CMemoryMgr::Malloc(size, 0);
}

void operator delete[](void* pMem)
{
	CMemoryMgr::Free(pMem);
}
#endif

//
// name:		FreeAlignPC
// description:	Allocate memory that is aligned to a certain value
void* mallocAlignPC(int32 size, int32 align)
{
	void* memAddr = GtaMalloc(size + align);

	void *pAlignedMem = (void*)((((uint32)memAddr) + align) & ~(align-1));
	*(((void**)pAlignedMem)-1) = memAddr;

	return pAlignedMem;
}

//
// name:		FreeAlignPC
// description:	Free memory that was aligned to a certain value
void freeAlignPC(void* pMem)
{
	void* pAddr = *(((void**)pMem)-1);
	GtaFree(pAddr);
}


