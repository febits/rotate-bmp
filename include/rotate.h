#ifndef ROTATE_H
#define ROTATE_H

#include "types.h"

#define _B_ 0x42
#define _M_ 0x4d

typedef struct {
  u8 b_magic[2];
  u32 b_filesize; 
  u16 b_reserved[2];
  u32 b_pixelarr_offset;
} __attribute__((packed)) bmp_header;


// BITMAPINFOHEADER
typedef struct {
  u32 b_dib_h_size;  
  i32 b_width;
  i32 b_height;
  u16 b_planes;
  u16 b_bits_per_pixel;
  u32 b_compression;
  u32 b_image_size;
  i32 b_horizontal_res;
  i32 b_vertical_res;
  u32 b_colors_number;
  u32 b_colors_number_impt;
} __attribute__((packed)) bmp_dib_header;

#endif
