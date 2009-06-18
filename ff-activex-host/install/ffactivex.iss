#define MyAppName "Firefox ActiveX Plugin"
#define MyAppURL "http://code.google.com/p/ff-activex-host/"
#define xversion "r28"
#define xbasepath "y:\wwwroot\projects\prodown\download\0.00.0000"

[Setup]
AppId={{97F2985C-B74A-4672-960E-E3769AE5657A}}
AppName={#MyAppName}
AppVerName={#MyAppName} {#version}
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
