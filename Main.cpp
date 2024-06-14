#include <GL/glew.h>      // Biblioteca para gerenciar extensões OpenGL.
#include <GLFW/glfw3.h>   // Biblioteca para criar janelas e capturar eventos de entrada.
#include <GL/glu.h>       // Biblioteca utilitária para OpenGL.
#include <iostream>       // Biblioteca padrão de entrada e saída.
#include <vector>         // Biblioteca para uso de vetores dinâmicos.
#include <fstream>        // Biblioteca para operações de arquivos.
#include <sstream>        // Biblioteca para manipulação de streams de strings.
#include <string>         // Biblioteca para manipulação de strings.
#include <cmath>          // Biblioteca para funções matemáticas.

#ifndef M_PI              // Verifica se PI está definido.
#define M_PI 3.14159265358979323846 // Define a constante PI.
#endif

// Estrutura que representa um vértice 3D.
struct Vertex {
    float x, y, z; // Coordenadas do vértice.
};

// Estrutura que representa uma face de um polígono, usando índices de vértices.
struct Face {
    unsigned int v1, v2, v3; // Índices dos vértices que formam a face.
};

// Vetores para armazenar vértices, faces e transformações do primeiro objeto.
std::vector<Vertex> vertices;
std::vector<Face> faces;
std::vector<std::string> transformations;

// Vetores para armazenar vértices, faces e transformações do segundo objeto (transformado).
std::vector<Vertex> verticesOther;
std::vector<Face> facesOther;
std::vector<std::string> transformationsOther;

// Função para obter o caminho do diretório de um arquivo.
std::string getDirectoryPath(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\"); // Encontra a última barra no caminho.
    return (pos == std::string::npos) ? "" : filepath.substr(0, pos + 1); // Retorna o caminho do diretório.
}

// Função para carregar um arquivo OBJ.
bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<Face>& faces, std::vector<std::string>& transformations) {
    std::ifstream file(filename); // Abre o arquivo.
    if (!file.is_open()) { // Verifica se o arquivo foi aberto corretamente.
        std::cerr << "Falha ao abrir arquivo: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) { // Lê o arquivo linha por linha.
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix; // Extrai o prefixo da linha para determinar o tipo de dado.

        if (prefix == "v") { // Processa vértices.
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z; // Lê as coordenadas do vértice.
            vertices.push_back(vertex); // Adiciona o vértice ao vetor.
        }
        else if (prefix == "f") { // Processa faces.
            Face face;
            char slash;
            std::string vertex1, vertex2, vertex3;

            iss >> vertex1 >> vertex2 >> vertex3; // Lê os índices dos vértices da face.

            // Usa sscanf para extrair os índices dos vértices (ignorando texturas e normais).
            sscanf_s(vertex1.c_str(), "%u/%*d/%*d", &face.v1);
            sscanf_s(vertex2.c_str(), "%u/%*d/%*d", &face.v2);
            sscanf_s(vertex3.c_str(), "%u/%*d/%*d", &face.v3);

            // Ajusta os índices para começarem em 0.
            face.v1--; face.v2--; face.v3--;

            // Verifica se os índices estão dentro dos limites do vetor de vértices.
            if (face.v1 < 0 || face.v1 >= vertices.size() ||
                face.v2 < 0 || face.v2 >= vertices.size() ||
                face.v3 < 0 || face.v3 >= vertices.size()) {
                std::cerr << "Índice de vértice fora dos limites em: " << line << std::endl;
                continue;
            }

            faces.push_back(face); // Adiciona a face ao vetor.
        }
        // Processa transformações específicas.
        else if (prefix == "s" || prefix == "t" || prefix == "x" ||
            prefix == "y" || prefix == "z" || prefix == "c" || prefix == "e") {
            transformations.push_back(line); // Adiciona a transformação ao vetor.
        }
    }

    // Exibe a contagem total de vértices e faces carregadas.
    std::cout << "Total de vértices carregados: " << vertices.size() << std::endl;
    std::cout << "Total de faces carregadas: " << faces.size() << std::endl;
    return true;
}

// Função para desenhar os eixos X, Y e Z no espaço 3D.
void drawAxes() {
    float axisLength = 50.0f; // Define o comprimento dos eixos.
    glLineWidth(2.0f); // Define a largura da linha.
    glBegin(GL_LINES);

    // Desenha o eixo X (em verde).
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(axisLength, 0.0f, 0.0f);

    // Desenha o eixo Y (em azul).
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, axisLength, 0.0f);

    // Desenha o eixo Z (em vermelho).
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, axisLength);

    glEnd(); // Finaliza o desenho das linhas.
}

// Define modos de exibição: WIREFRAME (apenas arestas) ou FILLED (preenchido).
enum DisplayMode { WIREFRAME, FILLED };
DisplayMode mode = WIREFRAME; // Modo de exibição inicial.

bool lightEnabled = false; // Estado da iluminação.
unsigned int currentTransformation = 0; // Índice da transformação atual.
bool showOriginal = true; // Flag para alternar entre mostrar o objeto original ou transformado.

// Função para alternar a iluminação.
void toggleLight() {
    if (lightEnabled) {
        glDisable(GL_LIGHTING); // Desabilita a iluminação.
        glDisable(GL_LIGHT0);   // Desabilita a luz 0.
    }
    else {
        glEnable(GL_LIGHTING);  // Habilita a iluminação.
        glEnable(GL_LIGHT0);    // Habilita a luz 0.

        // Define as propriedades da luz 0.
        GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        // Aplica as propriedades da luz.
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    }
    lightEnabled = !lightEnabled; // Alterna o estado da iluminação.
}

// Funções para aplicar transformações ao segundo conjunto de vértices e registrar as transformações.

// Aplica uma translação aos vértices.
void applyTranslationOther(std::vector<Vertex>& verticesOther, float translationX, float translationY, float translationZ) {
    for (auto& vertex : verticesOther) {
        vertex.x += translationX;
        vertex.y += translationY;
        vertex.z += translationZ;
    }
    // Registra a transformação.
    transformationsOther.push_back("t " + std::to_string(translationX) + " " + std::to_string(translationY) + " " + std::to_string(translationZ));
}

// Aplica uma rotação no eixo X aos vértices.
void applyRotationXOther(std::vector<Vertex>& verticesOther, float angle) {
    float rad = angle * M_PI / 180.0f; // Converte o ângulo para radianos.
    float cosAngle = cos(rad);
    float sinAngle = sin(rad);
    for (auto& vertex : verticesOther) {
        float y = vertex.y;
        float z = vertex.z;
        vertex.y = y * cosAngle - z * sinAngle;
        vertex.z = y * sinAngle + z * cosAngle;
    }
    // Registra a transformação.
    transformationsOther.push_back("x " + std::to_string(angle));
}

// Aplica uma rotação no eixo Y aos vértices.
void applyRotationYOther(std::vector<Vertex>& verticesOther, float angle) {
    float rad = angle * M_PI / 180.0f;
    float cosAngle = cos(rad);
    float sinAngle = sin(rad);
    for (auto& vertex : verticesOther) {
        float x = vertex.x;
        float z = vertex.z;
        vertex.x = x * cosAngle + z * sinAngle;
        vertex.z = -x * sinAngle + z * cosAngle;
    }
    // Registra a transformação.
    transformationsOther.push_back("y " + std::to_string(angle));
}

// Aplica uma rotação no eixo Z aos vértices.
void applyRotationZOther(std::vector<Vertex>& verticesOther, float angle) {
    float rad = angle * M_PI / 180.0f;
    float cosAngle = cos(rad);
    float sinAngle = sin(rad);
    for (auto& vertex : verticesOther) {
        float x = vertex.x;
        float y = vertex.y;
        vertex.x = x * cosAngle - y * sinAngle;
        vertex.y = x * sinAngle + y * cosAngle;
    }
    // Registra a transformação.
    transformationsOther.push_back("z " + std::to_string(angle));
}

// Aplica cisalhamento aos vértices.
void applyShearOther(std::vector<Vertex>& verticesOther, float shearXY, float shearXZ, float shearYX, float shearYZ, float shearZX, float shearZY) {
    for (auto& vertex : verticesOther) {
        float x = vertex.x;
        float y = vertex.y;
        float z = vertex.z;
        vertex.x = x + shearXY * y + shearXZ * z;
        vertex.y = y + shearYX * x + shearYZ * z;
        vertex.z = z + shearZX * x + shearZY * y;
    }
    // Registra a transformação.
    transformationsOther.push_back("c " + std::to_string(shearXY) + " " + std::to_string(shearXZ) + " " + std::to_string(shearYX));
}

// Aplica escala aos vértices.
void applyScaleOther(std::vector<Vertex>& verticesOther, float scaleX, float scaleY, float scaleZ) {
    for (auto& vertex : verticesOther) {
        vertex.x *= scaleX;
        vertex.y *= scaleY;
        vertex.z *= scaleZ;
    }
    // Registra a transformação.
    transformationsOther.push_back("s " + std::to_string(scaleX) + " " + std::to_string(scaleY) + " " + std::to_string(scaleZ));
}

// Aplica reflexão aos vértices.
void applyReflectionOther(std::vector<Vertex>& verticesOther, float reflectX, float reflectY, float reflectZ) {
    for (auto& vertex : verticesOther) {
        vertex.x *= reflectX;
        vertex.y *= reflectY;
        vertex.z *= reflectZ;
    }
    // Registra a transformação.
    transformationsOther.push_back("e " + std::to_string(reflectX) + " " + std::to_string(reflectY) + " " + std::to_string(reflectZ));
}

// Função para aplicar uma matriz de cisalhamento usando OpenGL.
void applyShearing(float shx, float shy, float shz) {
    GLfloat shear[16] = {
        1.0f, shx, 0.0f, 0.0f,
        shy, 1.0f, shz, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(shear); // Multiplica a matriz de transformação atual pela matriz de cisalhamento.
    std::cout << "Shearing: " << shx << ", " << shy << ", " << shz << std::endl;
}

// Função para aplicar uma matriz de reflexão usando OpenGL.
void applyReflection(float ex, float ey, float ez) {
    GLfloat reflect[16] = {
        ex == 1 ? -1.0f : 1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, ey == 1 ? -1.0f : 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, ez == 1 ? -1.0f : 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(reflect); // Multiplica a matriz de transformação atual pela matriz de reflexão.
    std::cout << "Reflection: " << ex << ", " << ey << ", " << ez << std::endl;
}

// Função para desenhar a malha de vértices com base no modo de visualização.
void drawMesh(const std::vector<Vertex>& verticesToDraw, const std::vector<Face>& facesToDraw) {
    if (mode == WIREFRAME) { // Se o modo for WIREFRAME, desenha apenas as arestas.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 0.0f, 0.0f); // Define a cor das arestas (preto).
    }
    else { // Se o modo for FILLED, desenha as faces preenchidas.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(0.0f, 0.0f, 1.0f); // Define a cor de preenchimento (azul).
    }

    glBegin(GL_TRIANGLES); // Começa a desenhar triângulos.
    for (const Face& face : facesToDraw) {
        glVertex3f(verticesToDraw[face.v1].x, verticesToDraw[face.v1].y, verticesToDraw[face.v1].z);
        glVertex3f(verticesToDraw[face.v2].x, verticesToDraw[face.v2].y, verticesToDraw[face.v2].z);
        glVertex3f(verticesToDraw[face.v3].x, verticesToDraw[face.v3].y, verticesToDraw[face.v3].z);
    }
    glEnd(); // Finaliza o desenho dos triângulos.
}

// Função para aplicar a transformação atual ao conjunto de vértices "verticesOther".
void applyCurrentTransformationOther() {
    if (currentTransformation < transformationsOther.size()) {
        std::istringstream iss(transformationsOther[currentTransformation]);
        std::string type;
        iss >> type;

        if (type == "t") { // Aplica translação.
            float tx, ty, tz;
            iss >> tx >> ty >> tz;
            applyTranslationOther(verticesOther, tx, ty, tz);
        }
        else if (type == "s") { // Aplica escala.
            float sx, sy, sz;
            iss >> sx >> sy >> sz;
            applyScaleOther(verticesOther, sx, sy, sz);
        }
        else if (type == "x") { // Aplica rotação no eixo X.
            float angle;
            iss >> angle;
            applyRotationXOther(verticesOther, angle);
        }
        else if (type == "y") { // Aplica rotação no eixo Y.
            float angle;
            iss >> angle;
            applyRotationYOther(verticesOther, angle);
        }
        else if (type == "z") { // Aplica rotação no eixo Z.
            float angle;
            iss >> angle;
            applyRotationZOther(verticesOther, angle);
        }
        else if (type == "c") { // Aplica cisalhamento.
            float shx, shy, shz;
            iss >> shx >> shy >> shz;
            applyShearing(shx, shy, shz);
        }
        else if (type == "e") { // Aplica reflexão.
            float ex, ey, ez;
            iss >> ex >> ey >> ez;
            applyReflection(ex, ey, ez);
        }
    }
}

// Função para desenhar a cena, incluindo os eixos e a malha de vértices.
void drawScene() {
    drawAxes(); // Desenha os eixos.

    // Desenha o objeto original em vermelho.
    glColor3f(1.0f, 0.0f, 0.0f);
    drawMesh(vertices, faces);

    if (!showOriginal) {
        applyCurrentTransformationOther(); // Aplica a transformação ao segundo conjunto de vértices.
        glColor3f(0.0f, 1.0f, 0.0f); // Define a cor verde para o objeto transformado.
        drawMesh(verticesOther, facesOther); // Desenha o objeto transformado.
    }
}

// Função de callback para eventos de teclado.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE: // Sai do programa.
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_P: // Alterna entre modos de exibição WIREFRAME e FILLED.
            mode = (mode == WIREFRAME) ? FILLED : WIREFRAME;
            break;
        case GLFW_KEY_L: // Alterna a iluminação.
            toggleLight();
            break;
        case GLFW_KEY_SPACE: // Alterna entre mostrar o original e o transformado, ou passa para a próxima transformação.
            if (currentTransformation < transformationsOther.size() - 1) {
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

// Função de callback para redimensionamento de janela.
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); // Define a área de visualização.
    glMatrixMode(GL_PROJECTION); // Configura a projeção.
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 100.0); // Define a perspectiva.
}

// Função principal do programa.
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_arquivo_obj>" << std::endl;
        return -1;
    }

    std::string filename = argv[1]; // Obtém o nome do arquivo OBJ a partir dos argumentos.
    if (!loadOBJ(filename, vertices, faces, transformations)) { // Carrega o arquivo OBJ.
        return -1;
    }

    // Copia os dados para o segundo conjunto de vértices e faces.
    verticesOther = vertices;
    facesOther = faces;
    transformationsOther = transformations;

    if (!glfwInit()) // Inicializa a biblioteca GLFW.
        return -1;

    // Cria uma janela de 640x480 com o título "Visualizador 3D".
    GLFWwindow* window = glfwCreateWindow(640, 480, "Visualizador 3D", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Torna o contexto da janela atual.

    if (glewInit() != GLEW_OK) { // Inicializa a biblioteca GLEW.
        std::cerr << "Falha ao inicializar GLEW" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, key_callback); // Define a função de callback para teclas.
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Define a função de callback para redimensionamento de janela.

    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade.

    glMatrixMode(GL_PROJECTION); // Configura a matriz de projeção.
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0); // Define a perspectiva.

    // Loop principal de renderização.
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpa o buffer de cor e profundidade.

        glMatrixMode(GL_MODELVIEW); // Configura a matriz de modelo/visualização.
        glLoadIdentity();
        gluLookAt(3.0, 3.0, 5.0,  // Define a posição da câmera.
            0.0, 0.0, 0.0,     // Define o ponto que a câmera está olhando.
            0.0, 1.0, 0.0);    // Define o vetor "up".

        drawScene(); // Desenha a cena.

        glfwSwapBuffers(window); // Troca os buffers de tela.
        glfwPollEvents(); // Processa eventos de entrada.
    }

    glfwTerminate(); // Finaliza a biblioteca GLFW.
    return 0;
}