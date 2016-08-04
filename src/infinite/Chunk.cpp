#include "infinite/Chunk.hpp"

#include <chrono>
#include <random>
#include <functional>

#include <mathfu/vector.h>

#include "gl/Buffer.hpp"

namespace infinite {

static thread_local std::default_random_engine re(std::chrono::high_resolution_clock::now().time_since_epoch().count());

//unsigned short instead of unsigned char because MSVC fails to compile it otherwise
static thread_local std::uniform_int_distribution<unsigned short> ud(0, 128);
static thread_local std::binomial_distribution<unsigned short> bd(127, 0.5);
static thread_local auto uniform = std::bind(ud, re);
static thread_local auto binomial = std::bind(bd, re);

Chunk::Chunk() {
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            heights_[i][j] = (unsigned char) (uniform() + binomial());
        }
    }
}

unsigned char& Chunk::operator()(int x, int y) {
    return heights_[x][y];
}

unsigned char Chunk::operator()(int x, int y) const {
    return heights_[x][y];
}

void Chunk::fillGLBuffer(gl::Buffer& verts,
                         mathfu::Vector<float, 2> xRange,
                         mathfu::Vector<float, 2> yRange,
                         mathfu::Vector<float, 2> zRange) const {
    static thread_local GLfloat buffer[3 * 16 * 16];

    int cur = 0;
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            buffer[cur + 0] = (xRange[1] - xRange[0]) * i / 15.0 + xRange[0];
            buffer[cur + 1] = (yRange[1] - yRange[0]) * heights_[i][j] / 255.0 + yRange[0];
            buffer[cur + 2] = (zRange[1] - zRange[0]) * j / 15.0 + zRange[0];
            
            cur += 3;
        }
    }

    verts.data(3 * 16 * 16 * sizeof(GLfloat), buffer, GL_STATIC_DRAW);
}

unsigned Chunk::fillGridIndices(gl::Buffer& glShortIndices) {
    //32 lines of length 16 each, with 32 primitive restarts between them
    GLushort* buffer = new GLushort[32 * 16 + 32];
    int cur = 0;

    //vertical lines, -1 is for primitive restart
    for(int i = 0, count = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            buffer[cur++] = count++;
        }

        buffer[cur++] = -1;
    }

    //horizontal lines
    for(int j = 0; j < 16; j++) {
        for(int i = 0; i < 16; i++) {
            buffer[cur++] = 16 * i + j;
        }

        buffer[cur++] = -1;
    }

    glShortIndices.data((32 * 16 + 31) * sizeof(GLushort), buffer, GL_STATIC_DRAW);

    delete[] buffer;

    return 32 * 16 + 31;
}

unsigned Chunk::fillTriangleFanIndices(gl::Buffer& glShortIndices) {
    GLushort* buffer = new GLushort[(16 * 15 * 2) + 15];
    int cur = 0;

    for(int i = 0, base = 0; i < 15; i++, base += 16) {
        for(int j = 0; j < 16; j++) {
            buffer[cur++] = base + j;
            buffer[cur++] = base + j + 16;
        }

        //primitive restart
        buffer[cur++] = -1;
    }

    glShortIndices.data(((16 * 15 * 2) + 14) * sizeof(GLushort), buffer, GL_STATIC_DRAW);

    delete[] buffer;

    return 16 * 15 * 2 + 14;
}

}
