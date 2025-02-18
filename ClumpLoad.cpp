//
// name:		ClumpLoad.cpp
// description:	Custum clump loaders
// written by:	Adam Fowler
//
#include "Debug.h"
#include "MemoryMgr.h"
#include "ClumpLoad.h"
#include "string.h"

// loading structures
// Atomic
typedef struct rpAtomicBinary304 _rpAtomicBinary304;
struct rpAtomicBinary304
{
    RwInt32             frameIndex;
    RwInt32             geomIndex;
    RwInt32             flags;
    RwInt32             unused;
};
// Geometry list
typedef struct rpGeometryList rpGeometryList;
struct rpGeometryList
{
    RpGeometry        **geometries;
    RwInt32             numGeoms;
};



bool RpClumpStreamReadAtomics(RwStream* pStream, LoadAtomicCallback atomCB)
{
	
	
	return true;
}


static rpGeometryList* GeometryListStreamRead(RwStream * pStream, rpGeometryList * geomList)
{
    RwInt32             gl;
    RwInt32             i;
    RwUInt32            size;
    RwUInt32            version;

    if (!RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version))
    {
        return((rpGeometryList *)NULL);
    }

    {
        /* Read it */
        if (RwStreamRead(pStream, &gl, sizeof(gl)) != sizeof(gl))
        {
            return((rpGeometryList *)NULL);
        }

        /* Convert it */
        //RwMemNative(&gl, sizeof(gl));

        /* Set up the geometry list */
        geomList->numGeoms = gl;

        if (geomList->numGeoms > 0)
        {
            geomList->geometries = (RpGeometry **) GtaMalloc(sizeof(RpGeometry *) * gl);
            if (!geomList->geometries)
            {
                return((rpGeometryList *)NULL);
            }
        }
        else
        {
            geomList->geometries = (RpGeometry **)NULL;
        }

        for (i = 0; i < gl; i++)
        {
            /* Read the geometry */
            if (!RwStreamFindChunk(pStream, rwID_GEOMETRY, (RwUInt32 *)NULL, &version))
            {
                return((rpGeometryList *)NULL);
            }

            {
                if (!(geomList->geometries[i] = RpGeometryStreamRead(pStream)))
                {
                    return((rpGeometryList *)NULL);
                }
            }
        }
    }

    return(geomList);
}


static int numberGeometrys=-1;
static int streamPosition;
static rpGeometryList* GeometryListStreamRead1(RwStream * pStream, rpGeometryList * geomList)
{
    RwInt32             gl;
    RwInt32             i;
    RwUInt32            size;
    RwUInt32            version;

	ASSERT(geomList->numGeoms == 0);
	numberGeometrys = 0;
    if (!RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version))
    {
        return((rpGeometryList *)NULL);
    }

    {
        /* Read it */
        if (RwStreamRead(pStream, &gl, sizeof(gl)) != sizeof(gl))
        {
            return((rpGeometryList *)NULL);
        }

        /* Convert it */
        //RwMemNative(&gl, sizeof(gl));

		numberGeometrys = gl/2;
        /* Set up the geometry list */
        geomList->numGeoms = gl;

        if (geomList->numGeoms > 0)
        {
            geomList->geometries = (RpGeometry **) GtaMalloc(sizeof(RpGeometry *) * gl);
            if (!geomList->geometries)
            {
                return((rpGeometryList *)NULL);
            }
            memset(geomList->geometries, 0, sizeof(RpGeometry *) * gl);
        }
        else
        {
            geomList->geometries = (RpGeometry **)NULL;
        }

        for (i = 0; i < numberGeometrys; i++)
        {
            /* Read the geometry */
            if (!RwStreamFindChunk(pStream, rwID_GEOMETRY, (RwUInt32 *)NULL, &version))
            {
                return((rpGeometryList *)NULL);
            }

            {
                if (!(geomList->geometries[i] = RpGeometryStreamRead(pStream)))
                {
                    return((rpGeometryList *)NULL);
                }
            }
        }
    }

    return(geomList);
}


static rpGeometryList* GeometryListStreamRead2(RwStream * pStream, rpGeometryList * geomList)
{
    RwInt32             i;
    RwUInt32            version;

	ASSERT(geomList->numGeoms > 0);

	for (i = numberGeometrys; i < geomList->numGeoms; i++)
	{
	  /* Read the geometry */
	  if (!RwStreamFindChunk(pStream, rwID_GEOMETRY, (RwUInt32 *)NULL, &version))
	  {
	      return((rpGeometryList *)NULL);
	  }

	  {
	      if (!(geomList->geometries[i] = RpGeometryStreamRead(pStream)))
	      {
	          return((rpGeometryList *)NULL);
	      }
	  }
	}

    return(geomList);
}


static RpAtomic* ClumpAtomicGtaStreamRead(RwStream * pStream, rwFrameList * fl, rpGeometryList * gl)
{
    RwBool              status;
    RwUInt32            size;
    RwUInt32            version;
    RpAtomic           *pAtomic;

    status = RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version);

    if (!status)
    {
        return((RpAtomic *)NULL);
    }

    {
        _rpAtomicBinary304  a304;
        RpGeometry         *geom;

        /* Read the atomic */
        //memset(&a304, 0, sizeof(a304));

        status = (size == RwStreamRead(pStream, &a304, size));
        if (!status)
        {
            return((RpAtomic *)NULL);
        }
        //RwMemNative(&a304, sizeof(a304));

        pAtomic = RpAtomicCreate();
        if (!pAtomic)
        {
            return((RpAtomic *)NULL);
        }

        /* Set the atomic types */
        RpAtomicSetFlags(pAtomic, a304.flags);
        if (fl->numFrames)
        {
            RpAtomicSetFrame(pAtomic, fl->frames[a304.frameIndex]);
        }

        /* get the geometry */
        if (gl->numGeoms)
        {
            RpAtomicSetGeometry(pAtomic, gl->geometries[a304.geomIndex], 0);
        }
        else
        {
            status = RwStreamFindChunk(pStream, rwID_GEOMETRY,
                                       (RwUInt32 *)NULL, &version);
            if (!status)
            {
                RpAtomicDestroy(pAtomic);
                return((RpAtomic *)NULL);
            }

            {
                geom = RpGeometryStreamRead(pStream);
                status = (NULL != geom);

                if (!status)
                {
                    RpAtomicDestroy(pAtomic);
                    return((RpAtomic *)NULL);
                }
            }

            RpAtomicSetGeometry(pAtomic, geom, 0);

            /* Bring the geometry reference count back down, so that
             * when the atomic is destroyed, so is the geometry.
             */
            RpGeometryDestroy(geom);
        }

#if 0
        /* Atomic extension data */
        status = (NULL != rwPluginRegistryReadDataChunks(&atomicTKList, pStream, pAtomic));

        if (!status)
        {
            return((RpAtomic *)NULL);
        }
#endif

        return(pAtomic);
    }
}

static rpGeometryList     *
GeometryListDeinitialize(rpGeometryList * geomList)
{
    if (geomList->numGeoms)
    {
        RwInt32             i;

        /* remove the read reference to each geometry */
        for (i = 0; i < geomList->numGeoms; i++)
        {
        	if(geomList->geometries[i])
	            RpGeometryDestroy(geomList->geometries[i]);
        }

        GtaFree(geomList->geometries);
    }

	geomList->numGeoms = 0;
	
    return(geomList);
}


RpClump* RpClumpGtaStreamRead(RwStream * pStream)
{
    RwBool              status;
    RwUInt32            size;
    RwUInt32            version;

    status = RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version);

    if (!status)
    {
        return((RpClump *)NULL);
    }

    {
        RpClump            *pClump;
        RpAtomic           *pAtomic;
        RpClumpChunkInfo     cl;
        rwFrameList         fl;
        rpGeometryList      gl;
        RwInt32             i;
        RwUInt32            chunkversion;

        status = (sizeof(cl) == RwStreamRead(pStream, &cl, sizeof(cl)) );

        if (!status)
        {
            return((RpClump *)NULL);
        }
        //RwMemNative(&cl, sizeof(cl));

        pClump = RpClumpCreate();
        if (!pClump)
        {
            return((RpClump *)NULL);
        }

        /* Read the frame list */
        status = RwStreamFindChunk(pStream, (RwUInt32)rwID_FRAMELIST,
                                   (RwUInt32 *)NULL, &chunkversion);

        if (!status)
        {
            return((RpClump *)NULL);
        }

        {
            status = (NULL != rwFrameListStreamRead(pStream, &fl));

            if (!status)
            {
                RpClumpDestroy(pClump);
                return((RpClump *)NULL);
            }
        }

        /* Read the geometry list */
        status = RwStreamFindChunk (pStream, (RwUInt32)rwID_GEOMETRYLIST,
                                    (RwUInt32 *)NULL, &chunkversion);

        if (!status)
        {
			rwFrameListDeinitialize(&fl);
			RpClumpDestroy(pClump);
            return((RpClump *)NULL);
        }

        {
            status = (NULL != GeometryListStreamRead(pStream, &gl));

            if (!status)
            {
                rwFrameListDeinitialize(&fl);
                RpClumpDestroy(pClump);
                return((RpClump *)NULL);
            }
        }

        /* Set the frame root */
        RpClumpSetFrame(pClump, fl.frames[0]);

        /* Iterate over the atomics */
        for (i = 0; i < cl.numAtomics; i++)
        {
            status = RwStreamFindChunk(pStream, (RwUInt32)rwID_ATOMIC,
                                       (RwUInt32 *)NULL, &version);

            if (!status)
            {
                GeometryListDeinitialize(&gl);
                rwFrameListDeinitialize(&fl);
                RpClumpDestroy(pClump);
                return((RpClump *)NULL);
            }

            {
                pAtomic = ClumpAtomicGtaStreamRead(pStream, &fl, &gl);
                status = (NULL != pAtomic);

                if (!status)
                {
                    GeometryListDeinitialize(&gl);
                    rwFrameListDeinitialize(&fl);
                    RpClumpDestroy(pClump);
                    return((RpClump *)NULL);
                }
            }

            /* Add the atomic to the clump */
            RpClumpAddAtomic(pClump, pAtomic);
        }

        /* Dont need the geometry list anymore */
        GeometryListDeinitialize(&gl);

        /* Dont need the frame list anymore */
        rwFrameListDeinitialize(&fl);

#if 0
        /* Clump extension data */
        status = (NULL != rwPluginRegistryReadDataChunks(&clumpTKList, pStream, pClump));

        if (!status)
        {
            RpClumpDestroy(pClump);
            return((RpClump *)NULL);
        }
#endif
        return(pClump);
    }
}


static rwFrameList         gFrameList;
static rpGeometryList      gGeomList = {0, NULL};
static RpClumpChunkInfo    gClumpInfo;

bool RpClumpGtaStreamRead1(RwStream* pStream)
{
    RwBool              status;
    RwUInt32            size;
    RwUInt32            version;

    status = RwStreamFindChunk(pStream, rwID_STRUCT, &size, &version);

    if (!status)
    {
        return false;
    }

    {
        RwUInt32  			chunkversion;

        status = (sizeof(RpClumpChunkInfo) == RwStreamRead(pStream, &gClumpInfo, sizeof(RpClumpChunkInfo)) );
        if (!status)
        {
            return false;
        }
        //RwMemNative(&cl, sizeof(cl));

        /* Read the frame list */
        status = RwStreamFindChunk(pStream, (RwUInt32)rwID_FRAMELIST,
                                   (RwUInt32 *)NULL, &chunkversion);

        if (!status)
        {
            return false;
        }

        {
            status = (NULL != rwFrameListStreamRead(pStream, &gFrameList));

            if (!status)
            {
	            return false;
            }
        }

        /* Read the geometry list */
        status = RwStreamFindChunk (pStream, (RwUInt32)rwID_GEOMETRYLIST,
                                    (RwUInt32 *)NULL, &chunkversion);

        if (!status)
        {
			rwFrameListDeinitialize(&gFrameList);
            return false;
        }

        {
            status = (NULL != GeometryListStreamRead1(pStream, &gGeomList));

            if (!status)
            {
                rwFrameListDeinitialize(&gFrameList);
	            return false;
            }
        }

		streamPosition = pStream->Type.memory.position;

        return true;
    }
}

RpClump* RpClumpGtaStreamRead2(RwStream* pStream)
{
    RwBool              status;
    RwUInt32            version;
    int 				i;
    RpClump				*pClump;
    RpAtomic            *pAtomic;
	
	pClump = RpClumpCreate();
	if (!pClump)
	{
		return NULL;
   	}

	// skip to new position
	RwStreamSkip(pStream, streamPosition - pStream->Type.memory.position);

	{
		{
            status = (NULL != GeometryListStreamRead2(pStream, &gGeomList));

            if (!status)
            {
                GeometryListDeinitialize(&gGeomList);
                rwFrameListDeinitialize(&gFrameList);
                RpClumpDestroy(pClump);
                return((RpClump *)NULL);
            }
        }

        /* Set the frame root */
        RpClumpSetFrame(pClump, gFrameList.frames[0]);

        /* Iterate over the atomics */
        for (i = 0; i < gClumpInfo.numAtomics; i++)
        {
            status = RwStreamFindChunk(pStream, (RwUInt32)rwID_ATOMIC,
                                       (RwUInt32 *)NULL, &version);

            if (!status)
            {
                GeometryListDeinitialize(&gGeomList);
                rwFrameListDeinitialize(&gFrameList);
                RpClumpDestroy(pClump);
                return((RpClump *)NULL);
            }

            {
                pAtomic = ClumpAtomicGtaStreamRead(pStream, &gFrameList, &gGeomList);
                status = (NULL != pAtomic);

                if (!status)
                {
                    GeometryListDeinitialize(&gGeomList);
                    rwFrameListDeinitialize(&gFrameList);
                    RpClumpDestroy(pClump);
                    return((RpClump *)NULL);
                }
            }

            /* Add the atomic to the pClump */
            RpClumpAddAtomic(pClump, pAtomic);
        }

        /* Dont need the geometry list anymore */
        GeometryListDeinitialize(&gGeomList);

        /* Dont need the frame list anymore */
        rwFrameListDeinitialize(&gFrameList);

#if 0
        /* Clump extension data */
        status = (NULL != rwPluginRegistryReadDataChunks(&clumpTKList, pStream, pClump));

        if (!status)
        {
            RpClumpDestroy(pClump);
            return((RpClump *)NULL);
        }
#endif
        return(pClump);
    }
}


void RpClumpGtaCancelStream()
{
     GeometryListDeinitialize(&gGeomList);
     rwFrameListDeinitialize(&gFrameList);
     gFrameList.numFrames = 0;
}
