#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TEST_BUF_SIZE 4096
#define MAX_READS 2 * 1024 * 1024
#define FILE_NAME "myfile"

uint64_t getRandom(uint64_t lower, uint64_t upper) {
  return (rand() % (upper - lower + 1)) + lower;
}

int main() {
  char buf[TEST_BUF_SIZE];
  int fd, i, numbytes;
  srand(time(0));
  fd = open(FILE_NAME, O_RDONLY);
  if (fd < 0) {
    perror("open " FILE_NAME " error");
    exit(1);
  }

  for (i = 0; i < MAX_READS; i++) {
    numbytes = read(fd, buf, TEST_BUF_SIZE);
    if (numbytes < 0) {
      perror("read");
      exit(2);
    }
    uint64_t upperBound = 8 * 1024 * 1024;
    upperBound *= 1024;
    uint64_t random = getRandom(0, upperBound);
    if (lseek(fd, random, SEEK_SET) < 0) {
      perror("lseek");
      exit(3);
    }
  }
  close(fd);
  exit(0);
}
