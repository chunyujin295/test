#include "3DViewer/point_viewer_widget.h"
#include "Core/AlgorithmBase.h"
#include "3DViewer/model_viewer_widget.h"
#include "Core/File.h"
#include "Core/Image.h"

namespace AI3D
{
    namespace GUI
    {

        PointViewerWidget::PointViewerWidget(QWidget* parent,
            ModelViewerWidget* model_viewer_widget)
            : QWidget(parent),
            model_viewer_widget_(model_viewer_widget),
           
            point3D_id_(kInvalidPoint3DId),
            zoom_(250.0 / 1024.0) {
            
            resize(parent->size().width() - 20, parent->size().height() - 20);

            QFont font;
            font.setPointSize(10);
            setFont(font);

            QGridLayout* grid = new QGridLayout(this);
            grid->setContentsMargins(5, 5, 5, 5);

            info_table_ = new QTableWidget(this);
            info_table_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            info_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
            info_table_->setSelectionMode(QAbstractItemView::SingleSelection);
            info_table_->setShowGrid(true);
            info_table_->horizontalHeader()->setStretchLastSection(true);
            info_table_->horizontalHeader()->setVisible(false);
            info_table_->verticalHeader()->setVisible(false);
            info_table_->verticalHeader()->setDefaultSectionSize(18);

            info_table_->setColumnCount(2);
            info_table_->setRowCount(3);

            info_table_->setItem(0, 0, new QTableWidgetItem("position"));
            xyz_item_ = new QTableWidgetItem();
            info_table_->setItem(0, 1, xyz_item_);

            info_table_->setItem(1, 0, new QTableWidgetItem("color"));
            rgb_item_ = new QTableWidgetItem();
            info_table_->setItem(1, 1, rgb_item_);

            info_table_->setItem(2, 0, new QTableWidgetItem("error"));
            error_item_ = new QTableWidgetItem();
            info_table_->setItem(2, 1, error_item_);

            grid->addWidget(info_table_, 0, 0);

            location_table_ = new QTableWidget(this);
            location_table_->setColumnCount(3);
            QStringList table_header;
            table_header << "image_id"
                << "reproj_error"
                << "track_location";
            location_table_->setHorizontalHeaderLabels(table_header);
            location_table_->resizeColumnsToContents();
            location_table_->setShowGrid(true);
            location_table_->horizontalHeader()->setStretchLastSection(true);
            location_table_->verticalHeader()->setVisible(true);
            location_table_->setSelectionMode(QAbstractItemView::NoSelection);
            location_table_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            location_table_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            location_table_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

            grid->addWidget(location_table_, 1, 0);

            QHBoxLayout* button_layout = new QHBoxLayout();

            zoom_in_button_ = new QPushButton(tr("+"), this);
            zoom_in_button_->setFont(font);
            zoom_in_button_->setFixedWidth(50);
            button_layout->addWidget(zoom_in_button_);
            connect(zoom_in_button_, &QPushButton::released, this,
                &PointViewerWidget::ZoomIn);

            zoom_out_button_ = new QPushButton(tr("-"), this);
            zoom_out_button_->setFont(font);
            zoom_out_button_->setFixedWidth(50);
            button_layout->addWidget(zoom_out_button_);
            connect(zoom_out_button_, &QPushButton::released, this,
                &PointViewerWidget::ZoomOut);

            delete_button_ = new QPushButton(tr("Delete"), this);
            button_layout->addWidget(delete_button_);
            connect(delete_button_, &QPushButton::released, this,
                &PointViewerWidget::Delete);

            grid->addLayout(button_layout, 2, 0, Qt::AlignRight);
        }

        void PointViewerWidget::Show(const point3D_t point3D_id) {
            location_pixmaps_.clear();
            image_ids_.clear();
            reproj_errors_.clear();

            if (model_viewer_widget_->points3D.count(point3D_id) == 0) {
                point3D_id_ = kInvalidPoint3DId;
                ClearLocations();
                return;
            }

            show();
            raise();

            point3D_id_ = point3D_id;

            setWindowTitle(QString::fromStdString("Point " + std::to_string(point3D_id)));

            const auto& point3D = model_viewer_widget_->points3D[point3D_id];

            xyz_item_->setText(QString::number(point3D.GetX()) + ", " +
                QString::number(point3D.GetY()) + ", " +
                QString::number(point3D.GetZ()));
            rgb_item_->setText(QString::number(point3D.GetColor(0)) + ", " +
                QString::number(point3D.GetColor(1)) + ", " +
                QString::number(point3D.GetColor(2)));
            error_item_->setText(QString::number(point3D.GetPixelRMS()));

            ResizeInfoTable();

            
            for (const auto& track_el : point3D.GetTrack().GetElements()) {
                const AI3D::CORE::Image& image = model_viewer_widget_->images[track_el.image_id];
                const AI3D::CORE::Camera& camera = model_viewer_widget_->cameras[image.GetCameraId()];
                const AI3D::CORE::Point2D& point2D = image.GetPoint2D(track_el.point2D_idx);
                
               const Eigen::Matrix3x4d proj_matrix = image.GetProjectionMatrix();
                const double error = AI3D::CORE::AlgorithmBase::CalculateReprojectionError(point2D.GetXY(), point3D.GetXYZ(),
                    proj_matrix, camera);
                std::string imagepath = image.GetPath();
                imagepath = AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(imagepath));
                const std::string path = AI3D::CORE::File::JoinPaths(imagepath, image.GetName());

                

                 
                QPixmap pixmap;
                if (!pixmap.load(path.c_str()))
                    continue;

                

                
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QPen pen;
                pen.setWidth(3);
                pen.setColor(Qt::red);
                painter.setPen(pen);
                painter.drawEllipse(static_cast<int>(point2D.GetX() - 5),
                    static_cast<int>(point2D.GetY() - 5), 10, 10);
                painter.drawEllipse(static_cast<int>(point2D.GetX() - 15),
                    static_cast<int>(point2D.GetY() - 15), 30, 30);
                painter.drawEllipse(static_cast<int>(point2D.GetX() - 45),
                    static_cast<int>(point2D.GetY() - 45), 90, 90);

                location_pixmaps_.push_back(pixmap);
                image_ids_.push_back(track_el.image_id);
                reproj_errors_.push_back(error);
            }

            UpdateImages();
        }

        void PointViewerWidget::closeEvent(QCloseEvent* event) {
            
            location_pixmaps_.clear();
            image_ids_.clear();
            reproj_errors_.clear();
            ClearLocations();
        }

        void PointViewerWidget::ResizeInfoTable() {
            
            info_table_->resizeColumnsToContents();
            int height =
                info_table_->horizontalHeader()->height() + 2 * info_table_->frameWidth();
            for (int i = 0; i < info_table_->rowCount(); i++) {
                height += info_table_->rowHeight(i);
            }
            info_table_->setFixedHeight(height);
        }

        void PointViewerWidget::ClearLocations() {
            while (location_table_->rowCount() > 0) {
                location_table_->removeRow(0);
            }
            for (auto location_label : location_labels_) {
                delete location_label;
            }
            location_labels_.clear();
        }

        void PointViewerWidget::UpdateImages() {
            ClearLocations();

            location_table_->setRowCount(static_cast<int>(location_pixmaps_.size()));

            for (size_t i = 0; i < location_pixmaps_.size(); ++i) {
                QLabel* image_id_label = new QLabel(QString::number(image_ids_[i]), this);
                location_table_->setCellWidget(i, 0, image_id_label);
                location_labels_.push_back(image_id_label);

                QLabel* error_label = new QLabel(QString::number(reproj_errors_[i]), this);
                location_table_->setCellWidget(i, 1, error_label);
                location_labels_.push_back(error_label);

                const QPixmap& pixmap = location_pixmaps_[i];
                QLabel* image_label = new QLabel(this);
                image_label->setPixmap(
                    pixmap.scaledToWidth(zoom_ * pixmap.width(), Qt::FastTransformation));
                location_table_->setCellWidget(i, 2, image_label);
                location_table_->resizeRowToContents(i);
                location_labels_.push_back(image_label);
            }
            location_table_->resizeColumnToContents(2);
        }

        void PointViewerWidget::ZoomIn() {
            zoom_ *= 1.33;
            UpdateImages();
        }

        void PointViewerWidget::ZoomOut() {
            zoom_ /= 1.3;
            UpdateImages();
        }

        void PointViewerWidget::Delete()
        {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "", tr("Do you really want to delete this point?"));

            if (reply == QMessageBox::Yes)
            {
                if (model_viewer_widget_->reconstruction->ExistsPoint3D(point3D_id_))
                {
                    
                }
                model_viewer_widget_->ReloadReconstruction();
            }
        }

    } 
}
