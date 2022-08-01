#include "288p_support.h"

extern void *align_down(void *);
extern void *align_up(void *);
extern unsigned int *Tulip2TD;
extern unsigned int *TD2Tulip;

void *C_sub_find(void *h, const char *fn_name)
{
	char fname[256];
	void **ret;
	unsigned int *addr, *tmp_addr, *end_addr;
	unsigned int bl_cnt;

	C_CASE(_ZN18TDcResolutionTable16GetReferenceDataE14TDResolution_kPNS_28TDResolutionReferenceTable_tE)
		addr = find_nth_func_by_string(h, "_ZN9TDBuilder8GetTDiCPE16TDSourceObject_k", "Out Of Range...!!! Resolution %d\n", F_SEEK_DOWN, -0x0L, 0);
		addr--;	// There's a CMP before STMFD
		C_FOUND(addr);
	C_RET(addr);

	C_CASE(_ZN24TDsValenciaHdmiProcessor17m_aHdmiSupportTblE)
		addr = NULL;
		const char *refStr = "t_CheckResolution";
		tmp_addr = find_func_by_string(h, "_Z13SCK_to_membufRKSsRiPPhRm", "t_Set_HDMI_CTS", F_SEEK_DOWN, -0x1a1000L);
		if(tmp_addr) {
			tmp_addr = (unsigned int *)check_is_LDR_RD(tmp_addr);
			if(tmp_addr) {
				tmp_addr = (unsigned int *)*tmp_addr;
				end_addr = tmp_addr + 0x40;
				while(!addr && (tmp_addr < end_addr)){
					if(!strncmp((char *)tmp_addr, refStr, strlen(refStr))){
						addr = (unsigned int *)align_up((void *)tmp_addr + strlen(refStr) + 1);
						C_FOUND(addr);
						break;
					}
					tmp_addr++;
				}
			}
		}
	C_RET(addr);

	C_CASE(_ZN21RectangleHelperCommon14mTBL_E_IN_MODEE)
		addr = NULL;
		char *refStr = "ConvertEnum";
		tmp_addr = find_func_by_string(h, "_ZN9TDBuilder8GetTDiCPE16TDSourceObject_k", refStr, F_SEEK_UP, 0x135d00L);
		if(tmp_addr){
			tmp_addr = (unsigned int *)check_is_LDR_RD(tmp_addr);
			if(tmp_addr){
				tmp_addr = (unsigned int *)*tmp_addr;
				addr = (unsigned int *)align_up((void *)tmp_addr + strlen(refStr) + 1) + 14;
				C_FOUND(addr);
			}
		}
	C_RET(addr);

	C_CASE(_ZN11WindowShare22ConvertResolutionToRTMEN9TPCSource7ESourceENS0_11EResolutionE14TDResolution_k)
		addr = NULL;
		tmp_addr = find_func_by_string(h, "_ZN6PCMath6RandomEm", " FRC BorderMode Set Error ", F_SEEK_UP, 0x119000L);
		end_addr = find_function_start(tmp_addr);
		bl_cnt = 0;
		for(--tmp_addr; tmp_addr > end_addr; tmp_addr--){
			if(((unsigned int)*tmp_addr & 0xff000000) == BRANCH_BL){
				bl_cnt++;
				if(bl_cnt == 6){
					addr = (unsigned int *)calculate_branch_addr(tmp_addr);
					C_FOUND(addr);
					break;
				}
			}
		}
	C_RET(addr);

	C_CASE(_ZN10SsInfoBase26ConvertTulipResolutionToTDEN9TPCSource11EResolutionEPK13TPTResolution)
		addr = Tulip2TD;
		if(addr) C_FOUND(addr);
	C_RET(addr);

	C_CASE(_ZN10SsInfoBase26ConvertTDResolutionToTulipEN9TPCSource7ESourceEP14TDResolution_k)
		addr = TD2Tulip;
		if(addr) C_FOUND(addr);
	C_RET(addr);

	return 0;
}
