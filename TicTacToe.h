/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  GAME                             1       /* callback function: PanelCallback */
#define  GAME_CANVAS                      2       /* control type: canvas, callback function: OnCanvasEvent */

#define  INSTRUCT                         2       /* callback function: PanelCallback */
#define  INSTRUCT_ANIMATION               2       /* control type: pictRing, callback function: (none) */
#define  INSTRUCT_EXPLAIN3                3       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_EXPLAIN2_2              4       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_EXPLAIN2                5       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_EXPLAIN1                6       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_TITLE3                  7       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_TITLE4                  8       /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_BACK_TO_MENU            9       /* control type: command, callback function: BackCallback */
#define  INSTRUCT_TITLE2                  10      /* control type: textMsg, callback function: (none) */
#define  INSTRUCT_TITLE1                  11      /* control type: textMsg, callback function: (none) */

#define  LOAD_GAME                        3       /* callback function: PanelCallback */
#define  LOAD_GAME_LOAD_FILE              2       /* control type: command, callback function: LoadGameCallback */
#define  LOAD_GAME_BACK_TO_MENU           3       /* control type: command, callback function: BackCallback */
#define  LOAD_GAME_SAVED_GAMES            4       /* control type: listBox, callback function: (none) */

#define  MAIN                             4       /* callback function: PanelCallback */
#define  MAIN_INSTRUCTIONS                2       /* control type: command, callback function: ButtonCallback */
#define  MAIN_QUIT                        3       /* control type: command, callback function: ButtonCallback */
#define  MAIN_ABOUT                       4       /* control type: command, callback function: ButtonCallback */
#define  MAIN_LOAD_GAME                   5       /* control type: command, callback function: ButtonCallback */
#define  MAIN_NEW_GAME                    6       /* control type: command, callback function: ButtonCallback */
#define  MAIN_PICTURE                     7       /* control type: picture, callback function: (none) */
#define  MAIN_TIMER                       8       /* control type: timer, callback function: CheckConnStatusCallback */

#define  POPUP                            5
#define  POPUP_TXT_STATUS                 2       /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENU                             1
#define  MENU_SAVE                        2       /* callback function: MenuBarCallback */
#define  MENU_BACK                        3       /* callback function: MenuBarCallback */


     /* Callback Prototypes: */

int  CVICALLBACK BackCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ButtonCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CheckConnStatusCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK LoadGameCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK MenuBarCallback(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK OnCanvasEvent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif