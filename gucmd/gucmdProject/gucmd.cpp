#include<stdio.h>
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<stdlib.h>
#include<tchar.h>
#define MAX_STR_LEN		256
#define	CMD_TOKEN_NUM	10

const wchar_t* SEPS = L" ,\t\n" ;

wchar_t GCommandString[MAX_STR_LEN] ;
wchar_t GCommandTokenList[CMD_TOKEN_NUM][MAX_STR_LEN] ;

bool CommandProcessing()
{
	wprintf(L"MyShell>> ") ;
	
	_getws_s(GCommandString) ;
	//���α׷� ������ ���ؼ� ����//
//	wchar_t copyCommmand[MAX_STR_LEN];
	if ( !_wcsicmp(GCommandString, L"exit") )
		return false ;

	wchar_t* nextToken = NULL ;
	int tokenNum = 0 ;

	wchar_t* token = wcstok_s(GCommandString, SEPS, &nextToken) ;

	while ( NULL != token )
	{
		wcscpy_s(GCommandTokenList[tokenNum++], token) ;
		token = wcstok_s(NULL, SEPS, &nextToken) ;
	}

	wprintf(L"\n") ;

	//Process by first string - GCommandTokenList[0]//
	if( !_tcscmp(GCommandTokenList[0],_T("blahbalh")) )
	{
		return TRUE;
	}
	else if ( !_tcscmp(GCommandTokenList[0],_T("blahabhabhah")) )
	{

	}
	
	else // example : abc.exe
	{
		STARTUPINFO si={0,};
		PROCESS_INFORMATION pi;
		DWORD returnValue;
		
		si.cb = sizeof(si);
		//�Է¹��� �״��(abc.exe 1 2) ���μ��� ���� ����
		CreateProcess(NULL,GCommandString,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
		CloseHandle(pi.hThread); //usage count �����ֱ�
		GetExitCodeProcess(pi.hProcess, &returnValue);

		wprintf(L"\nReturn Value : %d\n",returnValue);
		if(returnValue == -1) wprintf(L"EXECUTE ERROR\n");
	}

	
	wprintf(L"\n") ;
	return true ;
}
int _tmain(int argc, _TCHAR* argv[])
{
	_wsetlocale(LC_ALL, L"Korean") ;

	while ( CommandProcessing() ) ;

	wprintf(L"��ɾ� ó���� �����մϴ�\n") ;
	return 0;
}	 
