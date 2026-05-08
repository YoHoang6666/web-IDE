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
Source: "release*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\webIDE"; Filename: "{app}\webIDE.exe"
Name: "{commondesktop}\webIDE"; Filename: "{app}\webIDE.exe"

[Run]
Filename: "{app}\webIDE.exe"; Description: "Launch webIDE"; Flags: nowait postinstall skipifsilent
