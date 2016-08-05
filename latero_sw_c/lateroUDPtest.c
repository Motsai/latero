#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PKT_LEN 100

int main(int argc, char const *argv[])
{
	char package[PKT_LEN];
	int sleepTime, ii=0;

	if (argc == 0)
	{
		printf("Specify in microseconds the send period\n");
		return -1;
	}

	sleepTime = atoi(argv[1]);
	printf("%d\n", sleepTime);

	while(1)
	{
		usleep(sleepTime);
		memset(package, ii, PKT_LEN);
		printf("%s", package);
		ii++;
	}

	return 0;
}