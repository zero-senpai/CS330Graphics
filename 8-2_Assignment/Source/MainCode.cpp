#include <GLFW/glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159f / 180.0f;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick {
public:
    float red, green, blue;
    float x, y, width;
    BRICKTYPE brick_type;
    ONOFF onoff;

    Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb) {
        brick_type = bt;
        x = xx;
        y = yy;
        width = ww;
        red = rr;
        green = gg;
        blue = bb;
        onoff = ON;
    }

    void drawBrick() {
        if (onoff == ON) {
            double halfside = width / 2;

            glColor3d(red, green, blue);
            glBegin(GL_POLYGON);
            glVertex2d(x + halfside, y + halfside);
            glVertex2d(x + halfside, y - halfside);
            glVertex2d(x - halfside, y - halfside);
            glVertex2d(x - halfside, y + halfside);
            glEnd();
        }
    }
};
/*
*   Obj Paddle - Controllable paddle at bottom of the view that collides with circless
*/
class Paddle {
public:
    float x, y, width, height, speed;

    /*
    * Constructor, customize the call with float vallues for width, height, etc.
    */
    Paddle(float xx, float yy, float ww, float hh, float sp) {
        x = xx;
        y = yy;
        width = ww;
        height = hh;
        speed = sp;
    }

    void drawPaddle() {
        glColor3f(1, 1, 1);  // White paddle, change value for diff colors
        glBegin(GL_QUADS);
        glVertex2f(x - width / 2, y - height / 2);
        glVertex2f(x + width / 2, y - height / 2);
        glVertex2f(x + width / 2, y + height / 2);
        glVertex2f(x - width / 2, y + height / 2);
        glEnd();
    }

    /*
    * The moveLeft and moveRight functions control the speed for rendering movement
    */
    void moveLeft() {
        if (x - width / 2 > -1) {
            x -= speed;
        }
    }

    void moveRight() {
        if (x + width / 2 < 1) {
            x += speed;
        }
    }
};

/*
*   obj Circle - circle shape of random color created during processor event (space key pressed)
*/
class Circle {
public:
    float red, green, blue;
    float radius;
    float x, y;
    float speed = 0.03f;
    int direction;

    Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b) {
        x = xx;
        y = yy;
        radius = rr;
        red = r;
        green = g;
        blue = b;
        radius = rad;
        direction = dir;
    }

    void checkCollisionWithBrick(Brick* brk) {
        if (brk->onoff == ON &&
            x > brk->x - brk->width / 2 && x < brk->x + brk->width / 2 &&
            y > brk->y - brk->width / 2 && y < brk->y + brk->width / 2) {
            direction = getRandomDirection();
            if (brk->brick_type == DESTRUCTABLE) {
                brk->onoff = OFF;
            }
        }
    }

    void checkCollisionWithPaddle(Paddle* paddle) {
        if (y - radius <= paddle->y + paddle->height / 2 &&
            x >= paddle->x - paddle->width / 2 &&
            x <= paddle->x + paddle->width / 2) {
            direction = 1;  // Reflect upward
        }
    }

    void moveOneStep() {
        if (direction == 1 || direction == 5 || direction == 6) {  // Up
            if (y < 1 - radius) {
                y += speed;
            }
            else {
                direction = getRandomDirection();
            }
        }
        if (direction == 2 || direction == 5 || direction == 7) {  // Right
            if (x < 1 - radius) {
                x += speed;
            }
            else {
                direction = getRandomDirection();
            }
        }
        if (direction == 3 || direction == 7 || direction == 8) {  // Down
            if (y > -1 + radius) {
                y -= speed;
            }
            else {
                direction = getRandomDirection();
            }
        }
        if (direction == 4 || direction == 6 || direction == 8) {  // Left
            if (x > -1 + radius) {
                x -= speed;
            }
            else {
                direction = getRandomDirection();
            }
        }
    }

    void drawCircle() {
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * DEG2RAD;
            glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
        }
        glEnd();
    }

    int getRandomDirection() {
        return (rand() % 8) + 1;
    }
};

vector<Circle> world;
Paddle paddle(0.0f, -0.8f, 0.3f, 0.05f, 0.05f);  // Paddle initialization

int main(void) {
    srand(time(NULL));

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(480, 480, "Pong with Bricks - Jake Brunton", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Create bricks
    Brick brick(REFLECTIVE, 0.5, -0.33, 0.2, 1, 1, 0);
    Brick brick2(DESTRUCTABLE, -0.5, 0.33, 0.2, 0, 1, 0);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        // Move and draw circles
        for (Circle& circle : world) {
            circle.checkCollisionWithBrick(&brick);
            circle.checkCollisionWithBrick(&brick2);
            circle.checkCollisionWithPaddle(&paddle);
            circle.moveOneStep();
            circle.drawCircle();
        }

        // Draw paddle and bricks
        paddle.drawPaddle();
        brick.drawBrick();
        brick2.drawBrick();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        paddle.moveLeft();

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        paddle.moveRight();

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        double r = rand() / (double)RAND_MAX;
        double g = rand() / (double)RAND_MAX;
        double b = rand() / (double)RAND_MAX;
        Circle newCircle(0, 0, 0.02, 2, 0.05, r, g, b);
        world.push_back(newCircle);
    }
}
