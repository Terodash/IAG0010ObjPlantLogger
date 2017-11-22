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

		//Si la commande pr�cedemment envoy�e est "Ready", on continue � l'envoyer
		if (!_tcscmp(SentCommand, _T("Ready"))) {
			ptrCommandGot->SetEvent();
		}
		
		//Si la commande pr�cedemment envoy�e est "Start", on envoie maintenant "Ready"
		else if (!_tcscmp(SentCommand, _T("Start"))) {
			_tcscpy_s(CommandBuf, _T("Ready"));
			ptrCommandGot->SetEvent();
		}

		//En attente de l'�v�nement "ptrDataRecvEvent" ou "ptrStopEvent"
		//ptrDataRecvEvent est signal� d�s qu'un paquet est re�u de la part de l'emulateur
		if ((lockResult = ptrDataRecvLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}
		
		//En attente de l'�v�nement "ptrCommandGot" ou "ptrStopEvent"
		//ptrCommandGot est signal� d�s qu'une commande valide est entr�e au clavier
		if ((lockResult = ptrCommandEventsLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}

		if (lockResult == WAIT_OBJECT_0)
			return 0; // stopEvent raised


		//Si l'utilisateur a rentr� l'une de ces commandes, on copie CommandBuf dans SentCommand. C'est SentCommand qui sera envoy� � l'�mulateur.
		if (!_tcscmp(CommandBuf, _T("Start")) || !_tcscmp(CommandBuf, _T("Ready")) || !_tcscmp(CommandBuf, _T("coursework"))) {
			_tcscpy_s(SentCommand, CommandBuf);
		}

		//Dans tous les cas, on envoie � l'�mulateur le contenu de SentCommand. Peut �tre � modifier dans le cas de "Stop".
		ptrClientSocket->setSendMessage(SentCommand, (wcslen(SentCommand) + 1) * sizeof(_TCHAR));

		//Envoi des paquets
		if (ptrClientSocket->send() == 1) { 
			_tcout << "ptrClientSocket->send() failed in SendingThread" << endl;
			return 1;
		}
		
		else if (!_tcscmp(SentCommand, _T("Exit")))	{
			_tcout << "Closing program..." << endl;
			return 0;
		}

		// Events management and synchronization
		ptrDataRecvEvent->ResetEvent(); //Une fois que le paquet est envoy�, on attend de recevoir une r�ponse.
		ptrDataSentEvent->SetEvent();//On pr�vient ReceivingThread qu'on a envoy� un paquet. 
		ptrCommandGot->ResetEvent();//Une fois un paquet envoy�, on consid�re qu'on attend une nouvelle commande � envoyer.

	}

	return 0;
}
