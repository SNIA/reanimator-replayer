/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 */

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
