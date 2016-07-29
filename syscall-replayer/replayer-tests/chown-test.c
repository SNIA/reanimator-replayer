#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  chown("test.txt", 1317, 500);
  chown("test.txt", 3730, 1026);
  chown("test.txt", -1, -1);
  chown("test.txt", 3730, 500);
  chown("nonexistent.txt", 3730, 500);
  mkdir("testtmp", 0600);
  chown("testtmp", 3730, 1026);
  symlink("test.txt", "test2.txt");
  chown("test2.txt", 3730, 1026);
  rmdir("testtmp");
  unlink("test2.txt");
  close(fd);
  unlink("test.txt");
  return 0;
}
