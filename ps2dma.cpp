//
// file:	psxdma.cpp
// description: Functions for using the DMA on the PSX2
// written by:	Adam Fowler
//
// My headers
#include "ps2dma.h"

volatile uint32* CDma::pDmaRegs[NUM_DMA_CHANNELS] = {
	D0_CHCR,
	D1_CHCR,
	D2_CHCR,
	D3_CHCR,
	D4_CHCR,
	D5_CHCR,
	D6_CHCR,
	D7_CHCR,
	D8_CHCR,
	D9_CHCR};

#ifdef DEBUG

//
// DecodeSCDMAPacket: Decode a source chain DMA packet and print results to console
// in:	pTag = pointer to packet
// out:	next tag
//
static int32 dmaStackPosn = 0;
static sceDmaTag* DecodeSCDMAPacket(sceDmaTag* pTag)
{
	static sceDmaTag* dmaTagStack[2];
	sceDmaTag* pNextTag = NULL;
	void* pData = NULL;
	
	printf("Addr: 0x%x: ", pTag);
	DMA_ALIGN_CHECK(pTag);
	switch(pTag->id & 0x7f)
	{
	case 0x0:	
		printf("REFE");
		pData = pTag->next;
		break;
	case 0x10:
		printf("CNT");
		pData = pTag + 1;
		pNextTag = pTag + 1 + pTag->qwc;
		break;
	case 0x20:
		printf("NEXT");
		pData = pTag + 1;
		pNextTag = pTag->next;
		break;
	case 0x30:
		printf("REF");
		pData = pTag->next;
		pNextTag = pTag + 1;
		break;
	case 0x40:
		printf("REFS");
		pData = pTag->next;
		pNextTag = pTag + 1;
		break;
	case 0x50:
		printf("CALL");
		pData = pTag + 1;
		if(dmaStackPosn == 2)
		{
			printf(": DMA tag stack overflow\n");
			return NULL;
		}
		dmaTagStack[dmaStackPosn++]	= pTag + 1 + pTag->qwc;
		pNextTag = pTag->next;
		break;
	case 0x60:
		printf("RET");
		pData = pTag + 1;
		if(dmaStackPosn == 0)
		{
			printf(": No DMA tags left on the stack\n");
			return NULL;
		}
		pNextTag = dmaTagStack[--dmaStackPosn];
		break;
	case 0x70:
		pData = pTag + 1;
		printf("END");
		break;
	default:
		printf("ERROR");
		break;
	}
	
	if(pTag->id & 0x80)
		printf("[i]");
	printf(": data 0x%x, ", pData);
	DMA_ALIGN_CHECK(pData);
	printf("size 0x%x\n", pTag->qwc);
	
	return pNextTag;
}

//
// CDma::DecodeDCDMAPacket: Decode a destination chain DMA packet and print results to console
// in:	pTag = pointer to packet
// out:	next tag
//
static sceDmaTag* DecodeDCDMAPacket(sceDmaTag* pTag)
{
	sceDmaTag* pNextTag = NULL;
	void* pData = NULL;
	
	printf("Addr: 0x%x: ", pTag);
	
	DMA_ALIGN_CHECK(pTag);
	switch(pTag->id & 0x7f)
	{
	case 0x0:	
		printf("CNTS");
		pData = pTag + 1;
		pNextTag = pTag + 1 + pTag->qwc;
		break;
	case 0x10:
		printf("CNT");
		pData = pTag + 1;
		pNextTag = pTag + 1 + pTag->qwc;
		break;
	case 0x70:
		pData = pTag + 1;
		printf("END");
		break;
	default:
		printf("ERROR");
		break;
	}
	
	if(pTag->id & 0x80)
		printf("[i]");
	printf(": data 0x%x, ", pData);
	DMA_ALIGN_CHECK(pData);
	printf("size 0x%x\n", pTag->qwc);
	
	return pNextTag;
}

//
// CDma::DecodeDMASourceChain: Decode a DMA source chain and print results to console
// in:	pTag = pointer to first packet
//
void CDma::DecodeDMASourceChain(void* pTag)
{
	dmaStackPosn = 0;
	do
	{
		pTag = DecodeSCDMAPacket((sceDmaTag*)pTag);
	} while(pTag != NULL);
	
	if(dmaStackPosn != 0)
		printf("WARNING: dma tag left on the stack\n");
}

//
// CDma::DecodeDMADestinationChain: Decode a DMA destination chain and print results to console
// in:	pTag = pointer to first packet
//
void CDma::DecodeDMADestinationChain(void* pTag)
{
	do
	{
		pTag = DecodeDCDMAPacket((sceDmaTag*)pTag);
	} while(pTag != NULL);
}

#endif