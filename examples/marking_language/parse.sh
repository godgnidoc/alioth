#!/bin/bash

HERE=$(dirname $(realpath $0))
ALIOTH_HOME=$(realpath $HERE/../..)

${ALIOTH_HOME}/build/x64-linux-release/alioth \
    compile \
    -g ${HERE}/manifest.grammar \
    ${HERE}/example.manifest