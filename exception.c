/* SCEI CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library libexcep Version 1.0
*/
/*
 *               Emotion Engine Library Sample Program
 *
 *                           - <excep> -
 *
 *                          Version <1.00>
 *                               SJIS
 *
 *      Copyright (C) 1998-2000 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                            <excep.c>
 *                    <check system configuration>
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      1.00            Aug,18,2000    kumagae     first version
 *
 */


#include <eekernel.h>
#include <stdio.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libcdvd.h>
#include <libexcep.h>
//#include "main.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 224
#define OFFX		(((4096-SCREEN_WIDTH)/2)<<4)
#define OFFY		(((4096-SCREEN_HEIGHT)/2)<<4)
#define MAXWIDTH	(77)
#define MAXHEIGHT	(27)


#define UNCACHED         (0x20000000)

/*----------------------------------------------------*/
void InitGS(void);
void EEExceptionHandler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr);
void IOPExceptionHandler(void *p,void *d);
/*----------------------------------------------------*/

sceGsDBuff db;
sceExcepIOPExceptionData iop;

const char *ExceptionCode[]={
  "Interrupt",
  "TLB modification exception",
  "TLB exception (load or instruction fetch)",
  "TLB exception (store)",
  "Address error exception (load or instruction fetch)",
  "Address error exception (store)",
  "Bus error exception (data reference: load or store)",
  "Syscall exception",
  "Breakpint exception",
  "Reserved instruction exception",
  "Coprocessor Unusable exception",
  "Arithmetic overflow exception",
  "Trap exception"
};
/*----------------------------------------------------*/

/* InitGS
   GS�������������B
 */

void InitGS(void)
{
  sceVif1Packet packet;	/* DMA�p�P�b�g�n���h�� */
  sceDmaEnv env;			/* DMA ���ʊ� */
  sceDmaChan *p1;
  u_long giftagAD[2] = { SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1),
			 0x000000000000000eL };	/* A�{D�pGIF�^�O */
  long128 buff[10];
  sceGsResetPath();
  sceDmaReset(1);

  /*���L���b�V���A�N�Z�X�Ńp�P�b�g���쐬�����B*/
  sceVif1PkInit(&packet, (u_long128 *)((u_int)buff | UNCACHED) );
  
  sceDmaGetEnv(&env);		/* DMA���ʊ��擾 */
  env.notify = 1<<SCE_DMA_VIF1; /* notify channel */
  sceDmaPutEnv(&env);
  
  /* DMA VIF�P�`�����l���f�X�N���v�^�擾 */
  p1 = sceDmaGetChan(SCE_DMA_VIF1);
  p1->chcr.TTE = 1;
  
  sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME);
  sceGsSetDefDBuff(&db, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT,
		   SCE_GS_ZGEQUAL, SCE_GS_PSMZ24, SCE_GS_CLEAR);
  sceVif1PkReset(&packet);
  sceVif1PkCnt(&packet, 0);
  sceVif1PkOpenDirectCode(&packet, 0);
  sceVif1PkOpenGifTag(&packet, *(u_long128*)&giftagAD);
  
  /* add alpha environment packet */
  sceVif1PkReserve(&packet,
		   sceGsSetDefAlphaEnv((sceGsAlphaEnv *)packet.pCurrent,0)
		   * 4);
  sceVif1PkCloseGifTag(&packet);
  sceVif1PkCloseDirectCode(&packet);
  sceVif1PkEnd(&packet, 0);
  sceVif1PkTerminate(&packet);

  /* kick Gs initialize packet */
  /*DMA�̓]���A�h���X�͕����A�h���X���w�肷��*/
  sceDmaSend(p1,(long128*)(((u_int)(packet.pBase)) & 0x8fffffff));
  /* wait for Gs initialize packet end */
  sceGsSyncPath(0,0);
  while(!sceGsSyncV(0)) {}	/* display next in odd field when interlace*/
}


/*  EEExceptionHandler
    EE���O�n���h��
    ���O�������������Ăяo����,EE�̃��W�X�^���\������
*/
void   EEExceptionHandler(u_int stat, u_int cause, u_int epc, u_int bva, u_int bpa, u_long128 *gpr)
{
 	int frame=0;
 	
 	// If caused by a 'breakc' don't want this screen to hide the ASSERT print
 	if(((cause>>2)&31) == 9)
 		return;

	InitGS();
  sceExcepConsOpen(OFFX + (16<<4), OFFY + (16<<3), MAXWIDTH, MAXHEIGHT );
  db.clear0.rgbaq.R = 0x0;	/* clear color for dbuf0 */
  db.clear0.rgbaq.G = 0x0;
  db.clear0.rgbaq.B = 0x80;
  
  db.clear1.rgbaq.R = 0x0;	/* clear color for dbuf1 */
  db.clear1.rgbaq.G = 0x0;
  db.clear1.rgbaq.B = 0x80;
  scePrintf("*** EE Exception [%s] ***\n",ExceptionCode[(cause>>2)&31]);
    
  scePrintf("stat=0x%08x cause=0x%08x epc=0x%08x bva=0x%08x bpa=0x%08x\n",stat,cause,epc,bva,bpa);
  scePrintf("at  =0x%08x     v0-1=0x%08x,0x%08x \n",gpr[GPR_at],gpr[GPR_v0],gpr[GPR_v1]);
  scePrintf("a0-3=0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_a0],gpr[GPR_a1],gpr[GPR_a2],gpr[GPR_a3]);
  scePrintf("t0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",gpr[GPR_t0],gpr[GPR_t1],gpr[GPR_t2],gpr[GPR_t3]);
  scePrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_t4],gpr[GPR_t5],gpr[GPR_t6],gpr[GPR_t7]);
  scePrintf("s0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",gpr[GPR_s0],gpr[GPR_s1],gpr[GPR_s2],gpr[GPR_s3]);
  scePrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_s4],gpr[GPR_s5],gpr[GPR_s6],gpr[GPR_s7]);
  scePrintf("t8-9=0x%08x,0x%08x     ",gpr[GPR_t8],gpr[GPR_t9]);
  scePrintf("k0-1=0x%08x,0x%08x\n",gpr[GPR_k0],gpr[GPR_k1]);
  scePrintf("gp=0x%08x sp=0x%08x fp=0x%08x ra=0x%08x\n",gpr[GPR_gp],gpr[GPR_sp],gpr[GPR_fp],gpr[GPR_ra]);


  while(1){
    sceExcepConsLocate( 0, 0 );
    
    sceExcepConsPrintf("*** EE Exception [%s] ***\n",ExceptionCode[(cause>>2)&31]);
    
    sceExcepConsPrintf("stat=0x%08x cause=0x%08x epc=0x%08x bva=0x%08x bpa=0x%08x\n",stat,cause,epc,bva,bpa);
    sceExcepConsPrintf("at  =0x%08x     v0-1=0x%08x,0x%08x \n",gpr[GPR_at],gpr[GPR_v0],gpr[GPR_v1]);
    sceExcepConsPrintf("a0-3=0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_a0],gpr[GPR_a1],gpr[GPR_a2],gpr[GPR_a3]);
    sceExcepConsPrintf("t0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",gpr[GPR_t0],gpr[GPR_t1],gpr[GPR_t2],gpr[GPR_t3]);
    sceExcepConsPrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_t4],gpr[GPR_t5],gpr[GPR_t6],gpr[GPR_t7]);
    sceExcepConsPrintf("s0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",gpr[GPR_s0],gpr[GPR_s1],gpr[GPR_s2],gpr[GPR_s3]);
    sceExcepConsPrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",gpr[GPR_s4],gpr[GPR_s5],gpr[GPR_s6],gpr[GPR_s7]);
    sceExcepConsPrintf("t8-9=0x%08x,0x%08x     ",gpr[GPR_t8],gpr[GPR_t9]);
    sceExcepConsPrintf("k0-1=0x%08x,0x%08x\n",gpr[GPR_k0],gpr[GPR_k1]);
    sceExcepConsPrintf("gp=0x%08x sp=0x%08x fp=0x%08x ra=0x%08x\n",gpr[GPR_gp],gpr[GPR_sp],gpr[GPR_fp],gpr[GPR_ra]);
    
    if( frame & 1 )
      {	/* interrace half pixcel adjust */
	sceGsSetHalfOffset( &db.draw1, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
      } else {
	sceGsSetHalfOffset( &db.draw0, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
      }
    iFlushCache( 0 );
    sceGsSyncPath( 0, 0 );
    sceGsSwapDBuff( &db, frame++ );
  }
}

/*  IOPExceptionHandler
    IOP���O�n���h��
    ���O�������������Ăяo����,IOP�����󂯂Ƃ���IOP�̃��W�X�^��IOP���W���[�������\������
*/
void IOPExceptionHandler(void *p,void *d)
{
  int frame=0;
  /*���̊֐��̓����ł�scePrintf�����g���Ă͂����Ȃ��B
    IOP�������ł����̂�scePrintf���g���Ă��܂���,
    ���̏��Œ��~���Ă��܂��B*/
  InitGS();
  
  db.clear0.rgbaq.R = 0x00;
  db.clear0.rgbaq.G = 0x80;
  db.clear0.rgbaq.B = 0x00;

  db.clear1.rgbaq.R = 0x00;
  db.clear1.rgbaq.G = 0x80;
  db.clear1.rgbaq.B = 0x00;
  sceExcepConsOpen(OFFX + (16<<4), OFFY + (16<<3),MAXWIDTH ,MAXHEIGHT );
  while(1){
    sceExcepConsLocate( 0, 0 );
    sceExcepConsPrintf("*** IOP Exception [%s] ***\n",ExceptionCode[(iop.reg[IOP_CAUSE]>>2)&31]);
    sceExcepConsPrintf("Module [%s] Version [%02x.%02x] Offset [0x%08x]\n",iop.module ,(iop.version)>>8 ,(iop.version)&0xf,iop.offset); 
    sceExcepConsPrintf("stat=0x%08x cause=0x%08x epc=0x%08x\n",iop.reg[IOP_SR] ,iop.reg[IOP_CAUSE], iop.reg[IOP_EPC]);
    sceExcepConsPrintf("at  =0x%08x     v0-1=0x%08x,0x%08x \n",iop.reg[GPR_at],iop.reg[GPR_v0], iop.reg[GPR_v1]);
    sceExcepConsPrintf("a0-3=0x%08x,0x%08x,0x%08x,0x%08x\n",iop.reg[GPR_a0], iop.reg[GPR_a1], iop.reg[GPR_a2], iop.reg[GPR_a3]);
    sceExcepConsPrintf("t0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",iop.reg[GPR_t0], iop.reg[GPR_t1], iop.reg[GPR_t2], iop.reg[GPR_t3]);
    sceExcepConsPrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",iop.reg[GPR_t4], iop.reg[GPR_t5], iop.reg[GPR_t6], iop.reg[GPR_t7]);
    sceExcepConsPrintf("s0-7=0x%08x,0x%08x,0x%08x,0x%08x,\n",iop.reg[GPR_s0], iop.reg[GPR_s1], iop.reg[GPR_s2], iop.reg[GPR_s3]);
    sceExcepConsPrintf("     0x%08x,0x%08x,0x%08x,0x%08x\n",iop.reg[GPR_s4], iop.reg[GPR_s5], iop.reg[GPR_s6], iop.reg[GPR_s7]);
    sceExcepConsPrintf("t8-9=0x%08x,0x%08x     ",iop.reg[GPR_t8], iop.reg[GPR_t9]);
    sceExcepConsPrintf("k0-1=0x%08x,0x%08x\n",iop.reg[GPR_k0], iop.reg[GPR_k1]);
    sceExcepConsPrintf("gp=0x%08x sp=0x%08x fp=0x%08x ra=0x%08x\n",iop.reg[GPR_gp], iop.reg[GPR_sp], iop.reg[GPR_fp], iop.reg[GPR_ra]);
    sceExcepConsPrintf("hi=0x%08x lo=0x%08x sr=0x%08x epc =0x%08x\n",iop.reg[IOP_HI], iop.reg[IOP_LO], iop.reg[IOP_SR], iop.reg[IOP_EPC]);
    sceExcepConsPrintf("cause=0x%08x tar=0x%08x badadr=0x%08x\n", iop.reg[IOP_CAUSE], iop.reg[IOP_TAR], iop.reg[IOP_BADVADDR]);
    sceExcepConsPrintf("dcic=0x%08x bpc=0x%08x bpcm=0x%08x bda=0x%08x bpam=0x%08x\n",iop.reg[IOP_DCIC], iop.reg[IOP_BPC], iop.reg[IOP_BPCM], iop.reg[IOP_BDA], iop.reg[IOP_BDAM]);
   
    if( frame & 1 )
      {	/* interrace half pixcel adjust */
	sceGsSetHalfOffset( &db.draw1, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
      } else {
	sceGsSetHalfOffset( &db.draw0, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
      }
    iFlushCache( 0 );
    sceGsSyncPath( 0, 0 );
    sceGsSwapDBuff( &db, frame++ );
  }
}

void InitExceptionCode(char* pIOPModule)
{
	sceExcepSetDebugEEHandler(EEExceptionHandler);
  	sceExcepSetDebugIOPHandler(pIOPModule,IOPExceptionHandler, &iop);
}
