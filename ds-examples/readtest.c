#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(){

  char buf[128];
  int fd, i;

  if((fd = open("my_test.txt", O_RDONLY)) < 0){
    perror("open my_test.txt error");
    return -1;
  }
    
  /*one million read syscalls*/
  for(i = 0; i < 1000000; i++){
    if(read(fd, buf, 128) < 0){ /*read 128 bytes*/
      perror("read");
      return -1;
    }
    if(i%1000 == 0){ /*rewind fd every once in a while*/
      if(lseek(fd, 0, SEEK_SET) < 0){
	perror("lseek");
	return -1;
      }
    }
  }
 
  close(fd);
  return 0;
}
