#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TEST_BUF_SIZE 4096
#define MAX_READS 2*1024*1024
#define FILE_NAME "myfile"

int main() {
  char buf[TEST_BUF_SIZE];
  int fd, i, numbytes;

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
    if (numbytes < TEST_BUF_SIZE) {
      fprintf(stderr, "Seek count=%d\n", i);
      if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(3);
      }
    }
  }
  close(fd);
  exit(0);
}
