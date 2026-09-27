#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

// --- CONFIGURACIÓN Y ESTRUCTURA PARA PÍXELES INDEPENDIENTES ---
const int GRID_COLS = 40; // Número de columnas (eje X)
const int GRID_ROWS = 40; // Número de filas (eje Y)

struct PixelColor {
    float r, g, b;
};

// Matriz para modificar el color de cada píxel de forma independiente
PixelColor pixelGrid[GRID_ROWS][GRID_COLS];

// Función para cambiar el color de un píxel específico (columna, fila) a tu gusto
void setPixelColor(int col, int row, float r, float g, float b) {
    if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
        pixelGrid[row][col] = { r, g, b };
    }
}

// Callback para que la grilla sea DINÁMICA al redimensionar la ventana sin agregar más píxeles
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// --- FUNCIÓN BRESENHAM PARA LÍNEAS  ---
void drawLineBresenham(int x0, int y0, int x1, int y1, float r, float g, float b) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // Pintar el píxel en la posición actual
        setPixelColor(x0, y0, r, g, b);

        // Si llegó al punto final, termina
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int main()
{

    // Inicializar GLFW
    glfwInit();

    // especificar version de OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Ventana y sus parametros
    GLFWwindow* window = glfwCreateWindow(800, 800, "Mi primer ventana", NULL, NULL);


    //check de error si la ventana falla
    if (window == NULL) {
        std::cout << "Error a crear la ventana GLFW ";
        std::cout << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Conectar el callback de cambio de tamaño de ventana para que la grilla responda al tamaño
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGL(); //Carga GLAD y configura OpenGl

    glViewport(0, 0, 800, 800); //area de renderizado de la ventana x=0, y=0, a x=800, y=800 

    // --- INICIALIZACIÓN DE COLORES BASE DE CADA PÍXEL ---
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            pixelGrid[r][c] = { 0.2f, 0.2f, 0.2f }; // Color base gris oscuro
        }
    }

    // Dibuja una línea roja desde el píxel (x1, y1) hasta el píxel (x2, y2)
    drawLineBresenham(10, 10, 19, 19, 1.0f, 0.0f, 0.0f);

    // --- MODIFICA AQUÍ CUALQUIER PÍXEL A TU GUSTO (Ejemplos) ---
    //setPixelColor(0, 0, 1.0f, 0.0f, 0.0f);   // Píxel esquina inferior izquierda -> Rojo
    //setPixelColor(39, 39, 0.0f, 1.0f, 0.0f); // Píxel esquina superior derecha -> Verde
    //setPixelColor(20, 20, 0.0f, 0.0f, 1.0f); // Píxel centro -> Azul

    // --- GENERACIÓN DINÁMICA DE LA GRILLA ---
    const float STEP = 0.1f; // Medida mínima del cuadrado (0.1 en espacio NDC [-1, 1])
    std::vector<GLfloat> gridVertices;

    // OPCIÓN 1: LÍNEAS HORIZONTALES Y VERTICALES CONTINUAS
    //for (float pos = -1.0f; pos <= 1.0f + 0.0001f; pos += STEP)
    //{
        // Línea Vertical (x constante, va de y=-1 a y=1)
        //gridVertices.push_back(pos);   gridVertices.push_back(-1.0f); gridVertices.push_back(0.0f);   gridVertices.push_back(0.4f); gridVertices.push_back(0.4f); gridVertices.push_back(0.4f);
        //gridVertices.push_back(pos);   gridVertices.push_back(1.0f);  gridVertices.push_back(0.0f);   gridVertices.push_back(0.4f); gridVertices.push_back(0.4f); gridVertices.push_back(0.4f);

        // Línea Horizontal (y constante, va de x=-1 a x=1)
        //gridVertices.push_back(-1.0f); gridVertices.push_back(pos);   gridVertices.push_back(0.0f);   gridVertices.push_back(0.4f); gridVertices.push_back(0.4f); gridVertices.push_back(0.4f);
        //gridVertices.push_back(1.0f);  gridVertices.push_back(pos);   gridVertices.push_back(0.0f);   gridVertices.push_back(0.4f); gridVertices.push_back(0.4f); gridVertices.push_back(0.4f);
    //}


    //OPCIÓN 2: GENERACIÓN CUADRADO POR CUADRADO INDEPENDIENTE
    // Generar cuadrados como dos triángulos (triángulos llenos) y colorear por posición
    float stepX = 2.0f / GRID_COLS;
    float stepY = 2.0f / GRID_ROWS;
    float padding = 0.002f; // Margen para ver la separación


    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            
            //tamaño de pixeles con un pequeño espacio de separacion 
            float x = -1.0f + col * stepX + padding; 
            float y = -1.0f + row * stepY + padding;
            float xNext = -1.0f + (col + 1) * stepX - padding;
            float yNext = -1.0f + (row + 1) * stepY - padding;

            // Tomar el color INDIVIDUAL asignado a este píxel
            float r = pixelGrid[row][col].r;
            float g = pixelGrid[row][col].g;
            float b = pixelGrid[row][col].b;

            // Triángulo 1: (x,y), (xNext,y), (xNext,yNext)
            gridVertices.push_back(x);     gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);

            // Triángulo 2: (x,y), (xNext,yNext), (x,yNext)
            gridVertices.push_back(x);     gridVertices.push_back(y);     gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(xNext); gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
            gridVertices.push_back(x);     gridVertices.push_back(yNext); gridVertices.push_back(0.0f); gridVertices.push_back(r); gridVertices.push_back(g); gridVertices.push_back(b);
        }
    }



    //GLfloat vertices[] = {  //matriz de vertices con posicion -1<< x,y <<+1
        //                 COORDENADAS                /          COLORES
           // CUADRADO
              //-0.5f,  -0.5f * float(sqrt(3)) / 2,      0.0f,  0.8f, 0.3f,  0.02f, //esquina izquierda baja
              //0.5f,  -0.5f * float(sqrt(3)) / 2,      0.0f,  0.8f, 0.3f,  0.02f, //esquina derecha baja
              //-0.5f,  0.5f * float(sqrt(3)) / 2,      0.0f,  0.8f, 0.3f,  0.02f, //esquina izquierda alta
              //0.5f,  0.5f * float(sqrt(3)) / 2,      0.0f,  0.8f, 0.3f,  0.02f, //esquina derecha alta

         //TRIANGULO
           //-0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f, 
            //0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f,
            //0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f,     1.0f, 0.6f,  0.32f,
           //-0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f,
            //0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f,
            //0.0f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.8f, 0.3f,  0.02f


    //};

    //GLuint indices[] = {
         //0, 3, 5, //triangulo izquierdo
         //3, 2, 4, // triangulo derecho
         //5, 4, 1 // triangulo superior


         //0, 1, 2, //cuadrado
         //1, 3, 2  //cuadrado

    //};

    Shader shaderProgram("Shaders/default.vert", "Shaders/default.frag");


    // Genera un objeto de matriz de vértices (VAO) y lo vincula
    VAO VAO1;
    VAO1.Bind();

    // Genera un VBO con el vector dinámico de la grilla
    VBO VBO1(gridVertices.data(), gridVertices.size() * sizeof(GLfloat));

    // Genera un objeto de búfer de vértices y lo vincula a los vértices
    //VBO VBO1(vertices, sizeof(vertices));
    // Genera un objeto de búfer de elementos y lo vincula a los índices.
    //EBO EBO1(indices, sizeof(indices));

    // Vincula atributos de VBO, como coordenadas y colores, al VAO.
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    //  Desune todas para evitar modificarlas accidentalmente.
    VAO1.Unbind();
    VBO1.Unbind();
    //EBO1.Unbind();



    while (!glfwWindowShouldClose(window))
    {
        // color de fondo
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // limpia el back buffer y le da un color
        glClear(GL_COLOR_BUFFER_BIT);
        // le dice al programa de opengl que shader usar
        shaderProgram.Activate();
        // libera el VAO para wue opengl sepa que debe usarlo
        VAO1.Bind();
        // Dibujar primitivas, número de índices, tipo de datos de los índices, índice de los índices
        //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0); //(triangulos)
        // Dibujar como triángulos para obtener cuadrados llenos (6 vértices por cuadrado)
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(gridVertices.size() / 6));
        // cambia el back con el front buffer
        glfwSwapBuffers(window);
        // encargado de los eventos del GLWF
        glfwPollEvents();
    }

    // elimina todos los objetos creados
    VAO1.Delete();
    VBO1.Delete();
    //EBO1.Delete();
    shaderProgram.Delete();
    // Cierre la ventana antes de finalizar el programa.
    glfwDestroyWindow(window);
    // Finaliza GLFW antes de terminar el programa.
    glfwTerminate();
    return 0;
}