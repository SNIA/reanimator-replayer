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

#include<bits/stdc++.h>
#include <stdio.h> 
#include <sys/types.h> 
#include<stdlib.h>
#include<unistd.h>

using namespace std;

void testfork() {
	fork(); 
	fork(); 
	fork(); 
	printf("hello\n"); 
}

void testVfork() {
    int n =10;
    pid_t pid = vfork(); //creating the child process
    if (pid == 0)          //if this is a chile process
    {
        printf("Child process started\n");
	char *args[] = {"Hello", "C", "Programming", NULL};
    	execv("./hello", args);
        printf("Child process finished\n");
	
    }
    else//parent process execution
    {
        printf("Now i am coming back to parent process\n");
    }
    printf("value of n: %d \n",n); //sample printing to check "n" value
}

int main() {
//	testfork();
	testVfork();
	return 0;
}
