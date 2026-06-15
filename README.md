# <img style="float: left;" src="assets/pictures/logo.png" width="40" /> &nbsp; SCYCLONE
[![Build & Test](https://github.com/Torsion-Audio/Scyclone/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/Torsion-Audio/Scyclone/actions/workflows/build-and-test.yml) [![Sanitizers](https://github.com/Torsion-Audio/Scyclone/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/Torsion-Audio/Scyclone/actions/workflows/sanitizers.yml)
![interface](assets/pictures/interface.png)

**Scyclone** is an audio plugin that utilizes **neural timbre transfer** technology to offer a new approach to audio production. The plugin builds upon [RAVE](https://github.com/acids-ircam/RAVE) methodology, a realtime audio variational auto encoder, facilitating neural timbre transfer in both single and couple inference mode. <br /><br />
This enables a new artificial layering technique to be applied on the incoming signal in creating richer drum pallets, fuller atmospheres or simply transferring the timbre of the raw signal to another sound pallet. To further control the behaviour and production of the neural networks, we have internally equipped the plugin with signal processings modules allowing the user to shape, control and embellish the source and target timbres in a distinct manner.

## Overview
![signal_flow](assets/svg/signal_flow_control.svg)



**Signal flow**: <br />

Scyclone offers an intuitive signal flow allowing for a seamless influence over inference and sound synthesis. The pre-processing modules are:

- Transient Controller: Shaping the attack and sustain of the singal
- Low-/High-Cut Filter: Refining the frequency range of input audio favouring a consistent sound
 
Additional in-built postprocessing modules permit for further manipulation and formation of the timbre transferred signal. The post-processing modules are:
 
- Grain Delay: Adding depth and texture
- Blend: Crossfades between the outputs of the models and obtain a harmonious mix
- Post-Compressor: Controls the dynamics and glues the outputs together

**Trained models**:<br />

We have provided two pre-trained models (presets) accessible under **assets/models** directory.

- **Funk Drums**: Trained on four hours of captivating vintage drum-breaks
- **Djembe**: Trained on five hours of carefully compiled Djembe dataset (Three hours of Djembe solo performances and two hours of one-shot recordings)

## Installation

Detailed instructions can be found in the [Installation Guide](docs/install_instructions.md).

## Build instruction

Build with CMake presets ([`CMakePresets.json`](CMakePresets.json)):

```bash
# clone the repository
git clone https://github.com/Torsion-Audio/Scyclone
cd Scyclone/

# initialize and set up submodules
git submodule update --init --recursive

# Tests and IDE (Debug) — also generates compile_commands.json for clangd
cmake --preset default
cmake --build --preset test
ctest --test-dir build -L default --output-on-failure

# Plugin (Release) — required on Windows for VST3 / Standalone
cmake --preset release
cmake --build --preset release
```

See [test/README.md](test/README.md) for the full test layout, sanitizer presets, and calibration probes.

**CI:** Pull requests to `develop` require [Build & Test](.github/workflows/build-and-test.yml) and [Sanitizers](.github/workflows/sanitizers.yml) (Linux/macOS ASan+UBSan, Linux/macOS TSan). Pushes to `develop` run the same validation (no downloadable artifacts). To release signed builds, push a version tag — see [Release process](docs/maintainer/release.md).

**Notes:**
- The onnx library is now linked statically. No more need to download the onnx library via homebrew or via the github repository. macOS distribution builds are code-signed with Developer ID and notarized in CI.
- On Windows, **Release** is required for the plugin; the **`default`** preset (Debug) is for tests and IDE tooling.
- On macOS, set `CMAKE_OSX_ARCHITECTURES` (`arm64` or `x86_64`) in a local, gitignored [`CMakeUserPresets.json`](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html#additional-vendors) if needed.
- The AU plugin has not been tested with Logic yet. Logic support will come in futher updates.

## References

- RAVE Paper - [RAVE: A variational autoencoder for fast and high-quality neural audio synthesis](https://arxiv.org/abs/2111.05011)
- RAVE Scripts - [RAVE Github Repository](https://github.com/acids-ircam/RAVE)
- RNBO Tutorial - [JUCE & RNBO C++ Export](https://kengo.dev/posts/jr-granular)

## Licenses
This project is subject to multiple licenses. The primary license for the entire project is the GNU General Public License version 3 (GPLv3), which is the most restrictive of all the licenses applied herein.
 - The Granular Delay module located at ```modules/RnboExport/``` is licensed under the [GPLv3](https://support.cycling74.com/hc/en-us/articles/10730637742483-RNBO-Export-Licensing-FAQ)
 - All pretrained onnx models located at ```assets/models/``` are licensed under the [Creative Commons Attribution-NonCommercial 4.0 International License](https://github.com/acids-ircam/RAVE/blob/master/LICENSE) 
 - libsamplerate is licensed under BSD-2-Clause.
 - All other code within this project is licensed under the MIT License.
