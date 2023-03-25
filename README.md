# <img style="float: left;" src="assets/pictures/logo.png" width="40" /> &nbsp; SCYCLONE
![interface](assets/pictures/interface.png)

**Scyclone** is an audio plugin that utilizes **neural timbre transfer** technology to offer a new approach to audio production. This plugin builds upon [RAVE's](https://github.com/acids-ircam/RAVE) methodology, a realtime audio variational auto encoder, facilitating neural timbre transfer in both single and couple inference mode. <br /><br />
This enables a new artificial layering technique to be applied on the incoming signal in creating richer drum pallets, fuller atmospheres or simply transferring the timbre of the raw signal to another sound pallet. To further control the behaviour and production of the neural networks, we have internally equipped the plugin with signal processings modules allowing the user to shape, control and embellish the  source and target timbres.

## Overview
![signal_flow](assets/pictures/signal_flow_control.png)



**Signal flow**: <br />

Scyclone provides an intuitive signal flow, empowering the user to seamlessly influence the neural audio synthesis of the models with pre-processing effects:

- Transient Controller: Shape the attack and sustain of the audio and tailoring it to your preferences
- Low-/High-Cut Filter: Refine the frequency range of your input audio for a more focused sound
 
After the synthesis, you can further enrich the audio with these additional options:
 
- Grain Delay: Add depth and texture to each model's output by applying a granular delay
- Blend: Crossfade between the outputs of the models, enabling you to create a harmonious mix
- Post Compressor: Ensure a consistent sound by applying a group compressor to both models, blending them harmoniously

**Trained models**:
- Funk Drums: This model is inspired by the captivating sounds of vintage drum-breaks, trained on a diverse four hours dataset
- Djembe: Carefully trained on five hours of dataset. Three hours of djembe solo performances and two hours of one-shot recordings

## Installation
The plugin uses the onnxruntime libary to inference our neural networks. Unfortnuatly at the moment the library is is included as a shared library, so it is necessary to put the shared library next to the exectutable or in your system libaray path. 

Detailed instructions can be found here:
- [Windows Guide](docs/install_instructions_windows.md).
- [Mac Guide](docs/install_instructions_mac.md).

## Build instruction
Build with CMake
```bash
# clone the repository
git clone https://github.com/Torsion-Audio/Scyclone
cd Scyclone/

# initialize and set up submodules
git submodule update --init --recursive

# build plugin
cmake . -B cmake-build
cmake --build cmake-build --config Release

# build should be here: /cmake-build/Scyclone_artefacts/Release/
```

**Note:** If you use apple silicon you need to download the prebuild onnxruntime library (version 1.12.1) manually. The CMake script downloads the x64 version automatically, if no library is found in the modules path. Therefore follow the steps below before building the plugin with CMake.

**Note:** CMake should automatically download the prebuild onnxruntime library (version 1.12.1). **If the script fails**, download the library manually:

- Download [onnx v1.12.1](https://github.com/microsoft/onnxruntime/releases/tag/v1.12.1) (Select your required prebuild)
- Extract the file and rename to the folder ```onnxruntime-1.12.1```
- Copy to folder to ```path/to/Scyclone/modules/```
- Now you sould have the following file structes ```Scyclone/modules/onnxruntime-1.12.1/include```

## Credits

- RAVE Paper - [RAVE: A variational autoencoder for fast and high-quality neural audio synthesis](https://arxiv.org/abs/2111.05011)
- RAVE Scripts - [RAVE Github Repository](https://github.com/acids-ircam/RAVE)
- RNBO Tutorial - [JUCE & RNBO C++ Export](https://kengo.dev/posts/jr-granular)

## License
This project is subject to multiple licenses. The primary license for the entire project is the GNU General Public License version 3 (GPLv3), which is the most restrictive of all the licenses applied herein.
 - The Granular Delay module located at ```modules/RnboExport/``` is licensed under the GPLv3.
 - All pretrained onnx models located at ```assets/models/``` are licensed under the Creative Commons Attribution-NonCommercial 4.0 International License 
 - All other code within this project is licensed under the MIT License.
