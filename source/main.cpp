
#include <ogc/system.h>
#include <unistd.h>
#include <sys/stat.h>

#include "defines.h"
#include "booter/external_booter.hpp"
#include "channel/nand.hpp"
#include "channel/nand_save.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.hpp"
#include "gui/video.hpp"
#include "gui/text.hpp"
#include "homebrew/homebrew.h"
#include "loader/alt_ios_gen.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "menu/menu.hpp"
#include "memory/memory.h"
#include <unistd.h>
#include <fat.h>


bool isWiiVC = false;
bool useMainIOS = true;
bool sdOnly = false;
volatile bool NANDemuView = false;
volatile bool networkInit = false;

int main(int argc, char **argv)
{
	VIDEO_Init();
    GXRModeObj *dbg_rm = VIDEO_GetPreferredMode(NULL);
    void *dbg_fb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(dbg_rm));
    VIDEO_ClearFrameBuffer(dbg_rm, dbg_fb, COLOR_BLACK);
    VIDEO_Configure(dbg_rm);
    VIDEO_SetNextFramebuffer(dbg_fb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    CON_InitEx(dbg_rm, 20, 20, dbg_rm->fbWidth - 40, dbg_rm->xfbHeight - 40);

    /*printf("\n\n=== WIIFLOW DEBUG START ===\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc && i < 3; i++)
        printf("argv[%d] = %s\n", i, argv[i] ? argv[i] : "(null)");
    printf("AHBPROT = %s\n",
        (*(volatile u32*)0xCD800064 == 0xFFFFFFFF) ? "DISABLED" : "ENABLED");

    printf("Testing fatInitDefault()... ");
    bool sd_ok = fatInitDefault();
    printf("%s\n", sd_ok ? "OK" : "FAILED");

    if (sd_ok) {
        printf("Writing test file... ");
        FILE *f = fopen("sd:/wiiflorked_test.txt", "w");
        if (f) {
            fprintf(f, "hello from wiiflow boot\n");
            fclose(f);
            printf("OK\n");
        } else {
            printf("FAILED\n");
        }
    }*/
    //printf("Sleeping 3 sec...\n");
    usleep(200000);

	MEM_init(); //Inits both mem1lo and mem2
	mainIOS = 209;//DOL_MAIN_IOS;// 249
	__exception_setreload(10);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	#ifdef COMMITHASH
		gprintf(" \nWelcome to %s %s %s!\nThis is the debug output.\n", APP_NAME, APP_VERSION, COMMITHASH);
	#else
		gprintf(" \nWelcome to %s %s!\nThis is the debug output.\n", APP_NAME, APP_VERSION);
	#endif

	char gameid[6];
	memset(&gameid, 0, sizeof(gameid));
	bool wait_loop = false;
	char wait_dir[256];
	memset(&wait_dir, 0, sizeof(wait_dir));

	for(u8 i = 0; i < argc; i++)
	{
		if(argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0]))
				argv[i]++;
			if(atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				mainIOS = atoi(argv[i]);
		}
		else if(strcasestr(argv[i], "waitdir=") != NULL)
		{
			char *ptr = strcasestr(argv[i], "waitdir=");
			strncpy(wait_dir, ptr+strlen("waitdir="), sizeof(wait_dir) - 1);
		}
		else if(strcasestr(argv[i], "Waitloop") != NULL)
			wait_loop = true;
		else if(strlen(argv[i]) == 6 || strlen(argv[i]) == 4)
		{
			strcpy(gameid, argv[i]);
			for(u8 j = 0; j < strlen(gameid) - 1; j++)
			{
				if(!isalnum(gameid[j]))
				{
					gameid[0] = 0;
					break;
				}
			}
		}
			
	}
	/* Init video */
	m_vid.init();

	/* check if WiiVC */
	WiiDRC_Init();
	isWiiVC = WiiDRC_Inited();
	
	if(IsOnWiiU())
	{
		gprintf("WiiU\n");
		if(isWiiVC)
			gprintf("WiiVC\n");
		else
			gprintf("vWii Mode\n");
	}
	else 
		gprintf("Real Wii\n");
		
	gprintf("AHBPROT disabled = %s\n", AHBPROT_Patched() ? "yes" : "no");
	
	/* Init device partition handlers */
	DeviceHandle.Init();
	DeviceHandle.MountSD();// mount SD before calling isUsingUSB() duh!	
	LogToSD_SetBuffer(true);
	
	/* Init NAND handlers */
	NandHandle.Init();

	if(isWiiVC)
	{
		NandHandle.Init_ISFS();
		IOS_GetCurrentIOSInfo();
		DeviceHandle.SetModes();
	}
	else
	{
		NandHandle.Init_ISFS();
		
		/* load and check wiiflow save for possible new IOS and Port settings */
		if(InternalSave.CheckSave())
			InternalSave.LoadSettings();

		//useMainIOS = false;
			
		/* Handle (c)IOS Loading */
		if(useMainIOS && CustomIOS(IOS_GetType(mainIOS)))// load cios
		{
			NandHandle.DeInit_ISFS();
			NandHandle.Patch_AHB();
			IOS_ReloadIOS(mainIOS);
			DeviceHandle.MountSD();
			NandHandle.Init_ISFS();
			gprintf("AHBPROT disabled after IOS Reload: %s\n", AHBPROT_Patched() ? "yes" : "no");
			gprintf("Now using ");// gprintf finished in IOS_GetCurrentIOSInfo()
		}
		else
			gprintf("Using IOS58\n");// stay on IOS58. no reload to cIOS
		
		IOS_GetCurrentIOSInfo();
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
			load_ehc_module_ex();
		else if((CurrentIOS.Type == IOS_TYPE_WANIN || CurrentIOS.Type == IOS_TYPE_D2X) && CurrentIOS.Revision >= 18)
			load_dip_249();
		DeviceHandle.SetModes();
		WDVD_Init();
	}
	
	/* mount USB if needed */
	DeviceHandle.SetMountUSB(/*!sdOnly && !isWiiVC*/true);
	bool usb_mounted = DeviceHandle.MountAllUSB();// only mounts any USB if !sdOnly
	
	/* init wait images and show wait animation */
	if(strlen(gameid) == 0)// dont show if autobooting a wii game.
	{
		m_vid.setCustomWaitImgs(wait_dir, wait_loop);
		m_vid.waitMessage(0.15f);
	}

	/* init controllers for input */
	Open_Inputs();// WPAD_SetVRes() is called later in mainMenu.init() during cursor init which gets the theme pointer images

	/* sys inits */
	Sys_Init();// set reset and power button callbacks
	
	bool startup_successful = false;
	gprintf("Calling mainMenu.init(usb_mounted=%d)...\n", usb_mounted);

	// Also print to screen so we can see it even without USB Gecko
	printf("\n\n>>> mainMenu.init starting (usb_mounted=%d)\n", usb_mounted);
	VIDEO_WaitVSync(); VIDEO_WaitVSync(); VIDEO_WaitVSync();

	bool init_ok = mainMenu.init(usb_mounted);

	printf(">>> mainMenu.init returned: %s\n", init_ok ? "TRUE" : "FALSE");
	VIDEO_WaitVSync(); VIDEO_WaitVSync();
	sleep(5);  // give us 5 seconds to read the result

	if(init_ok)
	{
		startup_successful = true;
		if(!isWiiVC)
			writeStub();
		if(!isWiiVC && strlen(gameid) != 0)
			mainMenu.directlaunch(gameid);
		else
			mainMenu.main();
	}
	// at this point either wiiflow bootup failed or the user is exiting wiiflow
	ShutdownBeforeExit();// unmount devices and close inputs
	if(startup_successful)// use wiiflow's exit choice
		Sys_Exit();
	return 0;// otherwise just exit to loader (system menu or hbc).
}
