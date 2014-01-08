/**
 * \file      	ITS_AlgorithmFunc.c 
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.2.3
 * \date      	2014.1.8
 * \brief     	智能交通应用——图像处理算法
 * \update      代码整理头文件            
**/
#include "EE3_common.h"
#include "EE3_ITSAlg.h"
/************************************************************************************/
//外部初始化结构体,内存分配
extern AlgfuncData 		g_algfuncData;
extern VDalgfuncData 	g_VDalgfuncData;
/************************************************************************************/
/**
 * \function 	gaussian
 * \brief    	高斯背景建模算法
 * \			该模块输入相机实时图像，对图像进行背景建模，输出背景图像		
 * \			
**/

void gaussian(Uint8 *b_data, Uint8 *i_data, Float *ww, Float *mean, Float *sd,int yMin, int yMax, int xMin, int xMax) 
{
	Uint32 C = 3, xp, yp, kp, match, movep, k; //高斯分布个数3-5
	Float D = 2.5; //正偏差阀值
	Float std_init = 0.001;
	Float x = 0.08; //学习率0.01-0.5,x从0.01增大，前景更新越快，有模糊点存在，减小效果好
	Float p = x * C, mid_data;
	Uint32 min_a_index = 1; //min_a_index随意设定值
	Float sum_uu, min_a, tmp = 0;

  	for (yp = yMin; yp < yMax; yp++)
  	{
	  for (xp = xMin; xp < xMax; xp++) 
	  {
		  sum_uu = 1.0; //初始化值
		  min_a = *ww;
		  for (kp = 0; kp < C; kp++) {
			  match = 0;
			  movep = yp * WIDTH * 3 + xp * 3 + kp;

			  tmp = fabs((float) i_data[yp * WIDTH + xp] - mean[movep]);
			  sum_uu = tmp * tmp;
			  sd[movep] = D * D * sd[movep];

			  if (sum_uu <= sd[movep]) {
				  match = 1;
				  ww[movep] = (1 - x) * ww[movep] + x;
				  p = x / ww[movep];
				  mean[movep] = (1 - p) * mean[movep]
				  + p * (Float) i_data[yp * WIDTH + xp];
				  mid_data = ((Float) i_data[yp * WIDTH + xp] - mean[movep])
					  * ((Float) i_data[yp * WIDTH + xp] - mean[movep]);
				  sd[movep] = sqrt(
					  (1 - p) * sd[movep] * sd[movep] + p * mid_data);
			  } else {
				  ww[movep] = (1 - x) * ww[movep];
				  mean[movep] = (1 - p) * mean[movep];
			  }

			  if (match == 0) {
				  min_a = ww[movep - kp];
				  min_a_index = 0;

				  for (k = 1; k < C; k++) {
					  if (min_a >= ww[movep - kp + k]) {
						  min_a = ww[movep - kp + k];
						  ww[movep - kp + k] = ww[movep - kp + k - 1];
						  ww[movep - kp + k - 1] = min_a;
					  }
					  min_a_index = k - 1;
				  }
				  mean[movep - kp + min_a_index] = i_data[yp * WIDTH + xp];
				  sd[movep - kp + min_a_index] = std_init;
			  }
		  }
		  *(b_data + yp * WIDTH + xp) = (Uint8) mean[movep];
		  //*(i_data + yp * WIDTH + xp) = (Uint8) mean[movep];
	  }
	}
}

/**
 * \function 	setwwmeansd
 * \brief    	高斯背景建模算法中所需的初始化函数
 * \					
 * \			1、正常
 * \            2、
**/

void setwwmeansd(Float *ww, Float *mean, Float *sd) 
{
	int i,j,k;
	for (i = 0; i < 544; i++)  //初始化
    	for (j = 0; j < 960; j++)
       		for (k = 0; k < 3; k++) {
	        	*(ww +  960 * i * 3 + j * 3 + k) = 0.33333;
	        	*(sd + 960 * i * 3 + j * 3 + k) = 6;
	       	 	*(mean + 960 * i * 3 + j * 3 + k) = 40;
 			}
}

/************************************************************************************/
/**
 * \function 	GrayscaleQD
 * \brief    	队列检测算法
 * \			该模块输入背景图像和前景图像数据，输出车的前后坐标		
 * \			
**/

int GrayscaleQD(Uint8 *back_data, Uint8 *in_data, DpPoint *pt, Uint32 *firstip, Uint32 *lastip)
{

	Uint8 *y_data;
	Lineinfo *foreground;
	float *flag;
	DpPoint *ptP;
	int i, j, n;
	float occupancy;
	int firsti = 0, lasti = 0;
	Bool istart = TRUE;

	//y_data = (Uint8 *) malloc(1920 * 1088); //原为704*576
	//flag = (Float *) malloc(sizeof(Float) * 1088);
	//foreground = (Lineinfo *) malloc(1088 * sizeof(Lineinfo));

	y_data = g_algfuncData.ay_data;
	flag = g_algfuncData.aflag;
	foreground = g_algfuncData.aforeground;

	memset((Uint8 *) y_data, 0, ALG_COL * ALG_ROW); 
	memset((Uint8 *) foreground, 0, ALG_ROW * sizeof(Lineinfo));
	memset((Uint8 *) flag, 0, sizeof(Float) * ALG_ROW);
	ptP = pt; 
	*((int *) firstip) = 0;
	*((int *) lastip) = 0; 

	for (j = ptP->yMin; j < ptP->yMax; j++) {
		for (i = ptP->xMin; i < ptP->xMax; i++) {
			//n表示所有像素点投影到中线上的y值,互相垂直两条线斜率乘积-1
			int n = (int) ((ptP->k3 * ptP->k3 * j + ptP->k3 * i + ptP->b3)
					/ (ptP->k3 * ptP->k3 + 1)); //n表示所有像素点投影到中线上的y值
			if (n >= ALG_ROW || n < 0)
				continue;
			if ((i > (j - ptP->b1) / ptP->k1) && (i < (j - ptP->b2) / ptP->k2)
					&& (j > ptP->k4 * i + ptP->b4) && (j < ptP->k5 * i
					+ ptP->b5)) {
				foreground[n].total_pixel++;
				if (fabs(*(in_data + i + j * ALG_COL) - *(back_data + i + j * ALG_COL))
						> THRESHOLD1)//滤掉前景中噪点
				{
					*(y_data + i + j * ALG_COL) = 255; //设为白点，二值化111
					foreground[n].foreground_count++; //前景值加1
				}
			}
		}


	}

	for (n = 0; n < ALG_ROW; n++) //中线上0值开始扫描
	{
		if (foreground[n].foreground_count != 0) //存在前景
		{
			occupancy = (float)foreground[n].foreground_count/foreground[n].total_pixel;//一行中前景占这一行总像素的比值

			if (occupancy < THRESHOLD2) {
				foreground[n].foreground_count = 0; //设为背景，滤掉人等宽度比例小的事物
			} else {
				*(flag + n) = 1; //foreground flag
			}
		}
	}
	for (j = ptP->pt6y - 1; j >= ptP->pt5y; j--) //图像 img->height-1斑马线开始检测 ,实际中可能要重新设定
	{

		if (flag[j] == 0) //无前景，继续检测
		{
			continue;
		} else if ((flag[j] != 0) && (istart))//有前景，存储第一个前景点所在的y值
		{
			firsti = j;
			istart = FALSE;
		} else if (flag[j] != 0) //有前景，存储前景点所在y值,且存储最后一个前景点所在y值
		{
			lasti = j;
		}
	}
	//*resNump = scalep[lasti] - scalep[firsti];
	*((int *) firstip) = firsti;
	*((int *) lastip) = lasti;


	//free(foreground);
	//free(y_data);
	//free(flag);
	return 1;
}


/********************************************************
****************车辆检测图像处理算法*********************
********************************************************/
void GaussianPram(Float *nMat, int k) 
{
	Int s = (k + 1) / 2, m, n;
	Float bMat = 0.0;
	Float sMat = 0.0;
	Float pram;

	for (m = -k; m < k + 1; m++) {
		pram = ((Float) (m * m)) / (2 * s * s);
		bMat = exp(-pram);
		sMat += bMat;
	}
	for (m = 0; m < 2 * k + 1; m++) {
		n = m - k;
		pram = ((Float) (n * n)) / (2 * s * s);
		bMat = exp(-pram);
		nMat[m] = bMat / sMat;
	}
}

/*************************
车占比,获取前景占线圈的百分比
f_data	out	前景（从高斯模型中减去并做过二值化的前景）

yMin	in	线圈相关参数
yMax	in
xMin	in
xMax	in
*************************/
static float Percent(Uint8 *f_data, int yMin, int yMax, int xMin, int xMax) 
{
	int i, j;
	int fo = 0;
	int total = (yMax - yMin) * (xMax - xMin);
	float percentage = 0.0;

	for (i = yMin; i < yMax; i++) //bmp长
		for (j = xMin; j < xMax; j++) {
			if (f_data[i * WIDTH + j] == 255) {
				fo++;
			}
		}
	percentage = ((Float) fo) * 100 / total;
	if (percentage < PER_THR) {
		percentage = 0.0;
	}
	return percentage;

}

/*************************
腐蚀膨胀算法
data	out	原始图像

yMin	in	线圈相关参数
yMax	in
xMin	in
xMax	in
*************************/
static void erosion(Uint8 *data, int yMin, int yMax, int xMin, int xMax, int mMat) 
{
	int i, j, flag, m, n;
	int center = mMat / 2;
	Uint8 *tmpdata;
	//tmpdata = (UInt8*) malloc(WIDTH * HEIGHT);
	tmpdata = g_VDalgfuncData.tmpdata1;
	memcpy((Uint8*) tmpdata, (Uint8*) data, WIDTH * HEIGHT);
	for (i = yMin; i < yMax; i++) {
		for (j = xMin; j < xMax; j++) {
			flag = 1;
			for (m = i - center; m < i + center + 1; m++) {
				for (n = j - center; n < j + center + 1; n++) {
					//自身及领域中若有一个为255
					//则将该点设为255
					if (tmpdata[i * WIDTH + j] == 0
							|| tmpdata[m * WIDTH + n] == 0) {
						flag = 0;
						break;
					}
				} // for n
				if (flag == 0) {
					break;
				}
			} // for m
			if (flag == 0) {
				data[i * WIDTH + j] = 0;
			} else {
				data[i * WIDTH + j] = 255;
			}
		} // for j
	} // for i
	//free(tmpdata);
}
static void dilation(Uint8 *data, int yMin, int yMax, int xMin, int xMax, int mMat) 
{
	int i, j, flag, m, n;
	int center = mMat / 2;
	Uint8 *tmpdata;
	//tmpdata = (UInt8*) malloc(WIDTH * HEIGHT);
	tmpdata = g_VDalgfuncData.tmpdata2;
	memcpy((Uint8*) tmpdata, (Uint8*) data, WIDTH * HEIGHT);
	for (i = yMin; i < yMax; i++) {
		for (j = xMin; j < xMax; j++) {
			flag = 1;
			for (m = i - center; m < i + center + 1; m++) {
				for (n = j - center; n < j + center + 1; n++) {
					//自身及领域中若有一个为255
					//则将该点设为255
					if (tmpdata[i * WIDTH + j] == 255
							|| tmpdata[m * WIDTH + n] == 255) {
						flag = 0;
						break;
					}
				} // for n
				if (flag == 0) {
					break;
				}
			} // for m
			if (flag == 0) {
				data[i * WIDTH + j] = 255;
			} else {
				data[i * WIDTH + j] = 0;
			}
		} // for j
	} // for i
	//free(tmpdata);
}

/*************************
取得前景函数,前景二值化
in_data	in	当前原始图像
back_data in	高斯背景建模提取的背景

f_data	out 	二值化前景

yMin	in	线圈相关参数
yMax	in
xMin	in
xMax	in
*************************/
static void ForeGround(Uint8 *in_data, Uint8 *back_data, Uint8 *f_data, int yMin,
		int yMax, int xMin, int xMax) 
{
	int i, j;

	for (i = yMin; i < yMax; i++)
		for (j = xMin; j < xMax; j++) {
			if (abs(
					in_data[i * WIDTH + j]
							- back_data[i * WIDTH + j]) > THRESHOLD1)
				f_data[i * WIDTH + j] = 255;
			else
				f_data[i * WIDTH + j] = 0;
		}
	dilation(f_data, yMin, yMax, xMin, xMax, 5); //5*5矩阵
	erosion(f_data, yMin, yMax, xMin, xMax, 5);
	//dilation(f_data, 5);
	//erosion(f_data, 5);

}

/*************************
高斯滤波	对百分比队列做高斯滤波
tempPercent	in	计算出的一组百分比
nMat		in	高斯滤波系数
k		in	帧数

返回值	高斯滤波后的百分比
*************************/
static float GaussianFilter(float *tempPercent, float *nMat, int k) 
{
	float Gaus_percent = 0.0;
	int i;

	for (i = 0; i < 2 * k + 1; i++) {
		Gaus_percent += (float) (nMat[i] * tempPercent[i]);
	}
	return Gaus_percent;
}

/*************************
高斯背景建模，前次和后次比较
pro_imgback	in	计算出的一组百分比
b_data		in	高斯滤波系数

yMin		in	线圈相关参数
yMax		in
xMin		in
xMax		in

diff_point	out	前景背景不同的点数
*************************/
static void AbsDiff(Uint8 *pro_imgback, Uint8 *b_data, int yMin, int yMax, int xMin,
		int xMax, int *diff_point) 
{
	int yp, xp;
	*diff_point = 0;
	for(yp = yMin; yp < yMax; yp++)
		for(xp = xMin; xp < xMax; xp++) {
			if (abs(
					*(b_data + yp * WIDTH + xp)
							- (*(pro_imgback + yp * WIDTH + xp))) > SEG_THR) {
				(*diff_point)++;
			}
		}
}

/*************************
把前次高斯背景建模的结果拷贝一份
src		in	高斯背景原地址
dest		in	高斯背景备份地址

yMin		in	线圈相关参数
yMax		in
xMin		in
xMax		in

*************************/
static void memcpy_area(Uint8 *src, Uint8 *dest, int yMin, int yMax, int xMin,
		int xMax) 
{
	int yp, xp;
	for (yp = yMin; yp < yMax; yp++)
		for (xp = xMin; xp < xMax; xp++) {
			*(dest + yp * WIDTH + xp) = *(src + yp * WIDTH + xp);
		}
}

/************************************************
**
 * \function 	GrayscaleVD
 * \brief    	车辆检测主函数
 * \			该模块输入相机实时图像，对图像进行背景建模，输出背景图像		
 * \
把前次高斯背景建模的结果拷贝一份
in_data		in	原图

模型相关参数
ww
mean
sd

pt				in	线圈参数结构体

ptn				in	虚拟线圈数

carNump			out	虚拟线圈内是否有车，1有车2无车

nMatp			in	高斯滤波的矩阵数

back_data		out	建出来的高斯背景

numofframe       in  背景建模频率

*************************/			
int GrayscaleVD(Uint8 *in_data, float *ww, float *mean, float *sd, DpPoint *ptP,
		 Uint8 *ptn, Uint8 *carNum,	float *nMat, Uint8 *back_data,	Uint8 numofframe) 
{
	static Uint8 count = 0;
	static Uint8 iStart = 0;
	static Uint8 framesTemp = 0;
	float tempPercent[5][QUEUE_LENGTH*2+1];
	int i, k,  gask = QUEUE_LENGTH;
	static int st = 0;

	Bool BG_flag[5]={TRUE,TRUE,TRUE,TRUE,TRUE};

	float percentTotal = 0;
	float percent[2];
	static int count2;

	int diff_point = 0, thr_diff_point;
	Uint8 *f_data = NULL, *pro_imgback = NULL;
		

	f_data = g_VDalgfuncData.f_data;		
	pro_imgback = g_VDalgfuncData.pro_imgback;
	
	if(st == 0)
	{
		memset(f_data, 0, WIDTH * HEIGHT * sizeof(Uint8));
		memset((Uint8 *) tempPercent, 0, sizeof(tempPercent));
		st++;
	}
		
	count = count + 1;
	if(count >= 25)
	{
		count2++;
	}
	if (count == 255)
		count = 25;
	if(count >= 20 || iStart)
	{
		for(i = 0; i < *ptn; i++)
		{
			ForeGround(in_data, back_data, f_data, (ptP + i)->yMin,
					(ptP + i)->yMax, (ptP + i)->xMin, (ptP + i)->xMax);//取得前景
			percent[i] = Percent(f_data, (ptP + i)->yMin, (ptP + i)->yMax,
					(ptP + i)->xMin, (ptP + i)->xMax);
			if (percent[i] < PER_THR) 
			{
				percent[i] = 0.0;
			}
			percentTotal += percent[i];
		}
		if (percentTotal <= BAC_READY) 
		{
			iStart = 1;
		}
		if (iStart) {
			
			framesTemp++;
			if (framesTemp == 255)
				framesTemp = gask + 2;

			//从第11帧开始做滤波
			if (framesTemp <= gask + 1) {
				for (i = 0; i < *ptn; i++) {
					tempPercent[i][framesTemp + gask - 1] = percent[i];
					if (framesTemp == gask + 1) {
						 percent[i] = GaussianFilter(tempPercent[i], nMat, gask);
					}
				}
			} else { //tempPercent元素值往左移一位
				for (i = 0; i < *ptn; i++) {
					for (k = 0; k < (gask * 2); ++k) {
						tempPercent[i][k] = tempPercent[i][k + 1];
					}
					tempPercent[i][gask * 2] = percent[i];

					percent[i] = GaussianFilter(tempPercent[i], nMat, gask);
				}
			}
			//有车时停建模
			for(i = 0; i < *ptn; i++)
			{
				if(*ptn > 5)
					break;
				if(percent[i] > BG_THR)
				{
					BG_flag[i] = FALSE;
				}
				else
				{
					BG_flag[i] = TRUE;
				}
			}
			for (i = 0; i < *ptn; i++) {
				if (percent[i] - PER_THR >= 0) //PER_THR根据实际情况更改调试
				{
					carNum[i] = 1;
				} 
				else 
				{
					carNum[i] = 0;
				}
			}
		}
	}

	if (count2 == numofframe || !(iStart)) { //(*count % 10) == 0 || *count < 20
		memcpy(pro_imgback, back_data, WIDTH * HEIGHT);
		for (i = 0; i < *ptn; i++) {
			//高斯背景建模
			if(BG_flag[i] == TRUE)
			{
				gaussian(back_data, in_data, (Float *) ww, (Float *) mean,
					(Float *) sd, (ptP + i)->yMin, (ptP + i)->yMax,
					(ptP + i)->xMin, (ptP + i)->xMax);
				//某函数
				AbsDiff(pro_imgback, back_data, (ptP + i)->yMin, (ptP + i)->yMax,
					(ptP + i)->xMin, (ptP + i)->xMax, &diff_point);
				thr_diff_point = ((ptP + i)->yMax - (ptP + i)->yMin)
					* ((ptP + i)->xMax - (ptP + i)->xMin) * 16 / 100;

				//不同像素点的总和大于阈值，则使用之前的背景
				if (iStart) {
					if (diff_point > thr_diff_point) {
						memcpy_area(pro_imgback, back_data, (ptP + i)->yMin,
							(ptP + i)->yMax, (ptP + i)->xMin, (ptP + i)->xMax);
					}
				
				}
			}
		}
		count2 = 0;
	}

	return 1;
}

