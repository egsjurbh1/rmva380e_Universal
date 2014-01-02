/**
 * \file      	EE3_Version.c
 * \author    	Leo Bai
 * \version   	1.00
 * \date      	2011.07.13
 * \brief     	版本信息模块
 * \note		用户不可更改该模块中的内容
**/
#include "EE3_common.h"

int Roseek_RealWholeVersionInf(char* pCInf)
{
		Uint32	i	=0;
		RoseekModuleVersionInf item;	
		
		#ifdef _ROSEEK_MODULE_00
		++i;
		Roseek_Module_Reg_00((Uint8 *)(&item) );
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_01
		++i;
		Roseek_Module_Reg_01((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_02
		++i;
		Roseek_Module_Reg_02((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_03
		++i;
		Roseek_Module_Reg_03((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_04
		++i;
		Roseek_Module_Reg_04((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_05
		++i;
		Roseek_Module_Reg_05((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_06
		++i;
		Roseek_Module_Reg_06((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_07
		++i;
		Roseek_Module_Reg_07((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_08
		++i;
		Roseek_Module_Reg_08((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_09
		++i;
		Roseek_Module_Reg_09((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_10
		++i;
		Roseek_Module_Reg_10((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_11
		++i;
		Roseek_Module_Reg_11((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_12
		++i;
		Roseek_Module_Reg_12((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_13
		++i;
		Roseek_Module_Reg_13((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_14
		++i;
		Roseek_Module_Reg_14((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_15
		++i;
		Roseek_Module_Reg_15((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_16
		++i;
		Roseek_Module_Reg_16((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_17
		++i;
		Roseek_Module_Reg_17((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_18
		++i;
		Roseek_Module_Reg_18((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_19
		++i;
		Roseek_Module_Reg_19((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
			
		#ifdef _ROSEEK_MODULE_20
		++i;
		Roseek_Module_Reg_20((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_21
		++i;
		Roseek_Module_Reg_21((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_22
		++i;
		Roseek_Module_Reg_22((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_23
		++i;
		Roseek_Module_Reg_23((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_24
		++i;
		Roseek_Module_Reg_24((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_25
		++i;
		Roseek_Module_Reg_25((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_26
		++i;
		Roseek_Module_Reg_26((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_27
		++i;
		Roseek_Module_Reg_27((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_28
		++i;
		Roseek_Module_Reg_28((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_29
		++i;
		Roseek_Module_Reg_29((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_30
		++i;
		Roseek_Module_Reg_30((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_31
		++i;
		Roseek_Module_Reg_31((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_32
		++i;
		Roseek_Module_Reg_32((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_33
		++i;
		Roseek_Module_Reg_33((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_34
		++i;
		Roseek_Module_Reg_34((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_35
		++i;
		Roseek_Module_Reg_35((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_36
		++i;
		Roseek_Module_Reg_36((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_37
		++i;
		Roseek_Module_Reg_37((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_38
		++i;
		Roseek_Module_Reg_38((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_39
		++i;
		Roseek_Module_Reg_39((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_40
		++i;
		Roseek_Module_Reg_40((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_41
		++i;
		Roseek_Module_Reg_41((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_42
		++i;
		Roseek_Module_Reg_42((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_43
		++i;
		Roseek_Module_Reg_43((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_44
		++i;
		Roseek_Module_Reg_44((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_45
		++i;
		Roseek_Module_Reg_45((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_46
		++i;
		Roseek_Module_Reg_46((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_47
		++i;
		Roseek_Module_Reg_47((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_48
		++i;
		Roseek_Module_Reg_48((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_49
		++i;
		Roseek_Module_Reg_49((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_50
		++i;
		Roseek_Module_Reg_50((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_51
		++i;
		Roseek_Module_Reg_51((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_52
		++i;
		Roseek_Module_Reg_52((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_53
		++i;
		Roseek_Module_Reg_53((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_54
		++i;
		Roseek_Module_Reg_54((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_55
		++i;
		Roseek_Module_Reg_55((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_56
		++i;
		Roseek_Module_Reg_56((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_57
		++i;
		Roseek_Module_Reg_57((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_58
		++i;
		Roseek_Module_Reg_58((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_59
		++i;
		Roseek_Module_Reg_59((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_60
		++i;
		Roseek_Module_Reg_60((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_61
		++i;
		Roseek_Module_Reg_61((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_62
		++i;
		Roseek_Module_Reg_62((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_63
		++i;
		Roseek_Module_Reg_63((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_64
		++i;
		Roseek_Module_Reg_64((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_65
		++i;
		Roseek_Module_Reg_65((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_66
		++i;
		Roseek_Module_Reg_66((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_67
		++i;
		Roseek_Module_Reg_67((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_68
		++i;
		Roseek_Module_Reg_68((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_69
		++i;
		Roseek_Module_Reg_69((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_70
		++i;
		Roseek_Module_Reg_70((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_71
		++i;
		Roseek_Module_Reg_71((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_72
		R++i;
		Roseek_Module_Reg_72((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_73
		++i;
		Roseek_Module_Reg_73((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_74
		++i;
		Roseek_Module_Reg_74((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_75
		++i;
		Roseek_Module_Reg_75((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_76
		++i;
		Roseek_Module_Reg_76((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_77
		++i;
		Roseek_Module_Reg_77((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_78
		++i;
		Roseek_Module_Reg_78((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_79
		++i;
		Roseek_Module_Reg_79((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		#ifdef _ROSEEK_MODULE_80
		++i;
		Roseek_Module_Reg_80((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_81
		++i;
		Roseek_Module_Reg_81((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_82
		++i;
		Roseek_Module_Reg_82((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_83
		++i;
		Roseek_Module_Reg_83((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_84
		++i;
		Roseek_Module_Reg_84((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_85
		++i;
		Roseek_Module_Reg_85((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_86
		++i;
		Roseek_Module_Reg_86((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_87
		++i;
		Roseek_Module_Reg_87((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_88
		++i;
		Roseek_Module_Reg_88((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_89
		++i;
		Roseek_Module_Reg_89((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
		
		
		#ifdef _ROSEEK_MODULE_90
		++i;
		Roseek_Module_Reg_90((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_91
		++i;
		Roseek_Module_Reg_91((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_92
		++i;
		Roseek_Module_Reg_92((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_93
		++i;
		Roseek_Module_Reg_93((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_94
		++i;
		Roseek_Module_Reg_94((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_95
		++i;
		Roseek_Module_Reg_95((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_96
		++i;
		Roseek_Module_Reg_96((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_97
		++i;
		Roseek_Module_Reg_97((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_98
		++i;
		Roseek_Module_Reg_98((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif

		#ifdef _ROSEEK_MODULE_99
		++i;
		Roseek_Module_Reg_99((Uint8 *)(&item) ) ;
		strcat(pCInf,item.strVerInf);
		strcat(pCInf,"\n\n");
		#endif
	
		return i;
}


