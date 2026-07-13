MIXING COPILOT Windows packaging

This folder contains the Inno Setup installer script for the Windows VST3 build.

Expected Windows build output:
  build-windows\MixingCopilot_artefacts\Release\VST3\MIXING COPILOT.vst3

Installer output after compiling the script on Windows:
  Packaging\Dist\Windows\MIXING COPILOT Windows VST3 Installer.exe

Important:
  The macOS .vst3 bundle is a Mach-O binary and cannot run on Windows.
  Build the plugin on Windows first, then open MIXING_COPILOT_Windows_InnoSetup.iss with Inno Setup and compile it.
