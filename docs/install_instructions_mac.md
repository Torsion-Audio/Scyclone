# Scyclone Mac Installation Guide

## Installation Guide

- if you have not installed homebrew yet run ```/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"``` in the Terminal
- run ```brew install faressc/scyclone/onnxruntime@1.12.1``` to install v1.12.1 of the onnxruntime library with homebrew
- copy vst3 binary to your system vst3 folder: ```<HD>/Library/Audio/Plug-Ins/VST3```
- optionally you can also use the standalone version

**Own Build** <br />

|     vst3 plugin     | ```Scyclone/cmake-build/Scyclone_artefacts/Release/VST3/Scyclone.vst3``` |
|---------------------|-----------------------------------------------------------------------------------------------|
| onnxruntime library | ```Scyclone/modules/onnxruntime-1.12.1/lib/libonnxruntime.1.12.1.dylib```                                 |

