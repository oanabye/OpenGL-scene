#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <cstdlib>
#include <ctime>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

// Parametri fereastră
int glWindowWidth = 1928;
int glWindowHeight = 1019;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

// Matrice și locații
glm::mat4 model;
GLint modelLoc;

// Variabile Cameră
gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

// Variabile Mouse/Tastatură
bool pressedKeys[1024];
float angle = 0.0f;
float angleElice = 0.0;
bool firstMouse = true;
float lastX = 400.0f, lastY = 300.0f;
float yaw = -90.0f;
float pitch = 0.0f;
bool rotateObject = false;
float rotationSpeed = 500.0f;
float fogDensity = 0.01f;
glm::vec3 fogColor = glm::vec3(0.5f, 0.6f, 0.7f);
bool pointLightsEnabled = false;
bool pointFogEnabled = false;
bool rainEnabled = false;

// Pozițiile felinarelor
glm::vec3 felinar1Pos = glm::vec3(102.942f, 21.634f, -(8.87284f));
glm::vec3 felinar2Pos = glm::vec3(42.83f, 19.8223f, -51.8376f);

// Poziția luminii direcționale pentru umbre
glm::vec3 lightPos = glm::vec3(50.0f, 100.0f, 50.0f);

// Obiecte scenă
gps::Model3D myModel;          
gps::Model3D skydomeModel;      
gps::Model3D moriscaModel;      

gps::Shader myCustomShader;
gps::Shader depthMapShader;
gps::Shader rainShader;

GLuint verticesVBO;
GLuint verticesEBO;
GLuint objectVAO;
GLuint texture;

GLfloat vertexData[] = { 0.0f };
GLuint vertexIndices[] = { 0 };

// Structura de date a ploii
struct RainDrop {
    glm::vec3 pos;
    float speed;
};

std::vector<RainDrop> rain;
const int nrRaindrops = 3000;
GLuint rainVAO, rainVBO;

// Umbra
GLuint shadowMapFBO;
GLuint depthMapTexture;

// Variabile pentru skydome
bool renderingSkydome = false;
const float SKYDOME_SCALE = 500.0f;

// Variabile pentru animatia camerei
bool cinematicCamera = false;     
float cameraAngle = 0.0f;         
float cameraRadius = 80.0f;      
float cameraHeight = 25.0f;       
float cameraAngularSpeed = 0.5f;  
glm::vec3 cameraCenter = glm::vec3(0.0f, 20.0f, 0.0f); 


void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        rotateObject = !rotateObject;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        pointLightsEnabled = !pointLightsEnabled;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        pointFogEnabled = !pointFogEnabled;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        rainEnabled = !rainEnabled;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        cinematicCamera = !cinematicCamera;
    }


    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_1:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case GLFW_KEY_2:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case GLFW_KEY_3:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        }
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_Q]) {
        yaw -= 1.0f;
        myCamera.rotate(pitch, yaw);
    }
    if (pressedKeys[GLFW_KEY_E]) {
        yaw += 1.0f;
        myCamera.rotate(pitch, yaw);
    }

    if (rotateObject) {
        angleElice += rotationSpeed * 0.016f;
        if (angleElice >= 360.0f) angleElice -= 360.0f;
    }

    cameraSpeed = 2.0f;

    if (pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_UP]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_DOWN]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_LEFT]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_D] || pressedKeys[GLFW_KEY_RIGHT]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }
}
void updateCinematicCamera(float deltaTime) {
    cameraAngle += cameraAngularSpeed * deltaTime;

    float camX = cameraCenter.x + cameraRadius * cos(cameraAngle);
    float camZ = cameraCenter.z + cameraRadius * sin(cameraAngle);
    float camY = cameraHeight;

    glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);

    myCamera = gps::Camera(
        cameraPos,
        cameraCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Scene Traversal", NULL, NULL);

    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);

    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
    return true;
}

void initRain() {
    srand(static_cast<unsigned int>(time(NULL)));

    std::vector<glm::vec3> rainVertices;
    rain.clear();

    float sceneMinX = -200.0f, sceneMaxX = 200.0f;
    float sceneMinZ = -200.0f, sceneMaxZ = 200.0f;
    float rainHeight = 150.0f;

    for (int i = 0; i < nrRaindrops; i++) {
        glm::vec3 pos = glm::vec3(
            sceneMinX + static_cast<float>(rand()) / RAND_MAX * (sceneMaxX - sceneMinX),
            rand() % static_cast<int>(rainHeight) + 50.0f,   
            sceneMinZ + static_cast<float>(rand()) / RAND_MAX * (sceneMaxZ - sceneMinZ)
        );
        rainVertices.push_back(pos);
        rainVertices.push_back(pos + glm::vec3(0.0f, -2.0f, 0.0f));
        rain.push_back({ pos, 20.0f + static_cast<float>(rand() % 20) });
    }

    glGenVertexArrays(1, &rainVAO);
    glBindVertexArray(rainVAO);
    glGenBuffers(1, &rainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferData(GL_ARRAY_BUFFER, rainVertices.size() * sizeof(glm::vec3), rainVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}


void updateRain(float deltaTime) {
    float sceneMinX = -200.0f, sceneMaxX = 200.0f;
    float sceneMinZ = -200.0f, sceneMaxZ = 200.0f;
    float rainHeight = 150.0f;

    for (auto& d : rain) {
        d.pos.y -= d.speed * deltaTime;
        if (d.pos.y < 0.0f) {
            d.pos.y = rainHeight;
            d.pos.x = sceneMinX + static_cast<float>(rand()) / RAND_MAX * (sceneMaxX - sceneMinX);
            d.pos.z = sceneMinZ + static_cast<float>(rand()) / RAND_MAX * (sceneMaxZ - sceneMinZ);
        }
    }
}


void renderRain() {
    if (!rainEnabled) return;

    std::vector<glm::vec3> vertices;
    vertices.reserve(nrRaindrops * 2);

    for (auto& d : rain) {
        vertices.push_back(d.pos);
        vertices.push_back(d.pos + glm::vec3(0.0f, -2.0f, 0.0f));
    }

    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());

    rainShader.useShaderProgram();

    glm::mat4 view = myCamera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(retina_width) / static_cast<float>(retina_height),
        0.1f, 1000.0f);

    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "view"),
        1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(rainShader.shaderProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(rainShader.shaderProgram, "rainColor"), 0.4f, 0.6f, 1.0f);
    glUniform1f(glGetUniformLocation(rainShader.shaderProgram, "alpha"), 0.6f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(rainVAO);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR: Shadow framebuffer is not complete! Status: 0x%X\n", status);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    float near_plane = 1.0f, far_plane = 200.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lightPos = lightTarget - lightDir * 100.0f;

    glm::mat4 lightView = glm::lookAt(
        lightPos,
        lightTarget,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    return lightProjection * lightView;
}

void renderDepthMap(glm::mat4 lightSpaceTrMatrix) {
    glCullFace(GL_FRONT);

    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glm::vec3 pivot = glm::vec3(-41.6928f, 43.2391f, 61.8275f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pivot);
    model = glm::rotate(model, glm::radians(angleElice), glm::vec3(1, 0, 0));
    model = glm::translate(model, -pivot);
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    moriscaModel.Draw(depthMapShader);

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    myModel.Draw(depthMapShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);
}

void renderSkydome() {

    GLint oldDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);

    glDepthFunc(GL_LEQUAL);  
    glDepthMask(GL_TRUE);    

    myCustomShader.useShaderProgram();

    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isSkyDome"), 1);

    glm::mat4 view = myCamera.getViewMatrix();
    glm::mat4 skyView = glm::mat4(glm::mat3(view));  

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(retina_width) / static_cast<float>(retina_height),
        0.1f, 1000.0f);

    glm::mat4 skyModel = glm::mat4(1.0f);
    skyModel = glm::scale(skyModel, glm::vec3(0.001f));  

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(skyModel));
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
        1, GL_FALSE, glm::value_ptr(skyView));
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    skydomeModel.Draw(myCustomShader);

    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isSkyDome"), 0);

    glDepthFunc(oldDepthFunc);
}

void renderSceneWithShadows(glm::mat4 lightSpaceTrMatrix) {
    myCustomShader.useShaderProgram();

    // Setăm flag-ul pentru NU skydome
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "isSkyDome"), 0);

    // Matrice standard
    glm::mat4 view = myCamera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(retina_width) / static_cast<float>(retina_height),
        0.1f, 1000.0f);

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
        1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));

    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
        1, GL_FALSE, glm::value_ptr(lightSpaceTrMatrix));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 1);

    glActiveTexture(GL_TEXTURE0);

    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightsEnabled"),
        pointLightsEnabled ? 1 : 0);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointFogEnabled"),
        pointFogEnabled ? 1 : 0);

    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 cameraPos = myCamera.getCameraPosition();
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos"),
        1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor"),
        1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "viewPos"),
        1, glm::value_ptr(cameraPos));

    glm::vec3 lightPos2 = glm::vec3(-50.0f, 80.0f, -50.0f);
    glm::vec3 lightColor2 = glm::vec3(0.3f, 0.3f, 0.4f);
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2"),
        1, glm::value_ptr(lightPos2));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2"),
        1, glm::value_ptr(lightColor2));

    if (pointLightsEnabled) {
        glm::vec3 pointLightColor1 = glm::vec3(20.0f, 30.5f, 30.5f);
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos1"),
            1, glm::value_ptr(felinar1Pos));
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor1"),
            1, glm::value_ptr(pointLightColor1));

        float constant1 = 1.0f;
        float linear1 = 0.22f;
        float quadratic1 = 0.20f;
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightConstant1"), constant1);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLinear1"), linear1);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightQuadratic1"), quadratic1);

        glm::vec3 pointLightColor2 = glm::vec3(20.0f, 30.5f, 30.5f);
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos2"),
            1, glm::value_ptr(felinar2Pos));
        glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor2"),
            1, glm::value_ptr(pointLightColor2));

        float constant2 = 1.0f;
        float linear2 = 0.22f;
        float quadratic2 = 0.20f;
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightConstant2"), constant2);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightLinear2"), linear2);
        glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightQuadratic2"), quadratic2);
    }

    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "fogColor"),
        1, glm::value_ptr(fogColor));

    glm::vec3 pivot = glm::vec3(-41.6928f, 43.2391f, 61.8275f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pivot);
    model = glm::rotate(model, glm::radians(angleElice), glm::vec3(1, 0, 0));
    model = glm::translate(model, -pivot);
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    moriscaModel.Draw(myCustomShader);

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    myModel.Draw(myCustomShader);
}

void renderScene(glm::mat4 lightSpaceTrMatrix) {
    glViewport(0, 0, retina_width, retina_height);
    glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkydome();

    renderSceneWithShadows(lightSpaceTrMatrix);
}


void cleanup() {
    glDeleteBuffers(1, &verticesVBO);
    glDeleteBuffers(1, &verticesEBO);
    glDeleteVertexArrays(1, &objectVAO);

    if (rainVBO) glDeleteBuffers(1, &rainVBO);
    if (rainVAO) glDeleteVertexArrays(1, &rainVAO);
    if (shadowMapFBO) glDeleteFramebuffers(1, &shadowMapFBO);
    if (depthMapTexture) glDeleteTextures(1, &depthMapTexture);

    glfwDestroyWindow(glWindow);
    glfwTerminate();
}

int main(int argc, const char* argv[]) {
    if (!initOpenGLWindow()) {
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    depthMapShader.loadShader("shaders/shaderShadow.vert", "shaders/shaderShadow.frag");
    rainShader.loadShader("shaders/shaderRain.vert", "shaders/shaderRain.frag");

    printf("Loading scene without skydome...\n");
    myModel.LoadModel("objects/scene.obj");  

    printf("Loading skydome...\n");
    skydomeModel.LoadModel("objects/sk.obj");       

    printf("Loading windmill...\n");
    moriscaModel.LoadModel("objects/elicee.obj");        

    initFBO();
    initRain();

    myCustomShader.useShaderProgram();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        static_cast<float>(retina_width) / static_cast<float>(retina_height),
        0.1f, 1000.0f);

    GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(glWindow)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (cinematicCamera) {
            updateCinematicCamera(deltaTime);
        }
        else {
            processMovement();
        }


        glm::mat4 lightSpaceTrMatrix = computeLightSpaceTrMatrix();

        renderDepthMap(lightSpaceTrMatrix);

        renderScene(lightSpaceTrMatrix);

        if (rainEnabled) {
            updateRain(deltaTime);
            renderRain();
        }

        glfwPollEvents();  
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}