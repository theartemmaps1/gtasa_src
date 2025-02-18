//
// file			: TxdStore.cpp
// description	: Pool storing all the texture dictionaries in the game
// written by	: Adam Fowler
//
#include <string.h>
// Game headers
#include "TxdStore.h"
#include "TxdLoad.h"
#include "Streaming.h"
#include "General.h"
#include "Debug.h"
#include "Keygen.h"
#include "rwplugin.h"

//!PC - required for PC build
#include "varConsole.h"

#define NUM_SPECIALTXD_SLOTS	4
#define TXDPARENTFROMTXD(txd)	(*(RwTexDictionary **)(((RwUInt8*)txd) + ms_txdPluginOffset))

CPool<TxdDef>* CTxdStore::ms_pTxdPool = NULL;
RwTexDictionary* CTxdStore::ms_pStoredTxd = NULL;
int32 CTxdStore::ms_lastSlotFound = 0;
static int16 gSpecialSlots[NUM_SPECIALTXD_SLOTS];
static int32 ms_txdPluginOffset = 0;

void CalculateTextureRepetitionSize();
RwTexture* TxdStoreLoadCB(const char* pTexname, const char* pMaskName);

//
// --- Txd Parent Plugin --------------------------------------------------------------
//
void* TxdParentConstructor(void* pTxd, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	TXDPARENTFROMTXD(pTxd) = NULL;
	return pTxd;
}
void* TxdParentDestructor(void* pTxd, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	return pTxd;
}
void* TxdParentCopyConstructor(void* pDestTxd, const void* pSrcTxd, RwInt32 offset __RWUNUSED__, RwInt32 size __RWUNUSED__)
{
	TXDPARENTFROMTXD(pDestTxd) = TXDPARENTFROMTXD(pSrcTxd);
	return pDestTxd;
}

//
// --- TxdStore ----------------------------------------------------------------------
//

//
// TxdStore::Init(): Initialise the texture dictionary pool
//
void CTxdStore::Initialise()
{
	if(ms_pTxdPool == NULL)
	{
		ms_pTxdPool = new CPool<TxdDef>(TXD_STORE_POOL_SIZE, "TexDictionary");
	}

	// Add in dummy txd slots	
	for(int32 i=0; i<NUM_SPECIALTXD_SLOTS; i++)
		gSpecialSlots[i] = AddTxdSlot("*");

	RwTextureSetFindCallBack(&TxdStoreFindCB);
	RwTextureSetReadCallBack(&TxdStoreLoadCB);

#ifndef FINAL		
	VarConsole.Add("Calculate texture wastage", &CalculateTextureRepetitionSize, VC_MEMORY);
#endif	
}

//
// name:		PluginAttach
// description:	TexDictionary parent plugin
bool CTxdStore::PluginAttach()
{
	ms_txdPluginOffset = RwTexDictionaryRegisterPlugin(4, MAKECHUNKID(rwVENDORID_ROCKSTAR, rwTXDPARENT_ID),
													&TxdParentConstructor,
													&TxdParentDestructor,
													&TxdParentCopyConstructor);
	return (ms_txdPluginOffset != -1);
}

//
// Delete everything
//
void CTxdStore::Shutdown()
{
	delete ms_pTxdPool;
}

void CTxdStore::GameShutdown()
{
	// Remove all the tex dictionary slots which don't have a tex dictionary currently loaded
	for(int32 i=0; i<CTxdStore::GetSize(); i++)
	{
		if(CTxdStore::IsValidSlot(i) && CTxdStore::GetNumRefs(i) == 0)
		{
			CTxdStore::RemoveTxdSlot(i);
		}
	}
}

//
// AddTxdSlot: Add a slot for a texture dictionary
//
int32 CTxdStore::AddTxdSlot(const char* pName, bool keepCPU)
{
	TxdDef* pTxdDef = ms_pTxdPool->New();
	ASSERTMSG(pTxdDef, "No more tex dictionary slots available in store");
	ASSERTOBJ(strlen(pName) < TXD_NAME_LENGTH, pName, "Texture dictionary name is too long");
	pTxdDef->pTxd = NULL;
//	pTxdDef->cdInfo = 0;
	pTxdDef->refCount = 0;
	pTxdDef->parentId = -1;
	pTxdDef->keepCPU = keepCPU;
	
#ifdef USE_TXDSTORE_HASHKEY
	pTxdDef->m_hashKey = CKeyGen::GetUppercaseKey(pName);
#endif
#ifndef FINAL
	strcpy(&pTxdDef->name[0], pName);
#endif	
	
	return ms_pTxdPool->GetJustIndex(pTxdDef);
}

//
// RemoveTxdSlot: Remove a texture dictionary slot
//
void CTxdStore::RemoveTxdSlot(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	// delete texture dictionary
	ASSERTMSG(pTxdDef, "No texture dictionary slot");
	if(pTxdDef->pTxd)
		RwTexDictionaryDestroy(pTxdDef->pTxd);
		
	ms_pTxdPool->Delete(ms_pTxdPool->GetSlot(index));
}

//
// GetTxdSlot: return contents of txd slot
//
#ifndef FINAL
const char* CTxdStore::GetTxdName(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary slot");
	return pTxdDef->name;
}
#endif
//
// FindTxdSlot: find a texture dictionary given a name
//

int32 CTxdStore::FindTxdSlot(const char* pName)
{
	TxdDef* pTxdDef;
#ifdef USE_TXDSTORE_HASHKEY
	uint32 key = CKeyGen::GetUppercaseKey(pName);
#endif

#define DW_CACHE_LAST_SLOT_SEARCHED		 
#ifdef DW_CACHE_LAST_SLOT_SEARCHED		
	for (int32 i=ms_lastSlotFound;i>=0;i--)
	{
		pTxdDef = ms_pTxdPool->GetSlot(i);
		#ifdef USE_TXDSTORE_HASHKEY
		if(pTxdDef && pTxdDef->m_hashKey == key)
		#else
		if(pTxdDef && !CGeneral::faststricmp(pName, pTxdDef->name))
		#endif
		{
			ms_lastSlotFound = i;
			return i;
		}
	}

	for (int32 i=ms_lastSlotFound+1;i < ms_pTxdPool->GetSize(); i++)
	{
		pTxdDef = ms_pTxdPool->GetSlot(i);
		#ifdef USE_TXDSTORE_HASHKEY
		if(pTxdDef && pTxdDef->m_hashKey == key)
		#else
		if(pTxdDef && !CGeneral::faststricmp(pName, pTxdDef->name))
		#endif
		{
			ms_lastSlotFound = i;
			return i;
		}			
	}
		
#else			
	for(int32 i=0; i < ms_pTxdPool->GetSize(); i++)
	{	
		pTxdDef = ms_pTxdPool->GetSlot(i);
		#ifdef USE_TXDSTORE_HASHKEY
		if(pTxdDef && pTxdDef->m_hashKey == key)
		#else
		if(pTxdDef && !CGeneral::faststricmp(pName, pTxdDef->name))
		#endif
		{
			return i;
		}
	}			
#endif
	
	return -1;
}


//
// FindTxdSlotFromHashKey: find a texture dictionary given a hash key
//
int32 CTxdStore::FindTxdSlotFromHashKey(const int32 hashKey)
{
	TxdDef* pTxdDef;

	for(int32 i=0; i < ms_pTxdPool->GetSize(); i++)
	{	
		pTxdDef = ms_pTxdPool->GetSlot(i);
#ifdef USE_TXDSTORE_HASHKEY
		if(pTxdDef && pTxdDef->m_hashKey == hashKey)
#else
		if(pTxdDef && CKeyGen::GetUppercaseKey(pTxdDef->name) == hashKey)
#endif
			return i;
	}
	return -1;
}



//
// Load a texture dictionary from a stream
//
bool CTxdStore::LoadTxd(int32 index, const char* pFilename)
{
	bool rt = false;
	RwStream* pStream;
	char fullName[260];
	
	//xbox doesn't have a current directory - needs full paths always!
#if defined (GTA_XBOX)
	sprintf(fullName,"%s%s",CFileMgr::GetCurrentDirectory(), pFilename);
#else //GTA_XBOX
	sprintf(fullName,"%s",pFilename);
#endif //GTA_XBOX

	do { 
	 	pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, fullName);
	} while(pStream == NULL);
	
	if(pStream)
	{
		rt = LoadTxd(index, pStream);
		RwStreamClose(pStream, NULL);
	}
	return rt;
}

//
// name:		SetupTxdParent
// description:	Setup the parent Txd pointer
void CTxdStore::SetupTxdParent(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	// Set TXD parent
	if(pTxdDef->parentId != -1)
	{
		TxdDef* pParentTxdDef = ms_pTxdPool->GetSlot(pTxdDef->parentId);
		ASSERTMSG(pParentTxdDef, "No parent texture dictionary at this slot");
		ASSERTMSG(pParentTxdDef->pTxd!=NULL, "Parent Texture dictionary is not in memory");
		
		SetTxdParent(pTxdDef->pTxd, pParentTxdDef->pTxd);
		AddRef(pTxdDef->parentId);
	}
}

//
// Load a texture dictionary from a stream
//
bool CTxdStore::LoadTxd(int32 index, RwStream* pStream)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTMSG(pTxdDef->pTxd==NULL, "Texture is already in memory");

#ifdef DEBUG
#ifndef FINAL
	DEBUGLOG1("Loading TXD %s\n", pTxdDef->name);
#endif	
#endif	

	if (RwStreamFindChunk(pStream, rwID_TEXDICTIONARY, NULL, NULL))
	{
		
#ifdef OSW
		if (pTxdDef->keepCPU) {
			RwRasterSetKeepCPU(pTxdDef->keepCPU);
		}
#endif

		pTxdDef->pTxd = RwTexDictionaryGtaStreamRead(pStream);
		if(pTxdDef->pTxd)
			SetupTxdParent(index);
		
#ifdef OSW
		if (pTxdDef->keepCPU) {
			RwRasterSetKeepCPU(false);
		}
#endif

		return (pTxdDef->pTxd != NULL);
	}
	DEBUGLOG("Failed to load TXD\n");
	return false;
}

//#ifdef GTA_PS2

//
// Start to load a texture dictionary from a stream. Load the first half of the textures
//
bool CTxdStore::StartLoadTxd(int32 index, RwStream* pStream)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTMSG(pTxdDef->pTxd==NULL, "Texture is already in memory");

#ifdef DEBUG
#ifndef FINAL
	DEBUGLOG1("Start Loading TXD %s\n", pTxdDef->name);
#endif	
#endif	

	if (RwStreamFindChunk(pStream, rwID_TEXDICTIONARY, NULL, NULL))
	{
		pTxdDef->pTxd = RwTexDictionaryGtaStreamRead1(pStream);
		return (pTxdDef->pTxd != NULL);
	}
	DEBUGLOG("Failed to load TXD\n");
	return false;
}

bool CTxdStore::FinishLoadTxd(int32 index, RwStream* pStream)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTMSG(pTxdDef->pTxd, "Texture is already in memory");

#ifdef DEBUG
#ifndef FINAL
	DEBUGLOG1("Finish Loading TXD %s\n", pTxdDef->name);
#endif	
#endif	
	pTxdDef->pTxd = RwTexDictionaryGtaStreamRead2(pStream, pTxdDef->pTxd);
	if(pTxdDef->pTxd)
		SetupTxdParent(index);
	return (pTxdDef->pTxd != NULL);
}

//#endif // GTA_PS2

//
// CTxdStore::Create():Create an empty texture dictionary
//
void CTxdStore::Create(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTOBJ(pTxdDef->pTxd==NULL, pTxdDef->name, "Texture is already in memory");

	pTxdDef->pTxd = RwTexDictionaryCreate();	
}

RwTexture* RemoveIfRefCountIsGreaterThanOne(RwTexture* pTexture, void* pData)
{
	if(pTexture->refCount > 1)
	{
		RwTextureDestroy(pTexture);
		RwTexDictionaryRemoveTexture(pTexture);
	}	
	
	return pTexture;
}

//
// RemoveTxd: Remove a texture dictionary from its slot
//
void CTxdStore::RemoveTxd(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	// delete texture dictionary
	ASSERTMSG(pTxdDef, "No texture dictionary slot");
	ASSERTOBJ(pTxdDef->pTxd, pTxdDef->name, "Texture dictionary is not in memory");
	if(pTxdDef->pTxd)
	{
		//if(CStreaming::IsObjectInCdImage(RWTXD_INDEX_TO_INDEX(index)))
			RwTexDictionaryForAllTextures(pTxdDef->pTxd, &RemoveIfRefCountIsGreaterThanOne, NULL);
		RwTexDictionaryDestroy(pTxdDef->pTxd);
	}	
	
	if(pTxdDef->parentId != -1)	
		RemoveRef(pTxdDef->parentId);
		
	pTxdDef->pTxd = NULL;
}

//
// SetCurrentTxd: Set the current texture dictionary given an index
//
void CTxdStore::SetCurrentTxd(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTOBJ(pTxdDef->pTxd, pTxdDef->name, "Texture dictionary is not in memory");
	RwTexDictionarySetCurrent(pTxdDef->pTxd);
}

//
// PushCurrentTxd: push current texture dictionary onto stack
//
void CTxdStore::PushCurrentTxd()
{
	ASSERTMSG(ms_pStoredTxd == NULL, "Texture dictionary stack is full");
	ms_pStoredTxd = RwTexDictionaryGetCurrent();
}

//
// PopCurrentTxd: pop texture dictionary off stack
//
void CTxdStore::PopCurrentTxd()
{
	//ASSERTMSG(ms_pStoredTxd, "Texture dictionary stack is empty");
	RwTexDictionarySetCurrent(ms_pStoredTxd);
	ms_pStoredTxd = NULL;
}

//
// AddRef(): Add another object referencing this texture dictionary
//
void CTxdStore::AddRef(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTOBJ(pTxdDef->pTxd, pTxdDef->name, "Texture dictionary is not in memory");

	pTxdDef->refCount++;
}
//
// RemoveRef(): remove object referencing this texture dictionary. If no objects
//			are left referencing this texture dictionary delete it
//
void CTxdStore::RemoveRef(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTOBJ(pTxdDef->pTxd, pTxdDef->name, "Texture dictionary is not in memory");

	pTxdDef->refCount--;
	if(pTxdDef->refCount <= 0)
	{
#ifdef DEBUG
		DEBUGLOG1("Removing TXD %s\n", pTxdDef->name);
#endif		
		CStreaming::RemoveTxd(index);
	}
}

//
// RemoveRefWithoutDelete(): remove object referencing this texture dictionary. 
//
void CTxdStore::RemoveRefWithoutDelete(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");
	ASSERTOBJ(pTxdDef->refCount>0, pTxdDef->name, "Texture dictionary reference count is zero");

	pTxdDef->refCount--;
}

//
// name:		GetNumRefs
// description:	Return the number of objects referencing this texture dictionary
//
int32 CTxdStore::GetNumRefs(int32 index)
{
	TxdDef* pTxdDef = ms_pTxdPool->GetSlot(index);
	
	ASSERTMSG(pTxdDef, "No texture dictionary at this slot");

	return pTxdDef->refCount;
}

//
// name:		GetSpecialTxdSlot
// description:	Get one of the special txd slots
int32 CTxdStore::GetSpecialTxdSlot(int32 index)
{
	ASSERTMSG(index < NUM_SPECIALTXD_SLOTS, "Special slot index is out of range");
	return gSpecialSlots[index];
}

//
// name:		SetTxdParent/GetTxdParent
// description:	Access functions for Txd parent 
void CTxdStore::SetTxdParent(RwTexDictionary* pTxd, RwTexDictionary* pParent)
{
	TXDPARENTFROMTXD(pTxd) = pParent;
}

RwTexDictionary* CTxdStore::GetTxdParent(RwTexDictionary* pTxd)
{
	return TXDPARENTFROMTXD(pTxd);
}

//
// name:		TxdStoreLoadCB
// description:	Load texture stub
RwTexture* TxdStoreLoadCB(const char* pTexname, const char* pMaskName)
{
	return NULL;
}


//
// name:		TxdStoreFindCB
// description:	Find in TXD Texture  function
RwTexture* CTxdStore::TxdStoreFindCB(const char* pTexname)
{
	RwTexDictionary* pCurrent = RwTexDictionaryGetCurrent();
	
	while(pCurrent)
	{
		// If found texture then return
		RwTexture* pTexture = RwTexDictionaryFindNamedTexture(pCurrent, pTexname);
		if(pTexture)
			return pTexture;
		// see if we can find the texture in the parent of this TXD	
		pCurrent = GetTxdParent(pCurrent);
	}	
	return NULL;
}


#ifndef FINAL

struct TextureCount 
{
	RwTexture* pTexture;
	uint32 count;
};

static uint32 textureSize;
static uint32 textureNumber;
static uint32 texturesWasted;
RwTexture* AddTexturesToList(RwTexture* pTexture, void* data)
{
	TextureCount* ppTexture = (TextureCount*)data;
	
	for(int32 i=0; i<textureNumber; i++)
	{
		if(!stricmp(pTexture->name, ppTexture[i].pTexture->name))
		{
			RwRaster* pRaster = RwTextureGetRaster(pTexture);
			int32 size = pRaster->width * pRaster->height * pRaster->depth / 8;
			
			ppTexture[i].count++;
			if(RwRasterGetFormat(pRaster) & rwRASTERFORMATMIPMAP)
				size += size / 3;
			textureSize += size + 500;	// 500 accounts for raster and texture structures
			texturesWasted++;
			return pTexture;
		}
	}
	ppTexture[textureNumber].pTexture = pTexture;
	ppTexture[textureNumber].count = 1;
	textureNumber++;
	
	return pTexture;
}

void CalculateTextureRepetitionSize()
{
	static TextureCount textureArray[2000];
	textureSize = 0;
	textureNumber = 0;
	texturesWasted = 0;
	
	for(int32 i=4; i<TXD_STORE_POOL_SIZE; i++)
	{
		if(CTxdStore::IsValidSlot(i))
		{
			RwTexDictionary* pTxd = CTxdStore::GetTxd(i);
			if(pTxd)
			{
				RwTexDictionaryForAllTextures(pTxd, &AddTexturesToList, &textureArray[0]);
			}
		}
	}
	
	DEBUGLOG2("Repetition account for %d bytes from %d textures\n", textureSize, texturesWasted);
	for(int32 i=0; i<textureNumber; i++)
	{
		if(textureArray[i].count > 1)
		{
			DEBUGLOG2("%s\t%d\n", textureArray[i].pTexture->name, textureArray[i].count);
		}
	}
}
#endif