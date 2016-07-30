#pragma once

#include <GL/glew.h>

namespace gl {

class Buffer {
public:
	Buffer(GLenum target = GL_ARRAY_BUFFER);
	Buffer(const Buffer& other) = delete;
	Buffer& operator=(const Buffer& other) = delete;
	Buffer(Buffer&& other);
	Buffer& operator=(Buffer&& other);
	~Buffer();

	GLenum& target();
	const GLenum& target() const;
	void bind() const;

	void data(GLsizeiptr size, const GLvoid* data, GLenum usage);
	void subData(GLintptr offset, GLsizeiptr size, const GLvoid* data);

private:
	GLuint name_;
	GLenum target_;
};

}
