#include <cstdio>
#include <iostream>
extern "C"
void printNum(long long x)
{
		printf("%d\n",x);
}
extern "C"
void printStr(char * str)
{
		printf("%s\n",str);
}

