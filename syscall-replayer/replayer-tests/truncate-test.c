#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  int fd = open("truncatetest.txt", O_CREAT, 0600);
  write(fd, "writing to truncatetest.txt", 50);
  truncate("truncatetest.txt", 5);
  truncate("truncatetest.txt", 500);
  truncate("nonexistent.txt", 5);
  close(fd);
  unlink("truncatetest.txt");
  return 0;
}
