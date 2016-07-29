#include <stdlib.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
  dev_t dev;

  dev = 0;
  mknod("test2.reg", 0666 | S_IFREG, dev);
  unlink("test2.reg");

  dev = makedev(16, 25);
  mknod("test2.chr", S_IFCHR, dev);
  unlink("test2.chr");
  
  mknod("test2.blk", S_IFBLK, dev);
  unlink("test2.blk");

  mknod("test2.fifo", 0600 | S_IFIFO, dev);
  unlink("test2.fifo");

  mknod("test2.sock", 0600 | S_IFSOCK, dev);
  unlink("test2.sock");

  mknod("test.txt", 0600 | S_IFREG, dev);
  unlink("test.txt");

  mknod("test2.txt", 5555555, dev);
  unlink("test2.txt");
  mknod("test2.blk", S_IFBLK, 0);
  unlink("test2.blk");

  return 0;
}
