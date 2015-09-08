#include <stdio.h>
#include "NT88Api.h"

int main(int argc, char **argv)
{
	int nRet = 0;
	char hwID[64] ={0};	
        char *seed = NULL;
        seed = "192.168.1.104";

        nRet = NTFindFirst((char *)seed);
	if(0 != nRet)
	{
		printf("NT88 is not found! return %d\n ", nRet);
		return 1;
	}
        while(1)
        {
            nRet = NTGetHardwareID(hwID);
            if( 0 != nRet)
            {
                printf("Failed to get hardware id! return %d\n ", nRet);
                return 3;
            }
            else
            {
                printf("NT88 is excite!\n ", nRet);
            }
        }

        nRet = NTLogout();
	if( 0 != nRet)
	{
		printf("Failed to log out ! return %d\n ", nRet);
		return 5;
	}

	printf("Finished!\n");
	
	return 0;
}
