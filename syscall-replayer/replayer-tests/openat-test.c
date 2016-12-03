#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {
  char * path;
  int fd = openat(AT_FDCWD, "test.txt", O_CREAT, 0666);
  close(fd);
  openat(AT_FDCWD, "test.txt", O_APPEND | O_RDWR);
  unlink("test.txt");
  fd = openat(-1, "test.txt", O_CREAT, 0666);
  close(fd);
  fd = openat(AT_FDCWD, path, 0);
  fd = openat(AT_FDCWD, "test.txt", 6625436, 24661);
  close(fd);
  return 0;
}
