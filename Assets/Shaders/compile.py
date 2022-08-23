import os, glob, subprocess
exts = ('*.frag', '*.vert')
files = [f for ext in exts
         for f in glob.glob(os.path.join(".", '**', ext), recursive=True)]

for f in files:
    print(subprocess.run(args=["glslc", f, "-o", f + ".spv"], stdout=subprocess.PIPE).stdout)
