#include<stdio.h>
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<stdlib.h>
#include<tchar.h>
#define MAX_STR_LEN		256

int _tmain(int argc, TCHAR* argv[])
{
	_wsetlocale(LC_ALL, L"Korean") ;
	
	if(argc <2) return -1;
	wchar_t directory[MAX_STR_LEN] = L"";
		const DWORD maxBuf = 1024;
		wchar_t buf[maxBuf] = L"";
		DWORD readedByte = 0;
		for(int i =0; i<wcslen( argv[1] ); i++)
			directory[i] = (wchar_t)argv[1][i];
		HANDLE hFile = CreateFile(directory,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			wprintf(L"Error Type-Open file : %s",directory); // 파일이 안열린것
		}
		else
		{
			do{
				if(ReadFile(hFile,buf,maxBuf,&readedByte,NULL) == false)
				{
					wprintf(L"Error Type-Read file : %s",directory); // 파일 못읽은것
					break;
				}
				printf("%s",buf); // 유니코드 안될때 있으니까..
			}while(readedByte==maxBuf);
		}
		CloseHandle(hFile);
	return 1;
}	 
