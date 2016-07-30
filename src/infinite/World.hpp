#pragma once

#include "infinite/Chunk.hpp"

#include "gl/VArray.hpp"
#include "gl/Buffer.hpp"
#include "gl/Program.hpp"

namespace infinite {

class World {
public:
    World();
    void simulate(double units);
    void render(gl::Program& p) const;

private:
    gl::Buffer chunks_[16];
    Chunk lastChunk_;
    mutable gl::VArray vao_;
    gl::Buffer lines_;
    gl::Buffer triangles_;
    double offset_;
    double hue_;
    int start_;
};

}
