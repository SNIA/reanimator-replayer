#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  link("test.txt", "test2.txt");
  unlink("test2.txt");
  symlink("test.txt", "test2.txt");
  unlink("test2.txt");
  unlink("nonexistent.txt");
  close(fd);
  unlink("test.txt");
  return 0;
}
