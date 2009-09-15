#include <stdio.h>
#include "stack.h"

int main()
{
	Stack s;

	int i, tmp;

	s = stackAlloc(sizeof(int), 5);

	for(i = 0; i < 50; i+=2)
	{
		printf("Push: %d\n", i);
		stackPush(&s, &i);
	}

	for(i = 0; i < 50; i+=2)
	{
		tmp = *((int*)stackPop(&s));
		printf("Pop: %d\n", tmp);
	}

	stackFree(s);
}
