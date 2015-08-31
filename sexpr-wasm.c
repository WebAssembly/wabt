#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [file.wasm]\n", argv[0]);
    return 0;
  }

  const char* filename = argv[1];
  FILE* f = fopen(filename, "rb");
  if (!f) {
    fprintf(stderr, "unable to read %s\n", filename);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  void* data = malloc(fsize);
  if (!data) {
    fprintf(stderr, "unable to alloc %zd bytes\n", fsize);
    return 1;
  }
  if (fread(data, 1, fsize, f) != fsize) {
    fprintf(stderr, "unable to read %zd bytes from %s\n", fsize, filename);
    return 1;
  }
  fclose(f);

  return 0;
}
