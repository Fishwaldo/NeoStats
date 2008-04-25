#include "stdafx.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include "windows.h"

char* getftime(char *fname)
{
	char *rtn = new char[99];
	char modt[99];
	struct _stat buf;
	_stat( fname, &buf );
	strcpy(modt, ctime( &buf.st_atime ));
	strcpy(rtn,modt);
	return rtn;
}


long getfnoc(FILE * pFile)
{
	long fnoc=0;
	while(!feof(pFile))
	{
		fgetc(pFile);
		fnoc++;
	}
	fseek( pFile, 0, SEEK_SET);
	return fnoc;
}



void tailmain(int argc, char *argv[])
{

	FILE *pFile;

	if(argc<2){printf("Usage: Tail [filepath]\n");return;}
	
	pFile =fopen (argv[1],"r");
	if(pFile==NULL){printf("File Status: NULL\n");return;}

	long fnoc = getfnoc(pFile);
	fclose(pFile);

	char *modt = getftime(argv[1]);

	while(TRUE)
	{
		char *tmp = getftime(argv[1]);

		if(strcmp(modt,tmp)!=0)
		{
			strcpy(modt,tmp);

			pFile =fopen (argv[1],"r");
			long fnoccur = getfnoc(pFile);

			long charno = 0;

			while(TRUE)
			{
				char c=fgetc(pFile);
				if(++charno>=fnoc)printf("%c",c);
				if(feof(pFile) || charno==fnoccur-1)break;
			}

			fclose (pFile);

			fnoc=fnoccur;
			
		}

		Sleep(100);
	}

}