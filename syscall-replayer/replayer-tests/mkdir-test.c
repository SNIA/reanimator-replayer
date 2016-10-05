#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  mkdir("testtmp", S_IROTH | S_IWOTH);
  rmdir("testtmp");
  mkdir("testtmp", 0600);
  rmdir("testtmp");
  mkdir("testtmp", 55555555);
  rmdir("testtmp");
  mkdir("testtmp", S_IRWXU | S_IRWXG);
  rmdir("testtmp");
  return 0;
}
