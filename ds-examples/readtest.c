#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define TEST_BUF_SIZE	128
#define MAX_READS 1000000 /* number of times to read the file */
#define FILE_NAME	"my_test.random"

int main(){

  char buf[TEST_BUF_SIZE];
  int fd, i, numbytes;

  fd = open(FILE_NAME, O_RDONLY);
  if (fd < 0) {
    perror("open " FILE_NAME " error");
    exit(1);
  }

  /* read 1M till end of file, then seek to start */

  for (i = 0; i < MAX_READS; i++) {
    numbytes = read(fd, buf, TEST_BUF_SIZE);
    if (numbytes < 0) {
      perror("read");
      exit(2);
    }
    if (numbytes < TEST_BUF_SIZE) { /* partial read or EOF, seek to start */
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
