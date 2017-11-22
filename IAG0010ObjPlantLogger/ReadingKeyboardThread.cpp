#include "stdafx.h"
#include "ReadingKeyboardThread.h"
#include "common.h"

HANDLE KeyboardEvents[2];
	
ReadingKeyboardThread::ReadingKeyboardThread(CEvent* ptrStopEvent, CEvent* ptrCommandGot, TCHAR* CommandBuffer): ptrStopEvent(ptrStopEvent), ptrCommandGot(ptrCommandGot)
{
	CommandBuf = CommandBuffer;
}


ReadingKeyboardThread::~ReadingKeyboardThread(void)
{
}

int ReadingKeyboardThread::Run(void)
{ 
	DWORD nReadChars;
	DWORD WaitResult;

	while (_tcsicmp(CommandBuf, _T("exit"))) {

		
			if (!ReadConsole(GetStdHandle(STD_INPUT_HANDLE), CommandBuf, 80, &nReadChars, NULL)) {
				_tcout << "ReadConsole() failed, error " <<  GetLastError();
				return 1;
			}
			CommandBuf[nReadChars - 2] = 0; // to get rid of \r\n
			
			ptrCommandGot->SetEvent();

			//std::wcout << L"CommandBuf: " << CommandBuf << '\n';
	}
	ptrStopEvent->SetEvent();
	return 1;
}