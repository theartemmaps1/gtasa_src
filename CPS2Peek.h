#ifndef PCH_PRECOMPILED

//
// CPS2Peek - simple wrapper class for PS2peek profiler by Stormfront Studios;
//
//
//	26/08/2003	-	Andrzej:	- initial implementation;
//	12/08/2004	-	Andrzej:	- support for GTA_PS2PEEK_VIFCODESONLY macro added;
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
#ifndef __CPS2PEEK_H__
#define __CPS2PEEK_H__


//
//
//
#ifdef FINAL
	//
	// PS2Peek not available in FINAL/MASTER
	//
#else
	//
	// PS2Peek only available in DEBUG/RELEASE builds:
	//
	//
	// if defined, then PS2Peek full profiler support is available:
	//
	#define GTA_PS2PEEK_PROFILER			(1)
	//
	//
	// if defined, then CPS2PEEK_INSERT_VIFMARK() macro works, but all other PS2Peek functionality is disabled 
	//  (like sampling, interrupts, etc).
	//
	#define GTA_PS2PEEK_VIFCODESONLY		(1)
	//
	//
	//
#endif//FINAL...







//
//
//
//
//
#if defined (GTA_PS2PEEK_PROFILER) && defined (GTA_PS2)
	#define CPS2PEEK_INSERT_VIFMARK(MARK)		{if(MARK>=0) CPS2Peek::InsertVifMarkPkt(MARK);}
#else
	// do nothing
	#define CPS2PEEK_INSERT_VIFMARK(MARK)		{/*do nothing*/}
#endif




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GTA_PS2PEEK_PROFILER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////





//
// VifMark codes:
// 
// Note: they have to be in range <0; 255>!
//
enum
{
//	//////////////////////////////////////////
//	CPS2PEEK_VIFMARK_TO_BE_IGNORED		= -10000,
//
//	// insert vifmarks to be temporary ignored here:
//	// (particular enums should be negative to be ignored):



	//////////////////////////////////////////
	// all actual vifmarks are here:

	CPS2PEEK_VIFMARK_GRASS_START		= 1,
	CPS2PEEK_VIFMARK_GRASS_END,

	
	CPS2PEEK_VIFMARK_WATER_START,
	CPS2PEEK_VIFMARK_WATER_END,

	CPS2PEEK_VIFMARK_SPRITE_2D3D_START,
	CPS2PEEK_VIFMARK_SPRITE_2D3D_END,

	CPS2PEEK_VIFMARK_SPRITE_PRTCLE_START,
	CPS2PEEK_VIFMARK_SPRITE_PRTCLE_END,

	CPS2PEEK_VIFMARK_SPRITE_PRTCLEUV_START,
	CPS2PEEK_VIFMARK_SPRITE_PRTCLEUV_END,

	CPS2PEEK_VIFMARK_SPRITE_TRI3D_START,
	CPS2PEEK_VIFMARK_SPRITE_TRI3D_END,


	CPS2PEEK_VIFMARK_CARENVMAP_START,
	CPS2PEEK_VIFMARK_CARENVMAP_END,
	
	CPS2PEEK_VIFMARK_STARTOFFRAME_START,
	CPS2PEEK_VIFMARK_STARTOFFRAME_END,
	
	CPS2PEEK_VIFMARK_MBLUR_START,
	CPS2PEEK_VIFMARK_MBLUR_END,

	CPS2PEEK_VIFMARK_RENDER2DSTUFF_START,
	CPS2PEEK_VIFMARK_RENDER2DSTUFF_END,


	CPS2PEEK_VIFMARK_RENDERCLOUDS_START,
	CPS2PEEK_VIFMARK_RENDERCLOUDS_END,

	CPS2PEEK_VIFMARK_RENDERROADS_START,
	CPS2PEEK_VIFMARK_RENDERROADS_END,
	
	CPS2PEEK_VIFMARK_REFLECTIONS_START,
	CPS2PEEK_VIFMARK_REFLECTIONS_END,
	
	CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_START,
	CPS2PEEK_VIFMARK_EVERYTHINGBUTROADS_END,
	
	CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_START,
	CPS2PEEK_VIFMARK_HELISEARCHLIGHTS_END,

	CPS2PEEK_VIFMARK_RENDERTWISTER_START,
	CPS2PEEK_VIFMARK_RENDERTWISTER_END,

	CPS2PEEK_VIFMARK_RENDERFADING_START,
	CPS2PEEK_VIFMARK_RENDERFADING_END,

	CPS2PEEK_VIFMARK_RENDERRAINANDSUN_START,
	CPS2PEEK_VIFMARK_RENDERRAINANDSUN_END,

	CPS2PEEK_VIFMARK_RENDERBTMFRHEIGHT_START,
	CPS2PEEK_VIFMARK_RENDERBTMFRHEIGHT_END,


	CPS2PEEK_VIFMARK_BREAKMGR_START,
	CPS2PEEK_VIFMARK_BREAKMGR_END,
	CPS2PEEK_VIFMARK_BREAKMGRALPHA_START,
	CPS2PEEK_VIFMARK_BREAKMGRALPHA_END,

	CPS2PEEK_VIFMARK_INTERIORS_START,
	CPS2PEEK_VIFMARK_INTERIORS_END,

	CPS2PEEK_VIFMARK_DEBUGSHIT_START,
	CPS2PEEK_VIFMARK_DEBUGSHIT_END,



	CPS2PEEK_VIFMARK_GLASS_START,
	CPS2PEEK_VIFMARK_GLASS_END,

	CPS2PEEK_VIFMARK_WATERCANNONS_START,
	CPS2PEEK_VIFMARK_WATERCANNONS_END,

	CPS2PEEK_VIFMARK_SPECIALFX_START,
	CPS2PEEK_VIFMARK_SPECIALFX_END,

	CPS2PEEK_VIFMARK_STATICSHADOWS_START,
	CPS2PEEK_VIFMARK_STATICSHADOWS_END,

	CPS2PEEK_VIFMARK_STOREDSHADOWS_START,
	CPS2PEEK_VIFMARK_STOREDSHADOWS_END,

	CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_START,
	CPS2PEEK_VIFMARK_SKIDMARKSSTUNTS_END,

	CPS2PEEK_VIFMARK_ANTENNAS_START,
	CPS2PEEK_VIFMARK_ANTENNAS_END,

	CPS2PEEK_VIFMARK_RUBBISH_START,
	CPS2PEEK_VIFMARK_RUBBISH_END,

	CPS2PEEK_VIFMARK_CCORONAS_START,
	CPS2PEEK_VIFMARK_CCORONAS_END,



	CPS2PEEK_VIFMARK_PACMANPICKUPS_START,
	CPS2PEEK_VIFMARK_PACMANPICKUPS_END,
	CPS2PEEK_VIFMARK_WEAPONEFFECTS_START,
	CPS2PEEK_VIFMARK_WEAPONEFFECTS_END,
	CPS2PEEK_VIFMARK_RENDERFOGEFFECT_START,
	CPS2PEEK_VIFMARK_RENDERFOGEFFECT_END,
	CPS2PEEK_VIFMARK_MOVINGTHINGS_START,
	CPS2PEEK_VIFMARK_MOVINGTHINGS_END,
	CPS2PEEK_VIFMARK_CLIGHTING_START,
	CPS2PEEK_VIFMARK_CLIGHTING_END,
	CPS2PEEK_VIFMARK_FXSYSTEM_START,
	CPS2PEEK_VIFMARK_FXSYSTEM_END,
	CPS2PEEK_VIFMARK_SMOGEFFECT_START,
	CPS2PEEK_VIFMARK_SMOGEFFECT_END,


	CPS2PEEK_VIFMARK_RENDERMENUS_START,
	CPS2PEEK_VIFMARK_RENDERMENUS_END,
	CPS2PEEK_VIFMARK_DOFADE_START,
	CPS2PEEK_VIFMARK_DOFADE_END,
	CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_START,
	CPS2PEEK_VIFMARK_2DSTUFFAFTERFADE_END,


	// add new VifMarks here:
	




/////////////////////////////////////////////
	/////////////////////////////////////////
	// DON'T ADD ANYTHING AFTER THIS LINE: //
	/////////////////////////////////////////
	CPS2PEEK_VIFMARK_LAST,

#ifdef GTA_PS2PEEK_VIFCODESONLY
	CPS2PEEK_VIFMARK_CLEAR			= 0xFFFF	// this is reserved value
#else
	CPS2PEEK_VIFMARK_CLEAR			= 0xFF		// this is reserved value
#endif
};





//
//
//
//
class CPS2Peek
{
public:
	CPS2Peek();
	~CPS2Peek();

public:
	static bool8	Initialise();
	static void		Shutdown();

	static bool8	InitialiseVarConsoleVars();


public:
	static void		InsertVifMarkPkt(uint16 vifMark);
	static void		Profile();


private:
	static int32	VBlankInterruptHandler(int32 cause);

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif//GTA_PS2PEEK_PROFILER...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif//__CPS2PEEK_H__...

#endif