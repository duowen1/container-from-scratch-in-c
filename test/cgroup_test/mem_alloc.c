#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BLOCK_SIZE (1024*1024)

int main(int argc, char **argv)
{

	int thr, i;
	char *p1;

	if (argc != 2) {
		printf("Usage: mem_alloc <num (MB)>\n");
		exit(0);
	}

	thr = atoi(argv[1]);

	printf("Allocating," "set to %d Mbytes\n", thr);
	sleep(5);
	for (i = 0; i < thr; i++) {
		p1 = malloc(BLOCK_SIZE);
        if(p1!=NULL) memset(p1, i, BLOCK_SIZE);
        else printf("allocate fail\n");
	}

    printf("Allocated %dM Memory\n", thr);

	sleep(600);

	return 0;
}