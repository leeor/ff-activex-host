[Files]
Source: {#xbasepath}\Release\npffax.dll; DestDir: {app}; DestName: npffax.dll; Flags: overwritereadonly restartreplace uninsrestartdelete

[Registry]
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex; Flags: uninsdeletekey
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex; ValueType: string; ValueName: Description; ValueData: {#MyAppName} {#xversion}
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex; ValueType: string; ValueName: ProductName; ValueData: {#MyAppName} {#xversion}
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex; ValueType: string; ValueName: Path; ValueData: {app}\npffax.dll
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex; ValueType: string; ValueName: Version; ValueData: {#xversion}
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes; ValueType: string; ValueName: Dummy; ValueData: {#xversion}
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes\application/x-itst-activex
Root: HKLM32; Subkey: SOFTWARE\MozillaPlugins\@itstructures.com/ffactivex\MimeTypes\application/x-itst-activex; ValueType: string; ValueName: "Dummy"; ValueData: "{#xversion}"