#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 1920
#define HEIGHT 1080

unsigned char rgbBuffer[WIDTH * HEIGHT * 3];

const char* getCharRed(const char* chr) {
    static char buffer[32];
    sprintf(buffer, "\033[31m%s\033[0m", chr);
    return buffer;
}

const char* getCharBlue(const char* chr) {
    static char buffer[32];
    sprintf(buffer, "\033[34m%s\033[0m", chr);
    return buffer;
}

struct vector {
    float x;
    float y;
    float z;
};

const char* character1;
const char* character2;
const char* pixels[WIDTH][HEIGHT];
float offsetX = 0;
float offsetY = 0;
float offsetZ = 0;
float cameraAngleX = 0;
float cameraAngleY = 0;
float cameraAngleZ = 0;
float cameraDistance = 100;
float scale = 500;

int r = 0;
int g = 0;
int b = 0;

void swap(int x, int y) {
    const char* current = pixels[x][y];
    if (current == character1) {
        pixels[x][y] = character2;
    } else {
        pixels[x][y] = character1;
    }
}

void saveFrameAsImage(int frameNumber) {
    char filename[64];
    sprintf(filename, "frame_%04d.bmp", frameNumber);  
    stbi_write_bmp(filename, WIDTH, HEIGHT, 3, rgbBuffer);
}


void drawLine(int x0, int y0, int x1, int y1) {
    int steep = (abs(y1 - y0) > abs(x1 - x0));
    if (steep) {
        int t = x0;  x0 = y0;  y0 = t;  t = x1;  x1 = y1;  y1 = t;
    }
    if (x0 > x1) {
        int t = x0;  x0 = x1;  x1 = t;  t = y0;  y0 = y1;  y1 = t;
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int err = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            if (x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH) {
                swap(y, x);
            }
        } else {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                swap(x, y);
            }
        }
        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
}

void draw() {
    struct vector vertices[] = {
        {offsetX - 0.5f, offsetY - 0.5f, offsetZ - 0.5f},
        {offsetX - 0.5f, offsetY - 0.5f, offsetZ + 0.5f},
        {offsetX - 0.5f, offsetY + 0.5f, offsetZ - 0.5f},
        {offsetX - 0.5f, offsetY + 0.5f, offsetZ + 0.5f},
        {offsetX + 0.5f, offsetY - 0.5f, offsetZ - 0.5f},
        {offsetX + 0.5f, offsetY - 0.5f, offsetZ + 0.5f},
        {offsetX + 0.5f, offsetY + 0.5f, offsetZ - 0.5f},
        {offsetX + 0.5f, offsetY + 0.5f, offsetZ + 0.5f}
    };
    size_t numElements = sizeof(vertices) / sizeof(vertices[0]);

    struct vector projectedVertices[8];
    float offsetCanvasX = WIDTH / 2.0f;
    float offsetCanvasY = HEIGHT / 2.0f;
    int i = 0;
    while (i < numElements) {
        float x = vertices[i].x;
        float y = vertices[i].y;
        float z = vertices[i].z;

        float tmpY = y * cosf(cameraAngleX) - z * sinf(cameraAngleX);
        float tmpZ = y * sinf(cameraAngleX) + z * cosf(cameraAngleX);

        float tmpX = x * cosf(cameraAngleY) + tmpZ * sinf(cameraAngleY);
        tmpZ = -x * sinf(cameraAngleY) + tmpZ * cosf(cameraAngleY);

        x = tmpX * cosf(cameraAngleZ) - tmpY * sinf(cameraAngleZ);
        y = tmpX * sinf(cameraAngleZ) + tmpY * cosf(cameraAngleZ);

        float perspective = cameraDistance / (tmpZ + cameraDistance);
        projectedVertices[i].x = x * perspective * scale + offsetCanvasX;
        projectedVertices[i].y = y * perspective * scale + offsetCanvasY;
        i++;
    }

    int edges[][2] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0},
        {4, 5}, {5, 7}, {7, 6}, {6, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
    int marker = 0;
    numElements = sizeof(edges) / sizeof(edges[0]);
    while (marker < numElements) {
        int x1 = projectedVertices[edges[marker][0]].x - 0;
        int y1 = projectedVertices[edges[marker][0]].y + 0;
        int x2 = projectedVertices[edges[marker][1]].x - 0;
        int y2 = projectedVertices[edges[marker][1]].y + 0;
        drawLine(x1, y1, x2, y2);
        marker++;
    }
}

void resetCanvas() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            pixels[x][y] = character1;
            int index = (y * WIDTH + x) * 3;
            rgbBuffer[index + 0] = 0;
            rgbBuffer[index + 1] = 0;
            rgbBuffer[index + 2] = 0;
        }
    }
}

void printPixels() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            const char* pixel = pixels[x][y];
            int index = (y * WIDTH + x) * 3;
            if (pixel == character2) {
                rgbBuffer[index + 0] = r;
                rgbBuffer[index + 1] = g;
                rgbBuffer[index + 2] = b;
            }
        }
    }
}

void deleteLastLines(int numLines) {
    for (int i = 0; i < numLines; i++) {
        printf("\033[A");
        printf("\033[2K");
    }
}

int main() {
    character1 = getCharBlue("\u00A0");
    character2 = getCharRed("0");
    resetCanvas();

    struct timeval start, end;
    int frameNumber = 0;



    r = 255;
    while (frameNumber < 250) {
        gettimeofday(&start, NULL);

        cameraAngleX += 0.01f;
        cameraAngleY += 0.02f;
        cameraAngleZ += 0.03f;
        //resetCanvas();
        draw();
        printPixels();
        saveFrameAsImage(frameNumber);

        gettimeofday(&end, NULL);
        frameNumber++;
    }

    r = 0;
    b = 255;
    while (frameNumber < 500) {
        gettimeofday(&start, NULL);

        cameraAngleX += 0.01f;
        cameraAngleY += 0.02f;
        cameraAngleZ += 0.03f;
        //resetCanvas();
        draw();
        printPixels();
        saveFrameAsImage(frameNumber);

        gettimeofday(&end, NULL);
        frameNumber++;
    }

    b = 0;
    r = 0;
    b = 0;
    int num = frameNumber;
    while (frameNumber < 1000) {
        gettimeofday(&start, NULL);

        cameraAngleX -= 0.01f;
        cameraAngleY -= 0.02f;
        cameraAngleZ -= 0.03f;
        //resetCanvas();
        draw();
        printPixels();
        saveFrameAsImage(num);

        gettimeofday(&end, NULL);
        frameNumber++;
        gettimeofday(&start, NULL);

        cameraAngleX -= 0.01f;
        cameraAngleY -= 0.02f;
        cameraAngleZ -= 0.03f;
        //resetCanvas();
        draw();
        printPixels();

        gettimeofday(&end, NULL);
        frameNumber++;
        num++;
    }


    return 0;
}
