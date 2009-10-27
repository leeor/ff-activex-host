#ifndef MyAppName
#define MyAppName "Firefox ActiveX Plugin"
#endif

#ifndef MyAppURL
#define MyAppURL "http://code.google.com/p/ff-activex-host/"
#endif

#ifndef xversion
#define xversion "r28"
#endif

#ifndef xbasepath
#define xbasepath "c:\src\ff-activex-host\ff-activex-host\"
#endif

[Setup]
AppId={{97F2985C-B74A-4672-960E-E3769AE5657A}}
AppName={#MyAppName}
AppVerName={#MyAppName} {#xversion}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename=ffactivex-setup
Compression=lzma
SolidCompression=yes
InternalCompressLevel=ultra
ArchitecturesInstallIn64BitMode=x64 ia64

#include "ffactivex.inc"

[Registry]
Root: HKLM32; Subkey: "SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes\application/x-itst-activex\clsid\*"; ValueType: string; ValueName: "*"; ValueData: true;
Root: HKLM32; Subkey: "SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes\application/x-itst-activex\progid\*"; ValueType: string; ValueName: "*"; ValueData: true;
Root: HKLM32; Subkey: "SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes\application/x-itst-activex\codeBaseUrl\*"; ValueType: string; ValueName: "*"; ValueData: true;

[Files]
Source: {#xbasepath}\Release\transver.exe; DestDir: {app}; DestName: npffax.dll; Flags: overwritereadonly restartreplace uninsrestartdelete

