// abc.cpp : �⺻ ������Ʈ �����Դϴ�.

#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
int _tmain(int argc, TCHAR* argv[])
{
	if(argc != 3) return -1;
	_tprintf(_T("program execute\n"));
	_tprintf(_T("%d\n"),_ttoi(argv[1]) + _ttoi(argv[2]));

	return _ttoi(argv[1]) + _ttoi(argv[2]);
}
