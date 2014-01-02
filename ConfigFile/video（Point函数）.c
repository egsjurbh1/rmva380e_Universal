/*
 * video.c
 *
 *  Created on: 2011-9-26
 *      Author: amam
 */
/* Standard Linux headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

/* Codec Engine headers */
#include <xdc/std.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/osal/Memory.h>
#include <ti/sdo/ce/video/viddec.h>
#include <ti/sdo/ce/video/videnc.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include "encodedecode.h"
#include "display.h"
#include "video.h"
#include "capture.h"

extern void handler();

/* Number of buffers between video thread and display thread */
#define DISPLAY_BUFFERS          3

/* Number of buffers for the capture thread */
#define CAP_BUFFERS              4

/* Triple buffering for the capture driver */
#define NUM_BUFS                 3

/* Work around a bug in the algorithms (using scratch in init) */
#define ALGO_INIT_WORKAROUND

#define UYVY_BLACK               0x10801080

/* Structure containing statistics about the frames in a clip */
typedef struct FrameStats {
	Int framesRejected;
	Int iFrames;
	Int pFrames;
	Int bFrames;
	Int idrFrames;
} FrameStats;

#define carLength 5
#define carSlot 1
#define acceleration 2
#define responseTime 2
#define maxVelocity 12
#define minGT 20
#define maxGT 60

Int run(Float length) {
	int carNum = ceil(((double) length) / (carLength + carSlot));
	int maxAccTime = ceil(((double) maxVelocity) / acceleration);
	int maxAccLength = ceil(0.5 * acceleration * maxAccTime * maxAccTime);

	if (length < maxAccLength) {
		int tmp = responseTime * carNum + ceil(sqrt(((double) 2 * length)
				/ acceleration));
		if (tmp < minGT)
			return minGT;
		else if (tmp > maxGT)
			return maxGT;
		else
			return tmp;
	} else {
		int tmp = responseTime * carNum + maxAccTime + ceil(((double) length
				- maxAccLength) / maxVelocity);
		if (tmp < minGT)
			return minGT;
		else if (tmp > maxGT)
			return maxGT;
		else
			return tmp;
	}
}

/* Local function prototypes */
static Int videoEncodeAlgCreate(Engine_Handle hEngine,
		VIDENC_Handle *hEncodePtr, Int width, Int height, Int bitrate);
static Int encodeVideoBuffer(VIDENC_Handle hEncode, Char *inBuf, Int inBufSize,
		Int *firstI, Int *lastI, Char *backBuf, Point *pt);

/******************************************************************************
 * videoEncodeAlgCreate
 ******************************************************************************/
static Int videoEncodeAlgCreate(Engine_Handle hEngine,
		VIDENC_Handle *hEncodePtr, Int width, Int height, Int bitrate) {
	VIDENC_Params params;
	VIDENC_Handle hEncode;

	params.size = sizeof(VIDENC_Params);
	params.encodingPreset = XDM_DEFAULT;
	params.maxFrameRate = 25000;
	params.dataEndianness = XDM_BYTE;
	params.maxInterFrameInterval = 0;
	params.inputChromaFormat = XDM_YUV_422ILE;
	params.inputContentType = IVIDEO_PROGRESSIVE;
	params.maxHeight = HEIGHT_N;
	params.maxWidth = WIDTH_N;

	/* Create encoder instance */
	hEncode = VIDENC_create(hEngine, "Grayscale", &params);

	if (hEncode == NULL) {
		ERR("Can't open encode algorithm.\n");
		return FAILURE;
	}

	*hEncodePtr = hEncode;

	return SUCCESS;
}

/******************************************************************************
 * encodeVideoBuffer
 ******************************************************************************/
static Int encodeVideoBuffer(VIDENC_Handle hEncode, Char *inBuf, Int inBufSize,
		Int *firstI, Int *lastI, Char *backBuf, Point *pt) {
	XDAS_Int32 inBufSizeArray[3];
	XDAS_Int32 outBufSizeArray[2];
	XDAS_Int32 status;
	XDM_BufDesc inBufDesc;
	XDM_BufDesc outBufDesc;
	VIDENC_InArgs inArgs;
	VIDENC_OutArgs outArgs;
	XDAS_Int8 **addr;
	XDAS_Int8 **addrO;

	addr = (XDAS_Int8 **) malloc(sizeof(XDAS_Int8 *) * 3);
	*addr = backBuf;
	*(addr + 1) = inBuf;
	*(addr + 2) = pt;

	inBufSizeArray[0] = inBufSize;
	inBufSizeArray[1] = inBufSize;
	inBufSizeArray[2] = sizeof(Point);

	outBufSizeArray[0] = sizeof(Int);
	outBufSizeArray[1] = sizeof(Int);

	inBufDesc.numBufs = 3;
	inBufDesc.bufSizes = inBufSizeArray;
	inBufDesc.bufs = addr;

	addrO = (XDAS_Int8 **) malloc(sizeof(XDAS_Int8 *) * 2);
	*addrO = firstI;
	*(addrO + 1) = lastI;

	outBufDesc.numBufs = 2;
	outBufDesc.bufSizes = outBufSizeArray;
	outBufDesc.bufs = addrO;

	inArgs.size = sizeof(VIDENC_InArgs);
	outArgs.size = sizeof(VIDENC_OutArgs);

	/* Encode video buffer */
	status
			= VIDENC_process(hEncode, &inBufDesc, &outBufDesc, &inArgs,
					&outArgs);

	if (status != VIDENC_EOK) {
		ERR("VIDENC_process() failed with a fatal error (%ld ext: %#lx)\n",
				status, outArgs.extendedError);
		return FAILURE;
	}

	//*outBufSize = outArgs.bytesGenerated;
	free(addr);
	return SUCCESS;
}

Int set_opt(int fd) {
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;
	newtio.c_cflag &= ~PARENB;
	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cc[VTIME] = 0;//重要
	newtio.c_cc[VMIN] = 100;//返回的最小值  重要
	tcflush(fd, TCIFLUSH);
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
		perror("com set error");
		return -1;
	}
	DBG("set done!\n\r");
	return 0;
}

void getPoint(FILE *fp, Point *pt, Float *scale) {
	Int ptTmp[4][2];
	int i = 0, j;
	for (j = 0; j < 2; pt++, j++) {
		for (i = 0; i < 4; i++)
			fscanf(fp, "%d %d\n", &(ptTmp[i][0]), &(ptTmp[i][1]));

		pt->pt5x = (ptTmp[0][0] + ptTmp[2][0]) / 2;
		pt->pt5y = (ptTmp[0][1] + ptTmp[2][1]) / 2;
		pt->pt6x = (ptTmp[1][0] + ptTmp[3][0]) / 2;
		pt->pt6y = (ptTmp[1][1] + ptTmp[3][1]) / 2;

		pt->k1 = ((float) (ptTmp[1][1] - ptTmp[0][1])) / (ptTmp[1][0]
				- ptTmp[0][0] == 0 ? 1 : ptTmp[1][0] - ptTmp[0][0]);
		pt->k2 = ((float) (ptTmp[3][1] - ptTmp[2][1])) / (ptTmp[3][0]
				- ptTmp[2][0] == 0 ? 1 : ptTmp[3][0] - ptTmp[2][0]);
		pt->k3 = ((float) (pt->pt6y - pt->pt5y)) / (pt->pt6x - pt->pt5x);
		pt->k4 = ((float) (ptTmp[2][1] - ptTmp[0][1])) / (ptTmp[2][0]
				- ptTmp[0][0] == 0 ? 1 : ptTmp[2][0] - ptTmp[0][0]);
		pt->k5 = ((float) (ptTmp[3][1] - ptTmp[1][1])) / (ptTmp[3][0]
				- ptTmp[1][0] == 0 ? 1 : ptTmp[3][0] - ptTmp[1][0]);
		pt->b1 = (ptTmp[1][1] - pt->k1 * ptTmp[1][0]);
		pt->b2 = (ptTmp[3][1] - pt->k2 * ptTmp[3][0]);
		pt->b3 = (pt->pt6y - pt->k3 * pt->pt6x);
		pt->b4 = (ptTmp[2][1] - pt->k4 * ptTmp[2][0]);
		pt->b5 = (ptTmp[3][1] - pt->k5 * ptTmp[3][0]);

		pt->xMax = 0;
		pt->yMax = 0;
		pt->xMin = 99999;
		pt->yMin = 99999;
		for (i = 0; i < 4; i++) {
			if (pt->xMax < ptTmp[i][0])
				pt->xMax = ptTmp[i][0];
			if (pt->yMax < ptTmp[i][1])
				pt->yMax = ptTmp[i][1];
			if (pt->xMin > ptTmp[i][0])
				pt->xMin = ptTmp[i][0];
			if (pt->yMin > ptTmp[i][1])
				pt->yMin = ptTmp[i][1];
		}
	}

	for (i = 0; i < 576; i++)
		fscanf(fp, "%f", scale + i);

}

void getTermTable(Int termTable[4][3], FILE *fp, Int *timeTable[4]) {
	int i = 0, x, y, j;
	for (i = 0; i < 4; i++) {
		fscanf(fp, "%d %d %d\n", &(termTable[i][0]), &(termTable[i][1]),
				&(termTable[i][2]));
		DBG("term%d %d %d %d\n",i,termTable[i][0],termTable[i][1],termTable[i][2]);
		fscanf(fp, "%d %d", &x, &y);
		timeTable[i] = malloc(sizeof(Int) * ((x + 1) * (y + 1) + 2));
		*(timeTable[i]) = x;
		*(timeTable[i] + 1) = y;
		for (j = 2; j < (x + 1) * (y + 1) + 2; j++)
			fscanf(fp, "%d", timeTable[i] + j);
	}
}

Bool videoInit(Int *fds, Int **firstIp, Int **lastIp, Point **ptp,
		Float *scale[4], Char *backBuf[4], Point *pt, Int termTable[4][3],
		Int *timeTable[4]) {
	Char pathtmp[20];
	FILE *fp;
	Int i, imageSize, j;

	imageSize = HEIGHT * WIDTH * 2;

	//serias
	*fds = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);
	if (*fds <= 0) {
		perror("ttyUSB0 error");
		return FALSE;
	}
	if (set_opt(*fds) < 0) {
		ERR("set opt error!");
		return FALSE;
	}

	/* Allocate intermediate buffer (for encoded data) */
	*firstIp = (Int *) Memory_contigAlloc(sizeof(Int), Memory_DEFAULTALIGNMENT);
	*lastIp = (Int *) Memory_contigAlloc(sizeof(Int), Memory_DEFAULTALIGNMENT);
	*ptp = (Point *) Memory_contigAlloc(sizeof(Point) * 1,
			Memory_DEFAULTALIGNMENT);

	for (i = 0; i < 4; i++) {
		scale[i] = (Float *) malloc(sizeof(Float) * 576);
		backBuf[i] = (Char *) Memory_contigAlloc(imageSize * sizeof(char),
				Memory_DEFAULTALIGNMENT);

		memset(backBuf[i], 0x80, imageSize);

		sprintf(pathtmp, "config%d", i + 1);
		if ((fp = fopen(pathtmp, "r+")) == NULL) {
			ERR("Open config err!.\n");
			return FALSE;
		}
		getPoint(fp, pt + i * 2, scale[i]);
		fclose(fp);

		sprintf(pathtmp, "background%d", i + 1);
		if ((fp = fopen(pathtmp, "r+")) == NULL) {
			ERR("Open background err!.\n");
			return FALSE;
		}
		fread(backBuf[i], sizeof(char), imageSize, fp);
		fclose(fp);

	}

	if ((fp = fopen("TermConfig", "r+")) == NULL) {
		ERR("Open TermConfig err!.\n");
		return FALSE;
	}
	getTermTable(termTable, fp, timeTable);
	fclose(fp);

	return TRUE;
}

/******************************************************************************
 * videoThrFxn
 ******************************************************************************/
Void *videoThrFxn(Void *arg) {
	CaptureBufferElement cFlush = { CAPTURE_FLUSH };
	DisplayBufferElement dFlush = { DISPLAY_FLUSH };
	VideoEnv *envp = (VideoEnv *) arg;
	Void *status = THREAD_SUCCESS;
	Engine_Handle hEngine = NULL;
	VIDDEC_Handle hDecode = NULL;
	VIDENC_Handle hEncode = NULL;
	DisplayBufferElement bufferElements[DISPLAY_BUFFERS];
	Char *captureBuffers[CAP_BUFFERS];
	Int frameSize;
	Int imageSize, tmp = 0, tmp2 = 0;
	Int bufIdx, fds = 0, len, i, j;
	DisplayBufferElement de, *dep;
	CaptureBufferElement ce[4], *cep[4];
	Point pt[8], *ptp;
	UInt8 senum[10];
	time_t t_start, t_end;
	Float *scale[4] = { NULL }, max1, max2, maxTmp;
	Int firstI[8], lastI[8], *firstIp, *lastIp;
	Char light1, light2;
	Char *backBuf[4] = { NULL };
	Int *timeTable[4], termTable[4][3], termp = 0, timeLong, calFlag = 0;

	CLEAR(bufferElements);
	CLEAR(captureBuffers);
	signal(SIGINT, handler);

	if (!videoInit(&fds, &firstIp, &lastIp, &ptp, scale, backBuf, pt,
			termTable, timeTable)) {
		ERR("videoInit error!\n");
		cleanup(THREAD_FAILURE)
		;
	}

	//imageSize = 4149248;
	imageSize = HEIGHT * WIDTH * 2;

	/* Reset, load, and start DSP Engine */
	hEngine = Engine_open(ENGINE_NAME, NULL, NULL);

	if (hEngine == NULL) {
		DBG("Failed to open codec engine %s\n", ENGINE_NAME);
		cleanup(THREAD_FAILURE)
		;
	}

	DBG("Codec Engine opened in video thread\n");

	/* Allocate and initialize video encoder on the engine */
	if (videoEncodeAlgCreate(hEngine, &hEncode, envp->imageWidth,
			envp->imageHeight, envp->videoBitRate) == FAILURE) {
		cleanup(THREAD_FAILURE)
		;
	}

	DBG("Video encoder created\n");

#ifdef DEBUG
	for (bufIdx = 0; bufIdx < DISPLAY_BUFFERS; bufIdx++) {

		bufferElements[bufIdx].id = bufIdx;
		bufferElements[bufIdx].height = HEIGHT;
		bufferElements[bufIdx].width = WIDTH;
		bufferElements[bufIdx].virtBuf = (char *) Memory_contigAlloc(imageSize
				* sizeof(char), Memory_DEFAULTALIGNMENT);

		if (bufferElements[bufIdx].virtBuf == NULL) {
			ERR("Failed to allocate contiguous memory block.\n");
			cleanup(THREAD_FAILURE)
			;
		}
		bufferElements[bufIdx].physBuf = Memory_getBufferPhysicalAddress(
				bufferElements[bufIdx].virtBuf, imageSize, 0);

		memset(bufferElements[bufIdx].virtBuf, 0x80, imageSize);

		DBG("Contiguous buffer allocated at physical address 0x%lx\n",
				bufferElements[bufIdx].physBuf);
	}
#endif

	/* Allocate buffers for interacting with the capture thread */
	for (bufIdx = 0; bufIdx < CAP_BUFFERS; bufIdx++) {
		captureBuffers[bufIdx] = (char *) Memory_contigAlloc(imageSize
				* sizeof(char), Memory_DEFAULTALIGNMENT);

		if (captureBuffers[bufIdx] == NULL) {
			ERR("Failed to allocate contiguous memory block.\n");
			cleanup(THREAD_FAILURE)
			;
		}

		DBG("Capture buffer allocated at %#lx physical address %#lx\n",
				(unsigned long) captureBuffers[bufIdx],
				Memory_getBufferPhysicalAddress(captureBuffers[bufIdx],imageSize,0));

		ce[bufIdx].id = bufIdx;
		ce[bufIdx].virtBuf = captureBuffers[bufIdx];
		ce[bufIdx].physBuf = Memory_getBufferPhysicalAddress(
				captureBuffers[bufIdx], imageSize, 0);
		ce[bufIdx].bufSize = imageSize;

		memset(ce[bufIdx].virtBuf, 0x80, imageSize);

		if (Fifo_put(envp->hCaptureInFifo, &ce[bufIdx]) < 0) {
			ERR("Failed to put buffer in output fifo\n");
			cleanup(THREAD_FAILURE)
			;
		}
	}

	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);

	/* Tell the display thread that we are priming */
	de.id = DISPLAY_PRIME;
	if (Fifo_put(envp->hDisplayInFifo, &de) < 0) {
		ERR("Failed to put buffer to output fifo\n");
		cleanup(THREAD_FAILURE)
		;
	}
	/* Prime the display thread */
	for (bufIdx = 0; bufIdx < DISPLAY_BUFFERS; bufIdx++) {
		/* Get a buffer from the capture thread */
		DBG("entre prime\n");
		for (i = 0; i < 4; i++) {
			if (Fifo_get(envp->hCaptureOutFifo, &cep[i]) < 0) {
				ERR("Failed to put buffer in output fifo\n");
				breakLoop(THREAD_FAILURE)
				;
			}
			if (cep[i]->id == CAPTURE_FLUSH) {
				breakLoop(THREAD_SUCCESS)
				;
			}
		}
		/* Encode the buffer */
		/*if (encodeVideoBuffer(hEncode, cep1->virtBuf, cep1->bufSize, encBuf,
		 imageSize, backBuf1, resNum1, pt1, scale1, &frameSize)
		 == FAILURE) {
		 ERR("encodeVideoBuffer FAILLED!!\n");
		 breakLoop(THREAD_FAILURE)
		 ;
		 }*/
		/* Send the buffer back to the capture thread */

		memcpy(backBuf[0], cep[0]->virtBuf, imageSize);
		memcpy(backBuf[1], cep[1]->virtBuf, imageSize);
		memcpy(backBuf[2], cep[2]->virtBuf, imageSize);
		memcpy(backBuf[3], cep[3]->virtBuf, imageSize);
		memcpy(bufferElements[bufIdx].virtBuf, cep[0]->virtBuf, imageSize);

		for (i = 0; i < 4; i++)
			if (Fifo_put(envp->hCaptureInFifo, cep[i]) < 0) {
				ERR("Failed to put buffer in output fifo\n");
				breakLoop(THREAD_FAILURE)
				;
			}

		bufferElements[bufIdx].width = envp->imageWidth;
		bufferElements[bufIdx].height = envp->imageHeight;

		/* Send frame buffer to display thread */
		if (Fifo_put(envp->hDisplayInFifo, &(bufferElements[bufIdx])) < 0) {
			ERR("Failed to put buffer in output fifo\n");
			breakLoop(THREAD_FAILURE)
			;
		}
	}
	/* Release the display thread */
	Rendezvous_meet(envp->hRendezvousPrime);

	DBG("Entering video main loop.\n");
	bufIdx = 0;
	tmp2 = maxGT;
	termp = 0;
	timeLong = termTable[termp][2];
	/*senum[0] = 0xfe;
	 senum[1] = 0x02;
	 senum[2] = 0x02;
	 senum[3] = 0x07;
	 senum[4] = 0x03;
	 senum[5] = 0x03;
	 senum[6] = 0x00;
	 senum[7] = 0x14;
	 senum[8] = timeLong;
	 senum[9] = 0x41;
	 senum[10] = timeLong;
	 write(fds, senum, 11);
	 senum[1] = 0x03;
	 senum[2] = 0x02;
	 senum[3] = 0x07;
	 senum[4] = 0x03;
	 senum[5] = 0x03;
	 senum[6] = 0x00;
	 senum[7] = 0x11;
	 senum[8] = timeLong;
	 senum[9] = 0x11;
	 senum[10] = timeLong;
	 write(fds, senum, 11);*/
	t_start = time((time_t *) NULL);
	senum[0] = 0xfe;
	senum[1] = 0x02;
	senum[2] = 0x03;
	senum[3] = 0x01;
	senum[4] = 0x14;
	write(fds, senum, 5);
	while (!gblGetQuit()) {
		t_end = time((time_t *) NULL);
		//printf("%3.0f", difftime(t_end, t_start));
		if (difftime(t_end, t_start) < timeLong - 2)
			fprintf(stderr, "\b\b\b%3.0f", difftime(t_end, t_start));

		/* Receive a buffer with a displayed frame from the display thread */
		if (Fifo_get(envp->hDisplayOutFifo, &dep) < 0) {
			ERR("video Failed to get buffer from input fifo\n");
			breakLoop(THREAD_FAILURE)
			;
		}

		/* Is the display thread flushing the pipe? */
		if (dep->id == DISPLAY_FLUSH) {
			DBG("video DISPLAY_FLUSH!\n");
			cleanup(THREAD_SUCCESS)
			;
		}

		/* Get a buffer from the capture thread */
		for (i = 0; i < 4; i++) {
			if (Fifo_get(envp->hCaptureOutFifo, &cep[i]) < 0) {
				ERR("Failed to put buffer in output fifo\n");
				cleanup(THREAD_FAILURE)
				;
			}
			if (cep[i]->id == CAPTURE_FLUSH) {
				DBG("video CAPTURE_FLUSH!\n");
				cleanup(THREAD_SUCCESS)
				;
			}
		}
		t_end = time((time_t *) NULL);
		if (difftime(t_end, t_start) > timeLong- 3 && difftime(t_end, t_start)
				< timeLong && !calFlag) {
			max1 = 0;
			max2 = 0;
			senum[0] = 0xfe;
			senum[1] = 0x02;
			senum[2] = 0x03;
			senum[3] = 0x01;
			if (termp % 2 == 0)
				senum[4] = 0x12;
			else senum[4] = 0x21;
			write(fds, senum, 5);
			if (termTable[termp][0] != 0) {
				//DBG("cep1=%d %d\n",(termTable[termp][0] - 1)/ 2+1,(termTable[termp][1] - 1)/ 2+1);
				memcpy(ptp, pt + (termTable[termp][0] - 1), sizeof(Point));
				memcpy(firstIp, firstI + termTable[termp][0] - 1, sizeof(Int));
				memcpy(lastIp, lastI + termTable[termp][0] - 1, sizeof(Int));
				if (encodeVideoBuffer(hEncode, cep[(termTable[termp][0] - 1)
						/ 2]->virtBuf,
						cep[(termTable[termp][0] - 1) / 2]->bufSize, firstIp,
						lastIp, backBuf[(termTable[termp][0] - 1) / 2], ptp)
						== FAILURE) {
					ERR("encode Buffer error!\n");
					breakLoop(THREAD_FAILURE)
					;
				}
				max1 = scale[(termTable[termp][0] - 1) / 2][*lastIp]
						- scale[(termTable[termp][0] - 1) / 2][*firstI];
			}
			if (termTable[termp][1] != 0) {
				//DBG("pt=%d %d %d %d %d %d\n",ptp->xMax,*firstIp,*lastIp,pt[termTable[termp][1] - 1].yMin,pt[termTable[termp][1] - 1].pt5x,pt[termTable[termp][1] - 1].pt5y);
				memcpy(ptp, pt + (termTable[termp][1] - 1), sizeof(Point));
				memcpy(firstIp, firstI + termTable[termp][1] - 1, sizeof(Int));
				memcpy(lastIp, lastI + termTable[termp][1] - 1, sizeof(Int));
				if (encodeVideoBuffer(hEncode, cep[(termTable[termp][1] - 1)
						/ 2]->virtBuf,
						cep[(termTable[termp][1] - 1) / 2]->bufSize, firstIp,
						lastIp, backBuf[(termTable[termp][1] - 1) / 2], ptp)
						== FAILURE) {
					ERR("encode Buffer error!\n");
					breakLoop(THREAD_FAILURE)
					;
				}

				maxTmp = scale[(termTable[termp][1] - 1) / 2][*lastIp]
						- scale[(termTable[termp][1] - 1) / 2][*firstIp];
				//DBG("max1=%f %f\n",max1,maxTmp);
				max1 = maxTmp > max1 ? maxTmp : max1;
			}
			for (i = 1; (!termTable[(termp + i) % 4][0] && !termTable[(termp
					+ i) % 4][1]) && (!termTable[(termp + i) % 4][2]); i++)
				;
			if (termTable[(termp + i) % 4][0] != 0) {
				//DBG("cep2=%d %d\n",(termTable[(termp + i) % 4][0] - 1) / 2+1,(termTable[(termp + i) % 4][1] - 1) / 2+1);

				memcpy(ptp, pt + (termTable[(termp + i) % 4][0] - 1),
						sizeof(Point));
				memcpy(firstIp, firstI + termTable[(termp + i) % 4][0] - 1,
						sizeof(Int));
				memcpy(lastIp, lastI + termTable[(termp + i) % 4][0] - 1,
						sizeof(Int));
				//DBG("pt=%d %d %d %d %d %d\n",ptp->xMax,*firstIp,*lastIp,pt[termTable[(termp + i) % 4][1] - 1].yMin,pt[termTable[(termp + i) % 4][1] - 1].pt5x,pt[termTable[(termp + i) % 4][1] - 1].pt5y);
				if (encodeVideoBuffer(hEncode,
						cep[(termTable[(termp + i) % 4][0] - 1) / 2]->virtBuf,
						cep[(termTable[(termp + i) % 4][0] - 1) / 2]->bufSize,
						firstIp, lastIp, backBuf[(termTable[(termp + i) % 4][0]
								- 1) / 2], ptp) == FAILURE) {
					ERR("encode Buffer error!\n");
					breakLoop(THREAD_FAILURE)
					;
				}
				max2
						= scale[(termTable[(termp + i) % 4][0] - 1) / 2][*lastIp]
								- scale[(termTable[(termp + i) % 4][0] - 1) / 2][*firstIp];
			}
			if (termTable[(termp + i) % 4][1] != 0 && i < 4) {
				memcpy(ptp, pt + (termTable[(termp + i) % 4][1] - 1),
						sizeof(Point));
				memcpy(firstIp, firstI + termTable[(termp + i) % 4][1] - 1,
						sizeof(Int));
				memcpy(lastIp, lastI + termTable[(termp + i) % 4][1] - 1,
						sizeof(Int));
				if (encodeVideoBuffer(hEncode,
						cep[(termTable[(termp + i) % 4][1] - 1) / 2]->virtBuf,
						cep[(termTable[(termp + i) % 4][1] - 1) / 2]->bufSize,
						firstIp, lastIp, backBuf[(termTable[(termp + i) % 4][1]
								- 1) / 2], ptp) == FAILURE) {
					ERR("encode Buffer error!\n");
					breakLoop(THREAD_FAILURE)
					;
				}

				maxTmp
						= scale[(termTable[(termp + i) % 4][1] - 1) / 2][*lastIp]
								- scale[(termTable[(termp + i) % 4][1] - 1) / 2][*firstIp];
				//DBG("max2=%f %f\n",max2,maxTmp);
				max2 = maxTmp > max2 ? maxTmp : max2;
			}
			fprintf(stderr, "\n");
			DBG("max1=%f,max2=%f\n",max1,max2);
			calFlag = 1;
		}
		t_end = time((time_t *) NULL);
		if (difftime(t_end, t_start) >= timeLong) {
			if (termTable[termp][0] == 0 && termTable[termp][1] == 0) {
				timeLong = termTable[termp][2];
			} else {
				if (max1 <= *(timeTable[termp]) && max2 <= *(timeTable[termp]
						+ 1))
					timeLong = *(timeTable[termp] + (Int) max1
							* (*(timeTable[termp] + 1)) + (Int) max2 + 2);//timeTable[termp][(Int) max1][(Int) max2];
				else
					DBG("max error!\n");
				termTable[termp][2] = 0;
			}
			t_start = time((time_t *) NULL);
			//xu jian 2 miao gei huang deng
			DBG("term%d r=%d g=%d\n",termp,0,timeLong);
			senum[0] = 0xfe;
			senum[1] = 0x02;
			senum[2] = 0x03;
			senum[3] = 0x01;
			 if (termp % 2 == 0)
				 senum[4] = 0x41;
			 else senum[4] = 0x14;
			write(fds, senum, 5);
			/*senum[0] = 0xfe;
			 if (termp < 2)
			 senum[1] = 0x02;
			 else
			 senum[1] = 0x03;
			 senum[2] = 0x02;
			 senum[3] = 0x07;
			 senum[4] = 0x03;
			 senum[5] = 0x03;
			 senum[6] = 0x00;
			 if (termp % 2 == 0) {
			 senum[7] = 0x14;
			 senum[8] = timeLong;
			 senum[9] = 0x41;
			 senum[10] = timeLong;
			 } else {
			 senum[7] = 0x14;
			 senum[8] = timeLong;
			 senum[9] = 0x41;
			 senum[10] = timeLong;
			 }
			 write(fds, senum, 11);*/

			i = 0;
			while (i < 4) {
				for (i++; (!termTable[(termp + i) % 4][0] && !termTable[(termp
						+ i) % 4][1]) && (!termTable[(termp + i) % 4][2]) && (i
						< 4); i++)
					;
				if (i < 4) {
					if (termTable[(termp + i) % 4][0] != 0 && termTable[(termp
							+ i) % 4][1] != 0) {
						termTable[(termp + i) % 4][2] += timeLong;
					}
					DBG("term%d r=%d g=%d w=%d\n",(termp+i)%4,timeLong,0,termTable[(termp + i)
							% 4][2]);
					/*if (termp < 2)
					 senum[1] = 0x03;
					 else
					 senum[1] = 0x02;
					 senum[2] = 0x02;
					 senum[3] = 0x07;
					 senum[4] = 0x03;
					 senum[5] = 0x03;
					 senum[6] = 0x00;
					 senum[7] = 0x11;
					 senum[8] = termTable[(termp + i) % 4][2];
					 senum[9] = 0x11;
					 senum[10] = termTable[(termp + i) % 4][2];*/
					write(fds, senum, 11);
				}
			}
			for (i = 1; (!termTable[(termp + i) % 4][0] && !termTable[(termp
					+ i) % 4][1]) && (!termTable[(termp + i) % 4][2]); i++)
				;
			termp = (termp + i) % 4;
			calFlag = 0;
			DBG("Termp = %d    ",termp);
		}

		memcpy(dep->virtBuf, cep[0]->virtBuf, 576 * 704 * 2);

		//DBG("Num1= %f  Num2= %f  \n", scale[0][*lastI[0]] - scale[0][*firstI[0]], scale[1][*lastI[1]] - scale[1][*firstI[1]]);

		/* Send frame buffer to display thread */
		for (i = 0; i < 4; i++) {
			if (Fifo_put(envp->hCaptureInFifo, cep[i]) < 0) {
				ERR("video Failed to put buffer in output fifo\n");
				breakLoop(THREAD_FAILURE)
				;
			}
		}

		/* Send frame buffer to display thread */
		if (Fifo_put(envp->hDisplayInFifo, dep) < 0) {
			ERR("video Failed to put buffer in output fifo\n");
			breakLoop(THREAD_FAILURE)
			;
		}

	}

	cleanup: DBG("clean up video!\n");
	/* Make sure the other threads aren't waiting for init to complete */
	Rendezvous_force(envp->hRendezvousInit);

	/* Make sure the display thread isn't waiting for priming to complete */
	Rendezvous_force(envp->hRendezvousPrime);

	/* Make sure the other threads aren't stuck pausing */
	Pause_off(envp->hPause);
	//gblSetQuit2();

	/* Make sure the video thread isn't stuck in FifoUtil_get() */
	Fifo_put(envp->hDisplayInFifo, &dFlush);

	/* Make sure the capture thread isn't stuck in FifoUtil_get() */
	Fifo_put(envp->hCaptureInFifo, &cFlush);

	/* Meet up with other threads before cleaning up */
	DBG("video meet clean up!\n");
	Rendezvous_meet(envp->hRendezvousCleanup);

	for (bufIdx = 0; bufIdx < CAP_BUFFERS; bufIdx++) {
		if (captureBuffers[bufIdx]) {
			Memory_contigFree(captureBuffers[bufIdx], imageSize);
		}
	}

	/* Clean up the video thread */
	for (bufIdx = 0; bufIdx < DISPLAY_BUFFERS; bufIdx++) {
		if (bufferElements[bufIdx].virtBuf) {
			Memory_contigFree(bufferElements[bufIdx].virtBuf, imageSize);
		}
	}

	if (firstIp) {
		Memory_contigFree(firstIp, sizeof(Int) * 1);
	}
	if (lastIp) {
		Memory_contigFree(lastIp, sizeof(Int) * 1);
	}
	if (ptp) {
		Memory_contigFree(ptp, sizeof(Point) * 1);
	}

	for (i = 0; i < 4; i++) {
		if (scale[i]) {
			free(scale[i]);
		}
		if (backBuf[i]) {
			Memory_contigFree(backBuf[i], imageSize);
		}
		if (timeTable[i])
			free(timeTable[i]);
	}

	if (hEncode) {
		VIDENC_delete(hEncode);
	}

	if (hDecode) {
		VIDDEC_delete(hDecode);
	}

	if (hEngine) {
		Engine_close(hEngine);
	}

	if (fds > 0) {
		close(fds);
	}
	DBG("video return!\n");

	return status;
}
