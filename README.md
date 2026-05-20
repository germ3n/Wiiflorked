# WiiFlorked: Dual-Drive Setup Guide

## Step 1: The Dual-Port cIOS Foundation
Install the required cIOS banks via the d2x cIOS Installer.

Bank 1 (Port 0 / usb1):
- cIOS: d2x v11 beta3
- Slot 248: Base 38
- Slot 249: Base 56
- Slot 250: Base 57
- Slot 251: Base 58

Bank 2 (Port 1 / usb2):
- cIOS: d2x v10 beta53-alt
- Slot 208: Base 38
- Slot 209: Base 56
- Slot 210: Base 57
- Slot 211: Base 58

## Step 2: Installation
Simply drag the wiiflorked folder into sdcard:/apps/

## Step 3: Drive Layout Configuration
- Port 0 (usb1): Place accessory-dependent games or those requiring strict timing (e.g., Driver: San Francisco, Rock Band).
- Port 1 (usb2): Place standard Wii and GameCube backups here.





Launch WiiFlorked via HBC. The cache builder merges both drives into a unified view, and the cIOS router dynamically shifts between banks based on physical drive location. Leave game settings on AUTO.
Don't forget to 'Rebuild Cache' if you already use regular Wiiflow.



This might break some things in Wiiflow, but launching Wii & Gamecube games from both usb drives works, I'll get around to plugins etc soon





# WiiFlow Lite
My mod of the Wii USB Loader WiiFlow

## Description
WiiFlow Lite is a wii homebrew app used to display and launch your games and apps stored on a USB device or SD card plugged into a Wii or Wii U in Wii mode. The games and apps are displayed in cover flow style display.

## Installing
As of v5.2.0 WiiFlow Lite will simply be a replacement for WiiFlow. Put it in apps/wiiflow and use wiiflow forwarder's to launch it via the wii system menu. forwarders can be found on wiiflowiki4. for previous wiiflow lite users, sorry but you must uninstall your wiiflow lite forwarder and replace it with a wiiflow forwarder.

Simply download the latest release and extract it to your apps/wiiflow folder on SD or USB HDD. SD is recommended. Your device should be formatted to FAT32.

## Booting
To start WiiFlow Lite you will need the Homebrew Channel or a WiiFlow forwarder channel installed on your Wii or vWii system menu.

## Themes
Currently only Rhapsodii and Rhapsodii Shima themes are compatible with WiiFlow Lite. Other older wiiflow themes need to be updated to work properly with WFL.

Rhapsodii made by Hakaisha is a new theme designed for wiiflow lite. find it here - (https://gbatemp.net/threads/wiiflow-lite-theme-rhapsodii.511833/)

Other wiiflow lite themes can be found on the wiki linked below. but they need to be updated to properly work with wiiflow lite.

## Useful Links
[WiiFlow Lite GBATemp thread](https://gbatemp.net/threads/wiiflow-lite.422685/)

[WiiFlow Wiki](https://web.archive.org/web/20220414124727/https://sites.google.com/site/wiiflowiki4/)

[Newer Wiki WIP](https://sites.google.com/view/wiiflow-wiki/welcome)

[Github Wiki](https://github.com/Fledge68/WiiFlow_Lite/wiki)

[Old Sourceforge Project Repository](https://sourceforge.net/projects/wiiflow-lite/)
