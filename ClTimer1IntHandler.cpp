//-----------------------------------------------------------------
//
//	Copyright (c) 2001-2003 Stormfront Studios, Inc.
//	Confidential Trade Secrets
//
//	PS2 Performance Profiler - portions by Ewen Vowels
//
//-----------------------------------------------------------------
//
//	28/08/2003	-	Andrzej:	- initial adaptation;
//	01/09/2003	-	Andrzej:	- PROFILE_DRIVE_LETTER & PROFILE_FOLDER added;
//
//
//
//
//
//
//
//=============================================================================
// I N C L U D E S
//=============================================================================

#include "ClTimer1IntHandler.h"

#include <eeregs.h>
#include <eekernel.h>
#include <stdio.h>
#include <sifdev.h>
#include <string.h>

#ifdef TIMER1_EXTERNAL_BUILD
	#include <assert.h>
	#include <libpc.h>
#else
	#include "SfTypes.h"
	#include "ClPerfCounters.h"
	#include "ClMemory.h"
	#include "ClDevConfiguration.h"
#endif

#include "CPS2Peek.h"	//GTA_PS2PEEK_PROFILER...



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GTA_PS2PEEK_PROFILER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//
//
// drive where profile files will be saved:
//
#define PROFILE_DRIVE_LETTER		"d:\\"
#define PROFILE_FOLDER				"profile\\"


// CHANGE THIS TO YOUR DRIVE!
static const char				*scDriveLetterPath = PROFILE_DRIVE_LETTER""PROFILE_FOLDER;








//
// Timer1 interrupt handler callback function:
//
static int Timer1InterruptHandler(int cause);


enum eTimer1SamplingRate
{
	RATE_BUS_FREQ			= 0x00,	// full bus frequency 14745600Hz
	RATE_1_ON_16_BUS_FREQ	= 0x01,	// 1/16th of full bus frequency
	RATE_1_ON_256_BUS_FREQ	= 0x02,	// 1/256th of full bus frequency
	RATE_HBLANK				= 0x03,	// horizontal blank interval (depends on ntsc or pal)
};



#ifdef TIMER1_HARDWARE_PROFILING
	// structs used by the profiler
	typedef struct ClHardwareSample		ClHardwareSample;
	typedef struct ClRLEHardwareSample	ClRLEHardwareSample;
	typedef struct ClFrameSample		ClFrameSample;
	typedef struct ClGfxSystemDesc		ClGfxSystemDesc;
	typedef struct ClHardwareProfile	ClHardwareProfile;

	//=============================================================================
	// T Y P E S
	//=============================================================================

	struct ClHardwareSample
	{
		SFINT32		mVif1_Stat;		// vif1 stat register
		SFINT32		mVpu_Stat;		// vpu stat register (from VU0 or COP2 register vi29)
		SFINT32		mGif_Stat;		// gif stat register
		SFINT16		mVif1_Mark;		// mark register
		SFINT16		mDma_Stats;		// bit 0 = vif0, bit1 = vif1, bit2 = gif
	};

	struct ClRLEHardwareSample
	{
		SFINT32				mRunLen;
		ClHardwareSample	mSample;
	};

	struct ClFrameSample
	{
		SFINT32		sampleStart;
		SFINT32		sampleEnd;
		SFINT32		mNumVBlanks;
	};

	struct ClGfxSystemDesc
	{
		SFINT32		mId;
		SFINT8		mName[124];
	};

	struct ClHardwareProfile
	{
		enum eProfileState
		{
			STATE_INACTIVE,
			STATE_GOACTIVE,
			STATE_PROFILING,
			STATE_FINISHED
		};
		
							ClHardwareProfile();
		void				Reset();
		void				SaveProfile(const SFINT8 *pFilename);
		
		SFINT32				mNumProfiles;
		eProfileState		mState;
		SFINT32				mNumSampledFrames;
		ClFrameSample		maSampleFrames[PROFILER_BUFFER_NUM_FRAMES];		// 1 second of ntsc
		SFINT32				mNumGfxSystems;
		ClGfxSystemDesc		maGfxSystems[256];		// 0-255 entries
	};

#endif//TIMER1_HARDWARE_PROFILING...


//=============================================================================
// C O N S T A N T S 
//=============================================================================
static const SFINT8					scProfileMagic[4]	= { 'M', 'I', 'P', 'A' };

// version 0x1004 - emv added RLE compression...
static const SFINT32				scProfileVersion	= 0x00001004;
static const SFINT32				scPs2BusFrequency	= 147456000;	// half cpu clock speed - refer sony manuals
static const eTimer1SamplingRate 	scSampleRate		= RATE_1_ON_16_BUS_FREQ;
static const SFINT32				scSampleFreqDiv		= scSampleRate == RATE_BUS_FREQ ? 1 : scSampleRate == RATE_1_ON_16_BUS_FREQ ? 16 : scSampleRate == RATE_1_ON_256_BUS_FREQ ? 256 : 1;



//=============================================================================
// S T A T I C   V A R I A B L E S
//=============================================================================

static SFINT32					sTimer1IntHandlerID = -1;
static SFINT32					sDesiredHertz = 0;
static SFINT32					sActualHertz = 0;
#ifdef TIMER1_EXTERNAL_BUILD
static SFINT32					sPerfCountersRunning = 0;
#endif

//=============================================================================
// H A R D W A R E   P R O F I L I N G
//=============================================================================

#ifdef TIMER1_HARDWARE_PROFILING
// static data for profiling
static volatile SFINT32			sNumSamples = 0;	// number of samples - updated in interrupt handler





static const SFINT32			scDefaultProfileHz = TIMER1_DEFAULT_PROFILE_HZ;
static const SFINT32			scMinimumProfileHz = TIMER1_MIN_PROFILE_HZ;
static const SFINT32			scMaximumProfileHz = TIMER1_MAX_PROFILE_HZ;

// use 62Mb (64Mb - 2Mb for stack) as the maximum number of samples
//static const SFINT32			scMaxHardwareSamples = ((64*1024*1024) - (2*1024*1024)) / sizeof(ClHardwareSample);
static const SFINT32			scMaxHardwareSamples = ((32*1024*1024) - (2*1024*1024)) / sizeof(ClHardwareSample);

// point directly at 64 Mb and use 62Mb of data
//static ClHardwareSample		*samples		= (ClHardwareSample*)(64*1024*1024); // 64 Mb
static ClHardwareSample			*samples		= (ClHardwareSample*)((96)*1024*1024); // 96 Mb


//
// 0x3400 -> 64MB UA
// 0x3600 -> 96MB UA
//
//#define		HEX_SAMPLES_ADR_UC			(0x3000|0x0400)
#define		HEX_SAMPLES_ADR_UC			(0x3000|0x0600)


// point directly at 126Mb
static ClRLEHardwareSample		*file_buffer	= (ClRLEHardwareSample*)(126 * 1024 * 1024);

// accumulate samples to scratchpad then store to the file buffer
static	ClRLEHardwareSample		*temp_buffer	= (ClRLEHardwareSample*)(SCRATCHPAD_MEMORY_ADDRESS);

// static profiler object
static ClHardwareProfile		sProfiler;


// number of frames to collect stats for 1 second
static const SFINT32			scMaxSampledFrames = sizeof(sProfiler.maSampleFrames) / sizeof(ClFrameSample);
#endif // TIMER1_HARDWARE_PROFILING





//=============================================================================
// P U B L I C   M E T H O D S
//=============================================================================

//----------------------------------------------------------------------------


//
//
//
//
void ClTimer1IntHandler::Install()
{
    static bool firstTime = true;
    if (!firstTime)
    {
        // Can only install the timer1 interrupt handler once
        asm("breakc 0");
        return;
    }
    firstTime = false;
    
    // Disable the timer1 interrupt
    DisableIntc( INTC_TIM1 );
    // Install our interrupt handler
    sTimer1IntHandlerID = AddIntcHandler(INTC_TIM1, Timer1InterruptHandler, -1);
    // Enable our interrupt handler
    //EnableIntc(INTC_TIM1);
    // disable to begin with
    SetTimerInterruptFrequency( 0 );  

}




//
//
//
//
void ClTimer1IntHandler::SetTimerInterruptFrequency( const int newHertz )
{
    // Disable the timer1 interrupt
    DisableIntc( INTC_TIM1 );

    if (newHertz != 0)
    {
    	int t1CompValue;
        sDesiredHertz = newHertz;
        t1CompValue  = ( scPs2BusFrequency / scSampleFreqDiv) / sDesiredHertz;
        sActualHertz = ( scPs2BusFrequency / scSampleFreqDiv) / t1CompValue;
        
   		//PRINTF("sDesiredHertz: %d actualHertz: %d t1CompValue: %d\n", sDesiredHertz, sActualHertz, t1CompValue);
   		scePrintf("sDesiredHertz: %d actualHertz: %d t1CompValue: %d maxSamples: %d\n", sDesiredHertz, sActualHertz, t1CompValue, scMaxHardwareSamples);
        
        // Set up timer1 registers
        // Clear the counter 
        *T1_COUNT = 0;
        // Set the compare value (this is the calculated by taking the bus frequency divided 
        // by the the bus sampling rate and then dividing by the desired frequency in hertz
        *T1_COMP = t1CompValue;
        // Set the timer mode
        *T1_MODE =	(int)scSampleRate |		// Bus sampling rate
		        	T_MODE_EQUF_M |			// Clear the equal flag
		            T_MODE_CMPE_M |			// Enable compare interrupt
		            T_MODE_CUE_M |			// Restart count
		            T_MODE_ZRET_M;			// Clear counter to 0 when counter == reference value

        // Enable our interrupt handler
        EnableIntc(INTC_TIM1);

    }
    else
    {
		*T1_COUNT = 1;
		*T1_COMP = 0;
		*T1_MODE = 0;
    }
}




//
//
//
//
void ClTimer1IntHandler::Death()
{
	*T1_COUNT = 1;
	*T1_COMP = 0;
	*T1_MODE = 0;
	
    DisableIntc( INTC_TIM1 );
    if (sTimer1IntHandlerID >= 0)
    {
    	RemoveIntcHandler(INTC_TIM1, sTimer1IntHandlerID);
    	sTimer1IntHandlerID = -1;
    }
}


//
//
//
//
void ClTimer1IntHandler::Resurrection()
{
	if (sTimer1IntHandlerID != -1)
	{
    	sTimer1IntHandlerID = AddIntcHandler(INTC_TIM1, Timer1InterruptHandler, -1);
    }
    SetTimerInterruptFrequency(sDesiredHertz);
}





#ifndef	TIMER1_HARDWARE_PROFILING
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
	int Timer1InterruptHandler(int cause)
	{
	    // Clear equal flag
		*T1_MODE = T_MODE_EQUF_M | *T1_MODE;

	    ExitHandler();
	    return -1;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else	TIMER1_HARDWARE_PROFILING
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//
// sets an id/name pair for use with the vifmark decoding
//
void ClTimer1IntHandler::SetGfxDesc(int gfxId, const char* gfxName)
{
	ASSERT(gfxId >= 0 && gfxId <= 255 && gfxName);
	if (gfxId >= 0 && gfxId <= 255 && gfxName)
	{
		sProfiler.maGfxSystems[sProfiler.mNumGfxSystems].mId = gfxId;
		ASSERT( (strlen(gfxName)+1) < sizeof(sProfiler.maGfxSystems[sProfiler.mNumGfxSystems].mName) );
		strcpy(sProfiler.maGfxSystems[sProfiler.mNumGfxSystems].mName, gfxName);
		sProfiler.mNumGfxSystems++;
	}
}




//
//
//
//
#ifdef	TIMER1_EXTERNAL_BUILD
	void ClTimer1IntHandler::StartProfiling( int hertz )
#else
	void ClTimer1IntHandler::StartProfiling()
#endif
{
	SFINT32 desiredFreq = scDefaultProfileHz;
	bool bAllowProfiling = false;
		
#ifdef	TIMER1_EXTERNAL_BUILD
	desiredFreq = hertz;
	// clamp to valid range
	if (desiredFreq < scMinimumProfileHz)
		desiredFreq = scMinimumProfileHz;
	if (desiredFreq > scMaximumProfileHz)
		desiredFreq = scMaximumProfileHz;
	bAllowProfiling = true;
#else
#ifdef SFINTERNAL
	#ifndef SFBURN
		if(DEV_CONFIGURATION_GET(MINIPA_FREQ))
		{
			// only allow profiling if MINIPA_FREQ is in the project.cfg file
			bAllowProfiling = true;
			desiredFreq = DEV_CONFIGURATION_GETINT(MINIPA_FREQ);
			// clamp to valid range
			if (desiredFreq < scMinimumProfileHz)
				desiredFreq = scMinimumProfileHz;
			if (desiredFreq > scMaximumProfileHz)
				desiredFreq = scMaximumProfileHz;
			PRINTF("MINIPA_FREQ desiredFreq: %d\n", desiredFreq);
		}
		else
		{
			PRINTF("MINIPA_FREQ NOT SET\n");
		}
	#endif
#endif
#endif
  	
	if (bAllowProfiling && sProfiler.mState == ClHardwareProfile::STATE_INACTIVE)
	{
		// pause the perfcounters
#ifdef TIMER1_EXTERNAL_BUILD
	    // Pauses the performance counting system by disabling the CTE (enable counter)
	    // bit in the CCR register. The rest of the bits in the CCR register remain the
	    // same so that we only have to set the CTE bit in Resume() to reenable performance
	    // counting.
	    register int ccr;
	    asm volatile ("mfps %0, 0" : "=r" (ccr) );     		// Get CCR register
	    asm volatile ("sync.p");
	    // make sure that the performance counters are running
	    if (ccr & SCE_PC_CTE)
	    {
		    ccr &= ~SCE_PC_CTE;                  			// Disable CTE (enable counter) bit
		    asm volatile ("mtps %0, 0" : : "r" (ccr) );		// Set CCR register
		    asm volatile ("sync.p");
		    sPerfCountersRunning = 1;	    
		}
#else
		ClPerfCounters::Pause();
#endif		
		
		sProfiler.mState = ClHardwareProfile::STATE_GOACTIVE;
		// reset the number of sampled frames
		sProfiler.mNumSampledFrames = 0;
		// turn on the interrupt handler
		ClTimer1IntHandler::SetTimerInterruptFrequency(desiredFreq);
	}
}


//
//
//
//
void ClTimer1IntHandler::StopProfiling()
{
	if (sProfiler.mState == ClHardwareProfile::STATE_FINISHED)
	{
		static SFINT8 saFileName[128];
		// go to inactive state
		sProfiler.mState = ClHardwareProfile::STATE_INACTIVE;
		// turn off the interrupt handler
		ClTimer1IntHandler::SetTimerInterruptFrequency(0);
		// save a profile
		sprintf(saFileName, "host0:%sprofile%04d.psk", scDriveLetterPath, sProfiler.mNumProfiles++);
		scePrintf("Saving profile: %s\n", saFileName);
		sProfiler.SaveProfile(saFileName);

		// restart the perfcounters
#ifdef TIMER1_EXTERNAL_BUILD
	    // Resumes the performace counting system by enabling the CTE (enable counter)
	    // bit in the CCR register. This assumes that the rest of the bits in CCR
	    // are reasonable values.
	    register int ccr = 0;  //DC: initialized to get rid of warning.  There must be a better way.
	    asm volatile ("mfps %0, 0" : "=r" (ccr) );     // Get CCR register
	    asm volatile ("sync.p");
	    if (sPerfCountersRunning)
	    {
		    //ASSERT((ccr & SCE_PC_CTE) == 0);				// Be sure that we are paused
		    ccr |= SCE_PC_CTE;                   			// Set CTE (enable counter) bit
		    asm volatile ("mtps %0, 0" : : "r" (ccr) );		// Set CCR register
		    asm volatile ("sync.p");
		    sPerfCountersRunning = 0;
		}
#else	    
		ClPerfCounters::Resume();		
#endif		
	}	
}


//
//
//
//
bool ClTimer1IntHandler::IsProfilerInactive()
{
	// is the profiler inactive, ie. is it ready to start profiling
	return sProfiler.mState == ClHardwareProfile::STATE_INACTIVE;	
}


//
//
//
//
bool ClTimer1IntHandler::IsProfilerFinished()
{
	// has the profiler finished
	return sProfiler.mState == ClHardwareProfile::STATE_FINISHED;	
}


//
//
//
//
bool ClTimer1IntHandler::IsProfilerReady()
{
	// is the profiler ready
	return sProfiler.mState == ClHardwareProfile::STATE_GOACTIVE;
}

//
//
//
//
void ClTimer1IntHandler::ActivateProfiler()
{
	// reset the number of samples
	sNumSamples = 0;
	// go to the profiling state
	sProfiler.mState = ClHardwareProfile::STATE_PROFILING;	
}


//
//
//
//
void ClTimer1IntHandler::IncVBlankIfActive()
{
	// get the current value for the number of samples collected
	int sampleNum = sNumSamples;
	
	// only add a frame if the profiler is active 
	if ( (sProfiler.mState != ClHardwareProfile::STATE_PROFILING) || (sProfiler.mNumSampledFrames >= scMaxSampledFrames) )
		return;
		
	// increment the number of vblanks for this frame
	sProfiler.maSampleFrames[sProfiler.mNumSampledFrames].mNumVBlanks++;
	
	// check if max samples was reached
	if (sNumSamples >= scMaxHardwareSamples)
	{
		// check for max frames reached
		if (sProfiler.mNumSampledFrames < scMaxSampledFrames)
		{
			// add end of frame sample count
			sProfiler.maSampleFrames[sProfiler.mNumSampledFrames++].sampleEnd = scMaxHardwareSamples-1;
		}
		// go to finished state
		sProfiler.mState = ClHardwareProfile::STATE_FINISHED;	
	}
}



//
//
//
//
void ClTimer1IntHandler::IncFrameIfActive()
{
	// get the current value for the number of samples collected
	int sampleNum = sNumSamples;
	
	// only add a frame if the profiler is active 
	if ( (sProfiler.mState != ClHardwareProfile::STATE_PROFILING) || (sProfiler.mNumSampledFrames >= scMaxSampledFrames) )
		return;
		
	// add end of frame sample count
	sProfiler.maSampleFrames[sProfiler.mNumSampledFrames++].sampleEnd = sampleNum;
	// check for max frames reached
	if (sProfiler.mNumSampledFrames >= scMaxSampledFrames)
	{
		// go to finished state
		sProfiler.mState = ClHardwareProfile::STATE_FINISHED;
	}
	else
	{
		// start a new frame
		sProfiler.maSampleFrames[sProfiler.mNumSampledFrames].sampleStart = sampleNum;		
	}



//////////////////////////////////////////////////////////////////////////
	// reset VIF1_MARK register to default state:
	//*VIF1_MARK		= CPS2PEEK_VIFMARK_CLEAR;	//0x000000ff;
//////////////////////////////////////////////////////////////////////////

}





//
//
//
//

#if 1
	asm int Timer1InterruptHandler(int cause)
	{
		.set noreorder
		.set noat
		.p2align 6					// 64 byte alignment for function
		
		lw		$t0, sNumSamples
		lui		$t1, 0x003e			// max samples
		
		//
		// 0x3400 - 64MB UA
		// 0x3600 - 96MB UA
		//
		lui		$t2, HEX_SAMPLES_ADR_UC	//0x3400=64 Mb with Uncached Accelerated Setting
		slt		$v0, $t0, $t1			// compare sNumSamples with scMaxHardwareSamples
		
		lui		$t6, 0x1000			// get high half of hardware register addresses...
		sll		$t7, $t0, 4			// multiply sNumSamples by 16 to get the array address
		
		beq		$0, $v0, Timer1InterruptHandler_Finished
		add		$t7, $t7, $t2		// sample address to store (done in branch delay slot)

	Timer1InterruptHandler_StoreSample:
		.p2align 3 					// 8 byte align for pairing
		
		addi	$t0, $t0, 1			// increment sNumSamples
		lwu		$t1, 0x3c30($t6)	// load VIF1_MARK note uses offset from vif1_stat
		
		ori		$v0, $0, 0x8000		// constant used below	
		sw		$t0, sNumSamples	// store sNumSamples
		
		lwu		$t0, 0x3c00($t6)	// load VIF1_STAT
		andi	$t1, $t1, 0xffff	// and off upper 16 bits of vif1_mark

		cfc2.ni	$t3, $vi29			// get vpu_stat register from Cop2
		addu	$v0, $t6, $v0		// need to add 0x8000 to get around 16bit offset limitation

		lwu		$t2, 0x3020($t6)	// load GIF_STAT
		pextlw	$t4, $t3, $t0		// merge vif1_stat with vpu_stat

		lwu		$t0, 0x0000($v0)	// load D0_CHCR, vif0 channel
		pextlw	$t5, $t1, $t2		// merge gif_stat with vif1_mark

		lwu		$t1, 0x1000($v0)	// load D1_CHCR, vif1 channel
		andi	$t0, $t0, D_CHCR_STR_M	// get Dma active mask
		
		lwu		$t2, 0x2000($v0)	// load D2_CHCR, gif channel
		andi	$t1, $t1, D_CHCR_STR_M	// get Dma active mask
		
		andi	$t2, $t2, D_CHCR_STR_M	// get Dma active mask
		dsll32	$t0, $t0, 8			// vif0 in bit 48
		
		dsll32	$t1, $t1, 9			// vif1 in bit 48
		dsll32	$t2, $t2, 10		// vif2 in bit 48
		
		or		$t0, $t0, $t1		// vif0 | vif1
		sd		$t4, 0x00($t7)		// store bottom 64 bits of sample data

		or		$t0, $t0, $t2		// vif0 | vif1 | gif
		or		$t5, $t0, $t5
		
		nop
		sd		$t5, 0x08($t7)		// store upper 64 bits of sample data
		
	Timer1InterruptHandler_Finished:
		.p2align 3 					// 8 byte align for pairing
		
		lw		$t7, 0x0810($t6)	// T1_MODE
		ori		$t7, $t7, T_MODE_EQUF_M
		sw		$t7, 0x0810($t6)	// store T1_MODE
		
		sync.l		// flush any stores
		ei			// enable interrupts
		
		jr	ra		// return
		li	v0, -1	// set return value to -1 in branch delay slot (-1 means DO NOT call any subsequently registered handlers)
		.set at
		.set reorder
	}

#else


	int Timer1InterruptHandler(int cause)
	{
		if (sNumSamples < scMaxHardwareSamples)
		{
			register SFINT32 dmaStat; 
			ClHardwareSample *sample = (ClHardwareSample*)(UNCACHED_ACCELERATED_MEMORY_RANGE | (SFUINT32)&samples[sNumSamples++]);
			sample->mVif1_Stat		= *VIF1_STAT;
			asm ("cfc2.ni	%0, vi29": "=r" (sample->mVpu_Stat)  );
			sample->mGif_Stat		= *GIF_STAT;
			
			sample->mVif1_Mark		= (SFINT16)*VIF1_MARK;
//			if(sample->mVif1_Mark != 0x00ff)
//			{
//				//scePrintf("\n\n sample->mVif1_Mark = %d\n\n", sample->mVif1_Mark); 
//				//case(CPS2PEEK_VIFMARK_GRASS):
//				//case(CPS2PEEK_VIFMARK_WATER):
//					// nop:
//					lastVifMark = sample->mVif1_Mark;
//				//break;
//			}
			//*VIF1_MARK		= 0x000000ff;//0x00BACA);
			
			dmaStat 				= ( (D_CHCR_STR_M&(*D0_CHCR))>>8  ) |	// vif0 channel in bit 0
									  ( (D_CHCR_STR_M&(*D1_CHCR))>>7  ) |	// vif1 channel in bit 1
									  ( (D_CHCR_STR_M&(*D2_CHCR))>>6  );	// gif channel in bit 2
			sample->mDma_Stats		= (SFINT16)dmaStat;


			// wait for wbb to flush
			asm __volatile__("sync.l");
		}
	    // Clear equal flag
		*T1_MODE	= T_MODE_EQUF_M | *T1_MODE;
		// Exit the handler
	    ExitHandler();
	    // DO NOT Call any subsequently registered handlers
	    return -1;
	}

#endif


//
//
//
//
//
//
ClHardwareProfile::ClHardwareProfile()
{
	mNumProfiles	= 0;
	mNumGfxSystems	= 0;
	for(SFINT32 gfx=0; gfx<sizeof(maGfxSystems)/sizeof(ClGfxSystemDesc); gfx++)
	{
		maGfxSystems[gfx].mId = -1;
		strcpy(maGfxSystems[gfx].mName, "No gfx system");
	}

	Reset();
}



//
//
//
//
//
void ClHardwareProfile::Reset()
{
	mState = STATE_INACTIVE; 
	mNumSampledFrames = 0;
	for(SFINT32 i=0; i<scMaxSampledFrames; i++)
	{
		maSampleFrames[i].sampleStart	= 0;
		maSampleFrames[i].sampleEnd		= 0;
	}
}




//
//
//
//
//
void ClHardwareProfile::SaveProfile(const SFINT8 *pFilename)
{
	SFINT	frame, fd, res, numSamplesForFrame, totalNumSamples;
	SFINT	gfx, numGfxSystems, sampleCnt, numRleItems, numInBuffer;
	
	if (pFilename == SFNULL)
		return;
		
	fd = sceOpen(pFilename, SCE_WRONLY|SCE_CREAT|SCE_TRUNC);
	if (fd < 0)
	{
		scePrintf("ERROR: SaveProfile unable to open file: %s\n", pFilename);
		return;
	}
		
	FlushCache(WRITEBACK_DCACHE);
	
	//IPRINTF("mNumSampledFrames: %d\n",mNumSampledFrames);
	if (mNumSampledFrames > 0)
	{
		totalNumSamples = maSampleFrames[mNumSampledFrames-1].sampleEnd;
	}
	else
	{
		totalNumSamples = 0;
	}
	numGfxSystems = sProfiler.mNumGfxSystems;
	
	// header
	// SFINT8	magic[4];
	// SFINT32	version;
	// SFINT32	requestedSampleRate
	// SFINT32	numSampledFrames
	// SFINT32	totalNumSamples
	// SFINT32	numberOfGfxSystems
	//
	// array of numberOfGfxSystems * ClGfxSystemDesc
	//	struct ClGfxSystemDesc
	//	{
	//		SFINT32		mId;
	//		SFINT8		mName[124];
	//	};
	//
	// array of numSampledFrames * ClFrameSample
	//	struct ClFrameSample
	//	{
	//		SFINT32		sampleStart;
	//		SFINT32		sampleEnd;
	//		SFINT32		mNumVBlanks;
	//	};
	//
	// array of totalNumSamples * ClFrameSample
	//	struct ClHardwareSample
	//	{
	//		SFINT32		mVif1_Stat;		// vif1 stat register
	//		SFINT32		mVpu_Stat;		// vpu stat register (from VU0 or COP2 register vi29)
	//		SFINT32		mGif_Stat;		// gif stat register
	//		SFINT16		mVif1_Mark;		// mark register
	//		SFINT16		mDma_Stats;		// bit 0 = vif0, bit1 = vif1, bit2 = gif
	//	};
	
	res = sceWrite(fd, &scProfileMagic, sizeof(SFINT8) * 4);	
	res = sceWrite(fd, &scProfileVersion, sizeof(SFINT32));	
	res = sceWrite(fd, &sActualHertz, sizeof(SFINT32));	
	res = sceWrite(fd, &mNumSampledFrames, sizeof(SFINT32));
	res = sceWrite(fd, &totalNumSamples, sizeof(SFINT32));
	res = sceWrite(fd, &numGfxSystems, sizeof(SFINT32));

	for (gfx = 0; gfx < numGfxSystems; gfx++)
	{
		SFINT32			strLen;
		ClGfxSystemDesc	gfxDesc;
		const SFINT8*	pGfxName = sProfiler.maGfxSystems[gfx].mName;
		
		gfxDesc.mId = gfx;
		//scePrintf("gfxsystem: %3d name: %s\n", gfx, pGfxName);	
		memset(gfxDesc.mName, 0, sizeof(gfxDesc.mName));
		strLen = strlen(pGfxName);
		if ( (strLen+1) >= sizeof(gfxDesc.mName))
			memcpy(gfxDesc.mName, pGfxName, sizeof(gfxDesc.mName)-1);
		else
			strcpy(gfxDesc.mName, pGfxName);
		// write the gfx name entry
		res = sceWrite(fd, &gfxDesc, sizeof(ClGfxSystemDesc));
	}	
	
	for (frame = 0; frame < mNumSampledFrames; frame++)
	{
		//IPRINTF("frame: %d, [ %8d -> %8d ]\n", frame, maSampleFrames[frame].sampleStart, maSampleFrames[frame].sampleEnd);	
		res = sceWrite(fd, &maSampleFrames[frame], sizeof(ClFrameSample));
	}

	// generate the RLE compressed samples and save them to disk
	// NOTE: temp RLE samples are stored to a scratchpad then copied to 126Mb and saved from there
	//       because calling sceWrite thousands of times is incredibly slow
	sampleCnt = 0;
	numRleItems = 0;
	numInBuffer = 0;
	for (frame = 0; frame < mNumSampledFrames; frame++)
	{
		ClRLEHardwareSample rleSample SFS_ATTRIB_ALIGN(64);
		
		numSamplesForFrame = maSampleFrames[frame].sampleEnd - maSampleFrames[frame].sampleStart;
		ASSERT(numSamplesForFrame > 0);
		ASSERT(maSampleFrames[frame].sampleEnd < scMaxHardwareSamples);
		SFINT32 i, numRuns, totalRuns, curr = maSampleFrames[frame].sampleStart;

		rleSample.mRunLen	= 1;
		rleSample.mSample	= samples[maSampleFrames[frame].sampleStart];
		numRuns				= 0;
		totalRuns			= 0;		
		sampleCnt			+= numSamplesForFrame;
		
		for (i = 1; i < numSamplesForFrame; i++)
		{
			if (memcmp(&rleSample.mSample, &samples[curr+i], sizeof(ClHardwareSample)) == 0)
			{
				rleSample.mRunLen++;
			}
			else
			{
				temp_buffer[numInBuffer++] 	= rleSample;
				if (numInBuffer >= (SCRATCHPAD_MEMORY_SIZE/sizeof(ClRLEHardwareSample) - 8) )
				{
					// run length encoded
					memcpy(file_buffer, temp_buffer, numInBuffer * sizeof(ClRLEHardwareSample)); 
					FlushCache(WRITEBACK_DCACHE);
					res = sceWrite(fd, file_buffer, numInBuffer * sizeof(ClRLEHardwareSample));	
					numInBuffer = 0;		
				}
				// run length encoded
				//res = sceWrite(fd, &rleSample, sizeof(ClRLEHardwareSample));			
				
				numRuns++;
				numRleItems++;
				totalRuns			+= rleSample.mRunLen;
				rleSample.mRunLen	= 1;
				rleSample.mSample	= samples[curr+i];
				// end of run
			}
		}
		// run length encoded
		numRleItems++;
		numRuns++;
		//res = sceWrite(fd, &rleSample, sizeof(ClRLEHardwareSample));
		temp_buffer[numInBuffer++] 	= rleSample;		
		memcpy(file_buffer, temp_buffer, numInBuffer * sizeof(ClRLEHardwareSample)); 
		FlushCache(WRITEBACK_DCACHE);
		res = sceWrite(fd, file_buffer, numInBuffer * sizeof(ClRLEHardwareSample));	
		numInBuffer = 0;
	}	
	
	FlushCache(WRITEBACK_DCACHE);
	res = sceClose(fd);

	//scePrintf("sampleCnt: %d numRleItems: %d\n", sampleCnt, numRleItems);	
	
	for (frame = 0; frame < mNumSampledFrames; frame++)
	{
		numSamplesForFrame = maSampleFrames[frame].sampleEnd - maSampleFrames[frame].sampleStart;
		ASSERT(numSamplesForFrame > 0);
		ASSERT(maSampleFrames[frame].sampleEnd < scMaxHardwareSamples);
//		scePrintf("frame: %02d numSamples: %06d, [ %d -> %d ] numVBlanks: %d\n", frame, numSamplesForFrame, maSampleFrames[frame].sampleStart, maSampleFrames[frame].sampleEnd, maSampleFrames[frame].mNumVBlanks);	
	}	
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif	// TIMER1_HARDWARE_PROFILING




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif//GTA_PS2PEEK_PROFILER...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////





