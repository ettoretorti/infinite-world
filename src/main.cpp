#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <stdexcept>

#include <mathfu/glsl_mappings.h>

#include "gl/Shader.hpp"
#include "gl/Program.hpp"

#include "infinite/World.hpp"

const char* vshaderSrc =
    "#version 330\n"
    ""
    "uniform mat4 model;\n"
    "uniform mat4 viewProjection;\n"
    ""
    "in vec4 pos;\n"
    "in vec4 color;\n"
    ""
    "out vec4 fColor;\n"
    ""
    "void main() {\n"
    "  gl_Position = viewProjection * model * pos;\n"
    "  fColor = color;\n"
    "}\n";

const char* fshaderSrc =
    "#version 330\n"
    ""
    "in vec4 fColor;\n"
    "out vec4 color;\n"
    ""
    "void main() {\n"
    "  color = fColor;\n"
    "}\n";


void glfw_error(int error, const char* msg) {
    std::cerr << "GLFW error with code: " << error << std::endl;
    std::cerr << msg << std::endl;
}

#ifndef NDEBUG
void APIENTRY gl_error(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* uParam) {
    std::cerr << msg << std::endl;
}
#endif

/*!
 * Preconditions
 * - None
 *
 * Postconditions
 * - glew has been initialized
 * - a valid opengl context has been created and made current
 * - a valid window is returned
 */
GLFWwindow* init(int width, int height) {
    glfwSetErrorCallback(glfw_error);

    if(!glfwInit()) return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 8);  // 8xMSAA

    GLFWwindow* win = glfwCreateWindow(width, height, "Infinite World", nullptr, nullptr);
    if (!win) return nullptr;

    glfwMakeContextCurrent(win);
    glEnable(GL_MULTISAMPLE);

    // make sure we get what we want
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if(glewErr != GLEW_OK) {
        glfwDestroyWindow(win);
        return nullptr;
    }

#ifndef NDEBUG
    if (GLEW_KHR_debug) {
        glDebugMessageCallback(gl_error, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        std::cout << "Registered gl debug callback" << std::endl;
    }
#endif

    return win;
}

void shutdown() {
    glfwTerminate();
}

void mainloop(GLFWwindow* win, gl::Program& p) {
    using namespace gl;
    using namespace mathfu;
    
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glEnable(GL_DEPTH_TEST);

    vec2i size;
    glfwGetFramebufferSize(win, &size.x(), &size.y());
    glViewport(0, 0, size.x(), size.y());

    vec2i windowedSize(size.x(), size.y());
    vec2i windowedPos;
    glfwGetWindowPos(win, &windowedPos.x(), &windowedPos.y());

    mat4 mdl  = mat4::Identity();
    mat4 view = mat4::LookAt(mathfu::vec3(0.0, 0.0, -1000.0),
                             mathfu::vec3(0.0, 5.0, 0.0),
                             mathfu::vec3(0.0, 1.0, 0.0), 1.0);
    mat4 proj = mat4::Perspective(60.0 * 3.14159 / 180.0, size.x()/(double)size.y(), 0.01, 256.0);

    glUniformMatrix4fv(p.getUniform("model"), 1, false, &mdl[0]);
    glUniformMatrix4fv(p.getUniform("viewProjection"), 1, false, &(proj * view)[0]);

    infinite::World wo;

    bool running    = false;
    bool pPressed   = false;
    bool fullscreen = false;
    bool fPressed   = false;
    bool clone      = false;
    bool cPressed   = false;
    bool upsideDown = false;
    bool dPressed   = false;
    double time = glfwGetTime();
    while(!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        if(glfwGetKey(win, GLFW_KEY_ESCAPE)) { break; }

        bool pNow = glfwGetKey(win, GLFW_KEY_P);
        if(!pPressed && pNow) running = !running;
        pPressed = pNow;

        bool cNow = glfwGetKey(win, GLFW_KEY_C);
        if(!cPressed && cNow) clone = !clone;
        cPressed = cNow;
        
        bool dNow = glfwGetKey(win, GLFW_KEY_D);
        if(!dPressed && dNow) upsideDown = !upsideDown;
        dPressed = dNow;
        
        bool fNow = glfwGetKey(win, GLFW_KEY_F);
        if(!fPressed && fNow) {
            if(fullscreen) {
                glfwSetWindowMonitor(win, nullptr, windowedPos.x(), windowedPos.y(), windowedSize.x(), windowedSize.y(), 0);
            } else {
                windowedSize = size;
                glfwGetWindowPos(win, &windowedPos.x(), &windowedPos.y());

                const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
                glfwSetWindowMonitor(win, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            }

            fullscreen = !fullscreen;
        }
        fPressed = fNow;
        
        vec2i newSize;
        glfwGetFramebufferSize(win, &newSize.x(), &newSize.y());
        if(size != newSize) {
            size = newSize;
            glViewport(0, 0, size.x(), size.y());
            proj = mat4::Perspective(60.0 * 3.14159 / 180.0, size.x()/(double)size.y(), 2.0, 160.0);
        }

        double curTime = glfwGetTime();
        if(running) {
            wo.simulate(16 * (curTime - time));
        } else {
            glfwWaitEvents();
            curTime = glfwGetTime();
        }
        time = curTime;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(!upsideDown || clone) {
            view = mat4::LookAt(mathfu::vec3(0.0, 0.0, -1000.0),
                                 mathfu::vec3(0.0, 5.0, 0.0),
                                 mathfu::vec3(0.0, 1.0, 0.0), 1.0);
            glUniformMatrix4fv(p.getUniform("viewProjection"), 1, false, &(proj * view)[0]);
            wo.render(p);
        }

        if(upsideDown || clone) {
            view = mat4::LookAt(mathfu::vec3(0.0, 0.0, -1000.0),
                                 mathfu::vec3(0.0, 5.0, 0.0),
                                 mathfu::vec3(0.0, -1.0, 0.0), 1.0);
            glUniformMatrix4fv(p.getUniform("viewProjection"), 1, false, &(proj * view)[0]);
            wo.render(p);
        }
        glfwSwapBuffers(win);
    }
}

int main(int argc, char** argv) {
    using namespace std;
    using namespace gl;

    int width = 640, height = 480;

    if(argc >= 3) {
        try {
            int w = stoi(argv[1]);
            int h = stoi(argv[2]);

            width  = w;
            height = h;
        } catch(...) {
            cerr << "Could not parse dimensions from first 2 command line args" << endl;
        }
    }

    GLFWwindow* win = init(width, height);
    if (!win) {
        cerr << "Could not create window" << endl;
        return -1;
    }

    Shader vs(GL_VERTEX_SHADER);
    vs.source(vshaderSrc);
    if(!vs.compile()) {
        cerr << "Could not compile vertex shader\n\n" << vs.infoLog() << endl;
        return -1;
    }

    Shader fs(GL_FRAGMENT_SHADER);
    fs.source(fshaderSrc);
    if(!fs.compile()) {
        cerr << "Could not compile fragment shader\n\n" << fs.infoLog() << endl;
        return -1;
    }

    Program p;
    p.setVertexShader(vs);
    p.setFragmentShader(fs);
    p.bindAttrib(0, "pos");
    p.bindAttrib(1, "color");
    if(!p.link()) {
        cerr << "Could not link program\n\n" << p.infoLog() << endl;
        return -1;
    }
    p.use();

    try {
        mainloop(win, p);
    } catch(exception& ex) {
        cerr << ex.what() << endl;
    }

    glfwDestroyWindow(win);
    shutdown();

    return 0;
}
