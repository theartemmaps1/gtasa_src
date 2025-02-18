//
// file			: TxdLoad.cpp
// description	: Texture dictionary loading code
// written by	:  Stolen from RenderWare source
//
//!PC - required extra headers to compile
#if defined(GTA_PC) && !defined(OSW)
	#include "windows.h"
	#include "win.h"

	// D3D headers
	#include <d3d9.h>
#endif

#ifndef OSW
#include <rpanisot.h>
#endif

// game headers
#include "TxdLoad.h"
#include "Debug.h"
#include "TxdStore.h"
#include "cdstream.h"
#include "Streaming.h"
#include "FileMgr.h"
#include "FileLoader.h"
#include "loadingscreen.h"
#include "fx.h"
#include "Pad.h"

#ifdef GTA_PC
	#include "win.h"
#endif//GTA_PC

#include "Directory.h"
#include "Sprite2d.h"
#include "font.h"
#include "hud.h"

#include "timer.h"
float texLoadTime = 0.0f;
int32 texNumLoaded = 0;
float rasLoadTime = 0.0f;
int32 rasNumLoaded = 0;
bool AnisoAvailable = FALSE;
extern uint32 gGameState;


extern const RwPluginRegistry *_rwPluginRegistryReadDataChunks(const RwPluginRegistry * reg, RwStream *stream, void *object);


typedef struct _rwStreamTexDictionary rwStreamTexDictionary;
struct _rwStreamTexDictionary
{
    /* This is required as to be backward compatible, we need to convert
       with the 32 bit endian function.... */
    RwUInt16           numTextures;
    RwUInt16           deviceId;
};

//
// name:		destroyTexture
// description:	destroy texture callback
RwTexture* destroyTexture(RwTexture* pTexture, void* data)
{
	RwTextureDestroy(pTexture);
	pTexture = NULL;
	return pTexture;
}

 					 	
//
// name:		RwTextureGtaStreamRead
// description:	Read texture from stream
RwTexture* RwTextureGtaStreamRead(RwStream* stream)
{
    RwUInt32 size, version;
    RwTexture *pTexture;
	
    // Find the native texture 
    if (!RwStreamFindChunk(stream, rwID_TEXTURENATIVE, &size, &version))
    {
        return NULL;
    }

    ASSERT((version >= rwLIBRARYBASEVERSION) && (version <= rwLIBRARYCURRENTVERSION));

	float t = CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
    // Call the driver to read the texture 
    if (!RWSRCGLOBAL(stdFunc[rwSTANDARDNATIVETEXTUREREAD](stream, &pTexture, size)))
    {
        return NULL;
    }

#ifdef GTA_PC
	if(gGameState == 8)
	{
		t = CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond() - t;
		texLoadTime = (texLoadTime*texNumLoaded + t) / (float)(texNumLoaded+1);
		texNumLoaded++;
	}	 
#endif

    if (!pTexture)
    {
        return NULL;
    }

	// catch any textures which are point sampled and fix them
	uint32 texFilterMode = RwTextureGetFilterMode(pTexture);
	if (texFilterMode == rwFILTERNEAREST)
	{
		RwTextureSetFilterMode(pTexture, rwFILTERLINEAR);
	}
	else if (texFilterMode == rwFILTERMIPNEAREST)
	{
		RwTextureSetFilterMode(pTexture, rwFILTERMIPLINEAR);
	}
/*	else if (texFilterMode == rwFILTERLINEAR)
	{
		printf("Found texture with filter mode rwFILTERLINEAR\n");
	}
	else if (texFilterMode == rwFILTERMIPLINEAR)
	{
		printf("Found texture with filter mode rwFILTERMIPLINEAR\n");
	}
	else if (texFilterMode == rwFILTERLINEARMIPNEAREST)
	{
		printf("Found texture with filter mode rwFILTERLINEARMIPNEAREST\n");
	}
	else if (texFilterMode == rwFILTERLINEARMIPLINEAR)
	{
		printf("Found texture with filter mode rwFILTERLINEARMIPLINEAR\n");
	}
*/

#if 0
	RwRaster* pRaster = RwTextureGetRaster(pTexture);
	RwImage* pImage = RwImageCreate(pRaster->width, pRaster->height, 32);
	
	RwImageAllocatePixels(pImage);
	RwImageSetFromRaster(pImage,pRaster);
	
	RwRaster* pNewRaster = RwRasterCreate(pRaster->width, pRaster->height, 0, 
		RwRasterGetType(pRaster) | (RwRasterGetFormat(pRaster) & ~(rwRASTERFORMATPIXELFORMATMASK|rwRASTERFORMATPAL8)) | rwRASTERFORMAT8888);
	RwRasterSetFromImage(pNewRaster, pImage);
	
	RwImageDestroy(pImage);
	RwRasterDestroy(pRaster);
	
	RwTextureSetRaster(pTexture, pNewRaster);
#endif	

#if defined(GTA_PC) && !defined(OSW)
	// If texture has an anisotropy value then set it to the maximum amount
	if(AnisoAvailable && RpAnisotTextureGetMaxAnisotropy(pTexture) >= 1 && g_fx.GetFxQuality() >= FX_QUALITY_HIGH )
	{
		RpAnisotTextureSetMaxAnisotropy(pTexture, RpAnisotGetMaxSupportedMaxAnisotropy());
	}
#endif	
    // And read in the data chunks for the texture
    /*if (!_rwPluginRegistryReadDataChunks(&textureTKList, stream, pTexture))
    {
        // Tidy up and exit 
        RwTextureDestroy(pTexture);
        return NULL;
    }*/
    return pTexture;
}

//
// name:		RwTexDictionaryGtaStreamRead
// description:	Read texture dictionary from stream
RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream *pStream)
{
    RwUInt32 size, version;

    ASSERT(pStream);

    if (!RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version))
    {
        return NULL;
    }


    ASSERT((version >= rwLIBRARYBASEVERSION) && (version <= rwLIBRARYCURRENTVERSION));

    {
        rwStreamTexDictionary    binTexDict;
        RwTexDictionary    *texDict;

        // Read the texture dictionary back 
        ASSERT(size <= sizeof(binTexDict));
        if (RwStreamRead(pStream, &binTexDict, size) != size)
        {
            return NULL;
        }

#ifndef FINALBUILD
        RwDevice *const device = &RWSRCGLOBAL(dOpenDevice);
        RwUInt16 myId=0;
        
        //!PC
        //txds with no textures are OK - even if they are the wrong type!
        if (binTexDict.numTextures != 0)
        {
	        device->fpSystem(rwDEVICESYSTEMGETID, &myId, NULL, 0);
	        myId &= 0xff;
	        /* We allow for the case where no ids are defined. This will break
	           if an old texture dictionary is seen which has more than 65535
	           textures, but we hope this is an unusual case */
	        if (myId != 0 && 
	        	binTexDict.deviceId != 0 &&
	            myId != binTexDict.deviceId)
	        {
	            DEBUGLOG("Loading texture dictionary for wrong platform");
	            return NULL;
	        }
	    }
		
#endif
        texDict = RwTexDictionaryCreate();
        if (!texDict)
        {
            return NULL;
        }

        while (binTexDict.numTextures--)
        {
        	RwTexture* pTexture = RwTextureGtaStreamRead(pStream);
			
		    if (!pTexture)
		    {
		        // Tidy up and exit
		        RwTexDictionaryForAllTextures(texDict, destroyTexture, NULL);
		        RwTexDictionaryDestroy(texDict);

		        return NULL;
		    }

            RwTexDictionaryAddTexture(texDict, pTexture);
        }


        // And read in the data chunks for the texture dictionary
        /*if (!_rwPluginRegistryReadDataChunks(&texDictTKList, stream, texDict))
        {
            // Tidy up and exit 
            RwTexDictionaryForAllTextures(texDict, destroyTexture, NULL);
            RwTexDictionaryDestroy(texDict);

            return NULL;
        }*/

        // And we're all done 
        return (texDict);
    }
}

static int32 numberTextures=-1;
static int32 streamPosition;
RwTexDictionary *RwTexDictionaryGtaStreamRead1(RwStream *pStream)
{
    RwUInt32 size, version;
    ASSERT(pStream);

	numberTextures = 0;
    if (!RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version))
    {
        return NULL;
    }


    ASSERT((version >= rwLIBRARYBASEVERSION) && (version <= rwLIBRARYCURRENTVERSION));

    {
        rwStreamTexDictionary    binTexDict;
        RwTexDictionary    *texDict;

        // Read the texture dictionary back 
        ASSERT(size <= sizeof(binTexDict));
        if (RwStreamRead(pStream, &binTexDict, size) != size)
        {
            return NULL;
        }

#ifndef FINALBUILD
        RwDevice *const device = &RWSRCGLOBAL(dOpenDevice);
        RwUInt16 myId=0;
        
        device->fpSystem(rwDEVICESYSTEMGETID, &myId, NULL, 0);
        myId &= 0xff;
        /* We allow for the case where no ids are defined. This will break
           if an old texture dictionary is seen which has more than 65535
           textures, but we hope this is an unusual case */
        if (myId != 0 && 
        	binTexDict.deviceId != 0 &&
            myId != binTexDict.deviceId)
        {
            DEBUGLOG("Loading texture dictionary for wrong platform");
            return NULL;
        }
		
#endif
        texDict = RwTexDictionaryCreate();
        if (!texDict)
        {
            return NULL;
        }

		numberTextures = binTexDict.numTextures/2;

        while (binTexDict.numTextures > numberTextures)
        {
        	RwTexture* pTexture = RwTextureGtaStreamRead(pStream);
			
		    if (!pTexture)
		    {
		        // Tidy up and exit
		        RwTexDictionaryForAllTextures(texDict, destroyTexture, NULL);
		        RwTexDictionaryDestroy(texDict);

		        return NULL;
		    }

            RwTexDictionaryAddTexture(texDict, pTexture);
            binTexDict.numTextures--;
        }

		numberTextures = binTexDict.numTextures;
		streamPosition = pStream->Type.memory.position;
        // And we're all done 
        return (texDict);
    }
}
RwTexDictionary *RwTexDictionaryGtaStreamRead2(RwStream *pStream, RwTexDictionary* texDict)
{
    ASSERT(pStream);

	// skip to new position
	RwStreamSkip(pStream, streamPosition - pStream->Type.memory.position);
	
    {
        while (numberTextures--)
        {
        	RwTexture* pTexture = RwTextureGtaStreamRead(pStream);
			
		    if (!pTexture)
		    {
		        // Tidy up and exit
		        RwTexDictionaryForAllTextures(texDict, destroyTexture, NULL);
		        RwTexDictionaryDestroy(texDict);

		        return NULL;
		    }

            RwTexDictionaryAddTexture(texDict, pTexture);
        }


        // And read in the data chunks for the texture dictionary
        /*if (!_rwPluginRegistryReadDataChunks(&texDictTKList, stream, texDict))
        {
            // Tidy up and exit 
            RwTexDictionaryForAllTextures(texDict, destroyTexture, NULL);
            RwTexDictionaryDestroy(texDict);

            return NULL;
        }*/

        // And we're all done 
        return (texDict);
    }
}


extern "C" {
	RwInt32 _rwD3D8FindCorrectRasterFormat(RwRasterType type, RwInt32 flags);
	RwInt32 _rwD3D8CheckValidTextureFormat(RwInt32 format);

}

//
// name:		ReadVideoCardCapsFile
// description:	Read the video card capabilities from the file caps.dat
void ReadVideoCardCapsFile(uint32& regRasterType32, uint32& regRasterType24, uint32& regRasterType16, uint32& regRasterType8)
{
	regRasterType32=-1;
	regRasterType24=-1;
	regRasterType16=-1;
	regRasterType8=-1;
	
	int32 fid = CFileMgr::OpenFile("DATA\\CAPS.DAT", "rb");
	if(fid != 0)
	{
		CFileMgr::Read(fid, (char*)&regRasterType32, 4);
		CFileMgr::Read(fid, (char*)&regRasterType24, 4);
		CFileMgr::Read(fid, (char*)&regRasterType16, 4);
		CFileMgr::Read(fid, (char*)&regRasterType8, 4);
		
		CFileMgr::CloseFile(fid);
	}
}

bool CanVideoCardDoDXT()
{
#ifdef GTA_PC
	return (TRUE);
#else
	return(FALSE);
#endif
}

//
// name:		CheckVideoCardCaps
// description:	Check video card capabilities with what is stored in the registry, if this has
//				changed then we have to recreate the txd image. Return video card has changed
bool CheckVideoCardCaps()
{
	return(TRUE);
}

//
// name:		SetVideoCardCapsInRegistry
// description:	Write the video card capabilities into the file caps.dat
void WriteVideoCardCapsFile()
{
}

//
// name:		ConvertingTexturesScreen
// description:	Draw screen while converting textures
void ConvertingTexturesScreen(uint32 num, uint32 total, const char* pMsg)
{
#if defined(GTA_PC) && !defined(OSW)
	ProcessWindowsMessages();
#endif	
	
	float y=80;
	
	if(!DoRWStuffStartOfFrame( 0,0,0, 0,0,0, 255))
		return;
		
	CFont::InitPerFrame();
	DefinedState2d();

	// Draw background
	CLoadingScreen::RenderSplash();
	
	float barSize=num/(float)total;
	
	// progress bar back
	CSprite2d::DrawRect( CRect(200*HUD_MULT_X, 240*HUD_MULT_Y, 440*HUD_MULT_X, 248*HUD_MULT_Y), CRGBA(64, 64, 64, 255));
	// progress bar front
	CSprite2d::DrawRect( CRect(200*HUD_MULT_X, 240*HUD_MULT_Y, (200+240*barSize)*HUD_MULT_X, 248*HUD_MULT_Y), CRGBA(255,217,106,255));
	
	// draw background window:
	CSprite2d::DrawRect( CRect(120*HUD_MULT_X, 150*HUD_MULT_Y, SCREEN_WIDTH - 120*HUD_MULT_X, SCREEN_HEIGHT - 220*HUD_MULT_Y), CRGBA(50, 50, 50, 210));

	CFont::SetBackground(FALSE);
	CFont::SetProportional(TRUE);
	CFont::SetScale(0.45 * HUD_MULT_X, 0.7 * HUD_MULT_Y);
	CFont::SetWrapx(SCREEN_WIDTH - 170*HUD_MULT_X);
	CFont::SetOrientation(FO_LEFT);
	CFont::SetColor(CRGBA(255,217,106,255));
	CFont::SetFontStyle(FO_FONT_STYLE_STANDARD);

	CFont::PrintString(170*HUD_MULT_X, 160*HUD_MULT_Y, TheText.Get(pMsg));

	CFont::DrawFonts();
	DoRWStuffEndOfFrame();

}


#ifdef GTA_PC
//
// name:		DealWithTxdWriteError
// description:	Deal with the fact that you cannot write to the txd image. Print
//				message, wait for space to be pressed and delete txd image
void DealWithTxdWriteError(uint32 num, uint32 total, const char* pError)
{
	while(!RsGlobal.quit)
	{
		ConvertingTexturesScreen(num, total, pError);
		CPad::UpdatePads();
		if(CPad::GetPad(0)->GetEscapeJustDown())
			break;
	}
	RsGlobal.quit = FALSE;
	LoadingScreen(NULL, NULL);
	
	RsGlobal.quit = TRUE;
}


//
// name:		CreateTxdImageForVideoCard
// description:	Create a txd image file with textures in the preferred format for the PC the game is 
//				running on
bool CreateTxdImageForVideoCard()
{
	char filename[64];
	int32 i;
	char* pZero = new char[2048];
	CDirectory* pDirectory = new CDirectory(CTxdStore::GetSize());
	CDirectory::DirectoryInfo dirInfo;
	
	// flush streaming request list
/*	CStreaming::FlushRequestList();
	
	// make temp directory
	RwFileFunctions* pFS = RwOsGetFileInterface();
	RwStream* pStream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, "models\\txd.img");
	
	if(pStream == NULL)
	{
		if(theSystem.os == WinNT || theSystem.os == Win2000 || theSystem.os == WinXP)
		{
	       	DealWithTxdWriteError(0, CTxdStore::GetSize(), "CVT_CRT");
			delete[] pZero;
			delete pDirectory;
		}	
		return false;
	}	
		
	//
	// save each TXD into the temp directory
	// 
	for(i=0; i<CTxdStore::GetSize(); i++)
	{
		ConvertingTexturesScreen(i, CTxdStore::GetSize(), "CVT_MSG");
		
		if(CTxdStore::IsValidSlot(i))
		{
			// Only do texture dictionaries that are in the cd image
			if(CStreaming::IsObjectInCdImage(RWTXD_INDEX_TO_INDEX(i)))
			{
				CStreaming::RequestTxd(i, STR_FORCELOAD_MASK);
				CStreaming::RequestModelStream(0);
				CStreaming::FlushChannels();
								
				sprintf(filename, "%s.txd", CTxdStore::GetTxdName(i));
	
				// If found Txd			
				if(CTxdStore::GetTxd(i))
				{
					int32 posn = pFS->rwftell(pStream->Type.file.fpFile);
					
			        if(RwTexDictionaryStreamWrite(CTxdStore::GetTxd(i), pStream) == NULL)
			        {
			        	DealWithTxdWriteError(i, CTxdStore::GetSize(), "CVT_ERR");

		        		RwStreamClose(pStream, NULL);
						delete[] pZero;
						delete pDirectory;
						CStreaming::RemoveTxd(i);

						// try again
						//CreateTxdImageForVideoCard();
			        	return false;
			        }
			        
			        // ensure we then align the function pointer to a SEGMENT size
					// Get number of bytes still to write
					int32 size = pFS->rwftell(pStream->Type.file.fpFile) - posn;
					int32 num = size % SEGMENT_SIZE;
					
					size /= SEGMENT_SIZE;
					if(num != 0)
					{
						size++;
						num = SEGMENT_SIZE - num;
						// pad to sector size
						RwStreamWrite(pStream, pZero, num);
					}
			        
			        // size and posn can be use in directory structure
			        dirInfo.posn = posn / SEGMENT_SIZE;
			        dirInfo.size = size;
			        strncpy(dirInfo.name, filename, sizeof(dirInfo.name));
			        
			        pDirectory->AddItem(dirInfo);
			        
					CStreaming::RemoveTxd(i);
				}	
					
				CStreaming::FlushRequestList();	

			}
		}
	}
	
	RwStreamClose(pStream, NULL);
	delete[] pZero;
	
	// write directory file
	if(pDirectory->WriteDirFile("models\\txd.dir") == false)
	{
    	DealWithTxdWriteError(i, CTxdStore::GetSize(), "CVT_ERR");

		delete pDirectory;
		// try again
		//CreateTxdImageForVideoCard();
		return false;
	}
	
	delete pDirectory;
	
	WriteVideoCardCapsFile();
*/	
	return true;
}
#endif//GTA_PC...





