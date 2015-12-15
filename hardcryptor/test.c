#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_LEN 2048
#define IOCTL_SET_KEY 0
#define IOCTL_GET_KEY 1
#define IOCTL_INVALID_CALL 6
#define OLDKEY "thisIsOldKeyAndNowLongEnough"
#define NEWKEY "newKeyHereAndThisIsAlsoLongEnough"

static void
print_buf(const unsigned char *buf, size_t buf_len, const char *footer)
{
	size_t i = 0;
	for (i = 0; i < buf_len; ++i)
		fprintf(stdout, "%02X%s", buf[i],
			(i + 1) % 16 == 0 ? " " : " ");
	printf("%s\n", footer);
}

int main()
{
	int ret, fd;
	char msg[BUFFER_LEN];
	char enc[BUFFER_LEN];
	char dec[BUFFER_LEN];
	char buf[BUFFER_LEN];
	fd = open("/dev/hcry", O_RDWR);
	if (fd < 0) {
		perror("Could not open the device!");
		return errno;
	}

	ioctl(fd, IOCTL_SET_KEY, OLDKEY);
	ioctl(fd, IOCTL_GET_KEY, buf);
	printf("Current encryption key: %s\n\n", buf);

	printf("\nString that will be encrypted&decrypted: ");
	scanf("%[^\n]%*c", msg);
	int msgLen = strlen(msg);
	if (msgLen > BUFFER_LEN) {
		printf("Too long msg, maximum size is %d", BUFFER_LEN);
	}
	printf("\n");

	ret = write(fd, msg, msgLen);
	if (ret < 0) {
		perror("Could not send the string to the device!");
		close(fd);
		return errno;
	}

	ret = read(fd, enc, msgLen);
	if (ret < 0) {
		perror("Could not read the binary from the device!");
		close(fd);
		return errno;
	}

	ret = write(fd, enc, 2);
	if (ret < 0) {
		perror("Could not send the string to the device!");
		close(fd);
		return errno;
	}

	ret = read(fd, dec, BUFFER_LEN);
	if (ret < 0) {
		perror("Could not read the string from the device!");
		close(fd);
		return errno;
	}

	print_buf(msg, msgLen, " <-- Original  message (HEX)");
	print_buf(enc, msgLen, " <-- Encrypted message (HEX)");
	print_buf(dec, msgLen, " <-- Decrypted message (HEX)");

	printf("%s <-- Original  message\n", msg);
	printf("%s <-- Encrypted message\n", enc);
	printf("%s <-- Decrypted message\n", dec);
	printf("\n");

	ioctl(fd, IOCTL_GET_KEY, buf);
	printf("Current encryption key: %s\n", buf);
	printf("Setting new encryption key via IOCTL.\n");
	ioctl(fd, IOCTL_SET_KEY, NEWKEY);
	ioctl(fd, IOCTL_GET_KEY, buf);
	printf("Current encryption key: %s\n\n", buf);

	ret = write(fd, msg, msgLen);
	if (ret < 0) {
		perror("Could not send the string to the device!");
		close(fd);
		return errno;
	}

	ret = read(fd, enc, BUFFER_LEN);
	if (ret < 0) {
		perror("Could not read the binary from the device!");
		close(fd);
		return errno;
	}

	ret = write(fd, enc, msgLen);
	if (ret < 0) {
		perror("Could not send the string to the device!");
		close(fd);
		return errno;
	}

	ret = read(fd, dec, BUFFER_LEN);
	if (ret < 0) {
		perror("Could not read the string from the device!");
		close(fd);
		return errno;
	}

	print_buf(msg, msgLen, " <-- Original  message (HEX)");
	print_buf(enc, msgLen, " <-- Encrypted message (HEX)");
	print_buf(dec, msgLen, " <-- Decrypted message (HEX)");

	printf("%s <-- Original  message\n", msg);
	printf("%s <-- Encrypted message\n", enc);
	printf("%s <-- Decrypted message\n", dec);
	printf("\n");

	ioctl(fd, IOCTL_INVALID_CALL);

	close(fd);

	return 0;
}
