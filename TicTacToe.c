#include <userint.h>
#include "TicTacToe.h"
#include "Config.h"

int instruct;

__declspec(dllimport) int game;          /* panel handle – used by Logic */
__declspec(dllimport) int mainHandle;    /* panel handle – used by Logic */
__declspec(dllimport) int loadGame;      /* panel handle – used by ComThread */
__declspec(dllimport) int gPort;         /* opened COM port number       */

static int currentHandle;

char buffer[MAX_PATHNAME_LEN];

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	
	if ((instruct = LoadPanel (0, "TicTacToe.uir", INSTRUCT)) < 0)
		return -1;
	AnimateCtrl_ConvertFromPictRing (instruct, INSTRUCT_ANIMATION);
	
	if ((game = LoadPanel (0, "TicTacToe.uir", GAME)) < 0)
		return -1;
	
	if ((mainHandle = LoadPanel (0, "TicTacToe.uir", MAIN)) < 0)
		return -1;
	
	if ((loadGame = LoadPanel (0, "TicTacToe.uir", LOAD_GAME)) < 0)
		return -1;
	
	SetBreakOnLibraryErrors(0);       // Don't break on errors
    SetStdioWindowVisibility(0);  // Hide the CVI Output Window
	
	DisplayPanel (mainHandle);
	StartMenuMusic();
	
	RunUserInterface ();
	
	DiscardPanel (instruct);
	DiscardPanel (game);
	DiscardPanel (mainHandle);
	DiscardPanel (loadGame);
	return 0;
}


int CVICALLBACK ButtonCallback (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			PlayClick(); // Click sound
			
			switch (control)
			{
				case MAIN_NEW_GAME:
					currentHandle = game;
					EstablishConnection();   /* start worker              */
                    SetCtrlAttribute (mainHandle, MAIN_TIMER, ATTR_ENABLED, 1); //  polling
					break;
					
				case MAIN_LOAD_GAME:
					currentHandle = loadGame;
					EstablishConnection();   /* start worker              */
					SetCtrlAttribute (mainHandle, MAIN_TIMER, ATTR_ENABLED, 1); //  polling
					break;
					
					
				case MAIN_INSTRUCTIONS:
					HidePanel(mainHandle);
					DisplayPanel(instruct);
					break;
					
				
				case MAIN_ABOUT:
				    DisplayAbout();
					break;	
					
					
				case MAIN_QUIT:
					ComThread_ForceExit();
					StopMenuMusic();
					QuitUserInterface(0);
					break;
				
				default:
					break;
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK CheckConnStatusCallback (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	if (event != EVENT_TIMER_TICK)
		return 0;
	
    /* poll until connected or failed */
	if (GetConnectionState() == CTS_CONNECTED)
	{
		SetCtrlAttribute (mainHandle, MAIN_TIMER, ATTR_ENABLED, 0); // Stop polling
		
		if(currentHandle == game)
		{	
			Logic_LaunchGame(GetSymbol());
			StopMenuMusic();
		}
		
		if (currentHandle == loadGame)
		{
			HidePanel(mainHandle);
			FileLog_PopulateSavedList();  // panel init – populate listbox
			DisplayPanel(loadGame);
		}
	}

	if(GetConnectionState() == CTS_FAILED)
	{
		SetCtrlAttribute (mainHandle, MAIN_TIMER, ATTR_ENABLED, 0); // Stop polling
		MessagePopup ("Serial", "Could not establish connection.");
	}
		
	return 0;
}

int CVICALLBACK OnCanvasEvent (int panel, int control, int event,
							   void *callbackData, int y, int x)
{
	 if (event == EVENT_LEFT_CLICK)
        Logic_CanvasClick (x, y);
    return 0;
}


int CVICALLBACK LoadGameCallback (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			switch (control)
			{
				case LOAD_GAME_LOAD_FILE:
										
					PlayClick(); // Click sound
					int index;
					GetCtrlVal (panel, LOAD_GAME_SAVED_GAMES, &index); /* With a HOT list box, GetCtrlVal gives the *item value* (here: i) */
					if (index < 0) 
					{
						MessagePopup ("Load", "Please choose a file first.");
						break;
					}
					
					char fname[MAX_PATHNAME_LEN];
					GetLabelFromIndex (panel,
                   	LOAD_GAME_SAVED_GAMES,
                   	index,              /* same value we stored      */
                   	fname);             /* returns the label string  */ 
					
					/* Build full path and load */
					char path[MAX_PATHNAME_LEN];
			        sprintf (path, "%s\\%s", SAVE_DIR, fname);

			        Move seq[MAX_MOVES]; 
					int n, myS, oppS;
					
			        if (FileLog_LoadGame (path, seq, &n, &myS, &oppS) == 0)
			        {
						HidePanel (loadGame);
			            Logic_LaunchGame (myS);  /* show board, then apply moves one by one */
						
			            for (int i=0;i<n;++i)
			                Logic_ApplyMove (seq[i].row, seq[i].col, seq[i].sym);
						
						ComThread_SendByte ('B');                /* header                 */
						ComThread_SendByte ((char)oppS);         /* peer symbol            */
						ComThread_SendByte ((char)n);             /* how many moves        */
						for (int i = 0; i < n; ++i) {
						    ComThread_SendByte ((char)seq[i].row);
                    		ComThread_SendByte ((char)seq[i].col);
                    		ComThread_SendByte ((char)seq[i].sym);
						}
						ComThread_SendByte ('E');                /* tail                  */
						
			        }
					
					break;
					
				default:
					break;
			}

			break;
	}
	return 0;
}

int CVICALLBACK BackCallback (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			PlayClick(); // Click sound
			
			HidePanel(panel);
			DisplayPanel(mainHandle);
			break;
	}
	return 0;
}


void CVICALLBACK MenuBarCallback (int menuBar, int menuItem, void *callbackData,
								  int panel)
{
	switch (menuItem)
	{
		case MENU_SAVE:
			if (FileLog_SaveCurrentGame() == 0)
        		MessagePopup("Save","Game saved.");
			break;
			
		case MENU_BACK:
			Logic_EndGame();
			break;
			
		default:
			break;
	}
}


int CVICALLBACK PanelCallback (int panel, int event, void *callbackData,
							   int eventData1, int eventData2)
{
	if (event == EVENT_CLOSE)
	{
		ComThread_ForceExit();
		StopMenuMusic();
		QuitUserInterface (0);
	}
	return 0;
}



