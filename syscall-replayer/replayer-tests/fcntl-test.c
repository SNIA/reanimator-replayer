#define _GNU_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
  int fd = open("test.txt", O_CREAT, 0777);

  int fd2 = fcntl(fd, F_DUPFD, 7);
  close(fd2);

  int fd_flag = fcntl(fd, F_GETFD);
  fcntl(fd, F_SETFD, fd_flag);

  int status_flag = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, status_flag);

  int pid = fcntl(fd, F_GETOWN);
  fcntl(fd, F_SETOWN, pid);

  struct flock *lock = malloc(sizeof(struct flock));
  lock->l_pid = pid;
  lock->l_len = 50;
  lock->l_type = F_WRLCK;
  lock->l_whence = SEEK_CUR;
  fcntl(fd, F_GETLK, lock);

  free(lock);
  lock = malloc(sizeof(struct flock));

  lock->l_pid = pid;
  lock->l_len = 50;
  lock->l_whence = SEEK_CUR;
  lock->l_type = F_WRLCK;
  fcntl(fd, F_SETLK, lock);

  fcntl(fd, F_GETLK, NULL);
  fcntl(-1, F_GETLK, lock);

  int lease = fcntl(fd, F_GETLEASE);
  fcntl(fd, F_SETLEASE, F_RDLCK);

  free(lock);
  lock = malloc(sizeof(struct flock));

  lock->l_pid = pid;
  lock->l_len = 50;
  lock->l_whence = SEEK_CUR;
  lock->l_type = F_UNLCK;
  fcntl(fd, F_SETLK, lock);

  free(lock);

  int sig = fcntl(fd, F_GETSIG);
  fcntl(fd, F_SETSIG, sig);

  fcntl(fd, 55626, 739);

  close(fd);
  unlink("test.txt");
  return 0;
}
