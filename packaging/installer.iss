; Inno Setup script for MAPS0 Desktop Client (Windows)
; Build prerequisite:
;   1. cmake --build build --config Release
;   2. cmake --install build --prefix dist
;      (this stages GUI_MAPS.exe + Qt DLLs into dist/bin via windeployqt)
;   3. iscc packaging\installer.iss
;
; Output: packaging\Output\MAPS0DesktopClient-Setup-1.0.0.exe

#define MyAppName        "MAPS0 Desktop Client"
#define MyAppShortName   "MAPS0DesktopClient"
#define MyAppVersion     "1.0.0"
#define MyAppPublisher   "MAPS0"
#define MyAppExeName     "GUI_MAPS.exe"
#define MyStageDir       "..\dist\bin"

[Setup]
AppId={{8F4C2A9E-5B3D-4F1A-9C8E-2A1B7D6E3F4C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppShortName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename={#MyAppShortName}-Setup-{#MyAppVersion}
Compression=lzma2/ultra
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"

[Files]
; Pull everything windeployqt staged into dist/bin
Source: "{#MyStageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}";    Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall";       Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
