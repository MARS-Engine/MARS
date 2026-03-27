# MARS

MARS (Modular Asynchronous Rendering System) is a multithreaded, high-performance C++ rendering engine built as a collection of modules, designed to help developers create their own rendering pipelines with minimal friction.

## 🚧 Status

This engine is using C++ static reflection via [Bloomberg's Clang fork](https://github.com/bloomberg/clang-p2996), which implements most of the current reflection proposals. GCC has also been tried on Linux, although it is not the main compiler used nor always tested.

Because this is a WIP branch, some parts are either:
- Missing documentation
- In an experimental state
- Not yet integrated

For working examples and reflection-driven systems with documentation, see the [documentation](documentation/) folder.

- **Graphics** 🌱 - Features and optimizations constantly being made.
  - **Raytracing** ✔️
  - **DirectX** ✔️ - Usable, but subject to changes and new features.
  - **Vulkan** ✔️ - Usable, but subject to changes and new features.
- **ECS** ✔️
- **Event System** 🔒

### Legend
| Symbol | Meaning                             |
|--------|-------------------------------------|
| 🌱     | Currently working on                |
| ✔️     | Complete                            |
| 🔒     | Finished, no major reworks expected |

## Platform Support

> **Note:** Support is not guaranteed on the main branch as automated tests are not set up yet. This should change in the near future.

| Platform   | Supported | Notes                                                                                   |
|------------|-----------|-----------------------------------------------------------------------------------------|
| Windows 11 | ✅        |                                                                                         |
| Linux      | ✅        |                                                                                         |
| macOS      | ❌        | Could probably be supported, but there are no current plans due to lack of interest.    |

## Dependencies

Most dependencies are fetched automatically by CMake.

| Dependency | Fetched Automatically | Notes |
|---|---|---|
| [Vulkan](https://www.lunarg.com/vulkan-sdk/) | No | Must be installed manually |
| [SDL3](https://github.com/libsdl-org/SDL) | Yes | |
| [DirectX Shader Compiler (DXC)](https://github.com/microsoft/DirectXShaderCompiler) | Yes | |
| [fastgltf](https://github.com/spnda/fastgltf) | Yes | glTF 2.0 loader |
| [Dear ImGui](https://github.com/ocornut/imgui) | Yes | |
| [stb](https://github.com/nothings/stb) | Yes | |
| [Windows PIX Event Runtime](https://devblogs.microsoft.com/pix/) | Yes | Windows only |
| Direct3D 12 | No | Windows only, provided by the OS |
