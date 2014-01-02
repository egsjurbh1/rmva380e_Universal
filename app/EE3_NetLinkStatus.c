/**
 * \file      	EE3_NetLinkStatus.c 
 * \author    	Leo bai
 * \version   	1.00
 * \date      	2012.08.01
 * \brief     	网络物理层状态反馈模块
**/
#include "EE3_common.h"

/* This string array corresponds to link state */
static Int8 *LinkStr[] = {	"No Link",
							"None",
                          	"10Mb/s Half Duplex",
                          	"10Mb/s Full Duplex",
                          	"100Mb/s Half Duplex",
                          	"100Mb/s Full Duplex",
						  	"1000Mb/s Half Duplex", /*not suported*/
                          	"1000Mb/s Full Duplex"};


/*
 * CPSW3G_linkStatus()
 *
 * This is a callback from the Ethernet driver. This function
 * is called whenever there is a change in link state. The
 * current PHY and current link state are passed as parameters.
 */
void CPSW3G_linkStatus( Uint32 phy, Uint32 linkStatus )
{
    printf("Link Status: %s on PHY %d\n",LinkStr[linkStatus],phy);
}
