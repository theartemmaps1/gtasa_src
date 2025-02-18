/**********************************************************************
 *
 * File :     setup.c
 *
 * Abstract : Standard setup & termination routines for RenderWare
 *            applications.
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

/*--- Include Files ---*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rwcore.h>
#ifdef RWLOGO
#include <rplogo.h>
#endif
#include <rpworld.h>

#include <rpanim.h>
#include <rpbone.h>

#include "setup.h"


/*--- Global Variables ---*/

/* Scene contains all the global variables for this viewer */
GlobalScene   Scene;

/************************************************************************
 *
 *      Function:       InitialiseScene()
 *                      
 *      Description:    Initialise a scene structure. 
 *
 *      Parameters:     scene - the scene structure to be initialised.
 *
 *      Return Value:   None
 *
 ************************************************************************/
void
InitialiseScene(GlobalScene *scene)
{
    scene->world = NULL;
    scene->camera = NULL;
    scene->handle = NULL;
}

