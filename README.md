# Warning
Work is currently in progress in the [static reflection branch](https://github.com/MARS-Engine/MARS/tree/static_reflection) while the engine is rewritten to adopt C++ 26

# MARS
MARS (Modular Asynchronous Rendering System) is a multithreaded optimized c++ rendering engine.

MARS is a set of multiple modules that have as a goal help developers to easily create their high performance rendering engines.

Feel free to join MARS [Discord Server](https://discord.gg/VHEP99VShx) if you wish to help or talk

For a test sample, you can go [here](https://github.com/MARS-Engine/MARS-Samples)

## Main Modules Status

- Math - Partially complete
- Object Processing - Complete
- Input - Complete but needs an overhaul
- Rendering Thread - Complete but needs an overhaul
- Physics - In development

## MARS Layer System
For some insight into how the MARS Layer System works you can [visit this repository](https://github.com/MARS-Engine/MARS-Object-Processing)

## OpenGL/DirectX/Metal
OpenGL has started to become a bigger pain then excepted (due to multithreading) has the project is growing, therefor Vulkan is coming back as the main backend, OpenGL will probably never comeback, I have a idea of wrapping all function so that they are sent as commands to the graphics m_thread but honastly it's too much work, maybe in the future.

In regard to DirectX and Metal both should be added in the future as separate modules

## Building
To build you need the base-devel, cmake, vulkan and sdl2

For Windows use MinGW, [MSYS2](https://www.msys2.org/) should help a lot

## Debugging
When debugging Vulkan it's highly recommended to use LLDB

## MARS Structure

MARS is separated into multiples folders, each folder acts like a module by being separated into its own namespace

```bash
.
├── algo        - This contains algorithms to make life easier
├── components  - This contains mandatory components
├── debug       - Debug helpers
├── engine      - This contains the main processing engine
│   └── layers  - This contains the main processing layers
├── executioner - The executioner is a m_thread dedicated to handling GPU calls
├── graphics    - This contains the graphic classes, exp. shader, texture, etc.
│   └── backend - This contains the main backends to the graphical interfaces
├── loaders     - This contains basic loaders, exp. waveformat
├── math        - This contains the basic math classes, exp. vector 2-4, matrix4
├── resources   - This contains the main resource m_handler, which handles all resources of MARS
├── scenes      - This contains the basics to create a scene
└── time        - This contains the basic to update time delta
```
