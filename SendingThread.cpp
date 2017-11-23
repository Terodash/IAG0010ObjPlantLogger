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

		//Si la commande précedemment envoyée est "Ready", on continue à l'envoyer
		if (!_tcscmp(SentCommand, _T("Ready"))) {
			ptrCommandGot->SetEvent();
		}
		
		//Si la commande précedemment envoyée est "Start", on envoie maintenant "Ready"
		else if (!_tcscmp(SentCommand, _T("Start"))) {
			_tcscpy_s(CommandBuf, _T("Ready"));
			ptrCommandGot->SetEvent();
		}

		//En attente de l'évènement "ptrDataRecvEvent" ou "ptrStopEvent"
		//ptrDataRecvEvent est signalé dés qu'un paquet est reçu de la part de l'emulateur
		if ((lockResult = ptrDataRecvLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}
		
		//En attente de l'évènement "ptrCommandGot" ou "ptrStopEvent"
		//ptrCommandGot est signalé dés qu'une commande valide est entrée au clavier
		if ((lockResult = ptrCommandEventsLock->Lock(INFINITE, false)) == -1) {
			_tcout << "WSAWaitForMultipleEvents() failed for packetReceivedEvents in sendingDataThread" << endl;
			return 1;
		}

		if (lockResult == WAIT_OBJECT_0)
			return 0; // stopEvent raised


		//Si l'utilisateur a rentré l'une de ces commandes, on copie CommandBuf dans SentCommand. C'est SentCommand qui sera envoyé à l'émulateur.
		if (!_tcscmp(CommandBuf, _T("Start")) || !_tcscmp(CommandBuf, _T("Ready")) || !_tcscmp(CommandBuf, _T("coursework"))) {
			_tcscpy_s(SentCommand, CommandBuf);
		}

		//Dans tous les cas, on envoie à l'émulateur le contenu de SentCommand. Peut être à modifier dans le cas de "Stop".
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
		ptrDataRecvEvent->ResetEvent(); //Une fois que le paquet est envoyé, on attend de recevoir une réponse.
		ptrDataSentEvent->SetEvent();//On prévient ReceivingThread qu'on a envoyé un paquet. 
		ptrCommandGot->ResetEvent();//Une fois un paquet envoyé, on considère qu'on attend une nouvelle commande à envoyer.

	}

	return 0;
}
