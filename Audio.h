#ifndef __AUDIO_H
#define __AUDIO_H

#ifdef _WIN32
  #ifdef LOGIC_EXPORTS      /* defined when building LogicLib.dll */
    #define LOGIC_API  __declspec(dllexport)
  #else
    #define LOGIC_API  __declspec(dllimport)
  #endif
#else
  #define LOGIC_API
#endif

#include <windows.h> // For Windows API PlaySound function
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <utility.h>
#include <cvirte.h>
#include <userint.h>
#include <ansi_c.h>
#include <tlhelp32.h>      /* for CreateToolhelp32Snapshot – optional */

static HANDLE bgmProc = NULL;   /* helper process handle */
static HANDLE bgmThread = NULL; /* helper primary thread */

#define  AUDIO_BGM_PATH  "Sounds\\menu.wav"

// Audio function declarations
LOGIC_API int StartMenuMusic (void); 
LOGIC_API void StopMenuMusic(void); 
LOGIC_API void PlayClick (void);

#endif //__AUDIO_H