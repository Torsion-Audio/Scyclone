# Scyclone Mac Installation Guide

## Installation Guide

- if you have not installed homebrew yet, run the following line in the Terminal and follow the instructions

```/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"```

- to install v1.12.1 of the onnxruntime library with homebrew run

 ```brew install faressc/scyclone/onnxruntime@1.12.1``` 
 
- copy vst3 binary to your system vst3 folder: ```/Library/Audio/Plug-Ins/VST3```
- optionally you can also use the standalone version

**Own Build** <br />

|     vst3 plugin     | ```Scyclone/cmake-build/Scyclone_artefacts/Release/VST3/Scyclone.vst3``` |
|---------------------|-----------------------------------------------------------------------------------------------|
| onnxruntime library (x86_64) | ```/usr/local/opt/onnxruntime@1.12.1/lib/libonnxruntime.1.12.1.dylib```                                 |
| onnxruntime library (arm64) | ```/opt/homebrew/opt/onnxruntime@1.12.1/lib/libonnxruntime.1.12.1.dylib```                                 |

