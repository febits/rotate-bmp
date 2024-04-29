#include <stdio.h>
#include <stdlib.h>

#include "rotate.h"

int main(int argc, char **argv) {

  printf("%zu\n", sizeof(bmp_header));
  printf("%zu\n", sizeof(bmp_dib_header));

  return EXIT_SUCCESS;
}
