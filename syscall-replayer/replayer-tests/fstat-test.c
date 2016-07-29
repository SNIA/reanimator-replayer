#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  struct stat *buf = (struct stat *) malloc(sizeof(struct stat));
  fstat(fd, buf);
  free(buf);
  fstat(-1, buf);
  close(fd);
  symlink("test.txt",
	  "test2.txt");
  fd = open("test2.txt", O_APPEND);
  buf = (struct stat *) malloc(sizeof(struct stat));
  fstat(fd, buf);
  free(buf);
  close(fd);
  unlink("test2.txt");
  unlink("test.txt");
  return 0;
}
