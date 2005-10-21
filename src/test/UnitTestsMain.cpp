#include <stdio.h>
#include <states\ActionQueue.h>

int
main(int argc, char *argv[])
{
	// Run the self test's that we have
	printf("Testing ActionQueue.....");
	fflush(stdout);
	ActionQueue::SelfTest();
	printf("passed!\n");
}
