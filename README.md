# 🌄 OpenGL 3D Scene — Real-Time Rendering

A real-time 3D scene renderer built in **C++ with OpenGL 4.1**, featuring shadow mapping, dynamic lighting, fog, animated objects, and a particle rain system. Developed as a **Computer Graphics** university project.

---

## ✨ Features

| Feature | Details |
|---------|---------|
| **Shadow Mapping** | PCF soft shadows via a 4096×4096 depth map FBO |
| **Directional Light** | Main sun light with configurable position |
| **Point Lights** | Two toggleable lantern lights with attenuation (toggle `L`) |
| **Fog** | Exponential fog with adjustable density and color (toggle `F`) |
| **Skydome** | Surrounding sky sphere rendered with stripped rotation matrix |
| **Windmill Animation** | Rotating windmill blades around a custom pivot point (toggle `R`) |
| **Rain Particle System** | 3000 dynamic raindrop line-segments with alpha blending (toggle `P`) |
| **Cinematic Camera** | Auto-orbiting camera mode (toggle `Z`) |
| **Polygon Modes** | Solid / Wireframe / Point rendering (`1` / `2` / `3`) |
| **Retina / HiDPI** | Framebuffer scaled to monitor DPI |

---

## 🗂️ Project Structure

```
PGProiect/
├── src/
│   ├── main.cpp              # Entry point, render loop, scene setup
│   ├── Camera.cpp            # FPS camera (move + mouse rotate)
│   ├── Mesh.cpp              # VAO/VBO/EBO setup and draw call
│   ├── Model3D.cpp           # OBJ loader, texture loader (stb_image)
│   └── Shader.cpp            # GLSL shader loader and compiler
├── include/
│   ├── Camera.hpp
│   ├── Mesh.hpp
│   ├── Model3D.hpp
│   ├── Shader.hpp
│   ├── stb_image.h           # Single-header image loader
│   └── tiny_obj_loader.h     # Single-header OBJ parser
├── shaders/
│   ├── shaderStart.vert/.frag  # Main PBR-style shader (lighting, fog, shadows)
│   ├── shaderShadow.vert/.frag # Depth pass shader
│   └── shaderRain.vert/.frag   # Rain particle shader
├── objects/
│   ├── scene.obj             # Main scene geometry
│   ├── sk.obj                # Skydome mesh
│   └── elicee.obj            # Windmill blades
└── README.md
```

---

## ⌨️ Controls

| Key / Input | Action |
|-------------|--------|
| `W A S D` / Arrow keys | Move camera |
| Mouse | Look around |
| `Q` / `E` | Yaw camera left / right |
| `R` | Toggle windmill rotation |
| `L` | Toggle point lights (lanterns) |
| `F` | Toggle fog |
| `P` | Toggle rain |
| `Z` | Toggle cinematic (auto-orbit) camera |
| `1` | Solid polygon mode |
| `2` | Wireframe mode |
| `3` | Point mode |
| `ESC` | Exit |

---

## 🏗️ Architecture

### Rendering Pipeline

```
1. Compute light-space matrix (orthographic projection from sun)
2. Depth pass  → render scene to shadow map FBO (4096×4096)
3. Main pass   → render skydome (depth ≤, no rotation)
              → render scene with shadows, lighting, fog
4. Rain pass   → update particle positions, draw line segments
```

### Lighting Model

The main shader implements a **Phong-based model** with:
- Directional light (sun) + secondary fill light
- Two point lights (lanterns) with quadratic attenuation
- PCF shadow sampling from the depth map
- Exponential fog applied in view space

### Key Classes

`Camera` — stores position, front, right, up vectors; computes `glm::lookAt` view matrix; supports WASD movement and pitch/yaw mouse rotation.

`Shader` — reads GLSL files from disk, compiles vertex + fragment shaders, links the program, reports compile/link errors.

`Mesh` — holds per-vertex `Position`, `Normal`, `TexCoords`; sets up VAO/VBO/EBO; binds textures by name uniform and issues a single `glDrawElements` call.

`Model3D` — parses `.obj` + `.mtl` via `tiny_obj_loader`; loads ambient/diffuse/specular textures via `stb_image`; manages GPU resource cleanup in destructor.

---

## 🛠️ Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [OpenGL](https://www.opengl.org/) | 4.1 Core | Graphics API |
| [GLFW](https://www.glfw.org/) | 3.x | Window + input |
| [GLEW](https://glew.sourceforge.net/) | 2.x | OpenGL extension loader (Windows/Linux) |
| [GLM](https://github.com/g-truc/glm) | 0.9.x | Math (vectors, matrices) |
| [stb_image](https://github.com/nothings/stb) | — | Single-header image loader |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) | — | Single-header OBJ parser |

---

## 🚀 Building

### Visual Studio (Windows)
1. Open `PGProiect.sln`
2. Ensure GLFW, GLEW, GLM are linked in project properties
3. Set build configuration to **Release x64**
4. Build → Run (`F5`)

### Manual (CMake)
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./PGProiect
```

> **Note:** The executable must be run from the project root so that relative paths to `shaders/` and `objects/` resolve correctly.

---

## 📚 Skills Demonstrated

- Real-time 3D rendering with **OpenGL 4.1 Core Profile**
- **Shadow mapping** with a custom FBO and depth texture
- **Phong lighting model** with multiple light sources and attenuation
- **Particle systems** (GPU-side dynamic VBO updates)
- **OBJ model loading** and texture management
- **Camera systems** — FPS and cinematic orbit modes
- **GLSL shader programming** (vertex + fragment)
- Object-oriented C++ engine architecture

---

## 👩‍💻 Author

**Oana** — [GitHub Profile](https://github.com/oanabye)
