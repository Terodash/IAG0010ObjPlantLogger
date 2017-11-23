#include "stdafx.h"
#include "ReadingKeyboardThread.h"
#include "common.h"

TCHAR CommandBuf[81];
HANDLE KeyboardEvents[2];
	
ReadingKeyboardThread::ReadingKeyboardThread(CEvent* ptrStopEvent, CEvent* ptrCommandGot, CEvent * ptrCommandProcessed): ptrStopEvent(ptrStopEvent), ptrCommandGot(ptrCommandGot), ptrCommandProcessed(ptrCommandProcessed)
{
}


ReadingKeyboardThread::~ReadingKeyboardThread(void)
{
}

int ReadingKeyboardThread::Run(void)
{ 
	DWORD nReadChars;
	DWORD lockResult;
	//ptrCommandGot->SetEvent();

	while (_tcsicmp(CommandBuf, _T("exit"))) {

		if ((lockResult = ptrCommandGot->Lock(INFINITE)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for ptrCommandProcessed in ReadingKeyboardThread" << endl;
			return 1;
		}

		if (!ReadConsole(GetStdHandle(STD_INPUT_HANDLE), CommandBuf, 80, &nReadChars, NULL)) {
			_tprintf(_T("ReadConsole() failed, error %d\n"), GetLastError());
			return 1;
		}
		CommandBuf[nReadChars - 2] = 0; // to get rid of \r\n
			
		ptrCommandProcessed->ResetEvent();
		ptrCommandGot->SetEvent();

		//std::wcout << L"CommandBuf: " << CommandBuf << '\n';
	}
	ptrStopEvent->SetEvent();
	return 1;
}