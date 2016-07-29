#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  symlink("test.txt", "test2.txt");
  symlink("nonexistent.txt", "alsononexistent.txt");
  close(fd);
  unlink("test2.txt");
  unlink("alsononexistent.txt");
  unlink("test.txt");
  return 0;
}
