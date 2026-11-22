#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <queue> //colas para flood fill
#include <cmath> //funciones matematicas 
#include <stack> // undo 
#include <array> // std::array para GridSnapshot

#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// confiuracion de pixeles independientes 
const int GRID_COLS = 80; // Número de columnas (eje X)
const int GRID_ROWS = 80; // Número de filas (eje Y)

struct PixelColor {
    float r, g, b;
};

using GridSnapshot = std::array<std::array<PixelColor, GRID_COLS>, GRID_ROWS>;
std::stack<GridSnapshot> undoStack;
const size_t MAX_UNDO_STEPS = 50; // Límite de memoria  

PixelColor pixelGrid[GRID_ROWS][GRID_COLS];

// lista de herramientas usables 
enum Tool { PINCEL, BORRADOR, LINEA, CIRCULO, RECTANGULO, RELLENO };
Tool currentTool = PINCEL;

const PixelColor BG_COLOR = { 0.2f, 0.2f, 0.2f }; // Color base del lienzo
PixelColor currentColor = { 1.0f, 0.0f, 0.0f }; // Color activo por defecto

int startX = -1, startY = -1; //posicion inicial de click por raton 
bool isDrawing = false;
bool gridNeedsUpdate = true;

// FUNCIONES AUXILIARES Y DE LÓGICA

void saveState() {
    GridSnapshot snapshot;
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            snapshot[r][c] = pixelGrid[r][c];
        }
    }

    if (undoStack.size() >= MAX_UNDO_STEPS) {
        // Reconstruye la pila sin el elemento más antiguo para respetar el límite
        std::stack<GridSnapshot> temp;
        std::vector<GridSnapshot> items;
        while (!undoStack.empty()) {
            items.push_back(undoStack.top());
            undoStack.pop();
        }
        // items.back() es el más viejo; lo omitimos
        for (size_t i = items.size() - 1; i > 0; --i) {
            temp.push(items[i - 1]);
        }
        undoStack = std::move(temp);
    }

    undoStack.push(snapshot);
}

void undo() {
    if (undoStack.empty()) {
        std::cout << "No hay mas acciones para deshacer\n";
        return;
    }

    GridSnapshot previous = undoStack.top();
    undoStack.pop();

    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            pixelGrid[r][c] = previous[r][c];
        }
    }

    gridNeedsUpdate = true;
    std::cout << "Accion deshecha (Ctrl + Z)\n";
}

void setPixelColor(int col, int row, float r, float g, float b) {
    if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
        pixelGrid[row][col] = { r, g, b };
        gridNeedsUpdate = true;
    }
}

void screenToGrid(GLFWwindow* window, double mouseX, double mouseY, int& gridX, int& gridY) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    gridX = static_cast<int>((mouseX / width) * GRID_COLS);
    gridY = static_cast<int>(((height - mouseY) / height) * GRID_ROWS); // Inversión de eje Y

    if (gridX < 0) gridX = 0;
    if (gridX >= GRID_COLS) gridX = GRID_COLS - 1;
    if (gridY < 0) gridY = 0;
    if (gridY >= GRID_ROWS) gridY = GRID_ROWS - 1;
}

// Algoritmo de Bresenham para Líneas con Parámetro de Decisión Pk
void drawLineBresenham(int x0, int y0, int x1, int y1, float r, float g, float b) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    if (dx >= dy) {
        int pk = 2 * dy - dx;
        int x = x0, y = y0;

        for (int i = 0; i <= dx; i++) {
            setPixelColor(x, y, r, g, b);
            x += sx;
            if (pk >= 0) {
                y += sy;
                pk += 2 * (dy - dx);
            }
            else {
                pk += 2 * dy;
            }
        }
    }
    else {
        int pk = 2 * dx - dy;
        int x = x0, y = y0;

        for (int i = 0; i <= dy; i++) {
            setPixelColor(x, y, r, g, b);
            y += sy;
            if (pk >= 0) {
                x += sx;
                pk += 2 * (dx - dy);
            }
            else {
                pk += 2 * dx;
            }
        }
    }
}

void drawCircle8Points(int xc, int yc, int x, int y, float r, float g, float b) {
    setPixelColor(xc + x, yc + y, r, g, b);
    setPixelColor(xc - x, yc + y, r, g, b);
    setPixelColor(xc + x, yc - y, r, g, b);
    setPixelColor(xc - x, yc - y, r, g, b);
    setPixelColor(xc + y, yc + x, r, g, b);
    setPixelColor(xc - y, yc + x, r, g, b);
    setPixelColor(xc + y, yc - x, r, g, b);
    setPixelColor(xc - y, yc - x, r, g, b);
}

void drawCircleBresenham(int xc, int yc, int r_radius, float r, float g, float b) {
    int x = 0;
    int y = r_radius;
    int d = 3 - 2 * r_radius;

    drawCircle8Points(xc, yc, x, y, r, g, b);

    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else {
            d = d + 4 * x + 6;
        }
        drawCircle8Points(xc, yc, x, y, r, g, b);
    }
}

void drawRectangle(int x0, int y0, int x1, int y1, float r, float g, float b) {

    drawLineBresenham(x0, y0, x1, y0, r, g, b);
    drawLineBresenham(x1, y0, x1, y1, r, g, b);
    drawLineBresenham(x1, y1, x0, y1, r, g, b);
    drawLineBresenham(x0, y1, x0, y0, r, g, b);
}

void floodFill(int startCol, int startRow, float fillColorR, float fillColorG, float fillColorB) {
    if (startCol < 0 || startCol >= GRID_COLS || startRow < 0 || startRow >= GRID_ROWS) return;

    PixelColor target = pixelGrid[startRow][startCol];
    if (target.r == fillColorR && target.g == fillColorG && target.b == fillColorB) return;

    std::queue<std::pair<int, int>> q;
    q.push({ startCol, startRow });

    while (!q.empty()) {
        auto [col, row] = q.front();
        q.pop();

        if (col < 0 || col >= GRID_COLS || row < 0 || row >= GRID_ROWS) continue;

        PixelColor current = pixelGrid[row][col];
        if (current.r == target.r && current.g == target.g && current.b == target.b) {
            setPixelColor(col, row, fillColorR, fillColorG, fillColorB);

            q.push({ col + 1, row });
            q.push({ col - 1, row });
            q.push({ col, row + 1 });
            q.push({ col, row - 1 });
        }
    }
}

void updateGridVertices(std::vector<GLfloat>& gridVertices) {
    gridVertices.clear();
    float stepX = 2.0f / GRID_COLS;
    float stepY = 2.0f / GRID_ROWS;
    float padding = 0.002f;

    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            float x = -1.0f + col * stepX + padding;
            float y = -1.0f + row * stepY + padding;
            float xNext = -1.0f + (col + 1) * stepX - padding;
            float yNext = -1.0f + (row + 1) * stepY - padding;

            float r = pixelGrid[row][col].r;
            float g = pixelGrid[row][col].g;
            float b = pixelGrid[row][col].b;

            // Triángulo 1
            gridVertices.push_back(x);     gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);

            // Triángulo 2
            gridVertices.push_back(x);     gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(x);     gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
        }
    }
}

// CALLBACKS DE ENTRADA (MOUSE Y TECLADO)

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int col, row;
        screenToGrid(window, xpos, ypos, col, row);

        if (action == GLFW_PRESS) {
            saveState();
            startX = col;
            startY = row;
            isDrawing = true;

            if (currentTool == PINCEL) {
                setPixelColor(col, row, currentColor.r, currentColor.g, currentColor.b);
            }
            else if (currentTool == BORRADOR) {
                setPixelColor(col, row, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b); // Pinta con el gris base de fondo
            }
            else if (currentTool == RELLENO) {
                floodFill(col, row, currentColor.r, currentColor.g, currentColor.b);
            }
        }
        else if (action == GLFW_RELEASE && isDrawing) {
            isDrawing = false;
            int endX = col, endY = row;

            if (currentTool == LINEA) {
                drawLineBresenham(startX, startY, endX, endY, currentColor.r, currentColor.g, currentColor.b);
            }
            else if (currentTool == RECTANGULO) {
                drawRectangle(startX, startY, endX, endY, currentColor.r, currentColor.g, currentColor.b);
            }
            else if (currentTool == CIRCULO) {
                int radius = static_cast<int>(std::round(std::sqrt(std::pow(endX - startX, 2) + std::pow(endY - startY, 2))));
                drawCircleBresenham(startX, startY, radius, currentColor.r, currentColor.g, currentColor.b);
            }
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isDrawing) {
        int col, row;
        screenToGrid(window, xpos, ypos, col, row);

        if (currentTool == PINCEL) {
            setPixelColor(col, row, currentColor.r, currentColor.g, currentColor.b);
        }
        else if (currentTool == BORRADOR) {
            setPixelColor(col, row, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b);
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) {
            undo();
            return;
        }

        if (key == GLFW_KEY_P) { currentTool = PINCEL; std::cout << "Herramienta: Pincel\n"; }
        if (key == GLFW_KEY_L) { currentTool = LINEA; std::cout << "Herramienta: Linea\n"; }
        if (key == GLFW_KEY_C) { currentTool = CIRCULO; std::cout << "Herramienta: Circulo\n"; }
        if (key == GLFW_KEY_R) { currentTool = RECTANGULO; std::cout << "Herramienta: Rectangulo\n"; }
        if (key == GLFW_KEY_F) { currentTool = RELLENO; std::cout << "Herramienta: Balde Relleno\n"; }
        if (key == GLFW_KEY_E) { currentTool = BORRADOR; std::cout << "Herramienta: Borrador\n"; }

        if (key == GLFW_KEY_1) { currentColor = { 1.0f, 0.0f, 0.0f }; std::cout << "Color: Rojo\n"; }
        if (key == GLFW_KEY_2) { currentColor = { 0.0f, 1.0f, 0.0f }; std::cout << "Color: Verde\n"; }
        if (key == GLFW_KEY_3) { currentColor = { 0.0f, 0.0f, 1.0f }; std::cout << "Color: Azul\n"; }
        if (key == GLFW_KEY_4) { currentColor = { 1.0f, 1.0f, 1.0f }; std::cout << "Color: Blanco\n"; }
    }
}


// (MAIN)


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Paint Basico - OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Error al crear la ventana GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    gladLoadGL();
    glViewport(0, 0, 800, 800);

    // Inicializar matriz a gris oscuro
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            pixelGrid[r][c] = BG_COLOR;
        }
    }

    std::vector<GLfloat> gridVertices;
    updateGridVertices(gridVertices);

    Shader shaderProgram("Shaders/default.vert", "Shaders/default.frag");

    VAO VAO1;
    VAO1.Bind();
    VBO VBO1(gridVertices.data(), gridVertices.size() * sizeof(GLfloat));
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO1.Unbind();
    VBO1.Unbind();

    std::cout << "-------------------------------------------\n";
    std::cout << "CONTROLES POR TECLADO ACTIVOS:\n";
    std::cout << "[P] Pincel | [L] Linea | [C] Circulo | [R] Rectangulo | [F] Balde | [E] Borrador\n";
    std::cout << "[1] Rojo   | [2] Verde | [3] Azul    | [4] Blanco   | [Ctrl+Z] Deshacer\n";
    std::cout << "-------------------------------------------\n";

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        VAO1.Bind();

        if (gridNeedsUpdate) {
            updateGridVertices(gridVertices);
            VBO1.Bind();
            glBufferSubData(GL_ARRAY_BUFFER, 0, gridVertices.size() * sizeof(GLfloat), gridVertices.data());
            gridNeedsUpdate = false;
        }

        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(gridVertices.size() / 6));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    VAO1.Delete();
    VBO1.Delete();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}