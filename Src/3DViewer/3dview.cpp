#include "3DViewer/3dview.h"
#include "Core/CoordinateSystem.h"

#include "Core/Timer.h"
#include "Core/Logging.h"
#include "ui_3dview.h"
#include <QtConcurrent> 
#include "OSGEditor/AT3DViewInterface.h"


#ifdef USE_OSGVIEWER
#include "3DViewer/ModelViewer/osgProgressBar.h"
#include "3DViewer/ModelViewer/osgThread.h"
#include "3DViewer/ModelViewer/OsgModelView.h"
#endif 
namespace AI3D
{
	namespace GUI
	{
		ViewWidget::ViewWidget(AI3D::CORE::BlockObject* blockdata, RenderOptions options, QWidget* parent) :
			QMainWindow(parent),
			model_select(CAMERAS_MODEL),
			progress_bar_(NULL),
			_blockdata(blockdata),
			options_(options),
			ui(new Ui::ViewWidget)
		{
			ui->setupUi(this);
			ui->menubar->setVisible(false);
#ifdef USE_OSGVIEWER
			ui->menubar->setVisible(true);
#endif 
			
		
			ui->verticalLayout->setMargin(0);
			ui->horizontalLayout->setMargin(0);
			ui->splitter->setContentsMargins(0, 0, 0, 0);
			
				m_pImageLayer = new QComboBox(this);
				m_pImageLayer->addItem("Photos");
				m_pImageLayer->addItem("TiePoints");
				m_pImageLayer->addItem("GCP");
				m_pImageLayer->setVisible(false);

				lblSelectedImagesNum = new QLabel(this);
				lblSelectedImagesNum->setText("0");

				lblFirstSelectedImageName = new QLabel(this);
				lblFirstSelectedImageName->setText("None.png");

				m_pChkRightClicked = new QCheckBox(this);
				m_pChkRightClicked->setText("ForceRightClick");

				lblSelectedImagesNum->setVisible(false);
				lblFirstSelectedImageName->setVisible(false);
				m_pChkRightClicked->setVisible(false);

				m_pChkPhotos = new QCheckBox(this);

				m_pChkPhotos->setChecked(true);

				m_pChkTiePoints = new QCheckBox(this);

				m_pChkTiePoints->setChecked(true);

				m_pChkGCP = new QCheckBox(this);

				m_pChkGCP->setChecked(true);

				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					ui->Itemlabel->setText("选择");
					m_pChkPhotos->setText("影像");
					m_pChkTiePoints->setText("连接点");
					m_pChkGCP->setText("控制点");
				}
				else
				{
					ui->Itemlabel->setText("Selection");
					m_pChkPhotos->setText("Photos");
					m_pChkTiePoints->setText("TiePoints");
					m_pChkGCP->setText("GCP");
				}

				auto current_at = blockdata->GetCurrentAT();
				if (current_at)
				{
					if (!current_at->HasControlPoints())
					{
						m_pChkGCP->setChecked(false);
						m_pChkGCP->setEnabled(false);
					}

					if (!current_at->HasImages())
					{
						m_pChkPhotos->setChecked(false);
						m_pChkPhotos->setEnabled(false);
					}
				}
				else
				{
					m_pChkGCP->setChecked(false);
					m_pChkGCP->setEnabled(false);
					m_pChkPhotos->setChecked(false);
					m_pChkPhotos->setEnabled(false);
				}

				if (!blockdata->GetTiepointFullStatus())
				{
					
					m_pChkTiePoints->setChecked(false);
					m_pChkTiePoints->setEnabled(false);
				}

				ui->horizontalLayout->addSpacing(20);
				ui->horizontalLayout->addWidget(lblSelectedImagesNum);
				ui->horizontalLayout->addSpacing(20);
				ui->horizontalLayout->addWidget(lblFirstSelectedImageName);
				ui->horizontalLayout->addSpacing(20);
				ui->horizontalLayout->addWidget(m_pChkRightClicked);
				ui->horizontalLayout->addSpacing(20);

				ui->horizontalLayout->addWidget(m_pChkPhotos);
				ui->horizontalLayout->addWidget(m_pChkTiePoints);
				ui->horizontalLayout->addWidget(m_pChkGCP);
				ui->horizontalLayout->addWidget(m_pImageLayer);
				if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
				{
					m_pSelectType = new QComboBox(this);
					m_pSelectType->addItem("Single Item");
					m_pSelectType->addItem("Rectangle");
					m_pSelectType->addItem("Polygon");


					m_pSelectType->setEnabled(true);


					ui->horizontalLayout->addWidget(m_pSelectType);

					if (BlockObject::isChineseVersion())
					{
						m_pSelectType->setItemData(0,"单选",Qt::DisplayRole);
						m_pSelectType->setItemData(1, "矩形框选", Qt::DisplayRole);
						m_pSelectType->setItemData(2, "多边形框选", Qt::DisplayRole);
					}
				}
				else
				{
					m_pSelectType = nullptr;
				}

#if 0
			model_viewer_widget_ = new ModelViewerWidget(this, options_);

			QObject::connect(model_viewer_widget_, &ModelViewerWidget::update_item_deleted, this, [=]() {emit update_item_deleted_(); });
			QObject::connect(model_viewer_widget_, &ModelViewerWidget::update_delete_image, this, [=](point3D_t id, QString name) {
					emit update_delete_image_(id, name);
				});
#endif

			mWindow = new MWindow(this,0,false,true);
			
			ui->ItemComboBox->clear();
			ui->ItemComboBox->addItem("Photos"); 
			ui->ItemComboBox->addItem("Tie points");
			
			
			
			QObject::connect(ui->ItemComboBox, &QComboBox::currentTextChanged, this, &ViewWidget::comboxChangeText);
			QObject::connect(m_pImageLayer, &QComboBox::currentTextChanged, this, &ViewWidget::Slot_ImageLayer);
			QObject::connect(m_pChkPhotos, &QCheckBox::stateChanged, this, &ViewWidget::Slot_ImageLayerCheckBoxStateChanged);
			QObject::connect(m_pChkTiePoints, &QCheckBox::stateChanged, this, &ViewWidget::Slot_ImageLayerCheckBoxStateChanged);
			QObject::connect(m_pChkGCP, &QCheckBox::stateChanged, this, &ViewWidget::Slot_ImageLayerCheckBoxStateChanged);
			if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
			{
				
				QObject::connect(m_pSelectType, &QComboBox::currentTextChanged, this, &ViewWidget::SelectionModeChanged);

			
			}
			QObject::connect(mWindow, &MWindow::signal_delete_photos, this, &ViewWidget::send_delete_photos);
			QObject::connect(mWindow, &MWindow::signal_delete_tiepoints, this, &ViewWidget::send_delete_tiepoints);

			QObject::connect(mWindow, &MWindow::signal_selected_images_from_3dview, this, &ViewWidget::signal_selected_images_from_3dview);
			
			QObject::connect(mWindow, &MWindow::signal_right_selected_images_from_3dview, this, &ViewWidget::Slot_right_selected_images_from_3dview);

#ifdef USE_OSGVIEWER
			QObject::connect(ui->action_mesh, SIGNAL(triggered()), this, SLOT(import_meshModel()));
			QObject::connect(ui->action_osgb, SIGNAL(triggered()), this, SLOT(import_osgbModel()));
			
#endif
			ui->action_osgb->setEnabled(true);

			m_pScroll = new QScrollArea(this);

			stackedWidget = new QStackedWidget(this);
			

#if 0
			stackedWidget->addWidget(model_viewer_widget_->point_viewer_widget_);
#endif

			m_pScroll->viewport()->setBackgroundRole(QPalette::Dark);
			m_pScroll->viewport()->setAutoFillBackground(true);
			m_pScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  
			m_pScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    
			m_pScroll->setWidgetResizable(true);
			m_pScroll->setWidget(stackedWidget);
			
			
#if 0
			ui->splitter->addWidget(model_viewer_widget_);
#else
			ui->splitter->addWidget(mWindow);
#endif
			ui->splitter->addWidget(m_pScroll);
			ui->splitter->setStretchFactor(0, 15);
			ui->splitter->setStretchFactor(1, 1);
			QWidget* widget = ui->splitter->widget(1);
			widget->hide();			

			bCheckBoxsInited = false;
		}
		
		void ViewWidget::RestorePreviousState()
		{
			std::set<AI3D::VIEWER::image_layer_e> imageLayerSelected;

			if(mWindow == nullptr)
				return;

			bool bPhotoImageLayerSelected = m_pChkPhotos->isChecked();
			bool bTiePointsImageLayerSelected = m_pChkTiePoints->isChecked();
			bool bGCPImageLayerSelected = m_pChkGCP->isChecked();

			if (bPhotoImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LARER_PHOTOS);

			if (bTiePointsImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LAYER_TIEPOINTS);

			if (bGCPImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LAYER_GCP);

			
			
			
			

			mWindow->ResetImageLayerSeleted(imageLayerSelected);

			if (model_select == POINT_MODEL)
			{
				mWindow->ResetSelectLayer(AI3D::VIEWER::selection_layer_e::LAYER_TIEPOINTS);
			}
			else if (model_select == CAMERAS_MODEL)
			{
				mWindow->ResetSelectLayer(AI3D::VIEWER::selection_layer_e::LARYER_PHOTOS);
			}

			if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
			{
				int nCurrentIndex = m_pSelectType->currentIndex();
				if (nCurrentIndex == 0)
				{
					
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_SINGLE_MODE);
				}
				else if (nCurrentIndex == 1)
				{
					
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_RECT_MODE);
				}
				else if (nCurrentIndex == 2)
				{
					
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_POLYGON_MODE);
				}
			}
		}

		void ViewWidget::send_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names)
		{
			emit signal_delete_photos(ids, names);
		}

		void ViewWidget::send_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name)
		{
			emit signal_delete_tiepoints(ids, name);
		}

		void ViewWidget::Slot_selected_images_from_3dview(std::vector<image_t>& images)
		{
			int iSelectedImagesNum = images.size();
			saved_images = images;
			if (iSelectedImagesNum > 0)
			{
				image_t iFirstSelectedImageId = images.at(0);
				if (iFirstSelectedImageId == kInvalidImageId)
					return;
				auto image = _blockdata->GetCurrentAT()->GetImage(iFirstSelectedImageId);
				lblFirstSelectedImageName->setText(QString::fromStdString(image.GetName()));
				
				
				if (m_pChkRightClicked->isChecked() && iSelectedImagesNum == 1)
				{
				
					OpenUserTiePoints(this,image);
					emit signal_insert_gcp_tab();
				
				}
			}

			lblSelectedImagesNum->setText(QString::number(iSelectedImagesNum));
		}

		void ViewWidget::Slot_right_selected_images_from_3dview(std::vector<image_t>& images)
		{
			int iSelectedImagesNum = images.size();

			if (iSelectedImagesNum > 0)
			{
				image_t iFirstSelectedImageId = images.at(0);
				auto image = _blockdata->GetCurrentAT()->GetImage(iFirstSelectedImageId);
				lblFirstSelectedImageName->setText(QString::fromStdString(image.GetName()));
				
				
				

				
				if (iSelectedImagesNum == 1)
				{
					
					OpenUserTiePoints(this, image);
					emit signal_insert_gcp_tab();
					
				}
			}

			lblSelectedImagesNum->setText(QString::number(iSelectedImagesNum));
		}

		void ViewWidget::addTextEditData(QString str) 
		{
			QString formatedMessage = QStringLiteral("[") + QTime::currentTime().toString() + QStringLiteral("] ") + str;
			QTextCursor cursor = ui->textEdit->textCursor();
			cursor.movePosition(QTextCursor::Start);
			ui->textEdit->setTextCursor(cursor);		
			ui->textEdit->insertPlainText(formatedMessage += '\n');			
		
		}

		
		
		void ViewWidget::addTextEidtListStr(QList<QString> str_list)
		{

			for (auto& tmp : str_list) 
			{
				QString formatedMessage = QStringLiteral("[") + QTime::currentTime().toString() + QStringLiteral("] ") + tmp;
				ui->textEdit->append(tmp);
			}


		}

		void ViewWidget::addTextEditData(QMatrix4x4 matrix) {
			ui->textEdit->append(QString::asprintf("%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n",
				matrix.constData()[0], matrix.constData()[4], matrix.constData()[8], matrix.constData()[12],
				matrix.constData()[1], matrix.constData()[5], matrix.constData()[9], matrix.constData()[13],
				matrix.constData()[2], matrix.constData()[6], matrix.constData()[10], matrix.constData()[14],
				matrix.constData()[3], matrix.constData()[7], matrix.constData()[11], matrix.constData()[15]));
		}

		void beginLoadxmlWithoutWgt(bool isInitLoad, QStringList path)
		{

		}

#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message(__FILE__ "[" STRING(__LINE__) "]:before importing osgb model.")

#ifdef USE_OSGVIEWER
		void ViewWidget::import_osgbModel()
		{
#pragma message（"inside importing osgb model."）
			QString srcDirPath = QFileDialog::getExistingDirectory(
				this, "choose src Directory",
				"/");

			if (srcDirPath.isEmpty())
			{
				return;
			}
			else
			{
				
				
				
				thread = new osgThread;
				thread->datadirpath = srcDirPath;
				thread->type = "*.osgb";
				connect(thread, SIGNAL(readOver(QString)), this, SLOT(onreadOver(QString)));
				thread->start();
			}
		}

		void ViewWidget::import_meshModel()
		{
			QString filePath;
			QFileDialog fileDialog(this);
			fileDialog.setWindowTitle("Open File");
			fileDialog.setNameFilter("osg (*.osg *.ply *.obj *.osgb)");
			fileDialog.setFileMode(QFileDialog::ExistingFiles);

			if (fileDialog.exec() == QDialog::Accepted)
				filePath = fileDialog.selectedFiles().first();

			if (filePath.isEmpty())
				return;


			
			
			
			thread = new osgThread;
			thread->datadirpath = filePath;
			thread->type = "*" + QFileInfo(filePath).suffix();
			std::cout << thread->type.toStdString() << std::endl;
			connect(thread, SIGNAL(readOver(QString)), this, SLOT(onreadOver(QString)));
			thread->start();
		}

		void ViewWidget::onreadOver(QString msg)
		{
			
			int num_cld = thread->lightRoot->getNumChildren();
			if (num_cld == 0) {
				thread->closeBar();
				QMessageBox::warning(this, "warning", msg);
				return;
			}

			if (!msg.isEmpty())
				QMessageBox::warning(this, "warning", msg);

			

			OsgModelView* viewWidget = new OsgModelView("", "*.osgb", thread->lightRoot);
			QDateTime current_date_time = QDateTime::currentDateTime();
			QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
			
			
			viewWidget->setStyleSheet("border:none;margin:0px;padding:0px;");
			viewWidget->setContentsMargins(0, 0, 0, 0);

			viewWidget->resize(800,600);
			viewWidget->move(QApplication::desktop()->screen()->rect().center() - viewWidget->rect().center());

			current_date_time = QDateTime::currentDateTime();
			current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
			
			viewWidget->show();

			
			
			std::vector<osgViewer::View*> views;
			viewWidget->getViews(views);
			for (auto view : views) {
				osg::ref_ptr<osgGA::TrackballManipulator> manipulator = new osgGA::TrackballManipulator();
				manipulator->setAllowThrow(false);
				view->setCameraManipulator(manipulator.get());
			}

			

			
			
		}
#endif USE_OSGVIEWER
		void ViewWidget::SetBlock(std::shared_ptr<AI3D::CORE::BlockObject> block)
		{
			_blockdata = block.get();
		}

		
		void ViewWidget::keyPressEvent(QKeyEvent* e)
		{
#if 0
			if (e->key() == Qt::Key_Delete)
				model_viewer_widget_->keyPressEvent(e);
#endif
			e->accept();
		}
		void ViewWidget::showAT3dview()
		{

			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			_blockdata->LoadTiepoints();
			
#if 0			
			model_viewer_widget_->update();
#endif	
			if (_blockdata->GetCurrentAT() == nullptr)
			{
				return ;
			}

			reconstruction = _blockdata->GetCurrentATMutual().get();
			if (!(reconstruction->HasPositionImages() ||
				reconstruction->HasControlPoints() ||
				reconstruction->HasTiepoints()))
			{
				QApplication::restoreOverrideCursor();
				return;
			}
			
			
		
			 at_data = new AI3D::CORE::ATData();
			*at_data = *reconstruction;
			Eigen::Vector3d offset;
			srs_s srs;
			if (at_data->HasPositionImages())
			{
				at_data->RenderPoses(offset, srs);
			}
			else
			{
				if (at_data->HasControlPoints())
				{
					at_data->GetPoints3DMutual().clear();
					if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type == GEOGRAPHIC)
					{
						std::string definition = BASESRS;
						at_data->TransformControlPoints(definition);						
					}
					if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type != LOCAL_ENU)
					{



						Eigen::Vector3d sum = Eigen::Vector3d::Zero();
						Eigen::Vector3d point_first = at_data->GetControlPoints().cbegin()->second.GetGivenXYZ();

						for (auto iter : at_data->GetControlPoints())
						{
							sum += (iter.second.GetObjectPoint().GetXYZ() - point_first);
						}
						Eigen::Vector3d position_offset = sum / at_data->GetControlPoints().size() + point_first;

						if (AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(at_data->GetLocalGcpSrs()).type != LOCAL)
						{
							
							AI3D::CORE::CoordinateTransformer::Transform(1, &position_offset[0],
								&position_offset[1], &position_offset[2],
								at_data->GetLocalGcpSrs(), "EPSG:4326");
							position_offset[2] = 0.0;

							char buf[1024];
							sprintf(buf, "%.5f,%.5f", position_offset[1], position_offset[0]);
							std::string strlb(buf);
							std::string local_srs_definition = "ENU:" + strlb;
							at_data->SetLocalGcpSrs(local_srs_definition);
							at_data->TransformControlPoints(local_srs_definition);
						}
						else
						{
							for (auto& iter : at_data->GetControlPointsMutual())
							{
								iter.second.GetObjectPointMutual().GetXYZMutual() -= position_offset;
							}
						}
					}
					
				}
				else
				{
					QApplication::restoreOverrideCursor();
					return;
				}
			}

			at_data->Normalize();
		
#if 0
			model_viewer_widget_->item_select_ = item_select_;
			if (model_viewer_widget_->reconstruction != nullptr)
			{
				delete model_viewer_widget_->reconstruction;
				model_viewer_widget_->reconstruction = nullptr;
			}
			
			model_viewer_widget_->reconstruction = at_data; 
#endif

			QString msg = "RegisteredPhotos/Photos: " + QString::fromStdString(std::to_string(reconstruction->GetNumRegImages())) + "/"
				+ QString::fromStdString(std::to_string(reconstruction->GetNumImages())) +
				"    Points: " + QString::fromStdString(std::to_string(reconstruction->GetNumPoints3D()));
			addTextEditData(msg);
			
#if 0
			model_viewer_widget_->CoordinateAxis = model_viewer_widget_->reconstruction->ComputePoints3dCoordinateAxis();
			
			if (model_viewer_widget_->CoordinateAxis.empty())
				return;
			model_viewer_widget_->GetOptions() = options_;
			model_viewer_widget_->ReloadReconstruction();
			model_viewer_widget_->updateShow();
#endif

			QApplication::restoreOverrideCursor();
		}

		void ViewWidget::GetSelectedImages(std::vector<image_t>& selectedImages)
		{
			std::vector<image_t> pickedImages;
			if (mWindow != nullptr)
			{
				if (mWindow->getPickedPhotoNodeId2(selectedImages))
				{
					std::cout << "get selected images from 3dview:" << selectedImages.size() <<  std::endl;
				}
				else
				{
					std::cout << "get selected images from 3dview failed." << std::endl;
				}
			}
		}

		void ViewWidget::RenderATDataWithSelectedImages(std::vector<image_t>& selectedImages)
		{
			if (_blockdata->GetCurrentAT() == nullptr)
			{
				return;
			}
			{
			bool bSaveFinished = false;
		
			auto savefunc = [&, this]()
			{
				LOGI("=====================preparing to render block waiting=================");
				
				
				
				_blockdata->LoadTiepoints();

				reconstruction = _blockdata->GetCurrentATMutual().get();
				if (!(reconstruction->HasPositionImages() ||
					reconstruction->HasControlPoints() ||
					reconstruction->HasTiepoints()))
				{
					bSaveFinished = true;
					return false;
				}

				if (at_data)
					delete at_data;

				at_data = new AI3D::CORE::ATData();
				*at_data = *reconstruction;

				
				if (mWindow)
				{
					mWindow->getOsgEngine()->bCanDelete = !_blockdata->HasReconstructions();

					mWindow->RenderBlockWithSelectedImages(*at_data, _blockdata->GetStatus(), selectedImages);
				}
				
				

				bSaveFinished = true;
				return true;
			};
			if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
			{
				LOGI("OpenLoadingPrompt:render block now, pls wait for a while");
				QString infostr = "render block now,pls wait for a while";
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					infostr = tr("渲染中，请稍等");
				}
				OpenLoadingPromptV4(infostr);

				QFuture<bool> f1 = QtConcurrent::run(savefunc);

				while (!bSaveFinished)
				{
					qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				auto ret = f1.result();
				if (ret)
				{
					SetLayerStatus();
					QString msg = "RegisteredPhotos/Photos: " + QString::fromStdString(std::to_string(reconstruction->GetNumRegImages())) + "/"
						+ QString::fromStdString(std::to_string(reconstruction->GetNumImages())) +
						"    Points: " + QString::fromStdString(std::to_string(reconstruction->GetNumPoints3D()));
					addTextEditData(msg);
				}
				
				CloseLoadingPromptV4();
				LOGI("Closed Loading Prompt here.");
			}
			else
			{
				savefunc();
			}
		}


			
		}
		void ViewWidget::SetLayerStatus()
		{
			if (bCheckBoxsInited)
				return;

			m_pChkGCP->setChecked(true);
			m_pChkGCP->setEnabled(true);
			if (_blockdata->GetCurrentAT() != nullptr)
			{
				if (!_blockdata->GetCurrentAT()->HasControlPoints())
				{


					m_pChkGCP->setChecked(false);
					m_pChkGCP->setEnabled(false);
				}

				m_pChkPhotos->setChecked(true);
				m_pChkPhotos->setEnabled(true);
				if (!_blockdata->GetCurrentAT()->HasImages())
				{

					m_pChkPhotos->setChecked(false);
					m_pChkPhotos->setEnabled(false);
				}

				m_pChkTiePoints->setChecked(true);
				m_pChkTiePoints->setEnabled(true);
				if (!_blockdata->GetTiepointFullStatus())
				{

					m_pChkTiePoints->setChecked(false);
					m_pChkTiePoints->setEnabled(false);
				}
				if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
				{
					
					
				}
			}

			bCheckBoxsInited = true;
		}
		
		
		void ViewWidget::RenderOsgFile(std::string fileName)
		{
			if (!mWindow)
				return;

			mWindow->loadOsgFile(fileName);
		}

		void ViewWidget::RenderModel(std::string model)
		{
			if (!mWindow)
				return;

			mWindow->RenderModel(model);
		}

		void ViewWidget::uploadGcpPoints()
		{
#if 0
			model_viewer_widget_->UploadGcpPoint();
#endif
		}

		void ViewWidget::comboxChangeText(const QString& str)
		{
			old_model_select = model_select;			
			if (str == "Tie points") 
			{
				model_select = POINT_MODEL;
				if (mWindow != nullptr)
					mWindow->ResetSelectLayer(AI3D::VIEWER::selection_layer_e::LAYER_TIEPOINTS);
				
			
			}
			else if (str == "Cameras" || str == "Photos")
			{
				model_select = CAMERAS_MODEL;
				if (mWindow != nullptr)
					mWindow->ResetSelectLayer(AI3D::VIEWER::selection_layer_e::LARYER_PHOTOS);

			}
			
#if 0
			model_viewer_widget_->setModel_select(model_select);
			if (old_model_select != model_select)
			{
				model_viewer_widget_->use_preview_view_ = true;
				*item_select_ = kInvalidImageId;
				model_viewer_widget_->item_select_ = item_select_;
				model_viewer_widget_->selected_point3D_id_ = kInvalidPoint3DId;
				model_viewer_widget_->selected_image_id_ = kInvalidImageId;
				model_viewer_widget_->ReloadReconstruction();
				model_viewer_widget_->updateShow();
				old_model_select = model_select;
			}
#endif
		}

		void ViewWidget::PhotoLayerChanged(const QString& str)
		{

		}

		void ViewWidget::TiepointsLayerChanged(const QString& str)
		{

		}

		void ViewWidget::SelectionModeChanged(const QString& str)
		{
			int nCurrentIndex = m_pSelectType->currentIndex();

			if (nCurrentIndex == 0)
			{
				std::cout << "single item." << std::endl;
				
				if (mWindow != nullptr)
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_SINGLE_MODE);
			}
			else if (nCurrentIndex == 1)
			{
				std::cout << "rectangle item." << std::endl;
				
				if (mWindow != nullptr)
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_RECT_MODE);
			}
			else if (nCurrentIndex == 2)
			{
				std::cout << "polygon item." << std::endl;
				
				if (mWindow != nullptr)
					mWindow->ResetSelectionMode(AI3D::VIEWER::SEL_POLYGON_MODE);
			}
		}
		
		void ViewWidget::Slot_ImageLayer(const QString& str)
		{
			std::cout << "inside image layer list,choose " << str.toStdString() << std::endl;

			std::set<AI3D::VIEWER::image_layer_e> imageLayerSeleted;

			if (str == "Photos")
			{
				
				imageLayerSeleted.insert(AI3D::VIEWER::IMAGE_LARER_PHOTOS);
			}
			else if (str == "TiePoints")
			{
				
				imageLayerSeleted.insert(AI3D::VIEWER::IMAGE_LAYER_TIEPOINTS);			
			}
			else if (str == "GCP")
			{
				
				imageLayerSeleted.insert(AI3D::VIEWER::IMAGE_LAYER_GCP);
			}

			if (mWindow != nullptr)
				mWindow->ResetImageLayerSeleted(imageLayerSeleted);
		}

		void ViewWidget::Slot_SelectType(const QString& str)
		{
			std::cout << "inside select type list,choose " << str.toStdString() << std::endl;
			SelectionModeChanged(str);
		}

		void ViewWidget::Slot_ImageLayerCheckBoxStateChanged(int state)
		{
			QCheckBox* pChkBox = dynamic_cast<QCheckBox*>(sender());
			std::set<AI3D::VIEWER::image_layer_e> imageLayerSelected;

			if (pChkBox == m_pChkPhotos)
			{
				std::cout << "photos checkbox clicked:" << state << std::endl;
			}
			else if (pChkBox == m_pChkTiePoints)
			{
				std::cout << "tiepoints checkbox clicked:" << state << std::endl;
			}
			else if (pChkBox == m_pChkGCP)
			{
				std::cout << "gcp checkbox clicked:" << state << std::endl;
			}

			bool bPhotoImageLayerSelected = m_pChkPhotos->isChecked();
			bool bTiePointsImageLayerSelected = m_pChkTiePoints->isChecked();
			bool bGCPImageLayerSelected = m_pChkGCP->isChecked();

			if (bPhotoImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LARER_PHOTOS);

			if (bTiePointsImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LAYER_TIEPOINTS);

			if (bGCPImageLayerSelected)
				imageLayerSelected.insert(AI3D::VIEWER::IMAGE_LAYER_GCP);

			std::cout << "imagelayer selected status set " << "photos:" << bPhotoImageLayerSelected
				<< " tiepoints:" << bTiePointsImageLayerSelected
				<< " gcp:" << bGCPImageLayerSelected
				<< std::endl;

			if (mWindow != nullptr)
				mWindow->ResetImageLayerSeleted(imageLayerSelected);
		}

		ViewWidget::~ViewWidget()
		{
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
		}

}
}