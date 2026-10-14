# PREMATRON Mirage

Psychedelic dual delay. VST3, AU (macOS) and CLAP, built with JUCE 8.

## Build locally
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel

Plugins land in `build/Mirage_artefacts/Release/`. The first configure downloads JUCE and clap-juce-extensions.

## Build in the cloud
Push this folder to a GitHub repo. The workflow in `.github/workflows/build.yml` builds macOS (universal) and Windows and attaches the plugins as downloadable artifacts.

## Status
Complete first version: full delay engine plus the custom PREMATRON panel (resizable, vector drawn, logo embedded).
Compiled cleanly as VST3 and CLAP on Linux, and the processor passed headless audio tests (echo timing, ping-pong,
extreme-settings stability, freeze hold). Not yet built or heard on macOS or Windows, and AU has not been built.

## Before you release
- macOS: code sign and notarize with an Apple Developer account (AU needs a Mac build).
- JUCE is free below its revenue threshold; commercial releases above it need a paid JUCE license.
- Title font uses the system bold sans. To use a specific font, add the .ttf to Assets/ and load it through BinaryData.
