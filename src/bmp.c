#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "types.h"

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
int bmp_to_file(bmp_image *img) {
  u32 row_size = ROW_SIZE(img->dib->b_bits_per_pixel, img->dib->b_width);
  u32 padding_size = PADDING_SIZE(img->dib->b_width);
  u32 pixelarr_size = row_size * abs(img->dib->b_height);

  img->h->b_filesize =
      sizeof(bmp_header) + sizeof(bmp_dib_header) + pixelarr_size;
  img->h->b_pixelarr_offset = sizeof(bmp_header) + sizeof(bmp_dib_header);

  img->dib->b_dib_h_size = sizeof(bmp_dib_header);

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
