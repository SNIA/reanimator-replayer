#include <utime.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  struct utimbuf times;

  int fd = open("test.txt", O_CREAT, 0600);
  utime("test.txt", &times);
  times.actime = time(NULL);
  times.modtime = time(NULL);
  utime("test.txt", &times);
  utime ("nonexistent.txt", &times);
  utime("test.txt", NULL);
  struct timespec sleeptime;
  sleeptime.tv_sec = 2;
  sleeptime.tv_nsec = 0;
  nanosleep(&sleeptime, NULL);
  times.actime = time(NULL);
  times.modtime = time(NULL);
  utime("test.txt", &times);

  close(fd);
  unlink("test.txt");
  return 0;
}
