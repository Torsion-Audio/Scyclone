# Scyclone Windows Installation Guide

**Important:** If you are using the plugin inside a DAW, the shared libaray has to be put next to the DAW executable or to your system libary folder! Here is a guide how to:

## Overview important path

**Downloaded Release**

|     vst3 plugin     | ```path/to/extracted/Scyclone.vst3``` |
|---------------------|-----------------------------------------------------------------------------------------------|
| onnxruntime library | ```path/to/extracted/onnxruntime.dll```                                 |

**Own Build** <br />

|     vst3 plugin     | ```Scyclone/cmake-build/Scyclone_artefacts/Release/VST3/VAESynth.vst3/Contents/x86_64-win/Scyclone.vst3/``` |
|---------------------|-----------------------------------------------------------------------------------------------|
| onnxruntime library | ```Scyclone/modules/onnxruntime-1.12.1/lib/onnxruntime.dll```                                 |


## Installation Guide

- copy vst3 binary to your system vst3 folder: ```C:\Program Files\Common Files\VST3```
- copy the onnxruntime library **either** next to your DAW executable (recommended) **or** in your system folder:  <br />

Option DAW:
- copy the onnxruntime next to your DAW executable
- e.g. ```C:\ProgramData\Ableton\Live 11 Suite\Program``` or ```C:\Program Files\REAPER (x64)```

Option System Folder:
- copy the onnxruntime library to ```C:\Windows\System32\```
- there is probably already an existing ```onnxruntime.dll```
- if you cannot replace it, follow this [guide](https://www.makeuseof.com/tag/what-is-trustedinstaller-and-why-does-it-keep-me-from-renaming-files/)




