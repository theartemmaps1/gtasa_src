//-----------------------------------------------------------------
//
//	Copyright (c) 2001-2003 Stormfront Studios, Inc.
//	Confidential Trade Secrets
//	
//	PS2 Performance Profiler - portions by Ewen Vowels
//
//-----------------------------------------------------------------
//
//	28/08/2003	-	Andrzej:	- small adaptation changes;
//
//
//
//
#ifndef __CLTIMER1INTHANDLER_H__
#define __CLTIMER1INTHANDLER_H__



#pragma once

#define TIMER1_HARDWARE_PROFILING


#ifndef SFPS2

	// define the external build (removes some Stormfront only code)
	#define	TIMER1_EXTERNAL_BUILD

	// define standard types for external build
	typedef int                     SFINT;
	typedef char                    SFINT8;
	typedef short int               SFINT16;
	typedef int                     SFINT32;
	typedef long long               SFINT64;

	typedef signed int              SFSINT;
	typedef signed char             SFSINT8;
	typedef signed short int        SFSINT16;
	typedef signed int              SFSINT32;
	typedef signed long long        SFSINT64;
	typedef unsigned int            SFSINT128 __attribute__ ((mode(TI)));

	typedef unsigned int            SFUINT;
	typedef unsigned char           SFUINT8;
	typedef unsigned short int      SFUINT16;
	typedef unsigned int            SFUINT32;
	typedef unsigned long long      SFUINT64;
	typedef unsigned int            SFUINT128 __attribute__ ((mode(TI)));
	
	// define SFNULL if it is not defined
	#ifndef SFNULL
	    #ifdef  __cplusplus
	        #define SFNULL    0
	    #else
	        #define SFNULL    ((void *)0)
	    #endif
	#endif
	
#ifndef ASSERT
	#ifdef DEBUG
		#define ASSERT				assert
	#else
		#define ASSERT(expr)		((void)0)
	#endif
#endif
	
	// memory constants
	const SFUINT32 UNCACHED_MEMORY_RANGE				= 0x20000000;
	const SFUINT32 UNCACHED_ACCELERATED_MEMORY_RANGE	= 0x30000000;
	const SFUINT32 SCRATCHPAD_MEMORY_MASK				= 0x70000000;
	const SFUINT32 SCRATCHPAD_MEMORY_ADDRESS			= 0x70000000;
	const SFUINT32 SCRATCHPAD_MEMORY_SIZE				= 0x00004000; // 16384 bytes
	const SFUINT32 SCRATCHPAD_MEMORY_END_ADDRESS		= 0x70004000;
	
	// align macro
	#define SFS_ATTRIB_ALIGN(val)  __attribute__((aligned(val)))
		
#endif

//=============================================================================
// C L A S S
//=============================================================================

//
// number of frames collected in 1 profile session:
//
#define	PROFILER_BUFFER_NUM_FRAMES			(60)			// 1 second of ntsc
//#define		PROFILER_BUFFER_NUM_FRAMES			(2*60)			// 1 second of ntsc
//#define	PROFILER_BUFFER_NUM_FRAMES			(15)			// 1 second of ntsc
//#define	PROFILER_BUFFER_NUM_FRAMES			(5)			// 1 second of ntsc





// default sampling frequency
#define TIMER1_DEFAULT_PROFILE_HZ		(PROFILER_BUFFER_NUM_FRAMES*2000)	// 2000 samples per frame

// minimum sampling frequency
#define TIMER1_MIN_PROFILE_HZ			(60*10)		// 10 samples per frame

// maximum sampling frequency
#define TIMER1_MAX_PROFILE_HZ			(60*20000)	// 20000 samples per frame



///============================================================================
//
//	Interrupt handler for the PS2
// <P>
//  Deals with hardware profiling
//
///============================================================================
class ClTimer1IntHandler
{
	public:
    //=========================================================================
    // P U B L I C   M E T H O D S
    //=========================================================================

		//////////
		// Installs the timer1 interrupt handler but does not enable it
		static void Install();

		//////////
		// Sets the frequency of interrupts
		// NOTE: a value of 0 Hz will disable the interrupt
		static void SetTimerInterruptFrequency( const int newHertz );

		//////////
		// Uninstall the interrupt handler
		static void Death();

		//////////
		// Reinstalls the interrupt handler
		static void Resurrection();

#ifdef TIMER1_HARDWARE_PROFILING
		// sets an id/name pair for use with the vifmark decoding
		// gfxId must be >= 0 and <= 255
		static void SetGfxDesc(int gfxId, const char* gfxName);
		
		//////////
		// Prepares the hardware profiler by enabling the interrupt handler
		// NOTE: IsProfilerInactive() must be called to determine whether this function
		//       may be called
		// NOTE: This waits until the next VBlank before starting the stats collection
	#ifdef	TIMER1_EXTERNAL_BUILD
		static void StartProfiling( int hertz );
	#else
		static void StartProfiling();
	#endif

		//////////
		// Disables the hardware profiler and disables the interrupt handler
		// A file called host0:n:/profileX.bin is saved, where X is the number of times the 
		// profiler has been started since the program was running.
		// NOTE: IsProfilerFinished() must be called to determine whether this function
		//       may be called
		static void StopProfiling();

		//////////
		// Returns true if the profiler is inactive, at which point StartProfiling may be called
		static bool IsProfilerInactive();

		//////////
		// Returns true if the profiler is finished, that is it has collected the
		// maximum number of frames of information
		static bool IsProfilerFinished();

	//protected:
	public:
    //=========================================================================
    // P R O T E C T E D   M E T H O D S
    //=========================================================================

		//////////
		// the VBlank interrupt handler needs access to these methods
		friend int VBlankInterruptHandler( int cause );

		//////////
		// Returns true if ActivateProfiler should be called
		static bool IsProfilerReady();

		//////////
		// Starts stats collection
		// NOTE: IsProfilerReady() must be called to determine whether this function
		//       may be called
		static void ActivateProfiler();

		//////////
		// Called once each frame by the VBlank handler to record the number of vblanks for a logical game frame.
		static void IncVBlankIfActive();

		//////////
		// Called once each frame by the VBlank handler to record the stats for this frame.
		static void IncFrameIfActive();
#endif // TIMER1_HARDWARE_PROFILING

	private:
		// Do not call these
		ClTimer1IntHandler();
		~ClTimer1IntHandler();
};



#endif//__CLTIMER1INTHANDLER_H__...
