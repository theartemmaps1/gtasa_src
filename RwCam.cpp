//$DW$#include <rwcore.h>

#include "skeleton.h"
#include "RwCam.h"

#if defined (GTA_PC) || defined (GTA_PS2)
void CameraSize(RwCamera *camera, RwRect *rect, RwReal viewWindow, RwReal aspectRatio)
{
    if (camera)
    {
    	RwRect origSize;
        RwV2d vw;
        RwVideoMode videoMode;
        RwRect r;

        RwEngineGetVideoModeInfo(&videoMode, RwEngineGetCurrentVideoMode());

        origSize.w = RwRasterGetWidth(RwCameraGetRaster(camera));
        origSize.h = RwRasterGetHeight(RwCameraGetRaster(camera));

        if (!rect)
        {
            /* rect not specified - derive for full screen device or
             * simply reuse current values
             */
            if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
            {
                r.w = videoMode.width;
                r.h = videoMode.height;
            }
            else
            {
                r.w = RwRasterGetWidth(RwCameraGetRaster(camera));
                r.h = RwRasterGetHeight(RwCameraGetRaster(camera));
            }
            r.x = r.y = 0;
            rect = &r;
        }

		if(origSize.w != rect->w || origSize.h != rect->h)
		{
			RwRaster* raster;
			RwRaster* zRaster;
	        /*
	         * Destroy old rasters...
	         */
#ifndef GTA_PC 	         
	        raster = RwCameraGetRaster(camera);
	        if( raster )
	        {
	            RwRasterDestroy(raster);
	            raster = NULL;
	        }

	        zRaster = RwCameraGetZRaster(camera);
	        if( zRaster )
	        {
	            RwRasterDestroy(zRaster);
	            zRaster = NULL;
	        }

	        //
	        // Create new rasters... 
	        //
	        raster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERATEXTURE);
	        zRaster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

	        if( raster && zRaster )
	        {
	            RwCameraSetRaster(camera, raster);
	            RwCameraSetZRaster(camera, zRaster);
	        }
	        else
	        {
	        	// if failed then recreate rasters at original size
		        if( raster )
		        {
		            RwRasterDestroy(raster);
		            raster = NULL;
		        }
		        if( zRaster )
		        {
		            RwRasterDestroy(zRaster);
		            zRaster = NULL;
		        }
		        *rect = origSize;

		        //
		        // Create rasters from original size
		        //
		        raster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPECAMERA);
		        zRaster = RwRasterCreate(rect->w, rect->h, 0, rwRASTERTYPEZBUFFER);

	            RwCameraSetRaster(camera, raster);
	            RwCameraSetZRaster(camera, zRaster);
	        }
#else
		    raster = RwCameraGetRaster(camera);
	        zRaster = RwCameraGetZRaster(camera);
	        
	        raster->width  = zRaster->width = rect->w;
	        raster->height = zRaster->height = rect->h;
#endif	        
		}

#ifdef GTA_PS2	// DW - to enable widescreen this was required... since bits of the RW renderer have been replaced as far as video mode settings are concerned
            /* derive ratio from aspect ratio */
            vw.x = viewWindow;
            vw.y = viewWindow/aspectRatio;
#else            
		//
		// set view window
		//
        if (videoMode.flags & rwVIDEOMODEEXCLUSIVE)
        {
            /* derive ratio from aspect ratio */
            vw.x = viewWindow;
            vw.y = viewWindow/aspectRatio;
        }
        else
        {
            /* derive from pixel ratios */

            if (rect->w > rect->h)
            {
                vw.x = viewWindow;
                vw.y = (rect->h * viewWindow) / rect->w;
            }
            else
            {
                vw.x = (rect->w * viewWindow) / rect->h;
                vw.y = viewWindow;
            }
        }
#endif

        RwCameraSetViewWindow(camera, &vw);

		RsGlobal.screenWidth = rect->w;
		RsGlobal.screenHeight = rect->h;

    }    
}

void
CameraDestroy(RwCamera *camera)
{
    RwRaster *raster, *tmpRaster;
	RwFrame* frame = NULL;

    if (camera) 
    {
		frame = RwCameraGetFrame(camera);
        if (frame)
        {
			RwCameraSetFrame(camera, NULL);
            RwFrameDestroy(frame);
        }

        raster = RwCameraGetRaster(camera);
        if (raster)
        {
            tmpRaster = RwRasterGetParent(raster);
            
            RwRasterDestroy(raster);

            if ((tmpRaster != NULL) && (tmpRaster != raster))
            {
                RwRasterDestroy(tmpRaster);
            }
        }

        raster = RwCameraGetZRaster(camera);
        if (raster)
        {
            tmpRaster = RwRasterGetParent(raster);

            RwRasterDestroy(raster);

            if ((tmpRaster != NULL) && (tmpRaster != raster))
            {
                RwRasterDestroy(tmpRaster);
            }
        }
        RwCameraDestroy(camera);
    }
}	
 
#endif //GTA_PS2 & GTA_PC
/*
void
CameraMove(RwCamera *cam, RwV3d *v)
{
    RwV3d offset;

    RwMatrix *cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(cam));

    RwV3d *at = RwMatrixGetAt(cameraMatrix);
    RwV3d *up = RwMatrixGetUp(cameraMatrix);
    RwV3d *right = RwMatrixGetRight(cameraMatrix);

    offset.x = v->x * right->x + v->y * up->x + v->z * at->x;
    offset.y = v->x * right->y + v->y * up->y + v->z * at->y;
    offset.z = v->x * right->z + v->y * up->z + v->z * at->z;

    // Translate the camera back to its new position in the world
    RwFrameTranslate(RwCameraGetFrame(cam), &offset, rwCOMBINEPOSTCONCAT);
}
*/

/*
void
CameraPan(RwCamera *cam, const RwV3d *pos, RwReal angle)
{
    RwV3d invCamPos;
    RwFrame *cameraFrame = RwCameraGetFrame(cam);
    RwMatrix *cameraMatrix = RwFrameGetMatrix(cameraFrame);
    RwV3d camPos;

    camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

    RwV3dScale(&invCamPos, &camPos, -1.0f); 

    // Translate the camera back to the rotation origin.
    RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

    // Get the cameras Up vector and use this as the axis of rotation

    RwMatrixRotate(cameraMatrix, RwMatrixGetUp(cameraMatrix), 
                   angle, rwCOMBINEPOSTCONCAT);

    // Translate the camera back to its original position
    RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);
}
*/

/*
void
CameraTilt(RwCamera *cam, const RwV3d *pos, RwReal angle)
{
    RwV3d invCamPos;
    RwFrame *cameraFrame = RwCameraGetFrame(cam);
    RwMatrix *cameraMatrix = RwFrameGetMatrix(cameraFrame);
    RwV3d camPos;

    camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

    RwV3dScale(&invCamPos, &camPos, -1.0f); 

    // Translate the camera back to the rotation origin.
    RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

    // Get the cameras Right vector and use this as the axis of rotation

    RwMatrixRotate(cameraMatrix, RwMatrixGetRight(cameraMatrix), 
                   angle, rwCOMBINEPOSTCONCAT);

    // Translate the camera back to its original position
    RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);
}
*/

/*
void
CameraRotate(RwCamera *cam, const RwV3d *pos, RwReal angle)
{
    RwV3d invCamPos;
    RwFrame *cameraFrame = RwCameraGetFrame(cam);
    RwMatrix *cameraMatrix = RwFrameGetMatrix(cameraFrame);
    RwV3d camPos;

    camPos = (pos) ? *pos : *RwMatrixGetPos(cameraMatrix);

    RwV3dScale(&invCamPos, &camPos, -1.0f); 

    // Translate the camera back to the rotation origin.
    RwFrameTranslate(cameraFrame, &invCamPos, rwCOMBINEPOSTCONCAT);

    // Get the cameras At vector and use this as the axis of rotation

    RwMatrixRotate(cameraMatrix, RwMatrixGetAt(cameraMatrix), 
                   angle, rwCOMBINEPOSTCONCAT);

    // Translate the camera back to its original position
    RwFrameTranslate(cameraFrame, &camPos, rwCOMBINEPOSTCONCAT);
}
*/






