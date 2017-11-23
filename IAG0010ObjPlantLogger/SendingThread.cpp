#include "stdafx.h"
#include "SendingThread.h"
#include "common.h"

SendingThread::SendingThread(ClientSocket* ptrClientSocket, CEvent* ptrDataRecvEvent, CEvent* ptrDataSentEvent, CEvent* ptrStopEvent, CEvent* ptrCommandGot, CEvent* ptrCommandProcessed) :
	ptrClientSocket(ptrClientSocket), ptrDataRecvEvent(ptrDataRecvEvent), ptrDataSentEvent(ptrDataSentEvent), ptrStopEvent(ptrStopEvent), ptrCommandGot(ptrCommandGot), ptrCommandProcessed(ptrCommandProcessed)
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
	int validCommand = 0;
	ptrCommandGot->SetEvent();
	//ptrCommandProcessed->ResetEvent();

	//_tcout << "Sending password... \n" << endl;
	//ptrDataSentEvent->SetEvent();
	wcscpy_s(CommandBuf, _T("coursework"));

	while (true) {

		if (!_tcscmp(CommandBuf, _T("Start")) || !_tcscmp(CommandBuf, _T("Ready"))) {
			wcscpy_s(CommandBuf, _T("ready"));
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
			return 0; // stopEvent signaled

		ptrCommandGot->ResetEvent();

		if (!_tcscmp(CommandBuf, _T("coursework"))) {
			//_tcscpy_s(SentCommand, CommandBuf);+
			ptrCommandProcessed->SetEvent();
			ptrDataRecvEvent->ResetEvent(); //Une fois que le paquet est envoy�, on attend de recevoir une r�ponse.
			ptrDataSentEvent->SetEvent();//On pr�vient ReceivingThread qu'on a envoy� un paquet. 
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
			validCommand = 1;
		}

		else if (!_tcscmp(CommandBuf, _T("start"))) {
			//_tcout << "Command start sent..." << endl;
			wcscpy_s(CommandBuf, _T("Start"));
			ptrCommandProcessed->SetEvent();
			ptrDataRecvEvent->ResetEvent();
			ptrDataSentEvent->SetEvent();
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
			validCommand = 1;
		}

		else if (!_tcscmp(CommandBuf, _T("ready"))) {
			//_tcout << "Command ready sent..." << endl;
			wcscpy_s(CommandBuf, _T("Ready"));
			ptrCommandProcessed->SetEvent();
			ptrDataRecvEvent->ResetEvent();
			ptrDataSentEvent->SetEvent();
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
			validCommand = 1;
		}

		else if (!_tcscmp(CommandBuf, _T("break"))) {
			//_tcout << "Command break sent..." << endl;
			wcscpy_s(CommandBuf, _T("Break"));
			ptrCommandProcessed->SetEvent();
			//ptrDataRecvEvent->ResetEvent();
			ptrDataSentEvent->SetEvent();
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
			validCommand = 1;
			
		}

		else if (!_tcscmp(CommandBuf, _T("stop"))) {
			//_tcout << "Command stop sent..." << endl;
			wcscpy_s(CommandBuf, _T("Stop"));
			ptrCommandProcessed->SetEvent();
			//ptrDataRecvEvent->ResetEvent();
			ptrDataSentEvent->SetEvent();
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
			validCommand = 1;
		}

		else if (!_tcscmp(CommandBuf, _T("connect"))) {
			_tcout << "Trying to reconnect..." << endl;
						
			///// ?????????
			validCommand = 1;
			///// ?????

			//ptrClientSocket->closeConnection();
			ptrClientSocket->openConnection();
			
			wcscpy_s(CommandBuf, _T("coursework"));
			ptrCommandProcessed->SetEvent();
			ptrDataRecvEvent->ResetEvent();
			ptrDataSentEvent->SetEvent();//On pr�vient ReceivingThread qu'on a envoy� un paquet. 
			ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));

			//return SendingThread::Run();
		}

		else if (!_tcscmp(SentCommand, _T("exit"))) {
			_tcout << "Closing program..." << endl;
			return 0;
		}

		else {
			_tcout << "The command is not recognized..." << endl;
			wcscpy_s(CommandBuf, _T(""));
			ptrCommandProcessed->SetEvent();
			//return 0;
		}

		if (validCommand) {
			if (ptrClientSocket->send() == 1) {
				_tcout << "ptrClientSocket->send() failed in SendingThread" << endl;
				return 1;
			}
			validCommand = 0;
		}
		// Events management and synchronization
		
		ptrCommandGot->ResetEvent();//Une fois un paquet envoy�, on consid�re qu'on attend une nouvelle commande � envoyer.


									/*if (!_tcscmp(CommandBuf, _T("start"))) {
									_tcout << "Command start sent..." << endl;
									wcscpy_s(CommandBuf, _T("Start"));
									ptrCommandGot->SetEvent();
									ptrClientSocket->setSendMessage(CommandBuf, (wcslen(CommandBuf) + 1) * sizeof(_TCHAR));
									}
									*/

									/*	//Si la commande pr�cedemment envoy�e est "Ready", on continue � l'envoyer
									if (!_tcscmp(SentCommand, _T("Ready"))) {
									ptrCommandGot->SetEvent();
									}

									//Si la commande pr�cedemment envoy�e est "Start", on envoie maintenant "Ready"
									else if (!_tcscmp(SentCommand, _T("Start"))) {
									_tcout << "Command start sent..." << endl;
									_tcscpy_s(CommandBuf, _T("Ready"));
									ptrCommandGot->SetEvent();
									}

									if (!_tcscmp(SentCommand, _T("Break"))) {
									ptrCommandGot->SetEvent();
									}

									if (!_tcscmp(SentCommand, _T("Stop"))) {
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
									if (!_tcscmp(CommandBuf, _T("Start")) || !_tcscmp(CommandBuf, _T("Break")) || !_tcscmp(CommandBuf, _T("Stop")) || !_tcscmp(CommandBuf, _T("Ready")) || !_tcscmp(CommandBuf, _T("coursework"))) {
									_tcscpy_s(SentCommand, CommandBuf);
									ptrClientSocket->setSendMessage(SentCommand, (wcslen(SentCommand) + 1) * sizeof(_TCHAR));

									}

									//Dans tous les cas, on envoie � l'�mulateur le contenu de SentCommand. Peut �tre � modifier dans le cas de "Stop".
									//		ptrClientSocket->setSendMessage(SentCommand, (wcslen(SentCommand) + 1) * sizeof(_TCHAR));

									//Envoi des paquets
									if (ptrClientSocket->send() == 1) {
									_tcout << "ptrClientSocket->send() failed in SendingThread" << endl;
									return 1;
									}

									else if (!_tcscmp(SentCommand, _T("exit")))	{
									_tcout << "Closing program..." << endl;
									return 0;
									}

									// Events management and synchronization
									ptrDataRecvEvent->ResetEvent(); //Une fois que le paquet est envoy�, on attend de recevoir une r�ponse.
									ptrDataSentEvent->SetEvent();//On pr�vient ReceivingThread qu'on a envoy� un paquet.
									ptrCommandGot->ResetEvent();//Une fois un paquet envoy�, on consid�re qu'on attend une nouvelle commande � envoyer.
									*/
	}

	return 0;
}
