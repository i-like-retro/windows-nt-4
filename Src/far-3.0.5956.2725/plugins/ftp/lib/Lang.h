﻿#ifndef __FAR_PLUGIN_FTP_LANGUAGE
#define __FAR_PLUGIN_FTP_LANGUAGE

enum
{
	MNone__,                         //""
	MLanguage,                       //"English"

	MColumn9,                        //"N,S,O,C0,C1"
	MSizes9,                         //"0,15,15,10,15"
	MFullScreen9,                    //"1"
	MColumn0,                        //"N,C1"
	MSizes0,                         //"0,0"
	MFullScreen0,                    //"0"

	MOk,                             //"OK"
	MYes,                            //"&Yes"
	MNo,                             //"&No"
	MSkip,                           //"S&kip"
	MRetry,                          //"Retry"
	MCancel,                         //"Cancel"
	MCopy,                           //"Copy"
	MError,                          //"Error"

	MFtpDiskMenu,                    //"FTP"
	MFtpMenu,                        //"FTP client"

	MShiftF4,                        //"NewURL"
	MShiftF7,                        //"Tables"
	MAltF6,                          //"SavURL"

	MConfigTitle,                    //"FTP client"
	MConfigAddToDisksMenu,           //"Add to &Disks menu.  Item:"
	MConfigAddToPluginsMenu,         //"Add to &Plugins menu"
	MConfigDizNames,                 //"Description file &names (comma delimited):"
	MConfigReadDiz,                  //"&Read descriptions"
	MConfigUpdateDiz,                //"&Update descriptions"
	MConfigUploadLowCase,            //"Upload upper in lower&case"
	MConfigUploadDialog,             //"&Show upload options dialog"
	MConfigDefaultResume,            //"De&fault button is 'Resume'"
	MConfigDefPassword,              //"D&efault password"
	MConfigFirewall,                 //"F&irewall:port"
	MConfigPassiveMode,              //"Passi&ve mode"

	MCannotConnectTo,                //"Cannot connect to"
	MConnectionLost,                 //"Connection lost. Restore it?"
	MRestore,                        //"Restore"

	MWaitingForResponse,             //"Waiting for response"
	MSendingName,                    //"Sending user name"
	MPasswordName,                   //"Sending password"
	MRequestingFolder,               //"Requesting folder name"

	MFtpTitle,                       //"FTP"
	MEditFtpTitle,                   //"Edit FTP address"
	MEnterFtpTitle,                  //"Enter FTP address"
	MSaveFTPTitle,                   //"Save FTP address"
	MFtpName,                        //"&ftp://user:password@host:port/dir"
	MUser,                           //"&User       :"
	MPassword,                       //"&Password   :"
	MHostDescr,                      //"&Description:"
	MAskLogin,                       //"&Ask for password every time"
	MAsciiMode,                      //"ASC&II mode"
	MPassiveMode,                    //"Passi&ve mode"
	MUseFirewall,                    //"Use fi&rewall"
	MFtpSave,                        //"&Save"
	MFtpConnect,                     //"&Connect"
	MFtpSelectTable,                 //"Se&lect table"

	MFtpInfoFTPClient,               //"FTP client"
	MFtpInfoHostName,                //"Host name"
	MFtpInfoHostDescr,               //"Host info"
	MFtpInfoHostType,                //"Host type"
	MFtpInfoFtpTitle,                //"FTP title"

	MRenameTitle,                    //"Rename/Move"

	MDownloadTitle,                  //"Download"
	MDownloadTo,                     //"Download selected &files to"
	MDownloadAscii,                  //"&ASCII mode"
	MDownload,                       //"&Download"
	MDownloadCancel,                 //"&Cancel"
	MCannotDownload,                 //"Cannot download file"

	MUploadTitle,                    //"Upload"
	MUploadTo,                       //"Upload selected &files to"
	MUploadAscii,                    //"&Ascii mode"
	MUpload,                         //"&Upload"
	MUploadCancel,                   //"&Cancel"
	MCannotUpload,                   //"Cannot upload file"

	MAlreadyExist,                   //"File already exists"
	MAlreadyExistRO,                 //"Read-only file already exists"
	MOverwrite,                      //"&Overwrite"
	MOverwriteAll,                   //"&All"
	MCopySkip,                       //"&Skip"
	MCopySkipAll,                    //"S&kip all"
	MCopyResume,                     //"&Resume"
	MCopyCancel,                     //"&Cancel"

	MStatusDownload,                 //"Download"
	MStatusUpload,                   //"Upload"
	MStatusCPS,                      //"/s"
	MStatusTotal,                    //"Total"
	MStatusTime,                     //"Elapsed/remaining time"

	MDeleteTitle,                    //"Delete from FTP"
	MDeleteFiles,                    //"Delete selected files from FTP host"
	MDeleteNumberOfFiles,            //"Delete %d files from FTP host"
	MDeleteFolder,                   //"Delete the folder from FTP host"

	MDeleteHostsTitle,               //"Delete from FTP list"
	MDeleteHosts,                    //"Delete selected FTP hosts from the list"
	MDeleteNumberOfHosts,            //"Delete %d FTP hosts from the list"
	MDeleteHostFolder,               //"Delete folder from the list"

	MDeleteDelete,                   //"Delete"
	MDeleteCancel,                   //"Cancel"

	MDeleteGroupDelete,              //"&Delete"
	MDeleteGroupAll,                 //"&All"
	MDeleteGroupCancel,              //"&Cancel"

	MCannotDelete,                   //"Cannot delete"

	MDeleting,                       //"Deleting the file or folder"

	MMkdirTitle,                     //"Make folder"
	MMkdirName,                      //"Create the folder"
	MChDir,                          //"Change the folder"

	MCopyHostTitle,                  //"Copy host records"
	MCopyHostTo,                     //"Copy information about selected &hosts to"
	MMoveHostTitle,                  //"Move host records"
	MMoveHostTo,                     //"Move information about selected &hosts to"

	MCannotCopyHost,                 //"Cannot copy host information"
	MHostAlreadyExist,               //"Host already exists"
	MHostAskOverwrite,               //"Do you wish to overwrite the old host information"

	MLoginInfo,                      //"Login information"
	MUserName,                       //"User &name"
	MUserPassword,                   //"User &password"

	MHostColumn,                     //"Host"
	MUserColumn,                     //"User"
	MHomeColumn,                     //"Home"
	MDescColumn,                     //"Description"
	MFileMode,                       //"Access"
	MLink,                           //"Link"

	MTableTitle,                     //"Tables"
	MTableAuto,                      //"Autodetect"
	MTableDefault,                   //"Default"

	MChmodTitle,                     //"Set attributes"
	MCannotChmod,                    //"Cannot set file attributes"
	MChangingAttr,                   //"Setting attributes"

//JM
	MTerminateOp,                    //"Terminate current operation?"
	MTerminateConnection,            //"Terminate current connection?"
	MAttention,                      //"Attention..."
	MLogFileName,                    //"FARFtp.log"
	MIntError,                       //"Internal error!"
	MCreateDirError,                 //"Unable to create directory"
	MChangeDirError,                 //"Unable to change directory"
	MNotPlainFile,                   //"Not a plain file"
	MProxyNoThird,                   //"Proxy server does not support third party tran"
	MNotConnected,                   //"No primary connection"

	MResmResume,                     //"Resuming"
	MResmSupport,                    //"Supported"
	MResmNotSupport,                 //"Not supported"
	MResmNotConnected,               //"Not connected"

	MWaiting,                        //"Waiting for data "
	MSeconds,                        //" sec"
	MScaning,                        //"Scanning folder..."
	MAutoRetryText,                  //"(R)etry in"
	MRetryText,                      //"{Enter - retry; ESC - cancel}"

	MWSAEINTR,                       //"WSAEINTR: The (blocking) call was canceled thr"
	MWSAEBADF,                       //"WSAEBADF"
	MWSAEACCES,                      //"WSAEACCES: The requested address is a broadcas"
	MWSAEFAULT,                      //"WSAEFAULT: The addrlen argument is too small o"
	MWSAEINVAL,                      //"WSAEINVAL: listen was not invoked prior to acc"
	MWSAEMFILE,                      //"WSAEMFILE: No more socket descriptors are avai"
	MWSAEWOULDBLOCK,                 //"WSAEWOULDBLOCK: The socket is marked as nonblo"
	MWSAEINPROGRESS,                 //"WSAEINPROGRESS: A blocking Windows Sockets 1.1"
	MWSAEALREADY,                    //"WSAEALREADY"
	MWSAENOTSOCK,                    //"WSAENOTSOCK: The descriptor is not a socket"
	MWSAEDESTADDRREQ,                //"WSAEDESTADDRREQ"
	MWSAEMSGSIZE,                    //"WSAEMSGSIZE: The message was too large to fit"
	MWSAEPROTOTYPE,                  //"WSAEPROTOTYPE: The specified protocol is the w"
	MWSAENOPROTOOPT,                 //"WSAENOPROTOOPT"
	MWSAEPROTONOSUPPORT,             //"WSAEPROTONOSUPPORT: The specified protocol is"
	MWSAESOCKTNOSUPPORT,             //"WSAESOCKTNOSUPPORT: The specified socket type"
	MWSAEOPNOTSUPP,                  //"WSAEOPNOTSUPP: The referenced socket is not a"
	MWSAEPFNOSUPPORT,                //"WSAEPFNOSUPPORT"
	MWSAEAFNOSUPPORT,                //"WSAEAFNOSUPPORT: The specified address family"
	MWSAEADDRINUSE,                  //"WSAEADDRINUSE"
	MWSAEADDRNOTAVAIL,               //"WSAEADDRNOTAVAIL"
	MWSAENETDOWN,                    //"WSAENETDOWN: The network subsystem or the asso"
	MWSAENETUNREACH,                 //"WSAENETUNREACH"
	MWSAENETRESET,                   //"WSAENETRESET: The connection has been broken d"
	MWSAECONNABORTED,                //"WSAECONNABORTED: The virtual circuit was termi"
	MWSAECONNRESET,                  //"WSAECONNRESET: The virtual circuit was reset b"
	MWSAENOBUFS,                     //"WSAENOBUFS: No buffer space is available. The "
	MWSAEISCONN,                     //"WSAEISCONN"
	MWSAENOTCONN,                    //"WSAENOTCONN: The socket is not connected (conn"
	MWSAESHUTDOWN,                   //"WSAESHUTDOWN: The socket has been shut down; i"
	MWSAETOOMANYREFS,                //"WSAETOOMANYREFS"
	MWSAETIMEDOUT,                   //"WSAETIMEDOUT: The connection has been dropped,"
	MWSAECONNREFUSED,                //"WSAECONNREFUSED"
	MWSAELOOP,                       //"WSAELOOP"
	MWSAENAMETOOLONG,                //"WSAENAMETOOLONG"
	MWSAEHOSTDOWN,                   //"WSAEHOSTDOWN"
	MWSAEHOSTUNREACH,                //"WSAEHOSTUNREACH: The remote host cannot be rea"
	MWSAENOTEMPTY,                   //"WSAENOTEMPTY"
	MWSAEPROCLIM,                    //"WSAEPROCLIM"
	MWSAEUSERS,                      //"WSAEUSERS"
	MWSAEDQUOT,                      //"WSAEDQUOT"
	MWSAESTALE,                      //"WSAESTALE"
	MWSAEREMOTE,                     //"WSAEREMOTE"
	MWSASYSNOTREADY,                 //"WSASYSNOTREADY"
	MWSAVERNOTSUPPORTED,             //"WSAVERNOTSUPPORTED"
	MWSANOTINITIALISED,              //"WSANOTINITIALISED: A successful WSAStartup mus"
	MWSAEDISCON,                     //"WSAEDISCON"
	MWSAHOST_NOT_FOUND,              //"WSAHOST_NOT_FOUND: Authoritative Answer Host n"
	MWSATRY_AGAIN,                   //"WSATRY_AGAIN: Non-Authoritative Host not found"
	MWSANO_RECOVERY,                 //"WSANO_RECOVERY: Non recoverable errors, FORMER"
	MWSANO_DATA,                     //"WSANO_DATA: Valid name, no data record of requ"
	MWSANO_ADDRESS,                  //"WSANO_ADDRESS: No address, look for MX record"

	MWSAENoError,                    //"Operation successed"
	MWSAEUnknown,                    //"Unknown socket error"
	MWSANotInit,                     //"Windows socket error"
	MKeepAliveExec,                  //"Keepalive..."
	MConnecting,                     //"Connecting..."
	MErrRename,                      //"Can not rename file"
	MErrOtherRename,                 //"You can only move or rename to another FTP panel"

	MResolving,                      //"Resolving address..."
	MWaitingForConnect,              //"Waiting for connect..."

	MHostsMode,                      //"Hosts panel &mode"
	MKeepAlive,                      //"&Keepalive packet every"
	MAutoRetry,                      //"Timeout retr&y count:"
	MLongOp,                         //"&Long operation beep: >="
	MAskAbort,                       //"Confirm &abort"
	MDigitDelimit,                   //"Digits &grouping symbol:"
	MExtWindow,                      //"FTP command log &window"
	MExtSize,                        //"Log window size:"
	MHostIOSize,                     //"I/O &buffer size:"
	MExtOpt,                         //"Extended options"
	MSec,                            //"s"
	MKBytes,                         //"K"
	MLogFilename,                    //"Log file        Max si&ze:"
	MLogDir,                         //"L&og directory contents"
	MWaitTimeout,                    //"Server reply &timeout (s)"
	MShowIdle,                       //"Idle &header"
	MColor,                          //"Color"
	MScreen,                         //"&1.Screen"
	MCaption,                        //"&2.Caption"
	MBoth,                           //"&3.Both"
	MSilentText,                     //"Alert te&xt"
	//
	MColorTitle,                     //"Color "
	MColorFore,                      //"&Foreground"
	MColorBk,                        //"&Background"
	MColorColorSet,                  //"&Set"
	MColorText,                      //"Text Text Text Text Text Text"

	MShiftF1,                        //"Utils"
	MUtilsExtCmd,                    //"FTP command log &window"
	MUtilsDir,                       //"View &directory contents"
	MUtilsCmd,                       //"View FTP &command log"
	MErrorTempFile,                  //"Error creating temporary file"
	MDirTitle,                       //"Directory contents"
	MUtilsLog,                       //"View &log file"
	MLogTitle,                       //"FTP log file"
	MUtilsCaption,                   //"FTP Utilities"
	MExtList,                        //"Custom LIS&T command"
	MUtilsPassive,                   //"&Passive mode"
	MUtilsASCII,                     //"&ASCII mode"
	MUtilsProcessCmd,                //"Process c&md line"

	MReaded,                         //"Reading: "

	MInclude,                        //"&Include only files:"
	MExclude,                        //"E&xclude files:"
	MDoNotScan,                      //"Do not &scan directories"
	MSelectFromList,                 //"Select files from &list"
	MDefOverwrite,                   //"Default overwrite action"
	MOverAsk,                        //"As&k"
	MOverOver,                       //"&Overwrite all"
	MOverSkip,                       //"Ski&p all"
	MOverResume,                     //"Resu&me all"
	//
	MListTitle,                      //"List of files to process"
	MListFooter,                     //"INS - toggle, F1 - help"

	MBtnOverwrite,                   //"&Overwrite"
	MBtnCopySkip,                    //"&Skip"
	MBtnCopyResume,                  //"&Resume"
	MBtnCopyCancel,                  //"&Cancel"
	MBtnCopyNew,                     //" New"
	MBtnCopyExisting,                //" Existing"
	MBtnRemember,                    //"Re&member choice"
	MBtnCopyNewer,                   //"&Newer"
	MOverNewer,                      //"All &newer"

	MNoVersion_txt1,                 //"Unknown version of FTP host file."
	MNoVersion_txt2,                 //"Password may be wrong on usage."

	MCanNotMoveFolder,               //"You can not rename or move hosts folders"

	MOldHostFormatTitle,             //"Host entry contains data in old format..."
	MOldHostFormat1,                 //"The host entry for site:"
	MOldHostFormat2,                 //"Contains data in old format. Older plugin versions may incorrec-"
	MOldHostFormat3,                 //"tly read host parameters. Are you sure to save in new format?"
	MOldHostConvert,                 //"&Overwrite"

	MServerType,                     //"S&erver type"
	MUtilsSelectTable,               //"Change c&haracter table..."
	MUtilsServerType,                //"Change ser&ver type..."
	MDecodeCommands,                 //"Decode co&mmands text"
	MResumeRestart,                  //"FAR: Site does not support resuming. Restarting from the beginning."

	MAddToSites,                     //"Add to saved s&ites"
	MRemoveFromites,                 //"Remove from saved s&ites"

	MErrorOpenFile,                  //"Error opening local file"
	MErrorPosition,                  //"Error setting file pointer"

	MFLTitle,                        //"Files list"
	MFLSaveTo,                       //"Sa&ve file list to:"
	MFLUrls,                         //"&Urls list"
	MFLTree,                         //"&Files tree"
	MFLGroups,                       //"&Directory groups"
	MFLPrefix,                       //"Add &prefix"
	MFLPass,                         //"Add p&assword and user"
	MFLQuote,                        //"&Quote urls"
	MFLSizes,                        //"Add files si&zes"
	MFLSizeBound,                    //"Size at &bound"
	MFLErrGetInfo,                   //"Error getting another panel info"
	MFLErrCReate,                    //"Error creating list file"
	MFLDoneTitle,                    //"File list created"
	MFLFile,                         //"The list of files saved to:"
	MFLDone,                         //"file successfully."
	MFAppendList,                    //"Ap&pend to list"

	MRejectTitle,                    //"Warning..."
	MRejectCanNot,                   //"FTP plugin can not close because of:"
	MRejectAsk1,                     //"Do you want to ignore it and close anyway"
	MRejectAsk2,                     //" or move current session to site list?"
	MRejectIgnore,                   //"&Ignore"
	MRejectSite,                     //"To s&ites"
	MRejectFail,                     //"Plugin failed to add to sitee list and will be closed"

	MQMenuTitle,                     //"Process queque"
	MQMenuItems,                     //"items"
	MQMenuFooter,                    //"F4 Edit, DEL Remove, INS Insert"

	MFileComplete,                   //"File already complete"
	MUrlItem,                        //"Url item"
	MQueueParam,                     //
	MCopyFrom,                       //"Copy &from:"
	MCopyTo,                         //"Copy &to:"
	MFileName,                       //"&File name:"
	MUDownlioad,                     //"&Download"
	MUHost,                          //"&Host"
	MUError,                         //"&Error"

	MQRestore,                       //"Re&store plugin state after processing"
	MQRemove,                        //"&Remove completed items"
	MQSave,                          //"Sa&ve as default"
	MQCanNotConnect,                 //Can not connect
	MQNotLocal,                      //"Local directory not set and can not be determinned automatically"
	MQCanNotChangeLocal,             //"Can not change local dir to [%s]: %s"
	MQNotFoundSource,                //"Can not find source file [%s]: %s"
	MQCanNotChangeRemote,            //"Can not change FTP dir: %s"
	MQCanNotFindRemote,              //"Can not find source file [%s]: %s"
	MQErrDowload,                    //"Can not download file: %s"
	MQErrUpload,                     //"Can not upload file: %s"
	MQItemCancelled,                 //"Url item cancelled. Do you want to connect to host?"

	MHostParams,                     //"&Hosts parameters..."
	MCloseConnection,                //"Clo&se connection and switch to hosts mode"
	MShowQueue,                      //"Show process &queque..."
	MAddQueue,                       //"Add to process &queque"

	MQDeleteItem,                    //"Delete current item or clear entire queue list?"
	MQSingleItem,                    //"Single i&tem"
	MQEntireList,                    //"Entire &list"
	MErrGetPanelInfo,                //"Error getting panel information"
	MErrNoSelection,                 //"Panel does not contains selection"
	MErrExpandList,                  //"Error getting list of files"
	MQErrUploadHosts,                //"Cannot add files to queue in hosts mode"
	MErrNotFiles,                    //"Cannot get files list from non-file panel"
	MQISingle,                       //"&1. Insert single item"
	MQIFTP,                          //"&2. Add selection from current FTP panel"
	MQIAnother,                      //"&3. Add selection from another files panel"
	MQAddTitle,                      //"Adding queue item"
	MQErrAdding,                     //"Error adding items"

	MVersionTitle,                   //"Version: "
	MTransferFiles,                  //"Transferring files to FAR..."

	MEHTitle,                        //"Extended host options"
	MDupFF,                          //"Dup FF in commands"
	MUndupFF,                        //"UnDup FF in PWD"
	MEHDecodeCmd,                    //"Decode &cmd line commands"
	MSendAllo,                       //"Send ALLO command before upload"
	MUseStartSpaces,              //"Ignore spaces at file name start"

	METitle,                         //"Extended options"
	MEShowProgress,                  //"Show view/edit progress"
	MEBackup,                        //"Use connection &backup"
	MESendCmd,                       //"Send command line commands"
	MEDontError,                     //"Do not show window on error"
	MEAskLoginAtFail,                //"Ask login at login fail"
	MEAutoAn,                        //"Auto anonymous"
	MECloseDots,                     //"C&lose connection on \"..\""
	MQuoteClipboardNames,            //"Quote file names placed to clipboard"
	MSetHiddenOnAbort,               //"Set hidden on download aborted"

	MAskDir1,                        //"Can not change directory."
	MAskDir2,                        //"It seems full directory specified."
	MAskDir3,                        //"Try to change to nearest place?"

	MDownloadOnServer,               //"Move inside server"

	mNoNe

};

#endif