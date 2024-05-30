#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Vertex {
    float x, y, z;
};

struct Face {
    unsigned int v1, v2, v3;
};

std::vector<Vertex> vertices;
std::vector<Face> faces;

bool loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Falha ao abrir arquivo: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (prefix == "f") {
            Face face;
            char slash;
            iss >> face.v1 >> slash >> face.v2 >> slash >> face.v3;
            // OBJ format uses 1-based indexing
            face.v1--; face.v2--; face.v3--;
            faces.push_back(face);
        }
    }
    return true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        }
    }
}

void drawMesh() {
    glBegin(GL_TRIANGLES);
    for (const Face& face : faces) {
        glVertex3f(vertices[face.v1].x, vertices[face.v1].y, vertices[face.v1].z);
        glVertex3f(vertices[face.v2].x, vertices[face.v2].y, vertices[face.v2].z);
        glVertex3f(vertices[face.v3].x, vertices[face.v3].y, vertices[face.v3].z);
    }
    glEnd();
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
        return -1;
    }

    std::string filename = argv[1];
    if (!loadOBJ(filename)) {
        return -1;
    }

    GLFWwindow* window;

    /* Inicializa as bibliotecas */
    if (!glfwInit())
        return -1;

    /* Cria uma janela */
    window = glfwCreateWindow(640, 480, "Visualizador 3D", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Inicializa GLEW */
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    /* Registra o callback do teclado */
    glfwSetKeyCallback(window, key_callback);

    /* Habilita teste de profundidade */
    glEnable(GL_DEPTH_TEST);

    /* Configura a matriz de projeção */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /* Configura a matriz de visualização */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(1.0, 1.0, 3.0,  // Posição da câmera
            0.0, 0.0, 0.0,  // Ponto que a câmera está olhando
            0.0, 1.0, 0.0); // Vetor "up"

        // Desenha a malha
        drawMesh();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);


        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}