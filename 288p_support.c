/*
 *  Rapper_skull
 *	(c) 2022
 *
 *  License: GPLv3
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <memory.h>
#include <glob.h>
#include <stdarg.h>
#include <pthread.h>
#include <execinfo.h>
#include "common.h"
//#include "dynlib.h"
#include "hook.h"

#define LIB_VERSION "v0.1"
#define LIB_TV_MODELS "C/D/E/F/H"
#define LIB_PREFIX 288psupport
#define LIB_HOOKS LIB_PREFIX##_hooks
#define hCTX LIB_PREFIX##_hook_ctx

#include "util.h"
#include "C_support_custom.h"
#include "tv_info.h"
#include "288p_support.h"

const res_table_entry_t res_288p = {
  RES_ID_720x288P,
  720,
  288,
  864,
  312,
  0,
  {0, 0, 0},
  50
};

const hdmi_table_t res_288p_hdmi = {
  RES_ID_720x288P,
  720,
  288,
  0,
  {0, 0, 0},
  15625,
  50000,
  1728,
  312,
  "TD_RESOLUTION_288P"
};

const rect_table_t rect_table_720x288p = {
  RTM_ID_720x288P,
  {
    {21, 9, 42, 18},
    {21, 24, 42, 48},
    {21, 33, 42, 72 | 0x8000},
    {0, 0, 0, 0},
    {21, 57, 42, 123 | 0x8000},
    {21, 9, 42, 18},
    {0, 0, 0, 0}
  }
};

const rect_table_t rect_table_1440x288p = {
  RTM_ID_1440x288P,
  {
    {42, 9, 84, 18},
    {42, 24, 84, 48},
    {42, 33, 84, 72 | 0x8000},
    {0, 0, 0, 0},
    {42, 57, 84, 123 | 0x8000},
    {42, 9, 84, 18},
    {0, 0, 0, 0}
  }
};

#define PROCS_SIZE 3

typedef union
{
  const void *procs[PROCS_SIZE];
  struct
  {
    const signed int (*TDcResolutionTable_GetReferenceData)(int res_id, res_table_entry_t *result);
    hdmi_table_t *hdmi_table;
    size_rect_table_t *size_rect_table;
  };
} samyGO_whacky_t;

samyGO_whacky_t hCTX =
{
  (const void*)"_ZN18TDcResolutionTable16GetReferenceDataE14TDResolution_kPNS_28TDResolutionReferenceTable_tE",
  (const void*)"_ZN24TDsValenciaHdmiProcessor17m_aHdmiSupportTblE",
  (const void*)"_ZN21RectangleHelperCommon14mTBL_E_IN_MODEE",
};

unsigned int *Tulip2TD = NULL;
unsigned int *TD2Tulip = NULL;
res_table_entry_t *TDcResolutionTable_m_atExtInResolutionTable = NULL;
rect_table_t *hdmi_rect_table = NULL;
size_t hdmi_rect_table_size = 0;
rect_table_t *hdmi_rect_table_new = NULL;
size_t hdmi_rect_table_size_new = 0;

static int _hooked = 0;

_HOOK_IMPL(unsigned int, SsInfoBase_ConvertTulipResolutionToTD, void *this, unsigned int tulip_id, float_res_t *res){
  res_table_entry_t entry;
  int ret;
	_HOOK_DISPATCH(SsInfoBase_ConvertTulipResolutionToTD, this, tulip_id, res);
  if(res && tulip_id == TULIP_ID_720x576I && (unsigned int)h_ret == RES_ID_720x576I){
    ret = hCTX.TDcResolutionTable_GetReferenceData(RES_ID_720x288P, &entry);
    if(ret && res->v_res == entry.v_res){
      return RES_ID_720x288P;
    }
  }
	return (unsigned int)h_ret;
}

_HOOK_IMPL(unsigned int, SsInfoBase_ConvertTDResolutionToTulip, void *this, unsigned int src_id, unsigned int *res_id){

	_HOOK_DISPATCH(SsInfoBase_ConvertTDResolutionToTulip, this, src_id, res_id);
  if(*res_id == RES_ID_720x288P){
    return TULIP_ID_720x576I;
  }
	return (unsigned int)h_ret;
}

_HOOK_IMPL(unsigned int, WindowShare_ConvertResolutionToRTM, void *this, unsigned int src_id, unsigned int tulip_id, unsigned int res_id){

	_HOOK_DISPATCH(WindowShare_ConvertResolutionToRTM, this, src_id, tulip_id, res_id);
  if(tulip_id == TULIP_ID_720x576I){
    if(res_id == RES_ID_720x288P){
      return RTM_ID_720x288P;
    } else if(res_id == RES_ID_1440x288P){
      return RTM_ID_1440x288P;
    }
  }
	return (unsigned int)h_ret;
}

STATIC dyn_fn_t dyn_hook_fn_tab[] =
{
	{ 0, "_ZN10SsInfoBase26ConvertTulipResolutionToTDEN9TPCSource11EResolutionEPK13TPTResolution" },
	{ 0, "_ZN10SsInfoBase26ConvertTDResolutionToTulipEN9TPCSource7ESourceEP14TDResolution_k" },
  { 0, "_ZN11WindowShare22ConvertResolutionToRTMEN9TPCSource7ESourceENS0_11EResolutionE14TDResolution_k" },
};

STATIC hook_entry_t LIB_HOOKS[] =
{
#define _HOOK_ENTRY(F, I) \
	&hook_##F, &dyn_hook_fn_tab[I], &x_##F

	{ _HOOK_ENTRY(SsInfoBase_ConvertTulipResolutionToTD, 0) },
	{ _HOOK_ENTRY(SsInfoBase_ConvertTDResolutionToTulip, 1) },
  { _HOOK_ENTRY(WindowShare_ConvertResolutionToRTM, 2) },

#undef _HOOK_ENTRY
};

void *align_down(void *addr){
  return (void *)((intptr_t)addr & -4);
}

void *align_up(void *addr){
  return align_down(addr + 3);
}

res_table_entry_t *find_resolution_table(void *h, unsigned int *GetReferenceData){
  unsigned int *addr = NULL, *tmp_addr, *ldr_addr, *func_end, *rodata_begin, *rodata_end;
  const char *refStr = "GetReferenceData";
  if(GetReferenceData){
    rodata_begin = get_dlsym_addr(h, "_fini");
    rodata_end = rodata_begin + 0x2200000;
    func_end = find_function_end(GetReferenceData);
    for(tmp_addr = GetReferenceData; tmp_addr < func_end; tmp_addr++){
      ldr_addr = check_is_LDR_RD(tmp_addr);
      if(ldr_addr){
        ldr_addr = (unsigned int *)*(unsigned int *)ldr_addr;
        if(ldr_addr > rodata_begin && ldr_addr < rodata_end){
          if(!strncmp((const char *)ldr_addr, refStr, strlen(refStr))){
            addr = (unsigned int *)align_up((void *)ldr_addr + strlen(refStr) + 1); // The table is just after "GetReferenceData"
            log_found("_ZN18TDcResolutionTable24m_atExtInResolutionTableE", addr);
            break;
          }
        }
      }
    }
  }
  return (res_table_entry_t *)addr;
}

void find_tulip_functions(void *h, unsigned int *resolution_table){
  unsigned int *addr = NULL, *tmp_addr, *end_addr, *next_func, *text_begin, *rodata_begin, *test_addr, *branch_addr;
	unsigned int bl_cnt;
  tmp_addr = find_func_by_string(h, "_ZN6PCMath6RandomEm", "SetMediaMovieState", F_SEEK_UP, 0x4f000L);
  tmp_addr = (unsigned int *)*(unsigned int *)check_is_LDR_RD(tmp_addr);	// Address of the string in .rodata
  end_addr = tmp_addr + 0x400;
  text_begin = get_dlsym_addr(h, "_init");
  rodata_begin = get_dlsym_addr(h, "_fini");
  while(!addr && (tmp_addr < end_addr)){
    test_addr = (unsigned int *)*tmp_addr;
    if(test_addr > text_begin && test_addr < rodata_begin){	// We found a function (maybe)
      bl_cnt = 0;
      next_func = find_next_function_start(test_addr);
      while(!addr && (test_addr < next_func)){
        if((*(unsigned int *)test_addr & 0xff000000) == BRANCH_BL){
          branch_addr = (unsigned int *)calculate_branch_addr(test_addr);
          if(branch_addr == resolution_table){
            bl_cnt++;
          }
        }
        if(bl_cnt == 4){
          addr = find_function_start(test_addr);
          Tulip2TD = addr;
          log_found("_ZN10SsInfoBase26ConvertTulipResolutionToTDEN9TPCSource11EResolutionEPK13TPTResolution", Tulip2TD);
        }
        test_addr++;
      }
    }
    if(addr){
      TD2Tulip = (unsigned int *)*(tmp_addr - 3);
      log_found("_ZN10SsInfoBase26ConvertTDResolutionToTulipEN9TPCSource7ESourceEP14TDResolution_k", TD2Tulip);
    }
    tmp_addr++;
  }
}

EXTERN_C void lib_init(void *_h, const char *libpath){
	char *argv[100];
	int argc, model;
  intptr_t base_addr;
  size_t len;
  res_table_entry_t *entry;
  hdmi_table_t *hdmi_entry;
  int page_size = getpagesize();

  unlink(LOG_FILE);

	log("SamyGO "LIB_TV_MODELS" lib"LIB_NAME" "LIB_VERSION" - (c) Rapper_skull 2022\n");

  argc = getArgCArgV(libpath, argv);
  void *h = dlopen(0, RTLD_LAZY);
  if(!h)
  {
    char *serr = dlerror();
    log("dlopen error %s\n", serr);
    return;
  }

  patch_adbg_CheckSystem(h);

  model = getTVModel();
  logf("TV Model: %s\n", tvModelToStr(model));

  samyGO_whacky_t_init(h, &hCTX, PROCS_SIZE);

  TDcResolutionTable_m_atExtInResolutionTable = find_resolution_table(h, (unsigned int *)hCTX.TDcResolutionTable_GetReferenceData);

  find_tulip_functions(h, (unsigned int *)hCTX.TDcResolutionTable_GetReferenceData);

#define NEW_HDMI_ENTRIES  2
  if(hCTX.size_rect_table){
    for(int i = 0; i < SIZE_RECT_TABLE_ENTRIES; i++){
      if(hCTX.size_rect_table[i].mode_id == HDMI_SRC_RTM && hCTX.size_rect_table[i].table_addr && hCTX.size_rect_table[i].table_size){
        hdmi_rect_table = (rect_table_t *)hCTX.size_rect_table[i].table_addr;
        hdmi_rect_table_size = hCTX.size_rect_table[i].table_size;
        if(hdmi_rect_table && hdmi_rect_table_size){
          log_found("_ZN21RectangleHelperCommon17mTBL_HdmiSizeRectE", hCTX.size_rect_table[i].table_addr);
          hdmi_rect_table_size_new = hdmi_rect_table_size + NEW_HDMI_ENTRIES;
          hdmi_rect_table_new = malloc(hdmi_rect_table_size_new * sizeof(rect_table_t));
          if(hdmi_rect_table_new){
            memcpy(hdmi_rect_table_new, hdmi_rect_table, hdmi_rect_table_size * sizeof(rect_table_t));
            memcpy(&hdmi_rect_table_new[hdmi_rect_table_size], &rect_table_720x288p, sizeof(rect_table_t));
            memcpy(&hdmi_rect_table_new[hdmi_rect_table_size + 1], &rect_table_1440x288p, sizeof(rect_table_t));
            base_addr = (intptr_t)&hCTX.size_rect_table[i] & -page_size;
            len = (intptr_t)(&hCTX.size_rect_table[i] + 3) - base_addr;
            if(!mprotect((void *)base_addr, len, PROT_READ | PROT_WRITE)){
              hCTX.size_rect_table[i].table_addr = hdmi_rect_table_new;
              hCTX.size_rect_table[i].table_size = hdmi_rect_table_size_new;
              log("Replaced HDMI Size Rect table\n");
            } else {
              log("Failed to replace HDMI Size Rect table\n");
              free(hdmi_rect_table_new);
              hdmi_rect_table_new = NULL;
              hdmi_rect_table_size_new = 0;
            }
          } else {
            log("Failed to replace HDMI Size Rect table\n");
          }
        }
        break;
      }
    }
  }

  base_addr = (intptr_t)TDcResolutionTable_m_atExtInResolutionTable & -page_size;
  len = (intptr_t)(TDcResolutionTable_m_atExtInResolutionTable + RES_TABLE_SIZE) - base_addr;
  if(!mprotect((void *)base_addr, len, PROT_READ | PROT_WRITE)){
    for(entry = TDcResolutionTable_m_atExtInResolutionTable;
      entry < (TDcResolutionTable_m_atExtInResolutionTable + RES_TABLE_SIZE); entry++ ){
        if(entry->id == res_288p.id){
          entry->h_res = res_288p.h_res;
          entry->v_res = res_288p.v_res;
          entry->h_tot = res_288p.h_tot;
          entry->v_tot = res_288p.v_tot;
          entry->flag_interlace = res_288p.flag_interlace;
          entry->framerate = res_288p.framerate;
          log("Replaced resolution\n");
          break;
        }
    }
  } else {
    log("Failed to replace resolution\n");
  }

  base_addr = (intptr_t)hCTX.hdmi_table & -page_size;
  len = (intptr_t)(hCTX.hdmi_table + HDMI_TABLE_SIZE) - base_addr;
  if(!mprotect((void *)base_addr, len, PROT_READ | PROT_WRITE)){
    for(hdmi_entry = hCTX.hdmi_table; hdmi_entry < (hCTX.hdmi_table + HDMI_TABLE_SIZE); hdmi_entry++){
      if(hdmi_entry->id == res_288p_hdmi.id){
        hdmi_entry->h_res = res_288p_hdmi.h_res;
        hdmi_entry->v_res = res_288p_hdmi.v_res;
        hdmi_entry->flag_interlace = res_288p_hdmi.flag_interlace;
        hdmi_entry->h_freq = res_288p_hdmi.h_freq;
        hdmi_entry->v_freq = res_288p_hdmi.v_freq;
        hdmi_entry->h_tot = res_288p_hdmi.h_tot;
        hdmi_entry->v_tot = res_288p_hdmi.v_tot;
        log("Replaced HDMI resolution\n");
        if(hdmi_entry->name && strlen(hdmi_entry->name) >= strlen(res_288p_hdmi.name)){
          base_addr = (intptr_t)hdmi_entry->name & -page_size;
          len = strlen(hdmi_entry->name) + 1;
          if(!mprotect((void *)base_addr, len, PROT_READ | PROT_WRITE)){
            strncpy(hdmi_entry->name, res_288p_hdmi.name, strlen(res_288p_hdmi.name));
            log("Replaced HDMI resolution string\n");
          } else {
            log("Failed to replace HDMI resolution string\n");
          }
        }
        break;
      }
    }
  } else {
    log("Failed to replace HDMI resolution\n");
  }

  if(_hooked){
        log("Removing hooks\n");
        remove_hooks(LIB_HOOKS, ARRAYSIZE(LIB_HOOKS));
        _hooked = 0;
  }

  if(dyn_sym_tab_init(h, dyn_hook_fn_tab, ARRAYSIZE(dyn_hook_fn_tab)) >= 0){
    set_hooks(LIB_HOOKS, ARRAYSIZE(LIB_HOOKS));
    log("Set hooks.\n");
		_hooked = 1;
  }

  log("init done...\n");
  dlclose(h);
}

EXTERN_C void lib_deinit(void *_h)
{
  log(">>> %s\n", __func__);

  if(hCTX.size_rect_table){
    for(int i = 0; i < SIZE_RECT_TABLE_ENTRIES; i++){
      if(hCTX.size_rect_table[i].mode_id == HDMI_SRC_RTM && hCTX.size_rect_table[i].table_addr != hdmi_rect_table){
        // Restore old tables
        hCTX.size_rect_table[i].table_size = hdmi_rect_table_size;
        hCTX.size_rect_table[i].table_addr = hdmi_rect_table;
        break;
      }
    }
  }
  if(hdmi_rect_table_new){
    free(hdmi_rect_table_new);
    hdmi_rect_table_new = NULL;
    hdmi_rect_table_size_new = 0;
  }

  if(_hooked)
    remove_hooks(LIB_HOOKS, ARRAYSIZE(LIB_HOOKS));

  log("<<< %s\n", __func__);
}
