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
