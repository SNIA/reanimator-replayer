#include <stdlib.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <fcntl.h>

int main() {
  int fd = open(".", O_DIRECTORY);
  int count = 10000;
  struct linux_dirent *buf = malloc(count);
  syscall(SYS_getdents, fd, buf, count);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, -1, buf, count);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, fd, buf, 2);
  free(buf);
  buf = malloc(count);
  syscall(SYS_getdents, fd, buf, -1);
  free(buf);
  syscall(SYS_getdents, fd, NULL, count);
  close(fd);
  return 0;
}
