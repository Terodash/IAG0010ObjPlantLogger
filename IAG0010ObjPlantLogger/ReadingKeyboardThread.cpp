#include "stdafx.h"
#include "ReadingKeyboardThread.h"
#include "common.h"

TCHAR CommandBuf[81];
HANDLE KeyboardEvents[2];
	
ReadingKeyboardThread::ReadingKeyboardThread(CEvent* ptrStopEvent, CEvent* ptrCommandGot): ptrStopEvent(ptrStopEvent), ptrCommandGot(ptrCommandGot)
{
}


ReadingKeyboardThread::~ReadingKeyboardThread(void)
{
}

int ReadingKeyboardThread::Run(void)
{ 
	/*_tcout << "To stop, type the \"exit\" command." << endl;
	_tcin >> command;*/
	DWORD nReadChars;
	DWORD WaitResult;

	while (_tcsicmp(CommandBuf, _T("exit"))) {

		//WaitResult = WaitForMultipleObjects(2, KeyboardEvents, FALSE, INFINITE);
		//if (WaitResult == WAIT_OBJECT_0)
		//	return 0; // Stop command got
		//else if (WaitResult == WAIT_OBJECT_0 + 1) { // command processed

			if (!ReadConsole(GetStdHandle(STD_INPUT_HANDLE), CommandBuf, 80, &nReadChars, NULL)) {
				_tprintf(_T("ReadConsole() failed, error %d\n"), GetLastError());
				return 1;
			}
			CommandBuf[nReadChars - 2] = 0; // to get rid of \r\n
			
			//ptrCommandProcessed->ResetEvent();
			ptrCommandGot->SetEvent();

			//std::wcout << L"CommandBuf: " << CommandBuf << '\n';
		//}
	}
	ptrStopEvent->SetEvent();
	return 1;
}