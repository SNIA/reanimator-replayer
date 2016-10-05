#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  rename("test.txt", "test2.txt");
  rename("test2.txt", "test.txt");
  rename("nonexistent.txt", "stillnonexistent.txt");
  rename(NULL, NULL);
  close(fd);
  unlink("test.txt");
  return 0;
}
