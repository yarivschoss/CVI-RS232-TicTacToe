
/* MenuLoop.c  –  tiny helper that loops “Sounds\Menu.wav” forever */
#include <windows.h>
#include <mmsystem.h>
#pragma  comment(lib,"winmm.lib")

int APIENTRY WinMain (HINSTANCE h, HINSTANCE p, LPSTR c, int n)
{
    PlaySound ("Sounds\\Menu.wav",
               NULL,
               SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);
    /* message-only loop: close on WM_CLOSE */
    MSG msg;
    while (GetMessage (&msg, NULL, 0, 0))
        if (msg.message == WM_CLOSE) break;
    PlaySound (NULL, NULL, 0);          /* stop */
    return 0;
}