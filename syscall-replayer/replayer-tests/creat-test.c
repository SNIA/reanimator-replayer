#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int fd = creat("test2.txt", S_IRGRP | S_IWGRP | S_IRUSR | S_IWUSR);
  close(fd);
  unlink("test2.txt");
  fd = creat("test2.txt", 55555555);
  close(fd);
  unlink("test2.txt");
  creat("", 0);
  return 0;
}
