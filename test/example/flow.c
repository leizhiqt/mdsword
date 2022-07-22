#include "stdio.h"
void fun()
{
	int b=2;
	int *p=&b;
	p = p +5;
	*p = *p+7;
}
int main()
{
	int a=10;
	fun();
	a=100;
//	a=200;	
	printf("a=%d\n",a);	
// printf("%lu %lu\n",sizeof(s1_t),sizeof(int));
}
