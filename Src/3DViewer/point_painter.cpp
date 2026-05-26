#include "3DViewer/point_painter.h"



namespace AI3D
{
    namespace GUI
    {

        PointPainter::PointPainter() : num_geoms_(0) {}

        PointPainter::~PointPainter() {
            vao_.destroy();
            vbo_.destroy();
        }

        void PointPainter::Setup() {
            vao_.destroy();
            vbo_.destroy();
            if (shader_program_.isLinked()) {
                shader_program_.release();
                shader_program_.removeAllShaders();
            }

            shader_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                ":/shaders/points.v.glsl");
            shader_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                ":/shaders/points.f.glsl");
            shader_program_.link();
            shader_program_.bind();

            vao_.create();
            vbo_.create();

#if DEBUG
            glDebugLog();
#endif
        }

        void PointPainter::Upload(const std::vector<PointPainter::Data>& data) {
            num_geoms_ = data.size();
            if (num_geoms_ == 0) {
                return;
            }

            vao_.bind();
            vbo_.bind();

            
            vbo_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
            vbo_.allocate(data.data(),
                static_cast<int>(data.size() * sizeof(PointPainter::Data)));

            
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

        void PointPainter::Render(const QMatrix4x4& pmv_matrix,
            const float point_size) {
            if (num_geoms_ == 0) {
                return;
            }

            shader_program_.bind();
            vao_.bind();

            shader_program_.setUniformValue("u_pmv_matrix", pmv_matrix);
            shader_program_.setUniformValue("u_point_size", point_size);

            glDrawArrays(GL_POINTS, 0, (GLsizei)num_geoms_);

            
            vao_.release();

#if DEBUG
            glDebugLog();
#endif
        }
    }
} 
