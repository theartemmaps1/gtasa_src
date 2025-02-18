/****************************************************************************
 *                                                                          *
 *  Module  :   ps.c                                                        *
 *                                                                          *
 *  Purpose :   view demo platform specific module (sky)                    *
 *                                                                          *
 ****************************************************************************/

/* Predicated on SCE_11 for compatibility with version 1.1.x of SCE libs */
#if (!defined(SCE_11))
#define T10000
#endif /* (!defined(SCE_11)) */

/****************************************************************************
 Includes
 */

//$DW$#include <stdio.h>
//$DW$#include <stdlib.h>
//$DW$#include <string.h>
//$DW$#include <stdarg.h>
//$DW$#include <float.h>
//$DW$#include <malloc.h>
//$DW$#include <sys/errno.h>

/* renderware */
//$DW$#include <rwcore.h>
#include "skyfs.h"

#include "skeleton.h"
#include "vecfont.h"

//$DW$#include "eeregs.h"
//$DW$#include "sifdev.h"
//$DW$#include "sifrpc.h"
#include "Debug.h"
#include "MemoryMgr.h"
#include "PostEffects.h"

#if (!defined(SCE_11))
#include "libpad.h"
#endif /* (!defined(SCE_11)) */


#if (!defined(SCE_11))
typedef struct
{
        unsigned int        cnt;
        unsigned int        status;
        unsigned int        button; /* 16 buttons 1P */
        unsigned char       r3h;
        unsigned char       r3v;
        unsigned char       l3h;
        unsigned char       l3v;
        unsigned int        button2; /* 16 buttons 2P */
        unsigned char       r3h2;
        unsigned char       r3v2;
        unsigned char       l3h2;
        unsigned char       l3v2;
}
scePadData;
#endif /* (!defined(SCE_11)) */


/****************************************************************************
 Local Types
 */

/****************************************************************************
 Local (Static) Prototypes
 */

/****************************************************************************
 Local Defines
 */

#define JOYRANGE            127
#define JOYSKELRANGE        (RwReal) (1)
#define JOYRANGESCALE       (RwReal) (JOYSKELRANGE/(RwReal) (JOYRANGE))

/****************************************************************************
 Globals (across program)
 */
#if (!defined(SCE_11))
//unsigned int        paddata;
//unsigned char       rdata[32];
//u_long128           pad_dma_buf[scePadDmaBufferMax] __attribute__ ((aligned(64)));
//u_long128           pad2_dma_buf[scePadDmaBufferMax] __attribute__ ((aligned(64)));
#endif /* (!defined(SCE_11)) */


/****************************************************************************
 Local (static) Globals
 */

/*--- Macro Definitions ---*/

#if (!defined(__GNUC__))
#define __FUNCTION__ "Unknown"
#endif /* (!defined(__GNUC__)) */


/*--- Local Variables ---*/
volatile unsigned long sweHighCount;

int          skyTimerHandlerHid = -1;

#if (defined(SCE_11))
/* file handle for the pad device */
static int          padHandle;
#endif /* (defined(SCE_11)) */


/* file handle for the keyboard */
static int          keybHandle;

static RwMemoryFunctions bugWorkAround;


/****************************************************************************
 Functions
*/


void
psMouseSetVisibility(RwBool visible __RWUNUSED__)
{
    RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, "");
    return;
}

void
psMouseSetPos(RwV2d * pos __RWUNUSED__)
{
    RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, "");
    return;
}

void
psWindowSetText(RwChar * text __RWUNUSED__)
{
    RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, "");
    return;
}

void
psErrorMessage(const RwChar * message)
{
    RwDebugSendMessage(rwDEBUGERROR, __FUNCTION__, message);
    return;
}

void
psWarningMessage(RwChar * message)
{
    RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, message);
    return;
}

RwBool psCameraBeginUpdate(RwCamera* pCamera)
{
	if(RwCameraBeginUpdate(pCamera))
		return true;
	
	return false;
}

void psCameraShowRaster(RwCamera * camera)
{
    RwCameraShowRaster(camera, NULL, 0);
    return;
}

//
//
//
//
void psDebugMessageHandler(RwDebugType type, const RwChar * str)
{
#if rwLIBRARYCURRENTVERSION	>= 0x36000
	// Andrzej:
	// temporarily cancels annoying warning message: "rppds.c(310): ASSERT: PDSAtomicRights: rpPDSMAXPIPEID > id"
	// whenever CustomCVBuildingPipeline atomic is being loaded.
	// this should be removed after migrating to RW > 030904 (hopefully):
	if( ::strstr(str, "rppds.c(310)") )
	{
		return;
	}

	// same as above, but for message "rppds.c(271): ASSERT: PDSMaterialRights: rpPDSMAXPIPEID > id"
	if( ::strstr(str, "rppds.c(271)") )
	{
		return;
	}
	
	// fix for complaints about missing default matfx pipes while streaming in models
	//  (they were switched off to save memory - Andrzej):
	//"rppds.c(604): MESSAGE: RpPDSGetPipe: Requested missing pipeline - PipeID(plugin, pipe) = 1100e(1,100e)"
	if( ::strstr(str, "rppds.c(604)") )
	{
		return;
	}

	// "effectPipesSky.c(180): ASSERT: _rpMatFXPipelineAtomicSetup: NULL != pipeline"
	if( ::strstr(str, "effectPipesSky.c(180)") )
	{
		return;
	}
#endif


    switch (type)
    {
#if (defined(COLOR))
        case rwDEBUGASSERT:   /* red */
            printf("\033[31m%s\033[0m\n", str);
            break;
        case rwDEBUGERROR:    /* bold red */
            printf("\033[31;1m%s\033[0m\n", str);
            break;
        case rwDEBUGMESSAGE:  /* blue */
            printf("\033[34m%s\033[0m\n", str);
            break;
#endif /* (defined(COLOR)) */

        default:
            printf("%s\n", str);
    }

    return;
}

int
TimerHandler(int ca)
{
    if ((ca == INTC_TIM0) && (*T0_MODE & 0x800))
    {
        *T0_MODE |= 0x800;
        sweHighCount += 0x10000;
    }
    return (0);
}

/****************************************************************************
 psTimer

 Reads a millisecond timer to speed adjust things.

 On entry   : None
 On exit    : 32 bit millisecond counter
*/

RwUInt32
psTimer(void)
{
    unsigned long       high0, low0, high1, low1;

    high0 = sweHighCount;
    low0 = *T0_COUNT;
    high1 = sweHighCount;
    low1 = *T0_COUNT;

#if (!defined(T10000))
    if (high0 == high1)
    {
        return ((RwUInt32) ((high0 | (low0 & 0xffff)) / 6250));
    }
    else
    {
        return ((RwUInt32) ((high1 | (low1 & 0xffff)) / 6250));
    }
#else /* (!defined(T10000)) */
    if (high0 == high1)
    {
        return ((RwUInt32) ((high0 | (low0 & 0xffff)) / 9216));
    }
    else
    {
        return ((RwUInt32) ((high1 | (low1 & 0xffff)) / 9216));
    }
#endif /* (!defined(T10000)) */

}

RwChar             *
psPathnameCreate(const RwChar * pathname)
{
    if (pathname)
    {
        char               *dstBuffer;

        dstBuffer = (char *)GtaMalloc(sizeof(RwChar) * (rwstrlen(pathname) + 1));

        if (dstBuffer)
        {
            strcpy(dstBuffer, pathname);

            /* Convert a path for use on PSX 2.  Convert all \s and :s into /s */
/*	Take this bit cause it appears to crash
	This means from now on the paths have to be written with /'s rather than \'s
            while ((charToConvert = rwstrchr(dstBuffer, '\\')))
            {
                *charToConvert = '/';
            }
            while ((charToConvert = rwstrchr(dstBuffer, ':')))
            {
                *charToConvert = '/';
            }
*/
        }

        return (dstBuffer);
    }
    return (NULL);
}

void
psPathnameDestroy(RwChar * pathname)
{
    if (pathname)
    {
        GtaFree(pathname);
    }

    return;
}

RwChar
psPathGetSeparator(void)
{
    return '/';
}

#if (defined(RWTERMINAL))

/* opens a pipe, created with mkfifo on the Linux box */
static              RwBool
SkyKeyboardOpen(void)
{
#if (defined(SCE_11))
    static char _rwpipe[] = "sim:rwpipe";
#else /* (defined(SCE_11)) */
    static char _rwpipe[] = "host0:rwpipe";
#endif /* (defined(SCE_11)) */
    RwBool result;
    const int   mode = SCE_RDONLY | SCE_NOWAIT /*|SCE_NOWBDC */ ;

    RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, 
                       "Waiting for process to feed pipe\n");

    keybHandle = sceOpen(_rwpipe, mode);
    result = (keybHandle > 0);
    
    if (result)
    {
        RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, "Pipe opened");
    }
    else
    {
        RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__, 
                           "Continuing with no keyboard");
    }
    
    return result;
}
#endif /* defined(RWTERMINAL) */


static void
_handleKeyboard(void)
{
/*
    if (keybHandle)
    {
        static RwBool       first = TRUE;
        static unsigned char c[10];

        if (!first)
        {
            int                 is_end = 1, k;

            k = sceIoctl(keybHandle, SCE_FS_EXECUTING, &is_end);
            if (is_end == 0)
            {
                RsKeyStatus         keys;

                // we don't have a scan code, just ascii
                keys.keyScanCode = -1;
                keys.keyCharCode = c[0];

                RsKeyboardEventHandler(rsKEYDOWN, &keys);
                RsKeyboardEventHandler(rsKEYUP, &keys);

                // read more for later...
                sceRead(keybHandle, c, sizeof(char));
            }
        }
        else
        {
            sceRead(keybHandle, c, sizeof(char));

            first = FALSE;
        }
    }

    return;
   */
}





/*RwBool SkyPadOpen(void)
{
    // Grab iop modules

#if (defined(SCE_11))
	printf("SCE_11\n");
#endif

#if (!defined(SCE_11))
    {
//        char buf[256];
        char _SIO2MAN[256];
        char _PADMAN[256];
        static char _empty[] =  "";
        
#ifndef IOPPATH
ASSERT(0);
#endif


#define SkyPadOpen_Local_string(arg) #arg
#define SkyPadOpen_Local_eval(arg) SkyPadOpen_Local_string(arg)

        sprintf(_SIO2MAN, "host0:c:/gta3\\iopmodules\\sio2man.irx");
        sprintf(_PADMAN, "host0:c:/gta3\\iopmodules\\padman.irx");
 
//        sprintf(_SIO2MAN, "host0:c:/sce/iop\\modules\\sio2man.irx");
//        sprintf(_PADMAN, "host0:c:/sce/iop\\modules\\padman.irx");
//        sprintf(_SIO2MAN, "host0:%s\\modules\\sio2man.irx",
//                SkyPadOpen_Local_eval(IOPPATH));
//        sprintf(_PADMAN, "host0:%s\\modules\\padman.irx",
//                SkyPadOpen_Local_eval(IOPPATH));


#undef SkyPadOpen_Local_eval
#undef SkyPadOpen_Local_string

       
        sceSifInitRpc(0);
        
        // Try the C drive first
        _SIO2MAN[6] = 'c';
        _PADMAN[6] = 'c';
        DriveLetter = 'c';
        if (sceSifLoadModule(_SIO2MAN, 0, _empty) < 0)
        {
        	// If c doesn't work we'll try d
	        _SIO2MAN[6] = 'd';
 	      	_PADMAN[6] = 'd';
	        DriveLetter = 'd';
       		if (sceSifLoadModule(_SIO2MAN, 0, _empty) < 0)
        	{
        		// If d doesn't work either we're fucked really
        	    printf ("Can't Load Module %s", _SIO2MAN);
        	    ASSERT(0);
        	    return FALSE;
        	}
        }
        if (sceSifLoadModule(_PADMAN, 0, _empty) < 0)
        {
            printf ("Can't Load Module %s", _PADMAN);
            return FALSE;
        }

        scePadInit(0);
        scePadPortOpen(0, 0, pad_dma_buf);    
        scePadPortOpen(1, 0, pad2_dma_buf);	// also open 2nd controller port
    }
#else
    {
        static char _pad[] = "pad:";
        
        padHandle = sceOpen(_pad, SCE_RDONLY | SCE_NOWBDC);
    }
    
    //    padHandle = 0; 
#endif // (!defined(SCE_11)) 


#if (defined(SCE_11))
    if (padHandle > 0)
#endif // (defined(SCE_11)) 

    {
        return TRUE;
    }

    return FALSE;
    return TRUE;
}*/





static              RwUInt32
remapButtons(int sceButtons)
{
/*
    RwUInt32            rsButtons = 0;

    rsButtons |= (sceButtons & SCE_PADLup) ? rsPADDPADUP : 0;
    rsButtons |= (sceButtons & SCE_PADLdown) ? rsPADDPADDOWN : 0;
    rsButtons |= (sceButtons & SCE_PADLleft) ? rsPADDPADLEFT : 0;
    rsButtons |= (sceButtons & SCE_PADLright) ? rsPADDPADRIGHT : 0;
    rsButtons |= (sceButtons & SCE_PADstart) ? rsPADSTART : 0;
    rsButtons |= (sceButtons & SCE_PADselect) ? rsPADSELECT : 0;
    rsButtons |= (sceButtons & SCE_PADRup) ? rsPADBUTTON1 : 0;
    rsButtons |= (sceButtons & SCE_PADRdown) ? rsPADBUTTON2 : 0;
    rsButtons |= (sceButtons & SCE_PADRleft) ? rsPADBUTTON3 : 0;
    rsButtons |= (sceButtons & SCE_PADRright) ? rsPADBUTTON4 : 0;
    rsButtons |= (sceButtons & SCE_PADL1) ? rsPADBUTTON5 : 0;
    rsButtons |= (sceButtons & SCE_PADL2) ? rsPADBUTTON6 : 0;
    rsButtons |= (sceButtons & SCE_PADR1) ? rsPADBUTTON7 : 0;
    rsButtons |= (sceButtons & SCE_PADR2) ? rsPADBUTTON8 : 0;
    rsButtons |= (sceButtons & SCE_PADi) ? rsPADBUTTONA1 : 0;
    rsButtons |= (sceButtons & SCE_PADj) ? rsPADBUTTONA2 : 0;

    return (rsButtons);
*/
return(0);	// temp to stop warnings
}

static void
_handlePad(void)
{
/*
#if (defined(SCE_11))
    if (padHandle > 0)
#endif // (defined(SCE_11))

    {
        RsPadButtonStatus   padButtonsDown;
        RsPadButtonStatus   padButtonsUp;

#if (defined(SCE_11))
        static unsigned int cnt = ~0l;
        static volatile scePadData pad;
#endif // (defined(SCE_11))

        static RwUInt32     oldButtons = 0;
        RwUInt32            buttons;

        // Read controller data
#if (defined(SCE_11))
        sceRead(padHandle, &pad, sizeof(pad));
#else
        if (scePadRead(0, 0, rdata) > 0)
            paddata = 0xffff ^ ((rdata[2] << 8) | rdata[3]);
        else
            paddata = 0;
#endif // (defined(SCE_11))


#if (defined(SCE_11))
        if (cnt == pad.cnt)
        {
            // Pad read failed - cnt not updated since last time
            return;
        }
        else
        {
            cnt = pad.cnt;
        }
#endif // (defined(SCE_11))


        // Only one pad supported
        padButtonsDown.padID = padButtonsUp.padID = 0;

        // Remap the buttons to RenderWare ones
#if (defined(SCE_11))
        buttons = remapButtons(pad.button);
#else
        buttons = remapButtons(paddata);
#endif // (defined(SCE_11))


        // Figure out what has changed
        padButtonsDown.padButtons = ~oldButtons & buttons;
        padButtonsUp.padButtons = oldButtons & ~buttons;

        // manage the analogue sticks
#if (defined(SCE_11))
        if (pad.status & SCE_PAD_ANALOG)
#endif // (defined(SCE_11))

        {
            static unsigned char leftH = 0;
            static unsigned char leftV = 0;
            int                 dLeftx, dLefty;

            static unsigned char rightH = 0;
            static unsigned char rightV = 0;
            int                 dRightx, dRighty;

            // left analogue stick

            if (!leftH)
            {
                // first time through - sample the centre pos
#if (defined(SCE_11))
                leftH = pad.l3h;
                leftV = pad.l3v;
#else
                leftH = rdata[6];
                leftV = rdata[7];
#endif // (defined(SCE_11))

            }

#if (defined(SCE_11))
            dLeftx = leftH - pad.l3h;
            dLefty = leftV - pad.l3v;
#else
            dLeftx = leftH - rdata[6];
            dLefty = leftV - rdata[7];
#endif // (defined(SCE_11))


            if ((abs(dLeftx) > 16) || (abs(dLefty) > 16))
            {
                RwV2d               delta;

                delta.x =
                    (RwReal) (((RwReal) (dLeftx)) * JOYRANGESCALE);
                delta.y =
                    (RwReal) (((RwReal) (dLefty)) * JOYRANGESCALE);

                RsPadEventHandler(rsPADANALOGUELEFT, &delta);
            }

            // now do the right analogue stick
            if (!rightH)
            {
                // first time through - sample the centre pos
#if (defined(SCE_11))
                rightH = pad.r3h;
                rightV = pad.r3v;
#else
                rightH = rdata[4];
                rightV = rdata[5];
#endif // (defined(SCE_11))

            }

#if (defined(SCE_11))
            dRightx = rightH - pad.r3h;
            dRighty = rightV - pad.r3v;
#else
            dRightx = rightH - rdata[4];
            dRighty = rightV - rdata[5];
#endif // (defined(SCE_11))


            if ((abs(dRightx) > 16) || (abs(dRighty) > 16))
            {
                RwV2d               delta;

                delta.x =
                    (RwReal) (((RwReal) (dRightx)) * JOYRANGESCALE);
                delta.y =
                    (RwReal) (((RwReal) (dRighty)) * JOYRANGESCALE);

                RsPadEventHandler(rsPADANALOGUERIGHT, &delta);
            }
        }

        // If any buttons just went down, or came up, message the app 
        if (padButtonsDown.padButtons)
        {
            RsPadEventHandler(rsPADBUTTONDOWN, &padButtonsDown);
        }
        if (padButtonsUp.padButtons)
        {
            RsPadEventHandler(rsPADBUTTONUP, &padButtonsUp);
        }

        // Start and select at same time to quit
        if ((buttons & (rsPADSTART | rsPADSELECT)) ==
            (rsPADSTART | rsPADSELECT))
        {
            // Send a quit message - this allows app to do stuff
            RsEventHandler(rsQUITAPP, NULL);
        }

        // Save buttons so we know what's changed next time
        oldButtons = buttons;
    }
   */
}

static void SkyHandleEvents(void)
{
//    _handlePad();
//    _handleKeyboard();

    return;
}

RwBool psInitialize(void)
{
    return (TRUE);
}

void psTerminate(void)
{
    return;
}

RwBool psAlwaysOnTop(RwBool AlwaysOnTop __RWUNUSED__)
{
    return TRUE;
}

RwMemoryFunctions *psGetMemoryFunctions()
{
	static RwMemoryFunctions memFuncs = {
		&CMemoryMgr::Malloc,
		&CMemoryMgr::Free,
		&CMemoryMgr::Realloc,
		&CMemoryMgr::Calloc
	};	
    return &memFuncs;
}

RwBool psInstallFileSystem(void)
{
    if (SkyInstallFileSystem())
    {
        RwDebugSendMessage(rwDEBUGMESSAGE, __FUNCTION__,
                           "sky file system installed");
        return (TRUE);
    }

    return (FALSE);
}

RwBool psSelectDevice(void)
{
if (FALSE)
//#ifndef DMA_POST_FX
{
	int32 i;
	RwVideoMode	videoMode;
	int32 num = RwEngineGetNumVideoModes();
	
	for(i=0; i<num; i++)
	{
        RwEngineGetVideoModeInfo(&videoMode, i);
        if(videoMode.width == RsGlobal.screenWidth && 
        	videoMode.height == RsGlobal.screenHeight &&
        	videoMode.depth == 32 &&
			(videoMode.flags == (rwVIDEOMODEINTERLACE|rwVIDEOMODEEXCLUSIVE| rwVIDEOMODEFSAA1)))
        {
   	    	RwEngineSetVideoMode(i);
        	break;
        }
	}
	
	ASSERTMSG(i != num, "Can't find requested video mode");
}else
//#else
{
	CPostEffects::SetUpVideoModeForEngine();
}
//#endif
    return (TRUE);
}

RwImage *psGrabScreen(RwCamera * camera)
{
    RwImage            *result = (RwImage *) NULL;
    RwRaster           *camRas;
    RwInt32             width, height, depth;
    RwImage            *image;
    RwUInt8            *oldCamPixels, *pixels;

    camRas = RwCameraGetRaster(camera);

    if (camRas)
    {
        width = RwRasterGetWidth(camRas);
        height = RwRasterGetHeight(camRas);
        depth = RwRasterGetDepth(camRas) >> 3;

        /*
         * On PS2, we must give the Camera a data area.
         * Ordinarily, PS2 camera rasters have no in memory image
         * Under normal conditions it is impossible to lock the camera
         * raster on PS2 as its a real performance problem. However,
         * by setting the cpPixel pointer the driver will assume that
         * you know what you are doing and are willing to take the hit
         */

        /* We know that malloc aligns on qw */
        oldCamPixels = camRas->cpPixels;

        pixels = (RwUInt8 *) GtaMalloc(width * height * depth);

        if ((pixels))
        {
            camRas->cpPixels = pixels;
            /* Lock the camera raster for read. This will do what ever is */
            /* required to get the data into memory */

            /* The PS2 driver doesn't do this for you */
            if (!RwRasterLock(camRas, 0, rwRASTERLOCKREAD))
            {
                camRas->cpPixels = oldCamPixels;
                GtaFree(pixels);
            }

            image = RwImageCreate(width, height, 32);
            if (image)
            {
                RwImageAllocatePixels(image);
                RwImageSetFromRaster(image, camRas);

                RwRasterUnlock(camRas);
                camRas->cpPixels = oldCamPixels;
                GtaFree(pixels);

                result = (image);
            }
            else
            {
                RwRasterUnlock(camRas);
                camRas->cpPixels = oldCamPixels;
                GtaFree(pixels);

                result = (RwImage *)NULL;
            }
        }
        else
        {
            result = (RwImage *)NULL;
        }

    }
    else
    {
        result = (RwImage *)NULL;
    }

    return result;
}

RwBool psNativeTextureSupport(void)
{
    return (TRUE);
}

#if (defined(RWMETRICS))
void psMetricsRender(VecFont * vecFont, RwV2d * pos, RwMetrics * metrics)
{
    RwSkyMetrics       *skyMetrics =

        (RwSkyMetrics *) metrics->devSpecificMetrics;
    if (skyMetrics)
    {
        RwChar              message[200];
        RwInt32             val, integer, fraction;

        val = (10000 * skyMetrics->vu1Running) / skyMetrics->profTotal;
        integer = val / 100;
        fraction = val % 100;
        sprintf(message, "vu1 utilisation = %02d.%02d%%", integer,
                fraction);
        RsVecFontPrint(vecFont, pos, message);
        pos->y += (RwReal) (10.0f);

        val = (10000 * skyMetrics->dma1Running) / skyMetrics->profTotal;
        integer = val / 100;
        fraction = val % 100;
        sprintf(message, "dma1 utilisation = %02d.%02d%%", integer,
                fraction);
        RsVecFontPrint(vecFont, pos, message);
        pos->y += (RwReal) (10.0f);

        val = (10000 * skyMetrics->dma2Running) / skyMetrics->profTotal;
        integer = val / 100;
        fraction = val % 100;
        sprintf(message, "dma2 utilisation = %02d.%02d%%", integer,
                fraction);
        RsVecFontPrint(vecFont, pos, message);
        pos->y += (RwReal) (10.0f);

        sprintf(message, "VSyncs between flips = %d",
                skyMetrics->vSyncsSinceLastFlip);
        RsVecFontPrint(vecFont, pos, message);
        pos->y += (RwReal) (10.0f);
    }
}
#endif /* (defined(RWMETRICS)) */


/****************************************************************************
 main

 Application entry point

 On entry   : none
 On exit    : 0 if OK, -1 if not.
*/

/*
int
main(int argc, char *argv[])
{
    skyTimerHandlerHid = AddIntcHandler(INTC_TIM0, TimerHandler, 0);
    // Set up time0
    sweHighCount = 0;
    *T0_COUNT = 0;
    *T0_COMP = 0;
    *T0_HOLD = 0;
    *T0_MODE = 0x281;
    EnableIntc(INTC_TIM0);

    // Initialize the platform independent data 
    // This will in turn initialise the platform specific data
    //
    if (RsEventHandler(rsINITIALISE, NULL) == rsEVENTERROR)
    {
        return FALSE;
    }

    // Initialize the 3D (RenderWare) components of the app.
    if (RsEventHandler(rsRWINITIALISE, NULL) == rsEVENTERROR)
    {
        RsEventHandler(rsTERMINATE, NULL);
        return FALSE;
    }
    // Define the maximum extents based on the selected video mode

    {
        RwVideoMode         videoMode;

        RwEngineGetVideoModeInfo(&videoMode,
                                 RwEngineGetCurrentVideoMode());

        RsGlobal.maximumWidth = videoMode.width;
        RsGlobal.maximumHeight = videoMode.height;
    }

    // Force a camera resize event
    RsEventHandler(rsCAMERASIZE, NULL);

#if (defined(RWTERMINAL))
    // this will fail if the pipe doesn't exist, but that's okay
    SkyKeyboardOpen();
#endif // defined(RWTERMINAL)


    if (!SkyPadOpen())
    {
        psErrorMessage("Unable to open pad\n");
    }
    else
    {
        RwInt32             i;

        // Parse any command line parameters.
        for (i = 1; i < argc; i++)
        {
            RsEventHandler(rsCOMMANDLINE, argv[i]);
        }

        while (!RsGlobal.quit)
        {
            SkyHandleEvents();
            RsEventHandler(rsIDLE, NULL);
        }
    }

    //
    // Tidy up the 3D (RenderWare) components of the application.
    //
    RsEventHandler(rsRWTERMINATE, NULL);

    // free the platform dependent data
    RsEventHandler(rsTERMINATE, NULL);

    DisableIntc(INTC_TIM0);
    RemoveIntcHandler(INTC_TIM0, skyTimerHandlerHid);

    return (0);
}
*/
