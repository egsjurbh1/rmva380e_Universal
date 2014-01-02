/*  ============================================================================
 *  Copyright (c) Texas Instruments Inc 2002, 2003, 2004, 2005, 2006, 2007
 *
 *  Use of this software is controlled by the terms and conditions found in the
 *  license agreement under which this software has been supplied.
 *  ===========================================================================
 */

#ifndef _SOC_H_
#define _SOC_H_

/* =============================================================================
 *  Revision History
 *  ===============
 *  15-Feb-2007 NS  File created.
 *  Mar 20, 2007           ANUJ           Updated for DM648 SoC
 * =============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif
//#include <ti/sdo/pspdrivers/soc/dm648/dsp/cslr.h>
#include <cslr.h>

/**************************************************************************\
* 64 soc file
\**************************************************************************/

/*****************************************************************************\
* Static inline definition
\*****************************************************************************/
#ifndef CSL_IDEF_INLINE
#define CSL_IDEF_INLINE static inline
#endif

/**************************************************************************\
* Peripheral Instance count
\**************************************************************************/

/** @brief Number of PBIST instances */
#define CSL_PBIST_PER_CNT                1

/** @brief Number of UHPI instances */
#define CSL_HPI_PER_CNT                 1

/** @brief Number of McASP instances */
#define CSL_MCASP_PER_CNT                1

/** @brief Number of TIMER instances */
#define CSL_TMR_PER_CNT                  4

/** @brief Number of PSC instances */
#define CSL_PSC_PER_CNT                  1

/** @brief Number of UART instances */
#define CSL_UART_PER_CNT                 1

/** @brief Number of VIC instances */
#define CSL_VIC_PER_CNT                  1

/** @brief Number of SPI instances */
#define CSL_SPI_PER_CNT                  1

/** @brief Number of I2C instances */
#define CSL_I2C_PER_CNT                  1

/** @brief Number of PCI instances */
#define CSL_PCI_PER_CNT                  1

/** @brief Number of IMCOP instances */
#define CSL_IMCOP_PER_CNT               1

/** @brief Number of PLL instances */
#define CSL_PLLC_PER_CNT                  2

/** @brief Number of EDMA3 CC instances */
#define CSL_EDMA3CC_CNT             1

/** @brief Number of EDMA3 TC instances */
#define CSL_EDMA3TC_CNT             4

/** @brief Number of VIDEOPORT instances */
#define CSL_VIDEOPORT_PER_CNT            5

/** @brief Number of SGMII instances */
#define CSL_SGMII_PER_CNT                2

/** @brief Number of VLYNQ instances */
#define CSL_VLYNQ_PER_CNT                1

/** @brief Number of AEMIF instances */
#define CSL_AEMIF_PER_CNT               1

/** @brief Number of DDR instances */
#define CSL_DDR2_PER_CNT                 1

/** @brief Number of System module instances */
#define CSL_SYS_PER_CNT                  1


/**************************************************************************\
* Peripheral Instance definitions.
\**************************************************************************/

/** @brief Peripheral Instances of PBIST instances */
#define CSL_PBIST                       (0)

/** @brief Peripheral Instances of UHPI instances */
#define CSL_HPI                         (0)

/** @brief Peripheral Instances of McASP instances */
#define CSL_MCASP                       (0)

/** @brief Peripheral Instance of EDMA TC instances */
#define CSL_EDMA3TC_0                   (0)
#define CSL_EDMA3TC_1                   (1)
#define CSL_EDMA3TC_2                   (2)
#define CSL_EDMA3TC_3                   (3)

/** @brief Peripheral Instance of Timer 64 instances */
#define CSL_TMR_0                       (0)
#define CSL_TMR_1                       (1)
#define CSL_TMR_2                       (2)
#define CSL_TMR_3                       (3)

/** @brief Peripheral Instances of PSC instances */
#define CSL_PSC                         (0)

/** @brief Peripheral Instances of UART instances */
#define CSL_UART                        (0)

/** @brief Peripheral Instances of PBIST instances */
#define CSL_PBIST                       (0)

/** @brief Peripheral Instances of VIC instances */
#define CSL_VIC                         (0)

/** @brief Peripheral Instances of SPI instances */
#define CSL_SPI                         (0)

/** @brief Peripheral Instances of I2C instances */
#define CSL_I2C                         (0)

/** @brief Peripheral Instances of PCI instances */
#define CSL_PCI                         (0)

/** @brief Peripheral Instances of IMCOP instances */
#define CSL_IMCOP                      (0)

/** @brief Instance number of PLL controller 1 */
#define CSL_PLLC_1                     (0)
/** @brief Instance number of PLL controller 2 */
#define CSL_PLLC_2                     (1)

/** @brief Peripheral Instance of EDMA3 instances */
#define CSL_EDMA3                       (0)

/** @brief Peripheral Instance of VIDEOPORT instances */
#define CSL_VIDEOPORT_0                        (0)
#define CSL_VIDEOPORT_1                        (1)
#define CSL_VIDEOPORT_2                        (2)
#define CSL_VIDEOPORT_3                        (3)
#define CSL_VIDEOPORT_4                        (4)

/** @brief Peripheral Instance of SGMII instances */
#define CSL_SGMII_0                     (0)
#define CSL_SGMII_1                     (1)

/** @brief Peripheral Instance of VLYNQ instances */
#define CSL_VLYNQ                       (0)

/** @brief Peripheral Instance of AEMIF instances */
#define CSL_EMIFA                       (0)

/** @brief Peripheral Instance of DDR2 instances */
#define CSL_DDR2                        (0)

/**************************************************************************
* Peripheral Base Address
**************************************************************************/

/** @brief Base address of INTC memory mapped registers */
#define CSL_INTC_0_REGS                     (0x01800000u)

/** #brief Cache Module memory mapped address  */
#define CSL_CACHE_0_REGS                    (0x01840000u) 

/** @brief Base address of BWMNGMT memory mapped registers */
#define CSL_BWMNGMT_0_REGS                  (0x01820200u)

/** @brief Base address of PDC memory mapped registers */
#define CSL_PWRDWN_PDC_REGS 	            (0x01810000u)

/** @brief Base address of L2 power Down memory mapped registers */
#define CSL_PWRDWN_L2_REGS	                (0x0184C000u)

/** @brief Base address of SYS - Security ID register */
#define CSL_SYS_0_SECURITY_ID_REGS          (0x01811000u)

/** @brief Base address of SYS - Security ID register */
#define CSL_SYS_0_REV_ID_REGS               (0x01812000u)

/** #brief IDMA Module memory mapped address  */
#define CSL_IDMA_0_REGS                     (0x01820000u)

/** @brief Base address of UMC Memory protection registers */
#define CSL_MEMPROT_L2_REGS                 (0x0184A000u)
 
/** @brief Base address of PMC memory Protection registers */
#define CSL_MEMPROT_L1P_REGS                (0x0184A400u)

/** @brief Base address of DMC memory protection registers */
#define CSL_MEMPROT_L1D_REGS                (0x0184AC00u)

/** @brief Base address of CONFIG memory protection registers */
#define CSL_MEMPROT_CONFIG_REGS             (0x01820300u)

/** @brief Base address of HPI memory mapped registers */
#define CSL_HPI_0_REGS                      (0x02000000u)

/** @brief Base address of McASP memory mapped registers */
#define CSL_MCASP_0_REGS                    (0x02040000u)

/** @brief Base address of TIMER memory mapped registers */
#define CSL_TMR_0_REGS                      (0x02044400u)
#define CSL_TMR_1_REGS                      (0x02044800u)
#define CSL_TMR_2_REGS                      (0x02044C00u)
#define CSL_TMR_3_REGS                      (0x02045000u)

/** @brief Base address of PSC memory mapped registers */
#define CSL_PSC_0_REGS                      (0x02046000u)

/** @brief Base address of UART memory mapped registers */
#define CSL_UART_0_REGS                     (0x02047000u)

/** @brief Base address of VIC memory mapped registers */
#define CSL_VIC_0_REGS                      (0x02047400u)

/** @brief Base address of SPI memory mapped registers */
#define CSL_SPI_0_REGS                      (0x02047800u)

/** @brief Base address of I2C memory mapped registers */
#define CSL_I2C_0_REGS                      (0x02047C00u)

/** @brief Base address of GPIO memory mapped registers */
#define CSL_GPIO_0_REGS                     (0x02048000u)

/** @brief Base address of PCI memory mapped registers */
#define CSL_PCI_0_REGS                      (0x02048400u)

/** @brief Base address of DEV memory mapped registers */
#define CSL_SYS_0_REGS                      (0x02049000u)

/** @brief Base address of IMCOP memory mapped registers */
#define CSL_IMCOP_0_REGS                    (0x02080000u)

/** @brief PLL controller instance 1 module address */
#define CSL_PLLC_1_REGS	                    (0x020E0000u)

/** @brief PLL controller instance 2 module address */
#define CSL_PLLC_2_REGS	                    (0x02120000u)

/** @brief Base address of Channel controller  memory mapped registers */
#define CSL_EDMA3CC_0_REGS                  (0x02A00000u)

/** @brief Base address of Transfer controller  memory mapped registers */
#define CSL_EDMA3TC_0_REGS                  (0x02A20000u)
#define CSL_EDMA3TC_1_REGS                  (0x02A28000u)
#define CSL_EDMA3TC_2_REGS                  (0x02A30000u)
#define CSL_EDMA3TC_3_REGS                  (0x02A38000u)

/** @brief Base address of VIDEOPORT memory mapped registers */
#define CSL_VIDEOPORT_0_REGS                (0x02C00000u)
#define CSL_VIDEOPORT_1_REGS                (0x02C04000u)
#define CSL_VIDEOPORT_2_REGS                (0x02C08000u)
#define CSL_VIDEOPORT_3_REGS                (0x02C0C000u)
#define CSL_VIDEOPORT_4_REGS                (0x02C10000u)

/** @brief Base address of ECTL memory mapped registers */
#define CSL_ECTL_0_REGS                     (0x02D02000u)

/** @brief Base address of 3PSW memory mapped registers */
#define CSL_3PSW_0_REGS                     (0x02D03000u)

/** @brief Base address of MDIO memory mapped registers */
#define CSL_MDIO_0_REGS                     (0x02D04000u)

/** @brief Base address of SGMII memory mapped registers */
#define CSL_SGMII_0_REGS                    (0x02D04800u)
#define CSL_SGMII_1_REGS                    (0x02D04C00u)

/** @brief Base address of VLYNQ memory mapped registers */
#define CSL_VLYNQ_0_REGS                    (0x38000000u)

/** @brief Base address of AEMIF memory mapped registers */
#define CSL_EMIFA_0_REGS                   (0x70000000u)

/** @brief Base address of DDR2 memory mapped registers */
#define CSL_DDR2_0_REGS                     (0x78000000u)

/** @brief Base address of EMIFA_CE2 memory */
#define CSL_EMIFA_CE2_ADDR					(0xA0000000u)

/** @brief Base address of EMIFA_CE3 memory */
#define CSL_EMIFA_CE3_ADDR					(0xB0000000u)

/** @brief Base address of DDR2_SDRAM memory */
#define CSL_DDR2_SDRAM_ADDR					(0xE0000000u)

/******************************************************************************\
* MCASP Descriptor section
\******************************************************************************/
/** @brief McASP Tx Buffer Address */
#define CSL_MCASP_TXBUF_ADDR	            (0x02044000u)
/** @brief McASP Rx Buffer Address */
#define CSL_MCASP_RXBUF_ADDR	            (0x02044000u)

/** @brief Base address of PCI data section */
#define CSL_PCI_0_DATA_ADDR                   (0x40000000u)

/******************************************************************************\
* VLYNQ Descriptor section
\******************************************************************************/
/** @brief VLYNQ Remote Data */
#define CSL_VLYNQ_RDATA_ADDR	        (0x4C000000u)

/******************************************************************************\
* VIDEOPORTS Descriptor section
\******************************************************************************/
#define CSL_VIDEOPORT0_CHANNELA_DATA                (0x50000000u)
#define CSL_VIDEOPORT0_CHANNELB_DATA                (0x52000000u)
#define CSL_VIDEOPORT1_CHANNELA_DATA                (0x54000000u)
#define CSL_VIDEOPORT1_CHANNELB_DATA                (0x56000000u)
#define CSL_VIDEOPORT2_CHANNELA_DATA                (0x58000000u)
#define CSL_VIDEOPORT2_CHANNELB_DATA                (0x5A000000u)
#define CSL_VIDEOPORT3_CHANNELA_DATA                (0x60000000u)
#define CSL_VIDEOPORT3_CHANNELB_DATA                (0x62000000u)
#define CSL_VIDEOPORT4_CHANNELA_DATA                (0x64000000u)
#define CSL_VIDEOPORT4_CHANNELB_DATA                (0x66000000u)

/******************************************************************************\
* EMAC Descriptor section
\******************************************************************************/

#define CSL_EMAC_DSC_BASE_ADDR              (0x02D00000u)
#define CSL_EMAC_DSC_BASE_ADDR_L2           (0x00A00800u)

/* EMAC Descriptor Size and Element Count */
#define CSL_EMAC_DSC_SIZE                   8192
#define CSL_EMAC_DSC_ENTRY_SIZE             16   /* Size of a buffer descriptor, in bytes */
#define CSL_EDMA_DSC_ENTRY_COUNT            (CSL_EMAC_DSC_SIZE/CSL_EMAC_DSC_ENTRY_SIZE)  /* 512 */


/*****************************************************************************\
* Interrupt Event IDs
\*****************************************************************************/

/**
 * @brief   Interrupt Event IDs
 */
/* Output of event combiner 0, for events 1 to 31  */
#define    CSL_INTC_EVENTID_EVT0            (0)              
/* Output of event combiner 0, for events 32 to 63 */
#define    CSL_INTC_EVENTID_EVT1            (1)              
/* Output of event combiner 0, for events 64 to 95 */
#define    CSL_INTC_EVENTID_EVT2            (2)              
/* Output of event combiner 0, for events 96 to 127 */
#define    CSL_INTC_EVENTID_EVT3            (3)              

/* Event ID 4-8 are reserved */

/** EMU interrupt for: 
  * 1. Host scan access 
  * 2. DTDMA transfer complete
  * 3. AET interrupt
  */    
#define    CSL_INTC_EVENTID_EMU_DTDMA       (9)         

/* Event ID 10 is reserved */

/* EMU real time data exchange receive complete    */
#define    CSL_INTC_EVENTID_EMU_RTDXRX      (11)        
/* EMU RTDX transmit complete   */
#define    CSL_INTC_EVENTID_EMU_RTDXTX      (12)        
/* IDMA Channel 0 Interrupt */
#define    CSL_INTC_EVENTID_IDMA0           (13)        
/* IDMA Channel 1 Interrupt */
#define    CSL_INTC_EVENTID_IDMA1           (14)        
/* HPI/PCI Host interrupt   */
#define    CSL_INTC_EVENTID_PCIINT          (15)        
/* I2C interrupt            */
#define    CSL_INTC_EVENTID_I2CINT0          (16)  

/* Event ID 17 is reserved */
      
/* AEMIF Error Interrupt    */
#define    CSL_INTC_EVENTID_EMIFAINT         (18)   

/* Timer 2 lower counter interrupt     */
#define    CSL_INTC_EVENTID_TINTL2         (19)        
/* Timer 2 higher counter interrupt    */
#define    CSL_INTC_EVENTID_TINTH2         (20)        
/* Timer 3 lower counter interrupt     */
#define    CSL_INTC_EVENTID_TINTL3         (21)        
/* Timer 3 higher counter interrupt    */
#define    CSL_INTC_EVENTID_TINTH3         (22) 
/* PSC interrupt    */
#define    CSL_INTC_EVENTID_PSCINT          (23)      
/* EDMA3 channel global completion interrupt */
#define    CSL_INTC_EVENTID_EDMA3CC_GINT    (24)   
/* SPI interrupt    */
#define    CSL_INTC_EVENTID_SPIINT0          (25) 
/* SPI interrupt    */
#define    CSL_INTC_EVENTID_SPIINT1          (26)   
/* IMCOP SQR interrupt    */
#define    CSL_INTC_EVENTID_DSQINT          (27) 
/* IMX interrupt    */
#define    CSL_INTC_EVENTID_IMXINT          (28) 
/* VLCD interrupt    */
#define    CSL_INTC_EVENTID_VLCDINT         (29) 
/* 3PSWG RX pulse interrupt */
#define    CSL_INTC_EVENTID_RX_PULSE        (32)
/* 3PSWG RX threshold interrupt */    
#define    CSL_INTC_EVENTID_RX_THRESH_PULSE (33)
/* 3PSWG TX pulse interrupt */
#define    CSL_INTC_EVENTID_TX_PULSE        (34)
/* 3PSWG MISC pulse interrupt */
#define    CSL_INTC_EVENTID_MISC_PULSE      (35)
/* UART interrupt     */
#define    CSL_INTC_EVENTID_UARTINT0         (36)    
/* VIDOEPORT interrupt     */
#define    CSL_INTC_EVENTID_VP0_INT         (37)    
#define    CSL_INTC_EVENTID_VP1_INT         (38)    
#define    CSL_INTC_EVENTID_VP2_INT         (39)    
#define    CSL_INTC_EVENTID_VP3_INT         (40)    
#define    CSL_INTC_EVENTID_VP4_INT         (41)    
/* GPIO BANK 1 interrupt     */
#define    CSL_INTC_EVENTID_GPIO_BNK1_INT   (42)    
/* McASP TX interrupt     */
#define    CSL_INTC_EVENTID_AXINT0           (43)
/* McASP RX interrupt     */
#define    CSL_INTC_EVENTID_ARINT0           (44)

/* Event ID 45-49 is reserved */ 

/* VLYNQ interrupt         */
#define    CSL_INTC_EVENTID_VLQINT            (50)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO0          (51)         
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO1          (52)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO2          (53)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO3          (54)        
/* GPIO Interrupt */
#define    CSL_INTC_EVENTID_GPIO4          (55)        
/* GPIO Interrupt */
#define    CSL_INTC_EVENTID_GPIO5          (56)        
/* GPIO Interrupt */
#define    CSL_INTC_EVENTID_GPIO6          (57)        
/* GPIO Interrupt */
#define    CSL_INTC_EVENTID_GPIO7          (58)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO8          (59)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO9          (60)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO10         (61)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO11         (62)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO12         (63)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO13         (64)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO14         (65)        
/* GPIO Interrupt           */ 
#define    CSL_INTC_EVENTID_GPIO15         (66)        
/* Timer 0 lower counter interrupt     */
#define    CSL_INTC_EVENTID_TINTL0         (67)        
/* Timer 0 higher counter interrupt    */
#define    CSL_INTC_EVENTID_TINTH0         (68)        
/* Timer 1 lower counter interrupt     */
#define    CSL_INTC_EVENTID_TINTL1         (69)        
/* Timer 1 higher counter interrupt    */
#define    CSL_INTC_EVENTID_TINTH1         (70)        
/* CC Completion Interrupt - Mask0 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT0    (71)        
/* CC Completion Interrupt - Mask1 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT1    (72)        
/* CC Completion Interrupt - Mask2 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT2    (73)        
/* CC Completion Interrupt - Mask3 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT3    (74)        
/* CC Completion Interrupt - Mask4 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT4    (75)        
/* CC Completion Interrupt - Mask5 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT5    (76)        
/* CC Completion Interrupt - Mask6 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT6    (77)        
/* CC Completion Interrupt - Mask7 */
#define    CSL_INTC_EVENTID_EDMA3CC_INT7    (78)        
/* CC Error Interrupt    */
#define    CSL_INTC_EVENTID_EDMA3CC_ERRINT  (79)              
/* CC Memory Protection Interrupt */
#define    CSL_INTC_EVENTID_EDMA3CC_MPINT   (80)        
/* TC0 Error Interrupt */
#define    CSL_INTC_EVENTID_EDMA3TC0_ERRINT (81)        
/* TC1 Error Interrupt */
#define    CSL_INTC_EVENTID_EDMA3TC1_ERRINT (82)        
/* TC2 Error Interrupt */
#define    CSL_INTC_EVENTID_EDMA3TC2_ERRINT (83)        
/* TC3 Error Interrupt */
#define    CSL_INTC_EVENTID_EDMA3TC3_ERRINT (84)        

/* Event ID 86-95 is reserved */ 

/* Dropped CPU interrupt event */
#define    CSL_INTC_EVENTID_INTERR          (96)        
/* EMC Invalid IDMA parameters     */
#define    CSL_INTC_EVENTID_EMC_IDMAERR     (97)           

/* Event ID 99 is reserved */ 

/* EFI Interrupt from side A    */
#define    CSL_INTC_EVENTID_EFIINTA         (100)       
/* EFI Interrupt from side B    */
#define    CSL_INTC_EVENTID_EFIINTB         (101)       

/* Event ID 102-112 is reserved */ 

/* L1P Single bit error detected during DMA read */
#define    CSL_INTC_EVENTID_L1P_ED         (113)       

/* Event ID 114-115 is reserved */ 

/* L2 single bit error detected */
#define    CSL_INTC_EVENTID_L2_ED1          (116)       
/* L2 two bit error detected */
#define    CSL_INTC_EVENTID_L2_ED2          (117)       
/* Power Down sleep interrupt */
#define    CSL_INTC_EVENTID_PDC_INT         (118)       

/* Event ID 119 is reserved */ 

/* L1P CPU memory protection fault */
#define    CSL_INTC_EVENTID_L1P_CMPA        (120)       
/* L1P DMA memory protection fault */
#define    CSL_INTC_EVENTID_L1P_DMPA        (121)        
/* L1D CPU memory protection fault */
#define    CSL_INTC_EVENTID_L1D_CMPA        (122)       
/* L1D DMA memory protection fault */
#define    CSL_INTC_EVENTID_L1D_DMPA        (123)       
/* L2 CPU memory protection fault */
#define    CSL_INTC_EVENTID_L2_CMPA         (124)       
/* L2 DMA memory protection fault */
#define    CSL_INTC_EVENTID_L2_DMPA         (125)       
/* IDMA CPU memory protection fault */
#define    CSL_INTC_EVENTID_IDMA_CMPA       (126)       
/* IDMA Bus error interrupt  */
#define    CSL_INTC_EVENTID_IDMA_BUSERR     (127)       



/**** EDMA RELATED DEFINES  *********/


/**************************************************************************\
*  Parameterizable Configuration:- These are fed directly from the RTL
*  parameters for the given SOC
\**************************************************************************/

#define CSL_EDMA3_NUM_DMACH            64
#define CSL_EDMA3_NUM_QDMACH            8
#define CSL_EDMA3_NUM_PARAMSETS       512
#define CSL_EDMA3_NUM_EVQUE             4
#define CSL_EDMA3_CHMAPEXIST            1
#define CSL_EDMA3_NUM_REGIONS           8
#define CSL_EDMA3_MEMPROTECT            1

/**************************************************************************\
* Channel Instance count
\**************************************************************************/
#define CSL_EDMA3_CHA_CNT              72

/* EDMA channel synchronization events */
  
/* HPI/PCI-to-DSP event          */
#define CSL_EDMA3_CHA_DSP_EVT      0
/* Timer 0 lower counter event   */
#define CSL_EDMA3_CHA_TEVTL0      1
/* Timer 0 higher counter event  */
#define CSL_EDMA3_CHA_TEVTH0      2
/* Timer 2 lower counter event  */
#define CSL_EDMA3_CHA_TEVTL2      3
/* Timer 2 higher counter event */
#define CSL_EDMA3_CHA_TEVTH2      4
/* Timer 3 lower counter event  */
#define CSL_EDMA3_CHA_TEVTL3      5
/* Timer 3 higher counter event */
#define CSL_EDMA3_CHA_TEVTH3      6

/* IMCOP IMX event */
#define CSL_EDMA3_CHA_IMXINT       7
/* IMCOP VLCD event */
#define CSL_EDMA3_CHA_VLCDINT      8
/* IMCOP DSQ event */
#define CSL_EDMA3_CHA_DSQINT       9
/* McASP Transmit Event Even */
#define CSL_EDMA3_CHA_AXEVTE0      10
/* McASP Transmit Event Odd */
#define CSL_EDMA3_CHA_AXEVTO0      11
/* McASP Transmit Event  */
#define CSL_EDMA3_CHA_AXEVT0       12
/* McASP Receive Event Even */
#define CSL_EDMA3_CHA_AREVTE0      13
/* McASP Receive Event Odd */
#define CSL_EDMA3_CHA_AREVTO0      14
/* McASP Receive Event  */
#define CSL_EDMA3_CHA_AREVT0       15
/* Timer 1 lower counter event  */
#define CSL_EDMA3_CHA_TEVTL1     16
/* Timer 1 higher counter event */
#define CSL_EDMA3_CHA_TEVTH1     17

/* UART Receive event  */
#define CSL_EDMA3_CHA_URXEVT0      18
/* UART Transmit event  */
#define CSL_EDMA3_CHA_UTXEVT0      19
/* Videoport 0 events */
#define CSL_EDMA3_CHA_VP0EVTYA    20
#define CSL_EDMA3_CHA_VP0EVTUA    21
#define CSL_EDMA3_CHA_VP0EVTVA    22
#define CSL_EDMA3_CHA_VP0EVTYB    23
#define CSL_EDMA3_CHA_VP0EVTUB    24
#define CSL_EDMA3_CHA_VP0EVTVB    25
/* Videoport 1 events */
#define CSL_EDMA3_CHA_VP1EVTYA    26
#define CSL_EDMA3_CHA_VP1EVTUA    27
#define CSL_EDMA3_CHA_VP1EVTVA    28
#define CSL_EDMA3_CHA_VP1EVTYB    29
#define CSL_EDMA3_CHA_VP1EVTUB    30
#define CSL_EDMA3_CHA_VP1EVTVB    31
/* Videoport 2 events */
#define CSL_EDMA3_CHA_VP2EVTYA    32
#define CSL_EDMA3_CHA_VP2EVTUA    33
#define CSL_EDMA3_CHA_VP2EVTVA    34
#define CSL_EDMA3_CHA_VP2EVTYB    35
#define CSL_EDMA3_CHA_VP2EVTUB    36
#define CSL_EDMA3_CHA_VP2EVTVB    37
/* Videoport 3 events */
#define CSL_EDMA3_CHA_VP3EVTYA    38
#define CSL_EDMA3_CHA_VP3EVTUA    39
#define CSL_EDMA3_CHA_VP3EVTVA    40
#define CSL_EDMA3_CHA_VP3EVTYB    41
#define CSL_EDMA3_CHA_VP3EVTUB    42
#define CSL_EDMA3_CHA_VP3EVTVB    43
/*I2C Receive event */
#define CSL_EDMA3_CHA_ICREVT      44
/*I2C Transmit event */
#define CSL_EDMA3_CHA_ICXEVT      45
/*SPI Transmit event */
#define CSL_EDMA3_CHA_SPIXEVT     46
/*SPI Receive event */
#define CSL_EDMA3_CHA_SPIREVT     47
/* Videoport 4 events */
#define CSL_EDMA3_CHA_VP4EVTYA    48
#define CSL_EDMA3_CHA_VP4EVTUA    49
#define CSL_EDMA3_CHA_VP4EVTVA    50
#define CSL_EDMA3_CHA_VP4EVTYB    51
#define CSL_EDMA3_CHA_VP4EVTUB    52
#define CSL_EDMA3_CHA_VP4EVTVB    53
/* GPIO event 6 */
#define CSL_EDMA3_CHA_GPINT6      54
/* GPIO event 7 */
#define CSL_EDMA3_CHA_GPINT7      55
/* GPIO event 8 */
#define CSL_EDMA3_CHA_GPINT8      56
/* GPIO event 9 */
#define CSL_EDMA3_CHA_GPINT9      57
/* GPIO event 10 */
#define CSL_EDMA3_CHA_GPINT10     58
/* GPIO event 11 */
#define CSL_EDMA3_CHA_GPINT11     59
/* GPIO event 12 */
#define CSL_EDMA3_CHA_GPINT12     60
/* GPIO event 13 */
#define CSL_EDMA3_CHA_GPINT13     61
/* GPIO event 14 */
#define CSL_EDMA3_CHA_GPINT14     62
/* GPIO event 15 */
#define CSL_EDMA3_CHA_GPINT15     63


/* QDMA channels */
#define    CSL_EDMA3_QCHA_0       64   /* QDMA Channel 0*/
#define    CSL_EDMA3_QCHA_1       65   /* QDMA Channel 1*/
#define    CSL_EDMA3_QCHA_2       66   /* QDMA Channel 2*/
#define    CSL_EDMA3_QCHA_3       67   /* QDMA Channel 3*/
#define    CSL_EDMA3_QCHA_4       68   /* QDMA Channel 4*/
#define    CSL_EDMA3_QCHA_5       69   /* QDMA Channel 5*/
#define    CSL_EDMA3_QCHA_6       70   /* QDMA Channel 6*/
#define    CSL_EDMA3_QCHA_7       71   /* QDMA Channel 7*/


/* Enumerations for EDMA Event Queues */
typedef enum {
	CSL_EDMA3_QUE_0 		   = 			   0, /* Queue 0 */  
	CSL_EDMA3_QUE_1 		   = 			   1, /* Queue 1 */  
	CSL_EDMA3_QUE_2 		   = 			   2, /* Queue 2 */  
	CSL_EDMA3_QUE_3 		   = 			   3  /* Queue 3 */  
} CSL_Edma3Que;

/* Enumerations for EDMA Transfer Controllers
 *
 * There are 4 Transfer Controllers. Typically a one to one mapping exists
 * between Event Queues and Transfer Controllers.
 *
 */
typedef enum {
	CSL_EDMA3_TC_0 		   = 			   0, /* TC 0 */  
	CSL_EDMA3_TC_1 		   = 			   1, /* TC 1 */  
	CSL_EDMA3_TC_2 		   = 			   2, /* TC 2 */  
	CSL_EDMA3_TC_3 		   = 			   3  /* TC 3 */  
} CSL_Edma3Tc;

#define CSL_EDMA3_REGION_GLOBAL  -1
#define CSL_EDMA3_REGION_0  0
#define CSL_EDMA3_REGION_1  1
#define CSL_EDMA3_REGION_2  2
#define CSL_EDMA3_REGION_3  3
#define CSL_EDMA3_REGION_4  4
#define CSL_EDMA3_REGION_5  5
#define CSL_EDMA3_REGION_6  6
#define CSL_EDMA3_REGION_7  7



/*********** DAT RELATED DEFINES ******************/

/**************************************************************************\
*  Parameterizable Configuration:- These are fed directly from the RTL 
*  parameters for the given SOC
\**************************************************************************/
/**************************************************************************\
* Channel Instance count
\**************************************************************************/
/** @brief Number of Generic Channel instances */


/** @brief Enumerations for EDMA channels
*
*  There are 8 QDMA channels -
*
*/

#define    CSL_DAT_QCHA_0		   		      0 /**< QDMA Channel 0*/
#define    CSL_DAT_QCHA_1		   		      1 /**< QDMA Channel 1*/
#define    CSL_DAT_QCHA_2    	   		      2 /**< QDMA Channel 2*/
#define    CSL_DAT_QCHA_3		   		      3 /**< QDMA Channel 3*/
#define    CSL_DAT_QCHA_4		   		      4 /**< QDMA Channel 4*/
#define    CSL_DAT_QCHA_5		   		      5 /**< QDMA Channel 5*/
#define    CSL_DAT_QCHA_6    	   		      6 /**< QDMA Channel 6*/
#define    CSL_DAT_QCHA_7		   		      7 /**< QDMA Channel 7*/

/** @brief Enumerations for EDMA Event Queues
*
*  There are 4 Event Queues. Q0 is the highest priority and Q3 is the least priority
*
*/
typedef enum {
	CSL_DAT_PRI_DEFAULT   = 			   0, /* Queue 0 is default */  
	CSL_DAT_PRI_0 		   = 			   0, /* Queue 0 */  
	CSL_DAT_PRI_1 		   = 			   1, /* Queue 1 */  
	CSL_DAT_PRI_2 		   = 			   2, /* Queue 2 */  
	CSL_DAT_PRI_3 		   = 			   3  /* Queue 3 */  
} CSL_DatPriority;

/** @brief Enumeration for EDMA Regions 
*
*  
*/

#define	CSL_DAT_REGION_GLOBAL  -1  /* Global Region */
#define	CSL_DAT_REGION_0  0         /* EDMA Region 0 */
#define	CSL_DAT_REGION_1  1         /* EDMA Region 1 */
#define	CSL_DAT_REGION_2  2         /* EDMA Region 2 */
#define	CSL_DAT_REGION_3  3         /* EDMA Region 3 */
#define	CSL_DAT_REGION_4  4         /* EDMA Region 4 */
#define	CSL_DAT_REGION_5  5         /* EDMA Region 5 */
#define	CSL_DAT_REGION_6  6         /* EDMA Region 6 */
#define	CSL_DAT_REGION_7  7         /* EDMA Region 7 */


#define CSL_UART_0 CSL_UART

/**
 * @brief Represents the HWI Interrupt number to event combiner
 * mapping
 */

/** IRQ assignment for EVT Combiner 01 */
#define     HWI_EVT01_CMB_IRQ                                   (0x0080u)
/** IRQ assignment for EVT Combiner 02 */
#define     HWI_EVT02_CMB_IRQ                                   (0x0100u)
/** IRQ assignment for EVT Combiner 03 */
#define     HWI_EVT03_CMB_IRQ                                   (0x0200u)
/** IRQ assignment for EVT Combiner 04 */
#define     HWI_EVT04_CMB_IRQ                                   (0x0400u)
/** IRQ assignment for PCI interrupt   */
#define     HWI_PCI_IRQ                                         (0x0010u)

#ifdef __cplusplus
}
#endif

#endif  /* _SOC_H_ */

