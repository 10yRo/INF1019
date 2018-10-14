#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int i = 0;
	while(i < 3) {
		printf("Prog1\n");
		sleep(2);
		i++;
	}
	return 0;
}