//
//
//	file: TexturePool.cpp
// 	description: 
//	written by Adam Fowler
//	copyright DMA Design 2002
//

#if defined (GTA_PC) && !defined(OSW)
#include "windows.h"
#include "win.h"
#endif //GTA_PC

#include "TexturePool.h"
#include "Debug.h"

#if defined(GTA_PC) && !defined(OSW)


extern "C" {

extern LPDIRECT3DDEVICE_DXVER            _RwD3DDevice;

#if defined(RWDEBUG)
	extern HRESULT  _rwD3D9CheckError(HRESULT hr, const RwChar *function, const RwChar *file, RwUInt32 line);
	#define DXCHECK(fn) (_rwD3D9CheckError(fn, #fn, __FILE__, __LINE__))
#else
	#define DXCHECK(fn) (fn)
#endif

}

#ifndef FINALBUILD
bool gbIndexBufferError = false;
#endif

//
// name:		CTexturePool
//
class CTexturePool
{
public:
	CTexturePool() {}
	
	void Init(D3DFORMAT fmt, int32 dimensions, int32 levels, int32 size);
	void Destroy();
	void Empty();
	
	D3DFORMAT GetFormat() {return m_format;}
	float GetDimensions() {return m_dimensions;}
	int32 GetMipmapLevels() {return m_levels;}
	
	IDirect3DTexture_DXVER* GetTexture(int32 i) {if(i > m_number) return NULL; return m_pPool[i];}
	IDirect3DTexture_DXVER* RequestTexture();
	IDirect3DTexture_DXVER* RequestTexture(D3DFORMAT fmt, int32 width, int32 height, int32 levels);
	bool AddTexture(IDirect3DTexture_DXVER* pTexture);
		
	int32 GetSize() {return m_size;}
	int32 GetNumber() {return m_number;}
	int32 GetRealSize() {return m_realNumber;}
	bool RetryAddTexture(IDirect3DTexture_DXVER* pTexture) {m_realNumber--; return AddTexture(pTexture);}
	void Resize(int32 size);
	int32 GetMemoryUsed();
	
private:
	D3DFORMAT	m_format;
	int32		m_dimensions;
	int32		m_levels;
	
	int32		m_size;
	int32		m_realNumber;
	int32		m_number;
	IDirect3DTexture_DXVER** m_pPool;
};

//
// name:		CIndexBufferPool
//
class CIndexBufferPool
{
public:
	CIndexBufferPool() {}
	
	void Init(D3DFORMAT fmt, int32 bufferSize, int32 size);
	void Destroy();
	void Empty();
	
	D3DFORMAT GetFormat() {return m_format;}
	int32 GetIndexBufferSize() {return m_bufferSize;}
	
	IDirect3DIndexBuffer_DXVER* GetIndexBuffer(int32 i) {if(i > m_number) return NULL; return m_pPool[i];}
	IDirect3DIndexBuffer_DXVER* RequestIndexBuffer();
	IDirect3DIndexBuffer_DXVER* RequestIndexBuffer(int32 bufferSize);
	bool AddIndexBuffer(IDirect3DIndexBuffer_DXVER* pTexture);
		
	int32 GetSize() {return m_size;}
	int32 GetNumber() {return m_number;}
	int32 GetRealSize() {return m_realNumber;}
	bool RetryAddIndexBuffer(IDirect3DIndexBuffer_DXVER* pIndexBuffer) {m_realNumber--; return AddIndexBuffer(pIndexBuffer);}
	void Resize(int32 size);
	int32 GetMemoryUsed();
	
private:
	D3DFORMAT	m_format;
	int32		m_bufferSize;
	
	int32		m_size;
	int32		m_realNumber;
	int32		m_number;
	IDirect3DIndexBuffer_DXVER** m_pPool;
};

//
// name:		CPalettePool
// description:	Stores palette ids
class CPalettePool
{
public:
	CPalettePool() {}
	
	void Init(int32 size);
	void Destroy();
	void DestroyForRestart(int32 size);

	int32 RequestPalette();
	void AddPalette(int32 paletteId);
	void Resize(int32 size);

private:
	int32 m_size;
	int32 m_number;
	int32* m_pPool;
};


//
// --- CTexturePool ------------------------------------------------------------------------
//
//
// name:		Init
// description: Initialise a texture pool
void CTexturePool::Init(D3DFORMAT fmt, int32 dimensions, int32 levels, int32 size)
{
	m_format = fmt;
	m_dimensions = dimensions;
	m_levels = levels;
	
	m_pPool = new IDirect3DTexture_DXVER*[size];

	m_size = size;
	m_number = 0;
	m_realNumber = 0;
}

//
// name:		Empty
// description: Empty a texture pool by destroying all the textures in it
void CTexturePool::Empty()
{
	int32 i;
	for(i=0; i<m_number; i++)
	{
		ASSERT(m_pPool[i]);
		m_pPool[i]->Release();
	}	
	m_number = 0;
	m_realNumber = 0;
}

//
// name:		Destroy
// description: Destroy a texture pool and all the textures in it
void CTexturePool::Destroy()
{
	Empty();

	if (m_pPool)
	{
		delete[] m_pPool;
		m_pPool = NULL;
	}
}

//
// name:		RequestTexture
// description:	Request a texture from the texture pool
IDirect3DTexture_DXVER* CTexturePool::RequestTexture()
{
	if(m_number)
	{
		m_realNumber--;
		m_number--;
		return m_pPool[m_number];
	}
	return NULL;
}

//
// name:		RequestTexture
// description:	Request a texture from the texture pool with a given format and size
IDirect3DTexture_DXVER* CTexturePool::RequestTexture(D3DFORMAT fmt, int32 width, int32 height, int32 levels)
{
	int32 i = m_number;
	D3DSURFACE_DESC surfaceDesc;
	
	while(i--)
	{
		IDirect3DTexture_DXVER* pTexture = m_pPool[i];
		// if texture is mipmapped and this is required or it isnt mipmapped and that is required
		if((pTexture->GetLevelCount() == 1) == levels)
		{
			DXCHECK(pTexture->GetLevelDesc(0, &surfaceDesc));
			// surface has the required format and dimensions then use this texture
			if(surfaceDesc.Format == fmt &&
				surfaceDesc.Width == width &&
				surfaceDesc.Height == height)
			{
				m_number--;
				if(m_number > 0 && m_number != i)
					m_pPool[i] = m_pPool[m_number];
				return pTexture;
			}	
		}	
	}
	return NULL;
}

//
// name:		AddTexture
// description:	Add a texture to the texture pool
bool CTexturePool::AddTexture(IDirect3DTexture_DXVER* pTexture)
{
	m_realNumber++;
	if(m_number < m_size)
	{
		m_pPool[m_number] = pTexture;
		m_number++;
		return true;
	}
	return false;
}

//
// name:		Resize
// description:	Resize the texture pool
void CTexturePool::Resize(int32 size)
{
	if(size == m_size)
		return;
	
	// allocate space for new pool
	IDirect3DTexture_DXVER** pPool = new IDirect3DTexture_DXVER*[size];
	int32 i;
	
	// copy pool
	for(i=0; i<m_number && i<size; i++)
	{
		pPool[i] = m_pPool[i];
	}
	
	// if size of pool has decreased then release textures not used
	if(size < m_number)
	{
		for(i=size; i<m_number; i++)
		{
			ASSERT(m_pPool[i]);
			m_pPool[i]->Release();
		}	
	}

	delete[] m_pPool;
	m_pPool = pPool;
	m_size = size;
}

/*
static RwUInt32 D3D9SurfaceGetSize(D3DSURFACE_DESC surfaceDesc, RwUInt8 mipLevel)
{
    RwUInt32            blocksX, blocksY, totalBlocks;
    RwUInt32            width, height;
    RwUInt32            shift, size;

    // Calc mip level size 
    width = surfaceDesc.width >> mipLevel;
    height = surfaceDesc.height >> mipLevel;

    // Clamp width and height to 1 
    width = ((0 == width) ? 1 : width);
    height = ((0 == height) ? 1 : height);

    // Is the raster a DXT compressed format 
    if (0 == rasExt->compressed)
    {
        size = width * height * (raster->depth >> 3);
        return size;
    }
    else
    {
        // A DXT compressed format 
        if (D3DFMT_DXT1 == rasExt->d3dFormat)
        {
            shift = 3; // 64Bits / 8 = 8. 2^3 = 8 
        }
        else
        {
            shift = 4; // 128Bits / 8 = 16. 2^4 = 16 
        }

        blocksX = width >> 2;
        blocksY = height >> 2;

        blocksX = ((0 == blocksX) ? 1 : blocksX);
        blocksY = ((0 == blocksY) ? 1 : blocksY);

        totalBlocks = blocksX * blocksY;

        size = totalBlocks << shift;

        ASSERT(8 <= size);

        return size;

    }
    return -1;           
}
*/

//!PC - for dx9 the size might not be right - assumes texture is uncompressed!
//
// name:		GetMemoryUsed
// description:	Return the memory used by this texture pool
int32 CTexturePool::GetMemoryUsed()
{
	D3DSURFACE_DESC surfaceDesc;
	int32 mem = 0;
	
	if(m_format == D3DFMT_UNKNOWN)
	{
		for(int32 i=0; i<m_number; i++)
		{
			m_pPool[i]->GetLevelDesc(0, &surfaceDesc);
			// if mipmapped
			if(m_pPool[i]->GetLevelCount() == 0)
				mem += (surfaceDesc.Width * surfaceDesc.Height)*4.0f/3.0f;
			else	
				mem += (surfaceDesc.Width * surfaceDesc.Height);
		}
	}
	else
	{
		if(m_number)
		{
			m_pPool[0]->GetLevelDesc(0, &surfaceDesc);
			mem = (surfaceDesc.Width * surfaceDesc.Height) * m_number;
			// if mipmapped
			if(m_pPool[0]->GetLevelCount() == 0)
				mem *= 4.0f/3.0f;
		}
	}
	return mem;
}

//
// --- CIndexBufferPool ------------------------------------------------------------------------
//
//
// name:		Init
// description: Initialise an index buffer pool
void CIndexBufferPool::Init(D3DFORMAT fmt, int32 bufferSize, int32 size)
{
	m_format = fmt;
	m_bufferSize = bufferSize;
	
	m_pPool = new IDirect3DIndexBuffer_DXVER*[size];

	m_size = size;
	m_number = 0;
	m_realNumber = 0;
}

//
// name:		Empty
// description: Empty an index buffer pool by destroying all the index buffers in it
void CIndexBufferPool::Empty()
{
	int32 i;
	for(i=0; i<m_number; i++)
	{
		ASSERT(m_pPool[i]);
		m_pPool[i]->Release();
	}	

	m_number = 0;
	m_realNumber = 0;
}

//
// name:		Destroy
// description: Destroy an index buffer pool and all the index buffer in it
void CIndexBufferPool::Destroy()
{
	Empty();
	
	if (m_pPool)
	{
		delete[] m_pPool;
		m_pPool = NULL;
	}
}

//
// name:		RequestIndexBuffer
// description:	Request an index buffer from the index buffer pool
IDirect3DIndexBuffer_DXVER* CIndexBufferPool::RequestIndexBuffer()
{
	if(m_number)
	{
		m_realNumber--;
		m_number--;
		return m_pPool[m_number];
	}
	return NULL;
}

//
// name:		RequestIndexBuffer
// description:	Request an index buffer from the index buffer pool
IDirect3DIndexBuffer_DXVER* CIndexBufferPool::RequestIndexBuffer(int32 bufferSize)
{
	IDirect3DIndexBuffer_DXVER* pIndexBuffer;
	D3DINDEXBUFFER_DESC bufferDesc;
	int32 i = m_number;
	int32 smallestSize = 0x7fffffff;
	int32 index = -1;
	
	bufferSize *= sizeof(RwInt16);
	
	// find smallest buffer that index buffer would fit into
	while(i--)
	{
		pIndexBuffer = m_pPool[i];
		pIndexBuffer->GetDesc(&bufferDesc);
		ASSERT(bufferDesc.Format == D3DFMT_INDEX16);
		
		if(bufferDesc.Size < smallestSize && bufferDesc.Size >= bufferSize)
		{
			smallestSize = bufferDesc.Size;
			index = i;
		}
	}
	
	if(index != -1)
	{
		pIndexBuffer = m_pPool[index];
		m_number--;
		if(m_number > 0 && m_number != index)
			m_pPool[index] = m_pPool[m_number];
		return pIndexBuffer;
	}

	return NULL;
}

//
// name:		AddIndexBuffer
// description:	Add an index buffer to the index buffer pool
bool CIndexBufferPool::AddIndexBuffer(IDirect3DIndexBuffer_DXVER* pIndexBuffer)
{
	m_realNumber++;
	if(m_number < m_size)
	{
		m_pPool[m_number] = pIndexBuffer;
		m_number++;
		return true;
	}
	return false;
}

//
// name:		Resize
// description:	Resize the index buffer pool
void CIndexBufferPool::Resize(int32 size)
{
	if(size == m_size)
		return;
	
	// allocate space for new pool
	IDirect3DIndexBuffer_DXVER** pPool = new IDirect3DIndexBuffer_DXVER*[size];
	int32 i;
	
	// copy pool
	for(i=0; i<m_number && i<size; i++)
	{
		pPool[i] = m_pPool[i];
	}
	
	// if size of pool has decreased then release textures not used
	if(size < m_number)
	{
		for(i=size; i<m_number; i++)
		{
			ASSERT(m_pPool[i]);
			m_pPool[i]->Release();
		}	
	}

	delete[] m_pPool;
	m_pPool = pPool;
	m_size = size;
}


//
// name:		GetMemoryUsed
// description:	Return the memory used by this indexbuffer pool
int32 CIndexBufferPool::GetMemoryUsed()
{
	D3DINDEXBUFFER_DESC bufferDesc;
	int32 mem = 0;
	
	if(m_format == D3DFMT_UNKNOWN)
	{
		for(int32 i=0; i<m_number; i++)
		{
			m_pPool[i]->GetDesc(&bufferDesc);
			mem += bufferDesc.Size;
		}
	}
	else
	{
		if(m_number)
		{
			m_pPool[0]->GetDesc(&bufferDesc);
			mem = bufferDesc.Size * m_number;
		}
	}
	return mem;
}

//
// --- CPalettePool ------------------------------------------------------------------------
//
//
// name:		Init
// description:	Initialise palette id pool
void CPalettePool::Init(int32 size)
{
	m_pPool = new int32[size];
	m_size = size;
	m_number = 0;
}


//
// name:		Destroy
// description:	Destroy palette id pool
void CPalettePool::Destroy()
{
	if (m_pPool)
	{
		delete[] m_pPool;
		m_pPool = NULL;
	}
	m_number = 0;
}

//
// name:		Destroy
// description:	Destroy palette id pool
void CPalettePool::DestroyForRestart(int32 size)
{
/*
	int32 *pPool;
	
	pPool = new int32[size];
	pPool[0] = m_pPool[0];
	delete[] m_pPool;
	m_pPool = pPool;
	m_number = 10;
*/
CDebug::DebugLog("Palette Size %d\n", m_size);
}


//
// name:		RequestPalette
// description:	Request palette id from pool
int32 CPalettePool::RequestPalette()
{
	if(m_number)
	{
		m_number--;
		return m_pPool[m_number];
	}
	return -1;
}

//
// name:		AddPalette
// description:	Add palette id to pool
void CPalettePool::AddPalette(int32 paletteId)
{
	if(m_number < m_size)
	{
		m_pPool[m_number] = paletteId;
		m_number++;
		return;
	}
	Resize(m_size*2);
	AddPalette(paletteId);
}

//
// name:		Resize
// description:	Resize pool array
void CPalettePool::Resize(int32 size)
{
	if(size == m_size)
		return;
	
	// allocate space for new pool
	int32* pPool = new int32[size];
	int32 i;
	
	// copy pool
	for(i=0; i<m_number && i<size; i++)
	{
		pPool[i] = m_pPool[i];
	}
	
	delete[] m_pPool;
	m_pPool = pPool;
	m_size = size;
}



#define MAX_NUM_TEXTURE_POOLS	17
#define MAX_NUM_INDEXBUFFERS_POOLS 16
#define INDEXBUFFER_SIZE		100

static CTexturePool gTexturePoolArray[MAX_NUM_TEXTURE_POOLS];
static int32 gNumTexturePoolsUsed = 0;
static bool gEnableTexturePools = true;
//static CPalettePool gPalettePool;
//static int32 gNextPaletteAvailable = 0;

static CIndexBufferPool gIndexBufferPoolArray[MAX_NUM_INDEXBUFFERS_POOLS];
static CIndexBufferPool gAwkwardIndexBufferPool;
static bool gEnableIndexBufferPools = true;

//
// name:		TexturePoolCreateTexture
// description:	Either return a texture from one of the texture pools
//				or use D3D calls to create a new texture
HRESULT D3DPoolsCreateTexture(int32 width, int32 height, 
								int levels, D3DFORMAT format, 
								IDirect3DTexture_DXVER** ppTexture)
{
	CTexturePool* pTexPool = NULL;
	int32 i;
	
	//ASSERTMSG(levels == 1, "Mipmapping disabled in PC version of game");
	if(levels > 1)
		levels = 0;

	// Find texture pool texture should be in 
	if(width == height)
	{
		for(i=1; i<gNumTexturePoolsUsed; i++)
		{
			if(width == gTexturePoolArray[i].GetDimensions() &&
				levels ==  gTexturePoolArray[i].GetMipmapLevels() &&
				format == gTexturePoolArray[i].GetFormat())
			{
				pTexPool = &gTexturePoolArray[i];
				break;
			}	
		}
	}

	// if found a texture pool then request texture from it, otherwise see if texture is the
	// awkward texture pool
	if(pTexPool)
		*ppTexture = pTexPool->RequestTexture();
	else	
		*ppTexture = gTexturePoolArray[0].RequestTexture(format, width, height, levels);
	
	if(*ppTexture)
		return D3D_OK;

	return DXCHECK(_RwD3DDevice->CreateTexture(width,
                                               height,
                                               levels,
                                               0,
                                               format,
                                               D3DPOOL_MANAGED,
                                               ppTexture,
											   NULL));                                       
}

//
// name:		TexturePoolReleaseTexture
// description:	Either release a texture resource or store it in one of
//				the texture pools
void D3DPoolsReleaseTexture(IDirect3DTexture_DXVER* pTexture)
{
	D3DSURFACE_DESC surfaceDesc;
	int32 levels = 1;
	int32 i;
	
	if(gEnableTexturePools)
	{
		// if there is a second surface then texture must be mipmapped
		if(pTexture->GetLevelCount() > 1)
			levels = 0;
			
		DXCHECK(pTexture->GetLevelDesc(0, &surfaceDesc));
		// Texture pools are only used for square textures
		if(surfaceDesc.Width == surfaceDesc.Height)
		{
			for(i=1; i<gNumTexturePoolsUsed; i++)
			{
				CTexturePool* pTexPool = &gTexturePoolArray[i];
				if(surfaceDesc.Width == pTexPool->GetDimensions() &&
					surfaceDesc.Format == pTexPool->GetFormat() &&
					levels == pTexPool->GetMipmapLevels())
				{
					// if failed to add texture to the pool then resize pool
					if(!pTexPool->AddTexture(pTexture))
					{
//						if(pTexPool->GetRealSize() > (pTexPool->GetSize() * 3) / 2)
						{
							bool bAdded;
							pTexPool->Resize(pTexPool->GetSize()*2);
							bAdded = pTexPool->RetryAddTexture(pTexture);
							ASSERT(bAdded);
							return;
						}
//						pTexture->Release();
					}	
					return;
				}	
			}
		}
		
		//
		// do we want to create a pool for this format
		//
		if(gNumTexturePoolsUsed < MAX_NUM_TEXTURE_POOLS)
		{
			if(surfaceDesc.Width == surfaceDesc.Height)
			{
				// only allow textures between the following sizes
				if(surfaceDesc.Width >= 32 && surfaceDesc.Width <= 256)
				{
					bool bAdded;
					// Start texture pool at initial size of 16
					gTexturePoolArray[gNumTexturePoolsUsed].Init(surfaceDesc.Format, 
																surfaceDesc.Width, 
																levels, 16);
					bAdded = gTexturePoolArray[gNumTexturePoolsUsed].AddTexture(pTexture);
					ASSERT(bAdded);
					gNumTexturePoolsUsed++;											
					return;
				}
			}
		}
		
		// only allow textures below 256 pixels in size
		if(surfaceDesc.Width <= 256 && surfaceDesc.Height <= 256)
		{
			// awkward size texture pool
			CTexturePool* pTexPool = &gTexturePoolArray[0];
			// if failed to add texture to the pool then resize pool
			if(!pTexPool->AddTexture(pTexture))
			{
				bool bAdded;
				pTexPool->Resize(pTexPool->GetSize()*2);
				bAdded = pTexPool->RetryAddTexture(pTexture);
				ASSERT(bAdded);
			}	
			return;
		}	
	}
		
	pTexture->Release();
}


//
// name:		D3DPoolsReleaseTextures
// description:	Release a set number of textures. This is normally done when the screen is faded
//				out as it takes a while to do
void D3DPoolsReleaseTextures(uint32 num)
{
	static int32 index = 0;
	int32 lastDeleted = 0;
	while(num && lastDeleted < gNumTexturePoolsUsed)
	{
		IDirect3DTexture_DXVER* pTexture = gTexturePoolArray[index].RequestTexture();
		if(pTexture)
		{
			pTexture->Release();
			lastDeleted = 0;
			num--;
		}
		else
		{
			lastDeleted++;
		}
		index++;
		index %= gNumTexturePoolsUsed;
	}
}

//
// name:		D3DPoolsGetTextureMemoryUsage
// description:	Return the amount of memory used by texture pools
uint32 D3DPoolsGetTextureMemoryUsage()
{
	uint32 mem = 0;
	int32 index = gNumTexturePoolsUsed;
	
	while(index--)
	{
		mem += gTexturePoolArray[index].GetMemoryUsed();
	}
	return mem;
}

extern "C" {

//
// name:		D3DPoolsCreateIndexBuffer
// description:	Either return an index buffer from one of the index buffer pools
//				or use D3D calls to create a new texture
HRESULT D3DPoolsCreateIndexBuffer(int32 size, D3DFORMAT format, IDirect3DIndexBuffer_DXVER** ppIndexBuffer)
{
#ifndef FINALBUILD
	if(size == 0)
		gbIndexBufferError = true;
#endif			
	int32 index = ((size-1) / INDEXBUFFER_SIZE);
	size = (index+1) * INDEXBUFFER_SIZE;
		
	if(index < MAX_NUM_INDEXBUFFERS_POOLS)
	{
		*ppIndexBuffer = gIndexBufferPoolArray[index].RequestIndexBuffer();


		if(*ppIndexBuffer)
		{
			D3DINDEXBUFFER_DESC bufferDesc;
			(*ppIndexBuffer)->GetDesc(&bufferDesc);
			
			ASSERT((bufferDesc.Size/2) >= size);
			
			return D3D_OK;
		}	
	}
	else
	{
		*ppIndexBuffer = gAwkwardIndexBufferPool.RequestIndexBuffer(size);
		if(*ppIndexBuffer)
			return D3D_OK;
	}
		
	return DXCHECK(_RwD3DDevice->CreateIndexBuffer(size*sizeof(RwUInt16), 
													D3DUSAGE_WRITEONLY,
													D3DFMT_INDEX16,
                                                    D3DPOOL_MANAGED,
                                                    ppIndexBuffer,
                                                    NULL));
}

//
// name:		D3DPoolsReleaseIndexBuffer
// description:	Either release an index buffer resource or store it in one of
//				the index buffer pools
void D3DPoolsReleaseIndexBuffer(IDirect3DIndexBuffer_DXVER* pIndexBuffer)
{
	D3DINDEXBUFFER_DESC bufferDesc;
	int32 index;
		
	if(gEnableTexturePools)
	{
		pIndexBuffer->GetDesc(&bufferDesc);
		ASSERT(bufferDesc.Format == D3DFMT_INDEX16);

		index = ((bufferDesc.Size/sizeof(RwUInt16)) - 1) / INDEXBUFFER_SIZE;
		if(index < MAX_NUM_INDEXBUFFERS_POOLS)
		{
			CIndexBufferPool* pBufferPool = &gIndexBufferPoolArray[index];
			// if failed to add texture to the pool then release texture properly
			if(!pBufferPool->AddIndexBuffer(pIndexBuffer))
			{
//					if(pBufferPool->GetRealSize() > (pBufferPool->GetSize() * 3) / 2)
				{
					bool bAdded;
					pBufferPool->Resize(pBufferPool->GetSize()*2);
					bAdded = pBufferPool->RetryAddIndexBuffer(pIndexBuffer);
					ASSERT(bAdded);
					return;
				}
//					pIndexBuffer->Release();
			}	
			return;
		}
		// add to awkward index buffer pool	
		else if(!gAwkwardIndexBufferPool.AddIndexBuffer(pIndexBuffer))
		{
			bool bAdded;
			gAwkwardIndexBufferPool.Resize(gAwkwardIndexBufferPool.GetSize()*2);
			bAdded = gAwkwardIndexBufferPool.RetryAddIndexBuffer(pIndexBuffer);
			ASSERT(bAdded);
		}	
		return;
	}	
	pIndexBuffer->Release();
}

} // extern "C"

//
// name:		D3DPoolsReleaseIndexBuffers
// description:	Release a set number of index buffers. This is normally done when the screen is faded
//				out as it takes a while to do
void D3DPoolsReleaseIndexBuffers(uint32 num)
{
	static int32 index = 0;
	int32 lastDeleted = 0;

	// remove all the awkward size index buffers first
	while(num)
	{
		IDirect3DIndexBuffer_DXVER* pIndexBuffer = gAwkwardIndexBufferPool.RequestIndexBuffer();
		if(pIndexBuffer)
		{
			pIndexBuffer->Release();
			num--;
		}	
		else
			break;	
	}

	while(num && lastDeleted < MAX_NUM_INDEXBUFFERS_POOLS)
	{
		IDirect3DIndexBuffer_DXVER* pIndexBuffer = gIndexBufferPoolArray[index].RequestIndexBuffer();
		if(pIndexBuffer)
		{
			pIndexBuffer->Release();
			lastDeleted = 0;
			num--;
		}
		else
		{
			lastDeleted++;
		}
		index++;
		index %= MAX_NUM_INDEXBUFFERS_POOLS;
	}

}

//
// name:		D3DPoolsGetIndexBufferMemoryUsage
// description:	Return the amount of memory used by Index Buffers
uint32 D3DPoolsGetIndexBufferMemoryUsage()
{
	uint32 mem = 0;
	int32 index = MAX_NUM_INDEXBUFFERS_POOLS;
	
	while(index--)
	{
		mem += gIndexBufferPoolArray[index].GetMemoryUsed();
	}
	mem += gAwkwardIndexBufferPool.GetMemoryUsed();
	return mem;
}

/*
//
// name:		PalettePoolGetId
// description:	Return an available palette id
int32 PalettePoolGetId(void)
{
	int32 id = gPalettePool.RequestPalette();
	if(id == -1)
		return gNextPaletteAvailable++;
	return id;	
}

//
// name:		PalettePoolReleaseId
// description:	Add a palette id into the pool
void PalettePoolReleaseId(int32 id)
{
	if(gEnableTexturePools)
		gPalettePool.AddPalette(id);
}
*/
//
// name:		D3DPoolsInit
// description:	Initialise d3d object pools. 
void D3DPoolsInit(void)
{
	CDebug::DebugLog("Initialise Texture pools\n");
	
	gNumTexturePoolsUsed = 1;
	gEnableTexturePools = true;

	// Add one texture pool for unknown format and size textures i.e. bloody awkward size textures
	gTexturePoolArray[0].Init(D3DFMT_UNKNOWN, 0, -1, 16);
	
	// initialise index buffer pools	
	for(int32 i=0; i<MAX_NUM_INDEXBUFFERS_POOLS; i++)
		gIndexBufferPoolArray[i].Init(D3DFMT_INDEX16, (i+1)*INDEXBUFFER_SIZE, 16);
	
	gAwkwardIndexBufferPool.Init(D3DFMT_INDEX16, 0, 16);
//	gPalettePool.Init(100);
//	gNextPaletteAvailable = 0;
}

//
// name:		D3DPoolsShutdown
// description:	Shutdown d3d object pools. Delete all the d3d objects connected with them
void D3DPoolsShutdown(void)
{
	int32 i;
	
	CDebug::DebugLog("Shutdown Texture pools\n");

	// destroy texture pools
	for(i=0; i<gNumTexturePoolsUsed; i++)
		gTexturePoolArray[i].Destroy();
	gNumTexturePoolsUsed = 0;	
		
	// destroy index buffer pools	
	for(int32 i=0; i<MAX_NUM_INDEXBUFFERS_POOLS; i++)
		gIndexBufferPoolArray[i].Destroy();
	gAwkwardIndexBufferPool.Destroy();
	
	gEnableTexturePools = false;
	
//	gPalettePool.Destroy();
}

//
// name:		D3DPoolsShutdownForRestart
// description:	Shutdown d3d object pools. Delete all the d3d objects connected with them
void D3DPoolsShutdownForRestart(void)
{
	int32 i;
	
	for(i=0; i<gNumTexturePoolsUsed; i++)
		gTexturePoolArray[i].Empty();
	gNumTexturePoolsUsed = 0;
	
	// destroy index buffer pools	
	for(int32 i=0; i<MAX_NUM_INDEXBUFFERS_POOLS; i++)
		gIndexBufferPoolArray[i].Empty();
	gAwkwardIndexBufferPool.Empty();
}

//
// name:		D3DPoolsEnable
// description:	Enable/Disable d3d object pools
void D3DPoolsEnable(bool enable)
{
	gEnableTexturePools = enable;
}


#endif//GTA_PC...


