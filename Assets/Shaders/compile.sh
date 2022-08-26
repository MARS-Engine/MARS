#!/bin/zsh
for i in $(find . -type f -name "*.vert" -o -name "*.frag")
do
  glslc $i -o $i.spv
done