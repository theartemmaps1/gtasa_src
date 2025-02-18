//
//
//    Filename: ModelInfoAccel.h
//     Creator: Derek Ward
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: Accelerator class for level load times and modelInfo array lookups.
//
//

#ifndef MODEL_INFO_ACCEL_H_INCED
#define MODEL_INFO_ACCEL_H_INCED

#include "basemodelinfo.h"

//!PC - this causes a number of issues on PC so it's disabled
#if defined (GTA_PS2) || defined (GTA_XBOX)
// SWITCH THE WHOLE ACCELERATOR ON/OFF - 100% backoutable code.
#define MODEL_INFO_ACCELERATOR 	
#endif //GTA_PS2 & GTA_XBOX

#define MODELINFO_ACCELERATOR_DEFAULT_FILENAME "MODELS\\MINFO.BIN" 	// The file to generate or load by default in the case of one instance being used.=										// switch the whole thing on / off
#define MODELINFO_ACCELERATOR_MAX_FILELENGTH 20						// Max length of any filename.

#define INVALID_MODELID		0xffff
// ------------------------------------------------------------------
// DW - Class to improve load times by caching to file modelInfo ids.
// Can be multiply instanced and run at any time where the results are deterministic.
class CModelInfoAccelerator
{
private:
	uint16 	*m_pArrayModelInfoIds;	// array of ids for saving out or loading in.
	uint16  m_nModelInfosAdded;		// how many added?
	char    m_FileName[MODELINFO_ACCELERATOR_MAX_FILELENGTH]; // The filename we are dealing with for acceleration.		
	bool    m_bHasRun;				// Makes sure its only run once at startup.
	bool 	m_bFileFound;			   // has the accelerator file been loaded?
	
	void 	AllocModelInfoIds(void);   // alloc workspace
	void 	FreeModelInfoIds(void);	   // free workspace

	bool    GetModelInfoIdFile(void);  // File checking phase
	void 	EndOfLoadPhase(void);	   // End of load phase nothing is required any more.

	void    AddModelInfoId(uint16 id); // Add an entry into the buffer 

public :				
	CModelInfoAccelerator();
	~CModelInfoAccelerator();

	void    Init();					   // Init members.... use for reinstancing.
	void    Begin(char *pFileName);	   // Marker for begin tracking.
	void    End(char *pFileName);	   // Marker for end tracking.
	
	uint16  GetNextModelInfoId(void);  // retreive next expected entry	
	void 	GetEntry(CBaseModelInfo **ppModelInfo, int32 *pIndex, char *pName); // Grabs the data we may have cached in a file OR sets the file up with another entry and gets the data the slow way anyway.
};

extern CModelInfoAccelerator gModelInfoAccelerator;

#endif
