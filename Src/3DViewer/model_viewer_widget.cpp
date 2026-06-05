#include "3DViewer/model_viewer_widget.h"
#include "3DViewer/3DView.h"

#include "3DViewer/qt_utils.h"
#include <QOpenGLWidget>
#include "Core/Timer.h"
#include "Core/CoordinateSystem.h"
#include "Core/Types.h"
#include <glog/logging.h>
#include "Util/TaskProcess.h"

#define POINT_SELECTED_R 0.69
#define POINT_SELECTED_G 0.23
#define POINT_SELECTED_B 0.93
#define IMAGE_R 1
#define IMAGE_G 1
#define IMAGE_B 0
#define IMAGE_A 1

#define IMAGE_POINTSTYLE_R 1
#define IMAGE_POINTSTYLE_G 0
#define IMAGE_POINTSTYLE_B 0
#define IMAGE_POINTSTYLE_A 1


#define IMAGE_SELECTED_R 1
#define IMAGE_SELECTED_G 0
#define IMAGE_SELECTED_B 1
#define IMAGE_SELECTED_A 0.6
#define SELECTION_BUFFER_IMAGE 0
#define SELECTION_BUFFER_POINT 1

#define GRID_RGBA 0.2, 0.2, 0.2, 0.6
#define X_AXIS_RGBA 0.9, 0, 0, 0.5
#define Y_AXIS_RGBA 0, 0.9, 0, 0.5
#define Z_AXIS_RGBA 0, 0, 0.9, 0.5

const float _Pi = 3.1415926;

const Eigen::Vector4f kSelectedPointColor(0.0f, 1.0f, 0.0f, 1.0f);

const Eigen::Vector4f kSelectedImagePlaneColor(1.0f, 0.0f, 1.0f, 0.6f);
const Eigen::Vector4f kSelectedImageFrameColor(0.8f, 0.0f, 0.8f, 1.0f);

const Eigen::Vector4f kMovieGrabberImagePlaneColor(0.0f, 1.0f, 1.0f, 0.6f);
const Eigen::Vector4f kMovieGrabberImageFrameColor(0.0f, 0.8f, 0.8f, 1.0f);

const Eigen::Vector4f kGridColor(0.2f, 0.2f, 0.2f, 0.6f);
const Eigen::Vector4f kXAxisColor(0.9f, 0.0f, 0.0f, 0.5f);
const Eigen::Vector4f kYAxisColor(0.0f, 0.9f, 0.0f, 0.5f);
const Eigen::Vector4f kZAxisColor(0.0f, 0.0f, 0.9f, 0.5f);
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1
namespace AI3D
{
	namespace GUI
	{
		namespace
		{

			inline size_t RGBToIndex(const uint8_t r, const uint8_t g, const uint8_t b) 
			{
				return static_cast<size_t>(r) + static_cast<size_t>(g) * 256 +
					static_cast<size_t>(b) * 65536;
			}

			
			inline void IndexToRGB(const size_t index, float& r, float& g, float& b) 
			{
				r = ((index & 0x000000FF) >> 0) / 255.0f;
				g = ((index & 0x0000FF00) >> 8) / 255.0f;
				b = ((index & 0x00FF0000) >> 16) / 255.0f;
			}

			inline Eigen::Vector4f IndexToRGB(const size_t index) 
			{
				Eigen::Vector4f color;
				color(0) = ((index & 0x000000FF) >> 0) / 255.0f;
				color(1) = ((index & 0x0000FF00) >> 8) / 255.0f;
				color(2) = ((index & 0x00FF0000) >> 16) / 255.0f;
				color(3) = 1.0f;
				return color;
			}


			void BuildImageModel(const int width,const int height, 
				const float image_size,const float focal_length_raw,float& image_width,float& image_height,float focal_length)
			{

				
				 image_width = image_size;

				 image_height =
					image_width * static_cast<float>(height / width);
				const float image_extent = std::max(image_width, image_height);
				const float camera_extent = std::max(width, height);
				const float camera_extent_world = camera_extent / focal_length_raw;
				
				

				focal_length =  image_extent / camera_extent_world;
			}


			void BuildImageModel(const AI3D::CORE::Image& image, const AI3D::CORE::Camera& camera,
				const float image_size, const float r, const float g,
				const float b, const float a, LinePainter::Data& line1,
				LinePainter::Data& line2, LinePainter::Data& line3,
				LinePainter::Data& line4, LinePainter::Data& line5,
				LinePainter::Data& line6, LinePainter::Data& line7,
				LinePainter::Data& line8, TrianglePainter::Data& triangle1,
				TrianglePainter::Data& triangle2)
			{

				
				const float image_width = image_size;

				const float image_height =
					image_width * static_cast<float>(camera.GetHeight()) / camera.GetWidth();
				const float image_extent = std::max(image_width, image_height);
				const float camera_extent = std::max(camera.GetWidth(), camera.GetHeight());
				const float camera_extent_world =
					static_cast<float>(camera.ImageToWorldThreshold(camera_extent));
				

				const float focal_length =  image_extent / camera_extent_world;


				Eigen::Matrix<float, 3, 4> inv_proj_matrix =
					image.InverseProjectionMatrix().cast<float>();



				const Eigen::Vector3f pc = inv_proj_matrix.rightCols<1>();



				const Eigen::Vector3f tl =
					inv_proj_matrix *
					Eigen::Vector4f(-image_width / 2, image_height / 2, focal_length, 1);

				const Eigen::Vector3f tr =
					inv_proj_matrix *
					Eigen::Vector4f(image_width / 2, image_height / 2, focal_length, 1);
				const Eigen::Vector3f br =
					inv_proj_matrix *
					Eigen::Vector4f(image_width / 2, -image_height / 2, focal_length, 1);
				const Eigen::Vector3f bl =
					inv_proj_matrix *
					Eigen::Vector4f(-image_width / 2, -image_height / 2, focal_length, 1);

				

				line1.point1 = PointPainter::Data(pc(0), pc(1), pc(2), r, g, b, 1);
				line1.point2 = PointPainter::Data(tl(0), tl(1), tl(2), r, g, b, 1);

				line2.point1 = PointPainter::Data(pc(0), pc(1), pc(2), r, g, b, 1);
				line2.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, 1);

				line3.point1 = PointPainter::Data(pc(0), pc(1), pc(2), r, g, b, 1);
				line3.point2 = PointPainter::Data(br(0), br(1), br(2), r, g, b, 1);

				line4.point1 = PointPainter::Data(pc(0), pc(1), pc(2), r, g, b, 1);
				line4.point2 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, 1);

				line5.point1 = PointPainter::Data(tl(0), tl(1), tl(2), r, g, b, 1);
				line5.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, 1);

				line6.point1 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, 1);
				line6.point2 = PointPainter::Data(br(0), br(1), br(2), r, g, b, 1);

				line7.point1 = PointPainter::Data(br(0), br(1), br(2), r, g, b, 1);
				line7.point2 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, 1);

				line8.point1 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, 1);
				line8.point2 = PointPainter::Data(tl(0), tl(1), tl(2), r, g, b, 1);

				


				triangle1.point1 = PointPainter::Data(tl(0), tl(1), tl(2), r, g, b, a);
				triangle1.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, a);
				triangle1.point3 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, a);

				triangle2.point1 = PointPainter::Data(bl(0), bl(1), bl(2), r, g, b, a);
				triangle2.point2 = PointPainter::Data(tr(0), tr(1), tr(2), r, g, b, a);
				triangle2.point3 = PointPainter::Data(br(0), br(1), br(2), r, g, b, a);



			}


		}  

ModelViewerWidget::ModelViewerWidget(QWidget* parent, RenderOptions options)
	: QOpenGLWidget(parent),
	options_(options),
	point_viewer_widget_(new PointViewerWidget(parent, this)),
	
	
	
	mouse_is_pressed_(false),
	focus_distance_(kInitFocusDistance),
	selected_image_id_(kInvalidImageId),
	selected_point3D_id_(kInvalidPoint3DId),
	coordinate_grid_enabled_(true),
	near_plane_(kInitNearPlane),
	view3d((ViewWidget *)parent),
	model_select(CAMERAS_MODEL),
	isReloadReconstruction(false),
	scene_center_dot_(0.0f,0.0f,0.0f,1.0f)
{
	reconstruction = new AI3D::CORE::ATData();
	reconstruction_origin = new AI3D::CORE::ATData();
  bg_color_[0] = 0.811f;
  bg_color_[1] = 0.811f;
  bg_color_[2] = 0.811f;

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setSamples(4);
  format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef DEBUG
  format.setOption(QSurfaceFormat::DebugContext);
#endif
  setFormat(format);
  QSurfaceFormat::setDefaultFormat(format);

  SetPointColormap(new PointColormapPhotometric());

  image_size_ = static_cast<float>(devicePixelRatio() * image_size_);
  point_size_ = static_cast<float>(devicePixelRatio() * point_size_);
  
  item_select_ = &selected_image_id_;
  
  
  setFocusPolicy(Qt::StrongFocus);
  
}

ModelViewerWidget::~ModelViewerWidget()
{
	if (reconstruction != nullptr)
	{
		delete reconstruction;
		reconstruction = nullptr;
	}
	
	
	
	
	
	
}

void ModelViewerWidget::makeObject()
{
	makeCurrent();
	static const int coords[4][3] = {
		{ +1, -1, -1 },{ -1, -1, -1 },{ -1, +1, -1 },{ +1, +1, -1 }
	};
	QVector<GLfloat> vertData;
	for (int j = 0; j < 4; ++j) {
		
		vertData.append(0.2 * coords[j][0]);
		vertData.append(0.2 * coords[j][1]);
		vertData.append(0.2 * coords[j][2]);
		
		vertData.append(j == 0 || j == 3);
		vertData.append(j == 0 || j == 1);
	}
	
	texture_picture = new QOpenGLTexture(QImage(QString(":/new/prefix1/skin/center.png")).mirrored());

	vbo.bind();
	vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
	vbo.release();

}
void ModelViewerWidget::init_picture_texture()
{
	makeCurrent();
	program = new QOpenGLShaderProgram;
	vbo.destroy();
	if (program->isLinked()) {
		program->release();
		program->removeAllShaders();
	}


	QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	const char *vsrc =
		"attribute highp vec4 vertex;\n"
		"attribute mediump vec4 texCoord;\n"
		"varying mediump vec4 texc;\n"
		"uniform mediump mat4 matrix;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = matrix * vertex;\n"
		"    texc = texCoord;\n"
		"}\n";
	vshader->compileSourceCode(vsrc);

	QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	const char *fsrc =
		"uniform sampler2D texture;\n"
		"varying mediump vec4 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_FragColor = texture2D(texture, texc.st);\n"
		"}\n";
	fshader->compileSourceCode(fsrc);


	program->addShader(vshader);
	program->addShader(fshader);
	program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
	program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
	program->link();
	program->bind();

	vbo.create();

	makeObject();
}


void ModelViewerWidget::initializeGL() {

  initializeOpenGLFunctions();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  SetupPainters();
  SetupView();

  init_picture_texture();
}

void ModelViewerWidget::resizeGL(int width, int height) {
	
	
	glViewport(0, 0, width, height);
	ComposeProjectionMatrix();
	UploadCoordinateGridData();
	uploadPictureVetex();
}





void ModelViewerWidget::paintGL() 
{
	

  glClearColor(bg_color_[0], bg_color_[1], bg_color_[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  

   QMatrix4x4 pmv_matrix = projection_matrix_ * model_view_matrix_;
  

  
  QMatrix4x4 model_view_center_matrix = model_view_matrix_;
  const Eigen::Vector4f rot_center = QMatrixToEigen(model_view_matrix_).inverse() * Eigen::Vector4f(0, 0, -focus_distance_, 1);
  

  QMatrix4x4 model_view_axes_matrix = model_view_matrix_;

 
  {
	model_view_center_matrix.translate(rot_center(0), rot_center(1), rot_center(2));
  }
 

   
  if (coordinate_grid_enabled_) 
  {


	  const QMatrix4x4 pmv_axes_matrix = projection_matrix_ * model_view_axes_matrix;
	  coordinate_axes_painter_.Render(pmv_axes_matrix, width(), height(), 2);

	


	const QMatrix4x4 pmvc_matrix =
        projection_matrix_ * model_view_center_matrix;
      coordinate_grid_painter_.Render(pmvc_matrix, width(), height(), 1);
  }

  
  image_point_painter_.Render(pmv_matrix, point_size_);
  
  point_connection_painter_.Render(pmv_matrix, width(), height(), 1);


  
  
  
  image_line_painter_.Render(pmv_matrix, width(), height(), 1);
  image_triangle_painter_.Render(pmv_matrix);
  image_connection_painter_.Render(pmv_matrix, width(), height(), 1);



  circle_gcp.Render(pmv_matrix, width(), height(), 4);
 

   vbo.bind();
   texture_picture->bind();

   program->setUniformValue("texture", 0);
   program->setUniformValue("matrix", projection_matrix_ * model_view_axes_matrix);
   program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
   program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
   program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
   program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
   vbo.release();
   texture_picture->release();
}

void ModelViewerWidget::setModel_select(ModelSel modelSelect)
{
	model_select = modelSelect;
}



void  ModelViewerWidget::UploadGcpPoint()
{
	gcpPoints.clear();
	if (controlpoints.empty())
	{
		circle_gcp.Setup();
		return;
	}


	for (auto& it : controlpoints)
	{
		GcpPoint point;
		if (!it.second.GetObjectPoint().HasXYZ())
		{
			continue;
		}
		point.point = QVector3D(it.second.GetObjectPoint().GetXYZ().x(),
			it.second.GetObjectPoint().GetXYZ().y(), it.second.GetObjectPoint().GetXYZ().z());
		

		
		
		
		
		
		
		
		
		
		

		if (it.second.GetObjectPoint().GetTrack().Length() > 0 )
		{
			
			if( it.second.GetType() == gpt_e::GCP_CONTROL_H
				|| it.second.GetType() == gpt_e::GCP_CONTROL_HV || it.second.GetType() == gpt_e::GCP_CONTROL_V)
				{
				point.rgb.setRgb(255, 255, 0);
				point.type = 0;
				}
			else if (it.second.GetType() == gpt_e::GCP_CHECK_H
				|| it.second.GetType() == gpt_e::GCP_CHECK_HV || it.second.GetType() == gpt_e::GCP_CHECK_V)
			{
				point.rgb.setRgb(255, 0, 0);
				point.type = 1;
			}
			else
			{
				point.rgb.setRgb(0, 0, 255);
			}
		}
		else
			{
				point.rgb.setRgb(160, 160, 160);
			}


		point.name = str2qstr(it.second.GetName());
		gcpPoints.append(point);
	}
	
	UploadCircle();
	update();
}



void ModelViewerWidget::updateShow()
{
	Upload();
	update();
}

 

void ModelViewerWidget::Upload() 
{
	point_colormap_->Prepare(cameras, images, points3D, image_ids_);
	ComposeProjectionMatrix();
	UploadCoordinateGridData();
	if (item_select_ != nullptr)
	{
		if (*item_select_ != kInvalidImageId)
		{
			selected_image_id_ = *item_select_;
		}
	}
	std::vector<PointPainter::Data>().swap(m_data_point_painter);
	UploadGcpPoint();
	UploadPointDataNew();
	UploadImageData();
	image_point_painter_.Upload(m_data_point_painter);
	

	UploadPointConnectionData();
	UploadImageConnectionData();
	
}



void ModelViewerWidget::ReloadReconstruction() 
{
	if (!use_preview_view_)
	{
	point_size_ = kInitPointSize;
	image_size_ = kInitImageSize;
	focus_distance_ = kInitFocusDistance;
	model_view_matrix_.setToIdentity();
	if (reconstruction == nullptr) 
	{
		return;
	}
	auto bound = reconstruction->ComputeBoundsAndCentroid();
	
	if (std::get<0>(bound).x() == DBL_MAX || std::get<1>(bound).x() == -DBL_MAX
		|| std::get<0>(bound).y() == DBL_MAX || std::get<1>(bound).y() == -DBL_MAX
		|| std::get<0>(bound).z() == DBL_MAX || std::get<1>(bound).z() == -DBL_MAX
		)
	{
		return;
	}

	double z = (std::get<1>(bound).z() - std::get<0>(bound).z()) * 2;
	double scale = 1;
	if (double(height())/z > 3)
	{
		scale = 2;
	}
	
  
 
  isReloadReconstruction = true; 
  QMatrix4x4 view_matrix;
  
  

	  view_matrix.setToIdentity();
	  double x_distance;
	  double y_distance;
	  double z_distance;
	  x_distance = 9;
	  y_distance = 9;
	  z_distance = 9;

	  scene_center_dot_.x() = std::fabs(reconstruction->GetMin3dPoint_x() + reconstruction->GetMax3dPoint_x()) / 2;
	  scene_center_dot_.y() = std::fabs(reconstruction->GetMin3dPoint_y() + reconstruction->GetMax3dPoint_y()) / 2;
	  scene_center_dot_.z() = std::fabs(reconstruction->GetMin3dPoint_z() + reconstruction->GetMax3dPoint_z()) / 2;


	  const QVector3D eye((reconstruction->GetMax3dPoint_x() * 0.75), -reconstruction->GetMin3dPoint_y() - 2 * y_distance,
		  reconstruction->GetMax3dPoint_z() + 2 * z_distance);
	  const QVector3D center(scene_center_dot_.x(), scene_center_dot_.y(), scene_center_dot_.z());
	  const QVector3D up(0, 0, 1);

	  
	  focus_distance_ = qSqrt(qPow(scene_center_dot_.x() - eye.x(), 2) \
		  + qPow(scene_center_dot_.y() - eye.y(), 2) \
		  + qPow(scene_center_dot_.z() - eye.z(), 2));

	  
	  if (scene_center_dot_.x() == 0 && scene_center_dot_.y() == 0 && scene_center_dot_.z() == 0)
	  {
		  const QVector3D eyesee = QVector3D(2 * x_distance, 0, 2 * z_distance);
		  view_matrix.lookAt(eyesee, center, up);
	  }
	  else
		  view_matrix.lookAt(eye, center, up);
	 
	  model_view_matrix_ = view_matrix * model_view_matrix_;
  }

  cameras = reconstruction->GetCameras();
  points3D = reconstruction->GetPoints3D();
  image_ids_ = reconstruction->GetImagesIds();
  controlpoints = reconstruction->GetControlPoints();

  images.clear();
  for (const image_t image_id : image_ids_) 
  {
    images[image_id] = reconstruction->GetImage(image_id); 
  }

}

void ModelViewerWidget::UploadMinMaxPoint() {


	makeCurrent();
	std::vector<PointPainter::Data> data;
	data.reserve(2);

	PointPainter::Data painter_point;
	painter_point.x = static_cast<float>(reconstruction->GetMin3dPoint_x());
	painter_point.y = static_cast<float>(reconstruction->GetMin3dPoint_y());
	painter_point.z = static_cast<float>(reconstruction->GetMin3dPoint_z());
	painter_point.r = 255;
	painter_point.g = 0;
	painter_point.b = 0;
	painter_point.a = 1;

	PointPainter::Data painter_point1;
	painter_point1.x = static_cast<float>(reconstruction->GetMax3dPoint_x());
	painter_point1.y = static_cast<float>(reconstruction->GetMax3dPoint_y());
	painter_point1.z = static_cast<float>(reconstruction->GetMax3dPoint_z());
	painter_point1.r = 255;
	painter_point1.g = 255;
	painter_point1.b = 0;
	painter_point1.a = 1;


	data.push_back(painter_point);
	data.push_back(painter_point1);

	my_tab_point.Upload(data);

}

void ModelViewerWidget::ClearReconstruction() 
{
  cameras.clear();
  images.clear();
  points3D.clear();
  image_ids_.clear();
  reconstruction = nullptr;
  Upload();
}

int ModelViewerWidget::GetProjectionType() const 
{
  return options_.projection_type;
}

void ModelViewerWidget::SetPointColormap(PointColormapBase* colormap) 
{
  point_colormap_.reset(colormap);
}



void ModelViewerWidget::EnableCoordinateGrid() 
{
  coordinate_grid_enabled_ = true;
  update();
}

void ModelViewerWidget::DisableCoordinateGrid() 
{
  coordinate_grid_enabled_ = false;
  update();
}

void ModelViewerWidget::ChangeFocusDistance(const float delta) 
{
  if (delta == 0.0f) 
  {
    return;
  }
  const float prev_focus_distance = focus_distance_;
  float diff = delta * ZoomScale() * kFocusSpeed;

  focus_distance_ -= diff;
  if (focus_distance_ < kMinFocusDistance) 
  {
    focus_distance_ = kMinFocusDistance;
    diff = prev_focus_distance - focus_distance_;
  } 
  else if (focus_distance_ > kMaxFocusDistance) 
  {
    focus_distance_ = kMaxFocusDistance;
    diff = prev_focus_distance - focus_distance_;
  }

  const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();
  const Eigen::Vector3f tvec(0, 0, diff);
  const Eigen::Vector3f tvec_rot = vm_mat.block<3, 3>(0, 0) * tvec;
  model_view_matrix_.translate(tvec_rot(0), tvec_rot(1), tvec_rot(2));

  ComposeProjectionMatrix();
  UploadCoordinateGridData();
 
  update();
}

void ModelViewerWidget::ChangeNearPlane(const float delta) 
{
  if (delta == 0.0f) 
  {
    return;
  }
  near_plane_ *= (.5f + delta / 100.0f * kNearPlaneScaleSpeed);
  near_plane_ = std::max(kMinNearPlane, std::min(kMaxNearPlane, near_plane_));
  ComposeProjectionMatrix();
  UploadCoordinateGridData();
  update();
}

void ModelViewerWidget::ChangePointSize(const float delta) 
{
  if (delta == 0.0f) 
  {
    return;
  }
  point_size_ *= (1.0f + delta / 100.0f * kPointScaleSpeed);
  point_size_ = std::max(kMinPointSize, std::min(kMaxPointSize, point_size_));
  update();
}

void ModelViewerWidget::RotateView(const float x, const float y,
                                   const float prev_x, const float prev_y) {
  if (x - prev_x == 0 && y - prev_y == 0) {
    return;
  }


  
  
  

  
  const Eigen::Vector3f u = PositionToArcballVector(x, y);
  const Eigen::Vector3f v = PositionToArcballVector(prev_x, prev_y);

  
  const float angle = 2.0f * std::acos(std::min(1.0f, u.dot(v)));

  const float kMinAngle = 1e-3f;
  if (angle > kMinAngle) 
  {
    const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();
    
    Eigen::Vector3f axis = vm_mat.block<3, 3>(0, 0) * v.cross(u);
    axis = axis.normalized();
	 Eigen::Vector4f rot_center;
	
	{ 
		
	    rot_center = vm_mat * Eigen::Vector4f(0, 0, -focus_distance_, 1);
	}
    
    model_view_matrix_.translate(rot_center(0), rot_center(1), rot_center(2));
    model_view_matrix_.rotate(RadToDeg(angle), axis(0), axis(1), axis(2));
    model_view_matrix_.translate(-rot_center(0), -rot_center(1),-rot_center(2));
	
    update();
  }
}

void ModelViewerWidget::TranslateView(const float x, const float y,
                                      const float prev_x, const float prev_y) {
  if (x - prev_x == 0 && y - prev_y == 0) {
    return;
  }

  Eigen::Vector3f tvec(x - prev_x, prev_y - y, 0.0f);

  if (options_.projection_type ==
      RenderOptions::ProjectionType::PERSPECTIVE) {
    tvec *= (ZoomScale());

  } else if (options_.projection_type ==
             RenderOptions::ProjectionType::ORTHOGRAPHIC) {
    tvec *= 2.0f * OrthographicWindowExtent() / height();
  }
  
  const Eigen::Matrix4f vm_mat = QMatrixToEigen(model_view_matrix_).inverse();
  const Eigen::Vector3f tvec_rot = vm_mat.block<3, 3>(0, 0) * tvec;
 
  model_view_matrix_.translate(tvec_rot(0), tvec_rot(1), tvec_rot(2));



  update();
}

void ModelViewerWidget::ChangeCameraSize(const float delta) 
{
  if (delta == 0.0f) 
  {
    return;
  }

  image_size_ *= (1.0f + delta / 100.0f * kImageScaleSpeed);
  
  
	 
  
  
  
	 
  
  image_size_ = std::max(kMinImageSize, std::min(kMaxImageSize, image_size_));
  
  UploadImageData(false);
  
  update();
}

void ModelViewerWidget::ResetView() {
  SetupView();
  Upload();
}

QMatrix4x4 ModelViewerWidget::ModelViewMatrix() const {
  return model_view_matrix_;
}

void ModelViewerWidget::SetModelViewMatrix(const QMatrix4x4& matrix) {
  model_view_matrix_ = matrix;
  update();
}



void ModelViewerWidget::SelectObject(const int x, const int y) {
  makeCurrent();

  
  glDisable(GL_MULTISAMPLE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  std::vector<PointPainter::Data>().swap(m_data_point_painter);
  
  if (model_select == CAMERAS_MODEL) 
  {
	UploadImageData(true);
  }
  if (model_select == POINT_MODEL) 
  {
	
	UploadPointDataNew(true);
  }
  image_point_painter_.Upload(m_data_point_painter);
  
  const QMatrix4x4 pmv_matrix = projection_matrix_ * model_view_matrix_;
  image_triangle_painter_.Render(pmv_matrix);
 
  image_point_painter_.Render(pmv_matrix, 2 * point_size_);
  const int scaled_x = devicePixelRatio() * x;
  const int scaled_y = devicePixelRatio() * (height() - y - 1);

  QOpenGLFramebufferObjectFormat fbo_format;
  fbo_format.setSamples(0);
  QOpenGLFramebufferObject fbo(1, 1, fbo_format);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFramebufferObject());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.handle());
  glBlitFramebuffer(scaled_x, scaled_y, scaled_x + 1, scaled_y + 1, 0, 0, 1, 1,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  fbo.bind();
  std::array<uint8_t, 3> color;
  glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, color.data());
  fbo.release();

  const size_t index = RGBToIndex(color[0], color[1], color[2]);

  if (index < selection_buffer_.size())
  {
    const char buffer_type = selection_buffer_[index].second;
    if (buffer_type == SELECTION_BUFFER_IMAGE) 
	{
      selected_image_id_ = static_cast<image_t>(selection_buffer_[index].first);
	 
      selected_point3D_id_ = kInvalidPoint3DId;
      
	  
    } 
	else if (buffer_type == SELECTION_BUFFER_POINT) 
	{
      selected_image_id_ = kInvalidImageId;
      selected_point3D_id_ = selection_buffer_[index].first;
	  
      
    } 
	else 
	{
      selected_image_id_ = kInvalidImageId;
      selected_point3D_id_ = kInvalidPoint3DId;
     
    }
  }
  else 
  {
    selected_image_id_ = kInvalidImageId;
    selected_point3D_id_ = kInvalidPoint3DId;
	
    
  }

  
  glEnable(GL_MULTISAMPLE);

  selection_buffer_.clear();
  std::vector<PointPainter::Data>().swap(m_data_point_painter);
  
  UploadPointDataNew();
  UploadImageData();
  UploadPointConnectionData();
  UploadImageConnectionData();
  image_point_painter_.Upload(m_data_point_painter);
  *item_select_= selected_image_id_;
  update();
}



void ModelViewerWidget::ShowPointInfo(const point3D_t point3D_id) {
  point_viewer_widget_->Show(point3D_id);
}

void ModelViewerWidget::ShowImageInfo(const image_t image_id) {
  
}

float ModelViewerWidget::PointSize() const { return point_size_; }

float ModelViewerWidget::ImageSize() const { return image_size_; }

void ModelViewerWidget::SetPointSize(const float point_size) {
  point_size_ = point_size;
}

void ModelViewerWidget::SetImageSize(const float image_size) {
  image_size_ = image_size;
  UploadImageData();
}

void ModelViewerWidget::SetBackgroundColor(const float r, const float g,
                                           const float b) {
  bg_color_[0] = r;
  bg_color_[1] = g;
  bg_color_[2] = b;
  update();
}

void ModelViewerWidget::mousePressEvent(QMouseEvent* event) 
{
	
  if (mouse_press_timer_.isActive()) {  
    mouse_is_pressed_ = false;
    mouse_press_timer_.stop();
    selection_buffer_.clear();
    SelectObject(event->pos().x(), event->pos().y());
  } else {  
    mouse_press_timer_.setSingleShot(true);
    mouse_press_timer_.start(kDoubleClickInterval);
    mouse_is_pressed_ = true;
    prev_mouse_pos_ = event->pos();
  }
  event->accept();
}

void ModelViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
  mouse_is_pressed_ = false;
  event->accept();
}

void ModelViewerWidget::mouseMoveEvent(QMouseEvent* event) 
{
	
	{

		if (mouse_is_pressed_)
		{
			if (event->buttons() & Qt::RightButton ||
				(event->buttons() & Qt::LeftButton &&
					event->modifiers() & Qt::ControlModifier))
			{
				TranslateView(event->pos().x(), event->pos().y(), prev_mouse_pos_.x(),
					prev_mouse_pos_.y());
			}
			else if (event->buttons() & Qt::LeftButton)
			{
				RotateView(event->pos().x(), event->pos().y(), prev_mouse_pos_.x(),
					prev_mouse_pos_.y());
			}
		}
		prev_mouse_pos_ = event->pos();
		event->accept();
	}
}

void ModelViewerWidget::wheelEvent(QWheelEvent* event) 
{
  if (event->modifiers() & Qt::ControlModifier) 
  {
    ChangePointSize(event->delta());
  } 
  else if (event->modifiers() & Qt::AltModifier)
  {
    ChangeCameraSize(event->delta());
  } 
  else if (event->modifiers() & Qt::ShiftModifier) 
  {
    ChangeNearPlane(event->delta());
  } 
  else 
  {
    ChangeFocusDistance(event->delta());
  }
  event->accept();
}

void ModelViewerWidget::SetupPainters() {
  makeCurrent();

  coordinate_axes_painter_.Setup();
  coordinate_grid_painter_.Setup();
  image_point_painter_.Setup();
  point_painter_.Setup();
  point_connection_painter_.Setup();

  my_tab_point.Setup();
  my_picture_vetex.Setup();
  image_line_painter_.Setup();
  image_triangle_painter_.Setup();
  image_connection_painter_.Setup();

  circle_gcp.Setup();

}

void ModelViewerWidget::SetupView() {
  point_size_ = kInitPointSize;
  image_size_ = kInitImageSize;
  focus_distance_ = kInitFocusDistance;
  model_view_matrix_.setToIdentity();
 model_view_matrix_.translate(0, 0, -focus_distance_);

  
 
  model_view_matrix_.scale(2);

}



void ModelViewerWidget::UploadCoordinateGridData() 
{
  makeCurrent();

  const float scale = ZoomScale();

  
  std::vector<LinePainter::Data> grid_data(3);
 
  double grid_dis = 100 * scale;
  double axes_dis = 10 * scale;
  
  
	 
	 
  
  

  grid_data[0].point1 = PointPainter::Data(-axes_dis, 0, 0, GRID_RGBA);
  grid_data[0].point2 = PointPainter::Data(axes_dis, 0, 0, GRID_RGBA);

  grid_data[1].point1 = PointPainter::Data(0, -axes_dis, 0, GRID_RGBA);
  grid_data[1].point2 = PointPainter::Data(0, axes_dis, 0, GRID_RGBA);

  grid_data[2].point1 = PointPainter::Data(0, 0, -axes_dis, GRID_RGBA);
  grid_data[2].point2 = PointPainter::Data(0, 0, axes_dis, GRID_RGBA);
  
  
  coordinate_grid_painter_.Upload(grid_data);


  
  std::vector<LinePainter::Data> axes_data(3);
  if (isReloadReconstruction) 
  {
	  
	  if (CoordinateAxis.size() > 0)
	  {
		  


		  axes_data[0].point1 = PointPainter::Data(0, 0, 0, X_AXIS_RGBA);
		  axes_data[0].point2 = PointPainter::Data(CoordinateAxis.at(1)(0), CoordinateAxis.at(1)(1), CoordinateAxis.at(1)(2), X_AXIS_RGBA);

		  axes_data[1].point1 = PointPainter::Data(0, 0, 0, Y_AXIS_RGBA);
		  axes_data[1].point2 = PointPainter::Data(CoordinateAxis.at(2)(0), CoordinateAxis.at(2)(1), CoordinateAxis.at(2)(2), Y_AXIS_RGBA);

		  axes_data[2].point1 = PointPainter::Data(0, 0, 0, Z_AXIS_RGBA);
		  axes_data[2].point2 = PointPainter::Data(CoordinateAxis.at(3)(0), CoordinateAxis.at(3)(1), CoordinateAxis.at(3)(2), Z_AXIS_RGBA);

	  }
	 
  }
  else
  {
	  axes_data[0].point1 = PointPainter::Data(0, 0, 0, X_AXIS_RGBA);
	  axes_data[0].point2 = PointPainter::Data(grid_dis, 0, 0, X_AXIS_RGBA);
	  axes_data[1].point1 = PointPainter::Data(0, 0, 0, Y_AXIS_RGBA);
	  axes_data[1].point2 = PointPainter::Data(0, grid_dis, 0, Y_AXIS_RGBA);
	  axes_data[2].point1 = PointPainter::Data(0, 0, 0, Z_AXIS_RGBA);
	  axes_data[2].point2 = PointPainter::Data(0, 0, grid_dis, Z_AXIS_RGBA);

  }

  coordinate_axes_painter_.Upload(axes_data);
}
void ModelViewerWidget::uploadPictureVetex() 
{
	  makeCurrent();

  const float scale = ZoomScale();

  
  std::vector<LinePainter::Data> axes_data(4);
  
  axes_data[0].point1 = PointPainter::Data(1, -1, -1, X_AXIS_RGBA);
  axes_data[0].point2 = PointPainter::Data(-1, -1, -1, X_AXIS_RGBA);

  axes_data[1].point1 = PointPainter::Data(-1, -1, -1, Y_AXIS_RGBA);
  axes_data[1].point2 = PointPainter::Data(-1, 1, -1, Y_AXIS_RGBA);

  axes_data[2].point1 = PointPainter::Data(-1, 1, -1, Z_AXIS_RGBA);
  axes_data[2].point2 = PointPainter::Data(+1, +1, -1, Z_AXIS_RGBA);

  axes_data[3].point1 = PointPainter::Data(+1, +1, -1, Z_AXIS_RGBA);
  axes_data[3].point2 = PointPainter::Data(1, -1, -1, Z_AXIS_RGBA);

  my_picture_vetex.Upload(axes_data);

}


void ModelViewerWidget::UploadPointDataNew(const bool selection_mode)
{

	makeCurrent();

	std::vector<PointPainter::Data> data;

	
	data.reserve(points3D.size());

	const size_t min_track_len =
		static_cast<size_t>(options_.min_track_len);

	if (selected_image_id_ == kInvalidImageId &&
		images.count(selected_image_id_) == 0) 
	{
		for (const auto& point3D : points3D)
		{
			if (!point3D.second.HasXYZ())
				continue;
			
			
			{
				PointPainter::Data painter_point;

				painter_point.x = static_cast<float>(point3D.second.GetXYZ(0));
				painter_point.y = static_cast<float>(point3D.second.GetXYZ(1));
				painter_point.z = static_cast<float>(point3D.second.GetXYZ(2));

				Eigen::Vector4f color;
				if (selection_mode) 
				{
					const size_t index = selection_buffer_.size();
					selection_buffer_.push_back(
						std::make_pair(point3D.first, SELECTION_BUFFER_POINT));
					color = IndexToRGB(index);

				}
				else if (point3D.first == selected_point3D_id_)
				{
					color = kSelectedPointColor;
				}
				else
				{
					Eigen::Vector3f& rgb = point_colormap_->ComputeColor(point3D.first, point3D.second);
					color(3) = 1.0;
					color(0) = rgb.x();
					color(1) = rgb.y();
					color(2) = rgb.z();
				}

				painter_point.r = color(0);
				painter_point.g = color(1);
				painter_point.b = color(2);
				painter_point.a = color(3);

				data.push_back(painter_point);
			}
		}
	}
	else
	{  
		const auto& selected_image = images[selected_image_id_];
		for (const auto& point3D : points3D) 
		{

			{
				PointPainter::Data painter_point;

				painter_point.x = static_cast<float>(point3D.second.GetXYZ(0));
				painter_point.y = static_cast<float>(point3D.second.GetXYZ(1));
				painter_point.z = static_cast<float>(point3D.second.GetXYZ(2));

				Eigen::Vector4f color;
				if (selection_mode)
				{
					const size_t index = selection_buffer_.size();
					selection_buffer_.push_back(
						std::make_pair(point3D.first, SELECTION_BUFFER_POINT));
					color = IndexToRGB(index);
				}
				else if (selected_image.HasPoint3D(point3D.first))
				{
					color = kSelectedImagePlaneColor;
				}
				else if (point3D.first == selected_point3D_id_)
				{
					color = kSelectedPointColor;
				}
				else
				{
					
					Eigen::Vector3f& rgb = point_colormap_->ComputeColor(point3D.first, point3D.second);
					color(3) = 1.0;
					color(0) = rgb.x();
					color(1) = rgb.y();
					color(2) = rgb.z();
				}

				painter_point.r = color(0);
				painter_point.g = color(1);
				painter_point.b = color(2);
				painter_point.a = color(3);

				data.push_back(painter_point);
			}
		}
	}
	
	
	{
		m_data_point_painter.insert(m_data_point_painter.end(), data.begin(), data.end());
	}
	
}


void ModelViewerWidget::UploadPointConnectionData() 
{
  makeCurrent();

  std::vector<LinePainter::Data> line_data;

  if (selected_point3D_id_ == kInvalidPoint3DId) 
  {
    
    point_connection_painter_.Upload(line_data);
    return;
  }

  const auto& point3D = points3D[selected_point3D_id_];

  
  LinePainter::Data line;
  line.point1 = PointPainter::Data(
      static_cast<float>(point3D.GetXYZ(0)), static_cast<float>(point3D.GetXYZ(1)),
      static_cast<float>(point3D.GetXYZ(2)), POINT_SELECTED_R, POINT_SELECTED_G,
      POINT_SELECTED_B, 0.8);

  
  for (const auto& track_el : point3D.GetTrack().GetElements())
  {
    const AI3D::CORE::Image& conn_image = images[track_el.image_id];
    const Eigen::Vector3f conn_proj_center =
        conn_image.GetProjectionCenter().cast<float>();
	


    line.point2 = PointPainter::Data(conn_proj_center(0), conn_proj_center(1),
                                     conn_proj_center(2), POINT_SELECTED_R,
                                     POINT_SELECTED_G, POINT_SELECTED_B, 1);
    line_data.push_back(line);
  }

  point_connection_painter_.Upload(line_data);
}

void ModelViewerWidget::UploadImageData(const bool selection_mode,const bool changetri)
{
  makeCurrent();
  std::map<group_t, std::vector<std::pair<image_t, Eigen::Vector4f>> > groups;
  std::vector<LinePainter::Data> line_data;
  line_data.reserve(8 * image_ids_.size());

  std::vector<TrianglePainter::Data> triangle_data;
  triangle_data.reserve(2 * image_ids_.size());
  std::vector<PointPainter::Data> point_data;
  point_data.reserve( image_ids_.size());
  int cnt = 0;

  haspos_ids_.clear();
  std::vector <  image_t> nopos_ids;
  if (!changetri)
  {
	  for (image_t image_id : image_ids_)
	  {
		  AI3D::CORE::Image& image = images[image_id];

		  

		  AI3D::CORE::Camera& camera = cameras[image.GetCameraId()];

		  float r, g, b, a;
		  
		  if ((image_size_ > 0) && image.GetPosition() != Eigen::Vector3d::Zero() && image.GetRotationMatrix() != Eigen::Matrix3d::Zero()
			  && camera.GetParams(0) != 0)
		  {
			  if (selection_mode) 
			  {
				  const size_t index = selection_buffer_.size();
				  selection_buffer_.push_back(
					  std::make_pair(image_id, SELECTION_BUFFER_IMAGE));
				  IndexToRGB(index, r, g, b);
				  a = 1;
			  }
			  else 
			  {
				  if (image_id == selected_image_id_) 
				  {
					  r = IMAGE_SELECTED_R;
					  g = IMAGE_SELECTED_G;
					  b = IMAGE_SELECTED_B;
					  a = IMAGE_SELECTED_A;
				  }
				  else if (!image.IsRegistered())
				  {
					  r = IMAGE_POINTSTYLE_R;
					  g = IMAGE_POINTSTYLE_G;
					  b = IMAGE_POINTSTYLE_B;
					  a = IMAGE_A;
				  }
				  else 
				  {
					  r = IMAGE_R;
					  g = IMAGE_G;
					  b = IMAGE_B;
					  a = IMAGE_A;
				  }			  
			  }			 
			  Eigen::Vector4f rgb{ r,g,b,a };
			  haspos_ids_.push_back(std::make_pair(image_id, rgb));
			  groups[image.GetPhotoGroupID()].push_back(std::make_pair(image_id, rgb));
		  }
		  else
		  {
			  nopos_ids.push_back(image_id);

		  }
	  }
 
  
	  for (auto& image_id : nopos_ids)
	  {
		 

		  const AI3D::CORE::Image& image = images[image_id];
		  PointPainter::Data painter_point;
		  TrianglePainter::Data triangle1, triangle2;
		  Eigen::Vector3d center = image.GetPosition();

		  painter_point.x = static_cast<float>(center.x());
		  painter_point.y = static_cast<float>(center.y());
		  painter_point.z = static_cast<float>(center.z());
		  if (selection_mode) 
		  {
			  const size_t index = selection_buffer_.size();
			  selection_buffer_.push_back(
				  std::make_pair(image_id, SELECTION_BUFFER_IMAGE));
			  IndexToRGB(index, painter_point.r, painter_point.g, painter_point.b);
			  painter_point.a = 1;
		  }
		  else
		  {
			  if (image_id == selected_image_id_)
			  {
				  Eigen::Vector4f color = kSelectedImagePlaneColor;
				  painter_point.r = color(0);
				  painter_point.g = color(1);
				  painter_point.b = color(2);
				  painter_point.a = color(3);
			  }
			  else
			  {
				 
				  painter_point.r = IMAGE_POINTSTYLE_R;
				  painter_point.g = IMAGE_POINTSTYLE_G;
				  painter_point.b = IMAGE_POINTSTYLE_B;
				  painter_point.a = 1;
			  }
		  }
		  float radus = kInitImageSize*0.3;
		  Eigen::Vector3f tl, tr, bl, br;
		  tl.x() = painter_point.x - radus;
		  tl.y() = painter_point.y - radus;
		  tl.z() = painter_point.z ;
		  tr.x() = painter_point.x + radus;
		  tr.y() = painter_point.y - radus;
		  tr.z() = painter_point.z;
		  bl.x() = painter_point.x - radus;
		  bl.y() = painter_point.y + radus;
		  bl.z() = painter_point.z;
		  br.x() = painter_point.x + radus;
		  br.y() = painter_point.y + radus;
		  br.z() = painter_point.z;
		  triangle1.point1 = PointPainter::Data(tl(0), tl(1), tl(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);
		  triangle1.point2 = PointPainter::Data(tr(0), tr(1), tr(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);
		  triangle1.point3 = PointPainter::Data(bl(0), bl(1), bl(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);

		  triangle2.point1 = PointPainter::Data(bl(0), bl(1), bl(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);
		  triangle2.point2 = PointPainter::Data(tr(0), tr(1), tr(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);
		  triangle2.point3 = PointPainter::Data(br(0), br(1), br(2), painter_point.r, painter_point.g, painter_point.b, painter_point.a);
		  
		  triangle_data.push_back(triangle2);
		  triangle_data.push_back(triangle1);
	  }
  }


  for (auto& image_id : haspos_ids_)
  {
	 
	 

	  const AI3D::CORE::Image& image = images[image_id.first];
	  const AI3D::CORE::Camera& camera = cameras[image.GetCameraId()];
	  LinePainter::Data line1, line2, line3, line4, line5, line6, line7, line8;
	  TrianglePainter::Data triangle1, triangle2;
	 BuildImageModel(image, camera, image_size_, image_id.second(0), image_id.second(1), image_id.second(2), image_id.second(3), line1, line2, line3,
		  line4, line5, line6, line7, line8, triangle1, triangle2);


	  

	  
	  
	 
	  
		  line_data.push_back(line1);
		  line_data.push_back(line2);
		  line_data.push_back(line3);
		  line_data.push_back(line4);
		  line_data.push_back(line5);
		  line_data.push_back(line6);
		  line_data.push_back(line7);
		  line_data.push_back(line8);
	  

	  triangle_data.push_back(triangle1);
	  triangle_data.push_back(triangle2);
  }
	image_line_painter_.Upload(line_data);
	  image_triangle_painter_.Upload(triangle_data);

  if (!point_data.empty())
  {
	  
	  m_data_point_painter.insert(m_data_point_painter.end(), point_data.begin(), point_data.end());
  }
  
}

void ModelViewerWidget::UploadCircle() 
{
	makeCurrent();
	std::vector<LinePainter::Data> line_data;

	
	int radius = 1;
	double scale_r = 0.08;
	int numline = 50;
	for(auto &center :gcpPoints) 
	{
		QList< QVector3D > vertexLine;
		if (center.type == 1)
		{
			numline = 50;
		}
		else
		{
			numline = 50;
		}
		
		for (int i = 0; i < numline; i++)
		{
			
			float angle = 2 * std::_Pi * i / numline;

			double x = center.point.x() + cos(angle) * radius * scale_r;
			double z = center.point.z() + sin(angle) * radius * scale_r;
			vertexLine.append(QVector3D(x, center.point.y(), z));
		}
		for (int i = 0; i < vertexLine.size()-1; i++) 
		{
			LinePainter::Data line;
			line.point1 = PointPainter::Data(vertexLine[i].x(), vertexLine[i].y(), vertexLine[i].z(), center.rgb.redF(), center.rgb.greenF(), center.rgb.blueF(), 1);
			line.point2 = PointPainter::Data(vertexLine[i+1].x(), vertexLine[i+1].y(), vertexLine[i+1].z(), center.rgb.redF(), center.rgb.greenF(), center.rgb.blueF(), 1);
			line_data.push_back(line);
		}

		float r_line = 0.4;
		float g_line = 0.6;
		float b_line = 0.6;
		double radius1 = radius * scale_r;
		LinePainter::Data line1;
		line1.point1 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z(), r_line, g_line, b_line, 1);
		line1.point2 = PointPainter::Data(center.point.x() + radius1, center.point.y(), center.point.z(), r_line, g_line, b_line, 1);

		LinePainter::Data line2;
		line2.point1 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z(), r_line, g_line, b_line, 1);
		line2.point2 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z() + radius1, r_line, g_line, b_line, 1);

		LinePainter::Data line3;
		line3.point1 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z(), r_line, g_line, b_line, 1);
		line3.point2 = PointPainter::Data(center.point.x()- radius1, center.point.y(), center.point.z(), r_line, g_line, b_line, 1);

		LinePainter::Data line4;
		line4.point1 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z(), r_line, g_line, b_line, 1);
		line4.point2 = PointPainter::Data(center.point.x(), center.point.y(), center.point.z() - radius1, r_line, g_line, b_line, 1);

		line_data.push_back(line1);
		line_data.push_back(line2);
		line_data.push_back(line3);
		line_data.push_back(line4);

	}
	circle_gcp.Upload(line_data);
}


void ModelViewerWidget::UploadImageConnectionData() {
  makeCurrent();

  std::vector<LinePainter::Data> line_data;
  std::vector<image_t> image_ids;

  if (selected_image_id_ != kInvalidImageId)
  {
    
    image_ids.push_back(selected_image_id_);
  } 
  else if (options_.image_connections) 
  {
    
    image_ids = image_ids_;
  } 
  else 
  {  
    image_connection_painter_.Upload(line_data);
    return;
  }

  for (const image_t image_id : image_ids) 
  {
    const AI3D::CORE::Image& image = images.at(image_id);
	
    const Eigen::Vector3f proj_center = image.GetProjectionCenter().cast<float>();
    
    std::unordered_set<image_t> conn_image_ids;

    for (const AI3D::CORE::Point2D& point2D : image.GetPoints2D()) 
	{
      if (point2D.HasPoint3D())
	  {
        const AI3D::CORE::Point3D& point3D = points3D[point2D.GetPoint3DId()];
        for (const auto& track_elem : point3D.GetTrack().GetElements()) 
		{
          conn_image_ids.insert(track_elem.image_id);
        }
      }
    }

    
    LinePainter::Data line;
    line.point1 = PointPainter::Data(proj_center(0), proj_center(1),
                                     proj_center(2), IMAGE_SELECTED_R,
                                     IMAGE_SELECTED_G, IMAGE_SELECTED_B, 0.8);

    
    for (const image_t conn_image_id : conn_image_ids) 
	{

      const AI3D::CORE::Image& conn_image = images[conn_image_id];

      const Eigen::Vector3f conn_proj_center =
          conn_image.GetProjectionCenter().cast<float>();

      line.point2 = PointPainter::Data(conn_proj_center(0), conn_proj_center(1),
                                       conn_proj_center(2), IMAGE_SELECTED_R,
                                       IMAGE_SELECTED_G, IMAGE_SELECTED_B, 0.8);
      line_data.push_back(line);
    }
  }

  image_connection_painter_.Upload(line_data);
}


void ModelViewerWidget::ComposeProjectionMatrix() 
{
  projection_matrix_.setToIdentity();

  if (options_.projection_type == RenderOptions::ProjectionType::PERSPECTIVE) 
  {
	  
	projection_matrix_.perspective(kFieldOfView, AspectRatio(), near_plane_,kFarPlane);


  } else if (options_.projection_type ==RenderOptions::ProjectionType::ORTHOGRAPHIC) 
  {
    const float extent = OrthographicWindowExtent();
    projection_matrix_.ortho(-AspectRatio() * extent, AspectRatio() * extent, -extent, extent, near_plane_, kFarPlane);
  }
}

float ModelViewerWidget::ZoomScale() const 
{
  
  return 2.0f * std::tan(static_cast<float>(DegToRad(kFieldOfView)) / 2.0f) *
         std::abs(focus_distance_) / height();
}

float ModelViewerWidget::AspectRatio() const 
{
  return static_cast<float>(width()) / static_cast<float>(height());
}

float ModelViewerWidget::OrthographicWindowExtent() const 
{
  return std::tan(DegToRad(kFieldOfView) / 2.0f) * focus_distance_;
}

Eigen::Vector3f ModelViewerWidget::PositionToArcballVector(
    const float x, const float y) const 
{
  Eigen::Vector3f vec(2.0f * x / width() - 1, 1 - 2.0f * y / height(), 0.0f);
  const float norm2 = vec.squaredNorm();
  if (norm2 <= 1.0f) 
  {
    vec.z() = std::sqrt(1.0f - norm2);
  }
  else 
  {
    vec = vec.normalized();
  }
  return vec;
}




void ModelViewerWidget::requestImageDelete()
{
	
	if (selected_image_id_ != kInvalidImageId && images.count(selected_image_id_) == 1)
	{
		if (reconstruction->ExistsImage(selected_image_id_))
		{

			QString name = str2qstr(const_cast<std::string &>(images[selected_image_id_].GetName()));

			reconstruction->DeleteImage(selected_image_id_);
			
			emit update_delete_image((point3D_t)selected_image_id_, name);
			*item_select_ = kInvalidImageId;
			selected_image_id_ = kInvalidImageId;
			use_preview_view_ = true;
			ReloadReconstruction();
			updateShow();
			
			
		}
	}
	else if  (selected_point3D_id_ != kInvalidPoint3DId && points3D.count(selected_point3D_id_)==1)
	{	
		reconstruction->DeletePoint3D(selected_point3D_id_);
		
		QString msg = "tiepoints";
		emit update_delete_image(selected_point3D_id_, msg);;
		selected_point3D_id_ = kInvalidPoint3DId;
		
		use_preview_view_ = true;
		ReloadReconstruction();
		updateShow();
		
		
	}
}

void ModelViewerWidget::keyPressEvent(QKeyEvent *e) 
{
	if (e->key() == Qt::Key_Delete) 
		requestImageDelete();
	   
	e->accept();
}

}
}
