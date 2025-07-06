#define LOGIC_EXPORTS          /* enables __declspec(dllexport) */
#include "Logic.h" 

/* main game panel   */
__declspec(dllexport) int game;   			/* GAME panel       */
__declspec(dllexport) int mainHandle; 		/* MAIN_MENU panel  */
__declspec(dllexport) int loadGame;   		/* LOAD_GAME panel  */
extern __declspec(dllexport) int gPort;

/* 9 board rectangles --------------------------------- */
typedef struct { int l,t,r,b; } CellRect; /* left, top, right, bottom  */
static CellRect cell[3][3];

/* state ---------------------------------------------- */
static int board[3][3];            /* 0 empty, 1 X, 2 O */
static int mySym , oppSym;
static int myTurn = 0;
static int moveCount = 0;        /* 0 … 9 */

static int bmpX = 0 , bmpO = 0, bmpBoard = 0;    /* bitmap handles    */

/* ---------- helpers ------------------------------------------------- */
static int LoadBitmaps (void)
{
    if (!bmpX && GetBitmapFromFile (IMG_PATH_X, &bmpX) < 0)             return 0;
    if (!bmpO && GetBitmapFromFile (IMG_PATH_O, &bmpO) < 0)             return 0;
	if (!bmpBoard && GetBitmapFromFile (IMG_PATH_BOARD, &bmpBoard) < 0) return 0;
    return 1;
}

static void BuildCellRects (void)
{
    int w,h;
    GetCtrlAttribute (game, GAME_CANVAS, ATTR_WIDTH,  &w);
    GetCtrlAttribute (game, GAME_CANVAS, ATTR_HEIGHT, &h);
    int cw = w/3, ch = h/3;

    for (int r=0; r<3; ++r)
        for (int c=0; c<3; ++c) {
            cell[r][c].l = c*cw;
            cell[r][c].t = r*ch;
            cell[r][c].r = (c+1)*cw - 1;
            cell[r][c].b = (r+1)*ch - 1;
        }
}

static int HitCell (int x,int y,int *pr,int *pc)
{
    for (int r=0;r<3;++r)
        for (int c=0;c<3;++c)
            if (x>=cell[r][c].l && x<=cell[r][c].r &&
                y>=cell[r][c].t && y<=cell[r][c].b)
            { *pr=r; *pc=c; return 1; }
    return 0;
}

static void DrawSymbol (int r,int c,int sym)
{
    int bmp = (sym == SYMBOL_X) ? bmpX : bmpO;
   
	/* cell dimensions */
    int cellW = cell[r][c].r - cell[r][c].l + 1;
    int cellH = cell[r][c].b - cell[r][c].t + 1;

    int pad   = (int)(INSET_COEFF * (cellW < cellH ? cellW : cellH));  /* Tweaking symbol size by INSET_COEFF*/

    /* destination rectangle centred, with padding all around */
    Rect dest = MakeRect ( cell[r][c].t + pad,             /* top    */
                           cell[r][c].l + pad,             /* left   */
                           cellH - 2 * pad,                /* height */
                           cellW - 2 * pad);               /* width  */
	
	int status = CanvasDrawBitmap (game, GAME_CANVAS, bmp, VAL_ENTIRE_OBJECT, dest);

    if (status < 0)
		printf("error code %d occurred while drawing on canvas" ,status);
}

static int CheckWin (int *wr,int *wc,int *diag) /* win test (returns winner or 0) ---------------------- */
{
    #define EQ(a,b,c) (a && a==b && a==c)

    /* rows */
    for (int i = 0; i < 3; ++i)
        if (EQ(board[i][0], board[i][1], board[i][2]))
        { *wr = i;  *wc = -1; *diag = 0; return board[i][0]; }

    /* columns */
    for (int i = 0; i < 3; ++i)
        if (EQ(board[0][i], board[1][i], board[2][i]))
        { *wr = -1; *wc = i;  *diag = 0; return board[0][i]; }

    /* diagonals (diag = 1 TL?BR, =2 TR?BL) */
    if (EQ(board[0][0], board[1][1], board[2][2]))
        { *wr = *wc = -1; *diag = 1; return board[0][0]; }

    if (EQ(board[0][2], board[1][1], board[2][0]))
        { *wr = *wc = -1; *diag = 2; return board[0][2]; }

    return 0;
}

static void DrawStrike (int wr,int wc,int diag)
{
    int x0, y0, x1, y1;

    if (diag == 1) {          /* TL ? BR */
        x0 = cell[0][0].l;                y0 = cell[0][0].t;
        x1 = cell[2][2].r;                y1 = cell[2][2].b;
    }
    else if (diag == 2) {     /* TR ? BL */
        x0 = cell[0][2].r;                y0 = cell[0][2].t;
        x1 = cell[2][0].l;                y1 = cell[2][0].b;
    }
    else if (wr >= 0) {       /* full ROW win */
        y0 = y1 = (cell[wr][0].t + cell[wr][0].b) / 2;
        x0 = cell[wr][0].l;               x1 = cell[wr][2].r;
    }
    else {                    /* full COLUMN win (wc >= 0) */
        x0 = x1 = (cell[0][wc].l + cell[0][wc].r) / 2;
        y0 = cell[0][wc].t;               y1 = cell[2][wc].b;
    }
					   
	SetCtrlAttribute (game, GAME_CANVAS, ATTR_PEN_WIDTH, PEN_WIDTH);				   
    CanvasDrawLine (game, GAME_CANVAS, MakePoint(x0,y0), MakePoint(x1,y1));
}

/* ==================================================== */
/* public                                               */
void Logic_ApplySavedMove (void *data)
{
    typedef struct { int r, c, s; } Move;   /* matches malloc in worker */
    Move *mv = (Move *) data;

    Logic_ApplyMove (mv->r, mv->c, mv->s);  /* or ApplyMove if you kept it */
    free (mv);
}

void Logic_ApplyMove (int r,int c,int sym)
{
    if (board[r][c]) return;            /* already taken – ignore */

    board[r][c] = sym;
    DrawSymbol (r, c, sym);
    moveCount++;

    int wr,wc,dg;
    if (CheckWin (&wr,&wc,&dg) == sym) {
        DrawStrike (wr, wc, dg);
        MessagePopup (sym == mySym ? "Win" : "Game Over",
                      sym == mySym ? "You won!" : "You lost.");
		FileLog_Reset (mySym, oppSym);            /* clear buffer / dirty    */
        Logic_EndGame();
        return;
    }
    if (moveCount == 9) {
        MessagePopup ("Draw", "No winner – game tied.");
		FileLog_Reset (mySym, oppSym);
        Logic_EndGame();
        return;
    }
    myTurn = (sym != mySym);            /* switch turn */
}

void Logic_EndGame (void)
{
    /* stop serial + worker thread */
    ComThread_EndGame();

    /* offer to save only if something changed */
    if (FileLog_IsDirty())
    {
        if (ConfirmPopup ("Quit", "Save game before exiting?"))
            FileLog_SaveCurrentGame();
    }

    /* reset local state for the next match */
    moveCount = 0;
    myTurn    = 0;

    /* return to main menu UI */
    HidePanel (game);
    DisplayPanel (mainHandle);
	StartMenuMusic();
}

void Logic_LaunchGame (int mySymbol)
{
	moveCount = 0;
	 
    if(!LoadBitmaps())
        { MessagePopup("Error","Cannot load symbol PNGs"); return; }
		
	HidePanel(mainHandle);
	DisplayPanel(game);
	StopMenuMusic();

    memset(board,0,sizeof(board)); // resets board variable
    BuildCellRects();

    mySym  = mySymbol;
    oppSym = (mySym==SYMBOL_X)? SYMBOL_O : SYMBOL_X;
    myTurn = (mySym==SYMBOL_X);           /* X starts */

	FileLog_Reset (mySym, oppSym); // call after computing the symbols
    CanvasClear (game,GAME_CANVAS,VAL_ENTIRE_OBJECT);
	CanvasDrawBitmap (game,GAME_CANVAS,bmpBoard, VAL_ENTIRE_OBJECT,VAL_ENTIRE_OBJECT); 	/* draw fresh board background */
}

void Logic_CanvasClick (int x,int y)
{
    if (!myTurn) return;

    int r,c;
    if(!HitCell(x,y,&r,&c) || board[r][c]) return;

	/* notify peer + log */
    ComThread_SendMove   (r, c);
    FileLog_RegisterMove (r, c, mySym);
	
   /* apply locally (draw, win/draw check, turn swap) */
    Logic_ApplyMove (r, c, mySym);
}

void Logic_RemoteMove (void *data)
{
    typedef struct { int r,c; } Move;
    Move *mv = (Move *) data;
	FileLog_RegisterMove (mv->r, mv->c, oppSym);  /* record for save/load */
    Logic_ApplyMove (mv->r, mv->c, oppSym);
    free (mv);
}

BOOL Logic_CanSendMove (void){ return myTurn; }