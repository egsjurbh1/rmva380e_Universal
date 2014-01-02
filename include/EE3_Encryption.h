//////////////////////////////////////////////////////////////////////
// File name: EE3_Encryption.h		                                //
//////////////////////////////////////////////////////////////////////

#include "EE3_common.h"

#define FPGA_PUBLIC_I2C_WR_REG			0x3800018c
#define FPGA_PUBLIC_I2C_RD_REG			0x3800033c

//#define AT88SC25616C					0x20
#define	KEY_SET							2
#define	PASSWORD_SET					1

static Uint8 Encryption_AAC=0x00;
static Uint8 Encryption_PAC=0x00;
static Uint32 g_ui32FpgaI2cWrReg = 0x0;

//GPAµ•‘™ª∫≥Â«¯£® π”√∆µ¬ ∏ﬂ”¶∑≈‘⁄∆¨ƒ⁄RAM÷–£©
#pragma	DATA_SECTION( GPA, ".user_data_int");
#pragma	DATA_ALIGN( GPA, 4 );
static Uint8 GPA[20];

//√¸¡Ó∂¡–¥ª∫≥Â«¯£® π”√∆µ¬ ∏ﬂ”¶∑≈‘⁄∆¨ƒ⁄RAM÷–£©
#pragma	DATA_SECTION( rwdata, ".user_data_int");
#pragma	DATA_ALIGN( rwdata, 4 );
static Uint8 rwdata[2048];

//√‹‘ø
//◊¢“‚£∫GC0≤ªƒ‹π©”√ªß π”√£¨±ÿ–Î»´≤øŒ™0x00£¨”√“‘‘⁄≈‰÷√–æ∆¨µƒ ±∫Ú∆» π–æ∆¨ÕÀ≥ˆº”√‹Õ®—∂∑Ω 
static Uint8 GC0[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};	//GC0
static Uint8 GC1[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};	//GC1
static Uint8 GC2[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};	//GC2
static Uint8 GC3[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};	//GC3

//√‹¬Î
//◊¢“‚£∫PW_WRITE0≤ªƒ‹π©”√ªß π”√£¨±ÿ–Î»´≤øŒ™0x00£¨”√“‘‘⁄≈‰÷√–æ∆¨µƒ ±∫Ú∆» π–æ∆¨ÕÀ≥ˆº”√‹Õ®—∂∑Ω 
static Uint8 PW_WRITE0[]={0x00,0x00,0x00};	//WRITE PASSWORD 0
static Uint8 PW_WRITE1[]={0xff,0xff,0xff};	//WRITE PASSWORD 1
static Uint8 PW_WRITE2[]={0xff,0xff,0xff};	//WRITE PASSWORD 2
static Uint8 PW_WRITE3[]={0xff,0xff,0xff};	//WRITE PASSWORD 3
static Uint8 PW_WRITE4[]={0xff,0xff,0xff};	//WRITE PASSWORD 4
static Uint8 PW_WRITE5[]={0xff,0xff,0xff};	//WRITE PASSWORD 5
static Uint8 PW_WRITE6[]={0xff,0xff,0xff};	//WRITE PASSWORD 6
static Uint8 PW_WRITE7[]={0x17,0xc3,0x3a};	//WRITE PASSWORD 7

//∫Ø ˝…˘√˜
static void Encryption_Set_SDA();
static void Encryption_Clear_SDA();
static Uint32 Encryption_Read_SDA();
static void Encryption_Set_SCL();
static void Encryption_Clear_SCL();
static void Encryption_SMSTART();
static void Encryption_SMSTOP();
static void Encryption_AckPolling();
static void Encryption_GPA_CLOCK(Uint8 Datain,Uint8 times);
static void Encryption_Read(Uint8 rddata[0x20]);
static void Encryption_Write(Uint8 wrdata[0X14]);
static Uint8 Encryption_AUTHENTICATION(Uint8 GC_select);
static Uint8 Encryption_verify_write_password(Uint8 pw_select);
static void Encryption_set_user_zone(Uint8 zone);
static void Encryption_read_user_zone(Uint8 rd_high_addr,Uint8 rd_low_addr,Uint16 rd_number);
static void Encryption_write_user_zone(Uint8 wr_high_addr,Uint8 wr_low_addr,Uint8 wr_number);
static void Encryption_encrypto_data(Uint8 encryptodatanumber);
static void Encryption_send_checksum();
Bool Roseek_Encryption_Read_Configuration( Uint8 ui8SecureCode1of3, Uint8 ui8SecureCode2of3, Uint8 ui8SecureCode3of3);
Bool Roseek_Encryption_Initialization( Uint8 ui8SecureCode1of3, Uint8 ui8SecureCode2of3, Uint8 ui8SecureCode3of3);
Bool Roseek_Encryption_Read(	Uint16 ui16SubAddress, Uint8 *puiDataReadBuff, Uint16 ui16ReadByteNumber );
Bool Roseek_Encryption_Write( Uint16 ui16SubAddress, Uint8 *pui8DataWriteBuff, Uint16 ui16WriteByteNumber );

