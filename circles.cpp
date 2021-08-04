#include <vector>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/vector_angle.hpp>

class Circle {
public:
    glm::vec3 center;
    double radius;
};

class Collision {
public:
    Circle *a, *b;
    glm::vec3 impulse;
    Collision(Circle* _a, Circle* _b) : a(_a), b(_b), 
        impulse(glm::mat3(b->radius + a->radius) * normalize(b->center - a->center) - b->center + a->center) {}
};

std::vector<Collision> find_collisions(const std::vector<Circle*>& circles) {
    std::vector<Collision> collisions;
    for (int i = 0; i < circles.size(); i ++)
        for (int j = 0; j < i; j ++)
            if (length(circles[i]->center - circles[j]->center) < circles[i]->radius + circles[j]->radius)
                collisions.push_back(Collision(circles[i], circles[j]));
    
    return collisions;
}

class Graphics {
public:
    unsigned int VAO, verticesVBO, radiusVBO;
    unsigned int shaderProgram;
    GLFWwindow* window;
};


// from https://learnopengl.com/

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in float aRadius;\n"
    "void main()\n"
    "{\n"
    "   gl_PointSize = aRadius;\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "\n"
    "void main() {"
    "vec2 offset = vec2(0.5, 0.5);\n"
    "if (length(gl_PointCoord.xy - offset) > 0.5) discard;\n"
    "gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\0";

Graphics* graphics_init() {
    Graphics* gg = new Graphics();
    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    gg->window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (gg->window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(gg->window);
//    glfwSetFramebufferSizeCallback(gg->window, [](GLFWwindow* gw, int w, int h) -> void { glViewport(0, 0, w, h); });

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    gg->shaderProgram = glCreateProgram();
    glAttachShader(gg->shaderProgram, vertexShader);
    glAttachShader(gg->shaderProgram, fragmentShader);
    glLinkProgram(gg->shaderProgram);
    // check for linking errors
    glGetProgramiv(gg->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(gg->shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    glGenVertexArrays(1, &(gg->VAO));
    glGenBuffers(1, &(gg->verticesVBO));
    glGenBuffers(1, &(gg->radiusVBO));
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    return gg;
}

void graphics_draw(Graphics* gg, float vertices[], float radiuses[], int size) {
    glBindVertexArray(gg->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, gg->verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * size, vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, gg->radiusVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * size, radiuses, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    
    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

  
    glUseProgram(gg->shaderProgram);
    glBindVertexArray(gg->VAO);
    glDrawArrays(GL_POINTS, 0, size);
    glBindVertexArray(0); 

    glfwSwapBuffers(gg->window);
    glfwPollEvents();
}

class Event {
public:
    virtual ~Event() = default;
};

class CloseEvent : public Event {
};

class DragEvent : public Event {
    double x, y;
    public:
    DragEvent(double _x, double _y) : x(_x), y(_y) {}
    double get_x() { return x; }
    double get_y() { return y; }
};

Event* graphics_event(Graphics* gg) {
    if (glfwGetKey(gg->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(gg->window, true);
        
    if (glfwWindowShouldClose(gg->window))
        return new CloseEvent;
        
    if (glfwGetMouseButton(gg->window, GLFW_MOUSE_BUTTON_LEFT == GLFW_PRESS)) {
        double xpos, ypos;
        glfwGetCursorPos(gg->window, &xpos, &ypos);
        int height, width;
        glfwGetWindowSize(gg->window, &width, &height);
        return new DragEvent(2 * (xpos / width) - 1, 2 * (-ypos / height) + 1);
    }
     
    return new Event;   
    
}

void graphics_deinit(Graphics* gg) {
    glDeleteVertexArrays(1, &(gg->VAO));
    glDeleteBuffers(1, &(gg->verticesVBO));
    glDeleteBuffers(1, &(gg->radiusVBO));
    glDeleteProgram(gg->shaderProgram);

    glfwTerminate();
}


int main() {
    std::vector<Circle*> circles;
    
    Circle* selected_circle;
    bool is_drawing = false;
    glm::vec3 drawing_start;
    glm::vec3 moving_bias;
    
    
    Graphics* gg = graphics_init();
    
    bool finished = false;
    while (!finished) {
        float vertices[3 * circles.size()] = {};
        float radius[circles.size()] = {};
        for (int i = 0; i < circles.size(); i ++) {
            vertices[3 * i + 0] = circles[i]->center.x;
            vertices[3 * i + 1] = circles[i]->center.y;
            vertices[3 * i + 2] = circles[i]->center.z;
            radius[i] = 1.0 * SCR_WIDTH * circles[i]->radius;
        }
        
        graphics_draw(gg, vertices, radius, circles.size());
        auto event = graphics_event(gg);
        finished = dynamic_cast<CloseEvent*>(event) != nullptr;
        if (dynamic_cast<DragEvent*>(event) != nullptr) {
            DragEvent& drag_event = *(dynamic_cast<DragEvent*>(event));
            glm::vec3 p = glm::vec3(drag_event.get_x(), drag_event.get_y(), 0);
            if (selected_circle != nullptr) {
                if (is_drawing) {
                    selected_circle->radius = glm::length(p - drawing_start) / 2.0;
                    selected_circle->center = glm::mat3(0.5) * (p + drawing_start);
                } else {
                    selected_circle->center += p - selected_circle->center - moving_bias;
                }
            } else {
                bool found = false;
                for (const auto& c : circles)
                    if (glm::length(p - c->center) <= c->radius) {
                        found = true;
                        selected_circle  = c;
                        is_drawing = false;
                        moving_bias = p - c->center;
                    }
                if (!found) {
                    selected_circle = new Circle();
                    circles.push_back(selected_circle);
                    selected_circle->radius = 0;
                    selected_circle->center = p;
                    is_drawing = true;
                    drawing_start = p;
                }
                std::cout << "DEBUG :: " << "selected circle : " << selected_circle << " is_drawing " << is_drawing << std::endl;
            }
        } else {
            selected_circle = nullptr;
        }
        delete event;
        
        for (const auto& c: find_collisions(circles)) {
            if (selected_circle == c.a)
                c.b->center += c.impulse;
            else if (selected_circle == c.b)
                c.a->center -= c.impulse;
            else {
                c.a->center -= glm::mat3(0.5) * c.impulse;
                c.b->center += glm::mat3(0.5) * c.impulse;
            }
        }
    }
    graphics_deinit(gg);
    delete gg;
    
    for (const auto& c : circles) {
        delete c;
    }
    return 0;
}


