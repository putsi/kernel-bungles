#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_LEN 2048
static char recvBuffer[BUFFER_LEN];

int main() {
	int ret, fd;
	char msg[BUFFER_LEN];
	printf("Opening the rot device\n");
	fd = open("/dev/rot", O_RDWR);
	if (fd < 0) {
		perror("Could not open the device!");
		return errno;
	}

	printf("String that will be rotated: ");
	scanf("%[^\n]%*c", msg);
	int msgLen = strlen(msg);
	if (msgLen > BUFFER_LEN) {
		printf("Too long msg, maximum size is %d", BUFFER_LEN);
	}

	printf("Writing string to device.\n");
	ret = write(fd, msg, msgLen);
	if (ret < 0) {
		perror("Could not send the string to the device!");
		return errno;
	}

	printf("Press any key to read the string from the device.\n");
	getchar();

	ret = read(fd, recvBuffer, BUFFER_LEN);
	if (ret < 0) {
		perror("Could not read the string from the device!");
		return errno;
	}

	printf("Received message: [%s]\n", recvBuffer);
	printf("\n");
	return 0;
}
