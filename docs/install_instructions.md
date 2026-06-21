# Scyclone Installation Guide

## macOS

Download the macOS zip from [GitHub Releases](https://github.com/Torsion-Audio/Scyclone/releases) (or from a manual workflow run). Universal release builds support Apple Silicon and Intel; choose `arm64` or `universal` when running a manual build if needed. Minimum macOS version: **10.13**.

Extract the archive and copy the formats you need:

| Format | Destination |
|--------|-------------|
| Standalone | `/Applications/` |
| VST3 | `/Library/Audio/Plug-Ins/VST3/` |
| AU | `/Library/Audio/Plug-Ins/Components/` |

### Own build

If you build the plugins yourself with the `release` preset, binaries are under `Scyclone/build-release/Scyclone_artefacts/Release/`:

| Format | Path |
|--------|------|
| Standalone | `Standalone/Scyclone.app` |
| VST3 | `VST3/Scyclone.vst3` |
| AU | `AU/Scyclone.component` |

Copy those bundles into the folders above.

## Windows

Download the Windows zip from [GitHub Releases](https://github.com/Torsion-Audio/Scyclone/releases), or use the installer included in the archive.

### VST3 (release or own build)

Copy `Scyclone.vst3` to:

```
C:\Program Files\Common Files\VST3\
```

| Source | Path |
|--------|------|
| Downloaded release | `path\to\extracted\Scyclone.vst3` |
| Own build | `Scyclone\build-release\Scyclone_artefacts\Release\VST3\Scyclone.vst3` |

The standalone `.exe` and `Scyclone Installer.exe` can be run from any location after extraction.
