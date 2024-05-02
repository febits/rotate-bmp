#ifndef ROTATE_H
#define ROTATE_H

#include <stdio.h>
#include "types.h"

#define _B_ 0x42
#define _M_ 0x4d

#define BYTES_PER_PIXEL 3
#define ROW_SIZE(bpp, width) (((((bpp) * (width) + 31) / 32) * 4))
#define PADDING_SIZE(width) ((4 - ((sizeof(pixel)) * (width)) % 4) % 4)

enum bmp_parse_status { BMP_VALID, BMP_INVALID };

typedef struct {
  u8 b, g, r;
} __attribute__((packed)) pixel;

typedef struct {
  u8 b_magic[2];
  u32 b_filesize;
  u16 _b_reserved_[2];
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

typedef struct {
  const char *filepath;
  FILE *stream;
  bmp_header *h;
  bmp_dib_header *dib;

  pixel **pixelarr;
} bmp_image;

#endif
