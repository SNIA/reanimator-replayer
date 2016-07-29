#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main() {
  struct timeval acc;
  struct timeval mod;
  struct timeval tv[2];
  const char * notpath;

  int fd = open("test.txt", O_CREAT, 0600);

  gettimeofday(&acc, NULL);
  struct timespec sleep;
  sleep.tv_sec = 0;
  sleep.tv_nsec = 500;
  nanosleep(&sleep, NULL);
  gettimeofday(&mod, NULL);

  tv[0] = acc;
  tv[1] = mod;
  utimes("test.txt", tv);

  nanosleep(&sleep, NULL);

  utimes(notpath, tv);
  utimes("nonexistent.txt", tv);
  utimes("test.txt", NULL);

  gettimeofday(&acc, NULL);
  gettimeofday(&mod, NULL);
  tv[0] = acc;
  tv[1] = mod;
  utimes("test.txt", tv);

  close(fd);
  unlink("test.txt");
  return 0;
}
