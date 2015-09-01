#include "stdio.h"
#include "string.h"
#include <time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>

void getTime(char *buffer)
{
	time_t timep;
	timep = time(NULL);
	printf("%s\n", ctime(&timep));
	sprintf(buffer,"%s %s","stop at",ctime(&timep));
}

int main()
{
	int ret = -1;
	FILE *log_file;
	char buff[50]= {0};

	while(1)
	{
	   ret = system("ps -C demo");
	   if(ret != 0)
	   {
			getTime(buff);
			log_file = fopen("/home/hrd/MediaServer/Server/server.log","a+");
			fwrite(buff,strlen(buff),1,log_file);
			fclose(log_file);

			system("cd /home/hrd/MediaServer/Server/ && ./demo");
	   }
	    sleep(30);
	}
	return 0;
}

/*
terminate called after throwing an instance of 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::lock_error> >'
  what():  boost: mutex lock failed in pthread_mutex_lock: Invalid argument
Aborted (core dumped)

*/
