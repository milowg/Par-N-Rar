Par-N-Rar v1.3

Par-N-Rar comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

For more information, see http://www.milow.net/site/projects/parnrar.html
For questions, see the fourms at http://www.milow.net/forums

Check help file for usage information.


***************************************************************************
Version History:

1/29/08 - version 1.3
----------------------------------------------------------------------------
* FIXED - Files within folders inside of RAR archives were overwriting other files of the same name in the destination directory

* FIXED - If a file is in the middle of being extracted and something happens (machine dies, PNR crashes, etc) the next time PNR is started it will delete the file.
	
* FIXED - Got rid of "Can't find ListItem" popup message boxes

* ADDED - Ability to skip PAR sets during processing. NOTE: This will not skip the processing if files are in the process of being repaired or extracted.

* ADDED - Minimize to tray as startup option (/min)

* ADDED - If a RAR file set contains a RAR file, it will get extracted to the monitor directory and the "inner" RAR set will be decompressed.

* CHANGED - Password form for RAR files now times out after 30 seconds.

* CHANGED - Moved check for "delete par/rar files when done" option to after a repair rather than before it.

* CHANGED - Par-N-Rar is now portable. All settings are written to a file in the Par-N-Rar directory instead of the registry.


4/9/07- version 1.24
----------------------------------------------------------------------------
* FIXED - Par-N-Rar could crash when certain new files are found on subsequent iterations

* FIXED - When the extract directory was a UNC Path (such as "\\server\dir"), Par-N-Rar would show "Extracting" but not actually extract to that server but rather to a folder called "\server\dir"

* ADDED - New option for the current item being worked on to always be in focus

* ADDED - Added Close to tray option which can be disabled so that the close button actually closes Par-N-Rar instead of minimizing it


2/22/07 - version 1.23.3
----------------------------------------------------------------------------

* FIXED - SFV files without PAR sets were getting "Could not extract" errors

* CHANGED - Install now spawns off the uninstaller for any previously installed versions of PNR

* ADDED -  Context menu option to open directory for a PAR/SFV/RAR set


2/18/07 - version 1.23.2
----------------------------------------------------------------------------

* FIXED - PAR files from all found sets would be deleted when the first PAR set finished, leaving only RAR files behind


2/15/07 - version 1.23.1
----------------------------------------------------------------------------

* FIXED - PNR would crash when an SFV file, a PAR file, and a RAR set were all present at startup


2/11/07 - version 1.23
----------------------------------------------------------------------------
* FIXED - PNR would crash when dealing with an invalid PAR2 set (created by Unison Newsreader for Macs)

* FIXED - Issue with directories inside of RAR files being extracted wrong (instead of dir/file it would be dir/dir/file)

* FIXED - Issue with using %m/%s as the extract directory when split files are found

* CHANGED - "Could not UnRar" status changed to "Done" to avoid confusion

* CHANGED - Renamed files that have been fixed (ie. file.ext.1, file.ext.2, etc) are now deleted BEFORE extraction if the Delete When Done option is set to save disk space

* ADDED - Window position is now remembered and restored at startup

* ADDED - Support for RAR/split sets that are not part of a PAR set

* ADDED - /? command line option to show dialog of all command line options


11/9/06 - version 1.22.2
----------------------------------------------------------------------------
* FIXED - Par-N-Rar sometimes hangs when the "delete empty directories" option is checked  (thanks to Brian W for helping out with this)

* ADDED - A Debug folder to the Start Menu folder for Par-N-Rar to allow running PNR in debug mode

* ADDED - System tray icon is now animated while Par-N-Rar is processing


10/19/05 - version 1.22.1
----------------------------------------------------------------------------
* FIXED - When one RAR set fails to unrar, all RAR sets after it will unrar but also then show "Could not UnRar"

* FIXED - Clicking the projects link on the about box crashes PNR

* FIXED - When clicking the X to minimize when minimize to tray is on, processing may stop


10/4/06 - version 1.22
----------------------------------------------------------------------------
* FIXED - When a RAR file has a password set, and the cancel button is hit, ParNRar would not move on to the next set

* FIXED - When a RAR file has a password set, and the wrong password is entered, ParNRar crashes

* FIXED - When a folder is named .sfv or .par, ParNRar crashes

* FIXED - Problems with PNR crashing when extracting PAR files with spaces or dots at the end of their filenames when the destination directory has %s in it. (thanks Miciah for helping with this bug)

* ADDED - Ability to check for a new version of ParNRar. Also added an option to check on a periodic basis (every week/month/never)

* CHANGED - Close button (X in the top right) now minimizes to the system tray if that option is on

* ADDED - Ability to delete empty directories after checking them for PAR sets (this feature is now on by default and can be turned off from the options page)

* ADDED - A Clear History button on the main dialog
	


6/13/06 - version 1.21
----------------------------------------------------------------------------
* FIXED - Crash when done directory is blank and file set could not be unrared

* CHANGED - Par-N-Rar now automatically pauses when you run out of hard disk space

* ADDED - Ability to verify only (not repair and extract) PAR sets

* ADDED - The Monitor Directories are now stored in a drop-down list instead of a single edit box

* ADDED - Ability to drag and drop a folder onto the 'Monitor Directory' box

* ADDED - An option to do different things when a scan is done (restart, beep, shutdown, etc)



5/22/06 - version 1.20.3
----------------------------------------------------------------------------
* FIXED - Crash on some files when RAR extraction fails

* FIXED - Delete Files from the context menu was not deleting all the files properly

* FIXED - Crash when PAR2 files were corrupt or 0 length

* FIXED - SFV sets that contained the same files as PAR sets were being treated as a seperate set

* FIXED - Crash when PNR scans a PAR set the second time if any files are damaged and there are new blocks for repair (thanks Andreas Schmid for help with this code)



5/14/06 - version 1.20.2
----------------------------------------------------------------------------
* FIXED - When a RAR file contains directories, the files in them were being extracted to the wrong directory

* FIXED - Crash with split files when the "done directory" was blank

* ADDED - Better error messages for PAR2 errors



5/10/06 - version 1.20.1
----------------------------------------------------------------------------
* FIXED - Option to send deleted files to Recycle Bin was missing from Options dialog



5/9/06 - version 1.20
----------------------------------------------------------------------------
* ADDED - Par-N-Rar now remembers files it has validated in the past and does not validate them twice (as long as Par-N-Rar is not closed).

* ADDED - Support for split files that have their PAR2 files created based on the original file. 
          Note that these split files are not remembered upon subsequent validations.

* ADDED - Option to minimize to System Tray. This option will be checked by default.

* CHANGED - Instead of "Repair not possible" there is now a status showing how many blocks/files missing.

* CHANGED - SFV files that are contained in a PAR set are deleted along with the PAR set

* FIXED - If the "move extra files" option is checked and the done directory is blank those extra files were not getting deleted.

* FIXED - If the done directory was not previously created, a join of split files was failing.

* FIXED - Problems with PAR2 files that have no name (ex. ".PAR2")

* FIXED - Sometimes when multiple items were selected, the Remove context menu would only remove some of them.



3/22/06 - version 1.15
----------------------------------------------------------------------------
* FIXED - If a PAR and SFV both reference the same file set, they will be treated as one item and only the PAR will be used.

* FIXED - RAR files with subdirectories in them were creating invalid directories like "DirectoryName ("

* CHANGED - Multiple selection is now allowed

* CHANGED - To avoid taxing the hard drive and running inefficiently, PNR will now only verify sets when no other sets are being processed.

* ADDED - Main window can now be resized and size will be remembered on program exit.

* ADDED - "Remove Done" menu item.

* ADDED - Rescan menu option

* ADDED - Command line parameters (go on start and monitor directory)



2/22/06 - version 1.14
----------------------------------------------------------------------------
* FIXED: Sometimes PAR/PAR2 files were still not being removed

* FIXED: Lots of cases where "Can't find list item" would pop up and then Par-N-Rar would crash

* ADDED: Remove All menu option

* ADDED: Help file

* ADDED: Hotkeys for Go, Pause, Continue, Options

* ADDED: Option to delete to the Recycle Bin 

* ADDED: Directory where PAR file set is found to details

* ADDED: Support for PAR sets with multiple RARs in them

* ADDED: Support for SFV/MD5 files. Par-N-Rar will now verify SFV/MD5 file sets. Par-N-Rar will NOT repair these sets if they are incomplete.

* ADDED: Option to set Par-N-Rar's system priority

* ADDED: %m option for monitored directory

* ADDED: %d option for path off of monitored directory

* ADDED: Support for combining split files found in a PAR set. Files that have extension .001, .002, etc, will be combined instead of moved to the "done directory"

* FIXED: Backup copies of repaired files (filename.ext.1, etc) will be removed along with the PARs

KNOWN ISSUES:
* If both PAR/PAR2 and SFV files are found for the same files, they will be listed twice. 



1/5/06 - version 1.13
----------------------------------------------------------------------------
* FIXED: The main PAR/PAR2 files were not being deleted when extraction was done

* CHANGED: Status window is now a list view rather than scrolling text. Also added details to each line when done

* ADDED: Option to move Non-Rar files to another directory

* ADDED: Option to move non-RAR files found in PARed set to "done directory" along with any extracted files

* ADDED: Option to have Par-N-Rar restart scanning after a certain delay interval



12/18/05 - version 1.12
----------------------------------------------------------------------------
* ADDED: Pause button

* ADDED: Support for RARs with passwords (also stops deleting RAR files if there is a password problem and extraction is not complete)

* FIXED: PNR files were being created for some files with non-PAR extensions

* FIXED: PNR files were not being deleted along with the rest of the file set when the "delete when done" option is set



12/08/05 - version 1.11
----------------------------------------------------------------------------
* FIXED: Recognized RAR files that start with a .001 extension

* FIXED: Par-N-Rar was not working properly when the Extract To directory was blank in the options

* ADDED: Option to recurse through subdirectories to search for PAR files



11/26/05 - version 1.10
----------------------------------------------------------------------------
* FIXED: In the case where a PAR can't be repaired, Par-N-Rar would try the same PAR set N times, where N is the number of PAR files available from the PAR set. This has been fixed.

* FIXED: If a file to be extracted already exists, the file is extracted to "FILENAME (n).EXT" instead.

* ADDED: Option to extract files to another directory, which can use the name of the PAR file in its path.

* ADDED: Option to not delete PAR/RAR files when done extracting. These files will not be verified the second time Par-N-Rar is run.

* CHANGED: Par-N-Rar no longer moves non-RAR files to another directory. These files will not be verified the second time Par-N-Rar is run.

* CHANGED: Par-N-Rar is now GUI-based instead of command-line based.



11/07/05 - version 1.00
----------------------------------------------------------------------------
* Initial Version. 
