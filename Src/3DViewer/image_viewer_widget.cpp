#include "3DViewer/image_viewer_widget.h"

#include "3DViewer/model_viewer_widget.h"
#include "Core/File.h"
#include "Util/TaskProcess.h"


namespace AI3D
{
	namespace GUI
	{

		const double ImageViewerWidget::kZoomFactor = 1.20;

		ImageViewerGraphicsScene::ImageViewerGraphicsScene() {
			setSceneRect(0, 0, 0, 0);
			image_pixmap_item_ = addPixmap(QPixmap::fromImage(QImage()));
			image_pixmap_item_->setZValue(-1);
		}

		QGraphicsPixmapItem* ImageViewerGraphicsScene::ImagePixmapItem() const {
			return image_pixmap_item_;
		}

		ImageViewerWidget::ImageViewerWidget(QWidget* parent) : QWidget(parent) {
			
			
			

			resize(parent->width() - 700, parent->height() + 200);

			QFont font;
			font.setPointSize(10);
			setFont(font);

			grid_layout_ = new QGridLayout(this);
			grid_layout_->setContentsMargins(5, 5, 5, 5);

			graphics_view_ = new QGraphicsView();
			graphics_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

			graphics_view_->setScene(&graphics_scene_);
			graphics_view_->setAlignment(Qt::AlignLeft | Qt::AlignTop);

			grid_layout_->addWidget(graphics_view_, 0, 0);



			button_layout_ = new QHBoxLayout();

			
			m_image_view = nullptr;

			QLabel* label_view_image = new QLabel(this);
			label_view_image->setFont(font);
			label_view_image->setFixedWidth(50);
			label_view_image->setObjectName(QString::fromUtf8("label_view_image"));
			label_view_image->setText(QApplication::translate("ImageViewerWidget", "<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline; color:#0000ff;\">View</span></a></p></body></html>", nullptr));
			
			connect(label_view_image, &QLabel::linkActivated, this, &ImageViewerWidget::slot_linkActivated_label_view_image);
			label_view_image->setVisible(false);
			button_layout_->addWidget(label_view_image);
			

			QPushButton* zoom_in_button = new QPushButton("+", this);
			zoom_in_button->setFont(font);
			zoom_in_button->setFixedWidth(50);
			button_layout_->addWidget(zoom_in_button);
			connect(zoom_in_button, &QPushButton::released, this,
				&ImageViewerWidget::ZoomIn);

			QPushButton* zoom_out_button = new QPushButton("-", this);
			zoom_out_button->setFont(font);
			zoom_out_button->setFixedWidth(50);
			button_layout_->addWidget(zoom_out_button);
			connect(zoom_out_button, &QPushButton::released, this,
				&ImageViewerWidget::ZoomOut);

			QPushButton* save_button = new QPushButton("Save image", this);
			save_button->setFont(font);
			button_layout_->addWidget(save_button);
			connect(save_button, &QPushButton::released, this, &ImageViewerWidget::Save);
			QHBoxLayout* layout = new QHBoxLayout;
			grid_layout_->addLayout(layout, 1, 0, Qt::AlignRight);
		}

		void ImageViewerWidget::resizeEvent(QResizeEvent* event) {
			QWidget::resizeEvent(event);

			graphics_view_->fitInView(graphics_scene_.sceneRect(), Qt::KeepAspectRatio);
		}

		void ImageViewerWidget::closeEvent(QCloseEvent* event) {
			graphics_scene_.ImagePixmapItem()->setPixmap(QPixmap());
			
			QLabel* pLabel = qobject_cast<QLabel*>(button_layout_->itemAt(0)->widget());
			if (pLabel)
				pLabel->setVisible(false);
			
		}

		void ImageViewerWidget::ShowBitmap(const AI3D::CORE::Bitmap& bitmap) {
			ShowPixmap(QPixmap::fromImage(BitmapToQImageRGB(bitmap)));
		}

		void ImageViewerWidget::ShowPixmap(const QPixmap& pixmap) {
			graphics_scene_.ImagePixmapItem()->setPixmap(pixmap);
			graphics_scene_.setSceneRect(pixmap.rect());

			show();
			graphics_view_->fitInView(graphics_scene_.sceneRect(), Qt::KeepAspectRatio);

			raise();

			
			QLabel* pLabel = qobject_cast<QLabel*>(button_layout_->itemAt(0)->widget());
			if (pLabel)
				pLabel->setVisible(true);
			

		}

		void ImageViewerWidget::ReadAndShow(const std::string& path) {
			AI3D::CORE::Bitmap bitmap;
			if (!bitmap.Read(path, true)) {
				std::cerr << "ERROR: Cannot read image at path " << path << std::endl;
			}

			ShowBitmap(bitmap);
		}

		void ImageViewerWidget::ZoomIn() {
			graphics_view_->scale(kZoomFactor, kZoomFactor);
		}

		void ImageViewerWidget::ZoomOut() {
			graphics_view_->scale(1.0 / kZoomFactor, 1.0 / kZoomFactor);
		}

		void ImageViewerWidget::Save() {
			QString filter("PNG (*.png)");
			const QString save_path =
				QFileDialog::getSaveFileName(this, tr("Select destination..."), "",
					"PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)",
					&filter)
				.toUtf8()
				.constData();

			
			if (save_path == "") {
				return;
			}

			graphics_scene_.ImagePixmapItem()->pixmap().save(save_path);
		}

		
		void ImageViewerWidget::slot_linkActivated_label_view_image(QString link) {
			QPixmap pixmap = graphics_scene_.ImagePixmapItem()->pixmap();
			if (m_image_view) {
				delete m_image_view;
				m_image_view = nullptr;
			}

			m_image_view = new CImageViewWgt();

			m_image_view->show();
			m_image_view->setImage(pixmap);
		}

		

		FeatureImageViewerWidget::FeatureImageViewerWidget(
			QWidget* parent, const std::string& switch_text)
			: ImageViewerWidget(parent),
			switch_state_(true),
			switch_text_(switch_text) {
			switch_button_ = new QPushButton(tr(("Hide " + switch_text_).c_str()), this);
			switch_button_->setFont(font());
			button_layout_->addWidget(switch_button_);
			connect(switch_button_, &QPushButton::released, this,
				&FeatureImageViewerWidget::ShowOrHide);
		}

		void FeatureImageViewerWidget::ReadAndShowWithKeypoints(
			const std::string& path, const FeatureKeypoints& keypoints,
			const std::vector<char>& tri_mask) {

			
			
			
			
			

			QPixmap tmpPixmap;
			if (!tmpPixmap.load(path.c_str()))
				return;

			image1_ = tmpPixmap;

			image2_ = image1_;

			const size_t num_tri_keypoints = std::count_if(
				tri_mask.begin(), tri_mask.end(), [](const bool tri) { return tri; });

			FeatureKeypoints keypoints_tri(num_tri_keypoints);
			FeatureKeypoints keypoints_not_tri(keypoints.size() - num_tri_keypoints);
			size_t i_tri = 0;
			size_t i_not_tri = 0;
			for (size_t i = 0; i < tri_mask.size(); ++i) {
				if (tri_mask[i]) {
					keypoints_tri[i_tri] = keypoints[i];
					i_tri += 1;
				}
				else {
					keypoints_not_tri[i_not_tri] = keypoints[i];
					i_not_tri += 1;
				}
			}

			DrawKeypoints(&image2_, keypoints_tri, Qt::magenta);
			DrawKeypoints(&image2_, keypoints_not_tri, Qt::red);

			if (switch_state_) {
				ShowPixmap(image2_);
			}
			else {
				ShowPixmap(image1_);
			}
		}

		void FeatureImageViewerWidget::ReadAndShowWithMatches(
			const std::string& path1, const std::string& path2,
			const FeatureKeypoints& keypoints1, const FeatureKeypoints& keypoints2,
			const FeatureMatches& matches) {
			AI3D::CORE::Bitmap bitmap1;
			AI3D::CORE::Bitmap bitmap2;
			if (!bitmap1.Read(path1, true) || !bitmap2.Read(path2, true)) {
				std::cerr << "ERROR: Cannot read images at paths " << path1 << " and "
					<< path2 << std::endl;
				return;
			}

			const auto image1 = QPixmap::fromImage(BitmapToQImageRGB(bitmap1));
			const auto image2 = QPixmap::fromImage(BitmapToQImageRGB(bitmap2));

			image1_ = ShowImagesSideBySide(image1, image2);
			image2_ = DrawMatches(image1, image2, keypoints1, keypoints2, matches);

			if (switch_state_) {
				ShowPixmap(image2_);
			}
			else {
				ShowPixmap(image1_);
			}
		}

		void FeatureImageViewerWidget::ShowOrHide() {
			if (switch_state_) {
				switch_button_->setText(std::string("Show " + switch_text_).c_str());
				ShowPixmap(image1_);
				switch_state_ = false;
			}
			else {
				switch_button_->setText(std::string("Hide " + switch_text_).c_str());
				ShowPixmap(image2_);
				switch_state_ = true;
			}
		}

		DatabaseImageViewerWidget::DatabaseImageViewerWidget(
			QWidget* parent, ModelViewerWidget* model_viewer_widget)
			: FeatureImageViewerWidget(parent, "keypoints"),
			model_viewer_widget_(model_viewer_widget)
			
		{
			setWindowTitle("Image information");

			table_widget_ = new QTableWidget(this);
			table_widget_->setColumnCount(2);
			table_widget_->setRowCount(6);

			QFont font;
			font.setPointSize(10);
			table_widget_->setFont(font);

			table_widget_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

			table_widget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
			table_widget_->setSelectionMode(QAbstractItemView::SingleSelection);
			table_widget_->setShowGrid(true);

			table_widget_->horizontalHeader()->setStretchLastSection(true);
			table_widget_->horizontalHeader()->setVisible(false);
			table_widget_->verticalHeader()->setVisible(false);
			table_widget_->verticalHeader()->setDefaultSectionSize(18);

			int table_row = 0;

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("image_id"));
			image_id_item_ = new QTableWidgetItem();
			table_widget_->setItem(table_row, 1, image_id_item_);
			table_row += 1;

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("camera_id"));
			camera_id_item_ = new QTableWidgetItem();
			table_widget_->setItem(table_row, 1, camera_id_item_);
			table_row += 1;

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("camera_model"));
			camera_model_item_ = new QTableWidgetItem();
			table_widget_->setItem(table_row, 1, camera_model_item_);
			table_row += 1;

			
			
			
			

			
			
			
			

			
			
			
			

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("dims"));
			dimensions_item_ = new QTableWidgetItem();
			table_widget_->setItem(table_row, 1, dimensions_item_);
			table_row += 1;

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("num_points2D"));
			num_points2D_item_ = new QTableWidgetItem();
			num_points2D_item_->setForeground(Qt::red);
			table_widget_->setItem(table_row, 1, num_points2D_item_);
			table_row += 1;

			table_widget_->setItem(table_row, 0, new QTableWidgetItem("num_points3D"));
			num_points3D_item_ = new QTableWidgetItem();
			num_points3D_item_->setForeground(Qt::magenta);
			table_widget_->setItem(table_row, 1, num_points3D_item_);
			table_row += 1;

			
			
			
			
			

			
			
			
			

			

			delete_button_ = new QPushButton(tr("Delete"), this);
			delete_button_->setFont(font);
			button_layout_->addWidget(delete_button_);
			connect(delete_button_, &QPushButton::released, this,
				&DatabaseImageViewerWidget::DeleteImage);

			imageForm_ = new ImagePropertyForm(this);

			QWidget* widget = new QWidget;
			grid_layout_->addWidget(widget, 2, 0);

			ResizeTable();

		}

		void DatabaseImageViewerWidget::ShowImageWithId(const image_t image_id) {
			if (model_viewer_widget_->images.count(image_id) == 0) {
				return;
			}

			image_id_ = image_id;

			const AI3D::CORE::Image& image = model_viewer_widget_->images.at(image_id);
			const AI3D::CORE::Camera& camera = model_viewer_widget_->cameras.at(image.GetCameraId());

			image_id_item_->setText(QString::number(image_id));
			camera_id_item_->setText(QString::number(image.GetCameraId()));
			camera_model_item_->setText(QString::fromStdString(camera.GetModelName()));
			
			 
			 
			 
			 
			 
			 
			 
			dimensions_item_->setText(QString::number(camera.GetWidth()) + "x" +
				QString::number(camera.GetHeight()));
			num_points2D_item_->setText(QString::number(image.GetNumPoints2D()));

			std::vector<char> tri_mask(image.GetNumPoints2D());
			for (size_t i = 0; i < image.GetNumPoints2D(); ++i) {
				tri_mask[i] = image.GetPoint2D(i).HasPoint3D();
			}

			num_points3D_item_->setText(QString::number(image.GetNumPoints3D()));
			
			

			imageForm_->setFileName(str2qstr(const_cast<std::string &>(image.GetName())));

			imageForm_->setPos(image.GetProjectionCenter(), image.GetRotationMatrix());
			imageForm_->setGroup(camera.GetFocalLength(), camera.GetPrincipalPointX(), camera.GetPrincipalPointY());

			ResizeTable();

			FeatureKeypoints keypoints(image.GetNumPoints2D());
			for (point2D_t i = 0; i < image.GetNumPoints2D(); ++i) {
				keypoints[i].x = static_cast<float>(image.GetPoint2D(i).GetX());
				keypoints[i].y = static_cast<float>(image.GetPoint2D(i).GetY());
			}

			const std::string path = AI3D::CORE::File::JoinPaths(image.GetPath(), image.GetName());
			ReadAndShowWithKeypoints(path, keypoints, tri_mask);
		}

		void DatabaseImageViewerWidget::ResizeTable() {
			
			table_widget_->resizeColumnsToContents();
			int height = table_widget_->horizontalHeader()->height() +
				2 * table_widget_->frameWidth();
			for (int i = 0; i < table_widget_->rowCount(); i++) {
				height += table_widget_->rowHeight(i);
			}
			table_widget_->setFixedHeight(height);
		}

		void DatabaseImageViewerWidget::DeleteImage() {
			QMessageBox::StandardButton reply = QMessageBox::question(
				this, "", tr("Do you really want to delete this image?"));
			if (reply == QMessageBox::Yes) {

				
				model_viewer_widget_->requestImageDelete();

				

				 
				
				
				
				
				
				
			}

		}

	} 
}




CImageViewWgt::CImageViewWgt(QWidget * parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_ShowModal, true);
	ui.setupUi(this);
}

void CImageViewWgt::setImage(const QPixmap& pixmap) {

	ui.graphicsView->addImage(pixmap);


}
