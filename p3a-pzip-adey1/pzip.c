#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define THROW(f_, ...)                                                         \
  do {                                                                         \
    {                                                                          \
      fprintf(stderr, (f_), ##__VA_ARGS__);                                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

struct zip {
  int to_print;
  int count;
  char tracker;
};

int main(int argc, char **argv) {
  struct zip z = {};
  struct stat sb = {};

  if (argc == 1) {
    THROW("my-zip: file1 [file2 ...]\n");
  }

  for (size_t i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd == -1) {
      THROW("File not found!\n");
    }

    int status = fstat(fd, &sb);
    if (status == -1) {
      THROW("Error: %s\n", strerror(errno));
    }

    char const *const mapped =
        mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
      THROW("mmap failed: %s\n", strerror(errno));
    }

    z.to_print = mapped[0];
    size_t len = strlen(mapped);
    for (size_t j = 0; j < len; j++) {
      z.tracker = mapped[j];
      if (z.to_print != z.tracker) {
        fwrite(&z.count, sizeof(int), 1, stdout);
        fputc(z.to_print, stdout);
        z.count = 1;
        z.to_print = z.tracker;
      } else {
        z.count += 1;
      }
    }

    if (close(fd) == -1) {
      THROW("close failed: %s\n", strerror(errno));
    }
  }

  fwrite(&z.count, sizeof(int), 1, stdout);
  fputc(z.to_print, stdout);

  return EXIT_SUCCESS;
}