#ifndef PCH_PRECOMPILED

#if not defined (GTA_PS2)
	DONT_FUCKING_INCLUDE_THIS_HEADER_FOR_NON_PS2_PLATFORMS
#endif //!GTA_PS2
//
// file: eemacro.h
// description: useful macros when programming using the PS2 emotion engine
// written by Adam Fowler
//
#ifndef INC_EE_MACRO_H_
#define INC_EE_MACRO_H_

//#define PREINSTANCE

// non cached pointers
// uncached pointers are useful in read only situations
#define UNCACHED_PTR(c, p) ((c*)(((uint32)p) | 0x20000000))
// uncached accelerated pointers are useful in write only situations
#define UNCACHED_ACC_PTR(c, p) ((c*)(((uint32)p) | 0x30000000))


// Scratch Pad
#define SPR_PTR(c, p) ((c*)((p) | 0x70000000))
#define SPR_SIZE	16384

// calculation for DMA sizes
#define QSIZE				16
#define CLASS_QSIZE(c, n) 	((((n * sizeof(c))-1)/QSIZE)+1)
#define CACHELINE_SIZE				64
#define CLASS_CACHELINE_SIZE(c, n) 	((((n * sizeof(c))-1)/CACHELINE_SIZE)+1)

// Cache prefetch command
#ifndef DEBUG
#define PREFETCH(p) asm {"pref 0,0(%0)" : : "r"(p) }
#define PREFETCH_OFFSET(p, offset) asm {"pref 0,offset(%0)" : : "r"(p) }
#else
#define PREFETCH(p)
#define PREFETCH_OFFSET(p, offset)
#endif

// memory addresses
#ifdef CDROM
#define EE_MEMORY_SIZE		0x02000000
#elif defined(FINALBUILD)	
#define EE_MEMORY_SIZE		0x08000000	// final build but not a cd build
#elif defined(DEBUG)
#define EE_MEMORY_SIZE		0x08000000
#else

#ifdef PREINSTANCE
#define EE_MEMORY_SIZE		0x02060000
#else
#define EE_MEMORY_SIZE		0x08000000
#endif

#endif
#define MAX_EE_MEM_ADDR		(EE_MEMORY_SIZE - 1)
#define ASSERT_VALIDADDR(a)	ASSERTMSG((uint32)a >= 0x00100000 && (uint32)a <= MAX_EE_MEM_ADDR, "Invalid address")

#endif

#endif