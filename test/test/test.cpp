#include<stdio.h>
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<stdlib.h>
#include<tchar.h>
#define MAX_STR_LEN		256
#define	CMD_TOKEN_NUM	10
#define MAX_THREADS (1024*10)

DWORD WINAPI func( LPVOID lpParam)
{
	wchar_t data[MAX_PATH] = L"";
	int i=0;
	while(*((wchar_t*)lpParam+i) != NULL)
	{
		data[i++] = *((wchar_t*)lpParam+i);
	}
	wprintf(L"%s",data);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_wsetlocale(LC_ALL, L"Korean") ;
	wchar_t data[MAX_PATH] = L"hihihi";
	DWORD dwTreadID;
	HANDLE hTread = CreateThread(
					NULL, 0, func,(LPVOID)data,
					0, &dwTreadID);
	while(1);
	return 0;
}	 
