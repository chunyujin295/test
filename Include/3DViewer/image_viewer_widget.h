#ifndef _AI3D_GUI_IMAGE_VIEWER_WIDGET_H_
#define _AI3D_GUI_IMAGE_VIEWER_WIDGET_H_

#include <QtCore>
#include <QtWidgets>



#include "Core/ATData.h"
#include "3DViewer/qt_utils.h"

#include "3DViewer/imagepropertyform.h"
#include "3DViewer/featuretypes.h"
#include <ui_ImageViewWgt.h>

	
class CImageViewWgt : public QWidget
{
	Q_OBJECT

public:
	CImageViewWgt(QWidget * parent = Q_NULLPTR);

	void setImage(const QPixmap& pixmap);
private:
	Ui::ImageViewWgt ui;
};


namespace AI3D 
{
    namespace GUI
    {
        class ModelViewerWidget;

        class ImageViewerGraphicsScene : public QGraphicsScene {
        public:
            ImageViewerGraphicsScene();

            QGraphicsPixmapItem* ImagePixmapItem() const;

        private:
            QGraphicsPixmapItem* image_pixmap_item_ = nullptr;
        };

        class ImageViewerWidget : public QWidget {
        public:
            explicit ImageViewerWidget(QWidget* parent);

            void ShowBitmap(const AI3D::CORE::Bitmap& bitmap);
            void ShowPixmap(const QPixmap& pixmap);
            void ReadAndShow(const std::string& path);

        private:
            static const double kZoomFactor;

            
            CImageViewWgt* m_image_view;
            
            ImageViewerGraphicsScene graphics_scene_;
            QGraphicsView* graphics_view_;

        protected:
            void resizeEvent(QResizeEvent* event);
            void closeEvent(QCloseEvent* event);
            void ZoomIn();
            void ZoomOut();
            void Save();
            
            void slot_linkActivated_label_view_image(QString link);
            

            QGridLayout* grid_layout_;
            QHBoxLayout* button_layout_;

        };

        class FeatureImageViewerWidget : public ImageViewerWidget {
        public:
            FeatureImageViewerWidget(QWidget* parent, const std::string& switch_text);

            void ReadAndShowWithKeypoints(const std::string& path,
                const FeatureKeypoints& keypoints,
                const std::vector<char>& tri_mask);

            void ReadAndShowWithMatches(const std::string& path1,
                const std::string& path2,
                const FeatureKeypoints& keypoints1,
                const FeatureKeypoints& keypoints2,
                const FeatureMatches& matches);

        protected:
            void ShowOrHide();

            QPixmap image1_;
            QPixmap image2_;
            bool switch_state_;
            QPushButton* switch_button_;
            const std::string switch_text_;
        };

        class DatabaseImageViewerWidget : public FeatureImageViewerWidget {
        public:
            DatabaseImageViewerWidget(QWidget* parent,
                ModelViewerWidget* model_viewer_widget
                );

            void ShowImageWithId(const image_t image_id);

        private:
            void ResizeTable();
            void DeleteImage();

            ModelViewerWidget* model_viewer_widget_;

            

            QPushButton* delete_button_;

            image_t image_id_;

            QTableWidget* table_widget_;
            QTableWidgetItem* image_id_item_;
            QTableWidgetItem* camera_id_item_;
            QTableWidgetItem* camera_model_item_;
            QTableWidgetItem* camera_params_item_;
            QTableWidgetItem* qvec_item_;
            QTableWidgetItem* tvec_item_;
            QTableWidgetItem* dimensions_item_;
            QTableWidgetItem* num_points2D_item_;
            QTableWidgetItem* num_points3D_item_;
            QTableWidgetItem* num_obs_item_;
            QTableWidgetItem* name_item_;

            ImagePropertyForm* imageForm_;

        };
    }

} 

#endif  
