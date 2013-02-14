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
wchar_t GCommandByPipeList[CMD_TOKEN_NUM][MAX_STR_LEN] ;
bool CommandExecute(wchar_t command[], HANDLE* hReadPipe, HANDLE* hWritePipe)
{
	STARTUPINFO si={0,};
	PROCESS_INFORMATION pi;
	DWORD returnValue;
		
	si.cb = sizeof(si);
	if(hReadPipe == NULL) si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	else si.hStdInput = *hReadPipe;
	if(hWritePipe == NULL) si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	else si.hStdOutput = *hWritePipe;
	si.dwFlags |= STARTF_USESTDHANDLES;

	//�Է¹��� �״��(abc.exe 1 2) ���μ��� ���� ����
	CreateProcess(NULL,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

	CloseHandle(pi.hThread); //usage count �����ֱ�
	if( hReadPipe != NULL )CloseHandle(*hReadPipe);
	if( hWritePipe != NULL )CloseHandle(*hWritePipe);

	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &returnValue);

	if(returnValue == -1) wprintf(L"EXECUTE ERROR\n");
	//CloseHandle(pi.hProcess);
	return true;
}
bool CommandProcessing()
{
	wprintf(L"MyShell>> ") ;
	
	_getws_s(GCommandString) ;
	if ( !_wcsicmp(GCommandString, L"exit") )
		return false ;
	//���α׷� ������ ���ؼ� ����//
	wchar_t copyCommand[MAX_STR_LEN] = L"";
	wcscpy_s(copyCommand,GCommandString);

	wchar_t* nextToken = NULL ;
	int tokenNum = 0 ;

	wchar_t* token = wcstok_s(GCommandString, SEPS, &nextToken) ;

	while ( NULL != token )
	{
		wcscpy_s(GCommandTokenList[tokenNum++], token) ;
		token = wcstok_s(NULL, SEPS, &nextToken) ;
	}

	wprintf(L"\n") ;

	// | �ִ��� Ȯ��//
	for(int i=0; i<tokenNum;i++)
	{
		if( wcsncmp(GCommandTokenList[i],L"|",5) == 0)
		{
			// �ٽ� ��ū �ڸ���
			HANDLE *hReadPipe, *hWritePipe, *hPreReadPipe;
			SECURITY_ATTRIBUTES pipeSA = {sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
			nextToken = NULL;
			tokenNum = 0;
			token = wcstok_s(copyCommand,L"|",&nextToken);
			while( NULL != token )
			{
				if(tokenNum >0) token = token + sizeof(' '); // ���� �����
				wcscpy_s(GCommandByPipeList[tokenNum++], token);
				token = wcstok_s(0,L"|",&nextToken);
				
			}
				hPreReadPipe = NULL;
			for(int i=0; i<tokenNum;i++)
			{
				hReadPipe = new HANDLE();
				if(i == tokenNum-1)
				{
					//HANDLE _stdout = GetStdHandle(STD_OUTPUT_HANDLE);
					//hWritePipe = &_stdout;
					hWritePipe = NULL;
				}
				else
				{
					hWritePipe = new HANDLE();
					CreatePipe(hReadPipe,hWritePipe,&pipeSA,0);
				}
				CommandExecute(GCommandByPipeList[i],hPreReadPipe,hWritePipe);
				
				hPreReadPipe = hReadPipe;
			}
			return true;
		}
	}
	//Process by first string - GCommandTokenList[0]//
	if( !_tcscmp(GCommandTokenList[0],_T("dir")) )
	{
		WIN32_FIND_DATA findData;
		HANDLE hDir = INVALID_HANDLE_VALUE;
		
		//ã������ ���丮 �ٽ� ���ϱ�. ���� �׷��� ������ program files���� ��ū�� ���ؼ� �߶����� ��� ����Ͽ���...//
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =4; i<wcslen(copyCommand); i++)
			directory[i-4] = copyCommand[i];
		wcsncat(directory,L"\\*",3);
		hDir = FindFirstFile(directory, &findData);
		if(hDir == INVALID_HANDLE_VALUE){
			wprintf(L"Error : Invalid handle value");
		}
		else
		{
			do{
				wprintf(L"\nFile Name : %s",findData.cFileName);
				if(findData.nFileSizeLow > 0)
					wprintf(L" @@ Data Size : %dbyte",findData.nFileSizeLow);
			}while(FindNextFile(hDir, &findData));
			FindClose(hDir);
		}
	}
	else if ( !_tcscmp(GCommandTokenList[0],_T("md")) )
	{
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =3; i<wcslen(copyCommand); i++)
			directory[i-3] = copyCommand[i];
		if(CreateDirectory(directory,NULL) == 0) wprintf(L"Error create directory : %s",directory);
		
	}
	
	else if ( !_tcscmp(GCommandTokenList[0],_T("rd")) )
	{
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =3; i<wcslen(copyCommand); i++)
			directory[i-3] = copyCommand[i];
		if(RemoveDirectory(directory) == 0) wprintf(L"Error Remove directory : %s",directory);
	}
	else if ( !_tcscmp(GCommandTokenList[0],_T("cd")) )
	{
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =3; i<wcslen(copyCommand); i++)
			directory[i-3] = copyCommand[i];
		if(SetCurrentDirectory(directory) == 0) wprintf(L"Error set current directory : %s",directory);
		
		wchar_t currentDirectory[MAX_PATH] = L"";
		
		GetCurrentDirectory(MAX_PATH,currentDirectory);
		wprintf(L"Current Directory : %s",currentDirectory);
		
	}
	else if ( !_tcscmp(GCommandTokenList[0],_T("del")) )
	{
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =4; i<wcslen(copyCommand); i++)
			directory[i-4] = copyCommand[i];
		if(DeleteFile(directory) == 0) wprintf(L"Error delete file : %s",directory);
		
	}
	else if ( !_tcscmp(GCommandTokenList[0],_T("ren")) )
	{
		wchar_t file1[MAX_STR_LEN] = L"";
		wchar_t file2[MAX_STR_LEN] = L"";
		int l;
		for(int i=wcslen(copyCommand); i>=4; i--)
		{
			if(copyCommand[i] == '>')
			{
				l = i;
			}
		}
		for(int i=4; i<=l-1;i++)
			file1[i-4] = copyCommand[i];
		for(int i=l+2; i<wcslen(copyCommand);i++)
			file2[i-l-2] = copyCommand[i];
		if(MoveFile(file1,file2) == 0) wprintf(L"Error Rename file : %s , %s",file1,file2);
	}/*
	else if ( !_tcscmp(GCommandTokenList[0],_T("type")) )
	{
		wchar_t directory[MAX_STR_LEN] = L"";
		const DWORD maxBuf = 1024;
		wchar_t buf[maxBuf] = L"";
		DWORD readedByte = 0;
		for(int i =5; i<wcslen(copyCommand); i++)
			directory[i-5] = copyCommand[i];
		HANDLE hFile = CreateFile(directory,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			wprintf(L"Error Type-Open file : %s",directory); // ������ �ȿ�����
		}
		else
		{
			do{
				if(ReadFile(hFile,buf,maxBuf,&readedByte,NULL) == false)
				{
					wprintf(L"Error Type-Read file : %s",directory); // ���� ��������
					break;
				}
				printf("%s",buf); // �����ڵ� �ȵɶ� �����ϱ�..
			}while(readedByte==maxBuf);
		}
		CloseHandle(hFile);
	}
	*/
	else CommandExecute(copyCommand,NULL,NULL);
	/*
	else // example : abc.exe
	{
		STARTUPINFO si={0,};
		PROCESS_INFORMATION pi;
		DWORD returnValue;
		
		si.cb = sizeof(si);
		//�Է¹��� �״��(abc.exe 1 2) ���μ��� ���� ����
		CreateProcess(NULL,copyCommand,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
		CloseHandle(pi.hThread); //usage count �����ֱ�
		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &returnValue);

		wprintf(L"\nReturn Value : %d\n",returnValue);
		if(returnValue == -1) wprintf(L"EXECUTE ERROR\n");
		CloseHandle(pi.hProcess);
	}*/

	
	wprintf(L"\n") ;
	return true ;
}
int _tmain(int argc, _TCHAR* argv[])
{
	_wsetlocale(LC_ALL, L"Korean") ;
	//���� ����⸦ ���� �ӽ����� ����//
	HANDLE hfile = CreateFile (L"a.txt",GENERIC_WRITE,FILE_SHARE_WRITE,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
	CloseHandle(hfile);

	while ( CommandProcessing() ) ;

	wprintf(L"��ɾ� ó���� �����մϴ�\n") ;
	return 0;
}	 
