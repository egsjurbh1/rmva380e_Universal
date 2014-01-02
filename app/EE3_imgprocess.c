/**
 * \file      	EE3_imgprocess.c
 * \author    	Leo bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	开源的图像空间转换算法
 * \			Bayer->YUV420	Bayer->RGB	RGB->YUV420		YUV422->YUV420
**/


#include "EE3_common.h"
/**************************************************************************/


extern RoseekJPGEncBufStruct	g_JpgEncBufStruct;//JPEG图像缓存结构体,其实例化对象用来存储JPEG编码模块用到的中间数据

void Roseek_Get_YCrCb420ForJPGEncoder( enum Roseek_ImgAcquisition_Format, Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
Bool Roseek_BayerToRGB_8bits( Uint32*, Uint32*, Uint32, Uint32 );
void Roseek_Bayer2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32, Uint32 );
void Roseek_RGB2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
void Roseek_YUV2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );

//为JPG编码器准备YUV数据
void Roseek_Get_YCrCb420ForJPGEncoder( enum Roseek_ImgAcquisition_Format enumImgFormat, Uint8* pui8SurImg, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 ui32ColSize, Uint32 ui32RowSize )
{
	switch( enumImgFormat ){
		case RGB_ARRAY ://RGB Array
			Roseek_RGB2YCrCb420(	pui8SurImg,
									pui8YBuf,
									pui8CrBuf,
									pui8CbBuf,
									ui32ColSize,
									ui32RowSize );
			break;
		case YUV_ARRAY ://YUV Array
			Roseek_YUV2YCrCb420(	pui8SurImg,
									pui8YBuf,
									pui8CrBuf,
									pui8CbBuf,
									ui32ColSize,
									ui32RowSize );
			break;
		case BAYER_ARRAY ://Bayer Array
			Roseek_Bayer2YCrCb420(	pui8SurImg,
									pui8YBuf,
									pui8CrBuf,
									pui8CbBuf,
									1,
									ui32ColSize,
									ui32RowSize );
			break;
		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// function:	Roseek_Bayer2YCrCb420
// 
// Descript:	(Bayer->YUV420) Convert Bayer pattern souce data to YCrCb420 
// Input:		
// Output:		
// Note:		(1) Bayer Pattern( scan from left to right, from top to bottom):
//					RGrRGrRGr
//					GbBGbBGbB
//					RGrRGrRGr
//					GbBGbBGbB
//				(2) Fomula
//					Y  = truncate {0.299*R + 0.587*G + 0.114*B}
//					Cr = truncate {128 + 0.5*R - 0.4187*G - 0.0813*B}
//					Cb = truncate {128 - 0.1687*R - 0.3313*G + 0.5*B}
//					Equal to:
//					Y  = [ 19595*R + 37880*G + 7471*B] >> 16
//					Cr = [ 8388608 + 32768*R - 27440*G - 5328*B] >> 16
//					Cb = [ 8388608 - 11056*R - 21712*G + 32768*B] >> 16
///////////////////////////////////////////////////////////////////////////////////
void Roseek_Bayer2YCrCb420( Uint8* pui8BayerInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 ui32Fill, Uint32 colsize, Uint32 rowsize)
{
	Uint8 * restrict pBuff = pui8BayerInBuf + colsize;
	Uint8 * restrict pSrc = pui8BayerInBuf;
	Uint8 * restrict pDestY = pui8YBuf;
	Uint8 * restrict pDestCb = pui8CrBuf;
	Uint8 * restrict pDestCr = pui8CbBuf;
	Uint32 i,j;
	Uint8 G;

	for( i=0; i<(rowsize/2-1); i++) {
		// process the row: RGrRGrRGr
		for( j=0; j<(colsize/2-1); j++) {
			G = (*pBuff+*(pSrc+1))>>1;
			*pDestY = ( 19595*(*pSrc) + 37880*G + 7471*(*(pBuff+1))) >> 16;
			*pDestCr = ( 8388608 + 32768*(*pSrc) - 27440*G - 5328*(*(pBuff+1))) >> 16;
			pSrc++;
			pBuff++;
			pDestY++;
			pDestCr++;
			G = (*(pBuff+1)+*pSrc)>>1;
			*pDestY = ( 19595*(*(pSrc+1)) + 37880*G + 7471*(*pBuff)) >> 16;
			pSrc++;
			pBuff++;
			pDestY++;	
		}
		// process the last two pixels
		G = (*pBuff+*(pSrc+1))>>1;
		*pDestY = ( 19595*(*pSrc) + 37880*G + 7471*(*(pBuff+1))) >> 16;
		*(pDestY+1) = *pDestY;
		*pDestCr = ( 8388608 + 32768*(*pSrc) - 27440*G - 5328*(*(pBuff+1))) >> 16;
		pSrc += 2;
		pBuff += 2;
		pDestY += 2;
		pDestCr++;
		// process the row: GbBGbBGbB
		for( j=0; j<(colsize/2-1); j++) {
			G = (*(pBuff+1)+*pSrc)>>1;
			*pDestY = ( 19595*(*pBuff) + 37880*G + 7471*(*(pSrc+1))) >> 16;
			*pDestCb = ( 8388608 + 32768*(*(pSrc+1)) - 11056*(*pBuff) - 21712*G) >> 16;
			pSrc++;
			pBuff++;
			pDestY++;
			pDestCb++;
			G = (*(pSrc+1)+*pBuff)>>1;
			*pDestY = ( 19595*(*(pBuff+1)) + 37880*G + 7471*(*pSrc)) >> 16;
			pSrc++;
			pBuff++;
			pDestY++;
		}
		// process the last two pixels
		G = (*(pBuff+1)+*pSrc)>>1;
		*pDestY = ( 19595*(*pBuff) + 37880*G + 7471*(*(pSrc+1))) >> 16;
		*(pDestY+1) = *pDestY;
		*pDestCb = ( 8388608 + 32768*(*(pSrc+1)) - 11056*(*pBuff) - 21712*G) >> 16;
		pSrc += 2;
		pBuff+=2;
		pDestY += 2;
		pDestCb++;
	}
	// process the last two row
	if( ui32Fill>0 ){
		for( j=0; j<colsize/2; j++) {
			G = (*pBuff+*(pSrc+1))>>1;
			*pDestY = ( 19595*(*pSrc) + 37880*G + 7471*(*(pBuff+1))) >> 16;
			*(pDestY+colsize) = *pDestY;
			*pDestCr = ( 8388608 + 32768*(*pSrc) - 27440*G - 5328*(*(pBuff+1))) >> 16;
			*pDestCb = ( 8388608 + 32768*(*(pBuff+1)) - 11056*(*pSrc) - 21712*G) >> 16;
			pSrc++;
			pBuff += 2;
			pDestY++;
			pDestCr++;
			pDestCb++;
			G = (*pBuff+*pSrc)>>1;
			*pDestY = ( 19595*(*(pSrc+1)) + 37880*G + 7471*(*(pBuff-1))) >> 16;
			*(pDestY+colsize) = *pDestY;
			pSrc++;
			pDestY++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// function:	Roseek_RGB2YCrCb420
// 
// Descript:	(RGB->YUV420) Convert Bayer pattern souce data to YCrCb420
// Input:		
// Output:		
//				Fomula
//					Y  = truncate {0.299*R + 0.578*G + 0.114*B}
//					Cr = truncate {128 + 0.5*R - 0.4187*G - 0.0813*B}
//					Cb = truncate {128 - 0.1687*R - 0.3313*G + 0.5*B}
//					Equal to:
//					Y  = [ 19595*R + 37880*G + 7471*B] >> 16
//					Cr = [ 8388608 + 32768*R - 27440*G - 5328*B] >> 16
//					Cb = [ 8388608 - 11056*R - 21712*G + 32768*B] >> 16
///////////////////////////////////////////////////////////////////////////////////
void Roseek_RGB2YCrCb420( Uint8* pui8RGBInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 colsize, Uint32 rowsize)
{
	Uint8 * restrict pSrc = pui8RGBInBuf;
	Uint8 * restrict pDestY = pui8YBuf;
	Uint8 * restrict pDestCb = pui8CrBuf;
	Uint8 * restrict pDestCr = pui8CbBuf;
	Uint32 i,j;

	for( i=0; i<rowsize/2; i++){
		for( j=0; j<colsize/2;j++ ){
			*pDestY = ( 19595*(*(pSrc+2)) + 37880*(*(pSrc+1)) + 7471*(*pSrc)) >> 16;
			*pDestCr = ( 8388608 + 32768*(*(pSrc+2)) - 27440*(*(pSrc+1)) - 5328*(*pSrc)) >> 16;
			pSrc += 3;
			pDestY++;
			pDestCr++;
			*pDestY = ( 19595*(*(pSrc+2)) + 37880*(*(pSrc+1)) + 7471*(*pSrc)) >> 16;
			pSrc += 3;
			pDestY++;
		}
		for( j=0; j<colsize/2;j++ ){
			*pDestY = ( 19595*(*(pSrc+2)) + 37880*(*(pSrc+1)) + 7471*(*pSrc)) >> 16;
			*pDestCb = ( 8388608 + 32768*(*pSrc) - 11056*(*(pSrc+2)) - 21712*(*(pSrc+1))) >> 16;
			pSrc += 3;
			pDestY++;
			pDestCb++;
			*pDestY = ( 19595*(*(pSrc+2)) + 37880*(*(pSrc+1)) + 7471*(*pSrc)) >> 16;
			pSrc += 3;
			pDestY++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// function:	Roseek_YUV2YCrCb420
// 
// Descript:	(YUV422->YUV420) Convert Bayer pattern souce data to YCrCb420
// Input:		
// Output:		
//				Fomula
//					Y  = truncate {0.299*R + 0.578*G + 0.114*B}
//					Cr = truncate {128 + 0.5*R - 0.4187*G - 0.0813*B}
//					Cb = truncate {128 - 0.1687*R - 0.3313*G + 0.5*B}
//					Equal to:
//					Y  = [ 19595*R + 37880*G + 7471*B] >> 16
//					Cr = [ 8388608 + 32768*R - 27440*G - 5328*B] >> 16
//					Cb = [ 8388608 - 11056*R - 21712*G + 32768*B] >> 16
///////////////////////////////////////////////////////////////////////////////////
void Roseek_YUV2YCrCb420( Uint8* pui8YUVInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 colsize, Uint32 rowsize)
{
	Uint8 * restrict pSrc = pui8YUVInBuf;
	Uint8 * restrict pDestY = pui8YBuf;
	Uint8 * restrict pDestCb = pui8CrBuf;
	Uint8 * restrict pDestCr = pui8CbBuf;
	Uint32 i,j;

	for( i=0; i<rowsize/2; i++){
		for( j=0; j<colsize/2;j++ ){
			*pDestY = *(pSrc+1);
			*pDestCb = *pSrc;
			pSrc += 2;
			pDestY++;
			pDestCb++;
			*pDestY = *(pSrc+1);
			pSrc += 2;
			pDestY++;
		}
		for( j=0; j<colsize/2;j++ ){
			*pDestY = *(pSrc+1);
			*pDestCr = *(pSrc+2);
			pSrc += 2;
			pDestY++;
			pDestCr++;
			*pDestY = *(pSrc+1);
			pSrc += 2;
			pDestY++;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// function:	(Bayer->RGB) Roseek_BayerToRGB_8bits
// 
// Parameters:	ui32ImgType:		Image resolution(1: 1360X1024; 2: 1360X256)
//				pui32BayerImgBuf:	Bayer Image Buffer Pointer
//				pui32RGBImgBuf:		RGB Image Buffer Pointer
// Descript:	
// Note:	Bayer array	
//			RGrRGrRGr
//			GbBGbBGbB
//			RGrRGrRGr
//			GbBGbBGbB		
//////////////////////////////////////////////////////////////////////
Bool Roseek_BayerToRGB_8bits( Uint32* pui32BayerImgBuf, Uint32* pui32RGBImgBuf, Uint32 colsize, Uint32 rowsize )
{
	Uint32 i,j;
	Uint32 ui32TmpRow,ui32TmpCol;
	Uint8* restrict pui8Src1;
	Uint8* restrict pui8Src2;
	Uint8* restrict pui8Src3;
	Uint8* restrict pui8Src4;
	Uint8* restrict pui8Src5;
	Uint8* restrict pui8Src6;
	Uint8* restrict pui8Src7;
	Uint8* restrict pui8Src8;
	Uint8* restrict pui8Src9;
	Uint8* restrict pui8Src10;
	Uint8* restrict pui8Src11;
	Uint8* restrict pui8Src12;
	Uint8* restrict pui8Dest;

	ui32TmpRow = rowsize;
	ui32TmpCol = colsize;

	//pointers initialization for GbBGbB lines
	pui8Src1	= (Uint8*)pui32BayerImgBuf;							// --> R
	pui8Src2	= (Uint8*)pui32BayerImgBuf + 1;						// --> Gr
	pui8Src3	= (Uint8*)pui32BayerImgBuf + 2;						// --> R
	pui8Src4	= (Uint8*)pui32BayerImgBuf + 3;						// --> Gr
	pui8Src5	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>2));		// --> Gb
	pui8Src6	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>2)) + 1;	// --> B
	pui8Src7	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>2)) + 2;	// --> Gb
	pui8Src8	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>2)) + 3;	// --> B
	pui8Src9	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>1));		// --> R
	pui8Src10	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>1)) + 1;	// --> Gr
	pui8Src11	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>1)) + 2;	// --> R
	pui8Src12	= (Uint8*)(pui32BayerImgBuf + (ui32TmpCol>>1)) + 3;	// --> Gr
	pui8Dest	= (Uint8*)pui32RGBImgBuf + (ui32TmpCol + 1)*3;
	//Interpolation for Bayer array to RGB array (location corresponding B in GbBGbB lines)
	for( i=0; i<(ui32TmpRow-2)/2; i++){
		//GbBGbB lines
		for( j=0; j<(ui32TmpCol-2)/2; j++){
			*pui8Dest++ = *pui8Src6;											//b
			*pui8Dest++ = (*pui8Src2 + *pui8Src5 + *pui8Src7 + *pui8Src10)>>2;	//g
			*pui8Dest++ = (*pui8Src1 + *pui8Src3 + *pui8Src9 + *pui8Src11)>>2;	//r

			*pui8Dest++ = (*pui8Src6 + *pui8Src8)>>1;							//b
			*pui8Dest++ = (*pui8Src2 + *pui8Src7)>>1;							//g
			*pui8Dest++ = (*pui8Src3 + *pui8Src11)>>1;							//r

			pui8Src1	+= 2;
			pui8Src2	+= 2;
			pui8Src3	+= 2;
			pui8Src4	+= 2;
			pui8Src5	+= 2;
			pui8Src6	+= 2;
			pui8Src7	+= 2;
			pui8Src8	+= 2;
			pui8Src9	+= 2;
			pui8Src10	+= 2;
			pui8Src11	+= 2;
			pui8Src12	+= 2;
		}
		pui8Src1	+= 2;
		pui8Src2	+= 2;
		pui8Src3	+= 2;
		pui8Src4	+= 2;
		pui8Src5	+= 2;
		pui8Src6	+= 2;
		pui8Src7	+= 2;
		pui8Src8	+= 2;
		pui8Src9	+= 2;
		pui8Src10	+= 2;
		pui8Src11	+= 2;
		pui8Src12	+= 2;
		pui8Dest	+= 6;

		//RGrRGr lines
		for( j=0; j<(ui32TmpCol-2)/2; j++){
			*pui8Dest++ = (*pui8Src2 + *pui8Src10)>>1;							//b
			*pui8Dest++ = (*pui8Src1 + *pui8Src6)>>1;							//g
			*pui8Dest++ = (*pui8Src5 + *pui8Src7)>>1;							//r

			*pui8Dest++ = (*pui8Src2 + *pui8Src4 + *pui8Src10 + *pui8Src12)>>2;	//b
			*pui8Dest++ = (*pui8Src3 + *pui8Src6 + *pui8Src8 + *pui8Src11)>>2;	//g
			*pui8Dest++ = *pui8Src7;											//r

			pui8Src1	+= 2;
			pui8Src2	+= 2;
			pui8Src3	+= 2;
			pui8Src4	+= 2;
			pui8Src5	+= 2;
			pui8Src6	+= 2;
			pui8Src7	+= 2;
			pui8Src8	+= 2;
			pui8Src9	+= 2;
			pui8Src10	+= 2;
			pui8Src11	+= 2;
			pui8Src12	+= 2;
		}
		pui8Src1	+= 2;
		pui8Src2	+= 2;
		pui8Src3	+= 2;
		pui8Src4	+= 2;
		pui8Src5	+= 2;
		pui8Src6	+= 2;
		pui8Src7	+= 2;
		pui8Src8	+= 2;
		pui8Src9	+= 2;
		pui8Src10	+= 2;
		pui8Src11	+= 2;
		pui8Src12	+= 2;
		pui8Dest	+= 6;
	}

	//fill image edge
	pui8Src1 = (Uint8*)pui32RGBImgBuf + 3;
	pui8Dest = (Uint8*)pui32RGBImgBuf;
	for( i=0; i<ui32TmpRow; i++){
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		pui8Src1 += (ui32TmpCol-1)*3;
		pui8Dest += (ui32TmpCol-1)*3;
	}
	pui8Src1 = (Uint8*)pui32RGBImgBuf + (ui32TmpCol - 2)*3;
	pui8Dest = (Uint8*)pui32RGBImgBuf + (ui32TmpCol - 1)*3;
	for( i=0; i<ui32TmpRow; i++){
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		pui8Src1 += (ui32TmpCol-1)*3;
		pui8Dest += (ui32TmpCol-1)*3;
	}

	pui8Src1 = (Uint8*)pui32RGBImgBuf + ui32TmpCol*3;
	pui8Dest = (Uint8*)pui32RGBImgBuf;
	for( i=0; i<ui32TmpCol; i++){
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
	}
	pui8Src1 = (Uint8*)pui32RGBImgBuf + ui32TmpCol*(ui32TmpRow-2)*3;
	pui8Dest = (Uint8*)pui32RGBImgBuf + ui32TmpCol*(ui32TmpRow-1)*3;
	for( i=0; i<ui32TmpCol; i++){
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
		*pui8Dest++ = *pui8Src1++;
	}

	return TRUE;
}


