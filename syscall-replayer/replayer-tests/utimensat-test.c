#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
  struct timespec ts[2];
  const char *path = NULL;
  int fd = creat("test.txt", 0666);
  ts[0].tv_sec = 500;
  ts[0].tv_nsec = 500;
  ts[1].tv_sec = 400;
  ts[1].tv_nsec = 400;
  syscall(SYS_utimensat, fd, NULL, ts, 0);
  const char *path1 = "test.txt";
  utimensat(AT_FDCWD, path1, ts, AT_SYMLINK_NOFOLLOW);
  utimensat(AT_FDCWD, path1, NULL, 0);
  utimensat(fd, "", ts, 0);
  unlink("test.txt");
  return 0;
}
