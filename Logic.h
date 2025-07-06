#ifndef __LOGIC_H
#define __LOGIC_H

#ifdef  _WIN32
  #ifdef  LOGIC_EXPORTS           /* defined when building DLL */
    #define LOGIC_API  __declspec(dllexport)
  #else
    #define LOGIC_API  __declspec(dllimport)
  #endif
#else
  #define LOGIC_API               /* nothing on other OSes */
#endif

#define SYMBOL_X   1
#define SYMBOL_O   2

#define GAME_CANVAS     2          /* control ID in .uir */

#define INSET_COEFF    0.1
#define PEN_WIDTH      10

#define IMG_PATH_X ".\\Images\\x.png"
#define IMG_PATH_O ".\\Images\\o.png"
#define IMG_PATH_BOARD ".\\Images\\Board.png"

#include <windows.h>          
#include <analysis.h>         /* for BOOL */
#include <userint.h>
#include <utility.h>
#include <ansi_c.h>

LOGIC_API void  Logic_LaunchGame   (int mySymbol);                  /* called after CTS_CONNECTED */
LOGIC_API void  Logic_CanvasClick  (int x, int y);
LOGIC_API void  Logic_RemoteMove   (void *data);					/* called when byte comes from peer */
LOGIC_API BOOL  Logic_CanSendMove  (void);                          /* is it my turn & legal?           */
LOGIC_API void  Logic_EndGame (void);                               /* centralised clean-up after win, loss, draw, or BACK */
LOGIC_API void  Logic_ApplyMove (int row, int col, int sym); 		/* game-replay helper 			    */
LOGIC_API void  Logic_ApplySavedMove (void *data);   				/* wrapper for PostDeferredCall     */

/* forward-declarations ComThread */
extern void ComThread_SendMove (int row, int col);      
extern void ComThread_EndGame (void); 

/* forward-declarations FileLog */
extern void  FileLog_Reset    (int mySymbol, int oppSymbol);
extern int   FileLog_IsDirty  (void);
extern int   FileLog_SaveCurrentGame (void);
extern void  FileLog_RegisterMove (int row, int col, int sym);

/* forward-declarations Audio */
extern int StartMenuMusic (void); 
extern void StopMenuMusic(void); 




#endif // __LOGIC_H