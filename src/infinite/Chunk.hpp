#pragma once

#include <mathfu/vector.h>

#include "gl/Buffer.hpp"

namespace infinite {

class Chunk {
public:
    Chunk();
    
    unsigned char& operator()(int x, int y);
    unsigned char operator()(int x, int y) const;

    void fillGLBuffer(gl::Buffer& verts,
                      mathfu::Vector<float, 2> xRange,
                      mathfu::Vector<float, 2> yRange,
                      mathfu::Vector<float, 2> zRange) const;

    static unsigned fillGridIndices(gl::Buffer& glShortIndices);
    static unsigned fillTriangleFanIndices(gl::Buffer& glShortIndices);
private:
    unsigned char heights_[16][16];
};

}
