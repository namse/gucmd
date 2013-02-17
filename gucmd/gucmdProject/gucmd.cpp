#include<stdio.h>
#include<Windows.h>
#include<locale.h>
#include<wchar.h>
#include<stdlib.h>
#include<tchar.h>
#define MAX_STR_LEN		256
#define	CMD_TOKEN_NUM	10
#define MAX_THREADS (1024*10)

const wchar_t* SEPS = L" ,\t\n" ;

wchar_t GCommandString[MAX_STR_LEN] ;
wchar_t GCommandTokenList[CMD_TOKEN_NUM][MAX_STR_LEN] ;
wchar_t GCommandByPipeList[CMD_TOKEN_NUM][MAX_STR_LEN] ;
// sourceDir는 지금 현재 상태에서의 루트 디렉토리야.
// destDir은 커맨드에서의 디렉토리야. 나머지는 innerDir에서 처리해줘야해.
// destDir에서 \가 뺸 뒤에 안붙어있는지를 확인해서 innerDir에 추가해줘야해!
// 실제 카피는 destDir + innerDir로 하는거다앙
DWORD WINAPI CopyFileTread( LPVOID lpParam)
{
	wchar_t (*wcp)[MAX_PATH] = (wchar_t(*)[MAX_PATH])lpParam;
	if( CopyFile(wcp[0],wcp[1],FALSE) == 0) return false;
	return true;
}
DWORD WINAPI CopyFileCommand(LPVOID lpParam) 
{
	wchar_t (*wcp)[MAX_PATH] = (wchar_t(*)[MAX_PATH])lpParam;
	wchar_t sourceDir[MAX_PATH] = L"";
	wchar_t destDir[MAX_PATH] = L"";
	wchar_t innerDir[MAX_PATH] = L"";
	WIN32_FIND_DATA fileData;
	DWORD dwTreadID[MAX_THREADS];
	HANDLE hTread[MAX_THREADS];
	int cntTread = 0;
	wcscpy_s(sourceDir,wcp[0]);
	wcscpy_s(destDir,wcp[1]);
	wcscpy_s(innerDir,wcp[2]);
	
	wchar_t findDir[MAX_PATH] = L"";
	wcscpy_s(findDir,sourceDir);
	if(findDir[wcslen(findDir)-1] == '\\') findDir[wcslen(findDir)-1] = NULL;
	HANDLE hFind = FindFirstFile(findDir,&fileData); 
	if(hFind == INVALID_HANDLE_VALUE) return false;
	// sourceDir가 디렉토리인지 파일인지 확인
	else
	{
		if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(sourceDir[wcslen(sourceDir)-1] != '\\')
			{
				wcscat_s(sourceDir,L"\\"); // c:\programfile같은 경우 뒤에 \ 붙여줄려구.
				wcscat_s(findDir,L"\\");
			}
			wcsncat(findDir,L"\\*",3);
			FindClose(hFind);
			hFind = FindFirstFile(findDir,&fileData); 
			if(hFind == INVALID_HANDLE_VALUE) return false;
		}
	
		do
		{
			if( wcscmp(fileData.cFileName, L".") == 0 || wcscmp(fileData.cFileName, L"..") == 0)
			{
				// wow
			}
			else if(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wchar_t (*newDir)[MAX_PATH] = new (wchar_t[3][260])();  // 0 : sourcedir, 1 : dest, 2 : inner
				wcscat_s(newDir[0],sourceDir);
				wcscat_s(newDir[0],fileData.cFileName);
				wcscat_s(newDir[0],L"\\");

				wcscpy_s(newDir[1],destDir);

				wcscat_s(newDir[2],innerDir);
				wcscat_s(newDir[2],fileData.cFileName);
				wcscat_s(newDir[2],L"\\");
				
				wchar_t cd[MAX_PATH] = L"";
				wcscpy_s(cd,destDir);
				if(destDir[wcslen(destDir)-1] != '\\')
				{
					wcscat_s(cd,L"\\"); // destDir이 파일명이 아니라 폴더명이라는 뜻이니까.
				}
				wcscat_s(cd,newDir[2]);
				
				CreateDirectory(cd,NULL);
				//스레드 만들어서 copyFile함수 재귀시키기.
				hTread[cntTread++] = CreateThread(
					NULL, 0, CopyFileCommand,(LPVOID)(wchar_t(*)[MAX_PATH])newDir,
					0, &dwTreadID[cntTread]);
			}
			else
			{
				//스레드 만들어서 진짜카피시키기.
				wchar_t (*newDir)[MAX_PATH] = new (wchar_t[3][260])();
				wcscat_s(newDir[0],sourceDir);
				if(sourceDir[wcslen(sourceDir)-1] == '\\')
					wcscat_s(newDir[0],fileData.cFileName);
				
				wcscat_s(newDir[1],destDir);
				//destDir가 폴더명인지 확인
				WIN32_FIND_DATA destDirData;
				HANDLE hDestDir = FindFirstFile(destDir,&destDirData);
			//	if(hDestDir == INVALID_HANDLE_VALUE)
				if(destDirData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(destDir[wcslen(destDir)-1] != '\\')
					{
						wcscat_s(newDir[1],L"\\"); // destDir이 파일명이 아니라 폴더명이라는 뜻이니까.
					}
					wcscat_s(newDir[1],innerDir);
					wcscat_s(newDir[1],fileData.cFileName);
				}
				hTread[cntTread++] = CreateThread(
					NULL, 0, CopyFileTread,(LPVOID)(wchar_t(*)[MAX_PATH])newDir,
					0, &dwTreadID[cntTread]);
				if(hTread[cntTread-1] == NULL) wprintf(L"error Tread\n");
			}
		}while(FindNextFile(hFind,&fileData));
		WaitForMultipleObjects(cntTread,hTread,true,INFINITE);
	}
	return true;
}
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

	//입력받은 그대로(abc.exe 1 2) 프로세스 만들어서 실행
	CreateProcess(NULL,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);

	CloseHandle(pi.hThread); //usage count 맞춰주기
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
	//프로그램 실행을 위해서 복사//
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

	//파이프 처리하는곳//
	// | 있는지 확인//
	for(int i=0; i<tokenNum;i++)
	{
		if( wcsncmp(GCommandTokenList[i],L"|",5) == 0)
		{
			// 다시 토큰 자르기
			HANDLE *hReadPipe, *hWritePipe, *hPreReadPipe;
			SECURITY_ATTRIBUTES pipeSA = {sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
			nextToken = NULL;
			tokenNum = 0;
			token = wcstok_s(copyCommand,L"|",&nextToken);
			while( NULL != token )
			{
				if(tokenNum >0) token = token + sizeof(' '); // 공백 지우기
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
		
		//찾으려는 디렉토리 다시 구하기. 구지 그러는 이유는 program files같이 토큰에 의해서 잘라지는 경우 대비하여서...//
		wchar_t directory[MAX_STR_LEN] = L"";
		for(int i =4; i<wcslen(copyCommand); i++)
			directory[i-4] = copyCommand[i];
		if(wcscmp(directory,L"") == 0) wcscpy_s(directory,L".");
		wprintf(L"%s",directory);
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
	}
	*/
	else if(!_tcscmp(GCommandTokenList[0],_T("XCOPY")))
	{
		//파일 및 폴더명으로 스페이스바가 들어갈 수 있으니까 토큰을 다시 잘라내자.
		//그러므로 약속해야한다. Xcopy의 명령어는 "xcopy Adir > Bdir"이다!
		wchar_t sourceDir[MAX_PATH] = L"";
		wchar_t destDir[MAX_PATH] = L"";
		int i;
		for(i=6;i<wcslen(copyCommand);i++)
		{
			if(copyCommand[i+1] == '>')
				break;
			sourceDir[i-6] = copyCommand[i];
		}
		for(i+=3;i<wcslen(copyCommand);i++)
		{
			destDir[wcslen(destDir)] = copyCommand[i];
		}
		
		wchar_t newDir[3][MAX_PATH] = {L"",}; // 0 : sourcedir, 1 : dest, 2 : inner
		wcscpy_s(newDir[0],sourceDir);
		wcscpy_s(newDir[1],destDir);
		//스레드 만들어서 copyFile함수 재귀시키기.
		DWORD dwTreadID;
		HANDLE hTread = CreateThread(
			NULL, 0, CopyFileCommand,(LPVOID)(wchar_t(*)[MAX_PATH])newDir,
					0, &dwTreadID);
		WaitForSingleObject(hTread,INFINITE);
	}
	else CommandExecute(copyCommand,NULL,NULL);
	/*
	else // example : abc.exe
	{
		STARTUPINFO si={0,};
		PROCESS_INFORMATION pi;
		DWORD returnValue;
		
		si.cb = sizeof(si);
		//입력받은 그대로(abc.exe 1 2) 프로세스 만들어서 실행
		CreateProcess(NULL,copyCommand,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
		CloseHandle(pi.hThread); //usage count 맞춰주기
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
	//파일 지우기를 위한 임시파일 생성//
	HANDLE hfile = CreateFile (L"a.txt",GENERIC_WRITE,FILE_SHARE_WRITE,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
	CloseHandle(hfile);

	while ( CommandProcessing() ) ;

	wprintf(L"명령어 처리를 종료합니다\n") ;
	return 0;
}	 
