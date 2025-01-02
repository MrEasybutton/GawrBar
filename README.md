# GawrBar 🦈
### 🔱 The Taskbar Customisation Utility from the depths of Atlantis 🔱

---

GawrBar is a tool made to modify your taskbar alignment and appearance in a lightweight manner.
It's very similar in function to RoundedTB but very experimental. If RoundedTB is Gawr Gura, this is Gabriella Giselle.

## 💎 Features:

### Three modes: 
#### Shoreline (Docked)
Keeps the vanilla look of the taskbar with normalised margins, creating a sleek floating dock. You can also align your taskbar either left or center.
#### Core (Centered Dynamic)
Creates a floating MacOS-esque dock that adjusts to the number of apps in your taskbar.
#### Tide (Split Dynamic)
Creates two docked zones, one for your apps and one for your system tray, notifs and clock.
### Useless control panel
It sucks, sorrgy
### Dynamic Taskbar
The Core and Tide modes expand as you open apps, keeping your taskbar segmented for...whatever reason.

## ⚡ Use
If you're looking to use GawrBar, you can download the .exe in the Releases page and run that. 

It should apply immediately, and if it doesn't you may need to check your Task Manager to see if GawrBar.exe is running. Sadly constant issues here will mean that you can't run GawrBar.
If the right side of your taskbar does not have the rounded corners, your resolution may largely differ from mine, but this is unlikely to be the case as I have also tested GawrBar in VMs with several extreme resolutions.

If you're looking to fork, clone or take a look at GawrBar's code, you can open the source code in Visual Studio 2022.

## 🐛 Bugs, Issues and Troubleshooting
### Virus Warning
If Microsoft Defender or your antivirus is grrr at GawrBar, you can ignore that. The detection is due to GawrBar using the Windows registry to adjust the alignment of the taskbar.
If you are still unsure of GawrBar's safety you can create a restore point or test it out in Windows Sandbox/VM first. I'd like to remind you that GawrBar is an experimental tool that has some nonfatal bugs.
### Dynamic Mode
Dynamic Mode works mostly, but it has issues detecting when a pinned app is opened, leading to it expanding when you open something like File Explorer or the System Tray. This will be fixed eventually.
### Closing GawrBar
I recommend you close GawrBar the normal way by right-clicking its icon in the System Tray and clicking 'Exit' or alternatively closing the Control Panel window.

In the case that you close GawrBar through a third-party app manager or the Task Manager, you will likely see that the taskbar does not revert/reset to your previous arrangement.

You can fix this by restarting Windows Explorer in your Task Manager.

## 📷 Gallery

Shoreline Mode (Left-Aligned):
![Shoreline (Left-Align)](https://github.com/user-attachments/assets/c5888e39-ef12-43c2-8397-9182d172a4ba)
Core Mode:
![Core](https://github.com/user-attachments/assets/4411db8a-4aac-4072-8e59-452fa01ce2c6)
Tide Mode:
![Tide](https://github.com/user-attachments/assets/39f6d181-0ae5-4a1b-809c-af7bd55c7692)



