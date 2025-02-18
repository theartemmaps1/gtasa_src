//
//
//    Filename: ModelInfoAccel.cpp
//     Creator: Derek Ward
//     $Author: $
//       $Date: $
//   $Revision: $
// Description: 
//
//
// Game headers
#include "ModelInfoAccel.h"
#include "modelinfo.h"
#include "filemgr.h"
#include "streaming.h"

// Globals
#define NUM_ACCELERATORS	(NUM_MODEL_INFOS+NUM_EXTRADIRINFOS)

#ifdef MODEL_INFO_ACCELERATOR
	CModelInfoAccelerator gModelInfoAccelerator; // A reuasable instance if used carefully (non overlapping).. if they overlap in any way then simply create another instance with a different file to use.
#endif

//--------------------------------------------------
CModelInfoAccelerator::CModelInfoAccelerator()
{
	Init();
}

//--------------------------------------------------
CModelInfoAccelerator::~CModelInfoAccelerator()
{
	ASSERT(m_pArrayModelInfoIds == NULL); // would indicate a leak if this asserts.
}

//--------------------------------------------------
void CModelInfoAccelerator::Init()
{
	m_nModelInfosAdded = 0;
	m_bFileFound = false;
	m_pArrayModelInfoIds = NULL;
	m_bHasRun = false;	
	strcpy(m_FileName,"");
}

//--------------------------------------------------
bool  CModelInfoAccelerator::GetModelInfoIdFile(void)
{	
#if defined (GTA_PS2)
	int32 fd = CFileMgr::OpenFile(m_FileName); // ATTEMPT TO SEE IF FILE EXISTS
#else //GTA_PS2
	int32 fd = CFileMgr::OpenFile(m_FileName,"rb");
#endif //GTA_PS2

	m_bFileFound = fd > 0;

	#ifdef CDROM // Carry on regardless if file missing on DVD build
		if (!m_bFileFound)
			return false;
	#endif
			
	AllocModelInfoIds(); // ALLOC MEM FOR READING OR WRITING
				
	if (m_bFileFound) // it is valid for the file to be missing.
	{   		
		int32 sizeRead = NUM_ACCELERATORS*sizeof(uint16);
		int32 readSize = CFileMgr::Read(fd, (char*) m_pArrayModelInfoIds, sizeRead); // READ FILE INTO BUFFER
		ASSERT(readSize == sizeRead);
							
		CFileMgr::CloseFile(fd); // CLOSE FILE
	}
	
	return m_bFileFound;
}

//--------------------------------------------------
void CModelInfoAccelerator::EndOfLoadPhase(void)
{
	if (!m_bFileFound)
	{
		#ifdef CDROM // Carry on regardless if file missing on DVD build
			return;
		#endif
	
		int32 fd = CFileMgr::OpenFileForWriting(m_FileName); 		// Time to write data to file so OPEN FILE for writing
		ASSERTMSG(fd,"MODELINFO_ACCELERATOR_FILENAME file is missing! ...DVD version should have the file provided manually by *somebody* hence this may ASSERT");
										
		if(fd > 0)
		{			
			int32 sizeWrite = NUM_ACCELERATORS*sizeof(uint16);
			int32 writeSize = CFileMgr::Write(fd, (char*) m_pArrayModelInfoIds, sizeWrite); // WRITE DATA
			ASSERT(sizeWrite == writeSize);
							
			CFileMgr::CloseFile(fd); // CLOSE FILE
		}	
	}

	FreeModelInfoIds();
}

//--------------------------------------------------
void CModelInfoAccelerator::AddModelInfoId(uint16 id)
{	
#ifdef CDROM 
	if (!m_bFileFound)
		return;
#endif

	m_pArrayModelInfoIds[m_nModelInfosAdded] = id;
	m_nModelInfosAdded++;
	ASSERT(m_nModelInfosAdded<NUM_ACCELERATORS);
}


//--------------------------------------------------
uint16  CModelInfoAccelerator::GetNextModelInfoId(void)
{
#ifdef CDROM 
	ASSERT(false);
#endif

	uint16 id = m_pArrayModelInfoIds[m_nModelInfosAdded];
	m_nModelInfosAdded++;
	return id;
}

//--------------------------------------------------
void CModelInfoAccelerator::AllocModelInfoIds(void)
{
	ASSERT(!m_pArrayModelInfoIds);
	m_pArrayModelInfoIds = new uint16[NUM_ACCELERATORS];
	ASSERT(m_pArrayModelInfoIds);
	memset(m_pArrayModelInfoIds,0,NUM_ACCELERATORS*sizeof(uint16));
}


//--------------------------------------------------
void CModelInfoAccelerator::FreeModelInfoIds(void)
{
	ASSERT(m_pArrayModelInfoIds);
	delete [] m_pArrayModelInfoIds;
	m_pArrayModelInfoIds = NULL;
}


//--------------------------------------------------
void CModelInfoAccelerator::GetEntry(CBaseModelInfo **ppModelInfo, int32 *pIndex, char *pName)
{
#ifdef MODEL_INFO_ACCELERATOR
	if (m_bFileFound)
	{
		*ppModelInfo = CModelInfo::GetModelInfoFast(this, pName, pIndex);		// We are accelerated					
	}
	else
	{
		*ppModelInfo = CModelInfo::GetModelInfo(pName, pIndex);					// We are not accelerated yet, do it the slow way and build up results.
		
		#ifndef CDROM
		if(*ppModelInfo)
		{
			AddModelInfoId(*pIndex);
		}
		else
			AddModelInfoId(INVALID_MODELID);
		#endif
	}	
#endif 
}

//--------------------------------------------------
void CModelInfoAccelerator::Begin(char *pFileName)
{
	ASSERT(strlen(pFileName) < MODELINFO_ACCELERATOR_MAX_FILELENGTH);
	strcpy(m_FileName,pFileName);

	if(!m_bHasRun)
	{
		GetModelInfoIdFile();
	}
}

//--------------------------------------------------
void CModelInfoAccelerator::End(char *pFileName)
{
#ifdef MODEL_INFO_ACCELERATOR
	ASSERT(!strcmp(pFileName,m_FileName)); // warning an overlap is detected.

	if(!m_bHasRun)
	{
		gModelInfoAccelerator.EndOfLoadPhase();
		m_bHasRun = true;
	}
#endif 
}

