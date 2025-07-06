#define LOGIC_EXPORTS     /* enables __declspec(dllexport) */
#include "FileLog.h"

static Move moveBuf[MAX_MOVES];
static int  moveCnt = 0;
static int  dirty   = 0;
static int  mySym   = 0, oppSym = 0;

extern int loadGame;

/* LOAD_GAME panel init – populate listbox */
void FileLog_PopulateSavedList (void)
{
    char files[64][MAX_PATHNAME_LEN];
    int n = FileLog_ListSavedGames (files, 64);
	
    DeleteListItem (loadGame, LOAD_GAME_SAVED_GAMES, 0, -1);
	
	SetCtrlAttribute (loadGame, LOAD_GAME_SAVED_GAMES,
	                  ATTR_SELECTION_MODE, VAL_SELECTION_SINGLE);  /* NEW line */
	
    for (int i=0;i<n;++i)
        InsertListItem (loadGame, LOAD_GAME_SAVED_GAMES, -1, files[i], i);
}

/* ------------------------------------------------------- */
void FileLog_Reset (int myS, int oppS)
{
    moveCnt = 0; dirty = 0;
    mySym = myS; oppSym = oppS;
}

/* ------------------------------------------------------- */
void FileLog_RegisterMove (int r,int c,int s)
{
    if (moveCnt < MAX_MOVES) {
        moveBuf[moveCnt++] = (Move){r,c,s};
        dirty = 1;
    }
}

/* ------------------------------------------------------- */
int FileLog_IsDirty (void) { return dirty; }

/* ------------------------------------------------------- */
static void EnsureSaveDir (void)
{
    MakeDir (SAVE_DIR);          /* no-op if already exists */
}

/* yyyy-mm-dd_hh-mm-ss.csv */
static void BuildFilename (char path[MAX_PATHNAME_LEN])
{
    SYSTEMTIME st;
    GetLocalTime (&st);

    sprintf (path, "%s\\%04d-%02d-%02d_%02d-%02d-%02d.csv",
             SAVE_DIR,
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);
}

/* ------------------------------------------------------- */
int FileLog_SaveCurrentGame (void)
{
    if (!dirty) return 0;              /* nothing to save */

    EnsureSaveDir();

    char file[MAX_PATHNAME_LEN];
    BuildFilename (file);

    FILE *fp = fopen (file, "w");
    if (!fp) { MessagePopup("Save","Cannot create file"); return -1; }

    fprintf (fp, "mySym,%d,oppSym,%d\n", mySym, oppSym);
    for (int i=0;i<moveCnt;++i)
        fprintf (fp, "%d,%d,%d\n",
                 moveBuf[i].row, moveBuf[i].col, moveBuf[i].sym);
    fclose (fp);

    dirty = 0;
    return 0;
}

/* ------------------------------------------------------- */
int FileLog_LoadGame (const char *path, Move moves[], int *num, int *oMy, int *oOpp)
{
    FILE *fp = fopen (path, "r");
    if (!fp) return -1;

    char line[64];
    if (!fgets(line,sizeof(line),fp)) { fclose(fp); return -1; }
    sscanf (line, "mySym,%d,oppSym,%d", oMy, oOpp);

    int n=0;
    while (fgets(line,sizeof(line),fp) && n<MAX_MOVES)
        sscanf (line, "%d,%d,%d",
                &moves[n].row,&moves[n].col,&moves[n].sym), ++n;

    fclose(fp);
    *num = n;
    return 0;
}

/* ------------------------------------------------------- */
int FileLog_ListSavedGames (char files[][MAX_PATHNAME_LEN], int maxFiles)
{
    EnsureSaveDir();

    char pattern[MAX_PATHNAME_LEN];
    sprintf (pattern, "%s\\*.csv", SAVE_DIR);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA (pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return 0;

    int count = 0;
    do {
        if (count >= maxFiles) break;
        strcpy (files[count++], fd.cFileName);
    }
    while (FindNextFileA (hFind, &fd));

    FindClose (hFind);
    return count;
}

int FileLog_GetMoveSequence (Move buf[MAX_MOVES])
{
    memcpy (buf, moveBuf, moveCnt * sizeof(Move));
    return moveCnt;
}