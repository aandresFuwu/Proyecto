# Paint Básico en C++ con OpenGL, GLFW, GLAD y Dear ImGui

**Autor:** Andres Felipe Algarra Jimenez
**Codigo:** 6014444

Aplicación interactiva de dibujo 2D basada en una grilla lógica de **120x120** celdas renderizada en **OpenGL 3.3 Core Profile**, con panel de herramientas en **Dear ImGui**, historial de deshacer (`Ctrl + Z` / botón)

---

## Características principales

- **Lienzo en grilla de 80×80:** Cada celda se almacena como un color RGB en punto flotante (`[0.0f, 1.0f]`) y se renderiza en OpenGL mediante dos triángulos por celda (con soporte de lienzo continuo ajustando el `padding` en `updateGridVertices`).
- **Herramientas de dibujo y edición:**
  - **Pincel:** Pinta celdas de forma continua al hacer clic o arrastrar.
  - **Borrador:** Restaura celdas al color base del lienzo (`BG_COLOR`).
  - **Línea:** Algoritmo de Bresenham con parámetro de decisión (`drawLineBresenham`).
  - **Círculo:** Algoritmo de Bresenham con simetría en 8 octantes (`drawCircleBresenham` y `drawCircle8Points`).
  - **Rectángulo:** Traza los cuatro lados conectando esquinas mediante líneas de Bresenham (`drawRectangle`).
  - **Triángulo:** Calcula el ancho a partir del arrastre y traza los tres lados con líneas de Bresenham (`drawTriangle`).
  - **Balde de relleno:** Algoritmo `floodFill` iterativo con cola (`std::queue`) sobre celdas adyacentes del mismo color inicial.
- **Selector de color RGB:** Integrado con `ImGui::ColorPicker3` vinculado a `activeColor[3]`.
- **Historial de deshacer (`undo`):** Pila de estados (`std::stack<GridSnapshot>`) con límite de **50 pasos** (`MAX_UNDO_STEPS`) accesible desde el botón de la interfaz o con `Ctrl + Z`.
- **Exportación a imagen PPM:** Función `saveCanvasToPPM` que convierte la grilla RGB (`[0.0f, 1.0f]` a `[0, 255]`) y la guarda en formato de texto plano `.ppm` (`P3`) orientada de arriba hacia abajo.

---

## Estructura del proyecto

```text
├── main.cpp              # Lógica principal, algoritmos de dibujo, callbacks, ImGui y bucle OpenGL
├── CMakeLists.txt        # Configuración de compilación (C++17, GLFW, GLAD, ImGui)
├── shaderClass.h         # Clase auxiliar para cargar y compilar shaders
├── VAO.h                 # Encapsulamiento de Vertex Array Object
├── VBO.h                 # Encapsulamiento de Vertex Buffer Object
├── EBO.h                 # Encapsulamiento de Element Buffer Object
└── Shaders/
    ├── default.vert      # Vertex shader (posición y color por vértice)
    └── default.frag      # Fragment shader

## Arquitectura y flujo en `main.cpp`

1. **Estado y grilla (`PixelColor`, `pixelGrid`, `Tool`):** Define el tamaño de la grilla (`80×80`), el color activo (`activeColor`), el color base (`BG_COLOR`) y la herramienta seleccionada.
2. **Historial (`saveState` y `undo`):** Guarda copias completas de la matriz antes de modificarla y descarta los estados más antiguos al superar los 50 pasos.
3. **Mapeo de coordenadas (`screenToGrid`):** Convierte las coordenadas del cursor en la ventana (`800×800`) a índices de fila y columna, invirtiendo el eje `Y` (`height - mouseY`).
4. **Rasterización y relleno:** Implementa Bresenham para líneas, círculos, rectángulos y triángulos, además de `floodFill` con cola.
5. **Actualización de geometría (`updateGridVertices`):** Convierte `pixelGrid` a coordenadas normalizadas (`[-1.0f, 1.0f]`) y solo actualiza el `VBO` mediante `glBufferSubData` cuando `gridNeedsUpdate` es `true`.
6. **Entrada y bucle principal:** Valida `ImGui::GetIO().WantCaptureMouse` y `WantCaptureKeyboard` en los *callbacks* de GLFW para evitar pintar o disparar atajos al interactuar con Dear ImGui, manteniendo el orden de *frame* de ImGui antes de `glDrawArrays` y después antes de `glfwSwapBuffers`.

---

## Compilación y ejecución

### Requisitos
* Compilador compatible con **C++17** (MSVC, GCC o Clang).
* **CMake** (recomendado `CMAKE_CXX_STANDARD 17`).
* **GLFW**, **GLAD** y **Dear ImGui** (incluyendo `imgui_impl_glfw.cpp` e `imgui_impl_opengl3.cpp`).

### Compilar con CMake
```bash
cmake -B build -DCMAKE_CXX_STANDARD=17
cmake --build build --config Release
