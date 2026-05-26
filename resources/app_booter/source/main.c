/* Copyright 2011 Dimok
   This code is licensed to you under the terms of the GNU GPL, version 2;
   see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt  */
#include <gccore.h>
#include <stdio.h>
#include <stdarg.h>
#include "string.h"

#include "dolloader.h"
#include "elfloader.h"
#include "sync.h"

#define EXECUTABLE_MEM_ADDR     0x92000000
#define SYSTEM_ARGV             ((struct __argv *)0x93300800)

void _main(void)
{
    void *exeBuffer = (void *)EXECUTABLE_MEM_ADDR;
    u32 exeEntryPointAddress = 0;
    entrypoint exeEntryPoint;

    if(valid_elf_image(exeBuffer) == 1)
        exeEntryPointAddress = load_elf_image(exeBuffer);
    else
        exeEntryPointAddress = load_dol_image(exeBuffer);

    exeEntryPoint = (entrypoint)exeEntryPointAddress;
    if(!exeEntryPoint)
        return;

    if(SYSTEM_ARGV->argvMagic == ARGV_MAGIC)
    {
        void *new_argv = (void *) (exeEntryPointAddress + 8);
        memcpy(new_argv, SYSTEM_ARGV, sizeof(struct __argv));
        sync_before_exec(new_argv, sizeof(struct __argv));
    }

    u64 target_id = *(u64*)0x8000180C;

    if (target_id != 0) {
        // Priiloader magic bypass to intercept SYS_RETURNTOMENU
        *(volatile u32*)0x8132FFF0 = 0x50756E65; 
        *(volatile u32*)0x8132FFF4 = (u32)(target_id >> 32);
        *(volatile u32*)0x8132FFF8 = (u32)(target_id & 0xFFFFFFFF);
    }

    // Force FCEUX-TX to use SYS_RETURNTOMENU instead of exit(0)
    // Manually zeroing out memory to avoid memset/libc dependency
    volatile u32 *stub_mem = (volatile u32*)0x80001800;
    for(u32 idx = 0; idx < 4; ++idx) {
        stub_mem[idx] = 0;
    }

    // Use app_booter's native bare-metal cache flush instead of libogc
    sync_before_exec((void*)0x80001800, 16);

    exeEntryPoint();
}