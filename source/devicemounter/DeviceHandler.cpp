/****************************************************************************
 * Copyright (C) 2010 by Dimok
 *           (C) 2012 by FIX94
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/gcsd.h>
#include "DeviceHandler.hpp"
#include "fat.h"
#include "usbthread.h"
#include "sdhc.h"
#include "wiisd_libogc.h"
#include "usbstorage.h"
#include "usbstorage_libogc.h"
#include "loader/cios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"

DeviceHandler DeviceHandle;

void DeviceHandler::Init()
{
	/* PartitionHandle inits */
	gprintf("DeviceHandler::Init() - sd.Init()\n");
	sd.Init();
	for(int idx = 0; idx < 8; idx++)
	{
		gprintf("DeviceHandler::Init() - usb[%d].Init()\n", idx);
		usb[idx].Init();
	}
}

void DeviceHandler::SetMountUSB(bool using_usb)
{
	mount_usb = using_usb;
	gprintf("DeviceHandler::SetMountUSB() - mount_usb = %d\n", mount_usb);
}

void DeviceHandler::SetModes()
{
	sdhc_mode_sd = 1;// use libogc and ios 58 (wiisd_libogc.c)
	usb_libogc_mode = 1;// use libogc and ios 58 (usbstorage_libogc.c)
	if(CustomIOS(CurrentIOS.Type))// if wiiflow is using a cios (force cios is on)
	{
		usb_libogc_mode = 0;// use cios for USB (usbstorage.c)
		sdhc_mode_sd = 0;// use cios for SD (sdhc.c)
	}
	gprintf("DeviceHandler::SetModes() - sdhc_mode_sd = %d\n", sdhc_mode_sd);
	gprintf("DeviceHandler::SetModes() - usb_libogc_mode = %d\n", usb_libogc_mode);
}

void DeviceHandler::MountAll()
{
	gprintf("DeviceHandler::MountAll() - MountSD()\n");
	MountSD();
	gprintf("DeviceHandler::MountAll() - MountAllUSB()\n");
	MountAllUSB();
}

bool DeviceHandler::Mount(int dev)
{
	if(dev == SD)
	{
		gprintf("DeviceHandler::Mount() - MountSD()\n");
		return MountSD();
	}

	else if(dev >= USB1 && dev <= USB8)
	{
		gprintf("DeviceHandler::Mount() - MountUSB(%d)\n", dev - USB1);
		return MountUSB(dev-USB1);
	}

	return false;
}

bool DeviceHandler::MountSD()
{
	if(!sd.IsInserted() || !sd.IsMounted(0))
	{
		gprintf("DeviceHandler::MountSD() - !sd.IsInserted() || !sd.IsMounted(0)\n");
		if(CurrentIOS.Type == IOS_TYPE_HERMES)
		{	/* Slowass Hermes SDHC Module */
			gprintf("DeviceHandler::MountSD() - CurrentIOS.Type == IOS_TYPE_HERMES\n");
			for(int i = 0; i < 50; i++)
			{
				if(SDHC_Init())
					break;
				usleep(1000);
			}
		}
		gprintf("DeviceHandler::MountSD() - sd.SetDevice(&__io_sdhc)\n");
		sd.SetDevice(&__io_sdhc);
		//! Mount only one SD Partition
		gprintf("DeviceHandler::MountSD() - sd.Mount(0, DeviceName[SD], true)\n");
		return sd.Mount(0, DeviceName[SD], true); /* Force FAT, SD cards should always be FAT */
	}
	gprintf("DeviceHandler::MountSD() - true\n");
	return true;
}

bool DeviceHandler::MountUSB(int pos)
{
	gprintf("DeviceHandler::MountUSB() - pos >= GetUSBPartitionCount(pos)\n");
	if(pos < 0 || pos >= 8)
	{
		gprintf("DeviceHandler::MountUSB() - pos < 0 || pos >= 8\n");
		return false;
	}
	gprintf("DeviceHandler::MountUSB() - usb[pos].Mount(0, DeviceName[USB1+pos])\n");
	return usb[pos].Mount(0, DeviceName[USB1+pos]); 
}

bool DeviceHandler::MountAllUSB()
{
	if(!mount_usb)
	{
		gprintf("DeviceHandler::MountAllUSB() - !mount_usb\n");
		return false;
	}

	gprintf("DeviceHandler::MountAllUSB() - KillUSBKeepAliveThread()\n");
	KillUSBKeepAliveThread();

	bool result = false;

	for(int idx = 0; idx < 8; idx++) 
	{
		gprintf("DeviceHandler::MountAllUSB() - GetUSBInterface(%d)\n", idx);
		const DISC_INTERFACE* interface = GetUSBInterface(idx);
		if(interface == NULL)
		{
			gprintf("DeviceHandler::MountAllUSB() - interface == NULL\n");
			continue;
		}

		gprintf("DeviceHandler::MountAllUSB() - WaitForDevice(interface)\n");
		if(WaitForDevice(interface))
		{
			gprintf("DeviceHandler::MountAllUSB() - usb[idx].SetDevice(interface)\n");
			usb[idx].SetDevice(interface);

			if(usb[idx].GetPartitionCount() > 0)
			{
				gprintf("DeviceHandler::MountAllUSB() - usb[idx].Mount(0, DeviceName[USB1 + idx])\n");
				if(usb[idx].Mount(0, DeviceName[USB1 + idx]))
					result = true;
			}
		}
	}

	if(!result)
	{
		gprintf("DeviceHandler::MountAllUSB() - !result\n");
		gprintf("DeviceHandler::MountAllUSB() - usb[0].Mount(0, DeviceName[USB1], true)\n");
		result = usb[0].Mount(0, DeviceName[USB1], true); 
	}

	if(result && usb_libogc_mode)
	{
		gprintf("DeviceHandler::MountAllUSB() - CreateUSBKeepAliveThread()\n");
		CreateUSBKeepAliveThread();
	}

	return result;
}

bool DeviceHandler::IsInserted(int dev)
{
	if(dev == SD)
	{
		gprintf("DeviceHandler::IsInserted() - sd.IsInserted() && sd.IsMounted(0)\n");	
		return sd.IsInserted() && sd.IsMounted(0);
	}
	else if(dev >= USB1 && dev <= USB8)
	{
		gprintf("DeviceHandler::IsInserted() - usb[dev - 1].IsMounted(0)\n");
		return usb[dev - 1].IsMounted(0);
	}

	return false;
}

void DeviceHandler::UnMount(int dev)
{
	if(dev == SD)
	{
		gprintf("DeviceHandler::UnMount() - UnMountSD()\n");
		UnMountSD();
	}
	else if(dev >= USB1 && dev <= USB8)
	{
		gprintf("DeviceHandler::UnMount() - UnMountUSB(%d)\n", dev - USB1);
		UnMountUSB(dev-USB1);
	}
}

void DeviceHandler::UnMountAll()
{
	/* Kill possible USB thread */
	KillUSBKeepAliveThread();

	for(u32 i = SD; i < MAXDEVICES; i++)
	{
		gprintf("DeviceHandler::UnMountAll() - UnMount(%d)\n", i);
		UnMount(i);
	}
	USBStorage2_Deinit();
	gprintf("DeviceHandler::UnMountAll() - USBStorage2_Deinit()\n");
	USB_Deinitialize();
	gprintf("DeviceHandler::UnMountAll() - USB_Deinitialize()\n");
	SDHC_Close();

	sd.Cleanup();
	for(int idx = 0; idx < 8; idx++)
	{
		gprintf("DeviceHandler::UnMountAll() - usb[%d].Cleanup()\n", idx);
		usb[idx].Cleanup();
	}
}

void DeviceHandler::UnMountUSB(int dev)
{
	if(dev < 0 || dev >= 8)
	{
		gprintf("DeviceHandler::UnMountUSB() - dev < 0 || dev >= 8\n");
		return;
	}
	// Force local partition to 0
	usb[dev].UnMount(0);
}

void DeviceHandler::UnMountAllUSB()
{
	for(int idx = 0; idx < 8; idx++)
	{
		gprintf("DeviceHandler::UnMountAllUSB() - UnMountUSB(%d)\n", idx);
		UnMountUSB(idx);
	}
}

int DeviceHandler::PathToDriveType(const char *path)
{
	if(!path)
		return -1;

	for(int i = SD; i < MAXDEVICES; i++)
	{
		if(strncasecmp(path, DeviceName[i], strlen(DeviceName[i])) == 0)
			return i;
	}

	return -1;
}

const char *DeviceHandler::GetFSName(int dev)
{
	if(dev == SD)
		return sd.GetFSName(0);
	else if(dev >= USB1 && dev <= USB8)
	{
		if(usb[dev - 1].GetPartitionCount())
			return usb[dev - 1].GetFSName(0);
	}
	return "";
}

int DeviceHandler::GetFSType(int dev)
{
	const char *FSName = GetFSName(dev);
	if(!FSName) return -1;

	if(strncmp(FSName, "WBFS", 4) == 0)
		return PART_FS_WBFS;
	else if(strncmp(FSName, "FAT", 3) == 0)
		return PART_FS_FAT;
	else if(strncmp(FSName, "NTFS", 4) == 0)
		return PART_FS_NTFS;
	else if(strncmp(FSName, "LINUX", 4) == 0)
		return PART_FS_EXT;

	return -1;
}

u16 DeviceHandler::GetUSBPartitionCount(int port)
{
	//return usb[port].GetPartitionCount();
	return usb[port].GetPartitionCount() > 0 ? 1 : 0; // only one partition per port for now
}

wbfs_t * DeviceHandler::GetWbfsHandle(int dev)
{
	if(dev == SD)
	{
		gprintf("DeviceHandler::GetWbfsHandle() - sd.GetWbfsHandle(0)\n");
		return sd.GetWbfsHandle(0);
	}
	else if(dev >= USB1 && dev <= USB8)
	{
		gprintf("DeviceHandler::GetWbfsHandle() - usb[dev - 1].GetWbfsHandle(0)\n");
		return usb[dev - 1].GetWbfsHandle(0);
	}
	gprintf("DeviceHandler::GetWbfsHandle() - NULL\n");
	return NULL;
}

s32 DeviceHandler::OpenWBFS(int dev)
{
	u32 part_lba, part_idx = 1;
	u32 part_fs = GetFSType(dev);
	const char *partition = DeviceName[dev];

	if(dev == SD && IsInserted(dev))
	{
		gprintf("DeviceHandler::OpenWBFS() - sd.GetLBAStart(dev)\n");
		part_lba = sd.GetLBAStart(dev);
	}
	else if(dev >= USB1 && dev <= USB8 && IsInserted(dev))
	{
		gprintf("DeviceHandler::OpenWBFS() - usb[dev - 1].GetLBAStart(0)\n");
		part_idx = dev;
		part_lba = usb[dev - 1].GetLBAStart(0);
	}
	else
		return -1;

	gprintf("DeviceHandler::OpenWBFS() - WBFS_Init(GetWbfsHandle(dev), part_fs, part_idx, part_lba, partition) \n");
	gprintf("DeviceHandler::OpenWBFS() - part_fs = %d\n", part_fs);
	gprintf("DeviceHandler::OpenWBFS() - part_idx = %d\n", part_idx);
	gprintf("DeviceHandler::OpenWBFS() - part_lba = %d\n", part_lba);
	gprintf("DeviceHandler::OpenWBFS() - partition = %s\n", partition);
	return WBFS_Init(GetWbfsHandle(dev), part_fs, part_idx, part_lba, partition);
}

/* usb spinup wait for 20 seconds */
bool DeviceHandler::WaitForDevice(const DISC_INTERFACE *Handle)
{
	if(Handle == NULL)// apparently this never happens
		return false;
	time_t timeout = time(NULL);
	while(time(NULL) - timeout < 20)
	{
		gprintf("DeviceHandler::WaitForDevice() - Handle->startup() && Handle->isInserted()\n");
		if(Handle->startup() && Handle->isInserted())
			return true;
		usleep(50000);
	}
	return false;
}

bool DeviceHandler::UsablePartitionMounted()
{
	for(u8 i = SD; i < MAXDEVICES; i++)
	{
		if(IsInserted(i) && !GetWbfsHandle(i)) //Everything besides WBFS for configuration
			return true;
	}
	return false;
}

bool DeviceHandler::PartitionUsableForNandEmu(int Partition)
{
	if(IsInserted(Partition) && GetFSType(Partition) == PART_FS_FAT)
		return true;

	return false;
}

const DISC_INTERFACE __attribute__((noinline)) *DeviceHandler::GetUSBInterface(int port)
{
	if (port == 0)
		return &__io_usbstorage2_port0;
	else if (port == 1)
		return &__io_usbstorage2_port1;
	return NULL;
}
