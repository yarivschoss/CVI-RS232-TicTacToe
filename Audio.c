#define LOGIC_EXPORTS     /* enables __declspec(dllexport) */
#include "Audio.h"

static char gShortBgmPath[MAX_PATH] = {0};   /* cached */

static const char *GetBgmShortPath (void)
{
    if (gShortBgmPath[0])                 /* already cached */
        return gShortBgmPath;
    
    char full[MAX_PATH];
    GetModuleFileName(NULL, full, sizeof(full));      /* exe path */
    char *slash = strrchr(full, '\\');
    if (slash == NULL) {
        strcpy(gShortBgmPath, ".\\Sounds\\menu.wav");  /* fallback if no slash found */
        return gShortBgmPath;
    }
    
    strcpy(slash + 1, "Sounds\\menu.wav");            /* to file  */
    
    /* convert to 8.3 */
    if (GetShortPathName(full, gShortBgmPath, sizeof(gShortBgmPath)) == 0)
        strcpy(gShortBgmPath, full);      /* fallback – may still fail */
    
    return gShortBgmPath;
}

int StartMenuMusic (void)
{
    /* if already running, do nothing */
    if (bgmProc) return 0;

    STARTUPINFO         si = { sizeof si };
    PROCESS_INFORMATION pi = {0};

    BOOL ok = CreateProcess ("MenuLoop.exe",
                             NULL,         /* command line */
                             NULL, NULL,   /* security     */
                             FALSE,        /* inherit hndl */
                             CREATE_NO_WINDOW,
                             NULL, NULL,   /* env / dir    */
                             &si, &pi);
    if (!ok)
    {
        MessagePopup ("Audio – helper launch failed",
                      "MenuLoop.exe not found or cannot start.");
        return -1;
    }

    bgmProc   = pi.hProcess;
    bgmThread = pi.hThread;
    return 0;        /* success */
}


void StopMenuMusic (void)
{
    if (!bgmProc) return;        /* nothing to do */

    /* ask the helper nicely */
    PostThreadMessage (GetThreadId (bgmThread), WM_CLOSE, 0, 0);

    /* wait up to 1 s – then kill */
    if (WaitForSingleObject (bgmProc, 1000) == WAIT_TIMEOUT)
        TerminateProcess (bgmProc, 0);

    CloseHandle (bgmThread);
    CloseHandle (bgmProc);
    bgmThread = bgmProc = NULL;
}

void PlayClick (void)
{
/* play click */
PlaySound ("Sounds\\Click.wav",
           NULL,
           SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    return;
}