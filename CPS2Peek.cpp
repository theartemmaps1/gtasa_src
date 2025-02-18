//
// CPS2Peek - simple wrapper class for PS2Peek profiler by Stormfront Studios;
//
//
//
//	26/08/2003	-	Andrzej:	- initial;
//	12/08/2004	-	Andrzej:	- support for GTA_PS2PEEK_VIFCODESONLY macro added;
//
//
//
//
//****************************************************************************************************
//
// NOTES:
//
// 1. PS2Peek uses 30MB of PS2 Devkit's memory (range: 96MB-126MB) for storing its sampled data;
//	  (any alterations to above ranges can be done in ClTimer1IntHandler.cpp).
// 2. Last 2MB of Devkit's memory is supposed to be used as a stack (range 126MB-128MB).
// 3. CPS2Peek::Profile() uses Scratchpad RAM while saving profile file(s) to disk (see below). 
//
// 4. Profile() should be called OUTSIDE RwCameraBegin-EndUpdate() pair, because it uses Scratchpad RAM
//		(Renderware uses Scratchpad for building DMA packets in current DMA command stream).
// 5. Inserting custom VifMarks makes sence only INSIDE RwCameraBeginUpdate() - RwCameraEndUpdate() pair,
//		because then DMA command stream is built and VifMark codes will be properly inserted into it, then
//		(probably after next frame) will be dispatched by DMA into PS2 hardware.
//
//
//****************************************************************************************************
//
//
//
//
//
//
#include <mwutils.h>
#include <eekernel.h>
#include <eeregs.h>
#include <sifdev.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <stdlib.h>
#include <stdio.h>
//#include <memory.h>
#include <libpkt.h>
#include <libgraph.h>

#include "dma.h"
#include "VarConsole.h"
#include "Debug.h"		//ASSERTMSG()...


#include "ClTimer1IntHandler.h"
#include "CPS2Peek.h"





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GTA_PS2PEEK_PROFILER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define VIFCMD_UNPACK   				(0x6cL << 24)
#define VIFCMD_CYCLE    				(0x01L << 24)
#define VIFCMD_DIRECT	   				(0x50L << 24)
#define VIFCMD_NOP	    			  	(0x00L << 24)
#define VIFCMD_MARK						(0x07L << 24)
#define VIFCMD_FLUSHE					(0x10L << 24)
#define VIFCMD_FLUSH					(0x11L << 24)
#define VIFCMD_ENABLEINT				(1 << 31)




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
//
//
static
struct
{
	int16	vifMark;
	char	*vifMarkDesc;
} vifMarksTab[] = 
	{
		{CPS2PEEK_VIFMARK_GRASS_START,				"GRASS_START"			},
		{CPS2PEEK_VIFMARK_GRASS_END,				"GRASS_END"				},

		{CPS2PEEK_VIFMARK_WATER_START,				"WATER_START"			},
		{CPS2PEEK_VIFMARK_WATER_END,				"WATER_END"				},

		{CPS2PEEK_VIFMARK_SPRITE_2D3D_START,		"SPRITE_2D3D_START"		},
		{CPS2PEEK_VIFMARK_SPRITE_2D3D_END,			"SPRITE_2D3D_END"		},
	
		{CPS2PEEK_VIFMARK_SPRITE_PRTCLE_START,		"SPRITE_PRTCLE_START"	},
		{CPS2PEEK_VIFMARK_SPRITE_PRTCLE_END,		"SPRITE_PRTCLE_END"		},
		
		{CPS2PEEK_VIFMARK_SPRITE_PRTCLEUV_START,	"SPRITE_PRTCLEUV_START"	},
		{CPS2PEEK_VIFMARK_SPRITE_PRTCLEUV_END,		"SPRITE_PRTCLEUV_END"	},
		
		{CPS2PEEK_VIFMARK_SPRITE_TRI3D_START,		"SPRITE_TRI3D_START"	},
		{CPS2PEEK_VIFMARK_SPRITE_TRI3D_END,			"SPRITE_TRI3D_START"	},
		
		{CPS2PEEK_VIFMARK_CARENVMAP_START,			"CAR_ENVMAP_START"		},
		{CPS2PEEK_VIFMARK_CARENVMAP_END,			"CAR_ENVMAP_END"		},
		
		{CPS2PEEK_VIFMARK_STARTOFFRAME_START,		"STARTOFFRAME_START"	},
		{CPS2PEEK_VIFMARK_STARTOFFRAME_END,			"STARTOFFRAME_END"		},

		{CPS2PEEK_VIFMARK_MBLUR_START,				"MBLUR_START"			},
		{CPS2PEEK_VIFMARK_MBLUR_END,				"MBLUR_END"				},
		
		{CPS2PEEK_VIFMARK_RENDER2DSTUFF_START,		"RENDER2DSTUFF_START"	},
		{CPS2PEEK_VIFMARK_RENDER2DSTUFF_END,		"RENDER2DSTUFF_END"		},



		{CPS2PEEK_VIFMARK_RENDERCLOUDS_START,		"RENDERCLOUDS_START"	},
		{CPS2PEEK_VIFMARK_RENDERCLOUDS_END,			"RENDERCLOUDS_END"		},
		
		
		{CPS2PEEK_VIFMARK_RENDERROADS_START,		"RENDERROADS_START"},
		{CPS2PEEK_VIFMARK_RENDERROADS_END,			"RENDERROADS_END"},
		{CPS2PEEK_VIFMARK_REFLECTIONS_START,		"REFLECTIONS_START"},
		{CPS2PEEK_VIFMARK_REFLECTIONS_END,			"REFLECTIONS_END"},
		{CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_START,	"EVERYTHINGBUTROADS_START"},
		{CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_END,	"EVERYTHINGBUTROADS_END"},
		{CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_START,	"HELISEARCHLIGHTS_START"},
		{CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_END,		"HELISEARCHLIGHTS_END"},
		
		
		{CPS2PEEK_VIFMARK_RENDERTWISTER_START,		"TWISTER_START"			},
		{CPS2PEEK_VIFMARK_RENDERTWISTER_END,		"TWISTER_END"			},
		{CPS2PEEK_VIFMARK_RENDERFADING_START,		"RENDERFADING_START"	},
		{CPS2PEEK_VIFMARK_RENDERFADING_END,			"RENDERFADING_END"		},
		{CPS2PEEK_VIFMARK_RENDERRAINANDSUN_START,	"RAINANDSUN_START"		},
		{CPS2PEEK_VIFMARK_RENDERRAINANDSUN_END,		"RAINANDSUN_END"		},
		{CPS2PEEK_VIFMARK_RENDERBTMFRHEIGHT_START,	"RENDERBTMFRHEIGHT_START"},
		{CPS2PEEK_VIFMARK_RENDERBTMFRHEIGHT_END,	"RENDERBTMFRHEIGHT_END"	},

		{CPS2PEEK_VIFMARK_BREAKMGR_START,			"BREAKMGR_START"},
		{CPS2PEEK_VIFMARK_BREAKMGR_END,				"BREAKMGR_END"},
		{CPS2PEEK_VIFMARK_BREAKMGRALPHA_START,		"BREAKMGRALPHA_START"},
		{CPS2PEEK_VIFMARK_BREAKMGRALPHA_END,		"BREAKMGRALPHA_END"},
		{CPS2PEEK_VIFMARK_INTERIORS_START,			"INTERIORS_START"},
		{CPS2PEEK_VIFMARK_INTERIORS_END,			"INTERIORS_END"},
		{CPS2PEEK_VIFMARK_DEBUGSHIT_START,			"DEBUGSHIT_START"},
		{CPS2PEEK_VIFMARK_DEBUGSHIT_END,			"DEBUGSHIT_END"},

		{CPS2PEEK_VIFMARK_GLASS_START,				"GLASS_START"},
		{CPS2PEEK_VIFMARK_GLASS_END,				"GLASS_END"},
		{CPS2PEEK_VIFMARK_WATERCANNONS_START,		"WATERCANNONS_START"},
		{CPS2PEEK_VIFMARK_WATERCANNONS_END,			"WATERCANNONS_END"},
		{CPS2PEEK_VIFMARK_SPECIALFX_START,			"SPECIALFX_START"},
		{CPS2PEEK_VIFMARK_SPECIALFX_END,			"SPECIALFX_END"},
		{CPS2PEEK_VIFMARK_STATICSHADOWS_START,		"STATICSHADOWS_START"},
		{CPS2PEEK_VIFMARK_STATICSHADOWS_END,		"STATICSHADOWS_END"},
		{CPS2PEEK_VIFMARK_STOREDSHADOWS_START,		"STOREDSHADOWS_START"},
		{CPS2PEEK_VIFMARK_STOREDSHADOWS_END,		"STOREDSHADOWS_END"},
		{CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_START,	"SKIDMARKSSTUNTS_START"},
		{CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_END,		"SKIDMARKSSTUNTS_END"},
		{CPS2PEEK_VIFMARK_ANTENNAS_START,			"ANTENNAS_START"},
		{CPS2PEEK_VIFMARK_ANTENNAS_END,				"ANTENNAS_END"},
		{CPS2PEEK_VIFMARK_RUBBISH_START,			"RUBBISH_START"},
		{CPS2PEEK_VIFMARK_RUBBISH_END,				"RUBBISH_END"},
		{CPS2PEEK_VIFMARK_CCORONAS_START,			"CCORONAS_START"},
		{CPS2PEEK_VIFMARK_CCORONAS_END,				"CCORONAS_END"},


		{CPS2PEEK_VIFMARK_PACMANPICKUPS_START,		"PACMANPICKUPS_START"},
		{CPS2PEEK_VIFMARK_PACMANPICKUPS_END,		"PACMANPICKUPS_END"},
		{CPS2PEEK_VIFMARK_WEAPONEFFECTS_START,		"WEAPONEFFECTS_START"},
		{CPS2PEEK_VIFMARK_WEAPONEFFECTS_END,		"WEAPONEFFECTS_END"},
		{CPS2PEEK_VIFMARK_RENDERFOGEFFECT_START,	"RENDERFOGEFFECT_START"},
		{CPS2PEEK_VIFMARK_RENDERFOGEFFECT_END,		"RENDERFOGEFFECT_END"},
		{CPS2PEEK_VIFMARK_MOVINGTHINGS_START,		"MOVINGTHINGS_START"},
		{CPS2PEEK_VIFMARK_MOVINGTHINGS_END,			"MOVINGTHINGS_END"},
		{CPS2PEEK_VIFMARK_CLIGHTING_START,			"CLIGHTING_START"},
		{CPS2PEEK_VIFMARK_CLIGHTING_END,			"CLIGHTING_END"},
		{CPS2PEEK_VIFMARK_FXSYSTEM_START,			"FXSYSTEM_START"},
		{CPS2PEEK_VIFMARK_FXSYSTEM_END,				"FXSYSTEM_END"},
		{CPS2PEEK_VIFMARK_SMOGEFFECT_START,			"SMOGEFFECT_START"},
		{CPS2PEEK_VIFMARK_SMOGEFFECT_END,			"SMOGEFFECT_END"},


		{CPS2PEEK_VIFMARK_RENDERMENUS_START,		"RENDERMENUS_START"},
		{CPS2PEEK_VIFMARK_RENDERMENUS_END,			"RENDERMENUS_END"},
		{CPS2PEEK_VIFMARK_DOFADE_START,				"DOFADE_START"},
		{CPS2PEEK_VIFMARK_DOFADE_END,				"DOFADE_END"},
		{CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_START,	"2DSTUFFAFTERFADE_START"},
		{CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_END,		"2DSTUFFAFTERFADE_END"},

		
		{CPS2PEEK_VIFMARK_CLEAR,					"CLR"					}
	};
const int32 vifMarksTabSize = sizeof(vifMarksTab)/sizeof(vifMarksTab[0]);




//
// VarConsole control variables:
//
static bool8 	bEnablePS2PeekProfiler	= FALSE;
static int32	nPS2PeekHertz			= TIMER1_DEFAULT_PROFILE_HZ;	 //120000

static int32	vblankIntHandlerID = -1;

//static int32 VIF1InterruptHandler(RwInt32 ca);



//
//
//
//
CPS2Peek::CPS2Peek()
{
}

//
//
//
//
CPS2Peek::~CPS2Peek()
{
}




//
//
//
//
bool8 CPS2Peek::Initialise()
{
	DEBUGLOG1("CPS2PEEK_VIFMARK_LAST=%d\n",CPS2PEEK_VIFMARK_LAST);

#ifdef GTA_PS2PEEK_VIFCODESONLY

	ASSERT(CPS2PEEK_VIFMARK_LAST < 255 /*65535*/);
	// do nothing
	return(TRUE);

#else

	ASSERT(CPS2PEEK_VIFMARK_LAST < 255);		// no more descriptors available in ClTimer1IntHandler::SetGfxDesc():

    // install VBlank and Timer1 interrupt handlers
    vblankIntHandlerID = AddIntcHandler(INTC_VBLANK_S, CPS2Peek::VBlankInterruptHandler, 0);    // Register as first handler
    if(vblankIntHandlerID == -1)
    {
    	ASSERTMSG(FALSE, "Error initialising CPS2Peek!");
    	return(FALSE);
    }
    
    EnableIntc(INTC_VBLANK_S);

//  	DisableIntc(INTC_VIF1);
//	int32 vif1IntHandlerID = AddIntcHandler(INTC_VIF1, VIF1InterruptHandler, 0);
//    // Clear interrupt:
// 	*(RwUInt32*)(VIF1_FBRST) = 0x08;
//	EnableIntc(INTC_VIF1);

    
	ClTimer1IntHandler::Install();


	//
	// set descriptions for VifMarks:
	//
	ASSERTMSG(vifMarksTabSize <= 255, "vifMarksTabSize too big, decrease number of vifMarks.");

	for(int32 i=0; i<vifMarksTabSize; i++)
	{
		ClTimer1IntHandler::SetGfxDesc(vifMarksTab[i].vifMark,	vifMarksTab[i].vifMarkDesc);

	}

	return(TRUE);
#endif//GTA_PS2PEEK_VIFCODESONLY...
}


//
//
//
//
//
bool8 CPS2Peek::InitialiseVarConsoleVars()
{
#ifdef GTA_PS2PEEK_VIFCODESONLY
	// do nothing
	return(TRUE);
#else

	//
	// VarConsole:
	//
	VarConsole.Add("Enable PS2Peek Profiler", &bEnablePS2PeekProfiler, TRUE);

	const int32 inc_val = 5000;
	const int32 min_val = TIMER1_MIN_PROFILE_HZ; 
	const int32 max_val = TIMER1_MAX_PROFILE_HZ;	
	VarConsole.Add("PS2Peek Profiler Freq",	&nPS2PeekHertz, inc_val, min_val, max_val, TRUE);

	return(TRUE);
#endif//GTA_PS2PEEK_VIFCODESONLY...
}



//
//
//
//
void CPS2Peek::Shutdown()
{
#ifdef GTA_PS2PEEK_VIFCODESONLY
	// do nothing
#else
	ClTimer1IntHandler::Death();


	DisableIntc(INTC_VBLANK_S);
	if(vblankIntHandlerID != -1)
	{
		RemoveIntcHandler(INTC_VBLANK_S, vblankIntHandlerID);
		vblankIntHandlerID = -1;
	}
#endif//GTA_PS2PEEK_VIFCODESONLY...
}



//
//
// inserts VIFcode MARK into currently built DMA stream:
//
void CPS2Peek::InsertVifMarkPkt(uint16 vifMark)
{

    RwBool aBool =_rwDMAOpenVIFPkt(0, 1);
    if(aBool)
    {
        RwSkySplitBits128   packed;

		packed.field_32[0].nUInt = VIFCMD_FLUSH;
		packed.field_32[1].nUInt = VIFCMD_NOP;
		packed.field_32[2].nUInt = VIFCMD_MARK 	| uint32(vifMark);
		packed.field_32[3].nUInt = VIFCMD_NOP;
		RWDMA_ADD_TO_PKT(packed.field128);
    
    }//if(aBool)...

}



//
//
//
//
void CPS2Peek::Profile()
{
#ifdef GTA_PS2PEEK_VIFCODESONLY
	// do nothing
#else
	if(!bEnablePS2PeekProfiler)
	{
		if(ClTimer1IntHandler::IsProfilerFinished())
		{
			ClTimer1IntHandler::StopProfiling();
		}

		return;
	}



	if (ClTimer1IntHandler::IsProfilerInactive())
	{
		// prepares the timer for profiling.... stats are collected starting with the next VBlank
		ClTimer1IntHandler::StartProfiling(nPS2PeekHertz);
	}
	else if(ClTimer1IntHandler::IsProfilerFinished())
	{
		// stop profiling and save to disk
		ClTimer1IntHandler::StopProfiling();
	}

#endif//GTA_PS2PEEK_VIFCODESONLY...
}// end of CPS2Peek::Profile()...




static volatile int32 numVBlanks		= 0;

//
//
//
//
int32 CPS2Peek::VBlankInterruptHandler(int32 cause)
{
#ifdef GTA_PS2PEEK_VIFCODESONLY
	// do nothing
	return(0);
#else
	numVBlanks++;
		
	#ifdef	TIMER1_HARDWARE_PROFILING  
	   	// conditionally increments the vblank count if it is in the PROFILING state
	   	ClTimer1IntHandler::IncVBlankIfActive();
	#endif // TIMER1_HARDWARE_PROFILING


	#ifdef	TIMER1_HARDWARE_PROFILING   
		// if the hardware profiler is in the GOACTIVE state, it should be activated     
        if (ClTimer1IntHandler::IsProfilerReady())
        {
        	ClTimer1IntHandler::ActivateProfiler();
        }
        else
        {
        	// conditionally increments the frame count if it is in the PROFILING state
        	ClTimer1IntHandler::IncFrameIfActive();
        }
	#endif // TIMER1_HARDWARE_PROFILING


    ExitHandler();
    return(0);    // Call any subsequently registered handlers

#endif//GTA_PS2PEEK_VIFCODESONLY...
}// end of CPS2Peek::VBlankInterruptHandler()...



#ifdef GTA_PS2PEEK_VIFCODESONLY
	// do nothing
#else
//
//
//
//
static
uint32 PS2Peek_CurrentVIF1Mark = 0;

static
int32 VIF1InterruptHandler(RwInt32 ca)
{
   // RwUInt32 localSavedGP;

    if (ca == INTC_VIF1)
    {
//        RwUInt32 currentMark;

        /* Set our $gp */
        //__asm__ __volatile__ ("and %0, $gp, $gp; lw $gp, 0(%1)"
        //                      : "=r&" (localSavedGP) : "r" (&rwDMASavedGP) );


        PS2Peek_CurrentVIF1Mark = *(RwUInt32*)(VIF1_MARK);


#if 0
        if ((currentMark == DMA_VIF_RUN_MRK)
            && (rwDMAcolour & 0x010000))
        {
            RwUInt64 col;
            col = rwDMAcolour | 0xff0000 ;
            *(volatile RwUInt64*)(0x120000e0) = col;
            rwDMAcolour = col;
            __asm__ __volatile__ ("sync.l" : : : "memory" );
        }
        else if ((currentMark == DMA_VIF_STOP_MRK)
                 && (rwDMAcolour & 0x010000))
        {
            RwUInt64 col;
            col = rwDMAcolour & ~0xfe0000 ;
            *(volatile RwUInt64*)(0x120000e0) = col;
            rwDMAcolour = col;
            __asm__ __volatile__ ("sync.l" : : : "memory" );
        }
#ifdef DMA_COLOUR_USER
        else if (((currentMark & 0xff00) == DMA_VIF_USER_MRK)
                 && (rwDMAcolour & 0x0100))
        {
            RwUInt64 col;
            col = rwDMAcolour & ~0xfe00 ;
            col |= (currentMark & 0xFF) << 8;
            *(volatile RwUInt64*)(0x120000e0) = col;
            rwDMAcolour = col;
            __asm__ __volatile__ ("sync.l" : : : "memory" );
        }
#endif /* DMA_COLOUR_USER */
#endif


        /* Restore $gp. Not really required */
       // __asm__ __volatile__ ("and $gp, %0, %0" : : "r" (localSavedGP) );

        /* Clear interrupt */
        *(RwUInt32*)(VIF1_FBRST) = 0x08;
    }
    ExitHandler();
    return(0);
}
#endif//GTA_PS2PEEK_VIFCODESONLY...




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif//GTA_PS2PEEK_PROFILER...
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


