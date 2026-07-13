#define MyAppName "MIXING COPILOT"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "JOEWONGAUDIO"
#define MyAppId "{{8D455B38-6B78-4AC8-A730-C7E0B81F83B2}"
#define BuiltVst3Dir "..\..\build-windows\MixingCopilot_artefacts\Release\VST3\MIXING COPILOT.vst3"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputBaseFilename=MIXING COPILOT Windows VST3 Installer
OutputDir=..\Dist\Windows
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName} VST3

[Files]
Source: "{#BuiltVst3Dir}\*"; DestDir: "{commoncf64}\VST3\MIXING COPILOT.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[InstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\MIXING COPILOT.vst3"

[Code]
function InitializeSetup(): Boolean;
begin
  if not DirExists(ExpandConstant('{#BuiltVst3Dir}')) then
  begin
    MsgBox('Windows VST3 build not found: ' + ExpandConstant('{#BuiltVst3Dir}') + #13#10 +
           'Build MIXING COPILOT on Windows first, then run this installer script again.',
           mbError, MB_OK);
    Result := False;
  end
  else
    Result := True;
end;
