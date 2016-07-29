#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  fsync(fd);
  write(fd, "writing to test.txt", 20);
  fsync(fd);
  write(fd, "writing", 3);
  fsync(fd);
  fsync(fd);
  close(fd);
  fsync(fd);
  fsync(-1);
  unlink("test.txt");
  return 0;
}
