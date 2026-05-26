#ifndef COLMAP_SRC_UI_MODEL_VIEWER_WIDGET_H_
#define COLMAP_SRC_UI_MODEL_VIEWER_WIDGET_H_

#include <QtCore>
#include <QtOpenGL>

#include <QOpenGLFunctions_3_2_Core>


#include "Core/ATData.h"
#include "3DViewer/colormaps.h"

#include "3DViewer/line_painter.h"
#include "3DViewer/point_painter.h"
#include "3DViewer/point_viewer_widget.h"
#include "3DViewer/render_options.h"
#include "3DViewer/triangle_painter.h"




namespace AI3D
{
	namespace GUI
	{
		typedef struct gcpPoint {
			QVector3D point;
			QColor rgb;
			QString name;
			int type; 
		}GcpPoint;

		enum ModelSel {
			POINT_MODEL = 0,
			CAMERAS_MODEL,
			SURVEY_MODEL,
		};
		class ViewWidget;
		class ModelViewerWidget : public QOpenGLWidget,
			protected QOpenGLFunctions_3_2_Core 
		{

			
			Q_OBJECT
				
		public:
			const float kInitNearPlane = 0.5f;
			const float kMinNearPlane = 1e-3f;
			const float kMaxNearPlane = 1e5f;
			const float kNearPlaneScaleSpeed = 0.02f;
			const float kFarPlane =  1e5f; 
			const float kInitFocusDistance = 100.0f; 
			const float kMinFocusDistance = 1e-5f;;
			const float kMaxFocusDistance = 1e8f;
			const float kFieldOfView = 25.0f;
			const float kFocusSpeed = 1.0f;
			const float kTranslateSpeed = 2.0f;
			const float kInitPointSize = 4.0f;
			const float kMinPointSize = 0.5f;
			const float kMaxPointSize = 100.0f;
			const float kPointScaleSpeed = 0.1f;
			const float kInitImageSize = 0.04f; 
			const float kMinImageSize = 1e-6f;
			const float kMaxImageSize = 1e3f;
			const float kImageScaleSpeed = 0.1f;
			const int kDoubleClickInterval = 250;
		
			ModelViewerWidget(QWidget* parent, RenderOptions options);
			virtual ~ModelViewerWidget();

			void setModel_select(ModelSel modelSelect);
			void UploadGcpPoint();
			
			void updateShow();
			void ReloadReconstruction();
			void ClearReconstruction();

			int GetProjectionType() const;

			void SetPointColormap(PointColormapBase* colormap);

			

			void EnableCoordinateGrid();
			void DisableCoordinateGrid();

			void ChangeFocusDistance(const float delta);
			void ChangeNearPlane(const float delta);
			void ChangePointSize(const float delta);
			void ChangeCameraSize(const float delta);

			void RotateView(const float x, const float y, const float prev_x,
				const float prev_y);
			void TranslateView(const float x, const float y, const float prev_x,
				const float prev_y);

			void ResetView();

			QMatrix4x4 ModelViewMatrix() const;
			void SetModelViewMatrix(const QMatrix4x4& matrix);

			void SelectObject(const int x, const int y);
			
			image_t selected_image_id_;
			point3D_t selected_point3D_id_;

			void ShowPointInfo(const point3D_t point3D_id);
			void ShowImageInfo(const image_t image_id);

			float PointSize() const;
			float ImageSize() const;
			void SetPointSize(const float point_size);
			void SetImageSize(const float image_size);

			void SetBackgroundColor(const float r, const float g, const float b);

		
			void requestImageDelete();
			
			std::string srs_definition;

			
			AI3D::CORE::ATData* reconstruction = nullptr;
			AI3D::CORE::ATData* reconstruction_origin = nullptr;
			image_t* item_select_ = nullptr;
			EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera) cameras;
			EIGEN_STL_UMAP(image_t, AI3D::CORE::Image) images;
			EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D) points3D;
			EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) controlpoints;
			std::vector<image_t> image_ids_;
			bool use_preview_view_ = false;
			std::vector<Eigen::Vector3d> CoordinateAxis;
			
			QList<GcpPoint> gcpPoints;
			QLabel* statusbar_status_label;
			PointViewerWidget* point_viewer_widget_;
			
			void keyPressEvent(QKeyEvent* e) override;
		
			RenderOptions& GetOptions() { return options_; }
			
		signals:
			void update_delete_image(point3D_t, QString);
			void update_item_deleted();
			

		protected:
			void initializeGL() override;
			void resizeGL(int width, int height) override;
			void paintGL() override;

		private:
			void mousePressEvent(QMouseEvent* event) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void mouseMoveEvent(QMouseEvent* event) override;
			void wheelEvent(QWheelEvent* event) override;

			
			
			
			void SetupPainters();
			void SetupView();

			void Upload();
			void UploadCoordinateGridData();
			
			void UploadPointDataNew(const bool selection_mode = false);
			void uploadPictureVetex();
			void UploadPointConnectionData();
			void UploadImageData( const bool selection_mode = false,const bool changetri =false );
			void UploadImageConnectionData();
			
			void UploadMinMaxPoint();
			void UploadCircle();
			
			void ComposeProjectionMatrix();

			float ZoomScale() const;
			float AspectRatio() const;
			float OrthographicWindowExtent() const;
	
			Eigen::Vector3f PositionToArcballVector(const float x, const float y) const;

		private:

			
			std::vector<PointPainter::Data> m_data_point_painter;
			

			RenderOptions options_;
		
			std::vector<std::pair<image_t, Eigen::Vector4f> > haspos_ids_;
			double bbDiag_ = -1;
			
			QMatrix4x4 model_view_matrix_;
			QMatrix4x4 projection_matrix_;

			LinePainter coordinate_axes_painter_;
			LinePainter coordinate_grid_painter_;


			PointPainter point_painter_;
			LinePainter point_connection_painter_;
			
			PointPainter image_point_painter_;
			LinePainter image_line_painter_;
			TrianglePainter image_triangle_painter_;
			LinePainter image_connection_painter_;

			LinePainter circle_gcp;

			LinePainter movie_grabber_path_painter_;
			LinePainter movie_grabber_line_painter_;
			TrianglePainter movie_grabber_triangle_painter_;

			PointPainter my_tab_point;
			LinePainter my_picture_vetex;

			

			std::unique_ptr<PointColormapBase> point_colormap_;

			bool mouse_is_pressed_;
			QTimer mouse_press_timer_;
			QPoint prev_mouse_pos_;

			float focus_distance_;
			ModelSel  model_select;
			bool isReloadReconstruction;
			
			Eigen::Vector4f scene_center_dot_;

			std::vector<std::pair<size_t, char>> selection_buffer_;
		
			size_t selected_movie_grabber_view_;

			bool coordinate_grid_enabled_;

			
			float point_size_;
			
			float image_size_;
			
			float near_plane_;

			float bg_color_[3];

			ViewWidget* view3d;
			

			void init_picture_texture();
			void makeObject();
			QOpenGLTexture* texture_picture;
			QOpenGLShaderProgram* program;
			
			QOpenGLBuffer vbo;

		};
	}
} 

#endif  
