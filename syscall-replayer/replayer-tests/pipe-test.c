#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  int pipefd[2];
  pipe(pipefd);
  pipe(NULL);
  int pipefd2[2];
  pipe(pipefd2);
  close(pipefd[0]);
  close(pipefd[1]);
  close(pipefd2[0]);
  close(pipefd2[1]);
  return 0;
}
