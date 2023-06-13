#!/bin/bash

HERE=$(dirname $(realpath $0))
ALIOTH_HOME=$(realpath $HERE/../..)

${ALIOTH_HOME}/build/x64-linux-release/alioth \
    parse \
    -g ${HERE}/manifest.grammar \
    ${HERE}/example.manifest \
    -u \
    --form '$form' \
    --text '$text'