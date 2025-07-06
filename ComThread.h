#ifndef __COMTHREAD_H
#define __COMTHREAD_H

#ifdef _WIN32
  #ifdef LOGIC_EXPORTS      /* defined when building LogicLib.dll */
    #define LOGIC_API  __declspec(dllexport)
  #else
    #define LOGIC_API  __declspec(dllimport)
  #endif
#else
  #define LOGIC_API
#endif

#include "TicTacToe.h"
		
#include <userint.h>
#include <rs232.h>
#include <utility.h>
#include <stdio.h>
#include <time.h>     /* clock() */

#define COM_TIMEOUT         1.0
#define TIMEOUT_TICKS   	50
#define BYTE_HB          	0x55      /* heartbeat byte */
#define HB_PERIOD_MS     	1000      /* send every 1 s */
#define HB_TIMEOUT_MS    	4000      /* 4 s silence => FAIL */

#define BYTE_HELLO  'H'
#define BYTE_OK     'K'

/* Tic-Tac-Toe symbols */
#define SYMBOL_X   1
#define SYMBOL_O   2


/* public connection states */
typedef enum {CTS_IDLE = 0, CTS_CONNECTING, CTS_CONNECTED, CTS_FAILED} ConnState;
extern int  gPort;

LOGIC_API int  EstablishConnection (void);        				/* trigger ComWorker thread  */
LOGIC_API ConnState GetConnectionState (void);        			/* poll helper  */
LOGIC_API int  GetSymbol               (void);        			/* for choosing the symbol of the current user */
LOGIC_API void ComThread_SendMove (int row, int col);
LOGIC_API int ComThread_SendByte(char c);
LOGIC_API void ComThread_EndGame (void);          				/* call from Logic on win / loss */
LOGIC_API int ComThread_ForceExit(void);                         /* Force exit for quitting - Returns return value of ComWorker. */

/* Logic forward-declarations */
extern void Logic_RemoteMove (void *data);
extern void Logic_ApplySavedMove (void *data);
extern void Logic_LaunchGame (int mySymbol);
extern void Logic_EndGame (void);













#endif //__COMTHREAD_H