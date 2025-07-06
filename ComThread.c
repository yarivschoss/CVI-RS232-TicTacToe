#define LOGIC_EXPORTS     /* enables __declspec(dllexport) */
#include "ComThread.h"

static volatile ConnState gState = CTS_IDLE; 	      /* stays private here   */
static int          gThread  = 0;                     /* Cmt thread pool token */
static volatile int gRoleReady = 0;     		  	  /* 0 = not yet, 1 = decided */
static int      	gMySymbol = 0;              	  /* cached result */
static int      	popup;                            /* Local Handle          */
static clock_t      lastTraffic = 0;          		  /* tick of most-recent traffic */

/*--- For syncing with other user when loading a saved game ----- */
static enum { IDLE, SYNC_SYM, SYNC_LEN, SYNC_MOVES } syncState = IDLE;
static int  syncMoves = 0, movesNeeded = 0;

extern int  game, mainHandle, loadGame; 			  /* External Handles	  */
int  gPort = 0;

static const char *kConnectingTxt[4] = {"Connecting", "Connecting.", "Connecting..", "Connecting..."};

/* ---------- helpers ------------------------------------------------- */

/* Try to open COMx, configure to 9600-8-N-1.
   returns 0 on success, negative CVI error otherwise               */
static inline int TryOpenPort (int port)
{
    char portName[16];
    sprintf (portName, "COM%d", port);

    /* 9600 baud, no parity, 8 data bits, 1 stop bit, 512-byte queues */
    return OpenComConfig (port, portName, 9600, 0, 8, 1, 512, 512);
}

static void MarkTraffic (void)         /* call whenever byte RX/TX   */
{
    lastTraffic = clock();               /* update heartbeat           */
}

static void DecideSymbol (void)
{
    /* send our port number */
    ComWrtByte (gPort, 'P');
    ComWrtByte (gPort, (unsigned char) gPort);

    int remote = -1, stage = 0, to = 200;   /* 1-s timeout */
    while (to-- > 0 && remote < 0)
    {
        int ch = ComRdByte (gPort);
        if (ch < 0) { Delay (0.005); continue; }

        if (stage == 0 && ch == 'P') { stage = 1; continue; }
        if (stage == 1)              { remote = ch; }
    }

    if (remote < 0)          /* fallback if peer byte missing */
        gMySymbol = (gPort % 2) ? SYMBOL_X : SYMBOL_O;
    else
        gMySymbol = (gPort < remote) ? SYMBOL_X : SYMBOL_O;

    gRoleReady = 1;          /* tell UI thread we’re done */
}

/* Handle inbound traffic – returns 1 if packet complete */
static int ProcessIncomingByte (unsigned char ch)
{
    static unsigned char pkt[3];
    static int idx = 0;

    if (ch == BYTE_HB) {                /* heartbeat ? ignore */
        MarkTraffic();
        return 0;
    }
	
    /* board-sync state machine ----------------------- */
    switch (syncState)
    {
        case IDLE:
            if (ch == 'B') { syncState = SYNC_SYM; return 0; }
            break;
		
		case SYNC_SYM:
        int mySymbol = ch;                                     /* save byte			   */
		HidePanel(loadGame);								   /* hide load game panel */
        PostDeferredCall ((void*)Logic_LaunchGame,
                          (void*)(intptr_t)mySymbol);          /* draw board 		   */
        syncState   = SYNC_LEN;                                /* next = len 		   */
        return 0;
			
        case SYNC_LEN:
            movesNeeded = ch;           /* 0-9                 */
            syncState   = SYNC_MOVES;
            syncMoves   = 0;
            return 0;

        case SYNC_MOVES:
            pkt[idx++] = ch;            /* reuse existing M parser */
            if (idx < 3) return 0;     /* waiting for row/col     */
            
         	/* full “row col” ready    */
            int row = pkt[0], col = pkt[1], sym = pkt[2];
			
            typedef struct { int r,c,s; } Move;
            Move *mv = malloc(sizeof(Move));
            mv->r = row; mv->c = col; mv->s = sym;
			
            PostDeferredCall ((void*)Logic_ApplySavedMove, mv);
            idx = 0;

            if (++syncMoves == movesNeeded)
                syncState = IDLE;
            return 1;
    }

    /* finite-state matcher for "M row col" */
    if (idx == 0 && ch == 'M') { pkt[idx++] = ch; return 0; }
    if (idx == 1)              { pkt[idx++] = ch; return 0; }
    if (idx == 2)              { pkt[idx++] = ch;           }

    if (idx == 3) {                           /* packet ready */
        int row = pkt[1], col = pkt[2];
        typedef struct { int r,c; } Move;
        Move *mv = malloc (sizeof(Move));
        mv->r = row; mv->c = col;
        PostDeferredCall ((void*)Logic_RemoteMove, mv);
        idx = 0;
        return 1;
    }
    return 0;
}

static void CVICALLBACK ComDisconnectCallback (int port, int eventMask,
                                               void *cbData)
{
	/* confirm peer still asserts carrier / DSR ---------------- */
	int lineBits = GetComLineStatus (port);          /* always safe – no GP fault */
	int peerAlive =  (lineBits & kRS_DSR_ON)   &&   /* DSR asserted      */
                 (lineBits & kRS_RLSD_ON);      /* Carrier-Detect on */
	
	if (!peerAlive)
	{
	    gState = CTS_FAILED;
	    PostDeferredCall ((void*)Logic_EndGame, NULL);
	}
	/* else: benign transition, ignore and continue */
}

static void CVICALLBACK UpdatePopupDots (void *phasePtr)
{
    int phase = (int)(intptr_t) phasePtr;            /* 0-3 */
    SetCtrlVal (popup, POPUP_TXT_STATUS, kConnectingTxt[phase]);
}

static void CenterPopupText (int panel, int ctrl)
{
    int pw, cw;
    GetPanelAttribute (panel, ATTR_WIDTH,  &pw);      /* popup width  */
    GetCtrlAttribute  (panel, ctrl, ATTR_WIDTH, &cw); /* text width   */
	SetCtrlAttribute (panel, ctrl, ATTR_LEFT, ((pw-cw) / 3) - 5);
}

static void CVICALLBACK FinalPopup (void *statePtr)
{
    ConnState st = (ConnState)(intptr_t) statePtr;
    const char *msg = (st == CTS_CONNECTED) ? "Connection Success"
                                            : "Connection Failed";

	CenterPopupText (popup, POPUP_TXT_STATUS);
	SetCtrlVal (popup, POPUP_TXT_STATUS, msg);
    Delay (1.5);                       /* let user read the message */

    DiscardPanel (popup);        /* close pop-up */
}

/* --------------------------------------------------------------------- */

static inline int AutoInitComPort (int *resultPort)
{
    int status = -1;
    int port;

    for (port = 1; port <= 32; ++port)           /* scan COM1…COM32 */
    {
        status = TryOpenPort (port);
        if (status == 0)                   /* opened successfully?          */
        {
            *resultPort = port;            /* tell caller which COM it is   */
            return 0;                      /* and KEEP the port open        */
        }
    }
    return -1;                                   /* no port answered */
}

/* Returns return value of ComWorker. */
int ComThread_ForceExit(void)
{
	if (gPort > 0) {
        FlushOutQ (gPort);
        FlushInQ  (gPort);
        CloseCom  (gPort);
        gPort = -1;
    }
	
	int WorkerReturn = 0; 
	CmtExitThreadPoolThread (WorkerReturn);
	return WorkerReturn;
}

void ComThread_EndGame (void)
{
    /* 1. signal the worker loop to exit on its own */
    if (gState == CTS_CONNECTED)
        gState = CTS_IDLE;
	
	if (gState == CTS_FAILED)
	{
		if ((popup = LoadPanel (0, "TicTacToe.uir", POPUP)) < 0) return;
		DisplayPanel(popup);
		FinalPopup((void*)(intptr_t)gState);
		gState = CTS_IDLE;
	}

    /* 2. wait (non-blocking to UI) until it finishes and release ID */
    if (gThread) {
        CmtWaitForThreadPoolFunctionCompletion ( DEFAULT_THREAD_POOL_HANDLE, gThread, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
        CmtReleaseThreadPoolFunctionID (
            DEFAULT_THREAD_POOL_HANDLE,
            gThread);

        gThread = 0;
    }

    /* 3. close serial port so NEW GAME can reopen it */
    if (gPort > 0) {
		InstallComCallback (gPort, 0, 0, 0, 0, NULL); // the callback for the COM port is uninstalled
        FlushOutQ (gPort);
        FlushInQ  (gPort);
        CloseCom  (gPort);
        gPort = -1;
    }

    /* 4. reset role-ready flag if you use it */
    gRoleReady = 0;                 /* if declared; ignore otherwise */
}

void ComThread_SendMove (int row, int col)
{
    if (gState != CTS_CONNECTED)      /* link must be up */
        return;

    ComWrtByte (gPort, 'M');                          /* packet header */
    ComWrtByte (gPort, (unsigned char) row);          /* 0…2            */
    ComWrtByte (gPort, (unsigned char) col);          /* 0…2            */
}

/* Returns status of reading (0 - no byte/failed, 1 - read 1 byte). */
int ComThread_SendByte(char c)
{
	int status = ComWrtByte(gPort, c);
	MarkTraffic();	
	return status;
}

/* Returns SYMBOL_X or SYMBOL_O, unique per PC. */
int GetSymbol (void)
{
    while (!gRoleReady)
        Delay (0.005);      /* wait until worker decides */
    return gMySymbol;
}

/* expose current state to UI */
ConnState GetConnectionState (void)
{
    return gState;
}

/* Worker running in background:                                             */
/*  – does “H/K” handshake                                                   */
static int CVICALLBACK ComWorker (void *data)
{
	int dotPhase = 0;                      /* 0-3  */
    gState = CTS_CONNECTING;
	
    ComWrtByte (gPort, BYTE_HELLO); /* kick off our own HELLO ? both sides do this once */

    int timeoutTicks = TIMEOUT_TICKS; /* TIMEOUT_TICKS × (5 ms + COM_TIMEOUT) = 15.075 s timeout */
    while (timeoutTicks-- > 0 && gState == CTS_CONNECTING)
    {
        int ch = ComRdByte (gPort);
        if (ch < 0) {
		    PostDeferredCall (UpdatePopupDots, (void*)(intptr_t)dotPhase);
		    dotPhase = (dotPhase + 1) & 3;  /* 0->1->2->3->0                */
	        Delay (0.005);
	        continue;
        }

        if (ch == BYTE_HELLO)
            ComWrtByte (gPort, BYTE_OK);    /* peer's hello ? answer */
        else if (ch == BYTE_OK)
            gState = CTS_CONNECTED;         /* handshake done       */
    }

    if (gState != CTS_CONNECTED)
        gState = CTS_FAILED;

	PostDeferredCall (FinalPopup, (void*)(intptr_t)gState);
	
	DecideSymbol(); // calling this function from here for reading sync of symbol
	
    while (gState == CTS_CONNECTED)
    {
	int ch = ComRdByte (gPort);
    if (ch >= 0)
        ProcessIncomingByte ((unsigned char) ch);
    Delay (0.005);
    } 
    return 0;
}

/* Called (later) from NEW_GAME: port is already open by AutoInitComPort()    */
int EstablishConnection (void)
{
	if (AutoInitComPort(&gPort) < 0)
	{
	    MessagePopup ("Serial Error",
	                  "Could not find an available COM port.\n"
	                  "Check the cable and try again.");
	    return 0;           /* stay in MAIN menu */
	}
		
	InstallComCallback (gPort, LWRS_DSR | LWRS_RLSD | LWRS_ERR, 0, 0, ComDisconnectCallback, NULL);
	
	SetComTime(gPort, COM_TIMEOUT);  // setting com timout
	
	if ((popup = LoadPanel (0, "TicTacToe.uir", POPUP)) < 0)
		return -1;
	
	DisplayPanel(popup);
	SetCtrlVal (popup, POPUP_TXT_STATUS, "Connecting");

    int err = CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,
                                             ComWorker,
                                             NULL,          /* data ptr */
                                             &gThread);     /* ID       */
    if (err < 0) {
        gState = CTS_FAILED;
        return err;
    }
	
    return 0;                           /* thread launched OK */
}

