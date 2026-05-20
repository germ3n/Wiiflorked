#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>

#define DOL_PATH "sd:/apps/wiiflorked/boot.dol"

typedef struct {
    u32 text_pos[7];
    u32 data_pos[11];
    u32 text_start[7];
    u32 data_start[11];
    u32 text_size[7];
    u32 data_size[11];
    u32 bss_start;
    u32 bss_size;
    u32 entry_point;
} dolheader;

typedef void (*entrypoint)(void);
typedef struct { u32 src, dst, size; int is_text; } dol_sec_t;

static void *xfb;
static GXRModeObj *rmode;

static void init_video(void) {
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
    CON_InitEx(rmode, 20, 20, rmode->fbWidth - 40, rmode->xfbHeight - 40);
    VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
}

static void fail(const char *msg) {
    printf("\n[!] %s\n  Press any button to exit.\n", msg);
    WPAD_Init(); PAD_Init();
    while (1) {
        WPAD_ScanPads(); PAD_ScanPads();
        if (WPAD_ButtonsDown(0) || PAD_ButtonsDown(0)) break;
        VIDEO_WaitVSync();
    }
    exit(1);
}

int main(int argc, char **argv) {
    init_video();

    printf("\n WiiFlorked forwarder\n");
    printf(" Target: %s\n\n", DOL_PATH);

    printf(" * Mounting SD... "); fflush(stdout);
    if (!fatInitDefault()) fail("Mount failed");
    printf("OK\n");

    printf(" * Reading DOL... "); fflush(stdout);
    FILE *fp = fopen(DOL_PATH, "rb");
    if (!fp) fail("Open failed");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    u8 *dol = (u8 *)memalign(32, sz);
    if (!dol) fail("Alloc failed");
    if (fread(dol, 1, sz, fp) != (size_t)sz) fail("Read failed");
    fclose(fp);
    printf("OK (%ld bytes)\n", sz);

    dolheader *h = (dolheader *)dol;
    entrypoint ep = (entrypoint)h->entry_point;

    dol_sec_t secs[18];
    int n = 0;
    for (int i = 0; i < 7; i++) {
        if (h->text_size[i] && h->text_start[i] >= 0x100) {
            secs[n].src = (u32)dol + h->text_pos[i];
            secs[n].dst = h->text_start[i];
            secs[n].size = h->text_size[i];
            secs[n].is_text = 1;
            n++;
        }
    }
    for (int i = 0; i < 11; i++) {
        if (h->data_size[i] && h->data_start[i] >= 0x100) {
            secs[n].src = (u32)dol + h->data_pos[i];
            secs[n].dst = h->data_start[i];
            secs[n].size = h->data_size[i];
            secs[n].is_text = 0;
            n++;
        }
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (secs[j].dst < secs[j + 1].dst) {
                dol_sec_t t = secs[j];
                secs[j] = secs[j + 1];
                secs[j + 1] = t;
            }
        }
    }

    printf(" * Launching...\n");
    VIDEO_WaitVSync();
    VIDEO_WaitVSync();

    // Clean up SD before handing off.
    fatUnmount("sd:");
    __io_wiisd.shutdown();
    usleep(100000);

    SYS_ResetSystem(SYS_SHUTDOWN, 0, 0);

    for (int i = 0; i < n; i++) {
        memcpy((void *)secs[i].dst, (void *)secs[i].src, secs[i].size);
        DCFlushRange((void *)secs[i].dst, secs[i].size);
        if (secs[i].is_text)
            ICInvalidateRange((void *)secs[i].dst, secs[i].size);
    }
    if (h->bss_size) {
        memset((void *)h->bss_start, 0, h->bss_size);
        DCFlushRange((void *)h->bss_start, h->bss_size);
    }

    ep();
    return 0;
}
