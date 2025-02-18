/**********************************************************************
 *
 * File : lights.c
 *
 * Abstract : Provides light functionality
 *
 **********************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd. or
 * Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. will not, under any
 * circumstances, be liable for any lost revenue or other damages arising
 * from the use of this file.
 *
 * Copyright (c) 1999 Criterion Software Ltd.
 * All Rights Reserved.
 *
 * RenderWare is a trademark of Canon Inc.
 *
 ************************************************************************/

//$DW$#include <rwcore.h>
//$DW$#include <rpworld.h>

#include "lights.h"
#include "timecycle.h"
#include "coronas.h"
#include "world.h"
#include "setup.h"
#include "weather.h"
#include "timer.h"
#include "frontend.h"
#include "zonecull.h"
#include "mblur.h"
#include "PostEffects.h"

RwBool  Lighting = FALSE;

#define OBJECTCARANDPEDLIGHTMULTIPLIER (1.3f)

/* we have two lights in the demo */
RpLight *pAmbient = NULL;
RpLight *pDirect = NULL;

// A array of directional lights to be used for pointlights.
RpLight *pExtraDirectionals[NUM_EXTRA_DIRECTIONALS] = { NULL, NULL, NULL, NULL, NULL, NULL };
int32 NumExtraDirLightsInWorld = 0;

RwRGBAReal	AmbientLightColourForFrame;
RwRGBAReal	AmbientLightColourForFrame_PedsCarsAndObjects;
RwRGBAReal	DirectionalLightColourForFrame;
static RwRGBAReal	AmbientLightColour;
static RwRGBAReal	DirectionalLightColour;
RwRGBAReal	FullLight = { 1.0f, 1.0f, 1.0f, 1.0 };

void SetLightsWithTimeOfDayColour(RpWorld *world)
{
//    RwRGBAReal	color;
//    RwFrame		*frame;
    RwMatrix	Matrix;
	CVector		OtherVec1, OtherVec2;

   	if (pAmbient)
    {
//    	if (CMBlur::BlurOn)
//    	{
//        	AmbientLightColourForFrame.red   = CTimeCycle::m_fCurrentAmbientRed_Bl * CCoronas::LightsMult;
//        	AmbientLightColourForFrame.green = CTimeCycle::m_fCurrentAmbientGreen_Bl * CCoronas::LightsMult;
//        	AmbientLightColourForFrame.blue  = CTimeCycle::m_fCurrentAmbientBlue_Bl * CCoronas::LightsMult;
//        }
//        else
    	{
        	AmbientLightColourForFrame.red   = CMaths::MultNorm(CTimeCycle::GetAmbientRed(), 	CCoronas::LightsMult);
        	AmbientLightColourForFrame.green = CMaths::MultNorm(CTimeCycle::GetAmbientGreen(),	CCoronas::LightsMult);
        	AmbientLightColourForFrame.blue  = CMaths::MultNorm(CTimeCycle::GetAmbientBlue(),	CCoronas::LightsMult);
        }
        
//    	if (CMBlur::BlurOn)
//    	{
//        	AmbientLightColourForFrame_PedsCarsAndObjects.red   = CTimeCycle::m_fCurrentAmbientRed_Obj_Bl * CCoronas::LightsMult;
//        	AmbientLightColourForFrame_PedsCarsAndObjects.green = CTimeCycle::m_fCurrentAmbientGreen_Obj_Bl * CCoronas::LightsMult;
//        	AmbientLightColourForFrame_PedsCarsAndObjects.blue  = CTimeCycle::m_fCurrentAmbientBlue_Obj_Bl * CCoronas::LightsMult;
//        }
//        else
    	{
        	AmbientLightColourForFrame_PedsCarsAndObjects.red   = CMaths::MultNorm(CTimeCycle::GetAmbientRed_Obj(),		CCoronas::LightsMult);
        	AmbientLightColourForFrame_PedsCarsAndObjects.green = CMaths::MultNorm(CTimeCycle::GetAmbientGreen_Obj(),	CCoronas::LightsMult);
        	AmbientLightColourForFrame_PedsCarsAndObjects.blue  = CMaths::MultNorm(CTimeCycle::GetAmbientBlue_Obj(),	CCoronas::LightsMult);
        }

   		if (CWeather::LightningFlash)
		{
			AmbientLightColourForFrame.red		= 
			AmbientLightColourForFrame.green 	=
			AmbientLightColourForFrame.blue		= 1.0f;
			AmbientLightColourForFrame_PedsCarsAndObjects.red	=
			AmbientLightColourForFrame_PedsCarsAndObjects.green =
			AmbientLightColourForFrame_PedsCarsAndObjects.blue	= 1.0f;
		}


        RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
	}
	
	
	if (pDirect)
    {
        DirectionalLightColourForFrame.red   = CMaths::MultNorm(CTimeCycle::GetDirectionalRed(),	CCoronas::LightsMult);
        DirectionalLightColourForFrame.green = CMaths::MultNorm(CTimeCycle::GetDirectionalGreen(),	CCoronas::LightsMult);
        DirectionalLightColourForFrame.blue  = CMaths::MultNorm(CTimeCycle::GetDirectionalBlue(),	CCoronas::LightsMult);

        RpLightSetColor(pDirect, &DirectionalLightColourForFrame);

		// Also set the direction of the light depending on the position of the sun
//		CVector vecToSun = CTimeCycle::m_VectorToSun[CTimeCycle::m_CurrentStoredValue];
		// TEST for Aaron to manually set the directional light
		CVector vecToSun = CTimeCycle::m_vecDirnLightToSun;
		
		OtherVec1 = CVector(0.0f, 0.0f, 1.0f);
		OtherVec2 = CrossProduct(OtherVec1, vecToSun);
		OtherVec2.Normalise();
		OtherVec1 = CrossProduct(OtherVec2, vecToSun);
		RwMatrixGetAt(&Matrix)->x = -vecToSun.x;
		RwMatrixGetAt(&Matrix)->y = -vecToSun.y;
		RwMatrixGetAt(&Matrix)->z = -vecToSun.z;
		RwMatrixGetRight(&Matrix)->x = OtherVec1.x;
		RwMatrixGetRight(&Matrix)->y = OtherVec1.y;
		RwMatrixGetRight(&Matrix)->z = OtherVec1.z;
		RwMatrixGetUp(&Matrix)->x = OtherVec2.x;
		RwMatrixGetUp(&Matrix)->y = OtherVec2.y;
		RwMatrixGetUp(&Matrix)->z = OtherVec2.z;
        RwFrameTransform(RpLightGetFrame(pDirect), &Matrix, rwCOMBINEREPLACE);
	}

	// If the user wants the brightness crancked up we do that here. (toning down happens with a filter)
/*	if (FrontEndMenuManager.m_PrefsBrightness > 256)
	{
		// If we want to make the brightness have less effect (only for making things extra bright) we
		// have to decrease AARONSCALEBRIGHTNESS
#define AARONSCALEBRIGHTNESS (0.6f)
	
		float Multiplier = (((FrontEndMenuManager.m_PrefsBrightness / 256.0f) - 1.0f) * 2.0f) * AARONSCALEBRIGHTNESS + 1.0f;
		float ExtraMultiplier = (((FrontEndMenuManager.m_PrefsBrightness / 256.0f) - 1.0f) * 3.0f) * AARONSCALEBRIGHTNESS + 1.0f;
	
		AmbientLightColourForFrame.red = MIN(1.0f, AmbientLightColourForFrame.red * ExtraMultiplier);
		AmbientLightColourForFrame.green = MIN(1.0f, AmbientLightColourForFrame.green * ExtraMultiplier);
		AmbientLightColourForFrame.blue = MIN(1.0f, AmbientLightColourForFrame.blue * ExtraMultiplier);
		AmbientLightColourForFrame_PedsCarsAndObjects.red = MIN(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.red * Multiplier);
		AmbientLightColourForFrame_PedsCarsAndObjects.green = MIN(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.green * Multiplier);
		AmbientLightColourForFrame_PedsCarsAndObjects.blue = MIN(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.blue * Multiplier);
		DirectionalLightColourForFrame.red = MIN(1.0f, AmbientLightColourForFrame.red * Multiplier);
		DirectionalLightColourForFrame.green = MIN(1.0f, AmbientLightColourForFrame.green * Multiplier);
		DirectionalLightColourForFrame.blue = MIN(1.0f, AmbientLightColourForFrame.blue * Multiplier);
	}*/
}




void
LightsEnable(RwBool enable)
{
}

RpWorld *LightsDestroy(RpWorld *world)
{
	Int16	ExtraDirNum;
	
 	if (world)
 	{
    	if (pAmbient)
	    {
    	    RpWorldRemoveLight(world, pAmbient);
        	RpLightDestroy(pAmbient);
        	pAmbient = NULL;
    	}

    	if (pDirect)
	    {
	        RpWorldRemoveLight(world, pDirect);
	        RwFrameDestroy(RpLightGetFrame(pDirect));
	        RpLightDestroy(pDirect);
	        pDirect = NULL;
	    }

		for (ExtraDirNum = 0; ExtraDirNum < NUM_EXTRA_DIRECTIONALS; ExtraDirNum++)
		{
    		if (pExtraDirectionals[ExtraDirNum])
	    	{
	        	RpWorldRemoveLight(world, pExtraDirectionals[ExtraDirNum]);
	        	RwFrameDestroy(RpLightGetFrame(pExtraDirectionals[ExtraDirNum]));
	        	RpLightDestroy(pExtraDirectionals[ExtraDirNum]);
	        	pExtraDirectionals[ExtraDirNum] = NULL;
	        }
	    }
	}
    return world;
}


void WorldReplaceNormalLightsWithScorched(RpWorld *world, float Darkness)
{
	RwRGBAReal color;
	
    color.red   = Darkness;
    color.green = Darkness;
    color.blue  = Darkness;
    RpLightSetColor(pAmbient, &color);
    RpLightSetFlags(pDirect, 0);
}

void WorldReplaceScorchedLightsWithNormal(RpWorld *world)
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
    RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ModifyAmbientLightColourForArea
// PURPOSE :  Some areas have a certain colour to them like the red light district.
//			  The idea is to achieve this effect by modifying the ambient light colour.
/////////////////////////////////////////////////////////////////////////////////
/*
void ModifyAmbientLightColourForArea(CVector *pCoors, RpWorld *world)
{
	float	Distance, Factor;

    AmbientLightColour.red   = CTimeCycle::m_fCurrentAmbientRed * CCoronas::LightsMult;
    AmbientLightColour.green = CTimeCycle::m_fCurrentAmbientGreen * CCoronas::LightsMult;
    AmbientLightColour.blue  = CTimeCycle::m_fCurrentAmbientBlue * CCoronas::LightsMult;

#define LIGHTRADIUS (200.0f)

	if ( (Distance = (pCoors->x - 1050.0f)*(pCoors->x - 1050.0f) + (pCoors->y + 824.0f)*(pCoors->y + 824.0f)) < LIGHTRADIUS * LIGHTRADIUS)
	{		// Fuck the ambient colours here
		Distance = CMaths::Sqrt(Distance);
		Factor = 0.75f * (LIGHTRADIUS - Distance) / LIGHTRADIUS;
		AmbientLightColour.red = (AmbientLightColour.red * (1.0f - Factor) + Factor);
	}
	else
	{		// Restore normal ambient colours
	
	}
	
   	if (CWeather::LightningFlash)
	{
		AmbientLightColour.red = AmbientLightColour.green = AmbientLightColour.blue = 1.0f;
	}
	RpLightSetColor(pAmbient, &AmbientLightColour);
}
*/


float	LightStrengths[NUM_EXTRA_DIRECTIONALS];

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : AddAnExtraDirectionalLight
// PURPOSE :  Sets an extra directional light and adds it to the world.
/////////////////////////////////////////////////////////////////////////////////

void AddAnExtraDirectionalLight(RpWorld *world, float DirX, float DirY, float DirZ, float R, float G, float B)
{
    RwRGBAReal 	color;
	CVector 	OtherVec1, OtherVec2;
	RwFrame*	pFrame;
	RwMatrix* 	pMatrix;
	float		LowestStrength, OurStrength = CMaths::Max(CMaths::Max(R, G), B);
	Int32		SlotToBeAdded = -1;
	Int32		C;
	Int32		NumExtraDirectionalsWeCanHave = NUM_EXTRA_DIRECTIONALS_IN_WORLD;
		
	
	if (CGame::currArea != AREA_MAIN_MAP)
	{
		NumExtraDirectionalsWeCanHave = NUM_EXTRA_DIRECTIONALS;
	}
	
	if (NumExtraDirLightsInWorld < NumExtraDirectionalsWeCanHave)
	{
		SlotToBeAdded = NumExtraDirLightsInWorld;
	}
	else
	{	// Check if the current strength is higher than any of the lights already stored.
		LowestStrength = OurStrength;
		for (C = 0; C < NumExtraDirectionalsWeCanHave; C++)
		{
			if (LightStrengths[C] < LowestStrength)
			{
				SlotToBeAdded = C;
				LowestStrength = LightStrengths[C];
			}
		}
	}

	if (SlotToBeAdded >= 0)
	{
		// Set the colour of this light
        color.red   = R;
        color.green = G;
        color.blue  = B;
        RpLightSetColor(pExtraDirectionals[SlotToBeAdded], &color);

		// I'm sure this bit could be optimised. We don't actually need the
		// full 3 vectors in the matrix surely ?
		// AF: Yup that's right. We only need the at vector
		pFrame = RpLightGetFrame(pExtraDirectionals[SlotToBeAdded]);
		pMatrix = RwFrameGetMatrix(pFrame);
		
		RwMatrixGetAt(pMatrix)->x = -DirX;
		RwMatrixGetAt(pMatrix)->y = -DirY;
		RwMatrixGetAt(pMatrix)->z = -DirZ;

		RwMatrixUpdate(pMatrix);
		RwFrameUpdateObjects(pFrame);
		
		// Switch this light on
        RpLightSetFlags(pExtraDirectionals[SlotToBeAdded], rpLIGHTLIGHTATOMICS);
	
		LightStrengths[SlotToBeAdded] = OurStrength;
	
		NumExtraDirLightsInWorld = MIN(NumExtraDirLightsInWorld+1, NumExtraDirectionalsWeCanHave);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : RemoveExtraDirectionalLights
// PURPOSE :  Removes the extra directional lights from the world.
/////////////////////////////////////////////////////////////////////////////////

void RemoveExtraDirectionalLights(RpWorld *world)
{
	Int32	C;
	
	for (C = 0; C < NUM_EXTRA_DIRECTIONALS; C++)
	{
        RpLightSetFlags(pExtraDirectionals[C], 0);
	}
	NumExtraDirLightsInWorld = 0;
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : RandomiseLights
// PURPOSE :  Randomises lights so that peds look slightly different.
/////////////////////////////////////////////////////////////////////////////////
/*
void RandomiseLights(short RandomSeed)
{
    RwRGBAReal	ColourAmbient, ColourDirectional;
    float		RedMult, GreenMult, BlueMult;
    
    ColourAmbient = AmbientLightColour;
    ColourDirectional = DirectionalLightColour;

	RedMult   = 0.55f + (RandomSeed & 3) * 0.15f;
	GreenMult = 0.55f + ((RandomSeed>>2) & 3) * 0.15f;
	BlueMult  = 0.55f + ((RandomSeed>>4) & 3) * 0.15f;

	ColourAmbient.red *= RedMult;
	ColourAmbient.green *= GreenMult;
	ColourAmbient.blue *= BlueMult;
	ColourDirectional.red *= RedMult;
	ColourDirectional.green *= GreenMult;
	ColourDirectional.blue *= BlueMult;

	RpLightSetColor(pAmbient, &ColourAmbient);
    RpLightSetColor(pDirect, &ColourDirectional);

}
*/

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : DeRandomiseLights
// PURPOSE :  Fixes the random lights.
/////////////////////////////////////////////////////////////////////////////////
/*
void DeRandomiseLights()
{
	RpLightSetColor(pAmbient, &AmbientLightColour);
    RpLightSetColor(pDirect, &DirectionalLightColour);
}
*/

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAmbientAndDirectionalColours
// PURPOSE :  Scales the ambient and directional with the multiplier.
/////////////////////////////////////////////////////////////////////////////////

void SetAmbientAndDirectionalColours(float Mult)
{
	AmbientLightColour.red		= CMaths::MultNorm(AmbientLightColourForFrame.red,		Mult);
	AmbientLightColour.green 	= CMaths::MultNorm(AmbientLightColourForFrame.green,	Mult);
	AmbientLightColour.blue 	= CMaths::MultNorm(AmbientLightColourForFrame.blue,		Mult);
	DirectionalLightColour.red 	= CMaths::MultNorm(DirectionalLightColourForFrame.red,	Mult);
	DirectionalLightColour.green= CMaths::MultNorm(DirectionalLightColourForFrame.green,Mult);
	DirectionalLightColour.blue = CMaths::MultNorm(DirectionalLightColourForFrame.blue,	Mult);

    RpLightSetColor(pAmbient, &AmbientLightColour);
    RpLightSetColor(pDirect, &DirectionalLightColour);
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetFlashyColours
// PURPOSE :  Fixes the random lights.
/////////////////////////////////////////////////////////////////////////////////

void SetFlashyColours(float Mult)
{
	if (CTimer::GetTimeInMilliseconds() & 256)
	{
		AmbientLightColour.red 		= 1.0f;
		AmbientLightColour.green 	= 1.0f;
		AmbientLightColour.blue 	= 1.0f;;
		DirectionalLightColour.red 	= DirectionalLightColourForFrame.red;
		DirectionalLightColour.green= DirectionalLightColourForFrame.green;
		DirectionalLightColour.blue = DirectionalLightColourForFrame.blue;

	    RpLightSetColor(pAmbient, &AmbientLightColour);
	    RpLightSetColor(pDirect, &DirectionalLightColour);
	}
	else
	{
			// Use the standard light toned down a bit
		SetAmbientAndDirectionalColours(Mult * 0.75f);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetFlashyColours_Mild
// PURPOSE :  Fixes the random lights.
/////////////////////////////////////////////////////////////////////////////////

void SetFlashyColours_Mild(float Mult)
{
	if (CTimer::GetTimeInMilliseconds() & 256)
	{
		AmbientLightColour.red 		= 0.65f;
		AmbientLightColour.green	= 0.65f;
		AmbientLightColour.blue 	= 0.65f;;
		DirectionalLightColour.red 	= DirectionalLightColourForFrame.red;
		DirectionalLightColour.green= DirectionalLightColourForFrame.green;
		DirectionalLightColour.blue = DirectionalLightColourForFrame.blue;

	    RpLightSetColor(pAmbient, &AmbientLightColour);
	    RpLightSetColor(pDirect, &DirectionalLightColour);
	}
	else
	{
			// Use the standard light toned down a bit
		SetAmbientAndDirectionalColours(Mult * 0.9f);
	}
}

void SetBrightMarkerColours(float Mult)
{
	AmbientLightColour.red 		= 0.60f;	// was 0.6f
	AmbientLightColour.green 	= 0.60f;
	AmbientLightColour.blue 	= 0.60f;;
		// Now that the colour of the arrow is important we don't want to use the directional light anymore.
		// Blue markers end up being grey. The filters will still give the markers a colour that fits in with the scene.
	DirectionalLightColour.red 	= 1.0f;
	DirectionalLightColour.green= 1.0f;
	DirectionalLightColour.blue = 1.0f;
//	DirectionalLightColour.red = DirectionalLightColourForFrame.red + 0.4f*(1-DirectionalLightColourForFrame.red);
//	DirectionalLightColour.green = DirectionalLightColourForFrame.green + 0.4f*(1-DirectionalLightColourForFrame.green);
//	DirectionalLightColour.blue = DirectionalLightColourForFrame.blue + 0.4f*(1-DirectionalLightColourForFrame.blue);

	RpLightSetColor(pAmbient, &AmbientLightColour);
	RpLightSetColor(pDirect, &DirectionalLightColour);
}


/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAmbientAndDirectionalColours
// PURPOSE :  Fixes the random lights.
/////////////////////////////////////////////////////////////////////////////////

void ReSetAmbientAndDirectionalColours()
{
    RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
    RpLightSetColor(pDirect, &DirectionalLightColourForFrame);
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : DeActivateDirectional
// PURPOSE :  Switches off the directional lights.
/////////////////////////////////////////////////////////////////////////////////

void DeActivateDirectional()
{
	RpLightSetFlags(pDirect, 0);					// No directional light
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : ActivateDirectional
// PURPOSE :  Switches on the directional lights.
/////////////////////////////////////////////////////////////////////////////////

void ActivateDirectional()
{
	RpLightSetFlags(pDirect, rpLIGHTLIGHTATOMICS);	// Directional light back on
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAmbientColoursToIndicateRoadGroup
// PURPOSE :  Sets the ambient light colour to indicate what group this road
//			  segment belongs to.
/////////////////////////////////////////////////////////////////////////////////

UInt8	IndicateR [] = { 0, 255, 0,   0,   255, 255, 0    };
UInt8	IndicateG [] = { 0, 0,   255, 0,   255, 0,   255  };
UInt8	IndicateB [] = { 0, 0,   0,   255, 0,   255, 255  };

void SetAmbientColoursToIndicateRoadGroup(Int32 Number)
{
	AmbientLightColour.red 	= IndicateR[Number%7] / 255.0f;
	AmbientLightColour.green= IndicateG[Number%7] / 255.0f;
	AmbientLightColour.blue = IndicateB[Number%7] / 255.0f;;

    RpLightSetColor(pAmbient, &AmbientLightColour);
}

void SetFullAmbient()
{
    RpLightSetColor(pAmbient, &FullLight);
};

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAmbientColours
// PURPOSE :  Sets the ambient light colour for roads buildings etc
/////////////////////////////////////////////////////////////////////////////////

void SetAmbientColours()
{
	RpLightSetColor(pAmbient, &AmbientLightColourForFrame);
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetAmbientColoursForPedsCarsAndObjects
// PURPOSE :  Sets the ambient light colour for peds cars and objects
/////////////////////////////////////////////////////////////////////////////////

void SetAmbientColours(RwRGBAReal* pRgba)
{
	RpLightSetColor(pAmbient, pRgba);
}

void SetDirectionalColours(RwRGBAReal* pRgba)
{
	RpLightSetColor(pDirect, pRgba);
}

void SetLightColoursForPedsCarsAndObjects(float Mult)
{
	AmbientLightColour.red 		= CMaths::MultNorm(AmbientLightColourForFrame_PedsCarsAndObjects.red,	Mult);
	AmbientLightColour.green 	= CMaths::MultNorm(AmbientLightColourForFrame_PedsCarsAndObjects.green,	Mult);
	AmbientLightColour.blue 	= CMaths::MultNorm(AmbientLightColourForFrame_PedsCarsAndObjects.blue,	Mult);
	// Affect directional light more
	DirectionalLightColour.red 	= CMaths::MultNorm(DirectionalLightColourForFrame.red,		Mult);
	DirectionalLightColour.green= CMaths::MultNorm(DirectionalLightColourForFrame.green,	Mult);
	DirectionalLightColour.blue = CMaths::MultNorm(DirectionalLightColourForFrame.blue,		Mult);
	

	// If the brightness is turned up we want the ambient light to be boasted a bit.
//BrightnessXXX
	AmbientLightColour.red += CTimeCycle::m_BrightnessAddedToAmbientRed;
	AmbientLightColour.green += CTimeCycle::m_BrightnessAddedToAmbientGreen;
	AmbientLightColour.blue += CTimeCycle::m_BrightnessAddedToAmbientBlue;


    RpLightSetColor(pAmbient, &AmbientLightColour);
    RpLightSetColor(pDirect, &DirectionalLightColour);
}

//=======================================================
// The following functions are used for "Vision-FX" like
// the "night vision" and "infrared vision".
//=======================================================

static RwRGBAReal originalAmbientColour;
static RwRGBAReal originalDirectionalColour;

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetLightsForInfraredVisionHeatObjects
// PURPOSE :  Sets the color for objects which emit heat
/////////////////////////////////////////////////////////////////////////////////

void SetLightsForInfraredVisionHeatObjects()
{
	// Set "heat object" light color...
//	RwRGBAReal InfraredVisionFXColorHeatObjects = { 1.0f, 0.0f, 0.0f, 1.0f };

    RpLightSetColor( pAmbient, &CPostEffects::m_fInfraredVisionHeatObjectCol );
    RpLightSetColor( pDirect,  &CPostEffects::m_fInfraredVisionHeatObjectCol );
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : StoreAndSetLightsForInfraredVisionHeatObjects
// PURPOSE :  Stores current light values and sets the color for objects which
//            emit heat.
/////////////////////////////////////////////////////////////////////////////////

void StoreAndSetLightsForInfraredVisionHeatObjects()
{
	originalAmbientColour     = AmbientLightColour;
	originalDirectionalColour = DirectionalLightColour;

	SetLightsForInfraredVisionHeatObjects();
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : RestoreLightsForInfraredVisionHeatObjects
// PURPOSE :  Restores the original light color values for the lights stored
//            with the function "StoreAndSetLightsForInfraredVisionHeatObjects".
/////////////////////////////////////////////////////////////////////////////////

void RestoreLightsForInfraredVisionHeatObjects()
{
    RpLightSetColor( pAmbient, &originalAmbientColour );
    RpLightSetColor( pDirect,  &originalDirectionalColour );
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetLightsForInfraredVisionDefaultObjects
// PURPOSE :  Sets the light color for "cold" (default) objects
/////////////////////////////////////////////////////////////////////////////////

void SetLightsForInfraredVisionDefaultObjects()
{
	// Use blue as default color
	RwRGBAReal InfraredVisionFXColorDefault = { 0.0f, 0.0f, 1.0f, 1.0f };

    RpLightSetColor( pAmbient, &InfraredVisionFXColorDefault );
    RpLightSetColor( pDirect,  &InfraredVisionFXColorDefault );
}

/////////////////////////////////////////////////////////////////////////////////
// FUNCTION : SetLightsForNightVision
// PURPOSE :  Sets the object light color for the night vision FX
/////////////////////////////////////////////////////////////////////////////////

void SetLightsForNightVision()
{
	// Use green for everything
	RwRGBAReal NightVisionFXColor = { 0.0f, 1.0f, 0.0f, 1.0f };

    RpLightSetColor( pAmbient, &NightVisionFXColor );
    RpLightSetColor( pDirect,  &NightVisionFXColor );
}



