#ifndef __FILELOG_H
#define __FILELOG_H

#ifdef _WIN32
  #ifdef LOGIC_EXPORTS      /* defined when building LogicLib.dll */
    #define LOGIC_API  __declspec(dllexport)
  #else
    #define LOGIC_API  __declspec(dllimport)
  #endif
#else
  #define LOGIC_API
#endif

#include <windows.h> 
#include <ansi_c.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>
     

#define SAVE_DIR 				".\\Saved Games"

#define MAX_MOVES   			9     /* 3×3 grid */
#define LOAD_GAME_SAVED_GAMES   4

typedef struct { int row, col, sym; } Move;

/* recording during play */
LOGIC_API void  FileLog_Reset        (int mySymbol, int oppSymbol);  /* call in LaunchGame */
LOGIC_API void  FileLog_RegisterMove (int row, int col, int sym);    /* call on every move */
LOGIC_API int   FileLog_IsDirty      (void);                         /* unsaved? */

/* saving / loading */
LOGIC_API int   FileLog_SaveCurrentGame (void);                      /* returns 0 OK */
LOGIC_API int   FileLog_LoadGame        (const char *filepath, Move moves[], int *numMoves,
          			                     int *outMySym, int *outOppSym);

/* directory helper for LOAD GAME panel */
LOGIC_API int   FileLog_ListSavedGames  (char files[][MAX_PATHNAME_LEN], int maxFiles);
LOGIC_API void  FileLog_PopulateSavedList (void);

/* expose move list for future sync packet */
LOGIC_API int   FileLog_GetMoveSequence (Move buf[MAX_MOVES]);


#endif //__FILELOG_H