#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  int newfd = dup(fd);
  close(newfd);
  close(fd);
  newfd = dup(-1);
  close(newfd);
  newfd = dup(5000);
  close(newfd);
  unlink("test.txt");
  return 0;
}
