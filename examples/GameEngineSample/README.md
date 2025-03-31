
# ngengine (New Generation Engine)

ngengine is a modern game engine focused on advanced rendering techniques and deep exploration of the Vulkan API. Originally started as a hobby project to study the Vulkan API, ngengine has evolved into a comprehensive tool for studying graphics programming and implementing cutting-edge rendering methods.

## Features

### Completed
- **Bindless design**: Full bindless dynamic design based on self growing buffers.
- **Visibility Buffer**: Efficient management of visible objects to optimize rendering.
- **Shadow Maps**: Support for shadow mapping to achieve realistic lighting.
- **Vulkan 1.1-1.3 Support**: Full compatibility with Vulkan API versions 1.1 through 1.3.
- **Computation Support**: Leverage the power of GPUs for general-purpose computing tasks.
- **Render Graph**: A flexible render graph system for managing rendering pipelines.
- **Virtual Filesystem**: Abstraction layer for handling file operations within the engine.
- **Immediate Mode Debug Draw**: Real-time debugging with 2D and 3D support.
- **GPU Frustum Culling**: Efficient culling of objects outside the camera’s view using the GPU, applicable to all virtual cameras, including shadow casters.
- **Multithreaded resource uploading**: Simplified design of parallel resource loader running in COPY queue and in multiple threads to minimize possible delays.

### In Progress
- **Async Compute**: Support for asynchronous computing to optimize rendering performance.

### Future Plans
- **Rendering Transparency with Visibility Buffer**: Enhanced transparency handling integrated into the visibility buffer.
- **DVM (Decoupled Visibility Multisampling)**: Advanced multisampling technique to improve visual quality.
- **Ray-Traced Shadows**: Implementation of ray tracing for highly realistic shadows.
- **Triangle Culling**: Modern approaches to efficiently reject unnecessary triangles.
- **Shader Hot Reload**: Real-time shader reloading to streamline the development process.
- **Reverse-Z**: Enhanced depth precision through reverse-Z technique.
- **Reflection-Based Component Editor**: A user-friendly editor for modifying component properties based on reflection.
- **Virtual Tuning Variables**: Editable variables in the editor that can be used as parameters in the render graph, allowing dynamic adjustments.

## About ngengine

ngengine was born out of a desire to learn the Vulkan API but has since grown into a project that delves into advanced graphics programming techniques. With a focus on cutting-edge rendering methods, ngengine's current iteration aims to implement a full-fledged visibility buffer (including transparency), experiment with multithreaded rendering, asynchronous compute, and explore modern techniques for triangle rejection.

Stay tuned as ngengine continues to evolve, incorporating the latest advancements in graphics technology and pushing the boundaries of what's possible in real-time rendering.

## Getting Started

To get started with ngengine, clone the repository and follow the setup instructions provided in the [INSTALL.md](./INSTALL.md) file.

## Contributing

Contributions are welcome! Please see our [CONTRIBUTING.md](./CONTRIBUTING.md) for more details on how to contribute to the project.

## License

ngengine is licensed under the MIT License. See the [LICENSE](./LICENSE) file for more information.

