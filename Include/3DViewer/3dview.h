
#ifndef _AI3D_GUI_3DVIEW_H_
#define _AI3D_GUI_3DVIEW_H_


#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QtWidgets>
#include <QString>
#include <QList>
#include <QRunnable> 
#include <QThread>
#include <QCheckBox>
#include "Core/File.h"
#include "Core/ATData.h"

#include "3DViewer/render_options.h"

#include "3DViewer/model_viewer_widget.h"



#include "Core/Bitmap.h"

#include "Core/Point3d.h"
#include "Core/BlockObject.h"
#include "3DViewer/3dview.h"
#include "Gui/BlockWgt.h"

#ifdef USE_OSGVIEWER

#include "3DViewer/ModelViewer/osgThread.h"

#endif 



namespace Ui {
	class ViewWidget;
}

	namespace AI3D
	{
		namespace GUI
		{

			
			class MWindow;
			
			
			class ViewWidget : public QMainWindow
			{

				Q_OBJECT
			public:
				ViewWidget(AI3D::CORE::BlockObject* blockdata, RenderOptions options = RenderOptions(), QWidget* parent = 0);
				
				~ViewWidget();
				void addTextEditData(QString);
				RenderOptions& GetOptions() { return options_; }
				void addTextEditData(QMatrix4x4 matrix);
				void showAT3dview();
				image_t* item_select_ = nullptr;
				void keyPressEvent(QKeyEvent* e) override;
				void SetLayerStatus();
				
				void RenderATDataWithSelectedImages(std::vector<image_t> &selectedImages);
				void RenderOsgFile(std::string fileName);
				void RenderModel(std::string path);

				void GetSelectedImages(std::vector<image_t>& selectedImages);
				void RestorePreviousState();

#ifdef USE_OSGVIEWER		
				osgThread* thread;
#endif		
				
				
				
				
			
				
				
			signals:
				
				
				
				
				void set_progress(int,QString);
			
				void signal_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names);
				void signal_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name);				

				void signal_selected_images_from_3dview(std::vector<image_t>& images);
				void signal_add_user_tie_point(const AI3D::CORE::Image &image,const QString& userTiePointName);
				void signal_insert_gcp_tab();

			public slots:
				void uploadGcpPoints();
#ifdef USE_OSGVIEWER
				void import_meshModel();
				void import_osgbModel();
				void onreadOver(QString msg);
#endif
				void addTextEidtListStr(QList<QString> str_list);
				void SetBlock(std::shared_ptr<AI3D::CORE::BlockObject> block);

				void send_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names);
				void send_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name);

				void Slot_right_selected_images_from_3dview(std::vector<image_t>& images);
				void Slot_selected_images_from_3dview(std::vector<image_t>& images);

			protected slots:
				void comboxChangeText(const QString&);
				void PhotoLayerChanged(const QString&);
				void TiepointsLayerChanged(const QString&);
				void SelectionModeChanged(const QString&);

				void Slot_ImageLayer(const QString &);
				void Slot_SelectType(const QString &);
				void Slot_ImageLayerCheckBoxStateChanged(int state);



			private:			
				std::vector<image_t> saved_images;
				RenderOptions options_;
				ModelSel model_select;
				ModelSel old_model_select;
				QScrollArea* m_pScroll = nullptr;
				QStackedWidget* stackedWidget = nullptr;
				ModelViewerWidget* model_viewer_widget_ = nullptr;
				
				QProgressDialog* progress_bar_ = nullptr;
				QProgressDialog* progress_bar_osg = nullptr;
				QStringList tri_path;
				AI3D::CORE::BlockObject* _blockdata = nullptr;
				AI3D::CORE::ATData* reconstruction = nullptr;
				AI3D::CORE::ATData* at_data = nullptr;
			public:
				MWindow* mWindow;
			private:
				Ui::ViewWidget* ui;
				
				QComboBox* m_pImageLayer;
				QComboBox* m_pSelectType;
				QCheckBox* m_pChkPhotos;
				QCheckBox* m_pChkTiePoints;
				QCheckBox* m_pChkGCP;	
				QLabel* lblSelectedImagesNum;
				QLabel* lblFirstSelectedImageName;
				QCheckBox* m_pChkRightClicked;
				bool bCheckBoxsInited;
			};

			

		}
	}
#endif