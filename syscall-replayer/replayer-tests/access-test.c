#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  access("test.txt", R_OK);
  access("test.txt", W_OK);
  access("test.txt", X_OK);
  access("test.txt", F_OK);
  close(fd);
  access("nonexistent.txt", F_OK);
  access("nonexistent.txt", W_OK);
  unlink("test.txt");
  return 0;
}
