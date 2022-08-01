#ifndef __288P_SUPPORT_H__
#define __288P_SUPPORT_H__

#include <stdint.h>

#define RES_TABLE_SIZE          114
#define HDMI_TABLE_SIZE         72
#define SIZE_RECT_TABLE_ENTRIES 6
#define PICTURE_MODES           7
#define COMPONENT_SRC_RTM       4u
#define HDMI_SRC_RTM            6u
#define RES_ID_720x288P         28u
#define RES_ID_1440x288P        23u
#define RES_ID_720x576I         21u
#define TULIP_ID_720x576I       13u
#define RTM_ID_720x288P         28u
#define RTM_ID_1440x288P        23u

typedef struct {
  uint32_t  id;
  uint16_t  h_res;
  uint16_t  v_res;
  uint8_t   flag_interlace;
  uint8_t   __padding[3];
  uint32_t  h_freq;
  uint32_t  v_freq;
  uint16_t  h_tot;
  uint16_t  v_tot;
  char*     name;
} hdmi_table_t;

typedef struct {
  uint32_t  id;
  uint16_t  h_res;
  uint16_t  v_res;
  uint16_t  h_tot;
  uint16_t  v_tot;
  uint8_t   flag_interlace;
  uint8_t   __padding[3];
  uint32_t  framerate;
} res_table_entry_t;

typedef struct {
  uint16_t  h_res;
  uint16_t  v_res;
  float     v_freq;
  uint8_t   flag_interlace;
  uint8_t   __padding;
  uint16_t  aspect_ratio;
  uint32_t  h_tot;
  uint32_t  v_tot;
  uint32_t  __unknown[3];
} float_res_t;

typedef struct {
  uint32_t  starting_pixel;
  uint32_t  starting_row;
  uint32_t  cut_pixels;
  uint32_t  cut_rows;
} rectangle_t;

typedef struct {
  uint32_t    res_id;
  rectangle_t rectangles[PICTURE_MODES];
} rect_table_t;

typedef struct {
  uint32_t      mode_id;
  rect_table_t* table_addr;
  uint32_t      table_size;
} size_rect_table_t;

#endif // #ifndef __288P_SUPPORT_H__
