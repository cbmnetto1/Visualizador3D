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

// Estrutura para armazenar vértices e faces
struct Vertex {
    float x, y, z;
};

struct Face {
    int v1, v2, v3;
};

struct Transformation {
    char type;
    float x, y, z;
    float angle; // Usado para rotações
};

// Variáveis globais
std::vector<Vertex> vertices;
std::vector<Face> faces;
std::vector<Transformation> transformations;
bool showWireframe = true;
bool lightOn = false;
int currentTransformation = 0;
std::vector<Vertex> transformedVertices;

// Funções de Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_P:
            showWireframe = !showWireframe;
            break;
        case GLFW_KEY_L:
            lightOn = !lightOn;
            break;
        case GLFW_KEY_SPACE:
            currentTransformation = (currentTransformation + 1) % (transformations.size() + 1);
            break;
        }
    }
}

// Funções para carregar e aplicar transformações
void loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo .obj\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;
        if (type == 'v') {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (type == 'f') {
            Face f;
            iss >> f.v1 >> f.v2 >> f.v3;
            faces.push_back(f);
        }
        else if (type == 't' || type == 'r' || type == 's' || type == 'x' || type == 'y' || type == 'z' || type == 'c') {
            Transformation t;
            t.type = type;
            if (type == 'r') {
                iss >> t.angle >> t.x >> t.y >> t.z;
            }
            else {
                iss >> t.x >> t.y >> t.z;
            }
            transformations.push_back(t);
        }
    }

    transformedVertices = vertices;
}

void applyTransformations() {
    transformedVertices = vertices;

    for (int i = 0; i < currentTransformation; ++i) {
        const Transformation& t = transformations[i];
        if (t.type == 't') {
            for (auto& v : transformedVertices) {
                v.x += t.x;
                v.y += t.y;
                v.z += t.z;
            }
        }
        else if (t.type == 'r') {
            float rad = t.angle * M_PI / 180.0f;
            float cosA = cos(rad);
            float sinA = sin(rad);
            float x = t.x, y = t.y, z = t.z;
            float length = sqrt(x * x + y * y + z * z);
            x /= length;
            y /= length;
            z /= length;
            for (auto& v : transformedVertices) {
                float newX = (cosA + (1 - cosA) * x * x) * v.x + ((1 - cosA) * x * y - z * sinA) * v.y + ((1 - cosA) * x * z + y * sinA) * v.z;
                float newY = ((1 - cosA) * y * x + z * sinA) * v.x + (cosA + (1 - cosA) * y * y) * v.y + ((1 - cosA) * y * z - x * sinA) * v.z;
                float newZ = ((1 - cosA) * z * x - y * sinA) * v.x + ((1 - cosA) * z * y + x * sinA) * v.y + (cosA + (1 - cosA) * z * z) * v.z;
                v.x = newX;
                v.y = newY;
                v.z = newZ;
            }
        }
        else if (t.type == 's') {
            for (auto& v : transformedVertices) {
                v.x *= t.x;
                v.y *= t.y;
                v.z *= t.z;
            }
        }
    }
}

// Função para desenhar os eixos X, Y e Z
void drawAxis() {
    glLineWidth(1.5f);

    // x verde
    glColor3f(0.1f, 0.8f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(-10.0f, 0.0f, 0.0f);
    glVertex3f(10.0f, 0.0f, 0.0f);
    glEnd();

    // y azul
    glColor3f(0.2f, 0.2f, 0.8f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -10.0f, 0.0f);
    glVertex3f(0.0f, 10.0f, 0.0f);
    glEnd();

    // z vermelho
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -10.0f);
    glVertex3f(0.0f, 0.0f, 10.0f);
    glEnd();
}

// Função para desenhar o objeto
void drawObject(const std::vector<Vertex>& verticesToDraw, bool wireframe) {
    if (wireframe) {
        glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para o modelo de arame
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glColor3f(0.0f, 0.0f, 1.0f); // Cor azul para o modelo preenchido
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glLineWidth(1.0f);
        glColor3f(0.0f, 0.0f, 0.0f); // Cor preta para as arestas dos polígonos
    }

    glBegin(GL_TRIANGLES);
    for (const auto& face : faces) {
        const Vertex& v1 = verticesToDraw[face.v1 - 1];
        const Vertex& v2 = verticesToDraw[face.v2 - 1];
        const Vertex& v3 = verticesToDraw[face.v3 - 1];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path.obj>" << std::endl;
        return 1;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Visualizador 3D", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Configurar iluminação
    GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glEnable(GL_LIGHT0);

    // Carregar objeto .obj
    loadOBJ(argv[1]);

    // Configurar a matriz de projeção usando glFrustum
    float aspect = 640.0f / 480.0f;
    float near = 0.1f;
    float far = 100.0f;
    float fov = 45.0f;
    float top = tan(fov * M_PI / 360.0f) * near;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (lightOn) {
            glEnable(GL_LIGHTING);
        }
        else {
            glDisable(GL_LIGHTING);
        }

        // Configurar a matriz de visualização e projeção
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(left, right, bottom, top, near, far);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        // Desenhar eixos
        drawAxis();

        // Aplicar transformações e desenhar objeto
        applyTransformations();
        drawObject(transformedVertices, showWireframe);

        glfwSwapBuffers(window); 
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}