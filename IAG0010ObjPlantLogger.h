#pragma once

#pragma once
#include "afxmt.h"
#include "ClientSocket.h"
#include "ReadingKeyboardThread.h"
#include "ReceivingThread.h"
#include "SendingThread.h"

#if defined(UNICODE) || defined(_UNICODE)
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

using namespace std;