#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Função para desenhar os eixos X, Y e Z
void drawAxis() {
    glLineWidth(1.5f);
    
    //x verde
    glColor3f(0.1f, 0.4f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(-320.0f, 0.0f, 0.0f);
    glVertex3f(320.0f, 0.0f, 0.0f);
    glEnd();

    //y azul
    glColor3f(0.2f, 0.0f, 0.8f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -240.0f, 0.0f);
    glVertex3f(0.0f, 240.0f, 0.0f);
    glEnd();

    // z vermelho
    glColor3f(0.4f, 0.1f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -200.0f);
    glVertex3f(0.0f, 0.0f, 200.0f);
    glEnd();
}

void teclado(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS || key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


int main(void)
{
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

    /* Habilita teste de profundidade */
    glEnable(GL_DEPTH_TEST);

    /* Configura a matriz de projeção */
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /* Configura a matriz de visualização */
        glm::mat4 view = glm::lookAt(glm::vec3(1.0f, 1.0f, 3.0f),  // Posição da câmera
                                     glm::vec3(0.0f, 0.0f, 0.0f),  // Ponto que a câmera está olhando
                                     glm::vec3(0.0f, 1.0f, 0.0f)); // Vetor "up"
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(view));

        //desenha os eixos
        drawAxis();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);


        /* Poll for and process events */
        glfwPollEvents();

        glfwSetKeyCallback(window, teclado);
    }
    drawAxis();

    glfwTerminate();
    return 0;
}
