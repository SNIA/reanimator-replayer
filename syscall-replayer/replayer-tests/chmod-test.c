#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = ("test.txt", O_CREAT, 0600);
  chmod("test.txt", S_IRUSR);
  chmod("test.txt", 4262354);
  chmod("nonexistent.txt", 0600);
  mkdir("testtmp", 0600);
  chmod("testtmp", 0400);
  chmod("testtmp", 02346574);
  rmdir("testtmp");
  close(fd);
  unlink("test.txt");
  return 0;
}
