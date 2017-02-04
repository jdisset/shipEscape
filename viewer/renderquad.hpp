#ifndef RENDERQUAD_HPP
#define RENDERQUAD_HPP
#include "primitives/quad.hpp"
#include "extern.h"

class RenderQuad {
	QOpenGLShaderProgram shader;
	Quad quad;

 public:
	RenderQuad(){};
	void load(const QString &vs, const QString &fs) {
		shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vs);
		shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fs);
		shader.link();
		quad.load(shader);
	}

	void draw(const QMatrix4x4 &model, const QMatrix4x4 &view, const QVector3D &color1, const QVector3D &color2) {
		shader.bind();
		quad.vao.bind();
		shader.setUniformValue(shader.uniformLocation("model"), model);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("color1"), color1);
		shader.setUniformValue(shader.uniformLocation("color2"), color2);
		GL->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quad.vao.release();
		shader.release();
	}
	void draw(const QMatrix4x4 &model, const QMatrix4x4 &view, const QVector3D &color) {
		shader.bind();
		quad.vao.bind();
		shader.setUniformValue(shader.uniformLocation("model"), model);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("color"), color);
		GL->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quad.vao.release();
		shader.release();
	}
	void draw(GLuint tex, const QMatrix4x4 &model, const QMatrix4x4 &view,
	          const QVector3D &addedColor) {
		shader.bind();
		quad.vao.bind();
		GL->glActiveTexture(GL_TEXTURE0);
		GL->glBindTexture(GL_TEXTURE_2D, tex);
		shader.setUniformValue(shader.uniformLocation("tex"), 0);
		shader.setUniformValue(shader.uniformLocation("model"), model);
		shader.setUniformValue(shader.uniformLocation("view"), view);
		shader.setUniformValue(shader.uniformLocation("addedColor"), addedColor);
		GL->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		quad.vao.release();
		shader.release();
	}
};
#endif
