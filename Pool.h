#ifndef PCH_PRECOMPILED

// Title	:	Pool.h
// Author	:	Richard Jobling
// Started	:	27/11/98

#ifndef _POOL_H_
#define _POOL_H_

#include "Debug.h"
#include "Dma.h"

#define POOL_FLAG_ISFREE 0x80
#define POOL_FLAG_REFERENCEMASK 0x7f


template<class T, class S = T> class CPool
{
public:
	typedef T ReturnType;					// This is the interface type. All the functions
											// expect this type
	typedef uint8 StorageType[sizeof(S)];	// Storage type is an array of chars just to stop
											// constructors being calling unnecessarily

protected:
	StorageType* m_aStorage;
	uint8* m_aFlags;

	int32 m_nSize;
	int32 m_nFreeIndex;
	bool m_bOwnsArrays;
	bool m_bDealWithNoMemory;
#ifndef FINALBUILD
	#define TEMPPLATEPOOL_MAXNAMELEN			(32)
	char m_name[TEMPPLATEPOOL_MAXNAMELEN];
#endif	

public:
	
	CPool(const char* pName);
	CPool(int32 nSize, const char* pName);
	CPool(int32 nSize, ReturnType* pPool, uint8* pFlags, const char* pName);
	
	~CPool();
	void SavePool();
	void LoadPool();
	
	static int32 GetStorageSize() {return sizeof(StorageType);}
	int32 GetSize() { return m_nSize; }
	int32 GetFreeIndex() { return m_nFreeIndex; }
	int32 GetNoOfUsedSpaces();
	int32 GetNoOfFreeSpaces();

	void Init(int32 nSize, ReturnType* pPool, uint8* pFlags);
	void Flush();
	void Empty();

	T* New();
	T* New(int32 index);
	void Delete(T* pT);
	
	T* GetAt(int32 index);
	void SetNotFreeAt(int32 index);
	
	int32 GetIndex(T* pT);
	int32 GetJustIndex(T* pT);
	int32 GetJustIndex_NoFreeAssert(T* pT);
	void SetFlagsValue(Int32 Index, UInt8 Flags);
	void StoreInMem(UInt8 **ppMem1, UInt8 **ppMem2);
	void CopyBackFromMem(UInt8 **ppMem1, UInt8 **ppMem2);
	void RemoveTempMem(UInt8 **ppMem1, UInt8 **ppMem2);

	T* GetSlot(int32 index);
	bool GetIsFree(int32 index){ return (m_aFlags[index] & POOL_FLAG_ISFREE); }
	int32 GetReference(int32 index) { return (m_aFlags[index] & POOL_FLAG_REFERENCEMASK); }
	UInt8 GetFlags(int32 index) { return (m_aFlags[index]); }

	bool IsValidPtr(T* ptr);
	void SetCanDealWithNoMemory(bool b) {m_bDealWithNoMemory = b;}

#ifndef FINAL
	void *LowestAddress() { return(&m_aStorage[0]); }
	void *HighestAddress() { return(&m_aStorage[m_nSize]); }
#endif

protected:
	void SetIsFree(int32 index, bool bIsFree) 
	{ 
		bIsFree ? (m_aFlags[index] |= POOL_FLAG_ISFREE) : (m_aFlags[index] &= ~POOL_FLAG_ISFREE); 
	}
	void SetReference(int32 index, int32 nReference) 
	{
	 m_aFlags[index] = (m_aFlags[index] & ~POOL_FLAG_REFERENCEMASK) | (nReference & POOL_FLAG_REFERENCEMASK); 
	 }
	
};

//
// name:		CPool::CPool
// description:	Constructor. Allocates pool array
template<class T, class S> CPool<T,S>::CPool(const char* pName)
{
	m_aStorage = NULL;
	m_aFlags = NULL;

	m_nSize = 0;
	m_nFreeIndex = 0;

#ifndef FINALBUILD	
	m_bDealWithNoMemory = false;
	strncpy(m_name, pName, TEMPPLATEPOOL_MAXNAMELEN-1);
	m_name[TEMPPLATEPOOL_MAXNAMELEN-1] = 0;
#endif	
}


//
// name:		CPool::CPool
// description:	Constructor. Allocates pool array
template<class T, class S> CPool<T,S>::CPool(int32 nSize, const char* pName)
{
	ASSERT(nSize > 0);

	m_aStorage = new StorageType[nSize];
	m_aFlags = new uint8[nSize];

	m_bOwnsArrays = true;
	
	ASSERT(m_aStorage);
	ASSERT(m_aFlags);
	
	m_nSize = nSize;
	m_nFreeIndex = -1;

	for (int32 i = 0; i < nSize; i++)
	{
		SetIsFree(i, true);
		SetReference(i, 0);
	}

	DEBUGLOG2("Pool %s size : %d\n", pName, nSize * sizeof(StorageType));
	
#ifndef FINALBUILD	
	m_bDealWithNoMemory = false;
	strncpy(m_name, pName, TEMPPLATEPOOL_MAXNAMELEN-1);
	m_name[TEMPPLATEPOOL_MAXNAMELEN-1] = 0;
#endif	
}

//
// name:		CPool::CPool
// description:	Constructor. Allocates pool array
template<class T, class S> CPool<T,S>::CPool(int32 nSize, ReturnType* pPool, uint8* pFlags, const char* pName)
{
	ASSERT(nSize > 0);

	m_aStorage = pPool;
	m_aFlags = pFlags;

	m_bOwnsArrays = false;
	
	ASSERT(m_aStorage);
	ASSERT(m_aFlags);
	
	m_nSize = nSize;
	m_nFreeIndex = -1;

	for (int32 i = 0; i < nSize; i++)
	{
		SetIsFree(i, true);
		SetReference(i, 0);
	}
#ifndef FINALBUILD	
	m_bDealWithNoMemory = false;
	strncpy(m_name, pName, TEMPPLATEPOOL_MAXNAMELEN-1);
	m_name[TEMPPLATEPOOL_MAXNAMELEN-1] = 0;
#endif	
}

//
// name:		CPool::CPool
// description:	Destructor
template<class T, class S> CPool<T,S>::~CPool()
{
	Flush();
}

//
// name:		GetNoOfUsedSpaces
// description:	Returns number of slots that are used
template<class T, class S> int32 CPool<T,S>::GetNoOfUsedSpaces()
{
	int32 nNoOfUsedSpaces = 0;

	for (int32 i = 0; i < m_nSize; i++)
	{
		if (!GetIsFree(i))
		{
			nNoOfUsedSpaces++;
		}
	}

	return nNoOfUsedSpaces;
}

//
// name:		GetNoOfUsedSpaces
// description:	Returns number of slots that are not used
template<class T, class S> int32 CPool<T,S>::GetNoOfFreeSpaces()
{
	return (m_nSize - GetNoOfUsedSpaces());
}

//
// name:		Init
// description:	Construct pool arrays
template<class T, class S> void CPool<T,S>::Init(int32 nSize, ReturnType* pPool, uint8* pFlags)
{
	ASSERT(m_aStorage == NULL);
	ASSERT(nSize > 0);

	m_aStorage = (StorageType*)pPool;
	m_aFlags = pFlags;

	m_bOwnsArrays = false;
	
	ASSERT(m_aStorage);
	ASSERT(m_aFlags);
	
	m_nSize = nSize;
	m_nFreeIndex = -1;

	for (int32 i = 0; i < nSize; i++)
	{
		SetIsFree(i, true);
		SetReference(i, 0);
	}
}

//
// name:		Flush
// description:	Delete pool arrays
template<class T, class S> void CPool<T,S>::Flush()
{
	if (m_nSize > 0)
	{
		if(m_bOwnsArrays)
		{
			delete[] m_aStorage;
			delete[] m_aFlags;
		}
		m_aStorage = NULL;
		m_aFlags = NULL;

		m_nSize = 0;
		m_nFreeIndex = 0;
	}
}

//////////////////////////////////////////////////////////////
// Marks the elements as free.
//////////////////////////////////////////////////////////////

template<class T, class S> void CPool<T,S>::Empty()
{
	Int32	C;
	
	for (C = 0; C < m_nSize; C++)
	{
		SetIsFree(C, true);
		SetReference(C, 0);
	}
}


template<class T, class S> T* CPool<T,S>::New()
{
	bool wrap = false;

	do{
		m_nFreeIndex++;
		if(m_nFreeIndex == m_nSize)
		{
			m_nFreeIndex = 0;
			if(wrap)
			{
#ifndef FINAL
				if(!m_bDealWithNoMemory)
				{
					sprintf(gString, "%s Pool Full, Size == %d (See a programmer)", m_name, m_nSize);
					CDebug::DebugMessage(gString);
				}	
#endif			
				return NULL;
			}
			wrap = true;
		}
	}while (!GetIsFree(m_nFreeIndex));

	SetIsFree(m_nFreeIndex, false);
	SetReference(m_nFreeIndex, GetReference(m_nFreeIndex) + 1);
	
	return (ReturnType*)&m_aStorage[m_nFreeIndex];
}

///////////////////////////////////////////////////////
////////////////ONLY USE FOR LOADING GAME
/////////////////////////////////////////////////////

template<class T, class S> T* CPool<T,S>::New(int32 index)
{
	ASSERTMSG((index>>8) >=0 && (index>>8) < m_nSize, "Trying to allocate out of range slot")
	ASSERTMSG(GetIsFree(index>>8), "Trying to allocate already allocated slot in pool")
	StorageType* pT = &m_aStorage[index>>8];
	SetNotFreeAt(index);
	
	return (ReturnType*)pT;
}




template<class T, class S> void CPool<T,S>::Delete(T* pT)
{
	ASSERT(pT != NULL);

	int32 index = ((StorageType*)pT) - &m_aStorage[0];

	ASSERT(index >= 0);
	ASSERT(index < m_nSize);

	SetIsFree(index, true);

	// The following bit is asking for trouble since it immediately replaces things
	// that have been deleted. Error prone since things couls still have pointers to it.
	// If this bit isn't here, we get "Pool Full, Size == %d" messages
	if(index < m_nFreeIndex)
		m_nFreeIndex = index;
}

template<class T, class S> inline T* CPool<T,S>::GetSlot(int32 index) 
{
	if(m_aFlags[index] & POOL_FLAG_ISFREE) 
		return NULL;
	else 
		return (ReturnType*)&m_aStorage[index];
}

template<class T, class S> T* CPool<T,S>::GetAt(int32 index)
{
	if (m_aFlags[index >> 8] == (index & 0xff))
	{
		return (ReturnType*)&m_aStorage[index >> 8];
	}
	else
	{
		return NULL;
	}
}

template<class T, class S> void CPool<T,S>::SetNotFreeAt(int32 index)
{
	SetIsFree(index>>8, false);
	SetReference(index>>8, index&255);

	//Loop through the pool and set the free index at first available one
	m_nFreeIndex=0;
	bool FreeIndexSet=false;
	while (!GetIsFree(m_nFreeIndex))
	{
		m_nFreeIndex++;
	}
}




template<class T, class S> int32 CPool<T,S>::GetIndex(T* pT)
{
	int32 index = ((StorageType*)pT) - &m_aStorage[0];

	return ((index << 8) + m_aFlags[index]);
}

// Get index into array
template<class T, class S> int32 CPool<T,S>::GetJustIndex(T* pT)
{
	int32 index = ((StorageType*)pT) - &m_aStorage[0];

	ASSERT(((StorageType*)pT) == ((StorageType*)&m_aStorage[index]));	// Make sure it's a proper address
	ASSERT(!GetIsFree(index));									// Make sure the element isn't free
	ASSERT(index >= 0 && index < m_nSize);
	
	return (index);
}

// Get index into array
template<class T, class S> int32 CPool<T,S>::GetJustIndex_NoFreeAssert(T* pT)
{
	int32 index = ((StorageType*)pT) - &m_aStorage[0];

	ASSERT(((StorageType*)pT) == ((StorageType*)&m_aStorage[index]));	// Make sure it's a proper address
	ASSERT(index >= 0 && index < m_nSize);
		
	return (index);
}

// Set the flags value
template<class T, class S> void CPool<T,S>::SetFlagsValue(Int32 Index, UInt8 Flags)
{
	m_aFlags[Index] = Flags;
}

//
// name:		IsValidPtr
// description:	Returns if the pointer is pointing to something in this pool that is
//				currently allocated
template<class T, class S> bool CPool<T,S>::IsValidPtr(T* pPtr)
{
	int32 index = ((StorageType*)pPtr) - &m_aStorage[0];
	
	if(index < 0 || index >= m_nSize)
		return false;
	if(GetIsFree(index))
		return false;
	return true;	
}

//////////////////////////////////////////////////////////////
// Allocates memory for this pool to be stored and copies it into it
//////////////////////////////////////////////////////////////

template<class T, class S> void CPool<T,S>::StoreInMem(UInt8 **ppMem1, UInt8 **ppMem2)
{
	*ppMem1 = new UInt8[m_nSize];						// Mem for the flags
	*ppMem2 = new UInt8[m_nSize * GetStorageSize()];	// Mem for the elements

	ASSERT(*ppMem1);
	ASSERT(*ppMem2);

	memcpy(*ppMem1, m_aFlags, m_nSize);
	memcpy(*ppMem2, m_aStorage, m_nSize * GetStorageSize());

	Int32	Number = 0;
	for (Int32 C = 0; C < m_nSize; C++)
	{
		if (!(m_aFlags[C] & POOL_FLAG_ISFREE))
		{
			Number++;	
		}
	}
	DEBUGLOG2("Stored:%d (/%d)\n", Number, m_nSize);

}

//////////////////////////////////////////////////////////////
// Copies stuffy
//////////////////////////////////////////////////////////////

template<class T, class S> void CPool<T,S>::CopyBackFromMem(UInt8 **ppMem1, UInt8 **ppMem2)
{
	ASSERT(*ppMem1);
	ASSERT(*ppMem2);

	memcpy(m_aFlags, *ppMem1, m_nSize);
	memcpy(m_aStorage, *ppMem2, m_nSize * GetStorageSize());

	DEBUGLOG2("Size copied:%d (%d)\n",m_nSize * GetStorageSize(), GetStorageSize() );

	m_nFreeIndex = 0;

	RemoveTempMem(ppMem1, ppMem2);	

	Int32	Number = 0;
	for (Int32 C = 0; C < m_nSize; C++)
	{
		if (!(m_aFlags[C] & POOL_FLAG_ISFREE))
		{
			Number++;	
		}
	}
	DEBUGLOG2("CopyBack:%d (/%d)\n", Number, m_nSize);

}

//////////////////////////////////////////////////////////////
// Remove
//////////////////////////////////////////////////////////////

template<class T, class S> void CPool<T,S>::RemoveTempMem(UInt8 **ppMem1, UInt8 **ppMem2)
{
	ASSERT(*ppMem1);
	ASSERT(*ppMem2);

	delete [] *ppMem1;
	delete [] *ppMem2;

	*ppMem1 = NULL;
	*ppMem2 = NULL;
}


#endif

#endif