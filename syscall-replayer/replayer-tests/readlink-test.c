#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);

  symlink("test.txt", "test2.txt");

  char *buf = (char *)malloc(10);
  readlink("test2.txt", buf, 10);
  free(buf);

  buf = (char *)malloc(3);
  readlink("test2.txt", buf, 3);
  free(buf);

  buf = (char *)malloc(10);
  readlink("nonexistent.txt", buf, 10);
  free(buf);

  mkdir("testtmp", 0600);

  buf = (char *)malloc(10);
  readlink("testtmp", buf, 10);
  free(buf);

  rmdir("testtmp");

  symlink("test2.txt", "test3.txt");

  buf = (char *)malloc(10);
  readlink("test3.txt", buf, 10);
  free(buf);

  unlink("test2.txt");

  buf = (char *)malloc(10);
  readlink("test3.txt", buf, 10);
  free(buf);

  link("test.txt", "test2.txt");

  buf = (char *)malloc(10);
  readlink("test2.txt", buf, 10);
  free(buf);

  unlink("test3.txt");
  unlink("test2.txt");
  close(fd);
  unlink("test.txt");
  return 0;
}
