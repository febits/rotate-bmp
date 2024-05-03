#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rotate.h"

#define _DEBUG_ 1
#define PREFIX " "

#define error(fmt, ...)                                                        \
  fprintf(stderr, fmt, ##__VA_ARGS__);                                         \
  exit(EXIT_FAILURE);

void bmp_destroy(bmp_image *img) {
  for (i32 i = 0; i < img->dib->b_width * img->dib->b_height; i++) {
    free(img->pixelarr[i]);
  }
  free(img->pixelarr);
}

enum bmp_parse_status bmp_parse(bmp_image *img) {
  if (fread(img->h, 1, sizeof(bmp_header), img->stream) != sizeof(bmp_header)) {
    return BMP_INVALID;
  }

  fseek(img->stream, sizeof(bmp_header), SEEK_SET);

  if (fread(img->dib, 1, sizeof(bmp_dib_header), img->stream) !=
      sizeof(bmp_dib_header)) {
    return BMP_INVALID;
  }

  if (!(img->h->b_magic[0] == _B_ && img->h->b_magic[1] == _M_)) {
    return BMP_INVALID;
  }

  if (!(img->h->b_filesize && img->h->b_pixelarr_offset)) {
    return BMP_INVALID;
  }

  fseek(img->stream, 0, SEEK_SET);

  return BMP_VALID;
}

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

int bmp_load_pixelarr(bmp_image *img) {
  img->pixelarr =
      calloc(img->dib->b_width * img->dib->b_height, sizeof(pixel *));

  if (!img->pixelarr) {
    return -1;
  }

  u32 row_size = ROW_SIZE(img->dib->b_bits_per_pixel, img->dib->b_width);
  u32 padding = PADDING_SIZE(img->dib->b_width);

  for (i32 i = 0; i < img->dib->b_height * img->dib->b_width; i++) {
    img->pixelarr[i] = malloc(sizeof(pixel));

    if (!img->pixelarr[i]) {
      return -1;
    }
  }

  fseek(img->stream, img->h->b_pixelarr_offset, SEEK_SET);

  for (i32 i = 0, k = 0; i < img->dib->b_height; i++) {
    for (u32 j = 0; j < row_size - padding; j += BYTES_PER_PIXEL, k++) {
      fread(img->pixelarr[k], sizeof(u8), BYTES_PER_PIXEL, img->stream);
    }
    fseek(img->stream, padding, SEEK_CUR);
  }

  return 1;
}

static void print_pixels(bmp_image *img) {
  printf("\n");
  for (i32 i = 0; i < img->dib->b_width * img->dib->b_height; i++) {
    printf("[%02X %02X %02X]\n", img->pixelarr[i]->r, img->pixelarr[i]->g,
           img->pixelarr[i]->b);
  }
  printf("\n");
}

int bmp_rotate(bmp_image *img, bmp_image *new_img) {
  new_img->pixelarr =
      calloc(img->dib->b_width * img->dib->b_height, sizeof(pixel *));

  if (!new_img->pixelarr) {
    return -1;
  }

  for (i32 i = 0; i < img->dib->b_height; i++) {
    for (i32 j = 0; j < img->dib->b_width; j++) {
      new_img->pixelarr[j * img->dib->b_height + img->dib->b_height - i - 1] =
          img->pixelarr[i * img->dib->b_width + j];
    }
  }

  return 1;
}

static void swap(i32 *v1, i32 *v2) {
  i32 tmp = *v1;
  *v1 = *v2;
  *v2 = tmp;
}

int bmp_to_file(bmp_image *img) {
  u32 row_size = ROW_SIZE(img->dib->b_bits_per_pixel, img->dib->b_width);
  u32 padding_size = PADDING_SIZE(img->dib->b_width);
  u32 pixelarr_size = row_size * abs(img->dib->b_height);

  img->h->b_filesize =
      sizeof(bmp_header) + sizeof(bmp_dib_header) + pixelarr_size;
  img->h->b_pixelarr_offset = sizeof(bmp_header) + sizeof(bmp_dib_header);

  img->dib->b_dib_h_size = sizeof(bmp_dib_header);
  swap(&img->dib->b_width, &img->dib->b_height);

  if (fwrite(img->h, 1, sizeof(bmp_header), img->stream) !=
      sizeof(bmp_header)) {
    return -1;
  }

  if (fwrite(img->dib, 1, sizeof(bmp_dib_header), img->stream) !=
      sizeof(bmp_dib_header)) {
    return -1;
  }

  u8 padding[3] = {0, 0, 0};

  for (i32 i = 0; i < img->dib->b_width * img->dib->b_height - 1;
       i += img->dib->b_width) {
    for (i32 j = i; j < i + img->dib->b_width; j++) {
      fwrite(img->pixelarr[j], 1, sizeof(pixel), img->stream);
    }
    fwrite(padding, sizeof(u8), padding_size, img->stream);
  }

  return 1;
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

  print_pixels(&img);

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

  print_pixels(&new_img);

  if (bmp_to_file(&new_img) == -1) {
    error("rotate: failed on bmp to file\n");
  }

  fclose(stream);
  fclose(new_stream);
  bmp_destroy(&img);
  free(new_img.pixelarr);
  return EXIT_SUCCESS;
}
