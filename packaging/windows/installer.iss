[Setup]
AppName=webIDE
AppVersion=0.1.0
DefaultDirName={pf}\webIDE
DefaultGroupName=webIDE
OutputDir=dist
OutputBaseFilename=webIDE-setup
Compression=lzma
SolidCompression=yes

[Files]
Source: "build\Release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\webIDE"; Filename: "{app}\web-ide.exe"
Name: "{commondesktop}\webIDE"; Filename: "{app}\web-ide.exe"

[Run]
Filename: "{app}\web-ide.exe"; Description: "Launch webIDE"; Flags: nowait postinstall skipifsilent