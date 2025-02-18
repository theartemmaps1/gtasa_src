/* Turn dithering on/off in 16 bit mode */
//$DW$#include "rwcore.h"
//$DW$#include <eeregs.h>
//$DW$#include <libdma.h>


/* Define twos complement values for the dither matrix */
#ifndef DIMX0
#define DIMX_4 4l
#define DIMX_3 5l
#define DIMX_2 6l
#define DIMX_1 7l
#define DIMX0 0l
#define DIMX1 1l
#define DIMX2 2l
#define DIMX3 3l
#endif /* !DIMX0 */

/* Define the GS register for dither */
#ifndef GS_DIMX
#define GS_DIMX 0x44
#endif /* !GS_DIMX */

/* Handy 128 bit value burst */
#ifndef MAKE128
#define MAKE128(RES, MSB, LSB) \
    __asm__ volatile ( "pcpyld %0, %1, %2" : "=r" (RES) : "r" (MSB), "r" (LSB))
#endif /* !MAKE128 */

/* Stuff from the dma manager */
#ifndef SWE_LPS_CONT
#define SWE_LPS_CONT 1

extern "C" {
extern RwBool sweOpenLocalPkt(int type, int size);
extern u_long128 *sweLocalPacket;
}

/* Non-debug versions */
#define SWEADDCONTGIFFAST(A, C)  \
    *sweLocalPacket++ = (A)
#define SWEADDCONTFAST(A)        \
    *sweLocalPacket++ = (A)
#endif /* !SWE_LPS_CONT */

/* This function can only be called after the Engine has been started */
RwBool ditherOn(RwBool state)
{
    u_long128 gifTagCmd, dimxCmd;
    unsigned long tmp, tmp1;

    tmp = /* NLOOP */ 1l
        | /* EOP */ (1l<<15)
        | /* PRE */ (0l<<46)
        | /* FLG */ (0l<<58)
        | /* NREG */ (1l<<60);
    tmp1 = /* A+D */ (0xel<<(64-64));
    MAKE128(gifTagCmd, tmp1, tmp);

    if (state)
    {
        tmp = (DIMX_4 << 0)  | /* dimx(0,0) = -4 */
              (DIMX2 << 4)   | /* dimx(0,1) = 2 */
              (DIMX_3 << 8)  | /* dimx(0,2) = -3 */
              (DIMX3 << 12)  | /* dimx(0,3) = 3 */
              (DIMX0 << 16)  | /* dimx(1,0) = 0 */
              (DIMX_2 << 20) | /* dimx(1,1) = -2 */
              (DIMX1 << 24)  | /* dimx(1,2) = 1 */
              (DIMX_1 << 28) | /* dimx(1,3) = -1 */
              (DIMX_3 << 32) | /* dimx(2,0) = -3 */
              (DIMX3 << 36)  | /* dimx(2,1) = 3 */
              (DIMX_4 << 40) | /* dimx(2,2) = -4 */
              (DIMX2 << 44)  | /* dimx(2,3) = 2 */
              (DIMX1 << 48)  | /* dimx(3,0) = 1 */
              (DIMX_1 << 52) | /* dimx(3,1) = -1 */
              (DIMX0 << 56)  | /* dimx(3,2) = 0 */
              (DIMX_2 << 60);  /* dimx(3,3) = -2 */
        MAKE128(dimxCmd, GS_DIMX, tmp);
    }
    else
    {
        tmp = (DIMX3 << 0)  | /* dimx(0,0) = -4 */
              (DIMX2 << 4)   | /* dimx(0,1) = 2 */
              (DIMX3 << 8)  | /* dimx(0,2) = -3 */
              (DIMX3 << 12)  | /* dimx(0,3) = 3 */
              (DIMX0 << 16)  | /* dimx(1,0) = 0 */
              (DIMX2 << 20) | /* dimx(1,1) = -2 */
              (DIMX1 << 24)  | /* dimx(1,2) = 1 */
              (DIMX1 << 28) | /* dimx(1,3) = -1 */
              (DIMX3 << 32) | /* dimx(2,0) = -3 */
              (DIMX3 << 36)  | /* dimx(2,1) = 3 */
              (DIMX3 << 40) | /* dimx(2,2) = -4 */
              (DIMX2 << 44)  | /* dimx(2,3) = 2 */
              (DIMX1 << 48)  | /* dimx(3,0) = 1 */
              (DIMX1 << 52) | /* dimx(3,1) = -1 */
              (DIMX0 << 56)  | /* dimx(3,2) = 0 */
              (DIMX2 << 60);  /* dimx(3,3) = -2 */
        MAKE128(dimxCmd, GS_DIMX, tmp);
    }
    if (sweOpenLocalPkt(SWE_LPS_CONT, -2))
    {
        SWEADDCONTGIFFAST(gifTagCmd, 1);
        SWEADDCONTFAST(dimxCmd);
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}
