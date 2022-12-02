# MVRE
MVRE (multithreaded Vulkan Rendering Engine) is a multithreaded optimized c++ rendering engine.

Feel free to join MVRE [Discord Server](https://discord.gg/VHEP99VShx) if you wish to help or talk

For a test sample, you can go [here](https://github.com/MVREngine/MVRE-Samples)

## MVRE Structure

MVRE is separated into multiples folders, each folder acts like a module by being separated into its own namespace

```bash
.
├── algo        - This contains algorithms to make life easier
├── components  - This contains mandatory components
├── debug       - Debug helpers
├── engine      - This contains the main processing engine
│   └── layers  - This contains the main processing layers
├── executioner - The executioner is a thread dedicated to handling GPU calls
├── graphics    - This contains the graphic classes, exp. shader, texture, etc.
│   └── backend - This contains the main backends to the graphical interfaces
├── loaders     - This contains basic loaders, exp. waveformat
├── math        - This contains the basic math classes, exp. vector 2-4, matrix4
├── resources   - This contains the main resource handler, which handles all resources of MVRE
├── scenes      - This contains the basics to create a scene
└── time        - This contains the basic to update time delta
```