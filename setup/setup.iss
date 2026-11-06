; Inno Setup template. Заполняется sed'ом в Dockerfile.win.
; Плейсхолдеры: @APP_NAME_RU@ @APP_NAME_EN@ @APP_NAME_BIN@ @ICON_NAME@
;              @DIST_KIT_NAME@ @APP_VERSION@

#define MyAppName       "@APP_NAME_RU@"
#define MyAppVersion    "@APP_VERSION@"
#define MyAppPublisher  "ACS"
#define MyAppExeName    "@APP_NAME_BIN@.exe"
#define MyDistDir       "@DIST_KIT_NAME@"
#define MyIconFile      "@ICON_NAME@.ico"

[Setup]
AppId={{8F2C7B1A-3D4E-4F6A-9B0C-1D2E3F4A5B6D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename=@APP_NAME_EN@-{#MyAppVersion}-setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; иконка самого установщика (Setup.exe + заголовок мастера)
SetupIconFile={#MyIconFile}
; иконка в Add/Remove Programs — из установленного bundle
UninstallDisplayIcon={app}\{#MyIconFile}
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; сам bundle (включая modbus.ico, который мы туда положили в Dockerfile)
Source: "..\deploy\{#MyDistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; ярлык в меню «Пуск»
Name: "{group}\{#MyAppName}";        Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyIconFile}"; IconIndex: 0
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
; ярлык на рабочем столе — та же иконка
Name: "{autodesktop}\{#MyAppName}";  Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyIconFile}"; IconIndex: 0; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Dirs]
; Добавляем запись для папки установки, чтобы изменить её права
Name: "{app}"; Permissions: users-modify
Name: "{app}\log"; Permissions: users-modify