#!/bin/bash

cd ${AAC_SDK_HOME}/extensions/extras

# Unpack extensions
unzip -q local-voice-control-coassistant-patched-2.2.zip
mv local-voice-control lvc
rm local-voice-control-coassistant-patched-2.2.zip

unzip -q co-assistant-patched-2.2.zip
mv co-assistant coassistant
rm co-assistant-patched-2.2.zip

unzip -q amazonlite-coassistant-patched-2.2.zip
mv amazonlite amazonlite-coassistant
rm amazonlite-coassistant-patched-2.2.zip

rm -rf __MACOSX