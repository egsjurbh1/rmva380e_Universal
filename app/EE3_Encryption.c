/**
 * \file      	EE3_Encryption.c
 * \author    	Oliver Wang
 * \version   	1.00
 * \date      	2011.07.13
 * \brief     	相机的加密芯片配置以及控制API。
**/

#include "EE3_Encryption.h"
/**************************************************************************/
Bool bTestFlag=FALSE;
static volatile Bool g_bUserEnpOpLock = FALSE;

//配置区数据缓冲区
AT88SC_Config AT88SC25616C_ConfigZone = {
	0x3b,0xb3,0x11,0x00,0x00,0x00,0x02,0x56,0x56,0x80,0xff,0xff,0xff,0xff,0xff,0xff,//00
	0x34,0x3e,0x72,0xa8,0x50,0x54,0x56,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//10
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//20
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//30
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//40
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//50
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//60
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//70
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//80
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//90
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//a0
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//b0
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//c0
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,//d0
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x17,0xc3,0x3a,0xff,0xff,0xff,0xff,//e0
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff //f0
	};
/**************************************************************************/
//////////////////////////////////////////////////////////////////////
// function:	Encryption_Set_SDA
// 
// Descript:	设置SDA为高电平
// Note:		D1=>SDA: 0=HZ, 1=Low
//////////////////////////////////////////////////////////////////////
static void Encryption_Set_SDA()
{
	g_ui32FpgaI2cWrReg &= 0xfffffffd;	//SDA=1
	*(volatile Uint32*)FPGA_PUBLIC_I2C_WR_REG = g_ui32FpgaI2cWrReg;
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Clear_SDA
// 
// Descript:	设置SDA为低电平
// Note:		D1=>SDA: 0=HZ, 1=Low
//////////////////////////////////////////////////////////////////////
static void Encryption_Clear_SDA()
{
	g_ui32FpgaI2cWrReg |= 0x00000002;	//SDA=0
	*(volatile Uint32*)FPGA_PUBLIC_I2C_WR_REG = g_ui32FpgaI2cWrReg;
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Clear_SDA
// 
// Descript:	读取SDA逻辑状态
// Note:		D0<=SDA
//////////////////////////////////////////////////////////////////////
static Uint32 Encryption_Read_SDA()
{
	Uint32 temp;

	temp = (*(volatile Uint32*)FPGA_PUBLIC_I2C_RD_REG)&0x00000001;

	return temp;
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Set_SCL
// 
// Descript:	设置SCL为高电平
// Note:		D0=>SCL: 0=HZ, 1=Low
//////////////////////////////////////////////////////////////////////
static void Encryption_Set_SCL()
{
	g_ui32FpgaI2cWrReg &= 0xfffffffe;	//SCL=1
	*(volatile Uint32*)FPGA_PUBLIC_I2C_WR_REG = g_ui32FpgaI2cWrReg;
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Clear_SCL
// 
// Descript:	设置SCL为低电平
// Note:		D0=>SCL: 0=HZ, 1=Low
//////////////////////////////////////////////////////////////////////
static void Encryption_Clear_SCL()
{
	g_ui32FpgaI2cWrReg |= 0x00000001;	//SCL=0
	*(volatile Uint32*)FPGA_PUBLIC_I2C_WR_REG = g_ui32FpgaI2cWrReg;
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_SMSTART
// 
// Descript:	启动I2C
// Note:		与一般的I2C芯片不同，启动AT88SCXXC之前，必须先发送8个CLK
//////////////////////////////////////////////////////////////////////
static void Encryption_SMSTART()
{
	Uint32 i;

	Encryption_Set_SDA();
	Roseek_Waitusec(1);

	for( i=0; i<8; i++)
	{
		//先打出8个CLK
		Encryption_Set_SCL();
		Roseek_Waitusec(1);
		Encryption_Clear_SCL();
		Roseek_Waitusec(1);
	}

	Encryption_Set_SDA();
	Roseek_Waitusec(3);
	Encryption_Set_SCL();
	Roseek_Waitusec(3);
	Encryption_Clear_SDA();
	Roseek_Waitusec(3);
	Encryption_Clear_SCL();
	Roseek_Waitusec(3);

	Roseek_Waitusec(1000);
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_SMSTOP
// 
// Descript:	停止I2C
// Note:		
//////////////////////////////////////////////////////////////////////
static void Encryption_SMSTOP()
{
	Encryption_Clear_SCL();
	Roseek_Waitusec(3);
	Encryption_Clear_SDA();
	Roseek_Waitusec(3);
	Encryption_Set_SCL();
	Roseek_Waitusec(3);
	Encryption_Set_SDA();
	Roseek_Waitusec(3);
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_AckPolling
// 
// Descript:	检查所发送的命令是否已被执行
// Note:		只有当AckPolling_return为1时，该程序才退出
//////////////////////////////////////////////////////////////////////
static void Encryption_AckPolling()
{
	Uint8 AckPolling_return;
	Uint8 temp;
	Uint8 bitcounter;

	Encryption_SMSTART();
	temp=0xb6;
	do	//cmd send
	{
		for(bitcounter=0;bitcounter<8;bitcounter++)
		{
			if ((temp&0x80)==0x80)
				Encryption_Set_SDA();
			else
				{Encryption_Clear_SDA();}

				Roseek_Waitusec(3);
				Encryption_Set_SCL();
				Roseek_Waitusec(2);
				Encryption_Clear_SCL();
				Roseek_Waitusec(2);
				temp=temp<<1;
		}
		Encryption_Set_SDA();
		Roseek_Waitusec(2);
		Encryption_Set_SCL();
		Roseek_Waitusec(2);
		if(Encryption_Read_SDA()==0) {Encryption_Clear_SCL();AckPolling_return=1;}
		else {Encryption_Clear_SCL();Encryption_SMSTART();temp=0xb6;AckPolling_return=0;}
	}while(AckPolling_return==0);

	Encryption_SMSTOP();
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Read
// 
// Descript:	对at88scxx读操作函数
// Note:		本程序中以Encryption_Read(rwdata)的形式调用
//				入口:	rwdata数组
//				出口:	rwdata数组
//				rd数组长度可改
//				与一般的I2C芯片不同，启动AT88SCXXC之前，必须先发送8个CLK
//				cmd_send_counter发送命令次数,超出8次,本函数退出,并认为I2C线路上无器件
//////////////////////////////////////////////////////////////////////
static void Encryption_Read(Uint8 rd[0x20])
{
	Uint8 bitcounter;
	Uint8 temp;
	Uint8 cmd_send_counter;
	Uint8 bytecounter;

	temp=rd[0];
	bytecounter=0;
	cmd_send_counter=0;
	Encryption_SMSTART();

	do	//cmd send
	{
		for(bitcounter=0;bitcounter<8;bitcounter++)
		{
			if ((temp&0x80)==0x80)	Encryption_Set_SDA();
			else {Encryption_Clear_SDA();}

			Roseek_Waitusec(2);
			Encryption_Set_SCL();
			Roseek_Waitusec(2);
			Encryption_Clear_SCL();

			temp=temp<<1;
		}

		Encryption_Set_SDA();
		Roseek_Waitusec(3);
		Encryption_Set_SCL();
		Roseek_Waitusec(3);

		if(Encryption_Read_SDA()==0)
		{Encryption_Clear_SCL();bytecounter++;temp=rd[bytecounter];}
		else
		{Encryption_Clear_SCL();Encryption_SMSTART();temp=rd[0];bytecounter=0;cmd_send_counter++;}

		if(cmd_send_counter>8){bytecounter=4;}

	}while(bytecounter<4);

	Encryption_Set_SDA();
	for(bytecounter=0;bytecounter<rd[3];bytecounter++)
	{
		for(bitcounter=0;bitcounter<8;bitcounter++)
		{
			Encryption_Set_SCL();
			Roseek_Waitusec(2);

			if (Encryption_Read_SDA()==1) {temp=temp|0x01;}
			else{temp=(temp&0xfe);}
			if(bitcounter<7){temp=temp<<1;}
			else{Roseek_Waitusec(1);}

			Encryption_Clear_SCL();
			Roseek_Waitusec(2);
		}

		Encryption_Clear_SDA();
		Roseek_Waitusec(2);
		Encryption_Set_SCL();
		Roseek_Waitusec(2);
		Encryption_Clear_SCL();
		Roseek_Waitusec(2);
		Encryption_Set_SDA();
		Roseek_Waitusec(1);

		rd[bytecounter+4]=temp;
	}

	Encryption_SMSTOP();
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_Write
// 
// Descript:	对at88scxx写操作函数
// Note:		本程序中以Encryption_Write(rwdata)的形式调用
//				入口:	rwdata数组
//				出口:	rwdata数组
//				rd数组长度可改
//				与一的I2C芯片不同，启动AT88SCXXC之前，必须先发送8个CLK
//				cmd_send_counter发送命令次数,超出8次,本函数退出,并认为I2C线路上无器件
//////////////////////////////////////////////////////////////////////
static void Encryption_Write(Uint8 SDATA[0x14])
{
	Uint8 bitcounter;
	Uint8 temp;
	Uint8 bytecounter;
	Uint8 cmd_send_counter;

	temp=SDATA[0];
	bytecounter=0;
	cmd_send_counter=0;
	Encryption_SMSTART();

	do	//cmd send
	{
		for(bitcounter=0;bitcounter<8;bitcounter++)
		{
			if ((temp&0x80)==0x80)
			Encryption_Set_SDA();
			else
			{Encryption_Clear_SDA();}

			Roseek_Waitusec(3);
			Encryption_Set_SCL();
			Roseek_Waitusec(2);
			Encryption_Clear_SCL();
			Roseek_Waitusec(2);

			temp=temp<<1;
		}

		Encryption_Set_SDA();
		Roseek_Waitusec(2);
		Encryption_Set_SCL();
		Roseek_Waitusec(2);

		if(Encryption_Read_SDA()==0) {Encryption_Clear_SCL();bytecounter++;temp=SDATA[bytecounter];}
		else {Encryption_Clear_SCL();Encryption_SMSTART();temp=SDATA[0];bytecounter=0;cmd_send_counter++;}

		if(cmd_send_counter>8) {bytecounter=SDATA[3]+4;}

	}while(bytecounter<SDATA[3]+4);

	Encryption_SMSTOP();
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_GPA_CLOCK
// 
// Descript:	GPA函数
// Note:		入口:	data_in和上次计算后保存于GPA[]中的数据
//				出口:	GPA[]中的数据
//				参数:	times:GPA函数循环计算的次数
//				此函数是DES加密算法的一个变种，该算法是可逆的，故既可以用
//				该算法实现加密过程，也可以实现解密过程。
//////////////////////////////////////////////////////////////////////
static void Encryption_GPA_CLOCK(Uint8 Datain,Uint8 times)
{
	Uint8 t;
	Uint8 d;
	Uint8 din_gpa;	//GPA[2]
	Uint8 Ri;		//GPA[3]
	Uint8 R_Sum;	//GPA[13]
	Uint8 Si;		//GPA[4]
	Uint8 S_Sum;	//GPA[21]
	Uint8 Ti;		//GPA[5]
	Uint8 T_Sum;	//GPA[27]

	for(t=0x00;t<times;t++)
	{
		asm("NOP");
		din_gpa=Datain ^ GPA[0];
		Ri= din_gpa&0x1f;
		Si= ((din_gpa&0x0f)<<3)+((din_gpa&0xe0)>>5);
		Ti=(din_gpa&0xf8)>>3;

		//r元素
		if(((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4))>31) {R_Sum=((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4))-31;}
		else {R_Sum=((GPA[4])+((GPA[1]&0x0f)<<1)+((GPA[1]&0x10)>>4));}
		GPA[1]=GPA[2];
		GPA[2]=GPA[3];
		GPA[3]=GPA[4];
		GPA[4]=GPA[5]^Ri;
		GPA[5]=GPA[6];
		GPA[6]=GPA[7];
		GPA[7]=R_Sum;

		//s元素
		if ((GPA[9]+((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) )>127) {S_Sum=( (GPA[9]) + ((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) )-127;}
		else {S_Sum= (GPA[9]) + ((GPA[8]&0x3f)<<1)+((GPA[8]&0x40)>>6) ;}
		GPA[8]=GPA[9];
		GPA[9]=Si^GPA[10];
		GPA[10]=GPA[11];
		GPA[11]=GPA[12];
		GPA[12]=GPA[13];
		GPA[13]=GPA[14];
		GPA[14]=S_Sum;

		//t元素
		if ((GPA[15]+GPA[17])> 31) {T_Sum=GPA[15]+GPA[17]-31;}
		else {T_Sum=GPA[15]+GPA[17];}
		GPA[15]=GPA[16];
		GPA[16]=GPA[17];
		GPA[17]=GPA[18]^Ti;
		GPA[18]=GPA[19];
		GPA[19]=T_Sum;

		if((GPA[14]&0x01)==0) {d=((GPA[7]^GPA[3])&0x01);}
		else {d=((GPA[19]^GPA[16])&0x01);}

		if((GPA[14]&0x02)==0) {d=d+((GPA[7]^GPA[3])&0x02);}
		else {d=d+((GPA[19]^GPA[16])&0x02);}

		if((GPA[14]&0x04)==0) {d=d+((GPA[7]^GPA[3])&0x04);}
		else {d=d+((GPA[19]^GPA[16])&0x04);}

		if((GPA[14]&0x08)==0) {d=d+((GPA[7]^GPA[3])&0x08);}
		else {d=d+((GPA[19]^GPA[16])&0x08);}

		GPA[0]= ( (((GPA[0])&0x0f)<<4) +d);	//GPA出口
	}
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_AUTHENTICATION
// 
// Descript:	双向加密认证函数
// Note:		过程：
//				1.从器件读出Ci,产生随机数Q0,计算密钥;计算出Q1,将Q0、Q1发送给器件,
//				  计算出新的Ci,SK
//				2.用计算出的SK替代Ci，重复1过程，实现加密认证
//				AAC：认证错误记数器
//				AAC!=0xff表示双向加密认证未通过或无器件
//////////////////////////////////////////////////////////////////////
static Uint8 Encryption_AUTHENTICATION(Uint8 GC_select)
{
	Uint8 SK[8];			//会话密钥
	Uint8 Q_CH[0x14];		//发送14H字节载体
	Uint8 j;

	//GPA单元清零
	for(j=0;j<=19;j++)
	{GPA[j]=0x00;}

	//从器件读出Ci
	Q_CH[0]=0xb6;
	Q_CH[1]=0x00;
	Q_CH[2]=GC_select<<4;
	Q_CH[2]=Q_CH[2]+0x50;
	Q_CH[3]=0x08;
	Encryption_Read(Q_CH);

	//复制Ci在Q_CH[12~19]中
	//赋值随机数Q0=0xaaaaaaaaaaaaaaaa在Q_CH[4~12]中
	for(j=0;j<8;j++)
	{
		Q_CH[j+12]=Q_CH[j+4];
		Q_CH[j+4]=0xAA;
	}

	//计算Q1：第一次计算
	for(j=0;j<4;j++)
	{
		Encryption_GPA_CLOCK(Q_CH[12+2*j],0x03);
		Encryption_GPA_CLOCK(Q_CH[12+2*j+1],0x03);
		Encryption_GPA_CLOCK(Q_CH[4+j],0x01);
	}

	//取本地Gc在Q_CH[12~19]中
	if ((GC_select&0x0f)==0x00)
		for(j=0;j<8;j++)
		{Q_CH[12+j]=GC0[j];}
	else if ((GC_select&0x0f)==0x01)
		for(j=0;j<8;j++)
		{Q_CH[12+j]=GC1[j];}
	else if ((GC_select&0x0f)==0x02)
		for(j=0;j<8;j++)
		{Q_CH[12+j]=GC2[j];}
	else
		for(j=0;j<8;j++)
		{Q_CH[12+j]=GC3[j];}

	//计算Q1：第二次计算，结果Q1在Q_CH[12~19]中
	for(j=0;j<4;j++)
	{
		Encryption_GPA_CLOCK(Q_CH[12+2*j],0x03);
		Encryption_GPA_CLOCK(Q_CH[12+2*j+1],0x03);
		Encryption_GPA_CLOCK(Q_CH[8+j],0x01);
	}
	Encryption_GPA_CLOCK(0x00,0x06);
	Q_CH[12]=GPA[0];
	for(j=1;j<8;j++)
	{
		Encryption_GPA_CLOCK(0x00,0x07);
		Q_CH[12+j]=GPA[0];
	}

	//将Q0、Q1发送给器件：Q0=Q_CH[4~11], Q1=Q_CH[12~19]
	Q_CH[0]=0xb8;
	Q_CH[2]=0x00;
	Q_CH[3]=0x10;		//Q0,Q1各8字节
	Q_CH[1]=GC_select;	//发送Gc的组号，并要求认证
	Encryption_Write(Q_CH);
	Encryption_AckPolling();

	//模拟芯片内的计算过程，计算出与芯片内相同的会话密钥SK，以保持同步
	Q_CH[12]=0xff;		//该字节是AAC（认证错误计数器），必须是0xff
	for(j=1;j<8;j++)	//其后7字节是Ci
	{
		Encryption_GPA_CLOCK(0x00,0x02);
		Q_CH[12+j]=GPA[0];
	}
	for(j=0;j<8;j++)
	{
		Encryption_GPA_CLOCK(0x00,0x02);
		SK[j]=GPA[0];
	}

	Encryption_GPA_CLOCK(0x00,0x03);

	////////以会话密钥再认证一次，实现加密认证////////
	////////方法：用计算出的SK替代Ci,重复以上认证过程
	////////当前：Q_CH[4~11]中是Q0
	////////      Q_CH[12~19]中是新的Ci
	////////      SK[0~8]中是新的SK

	//GPA单元清零
	for(j=0;j<=19;j++)
	{GPA[j]=0x00;}

	//计算Q1：第一次计算
	for(j=0;j<4;j++)
	{
		Encryption_GPA_CLOCK(Q_CH[12+2*j],0x03);
		Encryption_GPA_CLOCK(Q_CH[12+2*j+1],0x03);
		Encryption_GPA_CLOCK(Q_CH[4+j],0x01);
	}

	//计算Q1：第二次计算，结果Q1在Q_CH[12~19]中
	for(j=0;j<4;j++)
	{
		Encryption_GPA_CLOCK(SK[2*j],0x03);
		Encryption_GPA_CLOCK(SK[2*j+1],0x03);
		Encryption_GPA_CLOCK(Q_CH[8+j],0x01);
	}
	Encryption_GPA_CLOCK(0x00,0x06);
	Q_CH[12]=GPA[0];
	for(j=1;j<8;j++)
	{
		Encryption_GPA_CLOCK(0x00,0x07);
		Q_CH[12+j]=GPA[0];
	}

	//再次将Q0、Q1发送给器件：Q0=Q_CH[4~11], Q1=Q_CH[12~19]
	Q_CH[0]=0xb8;
	Q_CH[1]=GC_select+0x10;	//发送Gc的组号，并要求加密认证
	Q_CH[2]=0x00;
	Q_CH[3]=0x10;			//Q0,Q1各8字节
	Encryption_Write(Q_CH);
	Encryption_AckPolling();

	//模拟芯片内的计算过程，计算出与芯片内相同的会话密钥SK，以保持同步
	Q_CH[12]=0xFF;		//该字节是AAC（认证错误计数器），必须是0xff
	for(j=1;j<8;j++)	//其后7字节是CIx
	{
		Encryption_GPA_CLOCK(0x00,0x02);
		Q_CH[12+j]=GPA[0];
	}
	for(j=0;j<8;j++)
	{
		Encryption_GPA_CLOCK(0x00,0x02);
		SK[j]=GPA[0];
	}

	Encryption_GPA_CLOCK(0x00,0x03);

	//读取配置区中相应组的AAC,查看认证是否正确
	//芯片内部认证错误后，AAC就不是0xff
	Q_CH[0]=0xb6;
	Q_CH[1]=0x00;
	Q_CH[2]=GC_select<<4;	//通过Gc的组号，计算出ACC地址
	Q_CH[2]=Q_CH[2]+0x50;
	Q_CH[3]=0x01;
	Encryption_Read(Q_CH);
	Encryption_AckPolling();

	//GPA中数据进一步处理并被保留，用于传输数据时的加解密过程
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(Q_CH[2],0x01);
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(0x01,0x01);
	Encryption_GPA_CLOCK(0xff,0x01);
	Encryption_GPA_CLOCK(0x00,0x05);

	return Q_CH[4];
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_verify_write_password
// 
// Descript:	校验写密码组主函数，正确校验了写密码后开放读写
// Note:		pw_select：密码套数选择
//				PAC:密码校验错误记数器.PAC!=0xff表示认证未通过或无器件
//////////////////////////////////////////////////////////////////////
static Uint8 Encryption_verify_write_password(Uint8 pw_select)
{
	Uint8 j;
	Uint8 pw[7];

	//查找指定的PW
	if ((pw_select&0x0f)==0x00)
		{for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE0[j];}}
	else if ((pw_select&0x0f)==0x01)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE1[j];}
	else if ((pw_select&0x0f)==0x02)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE2[j];}
	else if ((pw_select&0x0f)==0x03)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE3[j];}
	else if ((pw_select&0x0f)==0x04)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE4[j];}
	else if ((pw_select&0x0f)==0x05)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE5[j];}
	else if ((pw_select&0x0f)==0x06)
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE6[j];}
	else
		for(j=0;j<3;j++)
		{pw[4+j]=PW_WRITE7[j];}

	//加密明文的PW
	for(j=0;j<3;j++)
	{
		Encryption_GPA_CLOCK(pw[4+j],0x05);
		pw[4+j]=GPA[0];
	}

	//发送密文PW给芯片，并要求核对
	pw[0]=0xba;
	pw[1]=pw_select&0x0f;	//发送PW的组号
	pw[2]=0x00;
	pw[3]=0x03;
	Encryption_Write(pw);
	Encryption_AckPolling();

	Roseek_Waitusec(5000);

	//读取配置区中的PAC,查看认证是否正确
	pw[0]=0xb6;
	pw[1]=0x00;
	pw[2]=pw_select&0x0f;
	pw[2]=pw[2]*0x08;
	pw[2]=pw[2]+0xb0;
	pw[3]=0x01;
	Encryption_Read(pw);
	Encryption_AckPolling();

	//解密读出的PAC密文
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(pw[2],0x01);
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(0x01,0x01);
	pw[4]=pw[4]^GPA[0];
	Encryption_GPA_CLOCK(pw[4],0x01);
	Encryption_GPA_CLOCK(0x00,0x05);

	return pw[4];
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_set_user_zone
// 
// Descript:	选择用户区
// Note:		zone：用户区序号
//////////////////////////////////////////////////////////////////////
static void Encryption_set_user_zone(Uint8 zone)
{
	Uint8 zon[4];

	Encryption_GPA_CLOCK(zone,0x01);	zon[0]=0xb4;
	zon[1]=0x03;
	zon[2]=zone;
	zon[3]=0x00;
	Encryption_Write(zon);
	Encryption_AckPolling();
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_encrypto_data
// 
// Descript:	对明文数据进行加密
// Note:		encryptodatanumber加密数据的个数
//				需加密的明文数据存放在 rwdata[4]至rwdata[0x13]
//////////////////////////////////////////////////////////////////////
static void Encryption_encrypto_data(Uint8 encryptodatanumber)
{
	Uint8 j;
	Uint8 temp;

	for(j=0;j<encryptodatanumber;j++)
	{
		Encryption_GPA_CLOCK(0x00,0x05);
		temp=rwdata[4+j];
		rwdata[4+j]=rwdata[4+j]^GPA[0];
		Encryption_GPA_CLOCK(temp,0x01);
	 }
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_send_checksum
// 
// Descript:	发送校验和到器件(以密文形式写入数据后,需发送校验和)
// Note:		
//////////////////////////////////////////////////////////////////////
static void Encryption_send_checksum()
{
	Uint8 dat6[6];
	dat6[0]=0xb4;
	dat6[1]=0x02;
	dat6[2]=0x00;
	dat6[3]=0x02;

	Encryption_GPA_CLOCK(0x00,0x0f);
	dat6[4]=GPA[0];
	Encryption_GPA_CLOCK(0x00,0x05);
	dat6[5]=GPA[0];

	Encryption_Write(dat6);
	Encryption_AckPolling();
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_read_user_zone
// 
// Descript:	读出用户区密文数据，并解出明文数据
// Note:		rd_high_addr 用户区高字节地址
//				rd_low_addr  用户区低字节地址
//				rd_number    读取密文数据及解出明文数据的字节数，0<rd_number<=128
//				解密出的明文数据存放在rwdata[0x04]起始的存储区
//////////////////////////////////////////////////////////////////////
static void Encryption_read_user_zone(Uint8 rd_high_addr,Uint8 rd_low_addr,Uint16 rd_number)
{
	Uint16 j;

	rwdata[0]=0xb2;
	rwdata[1]=rd_high_addr;
	rwdata[2]=rd_low_addr;
	rwdata[3]=rd_number;
	Encryption_Read(rwdata);
	Encryption_AckPolling();

	//初始化解密
	Encryption_GPA_CLOCK(0x00,0x05);		//当使用AT88SC6416/12816/25616时添加;
	Encryption_GPA_CLOCK(rd_high_addr,0x01);//当使用AT88SC6416/12816/25616时添加;
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(rd_low_addr,0x01);
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(rd_number,0x01);

	for(j=0;j<rd_number;j++)
	{
		rwdata[4+j]=rwdata[4+j]^GPA[0];
		Encryption_GPA_CLOCK(rwdata[4+j],0x01);
		Encryption_GPA_CLOCK(0x00,0x05);
	}
}

//////////////////////////////////////////////////////////////////////
// function:	Encryption_write_user_zone
// 
// Descript:	加密明文并写到器件
// Note:		wr_high_addr	用户区高字节地址
//				wr_low_addr		用户区低字节地址
//				wr_number		写入数据的个数，0<wr_number<=128
//				将存放在 rwdata[4]起始的明文数据写入器件
//				每次写入不能超过128字节
//////////////////////////////////////////////////////////////////////
static void Encryption_write_user_zone(Uint8 wr_high_addr,Uint8 wr_low_addr,Uint8 wr_number)
{
	//初始化加密
	Encryption_GPA_CLOCK(0x00,0x05);		//当使用AT88SC6416/12816/25616时添加;
	Encryption_GPA_CLOCK(wr_high_addr,0x01);//当使用AT88SC6416/12816/25616时添加;
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(wr_low_addr,0x01);
	Encryption_GPA_CLOCK(0x00,0x05);
	Encryption_GPA_CLOCK(wr_number,0x01);

	rwdata[0]=0xb0;
	rwdata[1]=wr_high_addr;
	rwdata[2]=wr_low_addr;
	rwdata[3]=wr_number;
	Encryption_encrypto_data(wr_number);

	Encryption_Write(rwdata);
	Roseek_Waitusec(5000);					//AT88SCXX写入时间最大5mS

	Encryption_send_checksum();
	Encryption_AckPolling();
}

//////////////////////////////////////////////////////////////////////
// function:	Roseek_Encryption_Read_Configuration
// 
// Descript:	Read configuration zone
// Note:		
//////////////////////////////////////////////////////////////////////
Bool Roseek_Encryption_Read_Configuration( Uint8 ui8SecureCode1of3, 
											  Uint8 ui8SecureCode2of3, 
											  Uint8 ui8SecureCode3of3
											)
{
	Uint8 i;

	if( g_bUserEnpOpLock ){
		return FALSE;
	}
	g_bUserEnpOpLock = TRUE;

	//故意进行错误的认证和校验写密码，使该芯片退出密文通讯方式
	//以便于用明文方式读写配置区
	if( Encryption_AAC==0xff)
		Encryption_AAC=Encryption_AUTHENTICATION(0);

	if( Encryption_PAC==0xff)
		Encryption_PAC=Encryption_verify_write_password(0);

	//////UNLOCK CONFIGURATION ZONE//////
	rwdata[0]=0xba;
	rwdata[1]=0x07;
	rwdata[2]=0x00;
	rwdata[3]=0x03;
	rwdata[4]=ui8SecureCode1of3;
	rwdata[5]=ui8SecureCode2of3;
	rwdata[6]=ui8SecureCode3of3;
	Encryption_Write(rwdata);

	//read entire configuration zone
	rwdata[0]=0xb6;
	rwdata[1]=0x00;
	rwdata[2]=0x00;
	rwdata[3]=0xf0;	//N=240
	Encryption_Read(rwdata);

	//copy data
	for( i=0; i<240; i++)
		*(((Uint8*)&AT88SC25616C_ConfigZone)+i) = rwdata[i+4];

	g_bUserEnpOpLock = FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// function:	Roseek_Encryption_Initialization
// 
// Descript:	初始化芯片
// Note:		对于新的AT88SCXX芯片，需要初始化之后才能应用
//////////////////////////////////////////////////////////////////////
Bool Roseek_Encryption_Initialization( Uint8 ui8SecureCode1of3, 
										  Uint8 ui8SecureCode2of3, 
										  Uint8 ui8SecureCode3of3
										)
{
	Uint8 i,j,k;
	Uint8 * pnt = (Uint8*)&AT88SC25616C_ConfigZone;
	Uint8 tmp[256];

	if( g_bUserEnpOpLock ){
		return FALSE;
	}
	g_bUserEnpOpLock = TRUE;

	rwdata[0]=0xb6;
	rwdata[1]=0x01;
	rwdata[2]=0x00;
	rwdata[3]=0x01;
	Encryption_Read(rwdata);

	//本程序只供操作保险丝未熔断的芯片（fuse=0x07）
	if( (rwdata[4]&0x0f) != 0x07){
		g_bUserEnpOpLock = FALSE;
		return FALSE;
	}

	//故意进行错误的认证和校验写密码，使该芯片退出密文通讯方式
	//以便于用明文方式读写配置区
	if( Encryption_AAC==0xff)
		Encryption_AAC=Encryption_AUTHENTICATION(0);
	if( Encryption_PAC==0xff)
		Encryption_PAC=Encryption_verify_write_password(0);

	//恢复所有错误计数器
	*(pnt+0x50)=0xff;		//ACC0
	*(pnt+0x60)=0xff;		//ACC1
	*(pnt+0x70)=0xff;		//ACC2
	*(pnt+0x80)=0xff;		//ACC3
	*(pnt+0xb0)=0xff;		//PAC_Write0
	*(pnt+0xb8)=0xff;		//PAC_Write1
	*(pnt+0xc0)=0xff;		//PAC_Write2
	*(pnt+0xc8)=0xff;		//PAC_Write3
	*(pnt+0xd0)=0xff;		//PAC_Write4
	*(pnt+0xd8)=0xff;		//PAC_Write5
	*(pnt+0xe0)=0xff;		//PAC_Write6
	*(pnt+0xe8)=0xff;		//PAC_Write7
	*(pnt+0xb4)=0xff;		//PAC_Read0
	*(pnt+0xbc)=0xff;		//PAC_Read1
	*(pnt+0xc4)=0xff;		//PAC_Read2
	*(pnt+0xcc)=0xff;		//PAC_Read3
	*(pnt+0xd4)=0xff;		//PAC_Read4
	*(pnt+0xdc)=0xff;		//PAC_Read5
	*(pnt+0xe4)=0xff;		//PAC_Read6
	*(pnt+0xec)=0xff;		//PAC_Read7

	//////UNLOCK CONFIGURATION ZONE//////
	rwdata[0]=0xba;
	rwdata[1]=0x07;
	rwdata[2]=0x00;
	rwdata[3]=0x03;
	rwdata[4]=ui8SecureCode1of3;
	rwdata[5]=ui8SecureCode2of3;
	rwdata[6]=ui8SecureCode3of3;
	Encryption_Write(rwdata);
	Roseek_Waitusec(5000);	//EEPROM Write Cycle Time Max 5 mS

	//////PERFORM WRITING//////
	j=0x19;
	pnt += 0x19;

	//address from 0x19 to 0x1f
	rwdata[0]=0xb4;
	rwdata[1]=0x00;
	rwdata[2]=j;
	rwdata[3]=0x07;
	for( k=0;k<0x07;k++)
	{
		rwdata[4+k]=*pnt;
		pnt++;
	}
	Encryption_Write(rwdata);
	Roseek_Waitusec(5000);		//EEPROM Write Cycle Time Max 5 mS
	j+=0x07;

	//address from 0x20 to 0xef
	for(i=0;i<13;i++)
	{
		rwdata[0]=0xb4;
		rwdata[1]=0x00;
		rwdata[2]=j;
		rwdata[3]=0x10;		//16 bytes per page for MIN device: AT88SC0104C
		for( k=0;k<16;k++)
		{
			rwdata[4+k]=*pnt;
			pnt++;
		}
		Encryption_Write(rwdata);
		Roseek_Waitusec(5000);	//EEPROM Write Cycle Time Max 5 mS
		j+=0x10;					//the next page
	}

	//读取写入的内容并核对

	memcpy( tmp, &AT88SC25616C_ConfigZone, 256);

	pnt = (Uint8*)&AT88SC25616C_ConfigZone;
	pnt += 0xe9;
	ui8SecureCode1of3 = *pnt;
	pnt++;
	ui8SecureCode2of3 = *pnt;
	pnt++;
	ui8SecureCode3of3 = *pnt;
	pnt++;

	pnt = (Uint8*)&AT88SC25616C_ConfigZone;
	pnt += 0x19;

	if( Roseek_Encryption_Read_Configuration( ui8SecureCode1of3, ui8SecureCode2of3, ui8SecureCode3of3))
	{
		//address from 0x19 to 0xef
		for( i=0; i<215; i++)
		{
			if( *(pnt+i) != tmp[0x19+i]){
				g_bUserEnpOpLock = FALSE;
				return FALSE;
			}
		}
	}
	else{
		g_bUserEnpOpLock = FALSE;
		return FALSE;
	}

	g_bUserEnpOpLock = FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// function:	Roseek_Encryption_Read
// 
// Descript:	This function performs read operation from AT88SC25616C.
// Input:		
// Output:		None
// Note:		1 <= i2c_byte_number_read  <= 32768
//				if failure, return FALSE
//////////////////////////////////////////////////////////////////////
Bool Roseek_Encryption_Read(	Uint16 ui16SubAddress,
							Uint8 *puiDataReadBuff,
							Uint16 ui16ReadByteNumber
							)
{
	Uint8 current_zone,rd_high_addr,rd_low_addr;
	Uint16 zone_addr,current_len,rd_number,zone_rd_number,zone_rd_cnt;
	Uint16 j,k,l;

	if( g_bUserEnpOpLock ){
		return FALSE;
	}
	g_bUserEnpOpLock = TRUE;

	if( ui16ReadByteNumber < 1 ){
		g_bUserEnpOpLock = FALSE;
		return TRUE;
	}

	if( ui16SubAddress>32767 || ui16ReadByteNumber>32768 ){
		g_bUserEnpOpLock = FALSE;
		return FALSE;
	}

	if( Encryption_AAC!=0xff)									//只能进行一次认证
		{Encryption_AAC=Encryption_AUTHENTICATION(KEY_SET);}//使用相应的密钥进行认证

	if( Encryption_PAC!=0xff)												//只能进行一次写密码校验
		{Encryption_PAC=Encryption_verify_write_password(PASSWORD_SET);}	//使用相应的密码进行写密码校验

	if( Encryption_AAC!=0xff || Encryption_PAC!=0xff){
		g_bUserEnpOpLock = FALSE;
		return( FALSE );			//认证或者写密码教校验没有通过
	}

	current_zone = (Uint8)(ui16SubAddress/2048);
	zone_addr = ui16SubAddress%2048;
	current_len = ui16ReadByteNumber;

	while( current_len > 0)
	{
		Encryption_set_user_zone( current_zone);
		current_zone++;

		rd_high_addr = (Uint8)(zone_addr>>8);
		rd_low_addr = (Uint8)(zone_addr & 0x00ff);

		if( (zone_addr+current_len) >= 2048)
			zone_rd_number = 2048 - zone_addr;
		else
			zone_rd_number = current_len;

		if( zone_rd_number%128 == 0)
			zone_rd_cnt = zone_rd_number/128;
		else
			zone_rd_cnt = zone_rd_number/128+1;

		current_len -= zone_rd_number;

		//完成本区内数据的读取
		for( j=0; j<zone_rd_cnt; j++)
		{
			if( zone_rd_number >= 128)
				rd_number = 128;
			else
				rd_number = (Uint8)(zone_rd_number%128);

			//读取并解密数据：明文存放在rwdata[0x04]起始的位置
			Encryption_read_user_zone(rd_high_addr,rd_low_addr,rd_number);

			for( k=0; k<rd_number; k++){
				*puiDataReadBuff++ = rwdata[k+4];
			}

			l = rd_high_addr*256 + rd_low_addr;
			l += rd_number;
			rd_high_addr = (Uint8)(l>>8);
			rd_low_addr = (Uint8)(l & 0x00ff);

			zone_rd_number -= rd_number;
		}

		zone_addr = 0x0000;
	}

	g_bUserEnpOpLock = FALSE;
	return( TRUE );
}

//////////////////////////////////////////////////////////////////////
// function:	Roseek_Encryption_Write
// 
// Descript:	This function performs write operation for AT88SC25616C.
// Input:		
// Output:		None
// Note:		if failure, return FALSE;
//////////////////////////////////////////////////////////////////////
Bool Roseek_Encryption_Write( 	Uint16 ui16SubAddress,
								Uint8 *pui8DataWriteBuff,
								Uint16 ui16WriteByteNumber
							)
{
	Uint8 current_zone,wr_high_addr,wr_low_addr,wr_number;
	Uint16 zone_addr,current_len,zone_wr_number,zone_wr_cnt;
	Uint16 j,k,l;

	if( g_bUserEnpOpLock ){
		return FALSE;
	}
	g_bUserEnpOpLock = TRUE;

	if( ui16SubAddress>32767 || ui16WriteByteNumber>32768 ){
		g_bUserEnpOpLock = FALSE;
		return FALSE;
	}

	if( Encryption_AAC!=0xff)									//只能进行一次认证
		{Encryption_AAC=Encryption_AUTHENTICATION(KEY_SET);}	//使用相应的密钥进行认证

	if( Encryption_PAC!=0xff)												//只能进行一次写密码校验
		{Encryption_PAC=Encryption_verify_write_password(PASSWORD_SET);} //使用相应的密码进行写密码校验

	if( Encryption_AAC!=0xff || Encryption_PAC!=0xff){
		g_bUserEnpOpLock = FALSE;
		return( FALSE );			//认证或者写密码教校验没有通过
	}

	current_zone = (Uint8)(ui16SubAddress/2048);
	zone_addr = ui16SubAddress%2048;
	current_len = ui16WriteByteNumber;

	while( current_len > 0)
	{
		//切换用户区
		Encryption_set_user_zone( current_zone);

		wr_high_addr = (Uint8)(zone_addr>>8);
		wr_low_addr = (Uint8)(zone_addr & 0x00ff);

		if( (zone_addr+current_len) >= 2048)
			zone_wr_number = 2048 - zone_addr;
		else
			zone_wr_number = current_len;

		if( zone_wr_number%128 == 0)
			zone_wr_cnt = zone_wr_number/128;
		else
			zone_wr_cnt = zone_wr_number/128+1;

		zone_addr = 0x0000;
		current_len -= zone_wr_number;

		//完成本区内数据的写入
		for( j=0; j<zone_wr_cnt; j++)
		{
			if( zone_wr_number >= 128)
				wr_number = 128;
			else
				wr_number = (Uint8)(zone_wr_number%128);

			for( k=0; k<wr_number; k++){
				rwdata[k+4] = *pui8DataWriteBuff++;
			}

			//加密并写入数据到AT88SCXX：明文存放在rwdata[0x04]起始的位置
			Encryption_write_user_zone(wr_high_addr,wr_low_addr,wr_number);

			//执行以上两条指令以后，必须再执行一条指令（任意指令都可以）才能真正把
			//数据从AT88SCXXC的缓冲区中写入EEPROM中，使得掉电后数据不消失。
			//在此执行切换用户区这条指令。
			Encryption_set_user_zone( current_zone);
			Encryption_AckPolling();

			l = wr_high_addr*256 + wr_low_addr;
			l += wr_number;
			wr_high_addr = (Uint8)(l>>8);
			wr_low_addr = (Uint8)(l & 0x00ff);

			zone_wr_number -= wr_number;
		}
		current_zone++;
	}

	g_bUserEnpOpLock = FALSE;
	return( TRUE );
}


