#include "stdafx.h"
#include "SendingThread.h"
#include "common.h"

SendingThread::SendingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrCommandGot, BOOL* ptrDownloadingCompleted) :
	ptrClientSocket(ptrClientSocket), ptrDataRecvEvent(ptrDataRecvEvent), ptrDataSentEvent(ptrDataSentEvent), ptrStopEvent(ptrStopEvent), ptrCommandGot(ptrCommandGot), ptrDownloadingCompleted(ptrDownloadingCompleted)
{
	ptrCommandEvents[0] = ptrStopEvent;
	ptrCommandEvents[1] = ptrCommandGot;
	ptrCommandEventsLock = new CMultiLock(ptrCommandEvents, 2);

	ptrDataRecvEvents[0] = ptrStopEvent;
	ptrDataRecvEvents[1] = ptrDataRecvEvent;
	ptrDataRecvLock = new CMultiLock(ptrDataRecvEvents, 2);


	firstSending = true;
}

SendingThread::~SendingThread(void)
{
	delete ptrDataRecvLock;
}

int SendingThread::Run(void)
{
	DWORD lockResult;
	TCHAR SentCommand[81];
	ptrCommandGot->SetEvent();

	_tcout << "Sending password... \n" << endl;
	ptrDataSentEvent->SetEvent();
	wcscpy_s(CommandBuf, _T("coursework"));

	while (true) {

		if (!_tcscmp(SentCommand, _T("Ready"))) {
			ptrCommandGot->SetEvent();
		}

		if ((lockResult = ptrDataRecvLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}

		if ((lockResult = ptrCommandEventsLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}

		if (lockResult == WAIT_OBJECT_0)
			return 0; // stopEvent raised


		if (!_tcscmp(CommandBuf, _T("Start")) || !_tcscmp(CommandBuf, _T("Ready")) || !_tcscmp(CommandBuf, _T("coursework"))) {
			_tcscpy_s(SentCommand, CommandBuf);
		}

		ptrClientSocket->setSendMessage(SentCommand, (wcslen(SentCommand) + 1) * sizeof(_TCHAR));

		if (ptrClientSocket->send() == 1) { //sends a packet
			_tcout << "ptrClientSocket->send() failed in SendingThread" << endl;
			return 1;
		}
		
		else if (!_tcscmp(SentCommand, _T("Exit")))	{
			_tcout << "Closing program..." << endl;
			return 0;
		}

		// Events management and synchronization
		ptrDataRecvEvent->ResetEvent();
		ptrDataSentEvent->SetEvent();
		ptrCommandGot->ResetEvent();

		//if (firstSending){
		//	//ptrClientSocket->setSendMessage(_T("Start"), 6 * sizeof(_TCHAR));
		//	wcscpy_s(SentCommand, _T("Start"));
		//	firstSending = false;
		//}
		//else
		//	//ptrClientSocket->setSendMessage(_T("Ready"), 6 * sizeof(_TCHAR));
		//	wcscpy_s(SentCommand, _T("Ready"));



		//wcscpy_s(SentCommand, _T("Start"));
		
		////////////////////////////////////////////
		//if (!_tcsicmp(SentCommand, _T("Start"))) {
		//	_tcout << "Starting to send..." << endl;
		//	ptrClientSocket->setSendMessage(_T("Start"), 6 * sizeof(_TCHAR));
		//	//ptrCommandProcessed->SetEvent();
		//	ptrDataSentEvent->SetEvent();
		//	wcscpy_s(SentCommand, _T("Ready"));
		//}
		//else if (!_tcsicmp(SentCommand, _T("Ready"))) {
		//	_tcout << "Asking for next packet..." << endl;
		//	ptrClientSocket->setSendMessage(_T("Ready"), 6 * sizeof(_TCHAR));
		//	//ptrCommandProcessed->SetEvent();
		//	ptrDataSentEvent->SetEvent();
		//}

		//else _tcout << "Command not recognized." << endl;
		//
		//std::wcout << L"SentCommand from send : " << SentCommand << '\n';
		/////////////////////////////////////////////

		// Executed only the first time.
		/*if (firstSending) {
			_tcout << "Sending password... \n" << endl;
			ptrClientSocket->setSendMessage(_T("coursework"), 11 * sizeof(_TCHAR));
			ptrDataSentEvent->SetEvent();
			wcscpy_s(SentCommand, _T("Start"));

			firstSending = false;
		}*/
	}

	return 0;
}
