#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main(){
    int fd[2],nbytes,status;
    pid_t childpid,endpid;
    char string[] = "hello world!\n";
    char readbuffer[80];

    pipe(fd);

    if ((childpid = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (childpid == 0) {
    /* Child process closes up input side of pipe */
        close(fd[0]);
        printf("In child process\n");
        write(fd[1],string,strlen(string)+1);
        exit(0);
    } else {
    /* Parent process closes up output side of pipe */
        close(fd[1]);
        nbytes = read(fd[0],readbuffer,sizeof(readbuffer));
        printf("Received string: %s", readbuffer);
        exit(0);
    }
}
