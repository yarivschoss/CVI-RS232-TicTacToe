#ifndef __CONFIG_H
#define __CONFIG_H

#include "Audio.h" 
#include "TicTacToe.h"
#include "Logic.h" 
#include "ComThread.h"
#include "FileLog.h"

#include "animate.h"
#include <cvirte.h>		
#include <userint.h>
#include <stdio.h>
#include <string.h>

static int musicThreadID;

static inline void DisplayAbout()
{
	static const char *aboutTxt =
    "Tic-Tac-Toe over RS-232\n"
    "---------------------------------------------------------\n"
    "Course:  Computer Applications LabWindows/CVI Final Project\n"
    "Student:  Yariv Shossberger\n"
    "Instructor:  Pro. Eli Flaxer\n"
    "\n"
    "Built with: NI LabWindows/CVI 2020\n"
    "\n"
    "© 2025  Yariv Shossberger.";

	MessagePopup ("About – Tic-Tac-Toe RS-232", aboutTxt);
}

/* forward-declarations FileLog */
extern void FileLog_PopulateSavedList (void);










#endif // __CONFIG_H