CardWare(R) for Windows NT (CWNT) 6.00.017, July 27th, 1999
README.TXT
==========================================================================

   CONTENTS
   ========
   1) Installation of CardWare for Windows NT
   2) Loading CWNT Drivers
   3) Write Performance of Storage Cards
   4) CWNT Components
   5) Compatibility With Windows NT Versions
   6) IMPORTANT: Platform-Specific Information
   7) Release Notes
   8) Using SRAM Cards With CardWare
   9) Using Flash Cards With CardWare
  10) Operating in Silent Mode


1) Installation of CardWare for Windows NT
==========================================
   The Windows NT Setup utility (SETUP.EXE) installs CWNT on your system.
   Remove all PC Cards from their sockets before starting SETUP.EXE.
   Note that you must reboot your system after Setup finishes to load CWNT.


2) Loading CWNT Drivers
=======================
   After installation, all drivers load automatically during system
   startup. Utilities in the CWNT directory (default \Program Files\CWNT)
   can be used on demand.


3) Write Performance of Storage Cards
=====================================
   To increase performance, Windows NT uses a Write Delay Cache. Unfortunately,
   Windows NT doesn't disable the write Cache for removable media (such as
   PC Cards), thus causing lost data if the media is removed before the
   cache for the drive is flushed. To prevent this situation for drives it
   handles, PCDISK by default disables the write cache. Operating without cache
   seriously affects drive performance (approximately by a factor of 3).
   If you are absolutely sure of what you are doing, you can increase write
   speed (by this approximate factor of 3) by enabling the write cache:
   - For the current session only: Enter "PCDISK /WC:on" at the DOS prompt
     You can disable the cache again by entering "PCDISK /WC:off" at the
     DOS prompt.
   - Permanently: To the value CommandLine located in the key
     HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PCDISK\Options,
     add the string '/WC:On' at the end. Then reboot the workstation.
   - Easy alternative method: The PCDISK Properties dialog box offers
     checkboxes for both the current session and for permanent reconfiguration.
     Simply check the desired option.


4) CWNT Components
==================
   Note: Listed in alphabetical order

   - CWNT directory (default \Program Files\CWNT)
     --------------------------------------------
     CARDWARE.CNT   - Contents file of CWNT help file
     CARDWARE.HLP   - CardWare for Windows NT help file
     CWADV.DLL      - CWNT advisor DLL
     CWCUST.DLL     - CWNT custom DLL
     CWDEV.DLL      - CWNT device handling DLL
     CWDMAPI.DLL    - CWNT driver manager API DLL
     CWNOT.DLL      - CWNT notification DLL
     CWNT.WMF       - CWNT splash screen
     CWPD.DLL       - CWNT PCDISK user interface DLL
     CWPI.DLL       - CWNT PCDISK program interface DLL
     CWREM.DLL      - CWNT resource management DLL
     CWSUPOS.DLL    - CWNT startup position DLL
     DOSCARD.EXE    - Card information utility (console-based)
     PCCARD.EXE     - PCCard Control application for Windows NT
     PCCS.EXE       - Card Services device driver for 16-bit client support
     PCDISK.EXE     - CWNT memory card utility (console-based)
     PCRM.EXE       - Resource listing utility (console-based)
     PCTRAY.EXE     - CWNT system tray application
     README.TXT     - This file

   - Windows NT system32 directory %SystemRoot%\system32
     ---------------------------------------------------
     CSDLL.DLL      - Ring 3 Card Services access DLL
     CSVDD.DLL      - Card Services Virtual Device Driver (VDD)
     CWSERV.EXE     - CardWare user mode service
     CWUNIN.DLL     - CWNT uninstall DLL

   - Windows NT drivers directory %SystemRoot%\system32\drivers
     ----------------------------------------------------------
     PCATA.SYS      - ATA memory technology driver
     PCCS.SYS       - Card Services
     PCDISK.SYS     - Generic memory card enabler
     PCDRVM.SYS     - Driver manager
     PCENABLE.SYS   - Generic I/O enabler
     PCFDS.SYS      - Floppy MTD
     PCFLASH.SYS    - Flash memory technology driver with FTL support
     PCSRAM.SYS     - SRAM memory technology driver
     SSPCIC.SYS     - Socket Services (for PCIC-compatible PC Card adapters)
     SSTCIC.SYS     - Socket Services (for TCIC-compatible PC Card adapters)

     Note: Only one Socket Services driver (SSPCIC.SYS or SSTCIC.SYS)
           is installed (depending on the platform selected during CWNT
           installation).


5) Compatibility With Windows NT Versions
=========================================
   This product has been tested only under Windows NT 4.0 on Intel x86
   platforms. Currently it does not work with Windows NT 3.51.
   Don't install this release on Windows NT 3.51. Contact TouchStone Software
   for information about CardWare for Windows NT 3.51.


6) IMPORTANT: Platform-Specific Information
===========================================
   - COMPAQ Armada 4100 Series
     -------------------------
     3.3 volt cards, including CardBus cards, are not supported on Compaq
     Armada 4100 platforms.
     Some Armada 4100 series computers are equipped with PC Card controllers
     that accepts a 3.3V keyed card; however these platforms are not
     intended to support 3.3V cards, and attempts to use these cards may
     damage the card and/or the system.

   - Toshiba Libretto systems
     ------------------------
     - CardWare's PCFDS driver supports the external Y-E DATA floppy disk
       drive, which is connected via a PC Card. However, this support cannot
       coexist with the Libretto floppy drive driver that is installed via
       L70NTFDD.EXE. Also, after uninstallation of CWNT, L70NTFDD.EXE must be
       executed again to reinstall and reinvoke the Libretto floppy drive
       driver.
     - On the Libretto, all cards must be powered off (via PCTRAY Slot Power
       Off) before hibernation is executed.

   - Toshiba ToPIC PC Card controllers
     ---------------------------------
     Do not use 16-bit cards that only support 3.3V on platforms that
     use Toshiba ToPIC, ToPIC95 or ToPIC95M PC-Card controllers (e.g., on
     Toshiba platforms Tecra 700CT/530CDT/710CDT). Attempts to use these
     cards may damage the card. Please note that ToPIC95M PC Card controllers
     support 3.3V CardBus cards (e.g., on Toshiba platforms Tecra
     530CDT/710CDT), so these cards can be used without any risk on ToPIC95M
     platforms. Because the ToPIC95/95M supports voltage sense, use the SSPCIC
     switch /VS:50. Otherwise 16-bit 5V cards may not work properly.


7) Release Notes
================
   We strongly recommend that you read the following information about this
   release. It contains important hints and information about limitations and
   side effects known with this release of CWNT:

   - Previous versions of CardWare contained the entries
     'Platform with PCIC compatible controller' and
     'Platform with TCIC compatible controller' in the Setup platform list.
     These entries are replaced by
     'Unlisted Notebook (controller: PCIC family)',
     'Unlisted Desktop (controller: PCIC family)',
     'Unlisted Notebook (controller: TCIC family)',
     'Unlisted Desktop (controller: TCIC family)'.

     Select an 'Unlisted Notebook' entry during Setup if your system is
     not among the listed platforms and you install CardWare
     on a notebook that has built-in PC Card controller(s).

     Select an 'Unlisted Desktop' entry during Setup if your system is
     not among the listed platforms and you install CardWare
     on a desktop system with PC Card controller(s) plugged into
     standard ISA, EISA or PCI slot(s).

     Most PC Card controllers belong to the "PCIC family." But if you
     know that your system uses a controller that belongs to the
     "TCIC family" (e.g., most Databook controllers), select one of
     'Unlisted ...' entries for the TCIC family.

   - If you assign IP address AAA.BBB.CCC.DDD to a LAN PC Card, then
     CCC must not be unique for every adapter.  If it is, TCP/IP can't provide
     binding.

   - On removal of a modem card, a PCCS event viewer entry, "A conflict has
     been detected between two drivers which claimed two overlapping I/O port
     regions," may be logged. This behavior is related to stopping and
     restarting the serial driver after release of resources by CWNT.
     It doesn't have any impact on functionality.

   - To uninstall CWNT, follow these steps:
     a) Close all CWNT applications (e.g., PCCARD) and all
        applications that currently use the CWNT directory
        (default \Program Files\CWNT).
     b) Run 'Uninstall CardWare' in the CardWare program group.
     c) Reboot the machine.

   - Do NOT unload a Memory Technology Driver (MTD) if a PC Card is inserted
     that is currently using this MTD.
     - PCATA     is the MTD for ATA PC Cards (including flash-ATA cards.)
     - PCSRAM    is the MTD for SRAM PC Cards.
     - PCFLASH   is the MTD for linear flash PC Cards.
     - PCFDS.SYS is the floppy drive MTD.

   - Some old ATA cards may function improperly on PCI (CardBus) controllers.
     This behaviour is related to timing issues.

   - Some BIOSes don't assign an IRQ number for CardBus controllers, causing
     failure to configure CardBus cards that require an IRQ.
     You can solve this problem by using the SSPCIC command line switch
     /PI:m,n, where m is the controller number and n is the PCI IRQ number.
     Add this switch in the registry HKEY_LOCAL_MACHINE\
     \System\CurrentControlSet\Services\SSPCIC\Options.
     Alternative way: use SSPCIC command line switch /AP:on to force
     CardWare to assign IRQ numbers for CardBus controllers, where not
     assigned by BIOS.

   - New Media SCSI II bus toaster card: the driver for this card works only
     for IRQ 10. Therefore CWNT tries to configure this card to IRQ 10.
     If IRQ 10 is not available in the system, the configuration fails.

   - One version of the Western Digital ATA card works at the primary
     I/O address (0x1F0 - 0x1F7) only. Another version of the Western Digital
     ATA card works at the primary and secondary I/O addresses (0x170 - 0x177).
     To use those cards, use PCCARD to remove exclusions for I/O addresses
     0x1F0-0x1F7 and/or I/O 0x170-0x177.

   - With the Databook 86082A PC Card controller, sometimes writing to flash
     PC Cards fails (error message "Device not functioning").

   - When you format flash cards using the command PCDISK /TL:n
     (n = socket/drive) at the DOS prompt, some time may elapse before the
     DOS prompt returns after you enter the volume label. This is normal
     behavior, not a system lock up.

   - With the Ricoh RB5C478 PC Card controller, some ATA cards that are
     non-compliant to the PC Card Standard do not work, because card voltage
     is reported improperly to the controller.


8) Using SRAM Cards With CardWare
=================================
   CardWare can not recognize a blank (unformatted) SRAM card with the
   write-protect switch set to ON. If you insert a new, unformatted SRAM
   card and DOSCARD/PCCARD reports

      'Unable to get the card name'
      'Card was not configured'

   remove the card from the slot and check the write-protect switch.
   The switch should be set to OFF.

   Once you have formatted the card, you can use the WP switch to prevent
   software from writing to this card.


9) Using Flash Cards With CardWare
==================================
   CardWare provides full, built-in FTL support.

   FTL support lets you to work with flash cards the same way you work
   with other disk drives, using many of the same standard programs
   (for example, you can use format, dir, etc.). But keep in mind that
   the storage card is only emulating a disk drive. For this reason it
   makes no sense to run SCANDISK or a disk defragmentator on such a card.
   To use built-in FTL support, you must first prepare an FTL partition on
   the flash card. If there is no FTL partition on the card, create one
   by running the CardWare utility 'PCDISK.EXE /TL:DrvLetter' at the DOS
   prompt, where DrvLetter is the drive letter assigned to the flash card.
   This operation includes card formating, so the flash card should be
   accessible and usable afterwards.

   CardWare also contains built-in support for Flash File System II.
   The built-in FFS2 support allows you to read/write files on an FFS2
   partition, format a flash card in FFS2 format, and check the FFS2
   structure (see more details by typeing PCDISK.EXE /? at the DOS prompt).
   CardWare's built-in FFS support is fully compatible with Microsoft's
   FFS2 version.


10) Operating in Silent Mode
===========================
   The Setup program now supports the command line switch "silent" for
   installation without user interaction. Executing "setup silent" installs
   CardWare for Windows NT with the default values for the detected platform
   without any user interaction.
