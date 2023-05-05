# Scyclone Mac Installation Guide

## Installation Guide

Open the dmg from the latest release and copy the plugins or standalone you need in the respective system folders. Make sure you download the correct release for you system: *Scyclone-v.0.0.3-osx-arm64.dmg* for macs with the new apple silicon processors (Apple M1 and up) or *Scyclone-v.0.0.3-osx-x64.dmg* for older macs with intel processors. The minimum macOS version supported is 10.13.

|     Standalone    | ```/Applications/``` |
|---------------------|-----------------------------------------------------------------------------------------------|
|     VST3 plugin     | ```/Library/Audio/Plug-Ins/VST3/``` |
|     AU plugin     | ```/Library/Audio/Plug-Ins/Components/``` |

<br>

**Own Build** <br />

If you build the plugins yourself, you can find the binaries in the following folders:


|     Standalone    | ```Scyclone/cmake-build/Scyclone_artefacts/Release/Standalone/Scyclone``` |
|---------------------|-----------------------------------------------------------------------------------------------|
|     VST3 plugin     | ```Scyclone/cmake-build/Scyclone_artefacts/Release/VST3/Scyclone.vst3``` |
|     AU plugin     | ```Scyclone/cmake-build/Scyclone_artefacts/Release/VST3/Scyclone.component``` |

Copy those files in the folders.