#include "stdafx.h"
#include "ReceivingThread.h"


ReceivingThread::ReceivingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, BOOL* ptrDownloadingCompleted, HANDLE* ptrFile) :
	ptrClientSocket(ptrClientSocket), ptrDataRecvEvent(ptrDataRecvEvent), ptrDataSentEvent(ptrDataSentEvent), ptrStopEvent(ptrStopEvent), ptrDownloadingCompleted(ptrDownloadingCompleted), ptrFile(ptrFile)
{
	ptrDataSentEvents[0] = ptrStopEvent;
	ptrDataSentEvents[1] = ptrDataSentEvent;
	ptrDataSentLock = new CMultiLock(ptrDataSentEvents, 2);
	firstRecv = true;
}

ReceivingThread::~ReceivingThread(void)
{
	delete ptrDataSentLock;
}

int ReceivingThread::Run(void)
{
	DWORD lockResult;
	int recvResult;

	while (!*ptrDownloadingCompleted)
	{

		// We wait that sending message complete
		if ((lockResult = ptrDataSentLock->Lock(INFINITE, false)) == -1) {
			_tcout << "ptrDataSentLock->lock() failed for ptrDataSentEvents" << endl;
			return 1;
		}
		if (lockResult == WAIT_OBJECT_0) // stopEvent raised.
			return 0;
		// Receive packet
		if ((recvResult = ptrClientSocket->recv()) == 1) {
			_tcout << "ptrClientSocket->recv() failed in ReceivingThread" << endl;
			return 1;
		}
		if (recvResult == 2) {
			return 0;
		}

		// Events management
		ptrDataSentEvent->ResetEvent();
		ptrDataRecvEvent->SetEvent();

		// Executed only the first time.
		if (firstRecv) { helloProcessing(); }
		// Main processing.
		else { fileProcessing(); }
		//else { helloProcessing(); }
	}
	return 0;
}

void ReceivingThread::helloProcessing() {
	_TCHAR recvText[20];
	memcpy(recvText, ptrClientSocket->getRecvMessage() + 4, 40);
	_tcout << "Received message : " << recvText << endl;
	firstRecv = false;
}

void ReceivingThread::fileProcessing() {
	// Write in file
	if (!WriteFile(*ptrFile, ptrClientSocket->getRecvMessage() + 4, *(int*)ptrClientSocket->getRecvMessage(), &nWrittenBytes, NULL)) {
		_tcout << "Unable to write into file, error " << GetLastError() << endl;
	}
	_TCHAR recvText[20];
	memcpy(recvText, ptrClientSocket->getRecvMessage() + 4, 40);
	_tcout << "Received message : " << recvText << endl;
}