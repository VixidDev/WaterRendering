# Water Rendering

A project to render ocean waves in OpenGL using the Joint North Sea Wave Project (JONSWAP) wave spectra and Hasselmann directional spreading function, and synthesising the wave height field using Fast Fourier Transforms (FFTs) with compute shaders.

https://github.com/user-attachments/assets/c336ee78-29b4-46f9-841b-4f4a5f7387f9

### Results

The wave spectra and PBR lighting is computed in around 1.7ms (588fps) on the GPU at a 256x256 ocean resolution, with the lighting taking up around 0.8ms of this time. Timing was performed on the following hardware:
 - CPU: AMD Ryzen 5 7600
 - GPU: AMD Radeon RX 6700 XT

> There is the potential for many optimisations that were forgone in lieu of actually getting something realistic rendering. Some of which I may implement in the event I come back and visit this project.

### Appearance

There are many variables that can be played with to alter to appearance of the water patch and computation cost, such as the grid size, wave scales, ocean depth, wind fetch, wind speed, and the various PBR material variables.

### Usage

1. Clone the repository
2. Run premake5.exe with `$> premake5 vs2022`
3. Run the application through Visual Studio run configurations.
