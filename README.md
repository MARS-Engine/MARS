# MARS
MARS (Modular Asynchronous Rendering System) is a multithreaded optimized c++ rendering engine.

MARS is a set of multiple modules that have as a goal help developers to easily create their high performance rendering engines.

Feel free to join MARS [Discord Server](https://discord.gg/VHEP99VShx) if you wish to help or talk

For a test sample, you can go [here](https://github.com/MARSngine/MARS-Samples)

## Main Modules Status

- Math - Partially complete
- Processing - Complete
- Input - Complete but needs an overhaul
- Rendering Thread - Complete but needs an overhaul
- Physics - In development

## Vulkan
Originally this project was ment to use Vulkan but due to the project being "early" in devlopment and being constantly changing the headches of Vulkan due to all it's necessary boilerplate have become too much, in the future when everthing becomes more stable OpenGL will likelly be removed in favor of Vulkan as a main renderer followed by support for DirectX

## Building
To build you need the base-devel, cmake, vulkan and sdl2

You also need to manually compile and install [Parallel](https://github.com/PedroSilva8/parallel)

For Windows use MinGW, [MSYS2](https://www.msys2.org/) should help a lot

## MARS Structure

MARS is separated into multiples folders, each folder acts like a module by being separated into its own namespace

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
├── resources   - This contains the main resource handler, which handles all resources of MARS
├── scenes      - This contains the basics to create a scene
└── time        - This contains the basic to update time delta
```
