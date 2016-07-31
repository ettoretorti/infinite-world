#include "infinite/World.hpp"

#include <cmath>

#include <mathfu/glsl_mappings.h>

#include "infinite/Chunk.hpp"

#include "gl/Buffer.hpp"
#include "gl/Program.hpp"

namespace infinite {

World::World()
    : lastChunk_(),
      vao_(),
      lines_(GL_ELEMENT_ARRAY_BUFFER),
      triangles_(GL_ELEMENT_ARRAY_BUFFER),
      offset_(0.0), hue_(0.0), start_(0)
{
    Chunk::fillGridIndices(lines_);
    Chunk::fillTriangleFanIndices(triangles_);
    
    for(int i = 0; i < 16; i++) {
        Chunk curChunk;
        for(int j = 0; j < 16; j++) {
            curChunk(j, 15) = lastChunk_(j, 0);
        }

        curChunk.fillGLBuffer(chunks_[i],
                              {-8.0,8.0},
                              {0.0, 4.0},
                              {-8.0,8.0});
        lastChunk_ = curChunk;
    }
}

static void hue2RGB(float h, float& r, float& g, float &b) {
    //copied and adapted from http://www.easyrgb.com/index.php?X=MATH&H=21#text21

    h *= 6;
    int i = (int) h;   //floor        
    float v3 = h - i;
    float v2 = 1 - v3;

    if      ( i == 0 ) { r = 1.0; g = v3;  b = 0.0; }
    else if ( i == 1 ) { r = v2;  g = 1.0; b = 0.0; }
    else if ( i == 2 ) { r = 0.0; g = 1.0; b = v3;  }
    else if ( i == 3 ) { r = 0.0; g = v2;  b = 1.0; }
    else if ( i == 4 ) { r = v3;  g = 0.0; b = 1.0; }
    else                   { r = 1.0; g = 0.0; b = v2;  }
}

void World::simulate(double units) {
    offset_ += units;
    hue_ = std::fmod(hue_ + units / 256.0, 1.0);

    while(offset_ >= 8.0) {
        offset_ -= 16.0;


        Chunk newChunk;
        for(int i = 0; i < 16; i++) {
            newChunk(i, 15) = lastChunk_(i, 0);
        }

        newChunk.fillGLBuffer(chunks_[start_],
                              {-8.0,8.0},
                              {0.0, 4.0},
                              {-8.0,8.0});
        lastChunk_ = newChunk;

        start_ = (start_ + 1) % 16;
    }
}

void World::render(gl::Program& p) const {
    using namespace mathfu;
    
    GLint mdlLoc = p.getUniform("model");
    mat4 mdl = mat4::FromTranslationVector(vec3(0.0, 0.0, offset_));
    
    vao_.bind();
    vao_.enableVertexAttrib(0);
    
    //grey
    glVertexAttrib4fv(1, &vec4(0.1, 0.1, 0.1, 1.0)[0]);

    //draw triangles
    triangles_.bind();
    for(int i = start_, j = 0; j < 16; i = (i + 1) % 16, j++) {
        glUniformMatrix4fv(mdlLoc, 1, false, &mdl[0]);

        chunks_[i].bind();
        vao_.vertexAttribPointer(0, 3, GL_FLOAT);
        glDrawElements(GL_TRIANGLE_STRIP, (16 * 15 * 2) + 14, GL_UNSIGNED_SHORT, nullptr);

        mdl *= mat4::FromTranslationVector(vec3(0.0, 0.0, -16.0));
    }

    //offset lines slightly to avoid z-fighting
    mdl = mat4::FromTranslationVector(vec3(0.0, 0.01, offset_));

    //hue'ed
    vec4 color;
    hue2RGB(hue_, color[0], color[1], color[2]);
    color[3] = 1.0;

    glVertexAttrib4fv(1, &color[0]);
    
    //draw grid lines
    lines_.bind();
    for(int i = start_, j = 0; j < 16; i = (i + 1) % 16, j++) {
        glUniformMatrix4fv(mdlLoc, 1, false, &mdl[0]);

        chunks_[i].bind();
        vao_.vertexAttribPointer(0, 3, GL_FLOAT);
        glDrawElements(GL_LINE_STRIP, (32 * 16) + 31, GL_UNSIGNED_SHORT, nullptr);

        mdl *= mat4::FromTranslationVector(vec3(0.0, 0.0, -16.0));
    }
}

}
