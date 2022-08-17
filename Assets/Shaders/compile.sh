#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

for d in $parent_path/*/ ; do
    for f in $d*.{vert,frag}; do
        glslc $f -o $f.spv
    done
done