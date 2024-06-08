#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

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
std::vector<std::string> transformations;

std::string getDirectoryPath(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    return (pos == std::string::npos) ? "" : filepath.substr(0, pos + 1);
}

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
            std::string vertex1, vertex2, vertex3;

            iss >> vertex1 >> vertex2 >> vertex3;

            sscanf_s(vertex1.c_str(), "%u/%*d/%*d", &face.v1);
            sscanf_s(vertex2.c_str(), "%u/%*d/%*d", &face.v2);
            sscanf_s(vertex3.c_str(), "%u/%*d/%*d", &face.v3);

            face.v1--; face.v2--; face.v3--;

            if (face.v1 < 0 || face.v1 >= vertices.size() ||
                face.v2 < 0 || face.v2 >= vertices.size() ||
                face.v3 < 0 || face.v3 >= vertices.size()) {
                std::cerr << "Índice de vértice fora dos limites em: " << line << std::endl;
                continue;
            }

            faces.push_back(face);
        }
        else if (prefix == "s" || prefix == "t" || prefix == "x" ||
            prefix == "y" || prefix == "z" || prefix == "c" || prefix == "e") {
            transformations.push_back(line);
        }
    }

    std::cout << "Total de vértices carregados: " << vertices.size() << std::endl;
    std::cout << "Total de faces carregadas: " << faces.size() << std::endl;
    return true;
}

void drawAxes() {
    float axisLength = 50.0f; // Aumenta o comprimento dos eixos
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // Eixo X (Verde)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(axisLength, 0.0f, 0.0f);

    // Eixo Y (Azul)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, axisLength, 0.0f);

    // Eixo Z (Vermelho)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, axisLength);

    glEnd();
}

enum DisplayMode { WIREFRAME, FILLED };
DisplayMode mode = WIREFRAME;

bool lightEnabled = false;

unsigned int currentTransformation = 0;
bool showOriginal = true;

void toggleLight() {
    if (lightEnabled) {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
    else {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    }
    lightEnabled = !lightEnabled;
}

void applyTransformations() {
    for (unsigned int i = 0; i <= currentTransformation; ++i) {
        std::istringstream iss(transformations[i]);
        std::string type;
        iss >> type;

        if (type == "s") {
            float sx, sy, sz;
            iss >> sx >> sy >> sz;
            glScalef(sx, sy, sz);
        }
        else if (type == "t") {
            float tx, ty, tz;
            iss >> tx >> ty >> tz;
            glTranslatef(tx, ty, tz);
        }
        else if (type == "x") {
            float angle;
            iss >> angle;
            glRotatef(angle, 1.0f, 0.0f, 0.0f);
        }
        else if (type == "y") {
            float angle;
            iss >> angle;
            glRotatef(angle, 0.0f, 1.0f, 0.0f);
        }
        else if (type == "z") {
            float angle;
            iss >> angle;
            glRotatef(angle, 0.0f, 0.0f, 1.0f);
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_P:
            if (mode == WIREFRAME)
                mode = FILLED;
            else
                mode = WIREFRAME;
            break;
        case GLFW_KEY_L:
            toggleLight();
            break;
        case GLFW_KEY_SPACE:
            if (currentTransformation < transformations.size() - 1) {
                ++currentTransformation;
            }
            else {
                currentTransformation = 0;
                showOriginal = !showOriginal;
            }
            break;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 100.0);
}

void drawMesh() {
    if (mode == WIREFRAME) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f); // Cor das arestas
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(0.0f, 0.0f, 1.0f); // Cor de preenchimento
    }

    glBegin(GL_TRIANGLES);
    for (const Face& face : faces) {
        glVertex3f(vertices[face.v1].x, vertices[face.v1].y, vertices[face.v1].z);
        glVertex3f(vertices[face.v2].x, vertices[face.v2].y, vertices[face.v2].z);
        glVertex3f(vertices[face.v3].x, vertices[face.v3].y, vertices[face.v3].z);
    }
    glEnd();
}

void drawTransformedMesh() {
    for (unsigned int i = 0; i <= currentTransformation; ++i) {
        glPushMatrix();

        for (unsigned int j = 0; j < i; ++j) {
            std::istringstream iss(transformations[j]);
            std::string type;
            iss >> type;

            if (type == "s") {
                float sx, sy, sz;
                iss >> sx >> sy >> sz;
                glScalef(sx, sy, sz);
            }
            else if (type == "t") {
                float tx, ty, tz;
                iss >> tx >> ty >> tz;
                glTranslatef(tx, ty, tz);
            }
            else if (type == "x") {
                float angle;
                iss >> angle;
                glRotatef(angle, 1.0f, 0.0f, 0.0f);
            }
            else if (type == "y") {
                float angle;
                iss >> angle;
                glRotatef(angle, 0.0f, 1.0f, 0.0f);
            }
            else if (type == "z") {
                float angle;
                iss >> angle;
                glRotatef(angle, 0.0f, 0.0f, 1.0f);
            }
        }

        drawMesh();

        glPopMatrix();
    }
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

    /* Registra o callback de redimensionamento */
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
        gluLookAt(3.0, 3.0, 5.0,  // Posição da câmera
            0.0, 0.0, 0.0,  // Ponto que a câmera está olhando
            0.0, 1.0, 0.0); // Vetor "up"

        // Desenha os eixos
        drawAxes();

        // Aplica transformações e desenha malha transformada
        if (!showOriginal) {
            drawTransformedMesh();
        }
        else {
            drawMesh();
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}