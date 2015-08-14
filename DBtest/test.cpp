#include "stdio.h"
#include <stdlib.h>
#include <string>
#include <list>
#include <iostream>
#include "CppSQLite3.h"
using namespace std;

int getCameraID(long int * CameraID,const char *path)
{
	list <int> cam;
	int ret = 0;
	sqlite3* db = NULL;
	char sqlbuf[1024];
	char *ErrMsg=0;
	char open_db_result = 0;
	char **pszResult = 0;
	int nRow = 0;
	int nColumn = 0;
	memset(sqlbuf,0,sizeof(sqlbuf));
	
	open_db_result = sqlite3_open(path,&db);
	sprintf(sqlbuf,"select CameraID from vnmp_CameraInfo;");
	ret = sqlite3_get_table(db, sqlbuf, &pszResult, &nRow, &nColumn, &ErrMsg);
	if (ret != SQLITE_OK)
	{
		std::cout << "SQL error " << ErrMsg << std::endl;
		sqlite3_free(ErrMsg);
		ErrMsg = NULL;
		sqlite3_free_table(pszResult);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		int num = 0;
		for (int i = 0; i <= nRow; ++i)
		{
			for (int j = 0; j < nColumn; ++j)
			{
				int n = atoi(*(pszResult + nColumn * i + j));	
				if(n != 0){
					printf("n= %d\n",n);
				    cam.push_back(n);		
				    num++;
				}

			}
		}
		printf("size1 is %d\n",cam.size());
	}
	if (ErrMsg != NULL) sqlite3_free(ErrMsg);
	ErrMsg = NULL;
	sqlite3_free_table(pszResult);
	sqlite3_close(db);
	int a = cam.front();
	printf("a= %d\n",a);
	cam.pop_front(); 
	printf("size2 is %d\n",cam.size());
	int b = cam.front();
	printf("a= %d\n",b);
	return 0;
}
void seturl(string &url)
{
	url = "aaaaaaaaaaaaa";
}

int main(int argc, const char* argv[])
{
    long int ID[1024];
	getCameraID(ID,"MS.sqlite");
	string url;
	seturl(url);
	cout<<url<<endl;

	return 0;
}
