#include <stdio.h>
#include <string.h>
#include <cdtv/bookmark.h>
#include <cdtv/cdtvprefs.h>
#include <cdtv/screensaver.h>
#include <exec/types.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <exec/execbase.h>

#define OS_20 37

static UBYTE VersTag[] = "\0$VER: cdtvsaver 0.6 (7.1.2023) (c) Jan.2023 by km-l";

extern struct ExecBase *SysBase;
extern struct IOStdReq *CreateStdIO();
extern struct MsgPort *CreatePort();

struct IOStdReq *IOReq = NULL;
struct MsgPort  *IOPort = NULL;
struct CDTVPrefs cdtvPrefs;
struct Library *PlayerPrefsBase;

VOID Quit(UBYTE *s);
LONG DoIOR(struct IOStdReq *req, int cmd, LONG off, LONG len, APTR data);
VOID CheckOS(UWORD ver); 

int main()
{
	CheckOS(OS_20);
	
	if(!(PlayerPrefsBase=OpenLibrary("playerprefs.library", 0)))
		Quit("Playerprefs library will not open.");

	int i;
    LONG *cdtvprefs;
	cdtvprefs = &cdtvPrefs;
	
	LONG params[5];
    struct RDArgs *rd;
	UBYTE template[40];
	
	strcpy(template, "HELP/S,TIME/K/N,INFO/S,INSTALL/S,KILL/S");
	
	for(i=0; i<5; i++)
		params[i] = 0;
	
	rd = ReadArgs(template, params, NULL);

	IOPort = CreatePort(0,0);
    IOReq = CreateStdIO(IOPort);

    if(OpenDevice("bookmark.device", BID_CDTVPREFS, IOReq, 0))
        Quit("Bookmark device will not open.");
        
    if(DoIOR(IOReq, CMD_READ, 0, sizeof(cdtvPrefs), &cdtvPrefs))
		FillCDTVPrefs(cdtvprefs);
		
    if(rd)
	{
		if(params[0])
			printf("Command line tool for managing the CDTV screensaver.\n\nTIME - sets the screensaver activation time in minutes.\nINFO - shows the current screensaver activation time.\nHELP - shows these tips.\nINSTALL - install the screensaver.\nKILL - disable the screensaver.\n\n");

		if(params[1])
		{	
			LONG *savertime;
			savertime = (LONG*)params[1];
			cdtvPrefs.SaverTime = *savertime;
			SaveCDTVPrefs(cdtvprefs);
			ConfigureCDTV(cdtvprefs);
			printf("The screensaver activation time is set to %d min.\n", cdtvPrefs.SaverTime);
		}

		if(params[2])
		{
			if(FindPort("CDTV Screen Saver"))
				printf("The screensaver is active.\n");
			else
				printf("The screensaver is inactive.\n");
			printf("The current screensaver activation time is set to %d min.\n", cdtvPrefs.SaverTime);
		}

		if(params[3])
		{
			if(FindPort("CDTV Screen Saver"))
				printf("The screensaver is already running.\n");
			else
			{
				InstallScreenSaver();
				printf("The screensaver has been activated.\n");
			}
		}

		if(params[4])
		{
			ScreenSaverCommand(SCRSAV_DIE);
			printf("The screensaver has been disabled.\n");	
		}
		FreeArgs(rd);
	}
	else
		printf("Format: cdtvsaver HELP/S,TIME/K/N,INFO/S,INSTALL/S,KILL/S\n");

	Quit(0);
}

VOID Quit(UBYTE *s)
{
    if(IOReq)
    {
        if(IOReq->io_Device)
            CloseDevice(IOReq);
        DeleteStdIO(IOReq);
    }
    if(IOPort)
        DeletePort(IOPort);
    if(PlayerPrefsBase)
    	CloseLibrary(PlayerPrefsBase);
	if(s)
    {
        printf("ERROR: %s\n", s);
        exit(40);
    }
    else
        exit(0);
}

LONG DoIOR(struct IOStdReq *req, int cmd, LONG off, LONG len, APTR data)
{
    req->io_Command = cmd;
    req->io_Offset = off;
    req->io_Length = len;
    req->io_Data = data;
    return DoIO(req);
}

VOID CheckOS(UWORD ver)
{
    if(SysBase->LibNode.lib_Version < ver)
    {
        printf("This program requires OS2.0 or higher.\n");
        exit(0);
    }
}