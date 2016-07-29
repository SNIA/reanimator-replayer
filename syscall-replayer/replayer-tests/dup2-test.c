#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  int newfd = dup2(fd, 11);
  int newfd2 = dup2(newfd, 17);
  struct stat *buf = (struct stat *) malloc(sizeof(struct stat));
  fstat(newfd2, buf);
  free(buf);
  close(newfd2);
  newfd2 = dup2(newfd, newfd);
  newfd2 = dup2(-1, 11);
  close(newfd2);
  close(newfd);
  newfd = dup2(fd, 0);
  close(newfd);
  close(fd);
  unlink("test.txt");
  return 0;
}
