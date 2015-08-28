#include "stdio.h"
#include "string.h"
#include <time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
	int ret = -1;
	int fd;
	char buffer[100]= {0};
	int n = 0;
	char file[20] ={0};
	while(1)
	{
	    ret = system("ps -C demo");
	   if(ret != 0)
	    {
				n++;
				time_t timep;
				timep = time(NULL);
				printf("%s\n", ctime(&timep));
				sprintf(buffer,"%s %s","stop at",ctime(&timep));
				sprintf(file,"%s-%d","log",n);
				fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
				write(fd , buffer, 100);
				close(fd);
				break;
	   }
	    sleep(30);
	}
	return 0;
}
