#include "3DViewer/line_painter.h"



namespace AI3D
{
    namespace GUI
    {

        LinePainter::LinePainter() : num_geoms_(0) {}

        LinePainter::~LinePainter() {
            vao_.destroy();
            vbo_.destroy();
        }

        void LinePainter::Setup() {
            vao_.destroy();
            vbo_.destroy();
            if (shader_program_.isLinked()) {
                shader_program_.release();
                shader_program_.removeAllShaders();
            }

            shader_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                ":/shaders/lines.v.glsl");
            shader_program_.addShaderFromSourceFile(QOpenGLShader::Geometry,
                ":/shaders/lines.g.glsl");
            shader_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                ":/shaders/lines.f.glsl");
            shader_program_.link();
            shader_program_.bind();

            vao_.create();
            vbo_.create();

#if DEBUG
            glDebugLog();
#endif
        }

        void LinePainter::Upload(const std::vector<LinePainter::Data>& data) {
            num_geoms_ = data.size();
            if (num_geoms_ == 0) {
                return;
            }

            vao_.bind();
            vbo_.bind();

            
            
            vbo_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
            vbo_.allocate(data.data(),
                static_cast<int>(data.size() * sizeof(LinePainter::Data)));

            
            shader_program_.enableAttributeArray(0);
            shader_program_.setAttributeBuffer(0, GL_FLOAT, 0, 3,
                sizeof(PointPainter::Data));

            
            shader_program_.enableAttributeArray(1);
            shader_program_.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 4,
                sizeof(PointPainter::Data));

            
            vbo_.release();
            vao_.release();

#if DEBUG
            glDebugLog();
#endif
        }

        void LinePainter::Render(const QMatrix4x4& pmv_matrix, const int width,
            const int height, const float line_width) {
            if (num_geoms_ == 0) {
                return;
            }

            shader_program_.bind();
            vao_.bind();

            shader_program_.setUniformValue("u_pmv_matrix", pmv_matrix);
            shader_program_.setUniformValue("u_inv_viewport",
                QVector2D(1.0f / width, 1.0f / height));
            shader_program_.setUniformValue("u_line_width", line_width);

            glDrawArrays(GL_LINES, 0, (GLsizei)(2 * num_geoms_));

            
            vao_.release();

#if DEBUG
            glDebugLog();
#endif
        }
    }
} 
