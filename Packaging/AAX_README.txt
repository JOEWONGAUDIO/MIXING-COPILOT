MIXING COPILOT AAX build note

Pro Tools AAX builds require the official Avid AAX SDK.
The AAX SDK is not included in this repository and is not currently present on this machine.

The CMake project is prepared for AAX:
  third_party/cmake-3.30.9-macos-universal/CMake.app/Contents/bin/cmake -S . -B build-aax -DJUCE_PATH=third_party/JUCE -DAAX_SDK_PATH=/path/to/AAX_SDK
  third_party/cmake-3.30.9-macos-universal/CMake.app/Contents/bin/cmake --build build-aax --config Release

Expected AAX output after a successful SDK-backed build:
  build-aax/MixingCopilot_artefacts/AAX/MIXING COPILOT.aaxplugin

Standard macOS AAX install path:
  /Library/Application Support/Avid/Audio/Plug-Ins/MIXING COPILOT.aaxplugin

After the AAX plugin is built, add it to the pkg staging folder under:
  Packaging/Dist/staging/macOS/Library/Application Support/Avid/Audio/Plug-Ins/

Then rebuild the pkg so VST3, AU, and AAX are all included.
