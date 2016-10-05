#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  open("test.txt", O_CREAT, 0600);
  link("test.txt", "test2.txt");
  link("nonexistent.txt", "stillnonexistent.txt");
  unlink("test2.txt");
  unlink("test.txt");
  return 0;
}
