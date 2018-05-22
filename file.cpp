#include<stdio.h>
#include<stdlib.h>
#include<iostream>
using namespace std;
int main1()
{
         const char* filename="client.conf";
  	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		printf("can not open %s,errno = ", filename);
		return 0;
	}
       return 0;
}
