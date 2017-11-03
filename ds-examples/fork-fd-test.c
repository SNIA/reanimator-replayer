#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	int fd, ret;
	char buff[] = "This is dummy text.\n";

	fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

	ret = fork();
	if (ret == -1) {
		fprintf(stderr, "fork() failed\n");
		exit(EXIT_FAILURE);
	}

	if (ret == 0) {
		// Parent process closes fd.
		close(fd);
	} else {
		// Ensure parent closes fd (concurrency).
		sleep(1);

		// Child process then uses fd.
		if (write(fd, buff, strlen(buff)) < 0) {
			fprintf(stderr, "write() failed\n");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
