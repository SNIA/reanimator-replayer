#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0600);
  struct stat *buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test.txt", buf);
  free(buf);
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("nonexistent.txt", buf);
  free(buf);
  symlink("test.txt",
	  "test2.txt");
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test2.txt", buf);
  free(buf);
  unlink("test2.txt");
  link("test.txt",
       "test2.txt");
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("test2.txt", buf);
  free(buf);
  unlink("test2.txt");
  mkdir("testtmp", 0700);
  buf = (struct stat *) malloc(sizeof(struct stat));
  lstat("testtmp", buf);
  free(buf);
  rmdir("testtmp");
  unlink("test.txt");
  return 0;
}
  
