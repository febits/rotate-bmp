#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "bmp.h"

#define _DEBUG_ 1
#define PREFIX " "

#define error(fmt, ...)                                                        \
  fprintf(stderr, fmt, ##__VA_ARGS__);                                         \
  exit(EXIT_FAILURE);

void bmp_print(bmp_image *img) {
  printf("%s:\n", img->filepath);
  printf(PREFIX "Magic Bytes: %c%c\n", img->h->b_magic[0], img->h->b_magic[1]);
  printf(PREFIX "Filesize: %u\n", img->h->b_filesize);
  printf(PREFIX "Pixel Array Offset: 0x%x\n\n", img->h->b_pixelarr_offset);

  printf(PREFIX "DIB Size: %u\n", img->dib->b_dib_h_size);
  printf(PREFIX "Width: %d\n", img->dib->b_width);
  printf(PREFIX "Height: %d\n", img->dib->b_height);
  printf(PREFIX "Planes: %u\n", img->dib->b_planes);
  printf(PREFIX "Bits Per Pixel: %u\n", img->dib->b_bits_per_pixel);
  printf(PREFIX "Compression: %u\n", img->dib->b_compression);
  printf(PREFIX "Image Size: %u\n", img->dib->b_image_size);
  printf(PREFIX "Horizontal Resolution: %d\n", img->dib->b_horizontal_res);
  printf(PREFIX "Vertical Resolution: %d\n", img->dib->b_vertical_res);
  printf(PREFIX "Colors Number: %u\n", img->dib->b_colors_number);
  printf(PREFIX "Important Colors Number: %u\n\n",
         img->dib->b_colors_number_impt);

  u32 rowsize = ROW_SIZE(img->dib->b_bits_per_pixel, img->dib->b_width);

  printf(PREFIX "Row Size: %u\n", rowsize);
  printf(PREFIX "Pixel Array Size: %u\n", rowsize * abs(img->dib->b_height));
  printf(PREFIX "Padding Size: %lu\n", PADDING_SIZE(img->dib->b_width));
}

void print_pixels(bmp_image *img) {
  printf("\n");
  for (i32 i = 0; i < img->dib->b_width * img->dib->b_height; i++) {
    printf("[%02X %02X %02X]\n", img->pixelarr[i]->r, img->pixelarr[i]->g,
           img->pixelarr[i]->b);
  }
  printf("\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("Usage: %s [file]\n", argv[0]);
  }

  FILE *stream = fopen(argv[1], "rb");

  if (!stream) {
    error("rotate: file doesn't exist or permission denied\n");
  }

  bmp_header header = {0};
  bmp_dib_header dib_header = {0};

  bmp_image img = {argv[1], stream, &header, &dib_header, NULL};

  if (bmp_parse(&img) == BMP_INVALID) {
    error("rotate: invalid bmp image\n");
  }

#if defined(_DEBUG_)
  bmp_print(&img);
#endif

  if (bmp_load_pixelarr(&img) == -1) {
    error("rotate: error on load pixel array\n");
  }

#if defined(_DEBUG_) && _DEBUG_
  print_pixels(&img);
#endif

  char buffer[strlen(argv[1]) + 6];
  snprintf(buffer, strlen(argv[1]) + 5, "out/%s", argv[1]);

  FILE *new_stream = fopen(buffer, "wb");

  if (!new_stream) {
    error("rotate: failed on fopen new stream\n");
  }

  bmp_image new_img = {buffer, new_stream, img.h, img.dib, NULL};

  if (bmp_rotate(&img, &new_img) == -1) {
    error("rotate: failed on rotate()\n");
  }

#if defined(_DEBUG_) && _DEBUG_
  print_pixels(&new_img);
#endif

  if (bmp_to_file(&new_img) == -1) {
    error("rotate: failed on bmp to file\n");
  }

  printf("rotate: %s rotated in 90 degrees -> %s\n", img.filepath,
         new_img.filepath);

  fclose(stream);
  fclose(new_stream);
  bmp_destroy(&img);
  free(new_img.pixelarr);

  return EXIT_SUCCESS;
}
