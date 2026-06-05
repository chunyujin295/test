#include "Gui/BlockWgt.h"
#include <algorithm>

#include <QVariant>
#include <QDateTime>
#include <QtConcurrent>
#include <QStringList>
#include <QThreadPool>
#include <QHostInfo>
#include "Gui/MohackerWin.h"
#include "Core/CoordinateSystem.h"
#include "Gui/BlockManager.h"
#include "Gui/ImportGcpDia.h"
#include "Gui/PosSigmaDia.h"
#include "Core/ControlPoint.h"
#include "Core/CoordinateSystem.h"
#include "Gui/ProjectManager.h"
#include "Core/Timer.h"
#include "Core/ReconPerfLog.h"
#include "Gui/ImportPosDia.h"
//#include "Gui/Network.h"
#include "Core/Types.h"
#include"Core/BlockObject.h"
#include"Gui/AddSigGcp.h"
#include "Util/Statistic.h"
#include "Util/Settings.h"
#include "Util/TaskProcess.h"
#include "Util/JobMonitor.h"
#include "Core/ReconstructionCommandSet.h"
//#include "AdapterWidget.h"

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include<QtGui>
#include<QFileDialog>
///#include "MainWindow.h"

#include "Windows.h"
#include <QtCore/QFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osg/Switch>
#include <QTextStream>
#include <stdio.h>
#include <qstring.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <osg/DrawPixels>
#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgUtil/Optimizer>

//#include <osgQt/qWidgetImage>
        //end read image
        //显示汉字
#include <osg/Depth>
#include <osgText/Text>
#include <osg/Camera>
#include <sstream>

///#include"qprocess.h"

#include "Core/Application.h"
#include "Core/File.h"
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/EventManager.h"
#include "Gui/message_box.h"

//#include "Gui/OTA.h"

//?chy InitGcpData
using namespace AI3D::CORE;
using namespace AI3D::VIEWER;
namespace AI3D
{
    namespace GUI
    {
        //int xmain();

        QtVEditorDoubleValidator::QtVEditorDoubleValidator(QObject* parent) : QDoubleValidator(parent)
        {
        }
        QtVEditorDoubleValidator::QtVEditorDoubleValidator(double bottom, double top, int decimals, QObject* parent)
            : QDoubleValidator(bottom, top, decimals, parent)
        {
        }
        QValidator::State QtVEditorDoubleValidator::validate(QString& str, int& i)const
        {
            if (str.isEmpty())
                return Acceptable;
            //return QValidator::Intermediate;
            if (bottom() >= 0 && str.startsWith('-'))
                return Invalid;
            int dotPos = str.indexOf(".");
            if (dotPos > 0 && str.right(str.length() - dotPos - 1).length() > decimals())//
                return Invalid;
            bool ok = false;
            double val = str.toDouble(&ok);
            if (!ok)
                return(bottom() < 0 && !str.compare("-")) ? Intermediate : Invalid;
            if (val <= top() && val >= bottom())
                return Acceptable;
            if (val >= 0)
                return(val > top() && -val < bottom()) ? Invalid : Intermediate;
            else
                return(val < bottom()) ? Invalid : Intermediate;
        }

        
        std::set<int> tiles4test_index_used;
        bool bProductionModelRandomTest = false;

        void BlockWgt::keyPressEvent(QKeyEvent* e)
        {
            if (e->key() == Qt::Key_Delete)
            {
                if (current_tab_id_ == 3)
                {
                    viewWidget_ui->keyPressEvent(e);
                }
            }
            e->accept();
        }

        void BlockWgt::InitMainButton()
        {
            ui->btn_addsig->setEnabled(true);
            ui->btn_adddir->setEnabled(true);
            ui->btn_push_removePgtable->setEnabled(false);
            ui->btn_addpos->setEnabled(false);
            ui->btn_delpos->setEnabled(false);
            ui->btn_Siggcp->setEnabled(false);
            ui->btn_addgcp->setEnabled(false);
          /*  ui->btn_addgcp_measurements->setEnabled(false);
            ui->btn_exportgcpmeasurements->setEnabled(false);*/
            ui->btn_at->setEnabled(false);
            ui->btn_paus->setEnabled(false);
            ui->btn_rec->setEnabled(false);
            
            // should the following three button be changed dynamically according to the detail status of current chosen job?       
            

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                ui->btn_addsig->setToolTip(tr("导入影像"));
                ui->btn_adddir->setToolTip(tr("导入影像目录"));
                ui->btn_push_removePgtable->setToolTip(tr("删除已选择影像"));

                ui->btn_addpos->setToolTip(tr("导入位姿"));
                ui->btn_delpos->setToolTip(tr("删除位姿"));

                ui->btn_Siggcp->setToolTip(tr("添加单个控制点"));
                ui->btn_addgcp->setToolTip(tr("添加控制点文件"));
                ui->btn_delgcp->setToolTip(tr("删除控制点"));

              /*  ui->btn_addgcp_measurements->setToolTip(tr("导入刺点文件"));
                ui->btn_exportgcpmeasurements->setToolTip(tr("导出刺点结果"));*/

                ui->btn_at->setToolTip(tr("提交空三"));
                ui->btn_paus->setToolTip(tr("取消空三"));
                ui->btn_rec->setToolTip(tr("再次提交空三"));


                ui->label_AddData->setText(tr("导入影像"));

            
                ui->label_AT->setText(tr("空三"));
                ui->label_Reconstruction->setText(tr("重建"));
                ui->label_Production->setText(tr("生产"));

                ui->label_Pho->setText(tr("影像"));
                ui->label_Pos->setText(tr("位姿"));
                ui->label_5->setText(tr("控制点"));
                ui->label_AT_2->setText(tr("空三"));
            }
            else
            {
                ui->btn_addsig->setToolTip(tr("Import photo"));
                ui->btn_adddir->setToolTip(tr("Import directory"));
                ui->btn_push_removePgtable->setToolTip(tr("Remove selected"));

                ui->btn_addpos->setToolTip(tr("Import POS"));
                ui->btn_delpos->setToolTip(tr("Remove POS"));

                ui->btn_Siggcp->setToolTip(tr("Add Sig GCP"));
                ui->btn_addgcp->setToolTip(tr("Add GCP File"));
                ui->btn_delgcp->setToolTip(tr("Remove GCP"));

               /* ui->btn_addgcp_measurements->setToolTip(tr("Import GCPMeasurements From File"));
                ui->btn_exportgcpmeasurements->setToolTip(tr("Export GCP Measurements to File"));*/

                ui->btn_at->setToolTip(tr("Submit AT"));
                ui->btn_paus->setToolTip(tr("Cancel AT"));
                ui->btn_rec->setToolTip(tr("Resubmit AT"));
            }

        }

        void BlockWgt::SetMainButtonStatus(Block_Status_s blockManagerStatus)
        {

            if (!blockManagerStatus.can_del_photo)
            {
                if (ui->btn_push_removePgtable->isEnabled())
                {
                    ui->btn_push_removePgtable->setEnabled(false);
                }
            }
            else
            {
                QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
                QModelIndex currentphotoindex = ui->tableView_photo_pos->currentIndex();

                bool photogroupselected = false;
                bool photoselected = false;

                if (currentgroupindex.isValid())
                    photogroupselected = true;;

                if (currentphotoindex.isValid())
                    photoselected = true;

                if (photogroupselected || photoselected)
                {
                    ui->btn_push_removePgtable->setEnabled(true);
                }
                else
                {
                    ui->btn_push_removePgtable->setEnabled(false);
                }
            }

            ui->btn_adddir->setEnabled(blockManagerStatus.can_add_photo);
            ui->btn_addsig->setEnabled(blockManagerStatus.can_add_photo);

            ui->btn_Siggcp->setEnabled(blockManagerStatus.can_add_gcp);
            ui->btn_addpos->setEnabled(blockManagerStatus.can_add_pos);
            if (blockManagerStatus.can_del_pos)
            {
                QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
                QModelIndex currentphotoindex = ui->tableView_photo_pos->currentIndex();

                bool photogroupselected = false;
                bool photoselected = false;
                if (currentgroupindex.isValid())
                    photogroupselected = true;;

                if (currentphotoindex.isValid())
                    photoselected = true;;
                if (photogroupselected || photoselected)
                {
                    ui->btn_delpos->setEnabled(true);
                }
                else
                {
                    ui->btn_delpos->setEnabled(false);
                }
            }
            else
            {
                ui->btn_delpos->setEnabled(false);
            }
            //ui->btn_delpos->setEnabled(blockManagerStatus.can_del_pos);
            //ui->btn_addgcp_measurements->setEnabled(blockManagerStatus.can_add_gcp);
            ui->btn_addgcp->setEnabled(blockManagerStatus.can_add_gcp);
            ui->btn_delgcp->setEnabled(blockManagerStatus.can_del_gcp);
            //设置AT的状态
            ui->btn_at->setEnabled(blockManagerStatus.can_AT);
            ui->btn_rec->setEnabled(blockManagerStatus.can_resubAT);
            ui->btn_paus->setEnabled(blockManagerStatus.can_cancle);

            
        }


        void BlockWgt::Slot_ClickTab(int idx)
        {       
                SetPhotoTabEditable(!block_data_->HasReconstructions());
                controlPoints_ui_->SetEditable(!block_data_->HasReconstructions());
                SetCurrentTabId();
            
            //  std::cout << "inside SlotClickTab:" << idx << std::endl;
            //  return;
            QString tabtext = ui->tabWidget->tabText(idx);
            if (tabtext.toStdString() == VIEWTAB)
            {
                viewWidget_ui->item_select_ = item_select_;

                if (bNeedCheckSelectedImagesLater)
                {
                    selectedImages.clear();

                    QItemSelectionModel* model_selection = ui->tableView_photo_pos->selectionModel();
                    QModelIndexList IndexList = model_selection->selectedIndexes();

                    if (IndexList.size() > 0)
                    {
                        for (auto imgidx : IndexList)
                        {
                            auto imageid = ui->tableView_photo_pos->getImageIdByRow(imgidx.row());
                            selectedImages.push_back(imageid);
                        }
                    }

                    bNeedCheckSelectedImagesLater = false;
                }
                else
                {
                    // if not clicked the photogroups list or photos list inside phototab,just use the original selected images which may come from 3dview tab.
                }

                //viewWidget_ui->showAT3dview();

                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 4)
                {
                    viewWidget_ui->RenderATDataWithSelectedImages(selectedImages);
                }
                //current_tab_id_ = 3;

                //std::cout << "to 3dview tab inside BlockWgt:" << (bLastMatrixExists ? " lastMatrix exists." : "lastMatrix not exist") << std::endl;

                if (bLastMatrixExists)
                {
                    //for (int r = 0; r < 4; r++)
                    //{
                    //  for (int c = 0; c < 4; c++)
                    //  {
                    //      std::cout << lastMatrix(r, c) << " ";
                    //  }
                    //  std::cout << std::endl;
                    //}

///                 UserMatrixData::setCurrentMatrixObject(this, lastMatrix);
                    viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                }
                else
                {
                    osg::Matrixd cmt = viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }

                viewWidget_ui->RestorePreviousState();

                ///osg::Matrixd cmt = viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                ///UserMatrixData::setCurrentMatrixObject(this, cmt);
            }
            else if (tabtext.toStdString() == GCPTAB)
            {
                //current_tab_id_ = 2;

                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

                    if (bLastMatrixExists)
                    {
                        osg::Matrixd cmt = viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();

                        //std::cout << "enter gcp tab of BlockWgt." << std::endl;
                        //for (int j = 0; j < 4; j++)
                        //{
                        //  for (int i = 0; i < 4; i++)
                        //  {
                                ///     std::cout << cmt(j, i) << " ";
                        //  }
                            /// std::cout << std::endl;
                        //}

                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }

                controlPoints_ui_->InitImageSet();
                controlPoints_ui_->InitGcpData();
                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                //  viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                    viewWidget_ui->mWindow->clearSceneData();
                }               

                QApplication::processEvents();
            }
            else if (tabtext.toStdString() == PHOTOTAB)
            {
                //current_tab_id_ = 1;

                // got selected images from 3dview.
                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
            
                    if (bLastMatrixExists)
                    {
                        osg::Matrixd cmt = viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();

                        //std::cout << "enter photo tab of tabwgt." << std::endl;

                        //for (int j = 0; j < 4; j++)
                        //{
                        //  for (int i = 0; i < 4; i++)
                        //  {
                    ///         std::cout << cmt(j, i) << " ";
                        //  }

                    ///     std::cout << std::endl;
                        //}

                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }

                if (!item_deleted_happened_in3dview_)
                {

                    std::vector<image_t> vecTmpImages;
                    viewWidget_ui->GetSelectedImages(vecTmpImages);
                    Slot_selected_images_from_3dview(vecTmpImages);

                }
                else
                {

                    int num = block_data_->GetCurrentAT()->GetNumImages();

                    item_deleted_happened_in3dview_ = false;

                }
                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                //  viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                    viewWidget_ui->mWindow->clearSceneData();
                }
                
            }
            else if (tabtext.toStdString() == ATTAB)
            {
                //current_tab_id_ = 0;
                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

                    if (bLastMatrixExists)
                    {
                        osg::Matrixd cmt = viewWidget_ui->mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();

                        //std::cout << "enter at tab of blockwgt." << std::endl;
                        //for (int j = 0; j < 4; j++)
                        //{
                        //  for (int i = 0; i < 4; i++)
                        //  {
                                ///std::cout << cmt(j, i) << " ";
                        //  }

                            ///std::cout << std::endl;
                        //}

                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }
                
                if (viewWidget_ui->mWindow != nullptr && viewWidget_ui->mWindow->viewerWindow != nullptr)
                {
                //  viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                    viewWidget_ui->mWindow->clearSceneData();
                }

                if (!GetRunningInfoTime->isActive())
                    GetRunningInfoTime->start(1000);

            }

            RefreshRightSideKxPxEditable();
        }

        void BlockWgt::SetIndexByStr(QString str)
        {
            int paper = -1;
            //获取tabbar页卡
            for (int i = 0; i < 4; i++)
            {
                if (QString::compare(ui->tabWidget->tabText(i), str) == 0)
                {
                    paper = i;
                    break;
                }
            }

            ui->tabWidget->setCurrentIndex(paper);
            if (str == "AT")
            {
                StartJobInfoTimer(true);
            }
        }

        BlockWgt::BlockWgt(AI3D::CORE::BlockObject* block, QWidget* parent)
            : QWidget(parent),
            block_data_(nullptr)
            /*itemmodel_photogroup_(nullptr),*/
//          ,ui(new Ui::CBlockWgt)
        {
            //if (BlockObject::isChineseVersion() && false)
            //{
            //  ui = (Ui::CBlockWgt *)(new Ui::CBlockWgtCN2());
            //}
            //else
            //{
                ui = new Ui::CBlockWgt;
            //}
            ui->setupUi(this);
            ///block_data_ = new AI3D::CORE::BlockObject;

            item_select_ = new image_t();
            *item_select_ = kInvalidImageId;
            bNeedCheckSelectedImagesLater = false;

            block_data_ = block;
            qRegisterMetaType<QVariant>("QVariant");

            InitPhotoTabIsEdit();
            InitButtonAndLabel();
            InitATWgt();
            InitTableViewPhotoGroup();
            InitTableWidgetPosList();

            ui->btn_newContruction->setFixedWidth(147);
            ui->btn_newContruction->setFixedHeight(42);
            if (BlockObject::isChineseVersion())
            { 
                ui->btn_newContruction->setText("提交重建");
            }
            ///viewWidget_ui = nullptr;
            viewWidget_ui = new ViewWidget(block);
            connect(viewWidget_ui, &ViewWidget::signal_insert_gcp_tab, this, &BlockWgt::InsertGCPTab);

            ui->tabWidget->insertTab(4, viewWidget_ui, "3D View");

            InitGcpControlPointWgt();

            QWidget* mAtWidget = ui->tabWidget->widget(TabPage::TPPhotos);
            QWidget* mPhWidget = ui->tabWidget->widget(TabPage::TPGeneral);
            QWidget* mGpsWidget = controlPoints_ui_;
            // just for test.
            myTabWidget_.insert(PHOTOTAB.c_str(), mPhWidget);
            myTabWidget_.insert(ATTAB.c_str(), mAtWidget);
            myTabWidget_.insert(GCPTAB.c_str(), mGpsWidget);
            myTabWidget_.insert(VIEWTAB.c_str(), viewWidget_ui);

            ui->tabWidget->removeTab(TabPage::TPPhotos);
            ui->tabWidget->removeTab(TabPage::TPGeneral);

            srs_s srs = block_data_->GetBlockSRS();

            ui->comboBox->addItem(("WGS84"));

            /*if (!boost::filesystem::exists(block_data_->GetTaskInfo().job_))*/
            if (block_data_->GetTaskInfo().job_ != "")
            {
                //ui->label_AT->hide();
                ui->label_AT->show();
            }
            else
            {           
                //ui->label_AT->show();
                ui->label_AT->hide();
            }

            // temporarily disabled,change the visible state later based on the actual situation.
            ui->label_Reconstruction->setVisible(false);
            ui->label_Production->setVisible(false);

            ui->graphics_view_photo->setScene(&m_scene);
            ui->graphics_view_photo->setAlignment(Qt::AlignCenter);
            ui->graphics_view_photo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &BlockWgt::Slot_ClickTab);

            CreateConnection();
            //connect(this, &BlockWgt::Signal_Photo_Progress, my_Progress, &ProgressCom::setValue);
            connect(viewWidget_ui, &ViewWidget::set_progress, this, &BlockWgt::Show_3DView_Progress);
            appconfig_s temp = Application::Getinstance().ParseConfig();
            
            GetRunningInfoTime = new QTimer;
            /*if (bloaded)*/

            if (0)
            {
                connect(GetRunningInfoTime, &QTimer::timeout, this, &BlockWgt::GetRealTimeInfo);
            }
            else
            {
                connect(GetRunningInfoTime, &QTimer::timeout, this, &BlockWgt::GetRealTimeInfoV2);
            }

            connect(viewWidget_ui, &ViewWidget::signal_delete_photos, this, &BlockWgt::Slot_delete_photos);
            connect(viewWidget_ui, &ViewWidget::signal_delete_tiepoints, this, &BlockWgt::Slot_delete_tiepoints);
            connect(viewWidget_ui, &ViewWidget::signal_selected_images_from_3dview, this, &BlockWgt::Slot_selected_images_from_3dview);
            connect(viewWidget_ui, &ViewWidget::signal_add_user_tie_point, this, &BlockWgt::Slot_add_user_tie_point);
//#ifdef USE_AI3D_PROJ
//          
//          ui->lblRotation->setVisible(true);
//          ui->le_rotation->setVisible(true);
//#else
//          ui->lblRotation->setVisible(false);
//          ui->le_rotation->setVisible(false);
//#endif // USE_AI3D_PROJ
            if (BlockObject::isChineseVersion())
            {
                // for Photo detail
                ui->label_2->setText("名称");
                ui->label_3->setText("目录");
                ui->label_20->setText("大小(M)");
                ui->label_23->setText("经度");
                ui->label_24->setText("纬度");
                ui->label_25->setText("高度");
//#ifdef USE_AI3D_PROJ
//              ui->lblRotation->setText("旋转矩阵(ECEF)");
//
//
//#endif // USE_AI3D_PROJ
                // for Photogroup
                ui->label_14->setText("名称");
                ui->label_16->setText("目录");
                ui->label_18->setText("影像数");
                ui->label_27->setText("影像尺寸");
                ui->label_26->setText("相机");
                ui->label_28->setText("传感器尺寸(mm)");
                ui->label_focalength->setText("焦距长度(mm)");
                ui->label_30->setText("畸变");

                ui->label_Progress->setText("进度:");
                ui->label_7->setText("区块ID:");
                ui->label_Status->setText("空三阶段:");
                ui->label->setText("完成时间:");
                ui->label_11->setText("提交时间:");

                //ui->taskList

            }
            else
            {
                // needed only for dynamically switching purpose.
                // for Photo detail
                ui->label_2->setText("Name");
                ui->label_3->setText("Directory");
                ui->label_20->setText("Dimension(M)");
                ui->label_23->setText("Logitude");
                ui->label_24->setText("Latitude");
                ui->label_25->setText("Height");
//#ifdef USE_AI3D_PROJ
//              ui->lblRotation->setText("Rotation(ECEF)");
//                      
//#endif // USE_AI3D_PROJ
                // for Photogroup
                ui->label_14->setText("Name");
                ui->label_16->setText("Directory");
                ui->label_18->setText("Number of photos");
                ui->label_27->setText("Image dimension");
                ui->label_26->setText("Camera");
                ui->label_28->setText("Sensor size(mm)");
                ui->label_focalength->setText("Focal length(mm)");
                ui->label_30->setText("Distorion");
            }

            RefreshRightSideKxPxEditable();
        }

        BlockWgt::~BlockWgt()
        {
            if (item_select_ != nullptr)
            {
                delete item_select_;
                item_select_ = nullptr;
            }
        }


        void BlockWgt::InitButtonAndLabel()
        {

            InitMainButton();

        }

        void BlockWgt::CreateConnection()
        {
            InitGcpTabConnections();
            InitATTabConnections();
            
            connect(this, &BlockWgt::Sig_IsModifiedXml, MohackerWin::GetInstance(), &MohackerWin::SetFileModifiedXml);
            connect(this, &BlockWgt::Sig_SaveFinished, MohackerWin::GetInstance(), &MohackerWin::SaveFinished);
            InitPhotoTabConnections();
        }

        void BlockWgt::InitNewWidget()
        {

            SetIndexByStr("3D View");

        }

        void BlockWgt::InitLoadWidget()
        {

            ProjectManager* promanager = ProjectManager::GetInstance();
            std::vector<int> tabvec;
            auto blockstatus = block_data_->GetStatus();
            bool ATFlag = true;
            //chy增加一个逻辑就是ATtab如果已经有了则不退出了



            if ((blockstatus == jobsta_e::STATUS_NEW/*&& !isATTabExisted*/) || blockstatus == jobsta_e::STATUS_UNKNOWN)
            {
                ATFlag = false;
            }


            promanager->GetBlockManaget(block_data_->GetId())->ChangeTab(ATFlag, block_data_->GetCurrentAT()->HasImages(), \
                block_data_->GetCurrentAT()->HasControlPoints(), tabvec);

            UpdateTabPaper(tabvec);

            if (blockstatus == jobsta_e::STATUS_COMPLETE)
            {
                InfoForShow_s show;
                show.status = jobsta_e::STATUS_COMPLETE;
                show.progressstylestr = RUNNINGSTYLE;//因其原始颜色偏深，running的偏亮
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    show.ATStagetext = str2qstr(blk_status_str_chinese.at(jobsta_e::STATUS_COMPLETE));
                }
                else
                {
                    show.ATStagetext = QString::fromStdString(blk_status_str.at(jobsta_e::STATUS_COMPLETE));
                }
                show.progreesvalue = 100.0;
                //加入是否是relative的判断
                auto definition = block_data_->GetCurrentAT()->GetLocalSrs();
                if (block_data_->GetCurrentAT()->HasRegImages())
                {

                    std::string relatstr, absstr;
                    if (BlockObject::isChineseVersion())
                    {
                        relatstr = "自由坐标系";
                        absstr = "绝对坐标系";
                    }
                    else
                    {
                        relatstr = "relative";
                        absstr = "absolute";
                    }
                    std::string modestr = (CoordinateDescriptor::GetSRSFromDefinition(definition).type == LOCAL) ? relatstr : absstr;

                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        modestr = "位姿状态: " + modestr;
                    }
                    else
                    {
                        modestr = "Photo positioning level: " + modestr;
                    }
                    show.ATStatustext = QString::fromStdString(modestr);


                }

                UpdateATTabLabel(show);
            }
            else
            {
                ui->label_Status->setVisible(false);
                ui->label_11->setVisible(false);
                ui->label_create_time->setVisible(false);
                ui->label->setVisible(false);
                ui->label_complete_time->setVisible(false);
                ui->label_7->setVisible(false);
                ui->label_blockID->setVisible(false);
            }

        }

        void BlockWgt::UpdateNewReconstructionStatus(Block_Status_s blockManagerStatus)
        {
            //ProjectManager* promanager = ProjectManager::GetInstance();
            //BlockManager* blockmanager = promanager->GetBlockManaget(block_data_->GetId());
            //auto blockstatus = blockmanager->GetBlockStatusMutual();

//          std::cout << " inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            if ( !blockManagerStatus.can_submit_rec)
            {
                ui->btn_newContruction->setStyleSheet("background-color:gray;color:white;width:147px;height:42px;border-radius:2px;border:0px solid;font:14px \"Arial\"");
                ui->btn_newContruction->setEnabled(false);

                return;
            }
            else
            {
                ui->btn_newContruction->setEnabled(true);
            }

            if(blockManagerStatus.can_submit_rec)
            {
                ui->btn_newContruction->setStyleSheet("background-color:#0072BE;color:white;width:147px;height:42px;border-radius:2px;border:0px solid;font:14px \"Arial\"");
            }       
        }

        void BlockWgt::SetWgtStatus(Block_Status_s blockManagerStatus)
        {
            SetMainButtonStatus(blockManagerStatus);
            UpdateNewReconstructionStatus(blockManagerStatus);
        }


        void BlockWgt::Show_3DView_Progress(int num, QString str)
        {

            MohackerWin::GetInstance()->Set3DViewProgressValue(num, str);
        }

        void BlockWgt::SetRightSideKxPxEditable(bool bEditable)
        {
            ui->le_k1_2->setReadOnly(!bEditable);
            ui->le_k2_2->setReadOnly(!bEditable);
            ui->le_k3_2->setReadOnly(!bEditable);
            ui->le_p1_2->setReadOnly(!bEditable);
            ui->le_p2_2->setReadOnly(!bEditable);
        }

        void BlockWgt::RefreshRightSideKxPxEditable()
        {
            if (!block_data_)
                return;

            ui->le_k1_2->setReadOnly(block_data_->HasReconstructions());
            ui->le_k2_2->setReadOnly(block_data_->HasReconstructions());
            ui->le_k3_2->setReadOnly(block_data_->HasReconstructions());
            ui->le_p1_2->setReadOnly(block_data_->HasReconstructions());
            ui->le_p2_2->setReadOnly(block_data_->HasReconstructions());
        }

        void BlockWgt::Slot_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names)
        {
            if (block_data_->HasReconstructions())
            {
                return;
            }
            std::cout << "inside " <<  " " << __FUNCTION__ << block_data_->GetId()<< " " << __LINE__ << std::endl;
            std::set<image_t> remids;
            for (auto t : ids)
            {
                remids.insert(t);
            }

            {
                bool hasSurveyElements = false;
                
                
                    
                    for (auto id : remids)
                    {
                        int aa = id;
                        if (aa < 0)
                        {
                            continue;
                        }
                        auto image = block_data_->GetCurrentAT()->GetImage(id);
                        if (image.HasGCPs())
                        {
                            hasSurveyElements = true;
                            break;
                        }
                    }
                    
                
                if (hasSurveyElements)
                {
                    CommonDelDia commondia;
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        commondia.SetInfor(tr("部分已选影像包含控制点，确定要删除吗?"));
                    }
                    else
                    {
                        commondia.SetInfor(tr("Some selected photos are referenced in control points.Do you really want to \n remove them?"));
                    }
                    if (commondia.exec() == QDialog::Rejected)
                    {
                        return;
                    }

                    isupdategcp = true;
                }
            }

            block_data_->RemoveImages(remids);

            QString msg = "RegisteredPhotos/Photos: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumRegImages())) + "/"
                + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumImages())) +
                "    Points: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumPoints3D()));
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                msg = "已注册影像数/总影像数: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumRegImages())) + "/"
                + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumImages())) +
                "    连接点数: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumPoints3D()));
            }

            viewWidget_ui->addTextEditData(msg);
            
            SetModifityXml();
        }

        void BlockWgt::Slot_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name)
        {
            if (block_data_->HasReconstructions())
            {
                return;
            }
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				std::vector<image_t> imageids;					 
            block_data_->GetCurrentATMutual()->DeleteTiePoints(ids, imageids);
            std::vector<std::string> names;
            Slot_delete_photos(imageids, names);
           for (auto it : imageids)
            {
                viewWidget_ui->mWindow->getOsgEngine()->Remove(ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS, it);
            }
		   /* for (auto t : ids)
            {
                block_data_->GetCurrentATMutual()->DeletePoint3D(t);
            }*/

            QString msg;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                msg = "已注册影像数/总影像数: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumRegImages())) + "/"
                + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumImages())) +
                "    连接点数: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumPoints3D()));
            }
            else
            {
                msg = "RegisteredPhotos/Photos: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumRegImages())) + "/"
                + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumImages())) +
                "    Points: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumPoints3D()));
            }
            viewWidget_ui->addTextEditData(msg);

            SetModifityXml();
        }

        void BlockWgt::Slot_selected_images_from_3dview(std::vector<image_t>& images)
        {
            // note:if images is empty,clear all selected status inside photos list and photogroups list.
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            // auto select images and related photogroups based on selected images from 3dview.
            // refresh the photogroups tabview and images tabview,highlight selected photogroup and photos.

            selectedImagesFrom3dviewOnly = images;
            selectedImages = images;

            mapSelectedImagesFrom3dview.clear();
            setSelectedImagesFrom3dview.clear();

            if (images.size() <= 0)
            {
                ui->tableView_photogroup->clearSelection();
                ui->tableView_photo_pos->clearSelection();
                
                min_selected_image_id = kInvalidImageId;
                min_selected_group_id = kInvalidGroupId;
                ui->tableView_photogroup->setCurrentIndex(QModelIndex());
                ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
                PopulatePhotoGroupTable();
                ui->tableView_photo_pos->clearData();
                UpdateWgtAndProjStatus(!ExistsTab(PHOTOTAB));
                return;
            }

            for (auto image_id : images)
            {           
                setSelectedImagesFrom3dview.insert(image_id);

                // find photogroup id
                auto group_id = block_data_->GetCurrentATMutual()->GetImage(image_id).GetPhotoGroupID();
                mapSelectedImagesFrom3dview[group_id].emplace_back(image_id);
            }

            min_selected_image_id = *setSelectedImagesFrom3dview.begin();
            min_selected_group_id = block_data_->GetCurrentATMutual()->GetImage(min_selected_image_id).GetPhotoGroupID();

            bool bFoundMinSelectedGroup = false;
            int iMinSelectedGroupRow = -1;

           for (int i = 0; i < ui->tableView_photogroup->RowCount(); i++)
            {
                int groupId = ui->tableView_photogroup->getGroupIdByRow(i);
                QModelIndex modelIndex = ui->tableView_photogroup->model()->index(i, 0);
                Slot_TableView_Clicked(modelIndex);
                
            }

            for (int i = 0; i < ui->tableView_photogroup->RowCount(); i++)
            {
                int groupId = ui->tableView_photogroup->getGroupIdByRow(i);
                QModelIndex modelIndex = ui->tableView_photogroup->model()->index(i, 0);
                
                if (groupId == min_selected_group_id)
                {           
                    
                    ui->tableView_photogroup->setCurrentIndex(modelIndex);
                                    
                    bFoundMinSelectedGroup = true;
                    iMinSelectedGroupRow = i;
                    break;
                }
            }

            if (!bFoundMinSelectedGroup)
            {
                ui->tableView_photogroup->clearSelection();
                ui->tableView_photo_pos->clearSelection();
            }
            else
            {
                ui->tableView_photo_pos->clearSelection();
                ui->tableView_photo_pos->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
                // display ui->tableView_photo_pos based on current selected photo group.

                std::vector<image_t>& selectedImages = mapSelectedImagesFrom3dview[min_selected_group_id];
                for (int i = 0; i < ui->tableView_photo_pos->RowCount(); i++)
                {
                    image_t image_id = ui->tableView_photo_pos->getImageIdByRow(i);
                    if (std::find(selectedImages.begin(), selectedImages.end(), image_id) != selectedImages.end())
                    {
                        ui->tableView_photo_pos->selectRow(i);
                    }
                }

                ui->tableView_photo_pos->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            }
        }


        void BlockWgt::Slot_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName)
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " "  << image.GetName() << " " << userTiePointName.toStdString() << std::endl;
            SetIndexByStr(QString::fromStdString(GCPTAB));
            emit signal_add_user_tie_point(image,userTiePointName);
        }

        void BlockWgt::Slot_currentChanged(int index)
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            // compared new tab(currnet) with last tab(previous),recording related tab titles and determining next action how to do.
        }

        bool BlockWgt::ExistsTab(std::string page)
        {
            int num = ui->tabWidget->count();
            for (int i = 0; i < num; i++)
            {
                QString name = ui->tabWidget->tabText(i);
                if (name.toStdString() == page)
                {
                    return true;
                }
            }
            return false;
        }







        void BlockWgt::ClearModelWithoutHeader(QAbstractItemModel* itemModel)
        {
            if (itemModel)
            {
                itemModel->removeRows(0, itemModel->rowCount());
            }
        }

        void BlockWgt::SetModifityXml()
        {
            emit Sig_IsModifiedXml();

            block_data_->setModifily(true);
            block_data_->GetTaskInfoMutual().isSaved = false;
        }

        void BlockWgt::UpdateTabPaper(std::vector<int> papervec)
        {
            // note: consider using chinese text for displaying instead in chinese environment.
            for (int i = ui->tabWidget->count() - 1; i >= 0; i--)
            {
                ui->tabWidget->removeTab(i);
            }

            for (auto iterator = papervec.begin() + 1; iterator != papervec.end(); iterator++)
            {
                QString tabstr;
                switch (*iterator)
                {
                case 0:
                    tabstr = "AT";
                    break;
                case 1:
                    tabstr = "Photos";
                    break;
                case 2:
                    tabstr = "GCP";
                    break;
                case 3:
                    tabstr = "3D View";
                    break;
                default:
                    break;
                }
                if (tabstr != "")
                {
                    ui->tabWidget->addTab(myTabWidget_[tabstr], tabstr);

                }


            }
            QString tabstr;
            if (*papervec.begin() == 0)
            {
                tabstr = "AT";
                
            }
            else if (*papervec.begin() == 1)
            {
                tabstr = "Photos";
                
            }
            else if (*papervec.begin() == 2)
            {
                tabstr = "GCP";
                
            }
            else if (*papervec.begin() == 3)
            {
                tabstr = "3D View";
            
            }
            SetIndexByStr(tabstr);
            SetCurrentTabId();
        }
        int BlockWgt::GetCurrentTabId()
        {
            return current_tab_id_;
        }

        void BlockWgt::SetCurrentTabId()
        {
            // note!!!: be careful, relevant logic may function abnormally if modify relevant tabText with chinese title.
            if (ui->tabWidget->tabText(0).toStdString() == ATTAB)
            {
                current_tab_id_ = 0;
            }
            else if (ui->tabWidget->tabText(0).toStdString() == PHOTOTAB)
            {
                current_tab_id_ = 1;
            }
            else if (ui->tabWidget->tabText(0).toStdString() == GCPTAB)
            {
                current_tab_id_ = 2;
            }
            else if (ui->tabWidget->tabText(0).toStdString() == VIEWTAB)
            {
                current_tab_id_ = 3;
            }
        }

        void BlockWgt::UpdateTaskListAll(QVector<JobStage>& jobStage)
        {

            //ui->taskList->setRowCount(params.taskNum);
            ui->taskList->setColumnCount(4);

            //ui->taskList->setRowCount(jobStage.size());

            ui->taskList->setUpdatesEnabled(false);

            int rowCount = ui->taskList->rowCount();
            for (int i = rowCount - 1; i >= 0; i--)
            {
                ui->taskList->removeRow(i);
            }

            ui->taskList->setRowCount(jobStage.size());


            /*bool bHasGotFailedOrCancelled = false;*/

            for (int i = 0; i < jobStage.size(); i++)
            {

                AddItemContent(i, 0, jobStage.at(i).functionName);
                AddItemContent(i, 1, QString::number(jobStage.at(i).completedNum) + "/" + QString::number(jobStage.at(i).stagedTotalNum));

                if (jobStage.at(i).status == int(STATUS_CANCLE) || jobStage.at(i).status == int(STATUS_FAILURE))
                {
                    /*  bHasGotFailedOrCancelled = true;*/

                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
///                     AddItemContent(i, 2, QString::fromStdString(blk_status_str_chinese.at(job_status_e(jobStage.at(i).status))));// func(jobStage.at(i).status
///                     AddItemContent(i, 2, BlockWgt::getChineseString("",blk_status_str_chinese.at(job_status_e(jobStage.at(i).status)).c_str()));// func(jobStage.at(i).status
                        AddItemContent(i, 2, str2qstr(blk_status_str_chinese.at(job_status_e(jobStage.at(i).status))));// func(jobStage.at(i).status
                    }
                    else
                    {
                        AddItemContent(i, 2, QString::fromStdString(blk_status_str.at(job_status_e(jobStage.at(i).status))));// func(jobStage.at(i).status
                    }
                }
                else if (jobStage.at(i).status == int(STATUS_UNKNOWN))
                {
                    AddItemContent(i, 2, "--");
                }
                else
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        ///AddItemContent(i, 2, QString::fromStdString(blk_status_str_chinese.at(job_status_e(jobStage.at(i).status))));//func(jobStage.at(i).status)
///                     AddItemContent(i, 2, BlockWgt::getChineseString("",blk_status_str_chinese.at(job_status_e(jobStage.at(i).status)).c_str()));//func(jobStage.at(i).status)
                        AddItemContent(i, 2, str2qstr(blk_status_str_chinese.at(job_status_e(jobStage.at(i).status))));//func(jobStage.at(i).status)
                    }
                    else
                    {
                        AddItemContent(i, 2, QString::fromStdString(blk_status_str.at(job_status_e(jobStage.at(i).status))));//func(jobStage.at(i).status)
                    }
                }

                AddItemContent(i, 3, jobStage.at(i).stageTotalTime);
            }

            ui->taskList->setUpdatesEnabled(true);
        }

        void BlockWgt::showEvent(QShowEvent* event)
        {
            std::ostringstream oss;
            oss << "BlockWgt/showEvent.";
            //LOGI(oss.str());
        }

        void BlockWgt::hideEvent(QHideEvent* event)
        {
            std::ostringstream oss;
            oss << "BlockWgt/hideEvent.";
            //LOGI(oss.str());
            ///viewWidget_ui->mWindow->clearSceneData();
        }

        void BlockWgt::closeEvent(QCloseEvent* event)
        {
            std::ostringstream oss;
            oss << "BlockWgt/closeEvent.";
            LOGI(oss.str());
        }

        void InitOSGEngine()
        {
            OsgEngine* pOsgEngine = OsgEngine::getInstance();
            if(pOsgEngine)
                pOsgEngine->initViewer();


        }


        osgQOpenGLWidget::osgQOpenGLWidget(QWidget* parent)
            : QOpenGLWidget(parent)
        {
        }

        osgQOpenGLWidget::osgQOpenGLWidget(osg::ArgumentParser* arguments,
            QWidget* parent) :
            QOpenGLWidget(parent),
            _arguments(arguments)
        {

        }

        osgQOpenGLWidget::~osgQOpenGLWidget()
        {
        }


        ViewerQT::ViewerQT(QWidget* parent, const char* name, const QOpenGLWidget* shareWidget, WindowFlags f, bool bUseLaterSize ,int forceWidth, int forceHeight ) :
            AdapterWidget(parent, name, shareWidget, f)
        {
            if (bUseLaterSize && forceWidth > 0 && forceHeight > 0)
            {
                getCamera()->setViewport(new osg::Viewport(0, 0, forceWidth, forceHeight));
                getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(forceWidth) / static_cast<double>(forceHeight), 1.0f, 10000.0f);
                getCamera()->setGraphicsContext(getGraphicsWindow());
            }
            else
            {
                getCamera()->setViewport(new osg::Viewport(0, 0, width(), height()));
                getCamera()->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width()) / static_cast<double>(height()), 1.0f, 10000.0f);
                getCamera()->setGraphicsContext(getGraphicsWindow());
            }

            setThreadingModel(osgViewer::Viewer::AutomaticSelection);
            //setThreadingModel(osgViewer::Viewer::SingleThreaded);

            QSurfaceFormat format;
            format.setDepthBufferSize(24);
            format.setStencilBufferSize(4);
            format.setProfile(QSurfaceFormat::CoreProfile);
            setFormat(format);

            connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));  //updateGL
            _timer.start(10);
            
        }

        AdapterWidget::AdapterWidget(QWidget* parent, const char* name, const QOpenGLWidget* shareWidget, WindowFlags f) :
            QOpenGLWidget(parent,/* shareWidget, */f)
        {
            _gw = new osgViewer::GraphicsWindowEmbedded(0, 0, width(), height());            
            setFocusPolicy(Qt::StrongFocus);
            setMouseTracking(true);
        }

        void AdapterWidget::initializeGL()
        {
            makeCurrent();            
        }

        void AdapterWidget::setKeyboardModifiers(QInputEvent* event)
        {
            int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);
            unsigned int mask = 0;
            if (modkey & Qt::ShiftModifier) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
            if (modkey & Qt::ControlModifier) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
            if (modkey & Qt::AltModifier) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
            _gw->getEventQueue()->getCurrentEventState()->setModKeyMask(mask);
        }
        void AdapterWidget::resizeGL(int width, int height)
        {
            _gw->getEventQueue()->windowResize(0, 0, width, height);
            _gw->resized(0, 0, width, height);
        }
        void AdapterWidget::keyPressEvent(QKeyEvent* event)
        {
            setKeyboardModifiers(event);
            _gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol) * (event->text().toLatin1().data()));
        }
        void AdapterWidget::keyReleaseEvent(QKeyEvent* event)
        {
            setKeyboardModifiers(event);
            _gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol) * (event->text().toLatin1().data()));
        }

        void AdapterWidget::mousePressEvent(QMouseEvent* event)
        {
            int button = 0;
            switch (event->button())
            {
            case(Qt::LeftButton): button = 1; break;
            case(Qt::MidButton): button = 2; break;
            case(Qt::RightButton): button = 3; break;
            case(Qt::NoButton): button = 0; break;
            default: button = 0; break;
            }
            setKeyboardModifiers(event);
            _gw->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
        }
        void AdapterWidget::mouseReleaseEvent(QMouseEvent* event)
        {
            int button = 0;
            switch (event->button())
            {
            case(Qt::LeftButton): button = 1; break;
            case(Qt::MidButton): button = 2; break;
            case(Qt::RightButton): button = 3; break;
            case(Qt::NoButton): button = 0; break;
            default: button = 0; break;
            }
            setKeyboardModifiers(event);
            _gw->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
        }
        void AdapterWidget::mouseDoubleClickEvent(QMouseEvent* event)
        {
            int button = 0;
            switch (event->button())
            {
            case Qt::LeftButton: button = 1; break;
            case Qt::MidButton: button = 2; break;
            case Qt::RightButton: button = 3; break;
            case Qt::NoButton: button = 0; break;
            default: button = 0; break;
            }
            setKeyboardModifiers(event);
            _gw->getEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);
        }
        void AdapterWidget::mouseMoveEvent(QMouseEvent* event)
        {
            setKeyboardModifiers(event);
            _gw->getEventQueue()->mouseMotion(event->x(), event->y());
        }
        void AdapterWidget::wheelEvent(QWheelEvent* event)
        {
            setKeyboardModifiers(event);
            _gw->getEventQueue()->mouseScroll(
                event->delta() > 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN);
        }

        ConstructionWgt::ConstructionWgt(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject* recons_object, QStandardItem* recons_item, QWidget* parent)
            : QWidget(parent)
        {
            ///ui = new Ui::CBlockWgt();
            ui = new Ui::CReConstructionWgt();
            ui->setupUi(this);
            block_data_ = block;
//          std::cout << "constructor:get reconstruction info:" << __LINE__ << " " << recons_object->GetName() << " " << recons_object->GetNumTiles() << std::endl;
            recons_object_ = recons_object;
            this->recons_item = recons_item;
            qRegisterMetaType<QVariant>("QVariant");

            bShowDetails = false;
            bSupportMoreTileMode = true;
            bHasRenderedATData = false;
            bROIEditing = false;
            bInsideOverview = false;

            ui->btn_newContruction->setStyleSheet("background-color:#0072BE;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            if (AI3D::CORE::BlockObject::isChineseVersion()) 
            {
                ui->btn_newContruction->setText("提交生产");
            }
            else {
                ui->btn_newContruction->setText("Submit Production");
            }
            connect(ui->btn_newContruction, &QPushButton::clicked, this, &ConstructionWgt::Slot_SubmitProduction);

            ui->btn_newContruction->hide();

            ui->label_AT->show();
            ui->label_Reconstruction->show();
            ui->label_Production->hide();

            ui->btn_addsig->setEnabled(false);
            ui->btn_adddir->setEnabled(false);
            ui->btn_push_removePgtable->setEnabled(false);
            ui->btn_addpos->setEnabled(false);
            ui->btn_delpos->setEnabled(false);
            ui->btn_Siggcp->setEnabled(false);
            ui->btn_addgcp->setEnabled(false);
         

            ui->btn_delgcp->setEnabled(false);
            ui->btn_at->setEnabled(false);
            ui->btn_paus->setEnabled(false);
            ui->btn_rec->setEnabled(false);

            // should the following three buttons be changed dynamically according to the detail status of current chosen job?
            ui->btn_submit_rec->setEnabled(false);
            ui->btn_resubmit_recon->setEnabled(false);
            ui->btn_cancle_recon->setEnabled(false);

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                ui->btn_addsig->setToolTip(tr("导入影像"));
                ui->btn_adddir->setToolTip(tr("导入目录"));
                ui->btn_push_removePgtable->setToolTip(tr("删除已选择影像"));

                ui->btn_addpos->setToolTip(tr("导入位姿"));
                ui->btn_delpos->setToolTip(tr("删除位姿"));

                ui->btn_Siggcp->setToolTip(tr("添加单独控制点"));
                ui->btn_addgcp->setToolTip(tr("添加控制点文件"));
                ui->btn_delgcp->setToolTip(tr("删除控制点"));
              /*  ui->btn_addgcp_measurements->setToolTip(tr("导入刺点文件"));
                ui->btn_exportgcpmeasurements->setToolTip(tr("导出刺点结果"));*/

                ui->btn_submit_rec->setToolTip(tr("提交重建"));
                ui->btn_resubmit_recon->setToolTip(tr("再次提交重建"));
                ui->btn_cancle_recon->setToolTip(tr("取消重建"));


                ui->label_AddData->setText(tr("导入影像"));

                ui->label_AT->setText(tr("空三"));
                ui->label_Reconstruction->setText(tr("重建"));
                ui->label_Production->setText(tr("生产"));

                ui->label_Pho->setText(tr("影像"));
                ui->label_Pos->setText(tr("位姿"));
                ui->label_5->setText(tr("控制点"));
                ui->label_AT_2->setText(tr("空三"));
            }
            else {
                ui->btn_addsig->setToolTip(tr("Import photo"));
                ui->btn_adddir->setToolTip(tr("Import directory"));
                ui->btn_push_removePgtable->setToolTip(tr("Remove selected"));

                ui->btn_addpos->setToolTip(tr("Import POS"));
                ui->btn_delpos->setToolTip(tr("Remove POS"));

                ui->btn_Siggcp->setToolTip(tr("Add Sig GCP"));
                ui->btn_addgcp->setToolTip(tr("Add GCP File"));
                ui->btn_delgcp->setToolTip(tr("Remove GCP"));

          

                ui->btn_submit_rec->setToolTip(tr("Submit Reconstruction"));
                ui->btn_resubmit_recon->setToolTip(tr("Resubmit Reconstruction"));
                ui->btn_cancle_recon->setToolTip(tr("Cancel Reconstruction"));

            }


            ui->btn_submit_rec->hide();
            ui->btn_resubmit_recon->hide();
            ui->btn_cancle_recon->hide();
            ui->frame_5->hide();

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                ui->btn_at->setToolTip(tr("提交空三"));
                ui->btn_paus->setToolTip(tr("取消空三"));
                ui->btn_rec->setToolTip(tr("再次提交空三"));
            }
            else {
                ui->btn_at->setToolTip(tr("Submit AT"));
                ui->btn_paus->setToolTip(tr("Cancel AT"));
                ui->btn_rec->setToolTip(tr("Resubmit AT"));
            }

            for (int i = ui->tabWidget->count() - 1; i >= 0; i--)
            {
                //ui->tabWidget->removeTab(i);
            }

            ///         ui->widget->setVisible(false);

            ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tab), QApplication::translate("CBlockWgt", "Overview", nullptr));
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tab_4), QApplication::translate("CBlockWgt", "Spatial Framework", nullptr));

            //ui->widget->setStyleSheet("background-color:grey;width:1000px;height:600px;");
            ui->widget->setStyleSheet("background-color:rgb(40,40,40);");

            QVBoxLayout* vlOverviewContainer = new QVBoxLayout();
            vlOverviewContainer->setContentsMargins(5, 5, 5, 5);

            QWidget* panelOverview = new QWidget(ui->widget);
            panelOverview->setObjectName("panelOverview");
            panelOverview->setStyleSheet("#panelOverview {background-color:rgb(40,40,40);border:1px solid rgb(72,72,72);padding:5px;}");
            panelOverview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            vlOverviewContainer->addWidget(panelOverview, 1);

            QVBoxLayout* vlOverview = new QVBoxLayout();

            vlOverview->setContentsMargins(27, 27, 27, 27);

            vlOverview->setSpacing(12);

            QWidget* panelTop = new QWidget(ui->widget);
            //panelTop->setStyleSheet("border-radius:14px;background-color:rgb(46,59,74);padding-left:35px;padding-right:35px;padding-top:32px;margin:0px;");
            panelTop->setStyleSheet("border-radius:14px;background-color:rgb(46,59,74);");

            QHBoxLayout* hlTop = new QHBoxLayout();
            ///hlTop->setContentsMargins(35, 32, 35, 32);
            hlTop->setContentsMargins(0, 0, 0, 0);

            hlTop->addSpacing(45);
            hlTop->setSpacing(15);

            QLabel* lblTopLeft = new QLabel(panelTop);
            lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_succ.png"));

            // part of the top panel of the overview tabpage for the reconstruction.
            QLabel* lblTopRight = new QLabel(panelTop);
            bool valid = IsBoundingBoxValid(recons_object->GetBoundingBoxCustom());
            if (!valid)
            {
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    lblTopRight->setText("无效的建模范围");
                }
                else {
                    lblTopRight->setText(" Invalid SRS framework");
                }

                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_fail.png"));                 
            }
            else
            {
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    lblTopRight->setText("准备进行生产");
                }
                else {
                    lblTopRight->setText("Ready for production");
                }
            }
            //lblTopRight->setStyleSheet("color:rgb(146,231,197);font:bold 18px \"Arial\"");
            lblTopRight->setStyleSheet("color:rgb(146,231,197);font:bold 18px \"Arial\"");

            hlTop->addWidget(lblTopLeft);
            //          hlTop->addLayout(vlTopRight);
            hlTop->addWidget(lblTopRight);
            hlTop->addStretch(1);

            panelTop->setLayout(hlTop);

            QHBoxLayout* hlProductions = new QHBoxLayout();
            QLabel* lblProductions = new QLabel(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblProductions->setText("生产列表");
            }
            else {
                lblProductions->setText("Productions");
            }
            lblProductions->setStyleSheet("color:rgb(230,230,230);font:16px \"Arial\";");
            hlProductions->addWidget(lblProductions, 0, Qt::AlignLeft);

            twProductionList = new QTableWidget(ui->widget);
            twProductionList->setColumnCount(5);

            QStringList slProductionList;

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                slProductionList << "生产名称" << "格式" << "状态" << "进度" << "最后提交时间";
            }
            else {
                slProductionList << "Production" << "Format" << "Status" << "Progress" << "Last submitted";
            }

            twProductionList->setHorizontalHeaderLabels(slProductionList);
            twProductionList->verticalHeader()->hide();
            twProductionList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            twProductionList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            twProductionList->horizontalHeader()->setFixedHeight(40);
            //twProductionList->horizontalHeader()->setStretchLastSection(true);
            //twProductionList->setStyleSheet("background-color:yellow;color:orange;");
            twProductionList->setSelectionBehavior(QAbstractItemView::SelectRows);
            twProductionList->verticalHeader()->setDefaultSectionSize(50);
            twProductionList->setFocusPolicy(Qt::NoFocus);


            twProductionList->setStyleSheet("QTableWidget { background-color:rgb(40,40,40);color:rgb(230,230,230);font: 14px \"Arial\";border:none;border-right:1px solid rgb(60,60,60);}"
                "QHeaderView::section{ background-color:rgb(68,68,68);color: rgb(230,230,230); border:none;padding-left:40px;padding-top:0px;}"
                "QTableWidget::item { background-color:rgb(40,40,40);color:rgb(236,236,236);border:none;border-left:1px solid rgb(60,60,60);border-bottom:1px solid rgb(60,60,60);padding-top:0px;padding-bottom:0px;padding-left:40px;}"
                "QTableWidget::item:selected { background-color:rgb(36,48,55);color:rgb(236,236,236);}");

            QHBoxLayout* hlNewReconstruction = new QHBoxLayout();

            butNewProduction = new QPushButton(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                butNewProduction->setText("提交生产");
            }
            else {
                butNewProduction->setText("Submit Production");
            }
            butNewProduction->setStyleSheet("background-color:#0072BE;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            //          connect(butNewReconstruction, &QPushButton::clicked, this, &ConstructionWgt::Slot_SubmitProduction);
            ///         hlNewReconstruction->addStretch(1);
            hlNewReconstruction->addWidget(butNewProduction, 0, Qt::AlignRight);
            
            butNewProduction->setEnabled(valid ? true : false);
            //          vlOverview->addStretch(1);
            vlOverview->addWidget(panelTop, 1);
            vlOverview->addLayout(hlProductions);
            vlOverview->addWidget(twProductionList, 3);
            vlOverview->addLayout(hlNewReconstruction);
            //vlOverview->addStretch(1);

            panelOverview->setLayout(vlOverview);
            ///         ui->widget->setLayout(vlOverview);
            ui->widget->setLayout(vlOverviewContainer);

            QHBoxLayout* hlCenterArea = new QHBoxLayout();

            hlCenterArea->setContentsMargins(14, 14, 14, 9);

            QVBoxLayout* vlCenterMiddle = new QVBoxLayout();
            QHBoxLayout* hlTitle = new QHBoxLayout();

#if 0
            QVBoxLayout* vlCenterLeft = new QVBoxLayout();

            QPushButton* butCenterLeft1 = new QPushButton(ui->tab_4);
            QPushButton* butCenterLeft2 = new QPushButton(ui->tab_4);
            QPushButton* butCenterLeft3 = new QPushButton(ui->tab_4);
            QPushButton* butCenterLeft4 = new QPushButton(ui->tab_4);

            butCenterLeft1->setText("Left1");
            butCenterLeft2->setText("Left2");
            butCenterLeft3->setText("Left3");
            butCenterLeft4->setText("Left4");

            vlCenterLeft->setSpacing(20);
            vlCenterLeft->addWidget(butCenterLeft1);
            vlCenterLeft->addWidget(butCenterLeft2);
            vlCenterLeft->addWidget(butCenterLeft3);
            vlCenterLeft->addWidget(butCenterLeft4);
            vlCenterLeft->addStretch(1);

            QPushButton* butCenterMiddle1 = new QPushButton(this);
            QPushButton* butCenterMiddle2 = new QPushButton(this);
            QPushButton* butCenterMiddle3 = new QPushButton(this);
            butCenterMiddle1->setText("Middle1");
            butCenterMiddle2->setText("Middle2");
            butCenterMiddle3->setText("Middle3");

            vlCenterMiddle->setSpacing(20);
            vlCenterMiddle->addWidget(butCenterMiddle1);
            vlCenterMiddle->addWidget(butCenterMiddle2);
            vlCenterMiddle->addWidget(butCenterMiddle3);
            vlCenterMiddle->addStretch(1);
#endif

#if 0
            viewerWindow = new ViewerQT();
            osg::Camera* camera = viewerWindow->getCamera();//获得渲染器中的相机
#if 000
            camera->setClearColor(osg::Vec4(128.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0, 0.8));//设置清除缓存区背景的颜色.RGBA格式.

            QPalette p;
            p.setColor(QPalette::Background, QColor(0, 0, 0));
            ///viewerWindow->setCameraManipulator(new osgGA::DriveManipulator);//  osgGA::TrackballManipulator
            viewerWindow->setPalette(p);
#endif

            //窗口大小变化事件
            viewerWindow->addEventHandler(new osgGA::StateSetManipulator(viewerWindow->getCamera()->getOrCreateStateSet()));
            viewerWindow->addEventHandler(new osgViewer::WindowSizeHandler);
            viewerWindow->addEventHandler(new osgViewer::StatsHandler);

            ViewerQT* lblCenterLeft = viewerWindow;

            //添加操作器
            osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

            keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());

            keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
            keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
            keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());
            viewerWindow->setCameraManipulator(keyswitchManipulator.get());
#endif

            ///mWindow = new MWindow(ui->tab_4, 0, true, false);
            mWindow = new MWindow(ui->tab_4, 0, true, false, true); // bInsideConstruction is true.
            connect(mWindow, &MWindow::signal_projchanged, this, &ConstructionWgt::Slot_UpdateROIBy3DViewEdit);
            connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ConstructionWgt::Slot_ClickTab);
            QWidget* panelCenterRight = new QWidget(ui->tab_4);
            panelCenterRight->setObjectName("panelCenterRight");
            panelCenterRight->setStyleSheet("#panelCenterRight { background-color:rgb(40,40,40);margin:0px;padding:0px;border:1px solid #484848; } ");
            /*
            @attention此处只表明注册这两件事即可，是否在哪调用以及是否用指针需根据实际情况
            */

            QVBoxLayout* vlCenterRight = new QVBoxLayout();

            vlCenterRight->setContentsMargins(0, 0, 0, 0);
            vlCenterRight->setSpacing(7);
            ///vlCenterRight->setSpacing(20);
            //vlCenterRight->setSpacing(25);

            QLabel* lblCenterRightTitle = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblCenterRightTitle->setText("属性");
            }
            else
            {
                lblCenterRightTitle->setText("Attribute");
            }

            lblCenterRightTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            lblCenterRightTitle->setStyleSheet("background-color:rgb(68,68,68);color:white;padding-left:22px;font:bold 16px \"Arial\";");
            lblCenterRightTitle->setFixedHeight(38);

            QHBoxLayout* hlROIAction = new QHBoxLayout();
            hlROIAction->setContentsMargins(28, 0, 28, 0);

            QLabel* lblROI = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblROI->setText("兴趣区");
            }
            else
            {
                lblROI->setText("Region of Interest");
            }
            lblROI->setStyleSheet("background-color:transparent; color:#FFFFFF; font:bold 16px \"Arial\"");// ("background - color:transparent; color:white; font:bold 16px \"Arial\"");

            butROIEdit = new QPushButton(ui->tab_4);
            butROIImport = new QPushButton(ui->tab_4);
            butROIDefault = new QPushButton(ui->tab_4);

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                butROIEdit->setText("编辑");
                butROIImport->setText("导入");
                butROIDefault->setText("默认");
            }
            else
            {
                butROIEdit->setText("Edit");
                butROIImport->setText("Import");
                butROIDefault->setText("Default");
            }

            butROIEdit->setFixedWidth(54);
            butROIEdit->setFixedHeight(24);
            butROIImport->setFixedWidth(54);
            butROIImport->setFixedHeight(24);
            butROIDefault->setFixedWidth(54);
            butROIDefault->setFixedHeight(24);

            butROIEdit->setStyleSheet("QPushButton {background-color:#165DFF;color:white;border-radius:2px;font:14px \"Arial\";}"
                "QPushButton:disabled{background-color:#99646D83; color:#99B6B6B6;}"
            );
            butROIImport->setStyleSheet("QPushButton { background-color:#00165DFF;color:#165DFF;border-radius:2px;border:1px solid #165DFF;font:14px \"Arial\";}"
                "QPushButton:disabled{background-color:#00165DFF; color:#646D83;border:1px solid #99646D83;}"
            );
            butROIDefault->setStyleSheet("QPushButton { background-color:#00165DFF;color:#165DFF;border-radius:2px;border:1px solid #165DFF;font:14px \"Arial\";}"
                "QPushButton:disabled{background-color:#00165DFF; color:#646D83;border:1px solid #99646D83;}"
            );

            hlROIAction->setSpacing(8);
            hlROIAction->addWidget(lblROI);
            hlROIAction->addStretch(1);
            hlROIAction->addWidget(butROIEdit);
            hlROIAction->addWidget(butROIImport);
            hlROIAction->addWidget(butROIDefault);

            QHBoxLayout* hlScopeMethod = new QHBoxLayout();
            hlScopeMethod->setContentsMargins(41, 0, 106, 0);

            QLabel* lblScopeMethod = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblScopeMethod->setText("兴趣区模式:");
            }
            else
            {
                lblScopeMethod->setText("Scope method:");
            }

            butTiePoints = new QPushButton(ui->tab_4);
            butPhotos = new QPushButton(ui->tab_4);
            butFrustum = new QPushButton(ui->tab_4);

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                butTiePoints->setText("连接点");
                butPhotos->setText("影像");
                butFrustum->setText("影像视锥体");
            }
            else
            {
                butTiePoints->setText("Tie Points");
                butPhotos->setText("Photos");
                butFrustum->setText("Frustum");
            }

            butFrustum->setEnabled(true);

            lblScopeMethod->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
//          butTiePoints->setStyleSheet("width:100px;height:32px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px;");
//          butPhotos->setStyleSheet("width:100px;height:32px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px;");
//          butFrustum->setStyleSheet("QPushButton { width:100px;height:32px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px; }"
//              "QPushButton:disabled {background-color:#99646D83;color:#99B6B6B6;}"
//          );

            butTiePoints->setStyleSheet("width:100px;height:26px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px;");
            butPhotos->setStyleSheet("width:100px;height:26px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px;");
            butFrustum->setStyleSheet("QPushButton { width:100px;height:26px;background-color:#34363A;color:#CCFFFFFF;font:14px \"Arial\";border-radius:4px; }"
                "QPushButton:disabled {background-color:#99646D83;color:#99B6B6B6;}"
            );

            butTiePoints->setCursor(QCursor(Qt::PointingHandCursor));
            butPhotos->setCursor(QCursor(Qt::PointingHandCursor));
            butFrustum->setCursor(QCursor(Qt::PointingHandCursor));

            hlScopeMethod->setSpacing(26);//10
            hlScopeMethod->addWidget(lblScopeMethod, 0, Qt::AlignLeft);
            hlScopeMethod->addWidget(butTiePoints, 0, Qt::AlignRight);
            hlScopeMethod->addWidget(butPhotos, 0, Qt::AlignRight);
            hlScopeMethod->addWidget(butFrustum, 0, Qt::AlignRight);

            QHBoxLayout* hlShowOrHideDetails = new QHBoxLayout();
            setContentsMargins(0, 0, 0, 0);

            butShowOrHideDetails = new QPushButton(ui->tab_4);
            ///butShowOrHideDetails->setText("Show Details");
            butShowOrHideDetails->setStyleSheet("margin-left:41px;background-color:transparent;");

            // Hide Details / Show Details

            butIcon4ShowOrHideDetails = new QPushButton(ui->tab_4);
            //lblIcon4ShowOrHideDetails->setPixmap(QPixmap(":/new/prefix1/skin/hide_details.png"));
            ///butIcon4ShowOrHideDetails->setIcon(QPixmap(":/new/prefix1/skin/show_details.png"));
            butIcon4ShowOrHideDetails->setStyleSheet("width:9px;height:8px;margin-top:6px;");

            hlShowOrHideDetails->setSpacing(7);
            hlShowOrHideDetails->addWidget(butShowOrHideDetails, 0, Qt::AlignLeft);
            hlShowOrHideDetails->addWidget(butIcon4ShowOrHideDetails, 0, Qt::AlignLeft);
            hlShowOrHideDetails->addStretch(1);

            QHBoxLayout* hlROIX = new QHBoxLayout();
            hlROIX->setContentsMargins(41, 0, 84, 0);
            hlROIX->setSpacing(0);

            lblROIXUnit = new QLabel(ui->tab_4);
            lblROIXMin = new QLabel(ui->tab_4);
            leROIXMin = new QLineEdit(ui->tab_4);
            lblROIXMax = new QLabel(ui->tab_4);
            leROIXMax = new QLineEdit(ui->tab_4);

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblROIXUnit->setText("X(米):");
                lblROIXMin->setText("最小值 -");
                lblROIXMax->setText("最大值 -");
            }
            else
            {
                lblROIXUnit->setText("X(meters):");
                lblROIXMin->setText("min -");
                lblROIXMax->setText("max -");
            }

            lblROIXUnit->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIXMin->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIXMax->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            leROIXMin->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\"; }"
                "QLineEdit:disabled { background-color:#34363A;color:#4DFFFFFF;border:1px solid gray;}"
            );
            leROIXMax->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\";} "
                "QLineEdit:disabled { background-color:#34363A; color:#4DFFFFFF;border:none;}"
            );

            //QRegExp regexp4ROI("(([0-9])|([0-9]\.[0-9]{1,6})|([1-9][0-9]*)|([1-9][0-9]*\.[0-9]{1,6}))");
            //QRegExp regexp4ROI("(([0-9])|([0-9]\.[0-9]{1,6}))");
            QRegExp regexp4ROI("((\\d)+(\\.)?(\\d){0,6})");
            QRegExpValidator* regexpValidator4ROI = new QRegExpValidator(regexp4ROI);

            leROIXMin->setValidator(regexpValidator4ROI);
            leROIXMax->setValidator(regexpValidator4ROI);

            hlROIX->addWidget(lblROIXUnit);
            hlROIX->addSpacing(20);
            hlROIX->addWidget(lblROIXMin);
            hlROIX->addSpacing(6);
            hlROIX->addWidget(leROIXMin, 1);
            hlROIX->addSpacing(20);
            hlROIX->addWidget(lblROIXMax);
            hlROIX->addSpacing(6);
            hlROIX->addWidget(leROIXMax, 1);

            QHBoxLayout* hlROIY = new QHBoxLayout();
            hlROIY->setContentsMargins(41, 0, 84, 0);

            lblROIYUnit = new QLabel(ui->tab_4);
            lblROIYMin = new QLabel(ui->tab_4);
            leROIYMin = new QLineEdit(ui->tab_4);
            lblROIYMax = new QLabel(ui->tab_4);
            leROIYMax = new QLineEdit(ui->tab_4);
            
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblROIYUnit->setText("Y(米):");
                lblROIYMin->setText("最小值 -");
                lblROIYMax->setText("最大值 -");
            }
            else
            {
                lblROIYUnit->setText("Y(meters):");
                lblROIYMin->setText("min -");
                lblROIYMax->setText("max -");
            }
            lblROIYUnit->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIYMin->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIYMax->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            leROIYMin->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\"; }"
                "QLineEdit:disabled { background-color:#34363A; color:#4DFFFFFF;border:none;}"
            );
            leROIYMax->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\"; }"
                "QLineEdit:disabled { background-color:#34363A; color:#4DFFFFFF;border:none;}"
            );

            leROIYMin->setValidator(regexpValidator4ROI);
            leROIYMax->setValidator(regexpValidator4ROI);

            hlROIY->setSpacing(0);
            hlROIY->addWidget(lblROIYUnit);
            hlROIY->addSpacing(20);
            hlROIY->addWidget(lblROIYMin);
            hlROIY->addSpacing(6);
            hlROIY->addWidget(leROIYMin, 1);
            hlROIY->addSpacing(20);
            hlROIY->addWidget(lblROIYMax);
            hlROIY->addSpacing(6);
            hlROIY->addWidget(leROIYMax, 1);

            QHBoxLayout* hlROIZ = new QHBoxLayout();
            hlROIZ->setContentsMargins(41, 0, 84, 0);
            hlROIZ->setSpacing(0);

            lblROIZUnit = new QLabel(ui->tab_4);
            lblROIZMin = new QLabel(ui->tab_4);
            leROIZMin = new QLineEdit(ui->tab_4);
            lblROIZMax = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblROIZUnit->setText("Z(米):");
                lblROIZMin->setText("最小值 -");
                lblROIZMax->setText("最大值 -");
            }
            else
            {
                lblROIZUnit->setText("Z(meters):");
                lblROIZMin->setText("min -");
                lblROIZMax->setText("max -");
            }
            leROIZMax = new QLineEdit(ui->tab_4);
            lblROIZUnit->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIZMin->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblROIZMax->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            leROIZMin->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\"; }"
                "QLineEdit:disabled { background-color:#34363A; color:#4DFFFFFF;border:none;}"
            );
            //  "QLineEdit { height:36px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\";}"
            leROIZMax->setStyleSheet(
                "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\";}"
                "QLineEdit:disabled { background-color:#34363A; color:#4DFFFFFF;border:none;}"
            );

            leROIZMin->setValidator(regexpValidator4ROI);
            leROIZMax->setValidator(regexpValidator4ROI);

            //QRegExp rx("^(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*)){1,20}$");
            //QRegExpValidator* pReg = new QRegExpValidator(rx, nullptr);

            hlROIZ->addWidget(lblROIZUnit);
            hlROIZ->addSpacing(20);
            hlROIZ->addWidget(lblROIZMin);
            hlROIZ->addSpacing(6);
            hlROIZ->addWidget(leROIZMin, 1);
            hlROIZ->addSpacing(20);
            hlROIZ->addWidget(lblROIZMax);
            hlROIZ->addSpacing(6);
            hlROIZ->addWidget(leROIZMax, 1);

            QFrame* lineSplitterTop = new QFrame(ui->tab_4);
            lineSplitterTop->setFrameShape(QFrame::HLine);
            lineSplitterTop->setFrameShadow(QFrame::Plain);
            lineSplitterTop->setStyleSheet("border:none;max-height:1px;background-color:rgb(121,121,121);margin-left:28px;margin-right:28px;padding-left:0px;");

            QLabel* lblTiling = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTiling->setText("分块");
            }
            else
            {
                lblTiling->setText("Tiling");
            }
            //lblTiling->setStyleSheet("color:white;padding-left:25px;");
            lblTiling->setStyleSheet("background-color:rgb(40,40,40);color:white;font:bold 16px \"Arial\";padding-left:26px;margin:2px;");

            //butCenterRight->setStyleSheet("background-color:rgb(40,40,40);border:1px solid #484848;");

            QHBoxLayout* hlTileCategory = new QHBoxLayout();
            hlTileCategory->setContentsMargins(28, 0, 28, 0);

            cbbTileCategory = new QComboBox(ui->tab_4);
            if (BlockObject::isChineseVersion())
            {
            // !!!:note that current selected item may bring error to relevant logic if filling chinese items into this combobox control.
                cbbTileCategory->addItem("No Tiling");
                cbbTileCategory->addItem("Regular planar grid");
                cbbTileCategory->addItem("Regular volumetric grid");
                cbbTileCategory->addItem("Adaptive Tiling");
            }
            else
            {
                cbbTileCategory->addItem("No Tiling");
                cbbTileCategory->addItem("Regular planar grid");
                cbbTileCategory->addItem("Regular volumetric grid");
                cbbTileCategory->addItem("Adaptive Tiling");
            }

            //cbbTileCategory->setStyleSheet("margin-left:25px;margin-right:25px;");
            //cbbTileCategory->setStyleSheet("background-white:white;color:black;border:1px solid #484848;");

            if (!bSupportMoreTileMode)
            {
                QVariant zerov(0);
                cbbTileCategory->setItemData(1, zerov, Qt::UserRole - 1);
                cbbTileCategory->setItemData(2, zerov, Qt::UserRole - 1);

                //  cbbTileCategory->setItemData(1, Qt::lightGray, Qt::BackgroundColorRole);
                //  cbbTileCategory->setItemData(2, Qt::lightGray, Qt::BackgroundColorRole);
                cbbTileCategory->setItemData(1, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
                cbbTileCategory->setItemData(2, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
            }

            cbbTileCategory->setStyleSheet(QString::fromUtf8("\n"
                "QComboBox {\n"
                "   border: 0px solid gray;   \n"
                "   border-radius: 4px;   \n"
                "   height:36px;\n"
                "   color: #FFFFFF;\n"
                "   font: 14px \"Arial\";\n"
                "   background-color:#34363A;\n"
                "   margin-left:0px; \n"
                "   margin-right:0px; \n"
                "   padding:0px;\n"
                "   padding-left: 11px;\n"
                "   padding-right:0px;"
                "}\n"
                "QComboBox:disabled {\n"
                "   color: white;\n"
                "   background-color:gray;\n"
                "}\n"
                "QComboBox::drop-down { \n"
                "   subcontrol-position:top right;\n"
                "   subcontrol-origin:padding;\n"
                "   width:32px;\n"
                "   border:none;\n"
                "}\n"
                "QComboBox::down-arrow { \n"
                "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView {\n"
                "    outline: 0px solid gray;   \n"
                "    border: 0px solid;   \n"
                "    color:#FFFFFF;\n"
                "    background-color: #131313;  \n"
                "    selection-background-color:#333333;   \n"
                "    padding-left: 0px; \n"
                "    margin-left:0px; \n"
                "    margin-right:0px; \n"
                "    border-radius:4px;\n"
                "}\n"
                "QComboBox QAbstractScrollArea {\n"
                "    width: 10px;\n"
                "    color: black; \n"
                "    background-color:white;\n"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item {\n"
                "    height: 38px;   \n"
                "    background-color:#3F4146;\n"
                "    color:#FFFFFF;"
                "    padding-left: 0px; \n"
                "    margin-left:0px; \n"
                "    margin-right:0px; \n"
                "    padding-left:10px;\n"
                "    font:14px solid #FFFFFF;"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item:hover {\n"
                "    color: #FFFFFF;\n"
                "    background-color: #34363A;   \n"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item:selected {\n"
                "    color: #FFFFFF;\n"
                "    background-color:#34363A;\n"
                "}\n"
                "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
                "    width: 10px;\n"
                "    background-color: #d0d2d4;  \n"
                "}\n"
                "\n"
                "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
                "    border-radius: 5px;   "
                "    background: rgb(160,160"
                ",160);   \n"
                "}\n"
                "\n"
                "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
                "    background: rgb(90, 91, 93);   \n"
                "}\n"
            ));

            QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
            cbbTileCategory->setItemDelegate(itemDelegate);

            //
            /*cbbTileCategory->setStyleSheet(QString::fromUtf8("\n"
                "QComboBox {\n"
                "    border: 0px solid gray;   \n"
                "    border-radius: 3px;   \n"
                "    color: #FFFFFF;\n"
                "   font: 14px \"Arial\";\n"
                "   background-color:#34363A;\n"
                "   margin-left:0px; \n"
                "   margin-right:0px; \n"
                "   padding-left: 3px\n"
                "}\n"
                "QComboBox:disabled {\n"
                "   color: white;\n"
                "   background-color:gray;\n"
                "}\n"
                "QComboBox::drop-down { \n"
                "   subcontrol-position:top right;\n"
                "   subcontrol-origin:padding;\n"
                "   width:32px;\n"
                "   border:none;\n"
                "}\n"
                "QComboBox::down-arrow { \n"
                "   image:url(:/new/prefix1/skin/cb_down_arrow1516.png);"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView {\n"
                "    outline: 0px solid gray;   \n"
                "    border: 0px solid;   \n"
                "    color:#FFFFFF;\n"
                "    background-color: #131313;  \n"
                "    selection-background-color:#333333;   \n"
                "    padding-left: 0px; \n"
                "    margin-left:0px; \n"
                "    margin-right:0px; \n"
                "}\n"
                "QComboBox QAbstractScrollArea {\n"
                "    width: 10px;\n"
                "    color: black; \n"
                "    background-color:white;\n"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item {\n"
                "    height: 50px;   \n"
                "    background-color:white;\n"
                "    padding-left: 0px; \n"
                "    margin-left:0px; \n"
                "    margin-right:0px; \n"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item:hover {\n"
                "    color: #FFFFFF;\n"
                "    background-color: rgb(22,22,22);   \n"
                "}\n"
                "\n"
                "QComboBox QAbstractItemView::item:selected {\n"
                "    color: #FFFFFF;\n"
                "    background-color:rgb(22,22,22);\n"
                "}\n"
                "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
                "    width: 10px;\n"
                "    background-color: #d0d2d4;  \n"
                "}\n"
                "\n"
                "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
                "    border-radius: 5px;   "
                "    background: rgb(160,160"
                ",160);   \n"
                "}\n"
                "\n"
                "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
                "    background: rgb(90, 91, 93);   \n"
                "}\n"
            ));*/
            ///cbbTileCategory->setFixedHeight(33);
            cbbTileCategory->setFixedHeight(28);

            hlTileCategory->addWidget(cbbTileCategory, 1);

            QHBoxLayout* hlTileExtra = new QHBoxLayout();
            lblTileExtra = new QLabel(ui->tab_4);
            leTileExtra = new QLineEdit(ui->tab_4);
            hlTileExtra->addWidget(lblTileExtra, 1);
            hlTileExtra->addWidget(leTileExtra, 1);
            hlTileExtra->addStretch(1);
            hlTileExtra->setContentsMargins(2, 2, 2, 2);

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblTileExtra->setText("最大可用内存（GB）:");
            }
            else {
                lblTileExtra->setText("Target RAM usage（GB）:");
            }

            leTileExtra->setText("");
            lblTileExtra->setStyleSheet("background-color:rgb(40,40,40);color:white;padding-left:28px;font:14px \"Arial\";");
            /*leTileExtra->setStyleSheet("QLineEdit { background-color:white;color:black;font: 14px \"Arial\";}"
                "QLineEdit:disabled {background-color:gray;color:white;border:1px solid gray;}"*/
                leTileExtra->setStyleSheet(
                    "QLineEdit { height:28px;background-color:#34363A;color:white;border-radius:4px;padding-left:10px;font:14px \"Arial\"; }"
                    "QLineEdit:disabled { background-color:#34363A;color:#4DFFFFFF;border:1px solid gray;}"
                );
            leTileExtra->setFixedHeight(27);

            QRegExp regexp("((\\d){1,3}(\\.)?(\\d){0,1})");
            QRegExpValidator* regexpValidator = new QRegExpValidator(regexp);
            leTileExtra->setValidator(regexpValidator);

            vlCenterRight->addWidget(lblCenterRightTitle);
            //vlCenterRight->addSpacing(25);
            vlCenterRight->addLayout(hlROIAction);
            vlCenterRight->addLayout(hlScopeMethod);
            vlCenterRight->addLayout(hlROIX);
            vlCenterRight->addSpacing(7);
            vlCenterRight->addLayout(hlROIY);
            vlCenterRight->addSpacing(7);
            vlCenterRight->addLayout(hlROIZ);
            vlCenterRight->addLayout(hlShowOrHideDetails);
            vlCenterRight->addSpacing(5);
            vlCenterRight->addWidget(lineSplitterTop);
            vlCenterRight->addSpacing(5);
            vlCenterRight->addWidget(lblTiling);
            //vlCenterRight->addWidget(cbbTileCategory);
            vlCenterRight->addLayout(hlTileCategory);

            vlCenterRight->addLayout(hlTileExtra);

            QFrame* lineSplitter = new QFrame(ui->tab_4);
            lineSplitter->setFrameShape(QFrame::HLine);
            lineSplitter->setFrameShadow(QFrame::Plain);
            lineSplitter->setStyleSheet("border:none;max-height:1px;background-color:rgb(121,121,121);margin-left:28px;margin-right:28px;padding-left:0px;");

            vlCenterRight->addSpacing(5);
            vlCenterRight->addWidget(lineSplitter);
            vlCenterRight->addSpacing(5);

            QLabel* lblCenterRightOverview = new QLabel(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblCenterRightOverview->setText("重建概况");
            }
            else {
                lblCenterRightOverview->setText("Overview");
            }
            lblCenterRightOverview->setStyleSheet("background-color:rgb(40,40,40);color:white;padding-left:26px;font:bold 16px \"Arial\";margin:2px;");

            QHBoxLayout* hlOverviewROIDimension = new QHBoxLayout();
            hlOverviewROIDimension->setContentsMargins(0, 0, 0, 0);

            QLabel* lblOverviewROIDimensionIcon = new QLabel(ui->tab_4);
            lblOverviewROIDimensionIcon->setPixmap(QPixmap(":/new/prefix1/skin/circle_nine.png"));
            lblOverviewROIDimension = new QLabel(ui->tab_4);
            //lblOverviewROIDimension->setText("ROI Dimension : 2874 meters x 3950 meters x 88 meters");
            SetOverviewROIDimension(2874, 3950, 88);

            lblOverviewROIDimensionIcon->setStyleSheet("padding-left:26px;margin:2px;background-color:rgb(40,40,40);");
            lblOverviewROIDimension->setStyleSheet("background-color:rgb(40,40,40);font:14 \"Arial\";");

            hlOverviewROIDimension->setSpacing(10);
            hlOverviewROIDimension->addWidget(lblOverviewROIDimensionIcon);
            hlOverviewROIDimension->addWidget(lblOverviewROIDimension);
            hlOverviewROIDimension->addStretch(1);

            QHBoxLayout* hlCenterRightOverview = new QHBoxLayout();
            hlCenterRightOverview->setContentsMargins(0, 0, 0, 0);


            QLabel* lblCenterRightOverviewDetailIcon = new QLabel(ui->tab_4);
            lblCenterRightOverviewDetailIcon->setPixmap(QPixmap(":/new/prefix1/skin/circle_nine.png"));
            lblCenterRightOverviewDetailIcon->setStyleSheet("background-color:rgb(40,40,40);padding-left:26px;margin:2px;");

            QLabel* lblCenterRightOverviewDetailLeft = new QLabel(ui->tab_4);
            lblCenterRightOverviewDetailLeft->setStyleSheet("background-color:rgb(40,40,40);font:14 \"Arial\";color:rgb(223,223,223);margin-right:0px;padding-right:0px;");
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblCenterRightOverviewDetailLeft->setText("块数量:");
            }
            else {
                lblCenterRightOverviewDetailLeft->setText("The tiling contains");
            }

            lblCenterRightOverviewDetail = new QLabel(ui->tab_4);
            lblCenterRightOverviewDetail->setStyleSheet("background-color:rgb(40,40,40);font:14 bold \"Arial\";color:rgb(255,255,255);margin-left:0px;margin-right:0px;padding-left:0px;padding-right:0px;");
            lblCenterRightOverviewDetail->setText("87");

            QLabel* lblCenterRightOverviewDetailRight = new QLabel(ui->tab_4);
            lblCenterRightOverviewDetailRight->setStyleSheet("background-color:rgb(40,40,40);font:14 \"Arial\";color:rgb(223,223,223);margin-left:0px;padding-left:0px;");
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblCenterRightOverviewDetailRight->setText("块");
            }
            else {
                lblCenterRightOverviewDetailRight->setText("tile(s).");
            }

            hlCenterRightOverview->setSpacing(10);
            hlCenterRightOverview->addWidget(lblCenterRightOverviewDetailIcon);
            hlCenterRightOverview->addWidget(lblCenterRightOverviewDetailLeft);
            hlCenterRightOverview->addWidget(lblCenterRightOverviewDetail);
            hlCenterRightOverview->addWidget(lblCenterRightOverviewDetailRight);
            hlCenterRightOverview->addStretch(1);

            QHBoxLayout* hlExpectedMaxRamUsage = new QHBoxLayout();
            QLabel* lblExpectedMaxRamUsageIcon = new QLabel(ui->tab_4);
            QLabel* lblExpectedMaxRamUsageTitle = new QLabel(ui->tab_4);
            lblExpectedMaxRamUsage = new QLabel(ui->tab_4);

            hlExpectedMaxRamUsage->setContentsMargins(0, 0, 0, 0);
            hlExpectedMaxRamUsage->setSpacing(10);
            lblExpectedMaxRamUsageIcon->setPixmap(QPixmap(":/new/prefix1/skin/circle_nine.png"));
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblExpectedMaxRamUsageTitle->setText("任务期望的最大可用内存 :");
            }
            else
            {
                lblExpectedMaxRamUsageTitle->setText("Expected maximum RAM usage for a job  :");
            }
            //lblExpectedMaxRamUsage->setText("123GB");
            lblExpectedMaxRamUsageIcon->setStyleSheet("background-color:rgb(40,40,40);padding-left:26px;margin:2px;");
            //lblExpectedMaxRamUsageTitle->setStyleSheet("font:14px \"Arial\";");
            lblExpectedMaxRamUsageTitle->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            lblExpectedMaxRamUsage->setStyleSheet("background-color:transparent;color:white;font:14px \"Arial\";");
            //lblExpectedMaxRamUsage->setStyleSheet("font:bold 14px \"Arial\";");
            /*SetExpectedMaxRamUsage(123);*/

            hlExpectedMaxRamUsage->addWidget(lblExpectedMaxRamUsageIcon);
            hlExpectedMaxRamUsage->addWidget(lblExpectedMaxRamUsageTitle);
            hlExpectedMaxRamUsage->addWidget(lblExpectedMaxRamUsage);
            hlExpectedMaxRamUsage->addStretch(1);

            QHBoxLayout* hlOverviewWarning = new QHBoxLayout();
            lblOverviewWarningIcon = new QLabel(ui->tab_4);
            lblOverviewWarning = new QLabel(ui->tab_4);

            lblOverviewWarningIcon->setPixmap(QPixmap(":/new/prefix1/skin/warning_nine.png"));
            lblOverviewWarningIcon->setStyleSheet("padding-left:26px;margin:2px;background-color:rgb(40,40,40);");

            //lblOverviewWarning->setText("Warning:Invalid spatial framework,please adjust parameters.");
            lblOverviewWarning->setStyleSheet("font:14px \"Arial\";color:#F53F3F;");

            hlOverviewWarning->setContentsMargins(0, 0, 19, 20);
            hlOverviewWarning->setSpacing(12);
            hlOverviewWarning->addWidget(lblOverviewWarningIcon);
            hlOverviewWarning->addWidget(lblOverviewWarning);
            hlOverviewWarning->addStretch(1);

            QHBoxLayout* hlLineGeometryContraints = new QHBoxLayout();
            hlLineGeometryContraints->setContentsMargins(28, 0, 28, 0);

            lineGeometryContraints = new QFrame(ui->tab_4);
            lineGeometryContraints->setFrameShape(QFrame::HLine);
            lineGeometryContraints->setFrameShadow(QFrame::Plain);
///         lineGeometryContraints->setStyleSheet("border:none;max-height:1px;background-color:rgb(121,121,121);margin-left:28px;margin-right:28px;padding-left:0px;");
            lineGeometryContraints->setStyleSheet("border:none;max-height:1px;background-color:rgb(121,121,121);margin-left:0px;margin-right:0px;padding-left:0px;");
            hlLineGeometryContraints->addWidget(lineGeometryContraints, 1);

            QHBoxLayout* hlGeometryContraintsTitle = new QHBoxLayout();
            hlGeometryContraintsTitle->setContentsMargins(28, 0, 28, 0);

            lblGeometryContraintsTitle = new QLabel(ui->tab_4);
            butGeometryContraintsImport = new QPushButton(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblGeometryContraintsTitle->setText("几何约束");
            }
            else {
                lblGeometryContraintsTitle->setText("Geometry Constraints");
            }
            lblGeometryContraintsTitle->setStyleSheet("background-color:transparent; color:#FFFFFF; font:bold 16px \"Arial\"");

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                butGeometryContraintsImport->setText("导入");
            }
            else
            {
                butGeometryContraintsImport->setText("Import");
            }
            //butGeometryContraintsImport->setStyleSheet("background-color:transparent; color:rgb(22,93,255); font:16px \"Arial\; width:54px; height:24px; border:1px solid rgb(22,93,255); ");

            butGeometryContraintsImport->setStyleSheet("QPushButton { background-color:#00165DFF;color:#FF165DFF;border-radius:2px;border:1px solid #FF165DFF;font:14px \"Arial\";width:54px;height:24px;}"
                "QPushButton:disabled{background-color:#00165DFF; color:#646D83;border:1px solid #99646D83;}");

            hlGeometryContraintsTitle->addWidget(lblGeometryContraintsTitle, 0, Qt::AlignLeft);
            hlGeometryContraintsTitle->addStretch(1);
            hlGeometryContraintsTitle->addWidget(butGeometryContraintsImport, 0, Qt::AlignRight);

            QHBoxLayout* hlGeometryContraints = new QHBoxLayout();
            hlGeometryContraints->setContentsMargins(28, 0, 28, 0);

            twGeometryContraints = new QTableWidget(ui->tab_4);
            twGeometryContraints->setColumnCount(3);
            //twGeometryContraints->setRowCount(5);
            twGeometryContraints->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            twGeometryContraints->verticalHeader()->hide();
            twGeometryContraints->setSelectionMode(QAbstractItemView::SingleSelection);
            twGeometryContraints->setSelectionBehavior(QAbstractItemView::SelectRows);
            twGeometryContraints->setColumnHidden(2, true);
            twGeometryContraints->setContextMenuPolicy(Qt::CustomContextMenu);
            twGeometryContraints->setFixedHeight(170);
            twGeometryContraints->setShowGrid(false);
            ///QFont font = twGeometryContraints->font();
            ///font.setPixelSize(12);
            ///twGeometryContraints->setFont(font);
            twGeometryContraints->setStyleSheet(
                "QTableWidget { font:12px \"Arial\"; border:1px solid rgb(44,47,52); outline:none; } "
                "QTableWidget::item { background-color:transparent; color:rgb(255,255,255); font:12px \"Arial\; boder:none; border-bottom:1px solid rgb(44,47,52); } "
                "QTableWidget::item:selected { background-color:#FF2A4D84; } "
            );

            twGeometryContraints->horizontalHeader()->setStyleSheet(
                "QHeaderView::section { border:none; background-color:rgb(44,47,52); color:rgb(165,165,165); font:12px \"Arial\;}"
            );//solid

            QStringList slGeometryContraintsHeader;
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                slGeometryContraintsHeader << "名称" << "类型" << "纹理替换";
            }
            else {
                slGeometryContraintsHeader << "Name" << "Type" << "Texture replacement";
            }

            twGeometryContraints->setHorizontalHeaderLabels(slGeometryContraintsHeader);

///         hlGeometryContraints->addWidget(twGeometryContraints, 1);
            hlGeometryContraints->addWidget(twGeometryContraints);

            menu_RightClick4GeometryContraints = new QMenu(twGeometryContraints);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                action_delete4GeometryContraints = new QAction("删除", twGeometryContraints);
            }
            else {
                action_delete4GeometryContraints = new QAction("Delete", twGeometryContraints);
            }
            menu_RightClick4GeometryContraints->addAction(action_delete4GeometryContraints);

            QHBoxLayout* hlNewReconstruction2 = new QHBoxLayout();
            //hlNewReconstruction2->setContentsMargins(20, 0, 19, 20);
            hlNewReconstruction2->setContentsMargins(20, 0, 19, 10);

            //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 1)
            {
                lblMoreSettings = new QLabel(ui->tab_4);
                if (BlockObject::isChineseVersion())
                {
                    lblMoreSettings->setText("<a style='color:#B0B5E8;' href='http://'>更多设置</a>");
                }
                else
                {
                    lblMoreSettings->setText("<a style='color:#B0B5E8;' href='http://'>More Settings</a>");
                }

                lblMoreSettings->setStyleSheet(QString::fromUtf8(
                    "background-color:transparent;\n"
                    "color:#B0B5E8;\n"
                    "border-radius:0px;\n"
                    "margin-left:0px;\n"
                    "padding-left:0px;\n"
                    "border:none;\n"
                ));
                
                ///if (this->recons_object_->HasProductions())
                ///{
                /// lblMoreSettings->setEnabled(false);
                /// lblMoreSettings->setText("<a style='color:#B8B8B8;' href='http://'>More Settings</a>");
                ///}
                ///else
                {
                    lblMoreSettings->setEnabled(true);
                    if (BlockObject::isChineseVersion())
                    {
                        lblMoreSettings->setText("<a style='color:#B0B5E8;' href='http://'>更多设置</a>");
                    }
                    else
                    {
                        lblMoreSettings->setText("<a style='color:#B0B5E8;' href='http://'>More Settings</a>");
                    }
                }

                QFont font;
                font.setUnderline(false);
                font.setPixelSize(12);
                lblMoreSettings->setFont(font);
                
                connect(lblMoreSettings, &QLabel::linkActivated, this, &ConstructionWgt::Slot_MoreSettings);
                hlNewReconstruction2->addWidget(lblMoreSettings);
            }
            /*else
            {
                butMoreSettings = new QPushButton(ui->tab_4);
                butMoreSettings->setText("More Settings");
                butMoreSettings->setFixedWidth(147);
                ///butMoreSettings->setFixedHeight(42);
                butMoreSettings->setFixedHeight(32);
                ///butMoreSettings->setStyleSheet("width:147px;height:42px;border-radius:2px;border:0px solid;font:14px \"Arial\";");
                butMoreSettings->setStyleSheet("width:147px;height:32px;border-radius:2px;border:0px solid;font:14px \"Arial\";");
                hlNewReconstruction2->addWidget(butMoreSettings);
                connect(butMoreSettings, &QPushButton::clicked, this, &ConstructionWgt::Slot_MoreSettings);
            }*/
            butNewProduction2 = new QPushButton(ui->tab_4);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                butNewProduction2->setText("提交生产");
            }
            else {
                butNewProduction2->setText("Submit Production");
            }
            butNewProduction2->setFixedWidth(147);
            ///butNewReconstruction2->setFixedHeight(42);
            butNewProduction2->setFixedHeight(32);

            //butNewReconstruction2->setStyleSheet("background-color:#0072BE;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            ///butNewReconstruction2->setStyleSheet("background-color:#0072BE;color:white;width:147px;height:42px;border-radius:2px;border:0px solid;font:14px \"Arial\";margin-right:30px;margin-bottom:23px;");
            ///"QPushButton { background-color:#0072BE;color:white;width:147px;height:42px;border-radius:2px;border:0px solid;font:14px \"Arial\";"
            butNewProduction2->setStyleSheet(QString::fromUtf8(
                "QPushButton { background-color:#0072BE;color:white;width:147px;height:32px;border-radius:2px;border:0px solid;font:14px \"Arial\";"
                "}"
                "QPushButton:pressed {"
                "background-color:#3F455C;"
                "}"
            ));
            ///         hlNewReconstruction->addStretch(1);
            
            butNewProduction2->setEnabled(valid ? true : false);
            if (!valid)
            {
                butNewProduction2->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
                butNewProduction->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            }
            hlNewReconstruction2->addStretch(1);
//          hlNewReconstruction2->addWidget(butNewReconstruction2, 0, Qt::AlignRight);
            hlNewReconstruction2->addWidget(butNewProduction2);

            vlCenterRight->addWidget(lblCenterRightOverview);
            //vlCenterRight->addWidget(lblCenterRightOverviewDetail);
            vlCenterRight->addSpacing(10);
            vlCenterRight->addLayout(hlOverviewROIDimension);
            vlCenterRight->addSpacing(10);
            vlCenterRight->addLayout(hlCenterRightOverview);
            vlCenterRight->addSpacing(10);
            vlCenterRight->addLayout(hlExpectedMaxRamUsage);
            vlCenterRight->addLayout(hlOverviewWarning);

            vlCenterRight->addStretch(1);
            //vlCenterRight->addWidget(lineGeometryContraints);
            vlCenterRight->addLayout(hlLineGeometryContraints);
            vlCenterRight->addLayout(hlGeometryContraintsTitle);
            //vlCenterRight->addWidget(twGeometryContraints);

            vlCenterRight->addLayout(hlGeometryContraints);
            ///vlCenterRight->addStretch(1);            
            vlCenterRight->addSpacing(7);
            vlCenterRight->addLayout(hlNewReconstruction2);

            ///vlCenterRight->addWidget(butCenterRight, 1);

            panelCenterRight->setLayout(vlCenterRight);

            hlCenterArea->setSpacing(12);
        /// hlCenterArea->addLayout(vlCenterLeft);
///         hlCenterArea->addWidget(lblCenterLeft, 3);
 
            cbPhotos = new QCheckBox(this);
            if (BlockObject::isChineseVersion()) {
                cbPhotos->setText("影像");
            }
            else {
                cbPhotos->setText("Photos");
            }
            cbPhotos->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");

            cbPhotos->setChecked(false);
            if (this->recons_object_->GetATData().HasImages())
            {
        
                cbPhotos->setEnabled(true);
            }
            else
            {
    
                cbPhotos->setEnabled(false);
            }

            cbTiePoints = new QCheckBox(this);
            if (BlockObject::isChineseVersion()) {
                cbTiePoints->setText("连接点");
            }
            else {
                cbTiePoints->setText("TiePoints");
            }
            cbTiePoints->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
            if (this->recons_object_->GetATData().HasTiepoints())
            {
                cbTiePoints->setChecked(true);
                cbTiePoints->setEnabled(true);
            }
            else
            {
                cbTiePoints->setChecked(false);
                cbTiePoints->setEnabled(false);
            }

            cbGCP = new QCheckBox(this);
            if (BlockObject::isChineseVersion())
            {
                cbGCP->setText("控制点");
            }
            else
            {
                cbGCP->setText("GCP");
            }
            cbGCP->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    
            if (this->recons_object_->GetATData().HasSurveyPoints())
            {
                cbGCP->setChecked(true);
                cbGCP->setEnabled(true);
            }
            else
            {
                cbGCP->setChecked(false);
                cbGCP->setEnabled(false);
            }
            cbTiling = new QCheckBox(this);
            if (BlockObject::isChineseVersion())
            {
                cbTiling->setText("分块");
            }
            else {
                cbTiling->setText("Tiling");
            }
            cbTiling->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
    
            if (this->recons_object_->HasTiles())
            {
                cbTiling->setChecked(true);
                cbTiling->setEnabled(true);
            }
            else
            {
                cbTiling->setChecked(false);
                cbTiling->setEnabled(false);
            }
            cbROI = new QCheckBox(this);
            if (BlockObject::isChineseVersion())
            {
                cbROI->setText("兴趣区");
            }
            else
            {
                cbROI->setText("ROI");
            }
            cbROI->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
            //if (this->recons_object_->HasBoundary())
            {
                cbROI->setChecked(true);
                cbROI->setEnabled(true);
            }
            /*else
            {
                cbROI->setChecked(false);
                cbROI->setEnabled(false);
            }*/
    
            cbConstraints = new QCheckBox(this);
            if (BlockObject::isChineseVersion())
            {
                cbConstraints->setText("约束");
            }
            else
            {
                cbConstraints->setText("Constraints");
            }
            cbConstraints->setStyleSheet("QCheckBox { background-color:transparent;color:white;font:14px \"Arial\";} QCheckBox:disabled {color: gray;}");
            if (this->recons_object_->HasConstraints())
            {
                cbConstraints->setChecked(true);
                cbConstraints->setEnabled(true);
            }
            else
            {
                cbConstraints->setChecked(false);
                cbConstraints->setEnabled(false);
            }

            hlTitle->addStretch();
            hlTitle->addWidget(cbPhotos);
            hlTitle->addSpacing(10);
            hlTitle->addWidget(cbTiePoints);
            hlTitle->addSpacing(10);
            hlTitle->addWidget(cbGCP);
            hlTitle->addSpacing(10);
            hlTitle->addWidget(cbTiling);
            hlTitle->addSpacing(10);
            hlTitle->addWidget(cbROI);
            hlTitle->addSpacing(10);
            hlTitle->addWidget(cbConstraints);
            hlTitle->addSpacing(30);

            vlCenterMiddle->addLayout(hlTitle);
            vlCenterMiddle->addWidget(mWindow, 1);

            /// hlCenterArea->addWidget(mWindow, 3);
            hlCenterArea->addLayout(vlCenterMiddle,3);

///         hlCenterArea->addLayout(vlCenterRight, 2);
            hlCenterArea->addWidget(panelCenterRight,2);

            ui->verticalLayout_6->addLayout(hlCenterArea, 1);
            //此处到底用哪个
            connect(butNewProduction, &QPushButton::clicked, this, &ConstructionWgt::Slot_SubmitProduction);
            connect(butNewProduction2, &QPushButton::clicked, this, &ConstructionWgt::Slot_SubmitProduction);

            tiling_mode_e tiling_mode = ReconstructionCommandSet::GetTilingMode(this->block_data_, this->recons_object_->GetId() /*this->recons_id*/);
            // will need to consider more tile mode in the future,change the following logic based on the actual situation later.
            if (tiling_mode == TILE_NONE)
            {
                cbbTileCategory->setCurrentIndex(0);
                lblTileExtra->setVisible(false);
                leTileExtra->setVisible(false);
            }
            else if (tiling_mode == TILE_PALNAR_GRID)
            {
                cbbTileCategory->setCurrentIndex(1);

                // how about related input fields / label fields?
                float extraTileValue = this->recons_object_->GetTilingDiscriptorMutual()->GetParamsMutual().regular_params_.tilesize_;
                
                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("瓦片尺寸(米):");
                    else
                        lblTileExtra->setText("瓦片尺寸(单位):");
                } 
                else {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("Tile size(meters):");
                    else
                        lblTileExtra->setText("Tile size(units):");
                }
                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);
            }
            else if (tiling_mode == TILE_VOL_GRID)
            {
                cbbTileCategory->setCurrentIndex(2);

                // how about related input fields / label fields?
                float extraTileValue = this->recons_object_->GetTilingDiscriptorMutual()->GetParamsMutual().regular_params_.tilesize_;

                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("瓦片尺寸(米):");
                    else
                        lblTileExtra->setText("瓦片尺寸(单位):");
                }
                else {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("Tile size(meters):");
                    else
                        lblTileExtra->setText("Tile size(units):");
                }
                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);
            }
            else
            {
                cbbTileCategory->setCurrentIndex(3);
                ///disconnect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);
                float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
                // if not adaptive mode,will need to change lblTileExtra to new label as "Tile size".
                leTileExtra->setText(QString::number(extraTileValue,'f',1));
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    lblTileExtra->setText("可用目标内存（GB）:");
                }
                else {
                    lblTileExtra->setText("Target RAM usage（GB）:");
                }
                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);
                ///connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);
            }

            if (this->recons_object_->GetProductionsMutual().size() > 0)
            {
                cbbTileCategory->setEnabled(false);
                leTileExtra->setEnabled(false);
            }

            //std::cout << "get num tiles:" << this->recons_object_->GetNumTiles(this->recons_object_->GetProcessingSettings().bdiscard_emptytiles_) << std::endl;

            lblCenterRightOverviewDetail->setText(QString::number(this->recons_object_->GetNumTiles(this->recons_object_->GetProcessingSettings().bdiscard_emptytiles_)));

            ui->tabWidget->setCurrentIndex(1);
    
            if (!this->recons_object_->GetATDataMutual().HasControlPoints())
            {
                cbGCP->setChecked(false);
                cbGCP->setEnabled(false);
            }
    
            if (!this->recons_object_->GetATDataMutual().HasImages())
            {
                cbPhotos->setChecked(false);
                cbPhotos->setEnabled(false);
            }
        
            if (!this->recons_object_->GetATDataMutual().HasTiepoints())
            {
                cbTiePoints->setChecked(false);
                cbTiePoints->setEnabled(false);
            }
    
            refresh_timer_ = new QTimer(this);      

            const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

            leROIXMin->setText(QString::number(box3d.min().x()));
            leROIYMin->setText(QString::number(box3d.min().y()));
            leROIZMin->setText(QString::number(box3d.min().z()));

            leROIXMax->setText(QString::number(box3d.max().x()));
            leROIYMax->setText(QString::number(box3d.max().y()));
            leROIZMax->setText(QString::number(box3d.max().z()));
            
            connect(refresh_timer_, &QTimer::timeout, this, &ConstructionWgt::Slot_Refresh_Timeout);
            connect(cbbTileCategory, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(Slot_CurrentIndexChanged(const QString&)));
            //connect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged, Qt::DirectConnection);
            connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);

            connect(this, &ConstructionWgt::Sig_DelayedResetTileSize, this, &ConstructionWgt::Slot_DelayedResetTileSize);
            connect(this, &ConstructionWgt::Sig_DelayedResetTileMAXRamUsage, this, &ConstructionWgt::Slot_DelayedResetTileMAXRamUsage);
            connect(this, &ConstructionWgt::Sig_DelayedResizeTilingMode, this, &ConstructionWgt::Slot_DelayedResizeTilingMode);

            connect(butROIEdit, &QPushButton::clicked, this, &ConstructionWgt::Slot_ROIEdit);
            connect(butROIImport, &QPushButton::clicked, this, &ConstructionWgt::Slot_ROIImport);
            connect(butROIDefault, &QPushButton::clicked, this, &ConstructionWgt::Slot_ROIDefault);

            ConnectSignalMap4ROIlLimits();

            connect(butTiePoints, &QPushButton::clicked, this, &ConstructionWgt::Slot_TiePoints);
            connect(butPhotos, &QPushButton::clicked, this, &ConstructionWgt::Slot_Photos);
            connect(butFrustum, &QPushButton::clicked, this, &ConstructionWgt::Slot_Frustum);

            connect(butShowOrHideDetails, &QPushButton::clicked, this, &ConstructionWgt::Slot_ToggleShowOrHideDetails);
            connect(butIcon4ShowOrHideDetails, &QPushButton::clicked, this, &ConstructionWgt::Slot_ToggleShowOrHideDetails);
            connect(butGeometryContraintsImport, &QPushButton::clicked, this, &ConstructionWgt::Slot_GeometryContraintsImport);

            connect(twGeometryContraints, &QTableWidget::itemClicked, this, &ConstructionWgt::Slot_ItemClicked);
            connect(twGeometryContraints, &QTableWidget::customContextMenuRequested, this, &ConstructionWgt::Slot_GeometryContraints_CustomContextMenuRequested);

            connect(this, &ConstructionWgt::Sig_IsModifiedProj, MohackerWin::GetInstance(), &MohackerWin::SetFileModifiedProj);

            //connect(mWindow, &MWindow::mwindow_resized, this, &ConstructionWgt::Slot_MWindowResized);
            connect(mWindow, &MWindow::signal_update_overview, this, &ConstructionWgt::Slot_UpdateROI);
            connect(mWindow, &MWindow::signal_roiedit_saved, this, &ConstructionWgt::Slot_ROIEdit_Saved);
            connect(mWindow, &MWindow::signal_roiedit_cancelled, this, &ConstructionWgt::Slot_ROIEdit_Cancelled);

            connect(action_delete4GeometryContraints, &QAction::triggered, this, &ConstructionWgt::Slot_GeometryContraints_Delete);

            connect(cbPhotos, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);
            connect(cbTiePoints, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);
            connect(cbGCP, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);
            connect(cbTiling, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);
            connect(cbROI, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);
            connect(cbConstraints, &QCheckBox::clicked, this, &ConstructionWgt::Slot_SelectTypes);

            refresh_timer_->start(1000);

            iLastDelayedAction = -1;

            SetButtonStates();
            ShowOrHideDetails();
            //SetOverviewWarning("Warning:Invalid spatial framework,please adjust parameters.");
            SetOverviewWarning("");
            ShowOverviewWarning(false);
            SetRightSideEditable(!this->recons_object_->HasProductions());
            RefreshOverviewInfo();
#if 1
            //block_data_->LoadTiepoints();
            std::cout << "inside " << " " << __FUNCTION__ << " " << __LINE__
                << " get atdata:" << recons_object->GetATDataCustom().GetImages().size() << " / "
                << recons_object->GetATDataCustom().GetPoints3D().size() << " : "
                << block->GetATDataMutual()->GetImages().size() << " / " 
                << block->GetATDataMutual()->GetPoints3D().size()                       
                << std::endl;

#if  1
            
///         mWindow->RenderReconstruction(recons_object);
///         osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
///         UserMatrixData::setCurrentMatrixObject(this, cmt);
#else
            mWindow->loadOsgFile(std::string("D:\\osgb\\Tile_+003_+004_L22_000013400.osgb"));
#endif

        
#endif

            slTextureReplacementOption.append("Texture replacement");
            slTextureReplacementOption.append("Texture option 2");
            slTextureReplacementOption.append("Texture option 3");
            slTextureReplacementOption.append("Texture option 4");
            slTextureReplacementOption.append("Texture option 5");

            ///InsertGeometryContraintsItem("PhotoGroup1", "kml", "Texture replacement");
            ///InsertGeometryContraintsItem("Name2", "kml", "Texture replacement2");
            ///InsertGeometryContraintsItem("Name3", "kml", "Texture replacement3");
            ///InsertGeometryContraintsItem("Name4", "kml", "Texture replacement4");
            ///InsertGeometryContraintsItem("Name5", "kml", "Texture replacement5");

            if (recons_object_ != nullptr)
            {
                ///std::cout << "inside " << __FILE__ << " " << __LINE__ << " recons_object_ not null:" << recons_object_->GetConstraintCustom().size() << std::endl;
                for (auto& t : recons_object_->GetConstraintCustom())
                {
                /// std::cout << "inside " << __FILE__ << " " << __LINE__ << " recons_object_:" << t.name_ <<  std::endl;
                    InsertGeometryContraintsItem(str2qstr(t.name_), "kml", "");
                }
            }
            else
            {
                /// std::cout << "inside " << __FILE__ << " " << __LINE__ << " recons_object_ null." << std::endl;
            }

            setTileCategoryExtra();
            SetLayerType();
            QTimer::singleShot(200, this, &ConstructionWgt::DoFirstRenderReconstruction);

            ///lastTileExtraValue = leTileExtra->text();
            lastTileModeIndex = cbbTileCategory->currentIndex();
        }

        ConstructionWgt::~ConstructionWgt()
        {
            if (mWindow != nullptr)
                delete mWindow;
            mWindow = nullptr;
        }

        void ConstructionWgt::setTileCategoryExtra()
        {
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                cbbTileCategory->setItemData(0, "不分块", Qt::DisplayRole);
                cbbTileCategory->setItemData(1, "二维规则分块", Qt::DisplayRole);
                cbbTileCategory->setItemData(2, "三维规则分块", Qt::DisplayRole);
                cbbTileCategory->setItemData(3, "自适应分块", Qt::DisplayRole);
            }
        }

        void ConstructionWgt::Slot_CurrentIndexChanged(const QString& text)
        {       
            /// note: be careful for chinese options for current combobox.
            /// value / display value / edit.
            /// setItemData
            tiling_mode_e tiling_mode;
            int currentIndex = cbbTileCategory->currentIndex();
            ///LOGI("currentIndex / text:" + std::to_string(currentIndex) + "/" + text.toStdString());
            
            // avoid processing it twice or more for same tile mode consecutively.
            if (lastTileModeIndex == currentIndex)
                return;
            
            LOGI("currentIndex :" + std::to_string(currentIndex));
            ///if (text.startsWith("No", Qt::CaseInsensitive))
            if (currentIndex == 0)
            {
#if 0           
                lblTileExtra->setVisible(false);
                //lblTileExtra->setText("         ");
                leTileExtra->setVisible(false);
#endif
                tiling_mode = TILE_NONE;
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                savedTilingMode = tiling_mode;
                iLastDelayedAction = 1;

                ///ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), tiling_mode);
                ///float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
            
                //emit Sig_DelayedResizeTilingMode(tiling_mode);

                QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResizeTilingMode);

                ///std::cout << "NONE Mode ram " << " " << extraTileValue <<  " tilenum "<< this->recons_object_->GetNumTiles() << std::endl;
            }
            ///else if(text.startsWith("Adaptive",Qt::CaseInsensitive))
            else if(currentIndex == 3)
            {
                ///disconnect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged);
#if 0
                lblTileExtra->setVisible(true);
                //lblTileExtra->setText("Target RAM usage（GB）:");
                leTileExtra->setVisible(true);
#endif
                tiling_mode = TILE_ADAPTIVE;
                
                ///ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), tiling_mode);
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                savedTilingMode = tiling_mode;
                iLastDelayedAction = 1;

                //emit Sig_DelayedResizeTilingMode(tiling_mode);

///             float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
///             leTileExtra->setText(QString::number(extraTileValue,'f',1));

                QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResizeTilingMode);
                ///connect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged, Qt::DirectConnection);
            }   
            ///else if (text.startsWith("Regular planar grid", Qt::CaseInsensitive))
            else if (currentIndex == 1)
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                tiling_mode = TILE_PALNAR_GRID;
                savedTilingMode = tiling_mode;
                iLastDelayedAction = 1;
                QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResizeTilingMode/*Slot_Delayed2DMode*/);
            }
            ///else  if (text.startsWith("Regular volumetric grid", Qt::CaseInsensitive))
            else  if (currentIndex == 2)
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                tiling_mode = TILE_VOL_GRID;
                savedTilingMode = tiling_mode;
                iLastDelayedAction = 1;
                QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResizeTilingMode/*Slot_Delayed3DMode*/);
            }

            ///lastTileExtraValue = "";
            lastTileModeIndex = currentIndex;

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;

            ///lblCenterRightOverviewDetail->setText(QString::number(this->recons_object_->GetNumTiles()));
            ///emit Sig_ProjModifed();
        }

        void ConstructionWgt::Slot_TextChanged(const QString& text)
        {
            if (text.isEmpty())
                return;
            
            bool bOk = false;
            float newValue = text.toFloat(&bOk);
            if (bOk && newValue >= 1.0 && newValue <= 1000.0)
            {
                ///ReconstructionCommandSet::ResetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId(), newValue);              
                savedMaxRamUsage = newValue;
                iLastDelayedAction = 2;

                QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResetTileMAXRamUsage);               
                //emit Sig_DelayedResetTileMAXRamUsage(newValue);
            }

///         lblCenterRightOverviewDetail->setText(QString::number(this->recons_object_->GetNumTiles()));
///         emit Sig_ProjModifed();
        }

        void ConstructionWgt::Slot_EditingFinished()
        {
//          std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;

            QString text = leTileExtra->text();//@attention如果是空的则显示之前的值并返回
            if (text.isEmpty())
                return;

            if (!lastTileExtraValue.isEmpty() && lastTileExtraValue == text)
                return;

            lastTileExtraValue = text;

            // 1107 mWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_TILE);;
            bool bOk = false;
            float newValue = text.toFloat(&bOk);
//          std::cout << __TIME__ << " |||VVVV  " << __LINE__ << std::endl;
            if (bOk && newValue >= 1.0 && newValue <= 1000.0)
            {
                iLastDelayedAction = 2;
                if (this->recons_object_->GetTilingDiscriptor()->GetParams().mode_ == tiling_mode_e::TILE_PALNAR_GRID
                    || this->recons_object_->GetTilingDiscriptor()->GetParams().mode_ == tiling_mode_e::TILE_VOL_GRID)
                {
                    savedTileSize = newValue;
//                  std::cout << __TIME__ << " |||VVVV  " << __LINE__ << " palnar/vol mode." << std::endl;
                    QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResetTileSize);
                }
                else if (this->recons_object_->GetTilingDiscriptor()->GetParams().mode_ == tiling_mode_e::TILE_ADAPTIVE)
                {
                    savedMaxRamUsage = newValue;
//                  std::cout << __TIME__ << " |||VVVV " << __LINE__ << " max ram mode." << std::endl;
                    QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResetTileMAXRamUsage);
                }
                else
                {
//                  std::cout << __TIME__ << " |||VVVV " << __LINE__ << " unknown mode." << std::endl;
                }
                
            }

//          std::cout << __TIME__ << " |||VVVV  " << __LINE__ << std::endl;

            //@attention @zhangyufeng此处逻辑没放在QTimer::singleShot(100, this, &ConstructionWgt::Slot_DelayedResetTileMAXRamUsage); 中不知是否有影响
            //mWindow->RenderReconstruction(this->recons_object_);
            /*1107mWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_TILE);;

            AI3D::VIEWER::Tile3DViewInterface interface_(this->recons_object_, mWindow->getOsgEngine());
            interface_.BuildTilesNode();*/
            
            
        }

        void ConstructionWgt::Slot_ROIEditingFinished()
        {
            std::cout << "======================ROI Edit ,waiting=================" << std::endl;
            QLineEdit* pLineEdit = dynamic_cast<QLineEdit*>(sender());
            if (!pLineEdit || pLineEdit->text().isEmpty())
                return;

            DisconnectSignalMap4ROILimits();

            QString strRawValue = pLineEdit->text();

            bool bOk = false;
            double dValue = strRawValue.toDouble(&bOk);
            if (!bOk)
            {
                pLineEdit->setText("");
            }
            else
            {
                QString strConvertedValue = QString::number(dValue, 'f', 6);
                if(strRawValue != strConvertedValue)
                    pLineEdit->setText(strConvertedValue);
            }

            ABBox3d box3ds;
            box3ds.min().x() = leROIXMin->text().toDouble();
            box3ds.min().y() = leROIYMin->text().toDouble();
            box3ds.min().z() = leROIZMin->text().toDouble();
            box3ds.max().x() = leROIXMax->text().toDouble();
            box3ds.max().y() = leROIYMax->text().toDouble();
            box3ds.max().z() = leROIZMax->text().toDouble();
            MakeBoundingBoxValid(box3ds);
            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                if (bLastMatrixExists)
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }

            
                
                bool bRunFinished = false;

                auto savefunc = [&, this]()
                {

                    int  ret = ReconstructionCommandSet::ResetBoundingBox(this->block_data_, this->recons_object_->GetId(), box3ds);
                    
                    if (ret == AI3D_SUCCESS)
                    {

                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->ResetROI();
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                    }
                    bRunFinished = true;
                    return ret;
                };
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion()) {
                        OpenLoadingPromptV4("正复位分块模式，请耐心等待");
                    }
                    else {
                        OpenLoadingPromptV4("Please be patient and wait.Reset Tiling");
                    }
                    QFuture<int> f1 = QtConcurrent::run(savefunc);

                    while (!bRunFinished)
                    {
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    ConnectSignalMap4ROIlLimits();

                    RefreshOverviewInfo();
                    SetProjectModified();
                    CloseLoadingPromptV4();
                }
                else
                {
                    savefunc();
                    ConnectSignalMap4ROIlLimits();

                    RefreshOverviewInfo();
                    SetProjectModified();
                }
                
                LOGI("===================ROI Edit end.==============================");
                std::cout << "======================ROI Edit end=================" << std::endl;
            
        }

        void ConstructionWgt::Slot_DelayedResetTileSize()
        {
            if (iLastDelayedAction != 2)
                return;

            bool bRunFinished = false;

            auto savefunc = [&, this]()
            {
                std::cout << __FUNCTION__ << " tiling =====" << __LINE__ << std::endl;
                clock_t t1, t2, t3;
                t1 = clock();
                int ret = ReconstructionCommandSet::ResetTileSize(this->block_data_, this->recons_object_->GetId(), savedTileSize);
                t2 = clock();
                t3 = t2 - t1;
                t3 *= 0.001;
                std::cout << "reset tile engine finished. " << t3 << std::endl;
                    
                bRunFinished = true;
                return ret;
            };

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                OpenLoadingPromptV4("块操作进行中，请耐心等待");
            }
            else
            {
                OpenLoadingPromptV4("Please be patient and wait. Blocking is underway");
            }

            QFuture<int> f1 = QtConcurrent::run(savefunc);

            while (!bRunFinished)
            {

                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            {
                int ret = f1.result();
                if (ret == AI3D_SUCCESS)
                {
                    LOGI("=====================Rendering,pls wait=============");
                    std::cout << "======================Rendering,pls wait==================" << std::endl;

                    clock_t t1, t2, t3;
                    t1 = clock();
                    
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                    mWindow->RenderReconstruction(this->recons_object_);
                    if (bLastMatrixExists)
                    {
                        mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                    }
                    else
                    {
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }


                    LOGI("=====================Render end=============");
                    std::cout << "======================Render end==================" << std::endl;
                    RefreshOverviewInfo();
                    
                    emit Sig_ProjModifed();

                    t2 = clock();
                    t3 = t2 - t1;
                    t3 *= 0.001;
                    std::cout << "reset tile render finished. " << t3 << std::endl;
                }

            }


            
            CloseLoadingPromptV4();
            iLastDelayedAction = -1;

        }
        void ConstructionWgt::Slot_DelayedResetTileMAXRamUsage()
        {
            /*
            savedMaxRamUsage = newValue;
            iLastDelayedAction = 2;
            */
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << iLastDelayedAction << std::endl;
            if (iLastDelayedAction != 2)
                return;
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            //OpenLoadingPromptV4("Resetting Tile Max Ram Usage now,pls wait for a while");
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                OpenLoadingPromptV4("块操作进行中，请耐心等待");
            }
            else
            {
                OpenLoadingPromptV4("Please be patient and wait. Blocking is underway");
            }
            bool bRunFinished = false;

            auto savefunc = [&, this]()
            {
                int ret = ReconstructionCommandSet::ResetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId(), savedMaxRamUsage);
            //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                if(0)
                {
                    if (ret == AI3D_SUCCESS)
                    {
                        LOGI("=====================Rendering,pls wait=============");
                        std::cout << "======================Rendering,pls wait==================" << std::endl;


                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->RenderReconstruction(this->recons_object_);
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }


                        LOGI("=====================Render end=============");
                        std::cout << "======================Render end==================" << std::endl;
                    }

                }
                bRunFinished = true;


                return ret;

            };

            QFuture<int> f1 = QtConcurrent::run(savefunc);

            while (!bRunFinished)
            {
                if (f1.isFinished())
                {
                    ///      std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    ///      break;
                }
                else
                {
                    ///    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            {
                int ret = f1.result();
                if (ret == AI3D_SUCCESS)
                {
                    LOGI("=====================Rendering,pls wait=============");
                    std::cout << "======================Rendering,pls wait==================" << std::endl;

                    //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                    mWindow->RenderReconstruction(this->recons_object_);
                    if (bLastMatrixExists)
                    {
                        mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                    }
                    else
                    {
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }


                    LOGI("=====================Render end=============");
                    std::cout << "======================Render end==================" << std::endl;
                    RefreshOverviewInfo();

                    emit Sig_ProjModifed();


                }
                else
                {
                    std::cout  << " " << __FUNCTION__ << " " << __LINE__ << " " <<  ret << std::endl;
                }
            //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            }
            
            /*RefreshOverviewInfo();

            emit Sig_ProjModifed();*/

            CloseLoadingPromptV4();
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << std::endl;
            iLastDelayedAction = -1;            
        }

        void ConstructionWgt::Slot_DelayedResizeTilingMode()
        {
            if (iLastDelayedAction != 1)
                return;

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;

            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                if (bLastMatrixExists)
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }

            ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;

            mWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_TILE);

            /*
            savedTilingMode = newMode;
            iLastDelayedAction = 1;
            */
            bool bRunFinished = false;
            ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                OpenLoadingPromptV4("块设置进行中，请耐心等待");
            }
            else {
                OpenLoadingPromptV4("Please be patient and wait.Blocking is underway");
            }
            ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;

            ///if (savedTilingMode == TILE_ADAPTIVE)
            {
            //  disconnect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged);
                disconnect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);
            }

            auto savefunc = [&, this]()
            {
                int ret = AI3D_FAILURE;
                if (savedTilingMode == TILE_NONE)
                {
                    ret = ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), savedTilingMode);
                }
                else if (savedTilingMode == TILE_ADAPTIVE)
                {
                    //disconnect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged);
                    //
                    
                    ret = ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), savedTilingMode);


                    //float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
                    //leTileExtra->setText(QString::number(extraTileValue, 'f', 1));

                    //connect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged, Qt::DirectConnection);
                }
                else if (savedTilingMode == TILE_PALNAR_GRID)
                {
                    /*uint32_t saved_id = this->recons_object_->GetId();
                    
                    AI3D::CORE::ReconstructionObject* rec0 = this->block_data_->GetReconstruction(saved_id);

                    std::cout << "===============0000000000000000000"<< std::endl;
                    std::cout << "===============0000000000000000001" << std::endl;
                    
                    for (auto& iterblock : this->block_data_->GetReconstructions())
                    {
                        std::cout << std::hex << std::showbase << iterblock.second << "== " << iterblock.second->GetId() << std::endl;
                    }
                    std::cout << std::hex << std::showbase << this->recons_object_ << "== " << this->recons_object_->GetId() << std::endl;
                    std::cout << std::hex << std::showbase << rec0 << " " << rec0->GetId() << "--- " << saved_id << std::endl;

                    std::cout << "===============0000000000000000002" << std::endl;
                    std::cout << "===============0000000000000000003" << std::endl;*/

                    ret = ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), savedTilingMode);
                    /*AI3D::CORE::ReconstructionObject* rec1 = this->block_data_->GetReconstruction(saved_id);

                    std::cout << "===============0000000000000000004" << std::endl;
                    std::cout << "===============0000000000000000005" << std::endl;
                    for (auto& iterblock : this->block_data_->GetReconstructions())
                    {
                        std::cout << std::hex << std::showbase << iterblock.second << "== " << iterblock.second->GetId() << std::endl;
                    }

                    std::cout << std::hex << std::showbase << this->recons_object_ << "++ " << this->recons_object_->GetId() << std::endl;
                    std::cout << std::hex << std::showbase << rec1 << " " << rec1->GetId()<<"--- "<< saved_id << std::endl;
                    std::cout << "===============0000000000000000006=========================" << std::endl;
                    std::cout << "===============0000000000000000007=========================" << std::endl;*/
                }
                else if (savedTilingMode == TILE_VOL_GRID)
                {
                    ret = ReconstructionCommandSet::ResetTilingMode(this->block_data_, this->recons_object_->GetId(), savedTilingMode);
                }

                ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                bRunFinished = true;
                return ret;
            };

            ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            QFuture<int> f1 = QtConcurrent::run(savefunc);
            ///std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            while (!bRunFinished)
            {
                if (f1.isFinished())
                {
                    ///      std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    ///      break;
                //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
                }
                else
                {
                    ///    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            auto ret = f1.result();
            if (ret != AI3D_SUCCESS)
            {
                LOGI("tiling failed");
            /// if (savedTilingMode == TILE_ADAPTIVE)
                {
                    connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);
                }
                return;
            }
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            // consider other tiling mode such as 2D or 3D.
            if(savedTilingMode == TILE_ADAPTIVE)
            {           
                float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    lblTileExtra->setText("可用目标内存（GB）:");
                }
                else {
                    lblTileExtra->setText("Target RAM usage（GB）:");
                }
                //connect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged, Qt::DirectConnection);
                ///connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);

                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);
            }
            else if (savedTilingMode == TILE_PALNAR_GRID)
            {
                float extraTileValue = this->recons_object_->GetTilingDiscriptorMutual()->GetParamsMutual().regular_params_.tilesize_;
                ///  ???
                //float extraTileValue = ReconstructionCommandSet::GetTileMAXRamUsage(this->block_data_, this->recons_object_->GetId());
                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("瓦片尺寸(米):");
                    else
                        lblTileExtra->setText("瓦片尺寸(单位):");
                }
                else {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("Tile size(meters):");
                    else
                        lblTileExtra->setText("Tile size(units):");
                }
                //connect(leTileExtra, &QLineEdit::textChanged, this, &ConstructionWgt::Slot_TextChanged, Qt::DirectConnection);

                ///connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);

                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);

            }
            else if (savedTilingMode == TILE_VOL_GRID)
            {
                float extraTileValue = this->recons_object_->GetTilingDiscriptorMutual()->GetParamsMutual().regular_params_.tilesize_;
                leTileExtra->setText(QString::number(extraTileValue, 'f', 1));
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("瓦片尺寸(米):");
                    else
                        lblTileExtra->setText("瓦片尺寸(单位):");
                }
                else
                {
                    if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                        lblTileExtra->setText("Tile size(meters):");
                    else
                        lblTileExtra->setText("Tile size(units):");
                }
            
///             ///connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);

                ///  ???
                lblTileExtra->setVisible(true);
                leTileExtra->setVisible(true);

            }
            else
            {
                lblTileExtra->setVisible(false);
                leTileExtra->setVisible(false);
            }

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            
            bool bLastMatrixExists = false;
            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
            mWindow->RenderReconstruction(this->recons_object_);
            if (bLastMatrixExists)
            {
                mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
            }
            else
            {
                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                UserMatrixData::setCurrentMatrixObject(this, cmt);
            }

            connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished, Qt::DirectConnection);

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;

            RefreshOverviewInfo();
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            emit Sig_ProjModifed();
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            CloseLoadingPromptV4();
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            iLastDelayedAction = -1;
        }

        void ConstructionWgt::Slot_Delayed2DMode()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            disconnect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    lblTileExtra->setText("瓦片尺寸(米):");
                else
                    lblTileExtra->setText("瓦片尺寸(单位):");
            }
            else
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    lblTileExtra->setText("Tile size(meters):");
                else
                    lblTileExtra->setText("Tile size(units):");
            }

            leTileExtra->setText("");
            ///lastTileExtraValue = "";
            lblTileExtra->setVisible(true);
            leTileExtra->setVisible(true);

            connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
            RefreshOverviewInfo();

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << __TIME__ << std::endl;
        }

        void ConstructionWgt::Slot_Delayed3DMode()
        {
            disconnect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    lblTileExtra->setText("瓦片尺寸(米):");
                else
                    lblTileExtra->setText("瓦片尺寸(单位):");

            }
            else
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    lblTileExtra->setText("Tile size(meters):");
                else
                    lblTileExtra->setText("Tile size(units):");
            }       

            leTileExtra->setText("");
            ///lastTileExtraValue = "";

            lblTileExtra->setVisible(true);
            leTileExtra->setVisible(true);

            connect(leTileExtra, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_EditingFinished);

            RefreshOverviewInfo();
        }

        void ConstructionWgt::SetButtonStates()
        {
            // it has no sooner come to the constructor of current class than the scene unit has definite scene unit.
            if (!ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
            {
                butROIImport->setEnabled(false);
            }
            else
            {
                butROIImport->setEnabled(true);
            }

            ///butROIEdit->setEnabled(false);
        }

        void ConstructionWgt::ShowOrHideDetails()
        {
            if (bShowDetails)
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    butShowOrHideDetails->setText("显示详情");
                }
                else
                {
                    butShowOrHideDetails->setText("Show Details");
                }
                butIcon4ShowOrHideDetails->setIcon(QPixmap(":/new/prefix1/skin/show_details.png"));

                lblROIXUnit->hide();
                lblROIXMin->hide();
                leROIXMin->hide();
                lblROIXMax->hide();
                leROIXMax->hide();

                lblROIYUnit->hide();
                lblROIYMin->hide();
                leROIYMin->hide();
                lblROIYMax->hide();
                leROIYMax->hide();

                lblROIZUnit->hide();
                lblROIZMin->hide();
                leROIZMin->hide();
                lblROIZMax->hide();
                leROIZMax->hide();
            }
            else
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    butShowOrHideDetails->setText("隐藏详情");
                }
                else
                {
                    butShowOrHideDetails->setText("Hide Details");
                }
                butIcon4ShowOrHideDetails->setIcon(QPixmap(":/new/prefix1/skin/hide_details.png"));

                lblROIXUnit->show();
                lblROIXMin->show();
                leROIXMin->show();
                lblROIXMax->show();
                leROIXMax->show();

                lblROIYUnit->show();
                lblROIYMin->show();
                leROIYMin->show();
                lblROIYMax->show();
                leROIYMax->show();

                lblROIZUnit->show();
                lblROIZMin->show();
                leROIZMin->show();
                lblROIZMax->show();
                leROIZMax->show();
            }
        }

        void ConstructionWgt::SetOverviewROIDimension(float x, float y, float z)
        {       
            QString strUnitType;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    strUnitType = "米";
                else
                    strUnitType = "单位";
            }
            else
            {
                if (ReconstructionCommandSet::GetSceneUnit(*this->recons_object_))
                    strUnitType = "meters";
                else
                    strUnitType = "units";
            }

            QString strOverviewROIDimension;

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                strOverviewROIDimension = QString("兴趣区尺寸 : %1 %2 x %3 %4 x %5 %6").arg(QString::number(x, 'f', 2)).arg(strUnitType)
                    .arg(QString::number(y, 'f', 2)).arg(strUnitType).arg(QString::number(z, 'f', 2)).arg(strUnitType);
            }
            else
            {
                strOverviewROIDimension = QString("ROI Dimension : %1 %2 x %3 %4 x %5 %6").arg(QString::number(x, 'f', 2)).arg(strUnitType)
                    .arg(QString::number(y, 'f', 2)).arg(strUnitType).arg(QString::number(z, 'f', 2)).arg(strUnitType);
            }
            
            lblOverviewROIDimension->setText(strOverviewROIDimension);
        }

        void ConstructionWgt::SetExpectedMaxRamUsage(float maxRam)
        {
            QString text = QString::number(maxRam, 'f', 1) + " GB";
            lblExpectedMaxRamUsage->setText(text);
            if (maxRam > 16.0)
            {
                ShowOverviewWarning(true);
                QString text("");
                if (BlockObject::isChineseVersion())
                {

                    text = "预估最大内存超过16G，建议减小块大小";

                }
                else
                {
                    text = "the expected maximum RAM usage exceeds 16 GB,It is advised to use smaller tiles.";
                }
                SetOverviewWarning(text);
            }
            else
            {
                ShowOverviewWarning(false);
            }
        }

        void ConstructionWgt::SetOverviewWarning(QString strWarning)
        {
            lblOverviewWarning->setText(strWarning);
        }

        void ConstructionWgt::ShowOverviewWarning(bool bShow)
        {
            if (bShow)
            {
                lblOverviewWarningIcon->setVisible(true);
                lblOverviewWarning->setVisible(true);
            }
            else
            {
                lblOverviewWarningIcon->setVisible(false);
                lblOverviewWarning->setVisible(false);
            }
        }

        void ConstructionWgt::DoFirstRenderReconstruction()
        {
            {
                Slot_ClickTab(1);
                return;
            }


            if (bRenderReconstructionOnce)
                return;

            LOGI("=====================Rendering,pls wait=============");
            std::cout << "======================Rendering,pls wait==================" << std::endl;
            bool bRunFinished = false;
            auto savefunc = [&, this]()
            {

                mWindow->RenderReconstruction(this->recons_object_);
                
                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                UserMatrixData::setCurrentMatrixObject(this, cmt);
                bRenderReconstructionOnce = true;

                bRunFinished = true;
                return;
            };
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                if (BlockObject::isChineseVersion())
                {
                    OpenLoadingPromptV4("渲染中，请耐心等待");
                }
                else
                {
                    OpenLoadingPromptV4("Please be patient and wait.rendering");
                }
                QFuture<void> f1 = QtConcurrent::run(savefunc);

                while (!bRunFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else
            {
                savefunc();
            }

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }
            LOGI("=====================Render end=============");
            std::cout << "======================Render end==================" << std::endl; 
        }

        void ConstructionWgt::RefreshOverviewInfo()
        {
            static int stateId = 1000;
            float expectedMaxRam = ReconstructionCommandSet::GetExpectedMaxRamUsageForAJob(this->recons_object_);
            std::cout << "get max ram:" << expectedMaxRam << " stateId" << stateId++ << std::endl;
            SetExpectedMaxRamUsage(expectedMaxRam);

            const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

            float xval = (float)(box3d.max().x() - box3d.min().x());
            float yval = (float)(box3d.max().y() - box3d.min().y());
            float zval = (float)(box3d.max().z() - box3d.min().z());

            SetOverviewROIDimension(xval, yval, zval);

            lblCenterRightOverviewDetail->setText(QString::number(this->recons_object_->GetNumTiles(this->recons_object_->GetProcessingSettings().bdiscard_emptytiles_)));
        }

        void ConstructionWgt::RefreshEditableState()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (this->recons_object_ && this->recons_object_->HasProductions())
            {
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                // no action needed.
            }
            else
            {
                // make the relevant attributes for current widget editable again.
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                SetRightSideEditable(true);
            }
        }

        void ConstructionWgt::TestGeometryContraints()
        {

        }

        void ConstructionWgt::InsertGeometryContraintsItem(QString itemName, QString itemType, QString replacementOption)
        {
            if (!twGeometryContraints)
                return;
            std::cout << twGeometryContraints->rowCount() << std::endl;

            twGeometryContraints->insertRow(twGeometryContraints->rowCount());

            for (int i = 0; i < twGeometryContraints->columnCount(); i++)
            {
                QTableWidgetItem* pTableWidgetItem = 0;
                
                switch (i)
                {
                case 0:
                    pTableWidgetItem = new QTableWidgetItem();
                    pTableWidgetItem->setText(itemName);
                    break;
                case 1:
                    pTableWidgetItem = new QTableWidgetItem();
                    pTableWidgetItem->setText(itemType);
                    break;
                case 2:
                {
                    QComboBox* pComboBox = new QComboBox(ui->tab_4);

                    for (int j = 0; j < slTextureReplacementOption.size(); j++)
                        pComboBox->addItem(slTextureReplacementOption.at(j));
                    // replace item with combobox control later.
                    //pTableWidgetItem->setText(replacementOption);

                    twGeometryContraints->setCellWidget(twGeometryContraints->rowCount() - 1, i, pComboBox);
                }
                    break;
                }
            
                if (pTableWidgetItem != nullptr)
                {
                    pTableWidgetItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                    twGeometryContraints->setItem(twGeometryContraints->rowCount() - 1, i, pTableWidgetItem);
                }
            }
        }

        void ConstructionWgt::Slot_ToggleShowOrHideDetails()
        {
            bShowDetails = !bShowDetails;
            ShowOrHideDetails();
        }

        void ConstructionWgt::Slot_TiePoints()
        {
            std::cout << "======================ROI Tiepoints  ,waiting=================" << std::endl;
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                if (bLastMatrixExists)
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }

            {
            
                bool bRunFinished = false;


                auto savefunc = [&, this]()
                {

                    auto ret = ReconstructionCommandSet::ResetBoundingboxScopeMode(this->block_data_, this->recons_object_->GetId(), bb_scope_e::BB_SCOPE_TIEPOINTS);
                    if (ret == AI3D_SUCCESS)
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->ResetROI();
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                    }
                    bRunFinished = true;

                    return ret;
                };
                int ret = AI3D_FAILURE;
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        OpenLoadingPromptV4("复位分块模式，请耐心等待");
                    }
                    else
                    {
                        OpenLoadingPromptV4("Please be patient and wait.Reset Tiling");
                    }
                    QFuture<int> f1 = QtConcurrent::run(savefunc);

                    while (!bRunFinished)
                    {
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                     ret = f1.result();
                }
                else
                {
                    ret = savefunc();
                }
                if (ret == AI3D_SUCCESS)
                {
                    const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

                    DisconnectSignalMap4ROILimits();

                    leROIXMin->setText(QString::number(box3d.min().x()));
                    leROIYMin->setText(QString::number(box3d.min().y()));
                    leROIZMin->setText(QString::number(box3d.min().z()));

                    leROIXMax->setText(QString::number(box3d.max().x()));
                    leROIYMax->setText(QString::number(box3d.max().y()));
                    leROIZMax->setText(QString::number(box3d.max().z()));

                    ConnectSignalMap4ROIlLimits();

                    RefreshOverviewInfo();
                    
                    LOGI("========================ROI tiepoints end==========================");
                    std::cout << "======================ROI tiepoints end=====================" << std::endl;
                }
                else
                {

                    LOGI("========================ROI tiepoints failed and end==========================");
                    std::cout << "======================ROI tiepoints failed and end====================" << std::endl;
                }

            }
            
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }
            return;
        }

        void ConstructionWgt::Slot_Photos()
        {
            LOGI(  "======================ROI photos waiting=================" );
            std::cout << "======================ROI photos waiting=====================" << std::endl;
            {
                
                bool bRunFinished = false;
                auto savefunc = [&, this]()
                {
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        if (bLastMatrixExists)
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                    }
                    int ret = ReconstructionCommandSet::ResetBoundingboxScopeMode(this->block_data_, this->recons_object_->GetId(), bb_scope_e::BB_SCOPE_VIEWS);
                    if (ret == AI3D_SUCCESS)
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->ResetROI();
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                    }
                    bRunFinished = true;

                    return ret;
                };
                
                int ret = AI3D_FAILURE;
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        OpenLoadingPromptV4("正在复位分块模式，请耐心等待");
                    }
                    else
                    {
                        OpenLoadingPromptV4("Please be patient and wait.Reset Tiling");
                    }
                    QFuture<int> f1 = QtConcurrent::run(savefunc);

                    while (!bRunFinished)
                    {
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    ret = f1.result();
                }
                else
                {
                    ret = savefunc();
                }

                if (ret == AI3D_SUCCESS)
                {
                    const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

                    DisconnectSignalMap4ROILimits();

                    leROIXMin->setText(QString::number(box3d.min().x()));
                    leROIYMin->setText(QString::number(box3d.min().y()));
                    leROIZMin->setText(QString::number(box3d.min().z()));

                    leROIXMax->setText(QString::number(box3d.max().x()));
                    leROIYMax->setText(QString::number(box3d.max().y()));
                    leROIZMax->setText(QString::number(box3d.max().z()));

                    ConnectSignalMap4ROIlLimits();

                    RefreshOverviewInfo();
                    LOGI("======================ROI photos  end=================");
                    std::cout << "======================ROI photos  end===================" << std::endl;
                }
                else
                {
                    LOGI("======================ROI photos failed and end=================");
                    std::cout << "======================ROI photos failed and end===================" << std::endl;
                }
            }

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }
            return;
            
        }

        void ConstructionWgt::Slot_Frustum()
        {
            LOGI("======================ROI frustum waiting=================" );
            std::cout << "======================ROI frustum waiting===================" << std::endl;
            
                
                bool bRunFinished = false;


                auto savefunc = [&, this]()
                {
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        if (bLastMatrixExists)
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                    }
                    int ret = ReconstructionCommandSet::ResetBoundingboxScopeMode(this->block_data_, this->recons_object_->GetId(), bb_scope_e::BB_SCOPE_VIEWFRUSTUM);
                    if (ret == AI3D_SUCCESS)
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->ResetROI();
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }
                        
                    }
                    
                    bRunFinished = true;

                    return ret;
                };
                int ret = AI3D_FAILURE;
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion()) {
                        OpenLoadingPromptV4("正在复位分块模式，请耐心等待");
                    }
                    else {
                        OpenLoadingPromptV4("Please be patient and wait.Reset Tiling");
                    }
                    QFuture<int> f1 = QtConcurrent::run(savefunc);

                    while (!bRunFinished)
                    {
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                     ret = f1.result();
                }
                else
                {
                    ret = savefunc();
                }
                

                if (ret == AI3D_SUCCESS)
                {
                    const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

                    DisconnectSignalMap4ROILimits();

                    leROIXMin->setText(QString::number(box3d.min().x()));
                    leROIYMin->setText(QString::number(box3d.min().y()));
                    leROIZMin->setText(QString::number(box3d.min().z()));

                    leROIXMax->setText(QString::number(box3d.max().x()));
                    leROIYMax->setText(QString::number(box3d.max().y()));
                    leROIZMax->setText(QString::number(box3d.max().z()));

                    ConnectSignalMap4ROIlLimits();

                    RefreshOverviewInfo();
                    LOGI("======================ROI frustum end=================");
                    std::cout << "======================ROI frustum end===================" << std::endl;
                }
                else
                {

                    LOGI("======================ROI frustum failed end=================");              
                    std::cout << "======================ROI frustum failed end===================" << std::endl;

                }
        
        
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }
            return;
        }

        ConstructionWgt* pConstructionWgt = nullptr;
        void Refresh3DViewOfConstructionWgt()
        {
            if (pConstructionWgt)
            {
                ///pConstructionWgt->getMWindow()->RenderReconstruction(pConstructionWgt->getReconstructionObject());
            }
        }

        void ConstructionWgt::Slot_SubmitProduction()
        {
            /// the following code is just for test purpose,remember to uncomment or change it after testing.
            if (1)
            {
                production_option_s options;
                options.destination_ = File::GetParentDir(block_data_->GetTaskInfo().projectfile_);
                ReconstructionCommandSet::InitProductionOptions(block_data_,recons_object_, options);
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                pConstructionWgt = this;

                int result;

                if (BlockObject::isChineseVersion())
                {
                    result = OpenParamSettings4Production(options, "生产定义", 900, 570, block_data_, recons_object_, this);
                }
                else
                {
                    result = OpenParamSettings4Production(options, "Production Definition", 900, 570, block_data_, recons_object_, this);
                }

                if (result == AI3D_SUCCESS)
                {
                    std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " result:" << result << std::endl;
                    /// note:the original logic below has moved into Slot_DoneParamSettings4Production triggered by a signal.
                    return;
#if 0
                ///  not to run the following code logic until having closed the parameter settings dialog above.
                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

                    // note:!!! change the following code into a slot triggered only by a signal.

                    QPushButton* pSenderButton = dynamic_cast<QPushButton*>(sender());
                    if (!pSenderButton)
                        return;

                    cbbTileCategory->setEnabled(false);
                    leTileExtra->setEnabled(false);

                    ///butROIEdit->setEnabled(false);
                    butROIImport->setEnabled(false);
                    butROIDefault->setEnabled(false);



                    emit Sig_NewProductionStarted(block_data_, recons_object_->GetId(), this->recons_item);

                    BlockObject::Task_Info& task = block_data_->GetTaskInfoMutual();

                    std::string jobfile_path = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + NORMALLEVEL;
                    std::string projectfile = task.projectfile_;
                    std::string hostName = QHostInfo::localHostName().toStdString();

                    //  production_option_s options;

                    production_t production_id;

                    int ret = ReconstructionCommandSet::SubmitProduction(hostName, jobfile_path, projectfile, block_data_, recons_object_->GetId(), options, production_id);

                    if (ret != AI3D_SUCCESS)
                    {
                        return;
                    }
                    
                    SetRightSideEditable(false);
                    butMoreSettings->setEnabled(false);
                    //emit Sig_NewProduction(block_data_, recons_object_->GetId(), production->GetId(),this->recons_item);
                    emit Sig_NewProduction(block_data_, recons_object_->GetId(), production_id, this->recons_item);
                    
                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif
                }
                else
                {
                    /// note:the original logic above has moved into Slot_DoneParamSettings4Production triggered by a signal.
                    return;
                }
            }
        }

        void ConstructionWgt::Slot_Refresh_Timeout()
        {
#if 0
            static bool bHasLoadedOSGBFile = false;

            if(!bHasLoadedOSGBFile)
            {
                std::cout  << " " << __FUNCTION__ << " " << __LINE__ << " " << " will load osgb..." << std::endl;
                //Slot_LoadOSGBFile();
        ///     xmain();
                bHasLoadedOSGBFile = true;
                return;
            }
            else
            {
                std::cout  << " " << __FUNCTION__ << " " << __LINE__ << " " << " has already loaded osgb..." << std::endl;
                return;
            }
#endif
//          std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (!isVisible() || !bInsideOverview)
                return;

//          std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            for (int i = twProductionList->rowCount() - 1; i >= 0; i--)
            {
                twProductionList->removeRow(i);
            }

            if (this->recons_object_)
            {
                for (auto& t : this->recons_object_->GetProductionsMutual())
                {
                    //t.second->GetOptions().format_
                    int percent_ = 0;
                    job_status_e status_;

                    if (t.second->IsCompleted())
                    {
                        status_ = jobsta_e::STATUS_COMPLETE;
                        percent_ = 100;
                    }
                    else
                        status_ = ProductionWgt::CalcStatusAndPercent(this->block_data_, this->recons_object_, t.second, percent_);
                    //std::cout << t.first << " " << t.second->GetName() << " " << t.second->GetFormatString() << " " << status_ << " " << percent_ << std::endl;

                    twProductionList->insertRow(twProductionList->rowCount());
                    int lastRow = twProductionList->rowCount() - 1;

                    QTableWidgetItem* pItemZero = new QTableWidgetItem;
                    QTableWidgetItem* pItemOne = new QTableWidgetItem;
                    QTableWidgetItem* pItemTwo = new QTableWidgetItem;
                    QTableWidgetItem* pItemFour = new QTableWidgetItem;

                    pItemZero->setFlags(pItemZero->flags() & ~Qt::ItemIsEditable);
                    pItemOne->setFlags(pItemOne->flags() & ~Qt::ItemIsEditable);
                    pItemTwo->setFlags(pItemTwo->flags() & ~Qt::ItemIsEditable);
                    pItemFour->setFlags(pItemFour->flags() & ~Qt::ItemIsEditable);

                    pItemZero->setText(str2qstr(const_cast<std::string&>(t.second->GetName())));
                    pItemOne->setText(str2qstr(t.second->GetFormatString()));

                    //获取第一个任务的时间；
                    auto tilesinproduct = t.second->GetTiles();
                    std::string timestring = "";
                    if (!tilesinproduct.empty())
                    {
                        std::string tile_name = tilesinproduct.begin()->first;
                        block_t bid = this->block_data_->GetId();
                        auto rid = this->recons_object_->GetId();
                        auto pid = t.second->GetId();
                        std::string rptstring = "B" + std::to_string(bid) + "R" + std::to_string(rid) + "P" + std::to_string(pid);
                        std::string rptjobstring = rptstring + tile_name;
                        auto jobstrvec = this->block_data_->GetTaskInfo().reconstructionjobs_;
                        for (auto iterjob : jobstrvec)
                        {
                            if (strstr(iterjob.first.c_str(), rptstring.c_str()) != NULL)
                            {
                                std::string jobstr = iterjob.second;
                                auto strsvec = AI3D::CORE::String::StringSplit(jobstr,"_");
                                if (strsvec.size() > 3)
                                {
                                    timestring = strsvec[1];
                                    break;
                                }
                            }
                        }

                    }
                    if (timestring != "")
                    {
                        auto timestr = QString::fromStdString(timestring);
                        QString qSubmitTime = QDateTime::fromString(timestr, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
                        pItemFour->setText(qSubmitTime);
                    }
                    else
                    {
                        pItemFour->setText("--");
                    }
                    pItemZero->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    pItemOne->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    pItemTwo->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    pItemFour->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

                    twProductionList->setItem(lastRow, 0, pItemZero);
                    twProductionList->setItem(lastRow, 1, pItemOne);
                    twProductionList->setItem(lastRow, 2, pItemTwo);
                    twProductionList->setItem(lastRow, 4, pItemFour);

                    QWidget* pProgBarContainer = new QWidget(ui->widget);
                    pProgBarContainer->setStyleSheet("padding:0px;margin:0px;border-bottom:1px solid rgb(60,60,60);");
                    pProgBarContainer->setFixedHeight(50);

                    QHBoxLayout* hlProgBar = new QHBoxLayout();
                    hlProgBar->setContentsMargins(0, 0, 0, 0);

                    QProgressBar* pProgBar = new QProgressBar(ui->widget);
                    pProgBar->setAttribute(Qt::WA_StyledBackground);
                    pProgBar->setMinimum(0);
                    pProgBar->setMaximum(100);
                    //pProgBar->setFixedHeight(24);
                    pProgBar->setFixedWidth(140);
                    pProgBar->setFixedHeight(4);

                    QLabel* pLblProg = new QLabel(ui->widget);
                    pLblProg->setAlignment(Qt::AlignLeft);

                    //pLblProg->setText("color:rgb(185,185,185);padding:0px;margin:0px;font:12 \"Arial\";");
                    //hlProgBar->addSpacing(0);
                    //hlProgBar->addSpacing(40);
                    hlProgBar->setSpacing(12);
                    //hlProgBar->addStretch(1);
                    hlProgBar->addWidget(pProgBar);
                    hlProgBar->addWidget(pLblProg);
                    hlProgBar->addStretch(1);

                    pProgBarContainer->setLayout(hlProgBar);

                    if (status_ == jobsta_e::STATUS_PENDDING)
                    {
                                // pending

                        pProgBar->setValue(0);
                        pLblProg->setText("0%");
                        pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                        );
                        pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            pItemTwo->setText(QString("等待中"));
                        }
                        else
                        {
                            pItemTwo->setText(QString("pending"));
                        }
                    }
                    else if (status_ == jobsta_e::STATUS_RUNNING)
                    {
                                // running  
                        pProgBar->setValue(percent_);

                        pProgBar->setValue(percent_);
                        pLblProg->setText(QString("%1%").arg(percent_));

                        pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                            "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                        pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            pItemTwo->setText(QString("运行中"));
                        }
                        else
                        {
                            pItemTwo->setText(QString("running"));
                        }
                    }
                    else if (status_ == jobsta_e::STATUS_COMPLETE)
                    {
                        // completed

                        pProgBar->setValue(0);
                        pLblProg->setText("100%");
                        pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}"
                        );
                        pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            pItemTwo->setText(QString("完成"));
                        }
                        else
                        {
                            pItemTwo->setText(QString("completed"));
                        }
                    }
                    else
                    {
                        // failed
                        if (percent_ < 1)
                            percent_ = 1;
                        else if (percent_ == 100)
                            percent_ = 99;
                        pProgBar->setValue(percent_);

                        pLblProg->setText(QString("%1%").arg(percent_));
                        pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                            "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                        pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            pItemTwo->setText(QString("失败"));
                        }
                        else
                        {
                            pItemTwo->setText(QString("failed"));
                        }
                    }

                    pProgBar->setTextVisible(false);
                    twProductionList->setCellWidget(lastRow, 3, pProgBarContainer);

                    twProductionList->resizeRowToContents(lastRow);
                }
            }
        }

        void ConstructionWgt::Slot_DoneParamSettings4Production(bool bResult)
        {
            if (bResult)
            {
                LOGI("=================Submit Production wait.===================");
                std::cout << "======================Submit Production wait.==================" << std::endl;
                production_option_s options;
                options = ParamSettings4Production::GetSavedOptions();



                CloseParamSettings4Production();

                cbbTileCategory->setEnabled(false);
                leTileExtra->setEnabled(false);

                ///butROIEdit->setEnabled(false);
                butROIImport->setEnabled(false);
                butROIDefault->setEnabled(false);

                emit Sig_NewProductionStarted(block_data_, recons_object_->GetId(), this->recons_item);

                BlockObject::Task_Info& task = block_data_->GetTaskInfoMutual();

                std::string jobfile_path = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + NORMALLEVEL;
                std::string projectfile = task.projectfile_;
                std::string hostName = QHostInfo::localHostName().toStdString();
                bool ret1 = JobMonitor::CreateDirs();
                if (!ret1)
                    return;
                //  production_option_s options;

                block_t production_id;

                
                bool bRunFinished = false;
                
                
                auto savefunc = [&, this]()
                {

                    int ret = ReconstructionCommandSet::SubmitProduction(hostName, jobfile_path, projectfile, block_data_, recons_object_->GetId(), options, production_id);
                    bRunFinished = true;

                    return ret;
                };
                int ret = AI3D_FAILURE;
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    if (BlockObject::isChineseVersion())
                    {
                        OpenLoadingPromptV4("提交生产中，请耐心等待");
                    }
                    else
                    {
                        OpenLoadingPromptV4("Please be patient and wait. New production");
                    }

                    QFuture<int> f1 = QtConcurrent::run(savefunc);

                    while (!bRunFinished)
                    {
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                     ret = f1.result();
                     
                }
                else
                {
                    ret = savefunc();
                }
                if (ret == AI3D_SUCCESS)
                {
                    SetRightSideEditable(false);
                    
                    emit Sig_NewProduction(block_data_, recons_object_->GetId(), production_id, this->recons_item);
                    LOGI("=================Submit Production finished.===================");
                    std::cout << "======================Submit Production finished.=====================" << std::endl;
                }
                else
                {
                    LOGI( "======================Submit Production failed end=================");
                    
                    std::cout << "======================Submit Production failed end=====================" << std::endl;
                }

                
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    CloseLoadingPromptV4();
                }
                
            }
            else
            {
                mWindow->RenderReconstruction(this->recons_object_);
                CloseParamSettings4Production();
                std::cout << "close psp without submitting." << std::endl;
            }

            return;
        }

        void ConstructionWgt::Slot_ROIEdit()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            
            {
                ///bool bLastMatrixExists = false;
                ///osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                ///if (bLastMatrixExists)
                {
                    // force to set current matrix without considering whether it has ever saved.
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                    savedMatrix = cmt;
                }
            }
            
            mWindow->ROIEdit();
            
            SetRightSideEditable(false);
            butGeometryContraintsImport->setEnabled(false);

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 1)
            {
///             lblMoreSettings->setEnabled(false);
            }
            else
            {
///             butMoreSettings->setEnabled(false);
            }
        
            bROIEditing = true;

            ui->btn_newContruction->setEnabled(false);
            butNewProduction2->setEnabled(false);
            butNewProduction->setEnabled(false);
            butNewProduction2->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            butNewProduction->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
        }

        void  ConstructionWgt::Slot_UpdateROIBy3DViewEdit(ReconstructionObject* object, bool shouldmodified)
        {
            std::cout << this->recons_object_->GetId()<<" "<< this->recons_object_->GetTilesCustomMutual().size() << std::endl;
            this->recons_object_ = object;
            if (this->block_data_->GetReconstructionsMutual().count(this->recons_object_->GetId()))
            {
                this->block_data_->GetReconstructionsMutual().at(this->recons_object_->GetId()) = object;
                AI3D::CORE::ReconstructionCommandSet::UpdateBlockInfo(this->block_data_, this->recons_object_->GetId());
                std::cout << this->block_data_->GetReconstruction(this->recons_object_->GetId())->GetTilesCustom().size() << std::endl;
            }
            std::cout << this->recons_object_->GetId() << " " << this->recons_object_->GetTilesCustomMutual().size() << std::endl;
            if (!shouldmodified)

            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                mWindow->ResetROI();
                if (bLastMatrixExists)
                {
                    mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                }
                else
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
                ConnectSignalMap4ROIlLimits();

                RefreshOverviewInfo();
                
                /*AI3D::VIEWER::Tile3DViewInterface interface_(this->recons_object_, mWindow->getOsgEngine());
                interface_.BuildScene();*/
            }
            else
            {
                SetProjectModified();
            }
            SetRightSideEditable(true);
            butGeometryContraintsImport->setEnabled(true);
            bool valid = IsBoundingBoxValid(this->recons_object_->GetBoundingBoxCustom());
            ui->btn_newContruction->setEnabled(valid?true:false);
            
            butNewProduction2->setEnabled(valid ? true : false);
            butNewProduction2->setStyleSheet(QString::fromUtf8(
                "QPushButton { background-color:#0072BE;color:white;width:147px;height:32px;border-radius:2px;border:0px solid;font:14px \"Arial\";"
                "}"
                "QPushButton:pressed {"
                "background-color:#3F455C;"
                "}"
            ));
            butNewProduction->setEnabled(true);
            butNewProduction->setStyleSheet("background-color:#0072BE;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            //@attention 此处还要加submitproduction的状态；
            //emit signal_projchanged(object);
            if (!valid)
            {
                butNewProduction2->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
                butNewProduction->setStyleSheet("background-color:gray;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            }
        }

        void ConstructionWgt::Slot_UpdateROI(ReconstructionObject* object)
        {
            float expectedMaxRam = ReconstructionCommandSet::GetExpectedMaxRamUsageForAJob(object);
            SetExpectedMaxRamUsage(expectedMaxRam);

            const ABBox3d& box3d = object->GetBoundingBoxCustom();

            float xval = (float)(box3d.max().x() - box3d.min().x());
            float yval = (float)(box3d.max().y() - box3d.min().y());
            float zval = (float)(box3d.max().z() - box3d.min().z());

            SetOverviewROIDimension(xval, yval, zval);
            leROIXMin->setText(QString::number(box3d.min().x()));
            leROIYMin->setText(QString::number(box3d.min().y()));
            leROIZMin->setText(QString::number(box3d.min().z()));

            leROIXMax->setText(QString::number(box3d.max().x()));
            leROIYMax->setText(QString::number(box3d.max().y()));
            leROIZMax->setText(QString::number(box3d.max().z()));
            lblCenterRightOverviewDetail->setText(QString::number(object->GetNumTiles(object->GetProcessingSettings().bdiscard_emptytiles_)));
            SetProjectModified();
        }

        void ConstructionWgt::SetRightSideEditable(bool status)
        {
            //如果没有水域文件是可以设置为true的
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << status << std::endl;
            butROIImport->setEnabled(status);
            butROIDefault->setEnabled(status);
            butROIEdit->setEnabled(status);

            butTiePoints->setEnabled(status);
            butPhotos->setEnabled(status);
            butFrustum->setEnabled(status);

            leROIXMin->setEnabled(status);
            leROIXMax->setEnabled(status);
            leROIYMin->setEnabled(status);
            leROIYMax->setEnabled(status);
            leROIZMin->setEnabled(status);
            leROIZMax->setEnabled(status);
            cbbTileCategory->setEnabled(status);
            leTileExtra->setEnabled(status);
        }

        void ConstructionWgt::Slot_ROIImport()
        {

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            QString oldStr = ".";
            ///QFileDialog fd(nullptr, tr("Open KML/SHP file"), oldStr, tr("kml file(*.kml);;shp file(*.shp)"));
            QString strTitle;
            QString strInformation;

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                strTitle = "打开 KML/SHP 文件";
                strInformation = "kml/shp 文件(*.kml *.shp)";
            }
            else {
                strTitle = "Open KML/SHP file";
                strInformation = "kml/shp file(*.kml *.shp)";
            }

///         QFileDialog fd(nullptr, tr("Open KML/SHP file"), oldStr, tr("kml/shp file(*.kml *.shp)"));
            QFileDialog fd(nullptr, strTitle, oldStr, strInformation);

            fd.setAcceptMode(QFileDialog::AcceptOpen);
            fd.setFileMode(QFileDialog::ExistingFile);
            fd.setViewMode(QFileDialog::Detail);

            if (QDialog::Accepted != fd.exec())
            {
                std::cout << "got no kml file." << std::endl;
                return;
            }
            std::cout << "======================ROI Import waiting===================" << std::endl;
            LOGI("======================ROI Import waiting=================");
            QString file = fd.selectedFiles().first();
            std::cout << "got kml:" << qstr2str(file) << std::endl;
            std::string msg = "";



            bool bRunFinished = false;

            //@attention @zhangyufeng 此处msg会传回吗，不会需要改进一下
            auto savefunc = [&, this]()
            {
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                    if (bLastMatrixExists)
                    {
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }
                int ret = ReconstructionCommandSet::ResetBoundaryByFile(this->block_data_, this->recons_object_->GetId(), qstr2str(file), msg);

                if (ret == AI3D_SUCCESS)
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                    mWindow->ResetROI();
                    if (bLastMatrixExists)
                    {
                        mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                    }
                    else
                    {
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }
                bRunFinished = true;

                return ret;
            };
            int ret = AI3D_FAILURE;
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    OpenLoadingPromptV4("正在重置分块模式，请耐心等待");
                }
                else {
                    OpenLoadingPromptV4("Please be patient and wait.Reset Tiling");
                }
                QFuture<int> f1 = QtConcurrent::run(savefunc);

                while (!bRunFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                ret = f1.result();

            }
            else
            {
                ret = savefunc();
            }

            if (ret == AI3D_SUCCESS)
            {
                const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();

                DisconnectSignalMap4ROILimits();

                leROIXMin->setText(QString::number(box3d.min().x()));
                leROIYMin->setText(QString::number(box3d.min().y()));
                leROIZMin->setText(QString::number(box3d.min().z()));

                leROIXMax->setText(QString::number(box3d.max().x()));
                leROIYMax->setText(QString::number(box3d.max().y()));
                leROIZMax->setText(QString::number(box3d.max().z()));

                ConnectSignalMap4ROIlLimits();

                RefreshOverviewInfo();
                SetProjectModified();
                LOGI("====================ROI import end=================");
                std::cout << "======================ROI import end===================" << std::endl;
            }
            else
            {
                if (!msg.empty())
                {
                    ShowOverviewWarning(true);
                    SetOverviewWarning(str2qstr(msg));
                }
                LOGI("=========================ROI import failed and end=================");
                std::cout << "======================ROI import failed and end=====================" << std::endl;

            }

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {

                CloseLoadingPromptV4();
            }

            return;

        }

        void ConstructionWgt::DisconnectSignalMap4ROILimits()
        {
            disconnect(leROIXMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
            disconnect(leROIXMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
            disconnect(leROIYMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
            disconnect(leROIYMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
            disconnect(leROIZMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
            disconnect(leROIZMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished);
        }

        void ConstructionWgt::ConnectSignalMap4ROIlLimits()
        {
            connect(leROIXMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
            connect(leROIXMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
            connect(leROIYMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
            connect(leROIYMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
            connect(leROIZMin, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
            connect(leROIZMax, &QLineEdit::editingFinished, this, &ConstructionWgt::Slot_ROIEditingFinished, Qt::DirectConnection);
        }

        void ConstructionWgt::SetProjectModified()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            this->block_data_->GetTaskInfoMutual().isSaved = false;
            this->block_data_->setModifily(true);
            
            emit Sig_IsModifiedProj();
        }

        void ConstructionWgt::Slot_ROIDefault()
        {
            LOGI("======================ROI default waiting=================");
            std::cout << "======================ROI default waiting===================" << std::endl;
            SetProjectModified();
            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                if (bLastMatrixExists)
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }



            bool bRunFinished = false;


            auto savefunc = [&, this]()
            {

                int  ret = ReconstructionCommandSet::ResetBoundingboxToDefault(this->block_data_, this->recons_object_->GetId());

                bRunFinished = true;
                return ret;
            };
            int ret = false;
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                if (AI3D::CORE::BlockObject::isChineseVersion()) {
                    OpenLoadingPromptV4("正在复位分块模式");
                }
                else {
                    OpenLoadingPromptV4("Reset Tiling");
                }

                QFuture<int> f1 = QtConcurrent::run(savefunc);

                while (!bRunFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                ret = f1.result();
                
            }
            else
            {
                ret = savefunc();
            }

            if (ret == AI3D_SUCCESS)
            {
                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                mWindow->ResetROI();
                if (bLastMatrixExists)
                {
                    mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                }
                else
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }

                const ABBox3d& box3d = this->recons_object_->GetBoundingBoxCustom();
                DisconnectSignalMap4ROILimits();

                leROIXMin->setText(QString::number(box3d.min().x()));
                leROIYMin->setText(QString::number(box3d.min().y()));
                leROIZMin->setText(QString::number(box3d.min().z()));

                leROIXMax->setText(QString::number(box3d.max().x()));
                leROIYMax->setText(QString::number(box3d.max().y()));
                leROIZMax->setText(QString::number(box3d.max().z()));

                ConnectSignalMap4ROIlLimits();

                SetOverviewWarning("");
                ShowOverviewWarning(false);

                leROIXMin->setEnabled(true);
                leROIXMax->setEnabled(true);
                leROIYMin->setEnabled(true);
                leROIYMax->setEnabled(true);
                leROIZMin->setEnabled(true);
                leROIZMax->setEnabled(true);
                leTileExtra->setEnabled(true);

                RefreshOverviewInfo();
                LOGI("======================ROI default end=================");
                std::cout << "======================ROI default end====================" << std::endl;
            }
            else
            {
                LOGI("======================ROI default failed and end=================");
                std::cout << "======================ROI default failed and end====================" << std::endl;
            }
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }
            
            return;
        }

        void ConstructionWgt::Slot_ClickTab(int idx)
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << idx << std::endl;
            QString tabtext = ui->tabWidget->tabText(idx);

            ///int currentIndex = ui->tabWidget->currentIndex();
            ///std::cout << "now click at " << currentIndex << std::endl;

            //bool bLastMatrixExists = false;
            //osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

            ///if (tabtext.toStdString() == "Overview")
            if (idx == 0)
            {
                //              std::cout << "inside " << tabtext.toStdString() << "(overview) tab of productionWgt." << std::endl;
                bInsideOverview = true;
                if (mWindow->hasSceneData())
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }
            ///else if (tabtext.toStdString() == "Spatial Framework")
            else if (idx == 1)
            {
                bInsideOverview = false;
                if (!mWindow->hasSceneData())
                {
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

                    LOGI("=====================Rendering,pls wait=============");
                    std::cout << "======================Rendering,pls wait==================" << std::endl;
                    bool bRunFinished = false;
                    auto savefunc = [&, this]()
                    {
                        mWindow->RenderReconstruction(this->recons_object_);
                        bRunFinished = true;
                        return;
                    };

                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        if (BlockObject::isChineseVersion())
                        {
                            OpenLoadingPromptV4("渲染中，请耐心等待");
                        }
                        else
                        {
                            OpenLoadingPromptV4("Please be patient and wait.rendering");
                        }
                        QFuture<void> f1 = QtConcurrent::run(savefunc);

                        while (!bRunFinished)
                        {
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                    }
                    else
                    {
                        savefunc();
                    }

                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        CloseLoadingPromptV4();
                    }

                    if(bRenderReconstructionOnce)
                    {
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                    }
                    else
                    {
                        bRenderReconstructionOnce = true;
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                }
            }
            else
            {

            }
        }
        void ConstructionWgt::SetLayerType()
        {
            std::set<AI3D::VIEWER::reconst_element_e> imageLayers;
            if (cbPhotos->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_PHOTOS);

            if (cbTiePoints->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_TIEPOINTS);

            if (cbGCP->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_GCP);

            if (cbTiling->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_TILE);

            if (cbROI->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_ROI);

            if (cbConstraints->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_CONSTRAINT);

            if (mWindow != nullptr)
                mWindow->ResetImageLayerSeleted(imageLayers);
        }
        void ConstructionWgt::Slot_SelectTypes()
        {
            if (mWindow == nullptr)
                return;

            std::set<AI3D::VIEWER::reconst_element_e> imageLayers;

            QCheckBox* pCheckBox = dynamic_cast<QCheckBox*>(sender());
            if (!pCheckBox)
                return;

            std::cout << "inside " << " " << __FUNCTION__ << " " << __LINE__ << pCheckBox->text().toStdString() << std::endl;

            if (cbPhotos->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_PHOTOS);

            if (cbTiePoints->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_TIEPOINTS);

            if (cbGCP->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_GCP);

            if (cbTiling->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_TILE);

            if (cbROI->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_ROI);

            if (cbConstraints->isChecked())
                imageLayers.insert(AI3D::VIEWER::RD_ELE_CONSTRAINT);

            if (mWindow != nullptr)
                mWindow->ResetImageLayerSeleted(imageLayers);
        }

        void ConstructionWgt::Slot_LoadOSGBFile()
        {
                //std::string fileName = "D:/TestData/model20221205/fengtaikejiyuan/das/Production_1(2)/OSGB/Data/Tile_+006_+012/Tile_+006_+012_L20_0uuuu41.osgb";
            std::string fileName = "D:/worksp/data/Tile_23.osgb";
            OsgEngine* pOsgEngine = OsgEngine::getInstance();

            pOsgEngine->initViewer();
            auto loadedModel = pOsgEngine->LoadOsgModel(fileName);
            osgUtil::Optimizer optimizer;
            optimizer.optimize(loadedModel.get());
#if 0           
            viewerWindow->updateTraversal();
            viewerWindow->setSceneData(loadedModel.get());
#endif
        }

        // Importing file on GeometryContraints area.
        void ConstructionWgt::Slot_GeometryContraintsImport()
        {
            
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            QString oldStr = ".";
            ///QFileDialog fd(nullptr, tr("Import KML/SHP file"), oldStr, tr("kml file(*.kml);;shp file(*.shp)"));

            std::string strTitle;
            std::string strChooseType;

            if (AI3D::CORE::BlockObject::isChineseVersion())
                strTitle = "导入 KML/SHP 文件";
            else
                strTitle = "Import KML/SHP file";

            ///QFileDialog fd(nullptr, tr("Import KML/SHP file"), oldStr, tr("kml/shp file(*.kml *.shp)"));
            QFileDialog fd(nullptr, QString::fromStdString(strTitle), oldStr, QString::fromStdString(strChooseType));
            fd.setAcceptMode(QFileDialog::AcceptOpen);
            fd.setFileMode(QFileDialog::ExistingFiles);
            fd.setViewMode(QFileDialog::Detail);
            //fd.setWindowFlags(Qt::WindowStaysOnTopHint);
            //fd.setOption(QFileDialog::DontUseNativeDialog, true);

            //QListView* listView = fd.findChild<QListView*>();
            //QTreeView* treeView = fd.findChild<QTreeView*>();

            //if (listView && treeView) {
            //  listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            //  treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            //}

            if (QDialog::Accepted != fd.exec())
            {
                std::cout << "got no kml file." << std::endl;
                return;
            }

            if (fd.selectedFiles().size() <= 0)
                return;
            
            QString file = fd.selectedFiles().first();
            std::cout << "got geometry contraints kml:" << qstr2str(file) << std::endl;
            std::string strfile = file.QString::toStdString();

            //std::vector<std::string> files(1,strfile);
            std::vector<std::string> files;
            for (auto& f : fd.selectedFiles())
            {
                std::string fileName = File::GetFileNameWithoutExtension(f.toStdString());
            
                files.push_back(f.toStdString());
            }

            int* progress = 0;
            std::string msg;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                msg = "不能添加该约束.";
            }
            else
            {
                msg = "Can not add this constraint.";
            }
            std::vector<constraint_info_s> cinfos_touse;
            // note: may need to transfer language version flag to the following function.
            auto ret = ReconstructionCommandSet::ImportConstraintFile(this->block_data_, this->recons_object_->GetId(),files, cinfos_touse, *progress, msg,AI3D::CORE::BlockObject::isChineseVersion());
            if (ret == AI3D_SUCCESS)
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    if (QMessageBox::No == Message_Box::question(this, "", "你想增加该表面约束吗？", Message_Box_Type::Question_Yes_No))
                    {
                        return;
                    }
                }
                else
                {
                    if (QMessageBox::No == Message_Box::question(this, "", "Do you want to add this surface constraint？", Message_Box_Type::Question_Yes_No))
                    {
                        return;
                    }
                }
            }
            else
            {
                // note: test chinese message returned from subroutine.
                QString text = QString::fromStdString(msg);
                OpenOkDialog(this, text);
                
                return;
            }
            
            this->recons_object_->UpdateConstraint(cinfos_touse);

            /*for (int i = twGeometryContraints->rowCount() - 1; i >= 0; i--)
            {
                twGeometryContraints->removeRow(i);
            }*/
            for (auto& f : files)
            {
                std::string fileName = File::GetFileNameWithoutExtension(f);

                InsertGeometryContraintsItem(str2qstr(fileName), "kml", "");
            }

            bool beditable = this->recons_object_->GetConstraintCustom().empty() && this->recons_object_->GetProductions().empty();
            SetRightSideEditable(beditable);
            
            if (ret == AI3D_SUCCESS)
            {
                
                    bool bLastMatrixExists = false;
                    osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                    if (bLastMatrixExists)
                    {
                        osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                        UserMatrixData::setCurrentMatrixObject(this, cmt);
                    }
                
            
                mWindow->ResetConstraint();
                if (bLastMatrixExists)
                {
                    mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                }
                else
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            
                SetProjectModified();
            }
            else
                return;
        }


        void ConstructionWgt::Slot_ItemClicked(QTableWidgetItem* pItem)
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (!pItem)
                return;

            int row = pItem->row();
            int col = pItem->column();

            std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " clicked item at:" << row << "/" << col << std::endl;
        }

        void ConstructionWgt::Slot_MWindowResized()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            
        }

        void ConstructionWgt::Slot_MoreSettings()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            OpenMoreSettings(this->recons_object_, this->recons_object_->HasProductions());
            auto& taskinfo = this->block_data_->GetTaskInfoMutual();
            auto& recindfo = std::find_if(taskinfo.reconstructions_info_.begin(), taskinfo.reconstructions_info_.end(),
                [&](blk_recontruction_info_s a) { return this->recons_object_->GetId() == a.id_; });

            if (recindfo != taskinfo.reconstructions_info_.end())
            {
                this->recons_object_->ToTaskInfo(*recindfo);
            }
            else
            {
                //@attention 理论上不应该到这
                return;
            }
            taskinfo.isSaved = false;
            SetProjectModified();
        }

        void ConstructionWgt::Slot_GeometryContraints_CustomContextMenuRequested(const QPoint& pos)
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            QModelIndex index = twGeometryContraints->indexAt(pos);
            if (index.isValid())
            {
            //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                menu_RightClick4GeometryContraints->exec(QCursor::pos());
            }
        }

        void ConstructionWgt::Slot_GeometryContraints_Delete()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
#if 0
            QItemSelectionModel* itemSelectionModel = twGeometryContraints->selectionModel();
            if (!itemSelectionModel)
            {
                std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                return;
            }
            //@注：此处可以多选
            QModelIndexList modelIndexList = itemSelectionModel->selectedIndexes();
            if (modelIndexList.size() != 1)
            {
                std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " size:" << modelIndexList.size() << std::endl;
                return;
            }

            QModelIndex modelIndex = modelIndexList.at(0);
#else
            QModelIndex modelIndex = twGeometryContraints->currentIndex();
            if (!modelIndex.isValid())
            {
                std::cout << "right click invalid:" << modelIndex.row() << " " << modelIndex.column() << std::endl;
                return;
            }
#endif

            int row = modelIndex.row();

            QString strName = twGeometryContraints->item(row, 0)->text();
            QString strType = twGeometryContraints->item(row, 1)->text();

            ///std::cout << "item to delete:" << qstr2str(strName) << " / " << qstr2str(strType) << std::endl;
            std::vector<int> index(1,row);
            int num_tiles = 0; bool bStatusInProduction = false;
            std::map<std::string, bool> tilestoprocess;
            int ret = ReconstructionCommandSet::DeleteConstraintsPre(this->block_data_, this->recons_object_->GetId(), index, tilestoprocess);
            num_tiles = index.size();
            //if (ret == AI3D_SUCCESS)
            {
                // where to get the value of num_tiles?
                //@attention 需要加个确认弹窗，如果ok的话就删除,弹窗需要显示的tile数量有num_tiles获得
                ///if (QMessageBox::No == Message_Box::question(this, "", "Are you sure you want to delete this surface constraint？", Message_Box_Type::Question_Yes_No))
                QString strTitle = "Are you sure you want to delete this surface constraint(%1 tiles)？";
                if (BlockObject::isChineseVersion())
                {
                    strTitle = "确认要删除该表面约束(%1 块)吗？";
                }

                //if (QMessageBox::No == Message_Box::question(this, "", QString("Are you sure you want to delete this surface constraint(%1 tiles)？").arg(num_tiles), Message_Box_Type::Question_Yes_No))
                if (QMessageBox::No == Message_Box::question(this, "", QString(strTitle).arg(num_tiles), Message_Box_Type::Question_Yes_No))
                {
                    return;
                }

                {
                    bool bRunFinished = false;

                    auto savefunc = [&, this]()
                    {
                        {
                            bool bLastMatrixExists = false;
                            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                            if (bLastMatrixExists)
                            {
                                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                                UserMatrixData::setCurrentMatrixObject(this, cmt);
                            }
                        }
                        int ret = ReconstructionCommandSet::DeleteConstraintsPost(this->block_data_, this->recons_object_->GetId(), index, tilestoprocess);

                        if (ret == AI3D_SUCCESS)
                        {

                            bool bLastMatrixExists = false;
                            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                            mWindow->ResetConstraint();
                            if (bLastMatrixExists)
                            {
                                mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                            }
                            else
                            {
                                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                                UserMatrixData::setCurrentMatrixObject(this, cmt);
                            }
                        }
                        bRunFinished = true;
                        return ret;
                    };
                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        if (AI3D::CORE::BlockObject::isChineseVersion()) {
                            OpenLoadingPromptV4("删除约束，请耐心等待");
                        }
                        else {
                            OpenLoadingPromptV4("Please be patient and wait.Constraint delete ");
                        }
                        QFuture<int> f1 = QtConcurrent::run(savefunc);

                        while (!bRunFinished)
                        {
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        int ret = f1.result();
                        if (ret == AI3D_SUCCESS)
                        {
                            twGeometryContraints->removeRow(row);

                            bool beditable = this->recons_object_->GetConstraintCustom().empty() && this->recons_object_->GetProductions().empty();
                            SetRightSideEditable(beditable);

                            SetProjectModified(); 
                        }
                        
                        CloseLoadingPromptV4();
                    }
                    else
                    {
                        int ret = savefunc();
                        if (ret != AI3D_SUCCESS)
                        {
                            return;
                        }
                        twGeometryContraints->removeRow(row);

                        bool beditable = this->recons_object_->GetConstraintCustom().empty() && this->recons_object_->GetProductions().empty();
                        SetRightSideEditable(beditable);

                        SetProjectModified();
                    }

                    LOGI("===================Constraint delete end.==============================");
                    std::cout << "======================Constraint delete end=================" << std::endl;
                }



                if (0)//@commend by chy 这个是原来的逻辑，后来改为上述savefunc了
                {
                    int ret = ReconstructionCommandSet::DeleteConstraintsPost(this->block_data_, this->recons_object_->GetId(), index, tilestoprocess);
                    if (ret == AI3D_SUCCESS)
                    {
                        bool bLastMatrixExists = false;
                        osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
                        mWindow->ResetConstraint();
                        if (bLastMatrixExists)
                        {
                            mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                        }
                        else
                        {
                            osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                            UserMatrixData::setCurrentMatrixObject(this, cmt);
                        }


                    }
                    else
                        return;


                    twGeometryContraints->removeRow(row);

                    bool beditable = this->recons_object_->GetConstraintCustom().empty() && this->recons_object_->GetProductions().empty();
                    SetRightSideEditable(beditable);

                    SetProjectModified();
                }
            }
            /*else
                return;*/

            //std::cout << "delete " << num_tiles  <<" "<< bStatusInProduction <<__FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        }

        void ConstructionWgt::Slot_ROIEdit_Saved()
        {
            /*std::cout << "inside " << __FILE__ << " " << __FUNCTION__ <<
                " " << __LINE__ << std::endl;*/
            bROIEditing = false;

            {
                // force to set current matrix after successfully edited for roi.
                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                UserMatrixData::setCurrentMatrixObject(this, cmt);
                savedMatrix = cmt;
            }
        }

        void ConstructionWgt::Slot_ROIEdit_Cancelled()
        {
           /* std::cout << "inside " << __FILE__ << " " << __FUNCTION__ <<
                " " << __LINE__ << std::endl;*/
            bROIEditing = false;

            // giving up the last update for current osgviewer and restoring the matrix of it to previous 
            // status before roi editing. no need to save the last matrix.
//          bool bLastMatrixExists = false;
//          osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);
//          if (bLastMatrixExists)
//          {
//              mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
//          }
//          else
            {
                mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(savedMatrix);
                UserMatrixData::setCurrentMatrixObject(this, savedMatrix);
            }
        }

        void ConstructionWgt::Slot_Delete_Production_Done()
        {
            std::ostringstream oss;
            oss << "delete one production.";
            LOGI(oss.str());
            std::cout << oss.str() << std::endl;
            if (this->recons_object_)
            {
                if (this->recons_object_->HasProductions())
                {
                    std::ostringstream oss;
                    oss << "delete one production.";
                    LOGI(oss.str());
                    std::cout << oss.str() << std::endl;
                }
                else
                {
                    std::ostringstream oss;
                    oss << "delete one production.";
                    LOGI(oss.str());
                    std::cout << oss.str() << std::endl;
                    RefreshEditableState();
                }
            }
            else
            {
                std::ostringstream oss;
                oss << "delete one production.";
                LOGI(oss.str());
                std::cout << oss.str() << std::endl;
            }
        }

        void ConstructionWgt::showEvent(QShowEvent* event)
        {
            std::ostringstream oss;
            oss << "ConstructionWgt/showEvent:" << std::hex << std::showbase << this << std::dec;
            //LOGI(oss.str());

            QString tabtext = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
            if (tabtext.toStdString() == "Overview")
                return;
            else if (tabtext.toStdString() == "Spatial Framework")
                ;
            else
                return;

            if (mWindow->hasSceneData())
                return;

            if (!bRenderReconstructionOnce)
                return;

            bool bLastMatrixExists = false;
            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

            LOGI("=====================Rendering,pls wait=============");
            std::cout << "======================Rendering,pls wait==================" << std::endl;
            bool bRunFinished = false;
            auto savefunc = [&, this]()
            {
                mWindow->RenderReconstruction(this->recons_object_);
                bRunFinished = true;
                return;
            };

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                if (BlockObject::isChineseVersion())
                {
                    OpenLoadingPromptV4("渲染中，请耐心等待");
                }
                else
                {
                    OpenLoadingPromptV4("Please be patient and wait.rendering");
                }
                QFuture<void> f1 = QtConcurrent::run(savefunc);

                while (!bRunFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else
            {
                savefunc();
            }

            //modify by zhaobf
            SetLayerType();

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }

            if (bLastMatrixExists)
            {
                mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
            }

           
        }

        void ConstructionWgt::hideEvent(QHideEvent* event)
        {
            std::ostringstream oss;
            oss << "ConstructionWgt/hideEvent:" << std::hex << std::showbase << this << std::dec;
        //  LOGI(oss.str());

            if (mWindow->hasSceneData())
            {
                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                UserMatrixData::setCurrentMatrixObject(this, cmt);
                mWindow->clearSceneData();
            }
        }

        void ConstructionWgt::closeEvent(QCloseEvent* event)
        {
            std::ostringstream oss;
            oss << "ConstructionWgt/closeEvent:" << std::hex << std::showbase << this << std::dec;
        //  LOGI(oss.str());
        }

        ProductionWgt::ProductionWgt(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject* recons_object, QStandardItem* recons_item, AI3D::CORE::ProductionObject* production_object, QStandardItem* production_item, QWidget* parent)
            : QWidget(parent)
        {
            ///ui = new Ui::CBlockWgt();
            butResubmitProduction = nullptr;
            ui = new Ui::CReConstructionWgt();
            ui->setupUi(this);
            block_data_ = block;
            this->recons_object_ = recons_object;
            this->recons_item = recons_item;
    
            this->production_object_ = production_object;
            this->production_item = production_item;
            qRegisterMetaType<QVariant>("QVariant");

            vecProductionItemInfo.clear();

            ui->btn_newContruction->setStyleSheet("background-color:#0072BE;color:white;width:160px;height:40px;border-radius:0px;border:2px solid;font:15px \"Arial\"");
            if(production_object)
                ui->btn_newContruction->setText(str2qstr(const_cast<std::string&>(production_object->GetName())));
            else
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    ui->btn_newContruction->setText("未知生产");
                }
                else
                {
                    ui->btn_newContruction->setText("Unkown Production");
                }
            }

            ui->label_AT->show();
            ui->label_Reconstruction->show();
            ui->label_Production->show();

            ui->btn_addsig->setEnabled(false);
            ui->btn_adddir->setEnabled(false);
            ui->btn_push_removePgtable->setEnabled(false);
            ui->btn_addpos->setEnabled(false);
            ui->btn_delpos->setEnabled(false);
            ui->btn_Siggcp->setEnabled(false);
            ui->btn_addgcp->setEnabled(false);
        /*    ui->btn_addgcp_measurements->setEnabled(false);
            ui->btn_exportgcpmeasurements->setEnabled(false);*/
            ui->btn_delgcp->setEnabled(false);
            ui->btn_at->setEnabled(false);
            ui->btn_paus->setEnabled(false);
            ui->btn_rec->setEnabled(false);

            // should the following three button be changed dynamically according to the detail status of current chosen job?
            ui->btn_submit_rec->setEnabled(true);
            ui->btn_resubmit_recon->setEnabled(false);
            ui->btn_cancle_recon->setEnabled(false);
            ui->frame_5->hide();

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                ui->btn_addsig->setToolTip(tr("导入影像"));
                ui->btn_adddir->setToolTip(tr("导入影像目录"));
                ui->btn_push_removePgtable->setToolTip(tr("删除选中影像"));

                ui->btn_addpos->setToolTip(tr("导入位姿"));
                ui->btn_delpos->setToolTip(tr("删除位姿"));

                ui->btn_Siggcp->setToolTip(tr("添加单独控制点"));
                ui->btn_addgcp->setToolTip(tr("添加控制点文件"));
                ui->btn_delgcp->setToolTip(tr("删除控制点"));
               

                ui->btn_at->setToolTip(tr("提交空三"));
                ui->btn_paus->setToolTip(tr("取消空三"));
                ui->btn_rec->setToolTip(tr("再次提交空三"));


                ui->label_AddData->setText(tr("导入影像"));

                ui->label_AT->setText(tr("空三"));
                ui->label_Reconstruction->setText(tr("重建"));
                ui->label_Production->setText("生产");

                ui->label_Pho->setText(tr("影像"));
                ui->label_Pos->setText(tr("位姿"));
                ui->label_5->setText(tr("控制点"));
                ui->label_AT_2->setText(tr("空三"));
            }
            else {
                ui->btn_addsig->setToolTip(tr("Import photo"));
                ui->btn_adddir->setToolTip(tr("Import directory"));
                ui->btn_push_removePgtable->setToolTip(tr("Remove selected"));

                ui->btn_addpos->setToolTip(tr("Import POS"));
                ui->btn_delpos->setToolTip(tr("Remove POS"));

                ui->btn_Siggcp->setToolTip(tr("Add Sig GCP"));
                ui->btn_addgcp->setToolTip(tr("Add GCP File"));
                ui->btn_delgcp->setToolTip(tr("Remove GCP"));

               /* ui->btn_addgcp_measurements->setToolTip(tr("Import GCPMeasurements From File"));
                ui->btn_exportgcpmeasurements->setToolTip(tr("Export GCP Measurements to File"));*/

                ui->btn_at->setToolTip(tr("Submit AT"));
                ui->btn_paus->setToolTip(tr("Cancel AT"));
                ui->btn_rec->setToolTip(tr("Resubmit AT"));

            }


            ui->widget->setStyleSheet("background-color:rgb(40,40,40);");

            QVBoxLayout* vlOverviewContainer = new QVBoxLayout();
            vlOverviewContainer->setContentsMargins(5, 5, 5, 5);

            QWidget* panelOverview = new QWidget(ui->widget);
            panelOverview->setObjectName("panelOverview");
            panelOverview->setStyleSheet("#panelOverview {background-color:rgb(40,40,40);border:1px solid rgb(72,72,72);padding:5px;}");
            panelOverview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            vlOverviewContainer->addWidget(panelOverview, 1);
            //vlOverviewContainer->setContentsMargins(22, 0, 22, 0);

            QVBoxLayout* vlOverview = new QVBoxLayout();

            vlOverview->setSpacing(12);
            vlOverview->setContentsMargins(27, 27, 27, 27);

//          ui->widget->setStyleSheet("background-color:rgb(40,40,40);");

            QWidget* panelTop = new QWidget(ui->widget);
            //panelTop->setFixedHeight(222);
            ///panelTop->setStyleSheet("border-radius:14px;background-color:rgb(46,59,74);");

            QHBoxLayout* hlTop = new QHBoxLayout();

            hlTop->addSpacing(45);
            hlTop->setSpacing(15);

            lblTopLeft = new QLabel(panelTop);
            //lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_succ.png"));
            lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_wait.png"));
            lblTopLeft->setFixedWidth(120);
            lblTopLeft->setFixedHeight(120);
            ///lblTopLeft->hide();

            hlTop->addWidget(lblTopLeft);

            cpwLeft = new CircularProgressWgt(panelTop);
            cpwLeft->setAttribute(Qt::WA_StyledBackground, true);
            cpwLeft->resize(120, 120);

            hlTop->setSpacing(40);
            hlTop->addWidget(cpwLeft);

            cpwLeft->setStyleSheet("color:blue;background-color:rgb(40,40,40);");
            cpwLeft->hide();
            //cpwLeft->show();

            // part of the top panel of the overview tabpage for the production.
            QVBoxLayout* vlTopRight = new QVBoxLayout();
            lblTopRightTop = new QLabel(panelTop);
            lblTopRightBottom = new QLabel(panelTop);

            vlTopRight->setSpacing(20);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTopRightTop->setText("等待中");
            }
            else
            {
                lblTopRightTop->setText("Pending");
            }
            int tilecount = this->production_object_->GetTiles().size();
            std::string cntstr = std::to_string(tilecount);
            std::string str;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                str = " 生产已经提交，等待运行." + cntstr + " 块";
            }
            else
            {
                str = " Production submitted,waiting to run." + cntstr + " tiles.";
            }

            QString qstr =QString::fromStdString(str);
            lblTopRightBottom->setText(qstr);

            lblTopRightTop->setStyleSheet("color:rgb(146,231,197);font:18px \"Arial\";");
            lblTopRightBottom->setStyleSheet("color:rgb(213,213,213);font:14px \"Arial\";");

            //vlTopRight->addWidget(lblTopRightTop, 1);
            //vlTopRight->addWidget(lblTopRightBottom, 1);

            vlTopRight->addStretch(1);
            vlTopRight->addWidget(lblTopRightTop);
            vlTopRight->addWidget(lblTopRightBottom);
            vlTopRight->addStretch(1);

            hlTop->addLayout(vlTopRight);       

            butCancelProduction = new QPushButton(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                butCancelProduction->setText("取消");
            }
            else
            {
                butCancelProduction->setText("Cancel");
            }

            hlTop->addStretch(1);
            
            hlTop->addWidget(butCancelProduction);

            hlTop->addSpacing(50);

            panelTop->setLayout(hlTop);

            QHBoxLayout* hlProductions = new QHBoxLayout();
            QLabel* lblProductions = new QLabel(ui->widget);

            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblProductions->setText("生产列表");
            }
            else {
                lblProductions->setText("Productions");
            }

            lblProductions->setStyleSheet("color:white;font:18px \"Arial\";");
            hlProductions->addWidget(lblProductions, 0, Qt::AlignLeft);

            twProductionList = new QTableWidget(ui->widget);
            twProductionList->setColumnCount(5);

            QStringList slProductionList;

            //slProductionList << "Name" << "Status" << "Progress" << "Completed timestamp";
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                //
                slProductionList << "名称" << "状态" << "进度" << "描述" << "最后提交时间";
            }
            else {
                slProductionList << "Name" << "Status" << "Progress" << "Description" << "Last submitted";
            }

            twProductionList->setHorizontalHeaderLabels(slProductionList);
            twProductionList->verticalHeader()->hide();
            twProductionList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            twProductionList->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            //twProductionList->horizontalHeader()->setStretchLastSection(true);
            //twProductionList->setStyleSheet("background-color:yellow;color:orange;");
            //twProductionList->horizontalHeader()->setFixedHeight(52);
            twProductionList->horizontalHeader()->setFixedHeight(40);
            twProductionList->setSelectionBehavior(QAbstractItemView::SelectRows);
            //twProductionList->verticalHeader()->setDefaultSectionSize(72);
            twProductionList->verticalHeader()->setDefaultSectionSize(50);
            twProductionList->setFocusPolicy(Qt::NoFocus);

            twProductionList->setStyleSheet("QTableWidget { background-color:rgb(40,40,40);color:rgb(230,230,230);font: 14px \"Arial\";border:none;border-right:1px solid rgb(60,60,60);}"
                "QHeaderView::section{ background-color:rgb(68,68,68);color: rgb(230,230,230); border:none;padding-left:40px;padding-top:0px;}"
//              "QTableWidget::item { background-color:rgb(40,40,40);color:rgb(236,236,236);border:none;border-bottom:1px solid rgb(60,60,60);padding-top:24px;padding-bottom:22px;border-right:1px solid rgb(60,60,60);}"
                "QTableWidget::item { background-color:rgb(40,40,40);color:rgb(236,236,236);border-left:1px solid rgb(60,60,60);border-bottom:1px solid rgb(60,60,60);padding-top:0px;padding-bottom:0px;padding-left:40px;}"
                "QTableWidget::item:selected { background-color:rgb(36,48,55);color:rgb(236,236,236);}");

            QWidget* panelBottom = new QWidget(ui->widget);
            panelBottom->setStyleSheet("border-radius:14px;background-color:rgb(55,55,55);padding:0px;");

            std::vector<std::pair<std::string, std::string>> setInformation;
            std::vector<std::string> translated_infos;
            ReconstructionCommandSet::GetProductionSetInformation(this->production_object_, setInformation,translated_infos);
            
            setInformationByPurpose(panelBottom, setInformation,translated_infos);

            ///ui->widget->setVisible(false);
            ui->btn_newContruction->setVisible(false);

            //          vlOverview->addStretch(1);
            vlOverview->addWidget(panelTop, 2);
            vlOverview->addLayout(hlProductions);
            vlOverview->addWidget(twProductionList, 5);
            vlOverview->addWidget(panelBottom, 3);

        //  vlOverview->addLayout(hlNewReconstruction);
            //vlOverview->addStretch(1);

            panelOverview->setLayout(vlOverview);

            ///ui->widget->setLayout(vlOverview);
            ui->widget->setLayout(vlOverviewContainer);

            ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tab), QApplication::translate("CBlockWgt", "Overview", nullptr));
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->tab_4), QApplication::translate("CBlockWgt", "3D View", nullptr));

            refresh_timer_ = new QTimer(this);

            if (BlockObject::supportOptimization4ProductionListOverview())
            {
                connect(refresh_timer_, &QTimer::timeout, this, &ProductionWgt::Slot_Refresh_TimeoutV2);
            }
            else
            {
                connect(refresh_timer_, &QTimer::timeout, this, &ProductionWgt::Slot_Refresh_Timeout);
            }

            connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &ProductionWgt::Slot_ClickTab);

            refresh_timer_->start(200);

            mWindow = new MWindow(ui->tab_4,0,true);
            std::cout << "=====++++ inside productionwgt:" << std::hex << std::showbase << this << " / " << mWindow << std::dec << std::endl;

            QHBoxLayout* hlCenterArea = new QHBoxLayout();
            hlCenterArea->setContentsMargins(0, 0, 0, 0);
            hlCenterArea->addWidget(mWindow,1);
            ui->verticalLayout_6->addLayout(hlCenterArea, 1);

            bCancelled = false;
            status_ = job_status_e::STATUS_UNKNOWN;

            //ui->tabWidget->setCurrentIndex(1);

            if (!bProductionModelRandomTest)
            {
                std::cout << "render model inside productionwgt:" << recons_object->GetPath() << std::endl;
                /// note:Lazy rendering for production model,rendering it later only when needed.
                ///mWindow->RenderModel(recons_object->GetPath());

                ///mWindow->RenderModel("D:\\osgb");

#if 000
                mWindow->RenderReconstruction(this->recons_object_);
#else
                AI3D::CORE::ATData* reconstruction__ = block_data_->GetCurrentATMutual().get();
                AI3D::CORE::ATData* at_data__ = new AI3D::CORE::ATData();
                *at_data__ = *reconstruction__;
                std::vector<image_t> selectedImages__;

                ///mWindow->RenderBlockWithSelectedImages(*at_data__, block_data_->GetStatus(), selectedImages__);
                ////mWindow->RenderModel("D:\\osgb");
                std::cout << "inside ProductionWgt,mWindow render model: " << __DATE__ << " " << __TIME__ << " " << recons_object->GetPath() << std::endl;
                ///mWindow->RenderModel(recons_object->GetPath());
#endif
            }
            else
            {           
#if 0
                int tiles4test_num = tiles4test->size();
                int tiles4test_index = qrand() % tiles4test_num;
                std::string tiles4test_gotByRandom = tiles4test_dyt + tiles4test[tiles4test_index];

                int try_times_to_strip_duplicate = 0;
                std::ofstream tiles4test_logfile = AI3D::CORE::File::OpenOfstreamUtf8("d:/tiles4test.log", std::ios::out | std::ios::app);

                while (tiles4test_index_used.count(tiles4test_index) > 0 && try_times_to_strip_duplicate <= 20)
                {
                    std::cout << "got duplicate index:" << tiles4test_index << std::endl;
                    tiles4test_index = qrand() % tiles4test_num;
                    tiles4test_gotByRandom = tiles4test_dyt + tiles4test[tiles4test_index];
                    try_times_to_strip_duplicate++;
                    std::cout << "used index:" << tiles4test_index << std::endl;
                    if (tiles4test_logfile.is_open())
                    {
                        tiles4test_logfile << "used index:" << tiles4test_index << "\n";
                    }
                }

                if (tiles4test_index_used.count(tiles4test_index) > 0)
                {
                    if (tiles4test_logfile.is_open())
                    {
                        tiles4test_logfile << "still duplicate,used index:" << tiles4test_index << "\n";
                    }
                }
                else
                {
                    tiles4test_index_used.insert(tiles4test_index);
                    if (tiles4test_logfile.is_open())
                    {
                        tiles4test_logfile << "no duplicate,used index:" << tiles4test_index << "\n";
                    }
                }

                if (try_times_to_strip_duplicate > 0)
                {
                    std::cout << "test tiles index:" << tiles4test_index << "/" << tiles4test_num << " try times to strip duplicate:" << try_times_to_strip_duplicate << std::endl;
                    if (tiles4test_logfile.is_open())
                    {
                        tiles4test_logfile << "test tiles index:" << tiles4test_index << "/" << tiles4test_num << " try times to strip duplicate:" << try_times_to_strip_duplicate << "\n";
                    }
                }
                else
                {
                    std::cout << "test tiles index:" << tiles4test_index << "/" << tiles4test_num << std::endl;
                    if (tiles4test_logfile.is_open())
                    {
                        tiles4test_logfile << "test tiles index:" << tiles4test_index << "/" << tiles4test_num << "\n";
                    }
                }

                tiles4test_logfile.close();

                std:cout << "random test for production:" << tiles4test_gotByRandom << std::endl;

                mWindow->RenderModel(tiles4test_gotByRandom);
#endif
            }


            butCancelProduction->setStyleSheet("background-color:transparent;color:#8EA3D4;width:100px;height:36px;border-radius:6px;border:1px solid #6D7DA3;font:bold 14px \"Arial\";");
            connect(butCancelProduction, &QPushButton::clicked, this, &ProductionWgt::Slot_CancelProduction);

            if (butResubmitProduction != nullptr)
            {
                ///         butResubmitProduction->setStyleSheet("background-color:#165DFF;color:white;width:150px;height:42px;border-radius:0px;border:none;font:14px \"Arial\"");
                std::cout << "connect signal/slot for ResubmitProduction button." << std::endl;
                butResubmitProduction->setStyleSheet("background-color:#165DFF;color:white;width:150px;height:32px;border-radius:0px;border:none;font:14px \"Arial\"");
                connect(butResubmitProduction, &QPushButton::clicked, this, &ProductionWgt::Slot_ResubmitProduction);
            }

#if 0
            bool bTestAllCompleted = false;

            for (int i = 0; i < 4; i++)
            {
                twProductionList->insertRow(twProductionList->rowCount());
                int lastRow = twProductionList->rowCount() - 1;

                for (int j = 0; j < twProductionList->columnCount(); j++)
                {
                    if (j == 2)
                    {
                        QWidget* pProgBarContainer = new QWidget(ui->widget);
                        pProgBarContainer->setStyleSheet("padding:0px;margin:0px;");

                        QHBoxLayout* hlProgBar = new QHBoxLayout();
                        hlProgBar->setContentsMargins(0, 0, 0, 0);

                        QProgressBar* pProgBar = new QProgressBar(ui->widget);
                        pProgBar->setAttribute(Qt::WA_StyledBackground);
                        pProgBar->setMinimum(0);
                        pProgBar->setMaximum(100);
                        //pProgBar->setFixedHeight(24);
                        pProgBar->setFixedWidth(140);
                        pProgBar->setFixedHeight(4);

                        QLabel* pLblProg = new QLabel(ui->widget);
                        pLblProg->setAlignment(Qt::AlignLeft);

                        //pLblProg->setText("color:rgb(185,185,185);padding:0px;margin:0px;font:12 \"Arial\";");


                        hlProgBar->setSpacing(12);
                        //hlProgBar->addStretch(1);
                        hlProgBar->addWidget(pProgBar);
                        hlProgBar->addWidget(pLblProg);
                        hlProgBar->addStretch(1);

                        pProgBarContainer->setLayout(hlProgBar);

                        if (i  == 0)
                        {
                            pProgBar->setValue(0);
                            pLblProg->setText("100%");
                            pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}"
                            );
                            pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

//                          nCompletedTiles++;
                        }
                        else if (i == 1)
                        {
                            int percent = 0;
                            percent = 73;
                            if (percent < 1)
                                percent = 1;
                            pProgBar->setValue(percent);

                            pLblProg->setText(QString("%1%").arg(percent));
                            //pLblProg->setText("100%");
                            pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                            pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

//                          nFailedTiles++;
                        }
                        else if (i == 2)
                        {
                            //  pItem->setText("Running");
                            int percent = 43; // (int)(feedback.Percent + 0.5);
                            pProgBar->setValue(percent);
                            pLblProg->setText(QString("%1%").arg(percent));
                            //pLblProg->setText("100%");

                            pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                            pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

//                          nRunningTiles++;
                        }
                        else
                        {
                            pProgBar->setValue(0);
                            pLblProg->setText("0%");
                            //pLblProg->setText("100%");
                            pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                            );
                            pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

//                          nPendingTiles++;
                        }

                        pProgBar->setTextVisible(false);
                        //twProductionList->setCellWidget(lastRow, j, pProgBar);
                        twProductionList->setCellWidget(lastRow, j, pProgBarContainer);
                    }
                    else
                    {
                        QTableWidgetItem* pItem = new QTableWidgetItem;
                        pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);

                        if (j == 0)
                            pItem->setText(QString("tile_%1").arg(j));
                        else if (j == 1)
                        {
                            if (i == 0)
                            {
                                pItem->setText("Completed");
                            }
                            else if (i == 1)
                            {
                                pItem->setText("Failed");
                            }
                            else if (i == 2)
                            {
                                pItem->setText("Running");
                            }
                            else
                            {
                                pItem->setText("Pending");
                            }
                        }
                        else
                            pItem->setText("2023/7/18 15:00");
                        //  pItem->setText(QString("txt(%1,%2)").arg(i + 1).arg(j + 1));
                        pItem->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                        twProductionList->setItem(lastRow, j, pItem);
                    }
                }
            }

            if (bTestAllCompleted)
                panelBottom->setVisible(false);
#endif
            bProductionItemInfoFirstRendered = false;
            InitProductionItemInfo();
            //bProductionItemInfoNeedRendering = false;
            bDestroying = false;
            bResubmitting = false;
            bProductionItemInfoGetting = false;
            bProductionItemInfoGot = false;
        }

        ProductionWgt::~ProductionWgt()
        {
            bDestroying = true;
            emit signal_delete_production_done();

            while (bProductionItemInfoGetting || bProductionItemInfoGot)
            {
                // note: wait for the related thread fetching tiles status peroidically to finish.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::cout << "===+++ delete productionwgt:" << std::hex << std::showbase << this << std::dec <<  std::endl;
        }

        // 

        void ProductionWgt::Slot_Dummy()
        {

        }

        job_status_e ProductionWgt::CalcStatusAndPercent(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject* recons_object, AI3D::CORE::ProductionObject* cpo, int& percent)
        {
            percent = 0;

            if (!cpo)
                return jobsta_e::STATUS_PENDDING;

            if (cpo->GetTiles().size() <= 0)
                return jobsta_e::STATUS_PENDDING;

            int nTotalTiles = 0;
            int nCompletedTiles = 0;
            int nPendingTiles = 0;
            int nRunningTiles = 0;       
            int nFailedTiles = 0;

            QString lsMasterJobQueue = Settings::getMasterJobQueue();

            //for (int i = 0; i < cpo->GetTiles().size(); i++)
            for (auto & iter :cpo->GetTiles())
            {
                int jobStatus = -1;
                std::string fullPathJobName;
                std::string tile_name =iter.second.name_;
                std::string tile_jobstr = ReconstructionCommandSet::ResolveProductionTileJobStr(
                    block, recons_object, cpo, tile_name, true);
//              std::string feedback_file_ = cpo->GetFeedbackFiles().at(i);
                std::string feedback_file_;
                //临时
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
                {
                    std::string blockitembase_path = block->GetPath() + "/" + recons_object->GetIDString() +
                        "/" + "Productions/" + cpo->GetIDString() + "/" + tile_name + "/";
                    std::string BRPID = std::string("B") + std::to_string(block->GetId()) + std::string("R") +
                        std::to_string(recons_object->GetId()) + std::string("P") +
                        std::to_string(cpo->GetId()) + tile_name;
                    if (!block->GetTaskInfoMutual().reconstructionjobs_.count(BRPID))
                        continue;
                    std::string jobstring = block->GetTaskInfoMutual().reconstructionjobs_.at(BRPID);
                    //std::string feedback_file_ = "";
                    if (JOB_FEEDBACK_USE_BIN) {
                        feedback_file_ = MAKE_FEEDBAK_BIN_FILE(blockitembase_path, jobstring);
                    }
                    else {
                        feedback_file_ = MAKE_FEEDBAK_JSON_FILE(blockitembase_path, jobstring);
                    }
                    //feedback_file_ = File::EnsureUnifySlash(blockitembase_path + FEEDBACK_PREFIX + jobstring + ".json");
                }
                else
                {
                    
                    feedback_file_ = ReconstructionCommandSet::GenerateTileFeedbackFile(block, cpo, tile_name, tile_jobstr);

                    //feedback_file_ = ReconstructionCommandSet::GenerateFeedbackFile(block, recons_object, cpo, tile_, lsMasterJobQueue.toStdString(), fullPathJobName, &jobStatus);
                }
                if (!tile_name.empty())
                    nTotalTiles++;

                bool bFoundCorrectFeedbackFile = false;

                if (!tile_name.empty() && !feedback_file_.empty())
                {
                    JobFeedBack_s feedback;
                    bool ret = feedback.load_with_retry(feedback_file_);
                    if (ret)
                    {
                        bFoundCorrectFeedbackFile = true;
                        if (feedback.Status == STATUS_COMPLETE)
                        {
                            nCompletedTiles++;
                        }
                        else if (feedback.Status == STATUS_CANCLE || feedback.Status == STATUS_FAILURE)
                        {
                            nFailedTiles++;
                        }
                        else if (feedback.Status == STATUS_RUNNING)
                        {
                            nRunningTiles++;
                        }
                        else
                        {
                            nPendingTiles++;
                        }
                    }
                }

                if (!bFoundCorrectFeedbackFile && jobStatus >= 0 && jobStatus <= 4)
                {
                    if (jobStatus == 2 /*STATUS_COMPLETE*/)
                    {
                        nCompletedTiles++;
                    }
                    else if (jobStatus == 3 || jobStatus == 4 /*feedback.Status == STATUS_CANCLE || feedback.Status == STATUS_FAILURE*/)
                    {
                        nFailedTiles++;
                    }
                    else if (jobStatus == 1 /*feedback.Status == STATUS_RUNNING*/)
                    {
                        nRunningTiles++;
                    }
                    else
                    {
                        nPendingTiles++;
                    }
                }
            }

            if (nRunningTiles > 0)
            {
                percent = nCompletedTiles * 100 / nTotalTiles;
                //              lblTopRightTop->setText("Running");
                return jobsta_e::STATUS_RUNNING;
            }
            else if (nTotalTiles == 0 || nTotalTiles == nPendingTiles)
            {
                percent = 0;
                //              lblTopRightTop->setText("Pending");
                return jobsta_e::STATUS_PENDDING;
            }
            else if (nTotalTiles == nCompletedTiles)
            {
                percent = 100;
                //              lblTopRightTop->setText("Completed");
                return jobsta_e::STATUS_COMPLETE;
            }
            else if (nPendingTiles > 0)
            {
                percent = nCompletedTiles * 100 / nTotalTiles;
                //              lblTopRightTop->setText("Running");
                return jobsta_e::STATUS_RUNNING;
            }
            else if (nFailedTiles > 0)
            {
                percent = nCompletedTiles * 100 / nTotalTiles;
                //              lblTopRightTop->setText("Fail");
                return jobsta_e::STATUS_FAILURE;
            }
            else
            {
                // impossible to reach here.even if coming here,it should be completed state.
                percent = 100;
                return jobsta_e::STATUS_COMPLETE;
            }

            return jobsta_e::STATUS_PENDDING;
        }

        void ProductionWgt::setInformationOriginal(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation)
        {
            // set fixed height if needed.
            QVBoxLayout* vlPanelBottom = new QVBoxLayout();
            ///vlPanelBottom->setContentsMargins(29, 31, 29, 31);
            vlPanelBottom->setContentsMargins(29, 15, 29, 5);

            QHBoxLayout* hlBottomTitle = new QHBoxLayout();
            hlBottomTitle->setContentsMargins(0, 0, 0, 0);
            QLabel* lblBottomTitle = new QLabel(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                lblBottomTitle->setText("设置信息");
            }
            else {
                lblBottomTitle->setText("Setting Information");
            }
            lblBottomTitle->setStyleSheet("color:rgb(255,255,255);font:16px \"Arial\";padding:0px;margin:0px;");

            hlBottomTitle->addWidget(lblBottomTitle);
            hlBottomTitle->addStretch(1);

            QFrame* lineBottom = new QFrame(ui->widget);
            lineBottom->setFrameShape(QFrame::HLine);
            lineBottom->setFrameShadow(QFrame::Plain);
            //lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:2px;padding:0px;margin:0px;margin-top:31px;");
            lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;");

            QHBoxLayout* hlBottomBottomOne = new QHBoxLayout();
            hlBottomBottomOne->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomTwo = new QHBoxLayout();
            hlBottomBottomTwo->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomThree = new QHBoxLayout();
            hlBottomBottomThree->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomFour = new QHBoxLayout();
            hlBottomBottomFour->setContentsMargins(0, 0, 0, 0);

            lblID = new QLabel(panelBottom);
            lblID->setText("ID:");
            lblID->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblFormat = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblFormat->setText("格式:");
            }
            else {
                lblFormat->setText("Format:");
            }
            lblFormat->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblDestination = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblDestination->setText("输出目录");
            }
            else {
                lblDestination->setText("Destination");
            }
            lblDestination->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomOne->addWidget(lblID, 1);
            hlBottomBottomOne->addWidget(lblFormat, 1);
            hlBottomBottomOne->addWidget(lblDestination, 2);

            lblTypeOfLevelOfDetail = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTypeOfLevelOfDetail->setText("LOD类型：");
            }
            else
            {
                lblTypeOfLevelOfDetail->setText("Type of level of detail:");
            }

            lblSpatialReferenceSystem = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblSpatialReferenceSystem->setText("输出坐标系：");
            }
            else
            {
                lblSpatialReferenceSystem->setText("Spatial Reference System:");
            }

            lblSpatialReferenceSystem->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblOrigin = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblOrigin->setText("原点：");
            }
            else
            {
                lblOrigin->setText("Origin:");
            }

            lblOrigin->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            //          hlBottomBottomTwo->addWidget(lblOrigin, 1);
            //          hlBottomBottomTwo->addWidget(lblLevelOfDetailSize, 1);
            hlBottomBottomTwo->addWidget(lblTypeOfLevelOfDetail, 1);
            hlBottomBottomTwo->addWidget(lblOrigin, 1);
            hlBottomBottomTwo->addWidget(lblSpatialReferenceSystem, 2);

            lblLevelOfDetailSize = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblLevelOfDetailSize->setText("LOD大小：");
            }
            else
            {
                lblLevelOfDetailSize->setText("Level of detail size:");
            }

            lblLevelOfDetailSize->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");
            ///lblLevelOfDetailSize->hide();

            lblScopeOfLevelOfDetail = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblScopeOfLevelOfDetail->setText("LOD范围模式：");
            }
            else
            {
                lblScopeOfLevelOfDetail->setText("Scope of level of detail:");
            }

            lblScopeOfLevelOfDetail->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblTextureCompressionQuality = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTextureCompressionQuality->setText("纹理压缩质量：");
            }
            else
            {
                lblTextureCompressionQuality->setText("Texture compression quality:");
            }

            lblTextureCompressionQuality->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomThree->addWidget(lblLevelOfDetailSize, 1);
            hlBottomBottomThree->addWidget(lblScopeOfLevelOfDetail, 1);
            hlBottomBottomThree->addWidget(lblTextureCompressionQuality, 2);

            QLabel* lblTextureSharpening = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTextureSharpening->setText("纹理锐化：");
            }
            else
            {
                lblTextureSharpening->setText("Texture sharpening:");
            }

            lblTextureSharpening->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblIncludeTextureMaps = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblIncludeTextureMaps->setText("包含纹理贴图：");
            }
            else
            {
                lblIncludeTextureMaps->setText("Include texture maps:");
            }

            lblIncludeTextureMaps->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblMaximumTextureSize = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblMaximumTextureSize->setText("最大纹理尺寸：");
            }
            else
            {
                lblMaximumTextureSize->setText("Maximum texture size:");
            }

            lblMaximumTextureSize->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");
            
            QLabel* lblSkirtLengthInPixels = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblSkirtLengthInPixels->setText("裙边长度（像素）：");
            }
            else
            {
                lblSkirtLengthInPixels->setText("Skirt length in pixels:");
            }

            lblSkirtLengthInPixels->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            std::string texture_sharpening_ = "Enabled";
            std::string include_texture_maps_ = "true";
            std::string maximum_texture_size_ = "8192";
            std::string skirt_length_in_pixels_ = "";

            hlBottomBottomFour->addWidget(lblTextureSharpening, 1);
            hlBottomBottomFour->addWidget(lblIncludeTextureMaps, 1);
            hlBottomBottomFour->addWidget(lblMaximumTextureSize, 1);
            hlBottomBottomFour->addWidget(lblSkirtLengthInPixels, 1);

            vlPanelBottom->setSpacing(0);
            vlPanelBottom->addLayout(hlBottomTitle);
            ///vlPanelBottom->addSpacing(31);
            vlPanelBottom->addSpacing(10);
            vlPanelBottom->addWidget(lineBottom);

            //vlPanelBottom->addLayout(hlBottomBottom);

            vlPanelBottom->addLayout(hlBottomBottomOne, 1);
            vlPanelBottom->addLayout(hlBottomBottomTwo, 1);
            vlPanelBottom->addLayout(hlBottomBottomThree, 1);
            vlPanelBottom->addLayout(hlBottomBottomFour, 1);

            ///vlPanelBottom->addLayout(hlBottomBottom,1);

///         vlPanelBottom->addStretch(1);

            QHBoxLayout* hlResubmit = new QHBoxLayout();
            butResubmitProduction = new QPushButton(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion()) {
                butResubmitProduction->setText("重新提交");
            }
            else {
                butResubmitProduction->setText("Resubmit");
            }
            hlResubmit->addStretch(1);
            hlResubmit->addWidget(butResubmitProduction);

            vlPanelBottom->addLayout(hlResubmit);

            panelBottom->setLayout(vlPanelBottom);

            ///panelBottom->hide();

            std::string productionid_ = "";
            std::string format_ = "";
            std::string destination_ = "";

            std::string lod_type_ = "";
            std::string originstr_ = "";
            std::string srs_str_ = "";

            std::string tileoverlapunit_ = "";
            std::string scope_of_level_of_detail_ = "";
            std::string texture_compression_quality_ = "";
            std::string str_sampling_distance = "";
            std::string str_point_sampling_distance = "";
            std::string str_point_sampling_unit = "";
            std::string str_tdom_enabled = "";
            std::string str_dsm_enabled = "";

            //destination_ = "D:/jiaojie/test/test/testkml/Productions/Production_2";

            for (auto& t : setInformation)
            {
                //std::cout << "got " << t.first << " : " << t.second << std::endl;
                if (t.first == "Production ID")
                    productionid_ = t.second;
                else if (t.first == "Format")
                    format_ = t.second;
                else if (t.first == "Destination")
                    destination_ = t.second;
                else if (t.first == "Type of level of detail")
                    lod_type_ = t.second;
                else if (t.first == "Origin") // note:should strip the beginning space, but modify ReconstructionCommandSet::GetProductionSetInformation similarly. 
                    originstr_ = t.second;
                else if (t.first == "Spatial Reference System")
                    srs_str_ = t.second;
                else if (t.first == "Tile overlap in meters/units")
                    tileoverlapunit_ = t.second;
                else if (t.first == "Scope of level of detail")
                    scope_of_level_of_detail_ = t.second;
                else if (t.first == "Texture compression quality")
                    texture_compression_quality_ = t.second;
                else if (t.first == "Texture sharpening")
                    texture_sharpening_ = t.second;
                else if (t.first == "Include texture maps")
                    include_texture_maps_ = t.second;
                else if (t.first == "Maximum texture size")
                    maximum_texture_size_ = t.second;
                else if (t.first == "Sampling distance")
                    str_sampling_distance = t.second;
                else if (t.first == "Point sampling distance")
                    str_point_sampling_distance = t.second;
                else if (t.first == "Point sampling unit")
                    str_point_sampling_unit = t.second;
                else if (t.first == "Orthophoto enabled")
                    str_tdom_enabled = t.second;
                else if (t.first == "DSM enabled")
                    str_dsm_enabled = t.second;
                else
                {
                    //std::cout << "got/neq " << t.first << " : " << t.second << std::endl;
                }
            }
            if (productionid_ != "")
            {
                lblID->setText(QString("ID: %1").arg(str2qstr(productionid_)));
            }

            if (format_ != "")  lblFormat->setText(QString("Format: %1").arg(str2qstr(format_)));
            if (destination_ != "") lblDestination->setText(QString("Destination: %1").arg(str2qstr(destination_)));
            if (srs_str_ != "")
                lblSpatialReferenceSystem->setText(QString("Spatial Reference System: %1").arg(str2qstr(srs_str_)));
            else
            {
                lblSpatialReferenceSystem->setText(QString("                         "));
            }
            if (lod_type_ != "")
                lblTypeOfLevelOfDetail->setText(QString("Type of level of detail: %1").arg(str2qstr(lod_type_)));
            if (str_sampling_distance != "")
                lblTypeOfLevelOfDetail->setText(QString("Sampling distance: %1").arg(str2qstr(str_sampling_distance)));

            if (originstr_ != "")
                lblOrigin->setText(QString("Origin: %1").arg(str2qstr(originstr_)));


            if (tileoverlapunit_ != "")
                lblLevelOfDetailSize->setText(QString("Tile overlap in meters/units: %1").arg(str2qstr(tileoverlapunit_)));
            if (str_point_sampling_distance != "")
                lblLevelOfDetailSize->setText(QString("Point sampling distance: %1").arg(str2qstr(str_point_sampling_distance)));

            if (scope_of_level_of_detail_ != "")
                lblScopeOfLevelOfDetail->setText(QString("Scope of level of detail: %1").arg(str2qstr(scope_of_level_of_detail_)));
            if (str_point_sampling_unit != "")
                lblScopeOfLevelOfDetail->setText(QString("Point sampling unit: %1").arg(str2qstr(str_point_sampling_unit)));

            if (texture_compression_quality_ != "")
                lblTextureCompressionQuality->setText(QString("Texture compression quality: %1").arg(str2qstr(texture_compression_quality_)));
            if (str_tdom_enabled != "")
                lblTextureCompressionQuality->setText(QString("Orthophoto enabled: %1").arg(str2qstr(str_tdom_enabled)));

            if (texture_sharpening_ != "")
                lblTextureSharpening->setText(QString("Texture sharpening: %1").arg(str2qstr(texture_sharpening_)));
            if (str_dsm_enabled != "")
                lblTextureSharpening->setText(QString("DSM enabled: %1").arg(str2qstr(str_dsm_enabled)));
            if (include_texture_maps_ != "")
                lblIncludeTextureMaps->setText(QString("Include texture maps: %1").arg(str2qstr(include_texture_maps_)));
            if (maximum_texture_size_ != "")
                lblMaximumTextureSize->setText(QString("Maximum texture size: %1").arg(str2qstr(maximum_texture_size_)));

            if (skirt_length_in_pixels_ != "")
                lblSkirtLengthInPixels->setText(QString("Skirt length in pixels: %1").arg(str2qstr(skirt_length_in_pixels_)));
            else
                lblSkirtLengthInPixels->setText(QString("                           "));
        }

        void ProductionWgt::setInformationByPurpose(QWidget *panelBottom, std::vector<std::pair<std::string, std::string>> &setInformation,std::vector<std::string> &translated_infos)
        {
            if (!panelBottom)
                return;

            std::string str_format = "";

            for (auto& t : setInformation)
            {
//              std::cout << "pw setInformation:" << t.first << " " << t.second;
                if (t.first == "Format")
                {
                    str_format = t.second;
                    break;
                }
            }
            //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
            {
                SetInformation(panelBottom, setInformation,translated_infos);
                return;
            }
//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
            if (str_format.empty())
            {
                //setInformationOriginal(panelBottom, setInformation);
                setInformationBy3DMesh(panelBottom, setInformation);
                return;
            }

//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
            if (StringForProductionFormat.count(str_format) <= 0)
            {
                //setInformationOriginal(panelBottom, setInformation);
                setInformationBy3DMesh(panelBottom, setInformation);
                return;
            }

//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
            auto& format_id = StringForProductionFormat.at(str_format);
            if (PRODUCTION_MESH & format_id)
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
                setInformationBy3DMesh(panelBottom, setInformation);
            }
            else if (PRODUCTION_POINTCLOUD & format_id)
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
                setInformationBy3DPointCloud(panelBottom, setInformation);
            }
            else if (PRODUCTION_4D & format_id)
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
                setInformationBy4D(panelBottom, setInformation);
            }
            else
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << str_format << std::endl;
//              setInformationOriginal(panelBottom, setInformation);
                setInformationBy3DMesh(panelBottom, setInformation);
                return;
            }
        }

        // Purpose:3dmesh
        void ProductionWgt::setInformationBy3DMesh(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation)
        {
            if (!panelBottom)
                return;

            QVBoxLayout* vlPanelBottom = new QVBoxLayout();
            vlPanelBottom->setContentsMargins(29, 15, 29, 5);

            QHBoxLayout* hlBottomTitle = new QHBoxLayout();
            hlBottomTitle->setContentsMargins(0, 0, 0, 0);

            QLabel* lblBottomTitle = new QLabel(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblBottomTitle->setText("设置信息");
            }
            else
            {
                lblBottomTitle->setText("Setting Information");
            }

            lblBottomTitle->setStyleSheet("color:rgb(255,255,255);font:16px \"Arial\";padding:0px;margin:0px;");

            hlBottomTitle->addWidget(lblBottomTitle);
            hlBottomTitle->addStretch(1);

            QFrame* lineBottom = new QFrame(ui->widget);
            lineBottom->setFrameShape(QFrame::HLine);
            lineBottom->setFrameShadow(QFrame::Plain);
            lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;");

            QHBoxLayout* hlBottomBottomOne = new QHBoxLayout();
            hlBottomBottomOne->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomTwo = new QHBoxLayout();
            hlBottomBottomTwo->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomThree = new QHBoxLayout();
            hlBottomBottomThree->setContentsMargins(0, 0, 0, 0);

            lblID = new QLabel(panelBottom);
            lblID->setText("ID:");
            lblID->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblFormat = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblFormat->setText("格式：");
            }
            else
            {
                lblFormat->setText("Format:");
            }

            lblFormat->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblDestination = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblDestination->setText("输出目录：");
            }
            else
            {
                lblDestination->setText("Destination:");
            }

            lblDestination->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomOne->addWidget(lblID, 1);
            hlBottomBottomOne->addWidget(lblFormat, 1);
            hlBottomBottomOne->addWidget(lblDestination, 2);

            QLabel* lblIncludeTextureMaps = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblIncludeTextureMaps->setText("包含纹理贴图：");
            }
            else
            {
                lblIncludeTextureMaps->setText("Include texture maps:");
            }

            lblIncludeTextureMaps->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblTextureCompressionQuality = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTextureCompressionQuality->setText("纹理压缩质量：");
            }
            else
            {
                lblTextureCompressionQuality->setText("Texture compression quality:");
            }

            lblTextureCompressionQuality->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblMaximumTextureSize = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblMaximumTextureSize->setText("最大纹理尺寸：");
            }
            else
            {
                lblMaximumTextureSize->setText("Maximum texture size:");
            }

            lblMaximumTextureSize->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblTextureSharpeningEnabled = new QLabel(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTextureSharpeningEnabled->setText("纹理锐化：");
            }
            else
            {
                lblTextureSharpeningEnabled->setText("Texture sharpening:");
            }

            lblTextureSharpeningEnabled->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomTwo->addWidget(lblIncludeTextureMaps, 1);
            hlBottomBottomTwo->addWidget(lblTextureCompressionQuality, 1);
            hlBottomBottomTwo->addWidget(lblMaximumTextureSize, 1);
            hlBottomBottomTwo->addWidget(lblTextureSharpeningEnabled, 1);

            QLabel* lblCoordinatesOrigin = new QLabel(panelBottom);

            lblCoordinatesOrigin->setText("Origin:");
            lblCoordinatesOrigin->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblTypeOfLevelOfDetail = new QLabel(panelBottom);
            lblTypeOfLevelOfDetail->setText("Type of level of detail:");
            lblTypeOfLevelOfDetail->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            QLabel* lblScopeOfLevelOfDetail = new QLabel(panelBottom);
            lblScopeOfLevelOfDetail->setText("Scope of level of detail:");
            lblScopeOfLevelOfDetail->setStyleSheet("background-color:transparent;color:rgb(227,227,227);");

            QLabel* lblSpatialReferenceSystem = new QLabel(panelBottom);
            lblSpatialReferenceSystem->setText("Spatial Reference System:");
            lblSpatialReferenceSystem->setStyleSheet("background-color:transparent;color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomThree->addWidget(lblCoordinatesOrigin, 1);
            hlBottomBottomThree->addWidget(lblTypeOfLevelOfDetail, 1);
            hlBottomBottomThree->addWidget(lblScopeOfLevelOfDetail, 1);
            hlBottomBottomThree->addWidget(lblSpatialReferenceSystem, 1);

            vlPanelBottom->setSpacing(0);
            vlPanelBottom->addLayout(hlBottomTitle);
            vlPanelBottom->addSpacing(10);
            vlPanelBottom->addWidget(lineBottom);
            vlPanelBottom->addLayout(hlBottomBottomOne, 1);
            vlPanelBottom->addLayout(hlBottomBottomTwo, 1);
            vlPanelBottom->addLayout(hlBottomBottomThree, 1);
            vlPanelBottom->addStretch(1);

            QHBoxLayout* hlResubmit = new QHBoxLayout();
            butResubmitProduction = new QPushButton(panelBottom);
            butResubmitProduction->setText("Resubmit");
            hlResubmit->addStretch(1);
            hlResubmit->addWidget(butResubmitProduction);

            vlPanelBottom->addLayout(hlResubmit,1);

            panelBottom->setLayout(vlPanelBottom);

            std::string productionid_ = "";
            std::string format_ = "";
            std::string destination_ = "";

            std::string str_include_texture_maps = "";
            std::string str_texture_compression_quality = "";
            std::string str_maximum_texture_size = "";
            std::string str_texture_sharpening = "";

            std::string originstr_ = "";
            std::string lod_type_ = "";
            std::string scope_of_level_of_detail_ = "";
            std::string srs_str_ = "";

            for (auto& t : setInformation)
            {
                if (t.first == "Production ID")
                    productionid_ = t.second;
                else if (t.first == "Format")
                    format_ = t.second;
                else if (t.first == "Destination")
                    destination_ = t.second;
                else if (t.first == "Include texture maps")
                    str_include_texture_maps = t.second;
                else if (t.first == "Texture compression quality")
                    str_texture_compression_quality = t.second;
                else if (t.first == "Maximum texture size")
                    str_maximum_texture_size = t.second;
                else if (t.first == "Texture sharpening")
                    str_texture_sharpening = t.second;
                else if (t.first == "Origin")
                    originstr_ = t.second;
                else if (t.first == "Type of level of detail")
                    lod_type_ = t.second;
                else if (t.first == "Scope of level of detail")
                    scope_of_level_of_detail_ = t.second;
                else if (t.first == "Spatial Reference System")
                    srs_str_ = t.second;
            }
            
            lblID->setText(QString("Production ID: %1").arg(str2qstr(productionid_)));
            lblFormat->setText(QString("Format: %1").arg(str2qstr(format_)));
            lblDestination->setText(QString("Destination: %1").arg(str2qstr(destination_)));

            lblIncludeTextureMaps->setText(QString("Include texture maps: %1").arg(str2qstr(str_include_texture_maps)));
            lblTextureCompressionQuality->setText(QString("Texture compression quality: %1").arg(str2qstr(str_texture_compression_quality)));
            lblMaximumTextureSize->setText(QString("Maximum texture size: %1").arg(str2qstr(str_maximum_texture_size)));
            lblTextureSharpeningEnabled->setText(QString("Texture sharpening: %1").arg(str2qstr(str_texture_sharpening)));

            lblCoordinatesOrigin->setText(QString("Origin: %1").arg(str2qstr(originstr_)));
            lblTypeOfLevelOfDetail->setText(QString("Type of level of detail: %1").arg(str2qstr(lod_type_)));
            lblScopeOfLevelOfDetail->setText(QString("Scope of level of detail: %1").arg(str2qstr(scope_of_level_of_detail_)));
            if (!srs_str_.empty())
                lblSpatialReferenceSystem->setText(QString("Spatial Reference System: %1").arg(str2qstr(srs_str_)));
            else
                lblSpatialReferenceSystem->setText(QString("                         "));
        }

        void ProductionWgt::SetInformation(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation,std::vector<std::string> &translated_infos)
        {
            if (!panelBottom)
                return;
            if (setInformation.size() <= 3)
                return;
            QVBoxLayout* vlPanelBottom = new QVBoxLayout();
            vlPanelBottom->setContentsMargins(29, 15, 29, 5);

            QHBoxLayout* hlBottomTitle = new QHBoxLayout();
            hlBottomTitle->setContentsMargins(0, 0, 0, 0);

            QLabel* lblBottomTitle = new QLabel(ui->widget);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblBottomTitle->setText("设置信息");
            }
            else
            {
                lblBottomTitle->setText("Setting Information");
            }
            lblBottomTitle->setStyleSheet("color:rgb(255,255,255);font:16px \"Arial\";padding:0px;margin:0px;");

            hlBottomTitle->addWidget(lblBottomTitle);
            hlBottomTitle->addStretch(1);

            QFrame* lineBottom = new QFrame(ui->widget);
            lineBottom->setFrameShape(QFrame::HLine);
            lineBottom->setFrameShadow(QFrame::Plain);
            lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;");

            QHBoxLayout* hlBottomBottomOne = new QHBoxLayout();
            hlBottomBottomOne->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomTwo = new QHBoxLayout();
            hlBottomBottomTwo->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomThree = new QHBoxLayout();
            hlBottomBottomThree->setContentsMargins(0, 0, 0, 0);

            /*QHBoxLayout* hlBottomBottomFour = new QHBoxLayout();
            hlBottomBottomFour->setContentsMargins(0, 0, 0, 0);*/
            
            lblID = new QLabel(panelBottom);
            lblID->setText(QString("ID: %1").arg(str2qstr(setInformation[0].second)));
            //lblID->setText("ID:");
            lblID->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblFormat = new QLabel(panelBottom);
            //lblFormat->setText("Format:");
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblFormat->setText(QString("格式: %1").arg(str2qstr(setInformation[1].second)));
            }
            else
            {
                lblFormat->setText(QString("Format: %1").arg(str2qstr(setInformation[1].second)));
            }
            lblFormat->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblDestination = new QLabel(panelBottom);
            //lblDestination->setText("Destination");
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblDestination->setText(QString("输出目录: %1").arg(str2qstr(setInformation[2].second)));
            }
            else
            {
                lblDestination->setText(QString("Destination: %1").arg(str2qstr(setInformation[2].second)));
            }
            lblDestination->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomOne->addWidget(lblID, 1);
            hlBottomBottomOne->addWidget(lblFormat, 1);
            hlBottomBottomOne->addWidget(lblDestination, 2);

            int row = (setInformation.size()+1)/3;
            row = row < 3 ? 3: row;
            
            for (int i = 3; i < 6; i++)
            {
                QLabel* lbltemp = new QLabel(panelBottom);
                QString text = "                       ";
                if (i < setInformation.size())
                {
                    if (translated_infos.size() > 0 && i < translated_infos.size())
                    {
                    /// text = QString::fromStdString(translated_infos[i] + ": %1").arg(str2qstr(setInformation[i].second));
                        text = str2qstr(translated_infos[i] + ": %1").arg(str2qstr(setInformation[i].second));
                    }
                    else
                        text = QString::fromStdString(setInformation[i].first + ": %1").arg(str2qstr(setInformation[i].second));
                }
                
                lbltemp->setText(text);
                if (i == 5)
                {
                    hlBottomBottomTwo->addWidget(lbltemp, 2);
                }
                else
                {
                    hlBottomBottomTwo->addWidget(lbltemp, 1);
                }
            }
            
            for (int i = 6; i < 9; i++)
            {
                QLabel* lbltemp = new QLabel(panelBottom);
                QString text = "                       ";
                if (i < setInformation.size())
                {
                    if (translated_infos.size() > 0 && i < translated_infos.size())
                    {
                        ///text = QString::fromStdString(translated_infos[i] + ": %1").arg(str2qstr(setInformation[i].second));
                        text = str2qstr(translated_infos[i] + ": %1").arg(str2qstr(setInformation[i].second));
                    }
                    else
                        text = QString::fromStdString(setInformation[i].first + ": %1").arg(str2qstr(setInformation[i].second));
                }


                lbltemp->setText(text);
                if (i == 8)
                {
                    hlBottomBottomThree->addWidget(lbltemp, 2);
                }
                else
                {
                    hlBottomBottomThree->addWidget(lbltemp, 1);
                }
                
            }


            

            
            vlPanelBottom->setSpacing(0);
            vlPanelBottom->addLayout(hlBottomTitle);
            vlPanelBottom->addSpacing(15);
            vlPanelBottom->addWidget(lineBottom);
            vlPanelBottom->addLayout(hlBottomBottomOne, 1);
            vlPanelBottom->addLayout(hlBottomBottomTwo, 1);
            vlPanelBottom->addLayout(hlBottomBottomThree, 1);
            vlPanelBottom->addStretch(1);
            

            QHBoxLayout* hlResubmit = new QHBoxLayout();
            butResubmitProduction = new QPushButton(panelBottom);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                butResubmitProduction->setText("重新提交");
            }
            else
            {
                butResubmitProduction->setText("Resubmit");
            }
            hlResubmit->addStretch(1);
            hlResubmit->addWidget(butResubmitProduction);

            vlPanelBottom->addLayout(hlResubmit, 1);

            panelBottom->setLayout(vlPanelBottom);

            // set value for each label control based on setInformation vector.
            std::string productionid_ = "";
            std::string format_ = "";
            std::string destination_ = "";

            //Point sampling unit : pixel
            //Point sampling distance : 1
            std::string str_point_sampling_unit = "";
            std::string str_point_sampling_distance = "";
            std::string srs_str_ = "";

            for (auto& t : setInformation)
            {
            //  std::cout << "got " << t.first << " : " << t.second << std::endl;
                if (t.first == "Production ID")
                    productionid_ = t.second;
                else if (t.first == "Format")
                    format_ = t.second;
                else if (t.first == "Destination")
                    destination_ = t.second;
                else if (t.first == "Point sampling unit") ///
                    str_point_sampling_unit = t.second;
                else if (t.first == "Point sampling distance")
                    str_point_sampling_distance = t.second;
                else if (t.first == "Spatial Reference System") ///
                    srs_str_ = t.second;
                else
                {
            //      std::cout << "got/neq " << t.first << " : " << t.second << std::endl;
                }
            }

            /*if (productionid_ != "")
                lblID->setText(QString("ID: %1").arg(str2qstr(productionid_)));
            if (format_ != "")  lblFormat->setText(QString("Format: %1").arg(str2qstr(format_)));
            if (destination_ != "") lblDestination->setText(QString("Destination: %1").arg(str2qstr(destination_)));

            if (str_point_sampling_unit != "")
                lblPointSamplingDistance->setText(QString("Point sampling unit: %1").arg(str2qstr(str_point_sampling_unit)));
            if (str_point_sampling_distance != "")
                lblPointSamplingDistance->setText(QString("Point sampling distance: %1").arg(str2qstr(str_point_sampling_distance)));
            if (srs_str_ != "")
                lblSpatialReferenceSystem->setText(QString("Spatial Reference System: %1").arg(str2qstr(srs_str_)));
            else
                lblSpatialReferenceSystem->setText(QString("                         "));*/
        }

        // Purpose:3d pointlcoud
        void ProductionWgt::setInformationBy3DPointCloud(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation)
        {
            if (!panelBottom)
                return;

            QVBoxLayout* vlPanelBottom = new QVBoxLayout();
            vlPanelBottom->setContentsMargins(29, 15, 29, 5);

            QHBoxLayout* hlBottomTitle = new QHBoxLayout();
            hlBottomTitle->setContentsMargins(0, 0, 0, 0);

            QLabel* lblBottomTitle = new QLabel(ui->widget);
            lblBottomTitle->setText("Setting Information");
            lblBottomTitle->setStyleSheet("color:rgb(255,255,255);font:16px \"Arial\";padding:0px;margin:0px;");

            hlBottomTitle->addWidget(lblBottomTitle);
            hlBottomTitle->addStretch(1);

            QFrame* lineBottom = new QFrame(ui->widget);
            lineBottom->setFrameShape(QFrame::HLine);
            lineBottom->setFrameShadow(QFrame::Plain);
            lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;");

            QHBoxLayout* hlBottomBottomOne = new QHBoxLayout();
            hlBottomBottomOne->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomTwo = new QHBoxLayout();
            hlBottomBottomTwo->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomThree = new QHBoxLayout();
            hlBottomBottomThree->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomFour = new QHBoxLayout();
            hlBottomBottomFour->setContentsMargins(0, 0, 0, 0);

            lblID = new QLabel(panelBottom);
            lblID->setText("ID:");
            lblID->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblFormat = new QLabel(panelBottom);
            lblFormat->setText("Format:");
            lblFormat->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblDestination = new QLabel(panelBottom);
            lblDestination->setText("Destination");
            lblDestination->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomOne->addWidget(lblID, 1);
            hlBottomBottomOne->addWidget(lblFormat, 1);
            hlBottomBottomOne->addWidget(lblDestination, 2);

            QLabel* lblPointSamplingUnit = new QLabel(panelBottom);
            lblPointSamplingUnit->setText("Point sampling unit:");

            QLabel* lblPointSamplingDistance = new QLabel(panelBottom);
            lblPointSamplingDistance->setText("Point sampling distance:");

            QLabel* lblSpatialReferenceSystem = new QLabel(panelBottom);
            lblSpatialReferenceSystem->setText("Spatial Reference System:");

            hlBottomBottomTwo->addWidget(lblPointSamplingDistance, 1);
            hlBottomBottomTwo->addWidget(lblPointSamplingUnit, 1);
            hlBottomBottomTwo->addWidget(lblSpatialReferenceSystem, 2);

            vlPanelBottom->setSpacing(0);
            vlPanelBottom->addLayout(hlBottomTitle);
            vlPanelBottom->addSpacing(10);
            vlPanelBottom->addWidget(lineBottom);
            vlPanelBottom->addLayout(hlBottomBottomOne, 1);
            vlPanelBottom->addLayout(hlBottomBottomTwo, 1);
            vlPanelBottom->addStretch(1);

            QHBoxLayout* hlResubmit = new QHBoxLayout();
            butResubmitProduction = new QPushButton(panelBottom);
            butResubmitProduction->setText("Resubmit");
            hlResubmit->addStretch(1);
            hlResubmit->addWidget(butResubmitProduction);

            vlPanelBottom->addLayout(hlResubmit,1);

            panelBottom->setLayout(vlPanelBottom);

            // set value for each label control based on setInformation vector.
            std::string productionid_ = "";
            std::string format_ = "";
            std::string destination_ = "";

            //Point sampling unit : pixel
            //Point sampling distance : 1
            std::string str_point_sampling_unit = "";
            std::string str_point_sampling_distance = "";
            std::string srs_str_ = "";

            for (auto& t : setInformation)
            {
//              std::cout << "got " << t.first << " : " << t.second << std::endl;
                if (t.first == "Production ID")
                    productionid_ = t.second;
                else if (t.first == "Format")
                    format_ = t.second;
                else if (t.first == "Destination")
                    destination_ = t.second;
                else if (t.first == "Point sampling unit") ///
                    str_point_sampling_unit = t.second;
                else if (t.first == "Point sampling distance")
                    str_point_sampling_distance = t.second;
                else if (t.first == "Spatial Reference System") ///
                    srs_str_ = t.second;
                else
                {
//                  std::cout << "got/neq " << t.first << " : " << t.second << std::endl;
                }
            }

            if (productionid_ != "")
                lblID->setText(QString("ID: %1").arg(str2qstr(productionid_)));
            if (format_ != "")  lblFormat->setText(QString("Format: %1").arg(str2qstr(format_)));
            if (destination_ != "") lblDestination->setText(QString("Destination: %1").arg(str2qstr(destination_)));

            if (str_point_sampling_unit != "")
                lblPointSamplingDistance->setText(QString("Point sampling unit: %1").arg(str2qstr(str_point_sampling_unit)));
            if (str_point_sampling_distance != "")
                lblPointSamplingDistance->setText(QString("Point sampling distance: %1").arg(str2qstr(str_point_sampling_distance)));
            if (srs_str_ != "")
                lblSpatialReferenceSystem->setText(QString("Spatial Reference System: %1").arg(str2qstr(srs_str_)));
            else
                lblSpatialReferenceSystem->setText(QString("                         "));
        }

        // Purpose:TDOM/DSM
        void ProductionWgt::setInformationBy4D(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation)
        {
            if (!panelBottom)
                return;

            QVBoxLayout* vlPanelBottom = new QVBoxLayout();
            vlPanelBottom->setContentsMargins(29, 15, 29, 5);

            QHBoxLayout* hlBottomTitle = new QHBoxLayout();
            hlBottomTitle->setContentsMargins(0, 0, 0, 0);

            QLabel* lblBottomTitle = new QLabel(ui->widget);
            lblBottomTitle->setText("Setting Information");
            lblBottomTitle->setStyleSheet("color:rgb(255,255,255);font:16px \"Arial\";padding:0px;margin:0px;");

            hlBottomTitle->addWidget(lblBottomTitle);
            hlBottomTitle->addStretch(1);

            QFrame* lineBottom = new QFrame(ui->widget);
            lineBottom->setFrameShape(QFrame::HLine);
            lineBottom->setFrameShadow(QFrame::Plain);
            lineBottom->setStyleSheet("border:none;background-color:rgb(91,91,91);max-height:1px;padding:0px;margin:0px;");

            QHBoxLayout* hlBottomBottomOne = new QHBoxLayout();
            hlBottomBottomOne->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomTwo = new QHBoxLayout();
            hlBottomBottomTwo->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottomBottomThree = new QHBoxLayout();
            hlBottomBottomThree->setContentsMargins(0, 0, 0, 0);

            lblID = new QLabel(panelBottom);
            lblID->setText("ID:");
            lblID->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblFormat = new QLabel(panelBottom);
            lblFormat->setText("Format:");
            lblFormat->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            lblDestination = new QLabel(panelBottom);
            lblDestination->setText(tr("Destination"));
            lblDestination->setStyleSheet("color:rgb(227,227,227);font:14px \"Arial\";padding:0px;margin:0px;");

            hlBottomBottomOne->addWidget(lblID, 1);
            hlBottomBottomOne->addWidget(lblFormat, 1);
            hlBottomBottomOne->addWidget(lblDestination, 2);

            QLabel* lblSamplingDistance = new QLabel(panelBottom);
            lblSamplingDistance->setText("Sampling distance:");

            QLabel* lblMode = new QLabel(panelBottom);
            lblMode->setText("Mode:");

            QLabel* lblMaximumImagePartDimension = new QLabel(panelBottom);
            lblMaximumImagePartDimension->setText("Maximum image part dimension (px):");

            QLabel* lblOrthophotoEnabled = new QLabel(panelBottom);
            lblOrthophotoEnabled->setText("Orthophoto Enabled:");

            hlBottomBottomTwo->addWidget(lblSamplingDistance, 1);
            hlBottomBottomTwo->addWidget(lblMode, 1);
            hlBottomBottomTwo->addWidget(lblMaximumImagePartDimension, 1);
            hlBottomBottomTwo->addWidget(lblOrthophotoEnabled,1);

            QLabel* lblOrthophotoFormat = new QLabel(panelBottom);
            lblOrthophotoFormat->setText("Orthophoto Format:");

            QLabel* lblDSMEnabled = new QLabel(panelBottom);
            lblDSMEnabled->setText("DSM Enabled:");

            QLabel* lblDSMFormat = new QLabel(panelBottom);
            lblDSMFormat->setText("DSM Format:");

            QLabel* lblSpatialReferenceSystem = new QLabel(panelBottom);
            lblSpatialReferenceSystem->setText("Spatial Reference System:");

            hlBottomBottomThree->addWidget(lblOrthophotoFormat, 1);
            hlBottomBottomThree->addWidget(lblDSMEnabled, 1);
            hlBottomBottomThree->addWidget(lblDSMFormat, 1);
            hlBottomBottomThree->addWidget(lblSpatialReferenceSystem, 1);

            vlPanelBottom->setSpacing(0);
            vlPanelBottom->addLayout(hlBottomTitle);
            vlPanelBottom->addSpacing(10);
            vlPanelBottom->addWidget(lineBottom);
            vlPanelBottom->addLayout(hlBottomBottomOne, 1);
            vlPanelBottom->addLayout(hlBottomBottomTwo, 1);
            vlPanelBottom->addLayout(hlBottomBottomThree, 1);
            vlPanelBottom->addStretch(1);

            QHBoxLayout* hlResubmit = new QHBoxLayout();
            butResubmitProduction = new QPushButton(panelBottom);
            butResubmitProduction->setText("Resubmit");
            hlResubmit->addStretch(1);
            hlResubmit->addWidget(butResubmitProduction);

            vlPanelBottom->addLayout(hlResubmit, 1);

            panelBottom->setLayout(vlPanelBottom);

            // set value for each label control based on setInformation vector.
            std::string productionid_ = "";
            std::string format_ = "";
            std::string destination_ = "";

            std::string str_sampling_distance = "";
            std::string str_mode = "";
            std::string str_maximum_image_part_dimension = "";
            std::string str_tdom_enabled = "";

            std::string str_orthophoto_format = "";
            std::string str_dsm_enabled = "";
            std::string str_dsm_format = "";
            std::string srs_str_ = "";

            /*
            Production ID : Production_2(v)
                Format : Orthophoto / DSM(v)
                Destination : D : / jiaojie / test / yanshou / yanshou / Productions / Production_2(v)

                Sampling distance : 0.0031   （w）
                Mode : RapidMoscaic
                Maximum image part dimension(px) : 4096

                Orthophoto Enabled : true (w)
                Orthophoto Format : TIFF / GeoTIFF
                DSM Enabled : true (w)

                DSM Format : TIFF / GeoTIFF
                */

            for (auto& t : setInformation)
            {
//              std::cout << "got " << t.first << " : " << t.second << std::endl;
                if (t.first == "Production ID")
                    productionid_ = t.second;
                else if (t.first == "Format")
                    format_ = t.second;
                else if (t.first == "Destination")
                    destination_ = t.second;
                else if (t.first == "Sampling distance") ///
                    str_sampling_distance = t.second;
                else if (t.first == "Mode")
                    str_mode = t.second;
                else if (t.first == "Maximum image part dimension")
                    str_maximum_image_part_dimension = t.second;
                else if (t.first == "Orthophoto enabled") ///
                    str_tdom_enabled = t.second;
                else if (t.first == "Orthophoto Format") 
                    str_orthophoto_format = t.second;
                else if (t.first == "DSM enabled") ///
                    str_dsm_enabled = t.second;
                else if (t.first == "DSM Format") 
                    str_dsm_format = t.second;
                else if (t.first == "Spatial Reference System") ///
                    srs_str_ = t.second;
                else
                {
//                  std::cout << "got/neq " << t.first << " : " << t.second << std::endl;
                }
            }

            if (productionid_ != "")
                lblID->setText(QString("ID: %1").arg(str2qstr(productionid_)));
            if (format_ != "")  lblFormat->setText(QString("Format: %1").arg(str2qstr(format_)));
            if (destination_ != "") lblDestination->setText(QString("Destination: %1").arg(str2qstr(destination_)));

            if (str_sampling_distance != "")
                lblSamplingDistance->setText(QString("Sampling distance: %1").arg(str2qstr(str_sampling_distance)));
            if (str_mode != "")
                lblMode->setText(QString("Mode: %1").arg(str2qstr(str_mode)));
            if (str_maximum_image_part_dimension != "")
                lblMaximumImagePartDimension->setText(QString("Maximum image part dimension (px): %1").arg(str2qstr(str_maximum_image_part_dimension)));
            if (str_tdom_enabled != "")
                lblOrthophotoEnabled->setText(QString("Orthophoto Enabled: %1").arg(str2qstr(str_tdom_enabled)));

            if (str_orthophoto_format != "")
                lblOrthophotoFormat->setText(QString("Orthophoto Format: %1").arg(str2qstr(str_orthophoto_format)));
            if (str_dsm_enabled != "")
                lblDSMEnabled->setText(QString("DSM enabled: %1").arg(str2qstr(str_dsm_enabled)));
            if (str_dsm_format != "")
                lblDSMFormat->setText(QString("DSM Format: %1").arg(str2qstr(str_dsm_format)));
            if (srs_str_ != "")
                lblSpatialReferenceSystem->setText(QString("Spatial Reference System: %1").arg(str2qstr(srs_str_)));
            else
                lblSpatialReferenceSystem->setText(QString("                         "));
        }

        void ProductionWgt::InitProductionItemInfo()
        {
            vecTile.clear();
            vecProductionItemInfo.clear();
            mapProductionItemInfo.clear();

            if (block_data_ && recons_object_ && production_object_)
            {
                ReconstructionCommandSet::SyncProductionTileJobStrs(
                    block_data_, recons_object_, production_object_);
            }

            bProductionItemInfoNeedRendering = false;

            int nPendingNum = 0;
            int nRunningNum = 0;
            int nCompletedNum = 0;
            int nCancelledNum = 0;
            int nFailedNum = 0;
            int nUnknownNum = 0;

            for (auto& tile_ : production_object_->GetOrderedTiles())
            {
                ProductionItemInfo productionItemInfo;

                auto& tile = production_object_->GetTilesMutual().at(tile_);
                productionItemInfo.name_ = tile.name_;
                productionItemInfo.jobFileName_ = ReconstructionCommandSet::ResolveProductionTileJobStr(
                    this->block_data_, this->recons_object_, this->production_object_, tile_, true);

                productionItemInfo.initJobStat_ = tile.status_;
                productionItemInfo.lastJobStat_ = tile.status_;
                //productionItemInfo.jobStat_ = tile.status_;

                switch (tile.status_)
                {
                case jobsta_e::STATUS_PENDDING:
                    nPendingNum++;
                    break;
                case jobsta_e::STATUS_RUNNING:
                    nRunningNum++;
                    break;
                case jobsta_e::STATUS_COMPLETE:
                    nCompletedNum++;
                    break;
                case jobsta_e::STATUS_CANCLE:
                    nCancelledNum++;
                    break;
                case jobsta_e::STATUS_FAILURE:
                    nFailedNum++;
                    break;
                default:
                    nUnknownNum++;
                    break;
                }

                productionItemInfo.lastProgress = -1;
                //productionItemInfo.currentProgress = -1;

                productionItemInfo.feedbackFile_ = ReconstructionCommandSet::GenerateTileFeedbackFile(this->block_data_, this->production_object_, tile.name_, tile.jobstr_);

                productionItemInfo.needRefresh_ = false;

                vecTile.push_back(tile_);
                vecProductionItemInfo.push_back(productionItemInfo);
                mapProductionItemInfo.insert(std::make_pair(tile_, productionItemInfo));

                // calculate percent and taskStatus periodically based on latest feedback file if available.
                // no need to refresh the relevant control at this time.
                if (tile.status_ >= jobsta_e::STATUS_PENDDING && tile.status_ <= jobsta_e::STATUS_FAILURE)
                {
                    std::ostringstream oss;
                    oss << "inside "  << " " << __LINE__ << " " << tile_ << " / " << tile.name_ <<
                        " job:" << tile.jobstr_ << " " << tile.status_ << " feedbackfile:" << productionItemInfo.feedbackFile_;
                    LOGI(oss.str());
                }
            }

            {
                std::ostringstream oss;
                oss << "inside "  << " " << __LINE__ << " pending:" << nPendingNum
                    << " running:" << nRunningNum << " completed:" << nCompletedNum
                    << " cancelled:" << nCancelledNum << " failed:" << nFailedNum << " unknown:" << nUnknownNum;

                LOGI(oss.str());
            }

            bForceRefreshAll = true;
        }

        void ProductionWgt::RefreshTileRow(std::string& tile,bool bInsertNewRow)
        {

        }

        void ProductionWgt::RefreshProductionItemInfo()
        {
            if (/*!bForceRefreshAll &&*/ (!bProductionItemInfoNeedRendering || vecTile.size() <= 0))
                return;

            int i = 0;
            int nTotalTiles = 0;
            int nCompletedTiles = 0;
            int nPendingTiles = 0;
            int nRunningTiles = 0;
            int nFailedTiles = 0;

            int nCancelledTiles = 0;
            int nUnknownTiles = 0;
            int nRefreshItemNum = 0;

            nTotalTiles = vecTile.size();

            if (!bProductionItemInfoFirstRendered)
            {
                // clear production list data inside relevant tablewidget container when needed.
                for (auto& tile_ : vecTile)
                {
                    twProductionList->insertRow(twProductionList->rowCount());
                    int lastRow = twProductionList->rowCount() - 1;

                    if (lastRow >= vecTile.size())
                        return;

                    ///std::string tile_ = vecTile.at(lastRow);
                    ProductionItemInfo& productionItemInfo = mapProductionItemInfo.at(tile_);

                    for (int j = 0; j < twProductionList->columnCount(); j++)
                    {
                        if (j == 2)
                        {
                            ProgBarContainer* pProgBarContainer = nullptr;
                            pProgBarContainer = new ProgBarContainer(ui->widget);

                            if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_COMPLETE)
                            {
                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("100%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}");
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                nCompletedTiles++;

                                {
                                    std::ostringstream oss;
                                    oss << "Refresh:completed.";
                //                  LOGI(oss.str());
                                }
                            }
                            else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_FAILURE)
                            {
                                int percent = productionItemInfo.lastProgress;
                                if (percent < 1)
                                    percent = 1;
                                pProgBarContainer->pProgBar->setValue(percent);
                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                nFailedTiles++;

                                {
                                    std::ostringstream oss;
                                    oss << "Refresh:failed.";
                    //              LOGI(oss.str());
                                }
                            }
                            else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_CANCLE)
                            {
                                int percent = productionItemInfo.lastProgress;
                                if (percent < 1)
                                    percent = 0;
                                pProgBarContainer->pProgBar->setValue(percent);
                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61, 64, 70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(247, 186, 10);}");
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(247, 186, 10);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                nCancelledTiles++;
                                {
                                    std::ostringstream oss;
                                    oss << "Refresh:Cancelled.";
                        //          LOGI(oss.str());
                                }
                            }
                            else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_RUNNING)
                            {
                                int percent = productionItemInfo.lastProgress;
                                if (percent < 1)
                                    percent = 0;
                                pProgBarContainer->pProgBar->setValue(percent);

                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));

                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                nRunningTiles++;

                                {
                                    std::ostringstream oss;
                                    oss << "Refresh:running.";
                            //      LOGI(oss.str());
                                }
                            }
                            else
                            {
                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("0%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                nPendingTiles++;

                                {
                                    std::ostringstream oss;
                                    oss << "Refresh:pending.";
                                //  LOGI(oss.str());
                                }
                            }

                            twProductionList->setCellWidget(lastRow, j, pProgBarContainer);
                        }
                        else
                        {
                            QTableWidgetItem* pItem = nullptr;
                            pItem = new QTableWidgetItem;
                            pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                            if (j == 0)
                                pItem->setText(str2qstr(tile_));
                            else if (j == 1)
                            {
                                switch (productionItemInfo.lastJobStat_)
                                {
                                case jobsta_e::STATUS_COMPLETE:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("完成");
                                    }
                                    else
                                    {
                                        pItem->setText("Completed");
                                    }

                                    break;
                                case jobsta_e::STATUS_FAILURE:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("失败");
                                    }
                                    else
                                    {
                                        pItem->setText("Failed");
                                    }

                                    break;
                                case jobsta_e::STATUS_CANCLE:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("取消");
                                    }
                                    else
                                    {
                                        pItem->setText("Cancelled");
                                    }

                                    break;
                                case jobsta_e::STATUS_RUNNING:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("运行");
                                    }
                                    else
                                    {
                                        pItem->setText("Running");
                                    }

                                    break;
                                default:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("等待中");
                                    }
                                    else
                                    {
                                        pItem->setText("Pending");
                                    }

                                    break;
                                }
                            }
                            else if (j == 3)
                            {
                                // display message while tile status is failed or cancelled.
                                switch (productionItemInfo.lastJobStat_)
                                {
                                case jobsta_e::STATUS_FAILURE:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        if (!productionItemInfo.Msg.empty())
                                            pItem->setText("失败: " + QString::fromStdString(productionItemInfo.Msg));
                                        else
                                            pItem->setText("失败");

                                    }
                                    else
                                    {
                                        if (!productionItemInfo.Msg.empty())
                                            pItem->setText("Failed: " + QString::fromStdString(productionItemInfo.Msg));
                                        else
                                            pItem->setText("Failed");

                                    }
                                    break;
                                case jobsta_e::STATUS_CANCLE:
                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        if (!productionItemInfo.Msg.empty())
                                            pItem->setText("取消: " + QString::fromStdString(productionItemInfo.Msg));
                                        else
                                            pItem->setText("取消");

                                    }
                                    else
                                    {
                                        if (!productionItemInfo.Msg.empty())
                                            pItem->setText("Cancelled: " + QString::fromStdString(productionItemInfo.Msg));
                                        else
                                            pItem->setText("Cancelled");

                                    }
                                    break;
                                default:
                                    pItem->setText("--");
                                    break;
                                }
                            }
                            else if (j == 4)
                            {
                                if (productionItemInfo.submitTime_.empty())
                                {
                                    pItem->setText("--");
                                }
                                else
                                {
                                    auto timestr = QString::fromStdString(productionItemInfo.submitTime_);
                                    QString qSubmitTime = QDateTime::fromString(timestr, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
                                    pItem->setText(qSubmitTime);
                                }
                            }

                            twProductionList->setItem(lastRow, j, pItem);
                        }
                    }

                    // note:make further optimization for the following line later.
                    twProductionList->resizeRowToContents(lastRow);
                }
            }
            else if (vecTile.size() == twProductionList->rowCount())
            {
                int lastRow = 0;
                for (auto& tile_ : vecTile)
                {
                    if (lastRow >= vecTile.size())
                        return;

                    ProductionItemInfo& productionItemInfo = mapProductionItemInfo.at(tile_);
                    if (bForceRefreshAll || productionItemInfo.needRefresh_)
                    {
                        //if (bForceRefreshAll)
                        {
                            std::ostringstream oss;
                            oss << "Refresh/bForceRefreshAll: " << bForceRefreshAll;
                            //LOGI(oss.str());
                        }

                        for (int j = 0; j < twProductionList->columnCount(); j++)
                        {
                            if (j == 2)
                            {
                                ProgBarContainer* pProgBarContainer = nullptr;
                                pProgBarContainer = (ProgBarContainer*)twProductionList->cellWidget(lastRow, j);

                                if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_COMPLETE)
                                {
                                    pProgBarContainer->pProgBar->setValue(0);
                                    pProgBarContainer->pLblProg->setText("100%");
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}");
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                    nCompletedTiles++;

                                    {
                                        std::ostringstream oss;
                                        oss << "Refresh2:completed.";
                                    //  LOGI(oss.str());
                                    }
                                }
                                else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_FAILURE)
                                {
                                    int percent = productionItemInfo.lastProgress;
                                    if (percent < 1)
                                        percent = 1;
                                    pProgBarContainer->pProgBar->setValue(percent);
                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                    nFailedTiles++;
                                    {
                                        std::ostringstream oss;
                                        oss << "Refresh2:failed.";
                                    //  LOGI(oss.str());
                                    }
                                }
                                else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_CANCLE)
                                {
                                    int percent = productionItemInfo.lastProgress;
                                    if (percent < 1)
                                        percent = 0;
                                    pProgBarContainer->pProgBar->setValue(percent);
                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61, 64, 70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(247, 186, 10);}");
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(247, 186, 10);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
                                    nCancelledTiles++;
                                    {
                                        std::ostringstream oss;
                                        oss << "Refresh2:cancelled.";
                                    //  LOGI(oss.str());
                                    }
                                }
                                else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_RUNNING)
                                {
                                    int percent = productionItemInfo.lastProgress;
                                    if (percent < 1)
                                        percent = 0;
                                    pProgBarContainer->pProgBar->setValue(percent);

                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));

                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    nRunningTiles++;
                                    {
                                        std::ostringstream oss;
                                        oss << "Refresh2:running.";
//                                      LOGI(oss.str());
                                    }
                                }
                                else
                                {
                                    pProgBarContainer->pProgBar->setValue(0);
                                    pProgBarContainer->pLblProg->setText("0%");
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    );
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    nPendingTiles++;
                                    {
                                        std::ostringstream oss;
                                        oss << "Refresh2:pending.";
//                                      LOGI(oss.str());
                                    }
                                }
                            }
                            else
                            {
                                QTableWidgetItem* pItem = nullptr;
                                pItem = twProductionList->item(lastRow, j);

                                if (j == 0)
                                    pItem->setText(str2qstr(tile_));
                                else if (j == 1)
                                {
                                    switch (productionItemInfo.lastJobStat_)
                                    {
                                    case jobsta_e::STATUS_COMPLETE:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("完成");
                                        }
                                        else
                                        {
                                            pItem->setText("Completed");
                                        }

                                        break;
                                    case jobsta_e::STATUS_FAILURE:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("失败");
                                        }
                                        else
                                        {
                                            pItem->setText("Failed");
                                        }
                                        break;
                                    case jobsta_e::STATUS_CANCLE:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("取消");
                                        }
                                        else
                                        {
                                            pItem->setText("Cancelled");
                                        }

                                        break;
                                    case jobsta_e::STATUS_RUNNING:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("运行");
                                        }
                                        else
                                        {
                                            pItem->setText("Running");
                                        }

                                        break;
                                    default:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("等待中");
                                        }
                                        else
                                        {
                                            pItem->setText("Pending");
                                        }

                                        break;
                                    }
                                }
                                else if (j == 3)
                                {
                                    // display message while tile status is failed or cancelled.
                                    switch (productionItemInfo.lastJobStat_)
                                    {
                                    case jobsta_e::STATUS_FAILURE:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("失败: " + QString::fromStdString(productionItemInfo.Msg));
                                        }
                                        else
                                        {
                                            pItem->setText("Failed: " + QString::fromStdString(productionItemInfo.Msg));
                                        }

                                        break;
                                    case jobsta_e::STATUS_CANCLE:
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("取消: " + QString::fromStdString(productionItemInfo.Msg));
                                        }
                                        else
                                        {
                                            pItem->setText("Cancelled: " + QString::fromStdString(productionItemInfo.Msg));
                                        }

                                        break;
                                    default:
                                        pItem->setText("--");
                                        break;
                                    }
                                }
                                else if (j == 4)
                                {
                                    if (productionItemInfo.submitTime_.empty())
                                    {
                                        pItem->setText("--");
                                    }
                                    else
                                    {
                                        auto timestr = QString::fromStdString(productionItemInfo.submitTime_);
                                        QString qSubmitTime = QDateTime::fromString(timestr, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
                                        pItem->setText(qSubmitTime);
                                    }
                                }
                            }
                        }
                        nRefreshItemNum++;
                    }
                    else
                    {
                        if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_COMPLETE)
                        {
                            nCompletedTiles++;
                            {
                                std::ostringstream oss;
                                oss << "Refresh3:completed.";
                                //LOGI(oss.str());
                            }
                        }
                        else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_FAILURE)
                        {
                            nFailedTiles++;
                            {
                                std::ostringstream oss;
                                oss << "Refresh3:failed.";
                            //  LOGI(oss.str());
                            }
                        }
                        else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_CANCLE)
                        {
                            nCancelledTiles++;
                            {
                                std::ostringstream oss;
                                oss << "Refresh3:cancelled.";
                            //  LOGI(oss.str());
                            }
                        }
                        else if (productionItemInfo.lastJobStat_ == jobsta_e::STATUS_RUNNING)
                        {
                            nRunningTiles++;
                            {
                                std::ostringstream oss;
                                oss << "Refresh3:running.";
                            //  LOGI(oss.str());
                            }
                        }
                        else
                        {
                            nPendingTiles++;
                            {
                                std::ostringstream oss;
                                oss << "Refresh3:pending." << productionItemInfo.lastJobStat_;
                            //  LOGI(oss.str());
                            }
                        }
                    }

                    lastRow++;
                }
            }
            else
            {
                // impossible to reach here.
                std::ostringstream oss;
                oss << " error:" << vecTile.size() << " / " << twProductionList->rowCount();
                LOGI(oss.str());
            }

            //for (auto& t : mapProductionItemInfo)
            //{
            //  auto tile_ = t.first;
            //  //ProductionItemInfo& productionItemInfo = const_cast<ProductionItemInfo&>(mapProductionItemInfo.at(tile_));
            //  ProductionItemInfo& productionItemInfo = t.second;
            //  if (bProductionItemInfoFirstRendered)
            //  {
            //      // insert new row and set item data for all fields of current new row(tile).
            //      RefreshTileRow(tile_, true);
            //  }
            //  else if(productionItemInfo.needRefresh_)
            //  {
            //      // set item data for some specific fields of current row while new data for current tile is available.
            //      RefreshTileRow(tile_, false);
            //  }

            //  i++;
            //  if (i >= twProductionList->rowCount())
            //      break;
            //}

            bProductionItemInfoFirstRendered = true;
            bForceRefreshAll = false;

            for (auto& t : mapProductionItemInfo)
            {
                t.second.needRefresh_ = false;
            }

            bool  bcancelled = false;
            bool bcompleted = false;
            bool bpending = false;
            bool bfailed = false;
            bool brunning = false;
            if (nCancelledTiles > 0)
                bcancelled = true;
            if (nRunningTiles > 0)
                brunning = true;
            //@attetion  此处最主要的逻辑是nCompletedTiles > 0 && nTotalTiles == nCompletedTiles
            //但是有时候可能会出现两者不相等，原因待查，pending 同理所以逻辑暂定如下；
            if (nCompletedTiles > 0 && (nTotalTiles >= nCompletedTiles) &&
                nPendingTiles == 0 && nFailedTiles == 0)
            {
                bcompleted = true;
            }
            if (nPendingTiles > 0 && (nTotalTiles >= nPendingTiles) &&
                nRunningTiles == 0 && nFailedTiles == 0)
            {
                bpending = true;
            }

            //@add by chy ：失败只有所有状态为终态(cancle or  complete failed)时，如果有一个failede则为failed;
            if (nPendingTiles == 0 && nRunningTiles == 0 && nFailedTiles > 0)
            {
                bfailed = true;
            }

            if (bcancelled)
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_cancel.png"));
                lblTopLeft->show();

                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    lblTopRightTop->setText("取消");
                    lblTopRightBottom->setText(QString("%1/%2 块已完成.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }
                else
                {
                    lblTopRightTop->setText("Cancelled");
                    lblTopRightBottom->setText(QString("%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }

                cpwLeft->hide();
            }
            else if (brunning)
            {
                int percent = nCompletedTiles * 100 / nTotalTiles;

                cpwLeft->setPercent(percent);
                cpwLeft->show();

                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    lblTopRightTop->setText("运行中");
                    lblTopRightBottom->setText(QString("%1/%2 块已完成.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }
                else
                {
                    lblTopRightTop->setText("Running");
                    lblTopRightBottom->setText(QString("%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }

                lblTopLeft->hide();
            }
            else if (bpending)
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_wait.png"));
                lblTopLeft->show();

                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    lblTopRightTop->setText("等待中");
                    lblTopRightBottom->setText(QString("生产等待中，%1/%2 块已完成.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }
                else
                {
                    lblTopRightTop->setText("Pending");
                    lblTopRightBottom->setText(QString("Production is pending,%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                        .arg(nTotalTiles));

                }
                //lblTopRightBottom->setText("Production submitted,waiting to run");

                cpwLeft->hide();
            }
            else if (bcompleted)
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_succ.png"));
                lblTopLeft->show();

                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    lblTopRightTop->setText("完成");
                    //lblTopRightBottom->setText("Processing time:27min 42s");
                    lblTopRightBottom->setText(QString("%1/%2 任务已完成").arg(nCompletedTiles).arg(nTotalTiles));

                }
                else
                {
                    lblTopRightTop->setText("Completed");
                    //lblTopRightBottom->setText("Processing time:27min 42s");
                    lblTopRightBottom->setText(QString("%1/%2 job(s) completed.").arg(nCompletedTiles).arg(nTotalTiles));

                }

                cpwLeft->hide();

                if (this->production_object_)
                {
                    this->production_object_->SetCompleted();

                    if (refresh_timer_)
                        refresh_timer_->stop();
                }
            }
            else if (bfailed)//>0
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_fail.png"));
                lblTopLeft->show();

                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    lblTopRightTop->setText("失败");
                    //lblTopRightBottom->setText("Failure reason: xxxxxx.");
                    // 6 / 43 tasks completed ；4 errors
                    lblTopRightBottom->setText(QString("%1/%2 任务已完成，%3 失败.").arg(nCompletedTiles).arg(nTotalTiles).arg(nFailedTiles));

                }
                else
                {
                    lblTopRightTop->setText("Fail");
                    //lblTopRightBottom->setText("Failure reason: xxxxxx.");
                    // 6 / 43 tasks completed ；4 errors
                    lblTopRightBottom->setText(QString("%1/%2 job(s) completed,%3 failed.").arg(nCompletedTiles).arg(nTotalTiles).arg(nFailedTiles));

                }

                cpwLeft->hide();
            }
            else
            {

                // impossible to reach here.even if coming here,it should be completed state.
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_wait.png"));
                lblTopLeft->show();
                cpwLeft->hide();
            }
            if (0)
            {
#if 0
                // encounter exception.
                if (ReconstructionCommandSet::CanCancelProduction(*this->recons_object_, this->production_object_->GetId()))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
#else
                if (nCancelledTiles <= 0 && (nPendingTiles + nRunningTiles > 0))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
#endif

#if 0
                if (ReconstructionCommandSet::CanResubmitProduction(*this->recons_object_, this->production_object_->GetId()))

                {
                    butResubmitProduction->setVisible(true);
                }
                else
                {
                    butResubmitProduction->setVisible(false);
                }
#else
                if ((nCancelledTiles + nFailedTiles) > 0)
                {
                    if (butResubmitProduction)
                        butResubmitProduction->setVisible(true);
                }
                else
                {
                    if (butResubmitProduction)
                        butResubmitProduction->setVisible(false);
                }
#endif
            }
            else //chy modified by chy @20231215
            {
                //基本逻辑是：cancle和resubmit是互斥的；
                //cancel出现的逻辑是：
                //1：点cancel后的情形resubmit：cancelled、failed、complete三者均有，或者全为cancelled或着failed的，绝不能全是complete
                if (nCancelledTiles <= 0 && (nPendingTiles + nRunningTiles > 0))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
                if (butCancelProduction->isVisible() || nCompletedTiles == nTotalTiles)
                {
                    if (butResubmitProduction)
                        butResubmitProduction->setVisible(false);
                }
                else
                {
                    if (butResubmitProduction)
                        butResubmitProduction->setVisible(true);
                }
            }

            {
                std::ostringstream oss;
                oss << "refresh num:" << nRefreshItemNum << " / " << nTotalTiles;
//              LOGI(oss.str());
//              std::cout << oss.str() << std::endl;
            }
        }

        void ProductionWgt::GetProductionItemInfo()
        {
            bProductionItemInfoNeedRendering = false;
            bProductionItemInfoGetting = true;
            bProductionItemInfoGot = false;

            {
                std::ostringstream oss;
                oss << __FUNCTION__ << " " << __LINE__ << " " << mapProductionItemInfo.size();
                //LOGI(oss.str());
            }

            for (auto& t : mapProductionItemInfo)
            {
                auto tile_ = t.first;
                //ProductionItemInfo& productionItemInfo = const_cast<ProductionItemInfo &>(mapProductionItemInfo.at(tile_));
                ProductionItemInfo& productionItemInfo = t.second;

                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__ << " " << mapProductionItemInfo.size() << " " << tile_;
        //          LOGI(oss.str());
                }

                if (bDestroying || bResubmitting)
                    break;

                if (t.second.lastJobStat_ == jobsta_e::STATUS_COMPLETE)
                {
                    // primarily for optimization purpose.
                    // just to fill or set the necessary data field(s) and not need to check relevant feedback file if completed.
                    if (!bProductionItemInfoFirstRendered)
                    {
                            productionItemInfo.lastProgress = 100;

                            // note: need to clear needRefresh flag for each productionItemInfo inside refresh routine.
                            productionItemInfo.needRefresh_ = true;

                            bProductionItemInfoNeedRendering = true;

                            {
                                std::ostringstream oss;
                                oss << __FUNCTION__ << " " << __LINE__;
//                              LOGI(oss.str());
                            }
                    }
                    else
                    {
                        // just to skip any further action for no necessity.
                        {
                            std::ostringstream oss;
                            oss << __FUNCTION__ << " " << __LINE__;
    //                      LOGI(oss.str());
                        }
                    }
                }
                else if (!t.second.feedbackFile_.empty())
                {
                    // note: write back tile status if feedback_file has newer tile status than previous saved state.
                    JobFeedBack_s feedBack;
                    {
                        std::ostringstream oss;
                        oss << __FUNCTION__ << " " << __LINE__;
                        //LOGI(oss.str());
                    }
                    bool bRet = feedBack.load_with_retry2(t.second.feedbackFile_);
                    if (bRet)
                    {
                        // percent,job status.
                        auto& tile = this->production_object_->GetTilesMutual().at(tile_);
                        tile.status_ = feedBack.Status;

                        if (tile.status_ == jobsta_e::STATUS_COMPLETE)
                        {
                            //add by chy 后续需要提出一个接口专门用于更新reconstruction下的tile的状态的
                            //this->recons_object_->GetTilesBaseMutual().at(iter).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                            this->recons_object_->GetTilesCustomMutual().at(tile_).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                            //                          std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                            //                              << iter << " " << feedback_file_ << std::endl;
                        }


                        if (!bProductionItemInfoFirstRendered)
                        {
                            productionItemInfo.initJobStat_ = feedBack.Status;
                            productionItemInfo.lastJobStat_ = feedBack.Status;
                            //productionItemInfo.jobStat_ = feedBack.Status;
                            productionItemInfo.Msg = feedBack.Msg;
                            productionItemInfo.lastProgress = (int)feedBack.Percent;
                            //productionItemInfo.currentProgress = (int)feedBack.Percent;
                            if (feedBack.Status == jobsta_e::STATUS_COMPLETE)
                            {
                                productionItemInfo.lastProgress = 100;
                                //productionItemInfo.currentProgress = 100;
                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__;
    //                              LOGI(oss.str());
                                }
                            }
                            else
                            {
                                productionItemInfo.lastProgress = (int)(feedBack.Percent + 0.5);
                                //productionItemInfo.currentProgress = (int)(feedBack.Percent + 0.5);
                                if (productionItemInfo.lastProgress == 100)
                                {
                                    //productionItemInfo.lastProgress = 99;
                                    //productionItemInfo.currentProgress = 99;
                                    productionItemInfo.lastProgress = (int)(feedBack.Percent);
                                }
                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__;
        //                          LOGI(oss.str());
                                }
                            }

                            std::string feedbackname = AI3D::CORE::File::GetFileNameWithoutExtension(t.second.feedbackFile_);
                            auto strs = AI3D::CORE::String::StringSplit(feedbackname, "_");
                            if (strs.size() >= 3)
                            {
                                productionItemInfo.submitTime_ = strs[2];
                            }
                            else
                            {
                                productionItemInfo.submitTime_ = "";
                            }

                            productionItemInfo.needRefresh_ = true;
                            bProductionItemInfoNeedRendering = true;
                            {
                                std::ostringstream oss;
                                oss << __FUNCTION__ << " " << __LINE__;
            //                  LOGI(oss.str());
                            }

                        }
                        else
                        {
                            int feedBack_percent = 0;
                            if (feedBack.Status == jobsta_e::STATUS_COMPLETE)
                            {
                                //add by chy 后续需要提出一个接口专门用于更新reconstruction下的tile的状态的
                        ///     this->recons_object_->GetTilesCustomMutual().at(tile_).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                                feedBack_percent = 100;
                            }
                            else
                            {
                                feedBack_percent = (int)(feedBack.Percent + 0.5);
                                if (feedBack_percent == 100)
                                {
                                    //feedBack_percent = 99;
                                    feedBack_percent = (int)(feedBack.Percent);
                                }
                            }

                            if (productionItemInfo.lastJobStat_ != feedBack.Status || productionItemInfo.lastProgress != feedBack_percent)
                            {
                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__ << " " << productionItemInfo.lastJobStat_ << " / " << feedBack.Status
                                        << productionItemInfo.lastProgress << " / " << feedBack_percent;
                //                  LOGI(oss.str());
                                }

                                productionItemInfo.lastJobStat_ = feedBack.Status;
                                productionItemInfo.lastProgress = feedBack_percent;
                                productionItemInfo.Msg = feedBack.Msg;
                                productionItemInfo.needRefresh_ = true;
                                bProductionItemInfoNeedRendering = true;

                                if (!bDestroying)
                                {
                                    // write back current tile status into relevant data structure such as production object,etc.
                                }
                            }
                            else
                            {
                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__;
//                                  LOGI(oss.str());
                                }

                            }
                        }
                    }
                    else
                    {
                        if (!bProductionItemInfoFirstRendered)
                        {
                            if (productionItemInfo.lastProgress == -1)
                            {
                                productionItemInfo.lastProgress = 0;
                                //productionItemInfo.currentProgress = 0;
                                productionItemInfo.needRefresh_ = true;
                                bProductionItemInfoNeedRendering = true;

                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__;
                    //              LOGI(oss.str());
                                }

                            }
                            else
                            {
                                productionItemInfo.needRefresh_ = true;
                                bProductionItemInfoNeedRendering = true;
                                {
                                    std::ostringstream oss;
                                    oss << __FUNCTION__ << " " << __LINE__;
                    //              LOGI(oss.str());
                                }

                            }
                        }

                        {
                            std::ostringstream oss;
                            oss << "read feedback file error:" << t.second.feedbackFile_;
                    //      LOGI(oss.str());
                        }
                        // record the error information in the log file of specific project.
                    }
                }
                else
                {
                    if (!bProductionItemInfoFirstRendered)
                    {
                        if (productionItemInfo.lastProgress == -1)
                        {
                            productionItemInfo.lastProgress = 0;
                            ///productionItemInfo.currentProgress = 0;
                            productionItemInfo.needRefresh_ = true;
                            bProductionItemInfoNeedRendering = true;

                            {
                                std::ostringstream oss;
                                oss << __FUNCTION__ << " " << __LINE__;
                        //      LOGI(oss.str());
                            }

                        }
                        else
                        {
                            productionItemInfo.needRefresh_ = true;
                            bProductionItemInfoNeedRendering = true;

                            {
                                std::ostringstream oss;
                                oss << __FUNCTION__ << " " << __LINE__;
//                              LOGI(oss.str());
                            }

                        }
                    }
                }
            }

            //bProductionItemInfoFirstRendered = false;

            if (bDestroying || bResubmitting || !bProductionItemInfoNeedRendering)
            {
                bProductionItemInfoGetting = false;

                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__;
//                  LOGI(oss.str());
                }
            }
            else
            {
                bProductionItemInfoGot = true;
                bProductionItemInfoGetting = false;

                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__;
                //  LOGI(oss.str());
                }
            }
        }

        void ProductionWgt::Slot_ClickTab(int idx)
        {
#if 1
            QString tabtext = ui->tabWidget->tabText(idx);

            if (tabtext.toStdString() == "Overview")
            {
//              std::cout << "inside " << tabtext.toStdString() << "(overview) tab of productionWgt." << std::endl;
                //if (bLastMatrixExists)
                //{
                //  osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                //  UserMatrixData::setCurrentMatrixObject(this, cmt);
                //}

                if (mWindow->hasSceneData())
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }
            else if (tabtext.toStdString() == "3D View")
            {
                if (mWindow->hasSceneData())
                    return;

                bool bLastMatrixExists = false;
                osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

                std::cout << "inside " << tabtext.toStdString() << "(3dview) tab of productionWgt." << std::endl;
                //this->recons_object_->GetPath()

                std::string rootfile = this->production_object_->GetOptions().destination_ + "/Data/" + this->production_object_->GetOptions().name_ + ".osgb";
                rootfile = AI3D::CORE::File::EnsureUnifySlash(rootfile);
                if (1)
                {
                    
                    mWindow->RenderModel(rootfile);
                    
                }
                else
                {
                    LOGI("=====================Rendering model,pls wait=============");
                    std::cout << "======================Rendering model,pls wait==================" << std::endl;
                    bool bRunFinished = false;
                    auto savefunc = [&, this]()
                    {
                        mWindow->RenderModel(rootfile);
                        if (mWindow->bModelloaded)
                        {
                            bRunFinished = true;
                            return;
                        }
                        
                    };

                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        if (BlockObject::isChineseVersion())
                        {
                            OpenLoadingPromptV4("模型加载中，请耐心等待");
                        }
                        else
                        {
                            OpenLoadingPromptV4("Please be patient and wait.rendering");
                        }
                        QFuture<void> f1 = QtConcurrent::run(savefunc);

                        while (!bRunFinished)
                        {
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        CloseLoadingPromptV4();
                    }
                    else
                    {
                        savefunc();
                    }
                }
                if (bRenderProductionOnce)
                {
                    if (bLastMatrixExists)
                    {
                        mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                    }
                }
                else
                {
                    bRenderProductionOnce = true;
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }
            else
            {
//              std::cout << "inside " << tabtext.toStdString() << "(unknown) tab of productionWgt." << std::endl;
            }
#else
            QString tabtext = ui->tabWidget->tabText(idx);

            bool bLastMatrixExists = false;
            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

            if (tabtext.toStdString() == "Overview")
            {
                //              std::cout << "inside " << tabtext.toStdString() << "(overview) tab of productionWgt." << std::endl;
                if (bLastMatrixExists)
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }
            else if (tabtext.toStdString() == "3D View")
            {

                std::cout << "inside " << tabtext.toStdString() << "(3dview) tab of productionWgt." << std::endl;
                //this->recons_object_->GetPath()
                std::string rootfile = this->production_object_->GetOptions().destination_ +"/Data/"+ this->production_object_->GetOptions().name_ + ".osgb";
                rootfile = AI3D::CORE::File::EnsureUnifySlash(rootfile);
                mWindow->RenderModel(rootfile);

                if (bLastMatrixExists)
                {
                    mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
                }
                else
                {
                    osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                    UserMatrixData::setCurrentMatrixObject(this, cmt);
                }
            }
            else
            {
                //              std::cout << "inside " << tabtext.toStdString() << "(unknown) tab of productionWgt." << std::endl;
            }

#endif
        }

        job_status_e ProductionWgt::GetProductionStatus(BlockObject* block_data_,ReconstructionObject* recons_object_,ProductionObject* cpo)
        {
            if (!block_data_ || !recons_object_ || !cpo || cpo->GetTiles().size() <= 0)
                return jobsta_e::STATUS_PENDDING;

            int nTotalTiles = 0;
            int nCompletedTiles = 0;
            int nPendingTiles = 0;
            int nRunningTiles = 0;
            int nFailedTiles = 0;
            int lastRow = -1;
            int nUnknownTiles = 0;
            int nCancelledTiles = 0;
            int nCancelledTiles2 = 0;

            ProductionObject* production_object_ = cpo;

            if (cpo->GetOrderedTiles().size() <= 0)
                return jobsta_e::STATUS_PENDDING;

            if (production_object_->IsCompleted())
                return jobsta_e::STATUS_COMPLETE;

            QString lsMasterJobQueue = Settings::getMasterJobQueue();

            for (auto& iter : production_object_->GetOrderedTiles())
            {
                if (MohackerWin::GetInstance() && MohackerWin::GetInstance()->IsProjectDirty())
                    return jobsta_e::STATUS_UNKNOWN;
                if (!production_object_->GetTilesMutual().count(iter))
                {
                    LOGE("no tile "+iter);
                    continue;
                }
                auto& tile = production_object_->GetTilesMutual().at(iter);

                if (tile.status_ == jobsta_e::STATUS_CANCLE)
                {
                    nCancelledTiles++;
                }

                if (!iter.empty())
                    nTotalTiles++;
                
                int jobStatus = -1;
                std::string fullPathJobName;
                if (BlockObject::supportTempLogs())
                {
                    std::ostringstream oss;
                    oss << "CalcBlockStatus...";
                    ///LOGI(oss.str());
                }
            
                std::string feedback_file_ = ReconstructionCommandSet::GenerateTileFeedbackFile(block_data_, production_object_, tile.name_, tile.jobstr_);
                
                //std::string feedback_file_ = ReconstructionCommandSet::GenerateFeedbackFile(block_data_, recons_object_, production_object_, iter, lsMasterJobQueue.toStdString(), fullPathJobName, &jobStatus);

                if (BlockObject::supportTempLogs())
                {
                    std::ostringstream oss;
                    oss << "CalcBlockStatus...";
                    ///LOGI(oss.str());
                }

                bool bFoundCorrectFeedbackFile = false;
                if (!iter.empty())
                {
                    if (!feedback_file_.empty())
                    {
                        JobFeedBack_s feedback;
                        ///bool ret = feedback.load_with_retry(feedback_file_);
                        bool ret = feedback.load_with_retry2(feedback_file_);
                        if (ret)
                        {
                            bFoundCorrectFeedbackFile = true;
                            tile.status_ = feedback.Status;
                            if (feedback.Status == STATUS_COMPLETE)
                                nCompletedTiles++;
                            else if (feedback.Status == STATUS_CANCLE)
                                nCancelledTiles2++;
                            else if (feedback.Status == STATUS_FAILURE)
                                nFailedTiles++;
                            else if (feedback.Status == STATUS_RUNNING)
                                nRunningTiles++;
                            else
                            {
                                nPendingTiles++;
                                //std::cout << nPendingTiles <<" " <<feedback_file_ << " ====feedback==========="<< iter<< " index "<<tile.jobstr_ << std::endl;
                            }
                        }
                        else
                        {
                            if (jobStatus >= 0 && jobStatus <= 4)
                            {
                                if (jobStatus == 2  /*feedback.Status == STATUS_COMPLETE*/)
                                {
                                    tile.status_ = jobsta_e::STATUS_COMPLETE;
                                    nCompletedTiles++;
                                }
                                else if (jobStatus == 3 /*feedback.Status == STATUS_CANCLE*/)
                                {
                                    tile.status_ = jobsta_e::STATUS_CANCLE;
                                    nCancelledTiles2++;
                                }
                                else if (jobStatus == 4 /*feedback.Status == STATUS_FAILURE*/)
                                {
                                    tile.status_ = jobsta_e::STATUS_FAILURE;
                                    nFailedTiles++;
                                }
                                else if (jobStatus == 1 /*feedback.Status == STATUS_RUNNING*/)
                                {
                                    tile.status_ = jobsta_e::STATUS_RUNNING;
                                    nRunningTiles++;
                                }
                                else
                                {
                                    tile.status_ = jobsta_e::STATUS_PENDDING;
                                    nPendingTiles++;
                                    //std::cout << nPendingTiles <<" " <<feedback_file_ << " ====feedback==========="<< iter<< " index "<<tile.jobstr_ << std::endl;
                                }
                            }
                            else
                                nUnknownTiles++;
                        }

                    }
                    //add by chy 因为遇到一个实际debug就是当那个feedback所对应的tile块文件夹不存在时，会导致串到别的production
                    else
                    {
                        if (jobStatus >= 0 && jobStatus <= 4)
                        {
                            if (jobStatus == 2)
                            {
                                // completed
                                tile.status_ = jobsta_e::STATUS_COMPLETE;
                                nCompletedTiles++;
                            }
                            else if (jobStatus == 3)
                            {
                                // cancelled
                                tile.status_ = jobsta_e::STATUS_CANCLE;
                                nCancelledTiles2++;
                            }
                            else if (jobStatus == 4)
                            {
                                // failed
                                tile.status_ = jobsta_e::STATUS_FAILURE;
                                nFailedTiles++;
                            }
                            else if (jobStatus == 1)
                            {
                                // running
                                tile.status_ = jobsta_e::STATUS_RUNNING;
                                nRunningTiles++;
                            }
                            else
                            {
                                // pending
                                tile.status_ = jobsta_e::STATUS_PENDDING;
                                nPendingTiles++;
                            }

                        }
                        else
                            nUnknownTiles++; //@attention 此处和上两行的原因是因为存在可能访问不到feadback文件，具体原因待查
                    }
                    
                }
                //std::cout << nTotalTiles << " " << feedback_file_ << " ====feedback===========" << feedback_file_ << std::endl;
            }

            bool  bcancelled = false;
            bool bcompleted = false;
            bool bpending = false;
            bool bfailed = false;
            bool brunning = false;
            if (nCancelledTiles > 0 || nCancelledTiles2 > 0)
                bcancelled = true;
            if (nRunningTiles > 0)
                brunning = true;
            //@attetion  此处跟refresh_timeout函数保持一致；
            if (nCompletedTiles > 0 && (nTotalTiles >= nCompletedTiles) &&
                nPendingTiles == 0 && nFailedTiles == 0)
            {
                bcompleted = true;
            }
            if (nPendingTiles > 0 && (nTotalTiles >= nPendingTiles) &&
                nRunningTiles == 0 && nFailedTiles == 0)
            {
                bpending = true;
            }

            //@add by chy ：失败只有所有状态为终态(cancle or  complete failed)时，如果有一个failede则为failed;
            if (nPendingTiles == 0 && nRunningTiles == 0 && nFailedTiles > 0)
            {
                bfailed = true;
            }

            if (bcancelled)
            {

                return jobsta_e::STATUS_CANCLE;
            }
            else if (brunning)
            {

                return jobsta_e::STATUS_RUNNING;
            }
            else if (bpending)
            {

                return jobsta_e::STATUS_PENDDING;
            }
            else if (bcompleted)
            {

                production_object_->SetCompleted();
                return jobsta_e::STATUS_COMPLETE;
            }
            
            else if (bfailed)
            {

                return jobsta_e::STATUS_FAILURE;
            }
            else
            {               
                return jobsta_e::STATUS_PENDDING;
            }           
        }

        void ProductionWgt::Slot_Refresh_TimeoutV2()
        {
            if (!isVisible() || !production_object_ || bResubmitting || bDestroying)
                return;

            const ProductionObject* cpo = production_object_;
            if (cpo->GetTiles().size() <= 0)
                return;

            if (bProductionItemInfoGetting || bProductionItemInfoGot)
            {
                if (bProductionItemInfoGot)
                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__;
                    //LOGI(oss.str());

                    RefreshProductionItemInfo();
                    bProductionItemInfoGot = false;

                    // temp code for test purpose only,remember to remove or comment it inside production version.
                    // refresh_timer_->stop();
                }
                else
                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__;
                    //LOGI(oss.str());

                    return;
                }
            }
            else
            {
                auto savefunc = [&, this]()
                {
                    std::ostringstream oss;
                    oss << __FUNCTION__ << " " << __LINE__;
                    //LOGI(oss.str());

                    GetProductionItemInfo();
                    return 0;
                };

                QtConcurrent::run(savefunc);
            }
        }

        // refresh periodically to get current production status,percent and etc.
        void ProductionWgt::Slot_Refresh_Timeout()
        {       
            if (!isVisible() || !production_object_)
                return;

            //std::cout << "inside pwt timeout " << __LINE__ << std::endl;
            const ProductionObject* cpo = production_object_;
            if (cpo->GetTiles().size() <= 0)
                return;

            int nTotalTiles = 0;
            int nCompletedTiles = 0;
            int nPendingTiles = 0;
            int nRunningTiles = 0;
            int nFailedTiles = 0;
            int lastRow = -1;

            int nCancelledTiles = 0;
            int nCancelledTiles2 = 0;
            int  nUnknownTiles = 0;
            int nFoundEmpty = 0;
            int nFeedbackError = 0;

            //此处是否需要增加接口再商讨
            // @attetion 跟AT不一样.一旦某个tile是complete状态，就不在更新之列，就其永久状态为compplete
            //for (auto& iter : cpo->GetTiles())

            bool bNeed2InsertNewRow = false;


            ///if (twProductionList->rowCount() == 0 && production_object_->GetOrderedTiles().size() > 0)
            /// bNeed2InsertNewRow = true;

            if (twProductionList->rowCount() == 0 && production_object_->GetOrderedTiles().size() > 0 || twProductionList->rowCount() > 0 && twProductionList->rowCount() < production_object_->GetOrderedTiles().size())
                bNeed2InsertNewRow = true;

            //          for(auto& iter : recons_object_->GetOrderedTiles())
            for (auto& iter : production_object_->GetOrderedTiles())
            {
                int jobStatus = -1;
                QString lsMasterJobQueue = Settings::getMasterJobQueue();
                std::string fullPathJobName;
                std::string feedback_file_;
                //临时加d
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
                {
                    std::string blockitembase_path = block_data_->GetPath() + "/" + recons_object_->GetIDString() +
                        "/" + "Productions/" + production_object_->GetIDString() + "/" + iter + "/";
                    std::string BRPID = std::string("B") + std::to_string(block_data_->GetId()) + std::string("R") +
                        std::to_string(recons_object_->GetId()) + std::string("P") +
                        std::to_string(cpo->GetId()) + iter;
                    if (!block_data_->GetTaskInfoMutual().reconstructionjobs_.count(BRPID))
                        continue;
                    std::string jobstring = block_data_->GetTaskInfoMutual().reconstructionjobs_.at(BRPID);
                    //feedback_file_ = File::EnsureUnifySlash(blockitembase_path + FEEDBACK_PREFIX + jobstring + ".json");
                    //std::string feedback_file = "";
                    if (JOB_FEEDBACK_USE_BIN) {
                        feedback_file_ = MAKE_FEEDBAK_BIN_FILE(blockitembase_path, jobstring);
                    }
                    else {
                        feedback_file_ = MAKE_FEEDBAK_JSON_FILE(blockitembase_path, jobstring);
                    }
                }
                else
                {
                    auto& tile = production_object_->GetTilesMutual().at(iter);
                    feedback_file_ = ReconstructionCommandSet::GenerateTileFeedbackFile(this->block_data_,  this->production_object_, tile.name_, tile.jobstr_);
                    //feedback_file_ = ReconstructionCommandSet::GenerateFeedbackFile(this->block_data_, this->recons_object_, this->production_object_, iter, lsMasterJobQueue.toStdString(), fullPathJobName, &jobStatus);
                }
                if (!this->production_object_->GetTilesMutual().count(iter))
                {
                    continue;
                }
                auto& tile = this->production_object_->GetTilesMutual().at(iter);

                //std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                //  << iter << std::endl;

                if (tile.status_ == jobsta_e::STATUS_CANCLE)
                {
                    //std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                    //  << iter << std::endl;

                    nCancelledTiles++;
                }

                if (!iter.empty())
                    nTotalTiles++;
                else
                    nFoundEmpty++;

                bool bFeedbackError = true;
                if (!iter.empty() && !feedback_file_.empty())
                {
                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;

                    //std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                    //  << iter << " " << feedback_file_ <<  std::endl;

                    JobFeedBack_s feedback;
                    bool ret = feedback.load_with_retry(feedback_file_);
                    if (ret)
                    {
//                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                          << iter << " " << feedback_file_ << std::endl;

                        tile.status_ = feedback.Status;
                        if (tile.status_ == jobsta_e::STATUS_COMPLETE)
                        {
                            //add by chy 后续需要提出一个接口专门用于更新reconstruction下的tile的状态的
                            //this->recons_object_->GetTilesBaseMutual().at(iter).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
                            this->recons_object_->GetTilesCustomMutual().at(iter).reference_model_status_ = tile_info_s::reconst_status_e::RE_STA_COMPLETED;
//                          std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                              << iter << " " << feedback_file_ << std::endl;
                        }

                        //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                        if (bNeed2InsertNewRow)
                        {
                            twProductionList->insertRow(twProductionList->rowCount());
                            lastRow = twProductionList->rowCount() - 1;
//                          std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                              << iter << " " << feedback_file_ << std::endl;

                        }
                        else
                        {
                            lastRow++;
//                          std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                              << iter << " " << feedback_file_ << std::endl;

                        }

                        for (int j = 0; j < twProductionList->columnCount(); j++)
                        {
                            if (j == 2)
                            {
                                // construct progBarContainer struct object.
                                ProgBarContainer* pProgBarContainer = nullptr;

                                if (bNeed2InsertNewRow)
                                    pProgBarContainer = new ProgBarContainer(ui->widget);
                                else
                                {
                                    pProgBarContainer = (ProgBarContainer*)twProductionList->cellWidget(lastRow, j);
                                }

                                if (feedback.Status == STATUS_COMPLETE)
                                {
                                    //                                  pItem->setText("Completed");
                                    // complete
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    pProgBarContainer->pProgBar->setValue(0);
                                    pProgBarContainer->pLblProg->setText("100%");
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    );
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                    nCompletedTiles++;
                                }
                                else if (feedback.Status == STATUS_FAILURE)
                                {
                                    //  pItem->setText("Failed");
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    int percent = (int)(feedback.Percent + 0.5);
                                    if (percent < 1)
                                        percent = 1;

                                    pProgBarContainer->pProgBar->setValue(percent);
                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                    nFailedTiles++;
                                }
                                else if (feedback.Status == STATUS_CANCLE)
                                {
                                    //                                  pItem->setText("Cancelled");
                                    //std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                                    //  << iter << " " << feedback_file_ << std::endl;

                                    int percent = (int)(feedback.Percent + 0.5);
                                    /*if (percent < 1)
                                        percent = 1;*/

                                    pProgBarContainer->pProgBar->setValue(percent);
                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61, 64, 70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(247, 186, 10);}");
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(247, 186, 10);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                    nCancelledTiles2++;
                                }
                                else if (feedback.Status == STATUS_RUNNING)
                                {
                                    //  pItem->setText("Running");
                                    std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
                                        << iter << " " << feedback_file_ << " percent:" << feedback.Percent << std::endl;

                                    int percent = (int)(feedback.Percent + 0.5);
                                    if (percent < 1)
                                        percent = 0;
                                    else if (percent == 100)
                                    {
                                        percent = (int)(feedback.Percent);
                                    }

                                    pProgBarContainer->pProgBar->setValue(percent);

                                    pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));

                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                        "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                    nRunningTiles++;
                                }
                                else
                                {
                                    //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << " " << tile.status_ << std::endl;

                                    pProgBarContainer->pProgBar->setValue(0);
                                    pProgBarContainer->pLblProg->setText("0%");
                                    pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    );
                                    pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                    nPendingTiles++;
                                }

                                //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;

                                ///pProgBar->setTextVisible(false);

                                if (bNeed2InsertNewRow)
                                    twProductionList->setCellWidget(lastRow, j, pProgBarContainer);
                            }
                            else
                            {
                                QTableWidgetItem* pItem = nullptr;

                                if (bNeed2InsertNewRow)
                                {
                                    pItem = new QTableWidgetItem;
                                    pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                                }
                                else
                                {
                                    pItem = twProductionList->item(lastRow, j);
                                }

                                if (j == 0)
                                    pItem->setText(str2qstr(iter));
                                else if (j == 1)
                                {
                                    if (feedback.Status == STATUS_COMPLETE)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("完成");
                                        }
                                        else
                                        {
                                            pItem->setText("Completed");
                                        }
                                    }
                                    else if (feedback.Status == STATUS_FAILURE)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("失败");
                                        }
                                        else
                                        {
                                            pItem->setText("Failed");
                                        }
                                    }
                                    else if (feedback.Status == STATUS_CANCLE)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("取消");
                                        }
                                        else
                                        {
                                            pItem->setText("Cancelled");
                                        }
                                    }
                                    else if (feedback.Status == STATUS_RUNNING)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("运行中");
                                        }
                                        else
                                        {
                                            pItem->setText("Running");
                                        }
                                    }
                                    else
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;
                                        pItem->setText("Pending");
                                    }
                                }
                                else if (j == 3)
                                {
                                    //  add fail description(may include cancelling?);
                                    if (feedback.Msg.empty())
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        pItem->setText("--");
                                    }
                                    else if (feedback.Status == STATUS_FAILURE)
                                    {
                                        ///pItem->setText("Failed: " + str2qstr(feedback.Msg));
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("失败: " + QString::fromStdString(feedback.Msg));
                                        }
                                        else
                                        {
                                            pItem->setText("Failed: " + QString::fromStdString(feedback.Msg));
                                        }
                                    }
                                    else if (feedback.Status == STATUS_CANCLE)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        ///pItem->setText("Cancelled: " + str2qstr(feedback.Msg));
                                        if (AI3D::CORE::BlockObject::isChineseVersion())
                                        {
                                            pItem->setText("取消: " + QString::fromStdString(feedback.Msg));
                                        }
                                        else
                                        {
                                            pItem->setText("Cancelled: " + QString::fromStdString(feedback.Msg));
                                        }
                                    }
                                    else
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        pItem->setText("--");
                                    }
                                }
                                else
                                {
                                    std::string feedbackname = AI3D::CORE::File::GetFileNameWithoutExtension(feedback_file_);
                                    auto strs = AI3D::CORE::String::StringSplit(feedbackname,"_");
                                    if (strs.size() < 3)
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        pItem->setText("--");
                                    }
                                    else 
                                    {
//                                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                          << iter << " " << feedback_file_ << std::endl;

                                        auto timestr = QString::fromStdString(strs[2]);
                                        QString qSubmitTime = QDateTime::fromString(timestr, "yyyyMMddhhmmss").toString("yyyy/MM/dd hh:mm");
                                        pItem->setText(qSubmitTime);
                                    }
                                    //pItem->setText("2023/7/18 15:00");
                                }

                                //  pItem->setText(QString("txt(%1,%2)").arg(i + 1).arg(j + 1));
                                pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

                                if (bNeed2InsertNewRow)
                                    twProductionList->setItem(lastRow, j, pItem);
                            }

                            twProductionList->resizeRowToContents(lastRow);
                        }

                        bFeedbackError = false;
                    }
                }

                if (bFeedbackError)
                {
                    nFeedbackError++;

                    if (bNeed2InsertNewRow)
                    {
//                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                          << iter << " " << feedback_file_ << std::endl;

                        twProductionList->insertRow(twProductionList->rowCount());
                        lastRow = twProductionList->rowCount() - 1;
                    }
                    else
                    {
//                      std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                          << iter << " " << feedback_file_ << std::endl;

                        lastRow++;
                    }

                    for (int j = 0; j < twProductionList->columnCount(); j++)
                    {
                        if (j == 2)
                        {
                            // construct progBarContainer struct object.
                            ProgBarContainer* pProgBarContainer = nullptr;

                            if (bNeed2InsertNewRow)
                                pProgBarContainer = new ProgBarContainer(ui->widget);
                            else
                            {
                                pProgBarContainer = (ProgBarContainer*)twProductionList->cellWidget(lastRow, j);
                            }

                            if (tile.status_ == jobsta_e::STATUS_COMPLETE || jobStatus == 2)
                            {
                                //  pItem->setText("Completed");
                                // complete
//                              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                  << iter << " " << feedback_file_ << std::endl;

                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("100%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(116,238,191);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                nCompletedTiles++;
                            }
                            else if (tile.status_ == jobsta_e::STATUS_FAILURE || jobStatus == 4)
                            {
                                //                                  pItem->setText("Failed");
//                              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                  << iter << " " << feedback_file_ << std::endl;

#if 0
                                ///int percent = (int)(feedback.Percent + 0.5);
                                int percent = 0;
                                if (percent < 1)
                                    percent = 1;
                                pProgBarContainer->pProgBar->setValue(percent);
                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(227,84,91,51);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(227,84,91);}");
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(227,84,91);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
#else
                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("0%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
#endif
                                //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                nFailedTiles++;
                            }
                            else if (tile.status_ == jobsta_e::STATUS_CANCLE || jobStatus == 3)
                            {
//                              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                  << iter << " " << feedback_file_ << std::endl;

                                //      pItem->setText("Cancelled");
#if 0
///                             int percent = (int)(feedback.Percent + 0.5);
                                int percent = 0;
                                /*if (percent < 1)
                                    percent = 1;*/
                                pProgBarContainer->pProgBar->setValue(percent);
                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61, 64, 70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(247, 186, 10);}");
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(247, 186, 10);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
#else
                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("0%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

#endif
            //                  nFailedTiles++;
                                nCancelledTiles2++;
                            }
                            else if (tile.status_ == jobsta_e::STATUS_RUNNING || jobStatus == 1)
                            {
//                              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                  << iter << " " << feedback_file_ << std::endl;

                                //  pItem->setText("Running");
#if 0
                                ///int percent = (int)(feedback.Percent + 0.5);
                                int percent = 0;
                                if (percent < 1)
                                    percent = 0;
                                pProgBarContainer->pProgBar->setValue(percent);

                                pProgBarContainer->pLblProg->setText(QString("%1%").arg(percent));

                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                    "QProgressBar::chunk {background-color:rgb(116,238,191);}");

                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(116,238,191);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
#else
                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("0%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");
#endif

                                //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
                                nRunningTiles++;
                            }
                            else
                            {
                                //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;
//                              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                  << iter << " " << feedback_file_ << " tile.status:" << tile.status_ << std::endl;

                                pProgBarContainer->pProgBar->setValue(0);
                                pProgBarContainer->pLblProg->setText("0%");
                                pProgBarContainer->pProgBar->setStyleSheet("QProgressBar {background-color:rgb(61,64,70);border:none;border-radius:2px;margin:0px;padding:0px;}"
                                );
                                pProgBarContainer->pLblProg->setStyleSheet("color:rgb(185,185,185);padding-top:16px;margin:0px;font:12 \"Arial\";padding-left:0px;");

                                nPendingTiles++;
                            }

                            //std::cout << "inside pwt timeout " << __LINE__ << " " << tile_ << " " << feedback_file_ << std::endl;

                            ///pProgBar->setTextVisible(false);

                            if (bNeed2InsertNewRow)
                                twProductionList->setCellWidget(lastRow, j, pProgBarContainer);
                        }
                        else
                        {
                            QTableWidgetItem* pItem = nullptr;

                            if (bNeed2InsertNewRow)
                            {
                                pItem = new QTableWidgetItem;
                                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);
                            }
                            else
                            {
                                pItem = twProductionList->item(lastRow, j);
                            }

                            if (j == 0)
                                pItem->setText(str2qstr(iter));
                            else if (j == 1)
                            {
///                             if (feedback.Status == STATUS_COMPLETE)
                                if(tile.status_ == jobsta_e::STATUS_COMPLETE || jobStatus == 2)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("完成");
                                    }
                                    else
                                    {
                                        pItem->setText("Completed");
                                    }
                                }
///                             else if (feedback.Status == STATUS_FAILURE)
                                else if(tile.status_ == jobsta_e::STATUS_FAILURE || jobStatus == 4)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("失败");
                                    }
                                    else
                                    {
                                        pItem->setText("Failed");
                                    }
                                }
///                             else if (feedback.Status == STATUS_CANCLE)
                                else if(tile.status_ == jobsta_e::STATUS_CANCLE || jobStatus == 3)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("取消");
                                    }
                                    else
                                    {
                                        pItem->setText("Cancelled");
                                    }
                                }
///                             else if (feedback.Status == STATUS_RUNNING)
                                else if(tile.status_ == jobsta_e::STATUS_RUNNING || jobStatus == 1)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                        pItem->setText("运行中");
                                    }
                                    else
                                    {
                                        pItem->setText("Running");
                                    }
                                }
                                else
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    if (AI3D::CORE::BlockObject::isChineseVersion())
                                    {
                                    }
                                    else
                                    {
                                        pItem->setText("Pending");
                                    }
                                }
                            }
                            else if (j == 3)
                            {
                                //  add fail description(may include cancelling?);
#if 000
                                if (feedback.Msg.empty())
                                {
                                    pItem->setText("--");
                                }
                                else if (feedback.Status == STATUS_FAILURE)
                                {
                                    ///pItem->setText("Failed: " + str2qstr(feedback.Msg));
                                    pItem->setText("Failed: " + QString::fromStdString(feedback.Msg));
                                }
                                else if (feedback.Status == STATUS_CANCLE)
                                {
                                    ///pItem->setText("Cancelled: " + str2qstr(feedback.Msg));
                                    pItem->setText("Cancelled: " + QString::fromStdString(feedback.Msg));
                                }
                                else
#else
                                if (tile.status_ == jobsta_e::STATUS_FAILURE || jobStatus == 4)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    pItem->setText("Failed: --");
                                }
                                else if (tile.status_ == jobsta_e::STATUS_CANCLE || jobStatus == 3)
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    pItem->setText("Cancelled: --");
                                }
                                else
#endif
                                {
//                                  std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                                      << iter << " " << feedback_file_ << std::endl;

                                    pItem->setText("--");
                                }
                            }
                            else
                                pItem->setText("2023/7/18 15:00");
                            //  pItem->setText(QString("txt(%1,%2)").arg(i + 1).arg(j + 1));
                            pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

                            if (bNeed2InsertNewRow)
                                twProductionList->setItem(lastRow, j, pItem);
                        }

                        twProductionList->resizeRowToContents(lastRow);
                    }
                }
            }

            bool bcancelled = false;
            bool bcompleted = false;
            bool bpending = false;
            bool bfailed = false;
            bool brunning = false;
            if (nCancelledTiles > 0)
                bcancelled = true;
            if (nRunningTiles > 0)
                brunning = true;
            //@attetion  此处最主要的逻辑是nCompletedTiles > 0 && nTotalTiles == nCompletedTiles
            //但是有时候可能会出现两者不相等，原因待查，pending 同理所以逻辑暂定如下；
            if (nCompletedTiles > 0 && (nTotalTiles >= nCompletedTiles) &&
                nPendingTiles == 0 && nFailedTiles == 0)
            {
                bcompleted = true;
            }
            if (nPendingTiles > 0 && (nTotalTiles >= nPendingTiles) &&
                nRunningTiles == 0 && nFailedTiles == 0)
            {
//              std::cout << "inside " << __FUNCTION__ << " " << __FUNCTION__ << " " << __LINE__ << " "
//                  << std::endl;

                bpending = true;
            }

            //@add by chy ：失败只有所有状态为终态(cancle or  complete failed)时，如果有一个failede则为failed;
            if (nPendingTiles == 0 && nRunningTiles == 0 && nFailedTiles > 0)
            {
                bfailed = true;
            }

            if (bcancelled)
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_cancel.png"));
                lblTopLeft->show();

                lblTopRightTop->setText("Cancelled");
                lblTopRightBottom->setText(QString("%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                    .arg(nTotalTiles));

                cpwLeft->hide();
            }
            else if (brunning)
            {
                int percent = nCompletedTiles * 100 / nTotalTiles;

                cpwLeft->setPercent(percent);
                cpwLeft->show();

                lblTopRightTop->setText("Running");
                lblTopRightBottom->setText(QString("%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                    .arg(nTotalTiles));

                lblTopLeft->hide();


            }
            else if (bpending) 
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_wait.png"));
                lblTopLeft->show();

                lblTopRightTop->setText("Pending");
                lblTopRightBottom->setText(QString("Production is pending,%1/%2 milestone(s) completed.").arg(nCompletedTiles)
                    .arg(nTotalTiles));
                //lblTopRightBottom->setText("Production submitted,waiting to run");

                cpwLeft->hide();
            }
            else if (bcompleted)
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_succ.png"));
                lblTopLeft->show();

                lblTopRightTop->setText("Completed");
                //lblTopRightBottom->setText("Processing time:27min 42s");
                lblTopRightBottom->setText(QString("%1/%2 job(s) completed.").arg(nCompletedTiles).arg(nTotalTiles));
                cpwLeft->hide();

                if (this->production_object_)
                {
                    this->production_object_->SetCompleted();

                    if (refresh_timer_)
                        refresh_timer_->stop();
                }
            }       
            else if (bfailed)//>0
            {
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_fail.png"));
                lblTopLeft->show();

                lblTopRightTop->setText("Fail");
                //lblTopRightBottom->setText("Failure reason: xxxxxx.");
                // 6 / 43 tasks completed ；4 errors
                lblTopRightBottom->setText(QString("%1/%2 job(s) completed,%3 failed.").arg(nCompletedTiles).arg(nTotalTiles).arg(nFailedTiles));
                cpwLeft->hide();
            }
            else
            {
                
                // impossible to reach here.even if coming here,it should be completed state.
                lblTopLeft->setPixmap(QPixmap(":/new/prefix1/skin/progress_wait.png"));
                lblTopLeft->show();
                cpwLeft->hide();
            }
            if (0)
            {
#if 0
                // encounter exception.
                if (ReconstructionCommandSet::CanCancelProduction(*this->recons_object_, this->production_object_->GetId()))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
#else
                if (nCancelledTiles <= 0 && (nPendingTiles + nRunningTiles > 0))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
#endif

#if 0
                if (ReconstructionCommandSet::CanResubmitProduction(*this->recons_object_, this->production_object_->GetId()))

                {
                    butResubmitProduction->setVisible(true);
                }
                else
                {
                    butResubmitProduction->setVisible(false);
                }
#else
                if ((nCancelledTiles + nFailedTiles) > 0)
                {
                    if(butResubmitProduction)
                        butResubmitProduction->setVisible(true);
                }
                else
                {
                    if(butResubmitProduction)
                        butResubmitProduction->setVisible(false);
                }
#endif
            }
            else //chy modified by chy @20231215
            {
                //基本逻辑是：cancle和resubmit是互斥的；
                //cancel出现的逻辑是：
                //1：点cancel后的情形resubmit：cancelled、failed、complete三者均有，或者全为cancelled或着failed的，绝不能全是complete
                if (nCancelledTiles <= 0 && (nPendingTiles + nRunningTiles > 0))
                {
                    butCancelProduction->setVisible(true);
                }
                else
                {
                    butCancelProduction->setVisible(false);
                }
                if (butCancelProduction->isVisible() || nCompletedTiles == nTotalTiles)
                {
                    if(butResubmitProduction)
                        butResubmitProduction->setVisible(false);
                }
                else
                {
                    if(butResubmitProduction)
                        butResubmitProduction->setVisible(true);
                }
            }
        }

        void ProductionWgt::Slot_CancelProduction()
        {
            if (QMessageBox::No == Message_Box::question(this, "", "Are you sure you want to cancel processing？", Message_Box_Type::Question_Yes_No))
            {
                return;
            }

            std::string jobPath = qstr2str(Settings::getMasterJobQueue());
            ReconstructionCommandSet::CancelProductionJob(jobPath, block_data_, recons_object_->GetId(), production_object_->GetId());
//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        }

        void ProductionWgt::Slot_ResubmitProduction()
        {
            bResubmitting = true;

            while (bProductionItemInfoGetting || bProductionItemInfoGot)
            {
                // note: wait for the related thread fetching tiles status peroidically to finish.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            BlockObject::Task_Info& task = block_data_->GetTaskInfoMutual();
            std::string jobfile_path = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + NORMALLEVEL;
            std::string projectfile = task.projectfile_;
            std::string hostName = QHostInfo::localHostName().toStdString();

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            bool ret = JobMonitor::CreateDirs();
            if (!ret)
                return;
            production_option_s options;
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            ReconstructionCommandSet::ResubmitProductionJob(hostName, jobfile_path, projectfile, block_data_, recons_object_->GetId(), production_object_->GetId());
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            InitProductionItemInfo();
            bResubmitting = false;
        }

        void ProductionWgt::showEvent(QShowEvent* event)
        {
            //return;
            std::ostringstream oss;
            oss << "ProductionWgt/show.";
            LOGI(oss.str());

            QString tabtext = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
            if (tabtext.toStdString() == "Overview")
                return;
            else if (tabtext.toStdString() == "3D View")
                ;
            else
                return;

            if (mWindow->hasSceneData())
                return;

            if (!bRenderProductionOnce)
                return;

            bool bLastMatrixExists = false;
            osg::Matrixd lastMatrix = UserMatrixData::getCurrentMatrixObject(this, bLastMatrixExists);

            LOGI("=====================Rendering,pls wait=============");
            std::cout << "======================Rendering,pls wait==================" << std::endl;
            bool bRunFinished = false;
            auto savefunc = [&, this]()
            {
                std::string rootfile = this->production_object_->GetOptions().destination_ + "/Data/" + this->production_object_->GetOptions().name_ + ".osgb";
                rootfile = AI3D::CORE::File::EnsureUnifySlash(rootfile);
                mWindow->RenderModel(rootfile);
                //mWindow->RenderModel(this->production_object_->GetOptions().destination_);

                bRunFinished = true;
                return;
            };

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                if (BlockObject::isChineseVersion())
                {
                    OpenLoadingPromptV4("渲染中，请耐心等待");
                }
                else
                {
                    OpenLoadingPromptV4("Please be patient and wait.rendering");
                }

                QFuture<void> f1 = QtConcurrent::run(savefunc);

                while (!bRunFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            else
            {
                savefunc();
            }

            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                CloseLoadingPromptV4();
            }

            if (bLastMatrixExists)
            {
                mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->setByMatrix(lastMatrix);
            }
        }

        void ProductionWgt::hideEvent(QHideEvent* event)
        {
            //return;

            std::ostringstream oss;
            oss << "ProductionWgt/hide.";
            //LOGI(oss.str());

            if (mWindow->hasSceneData())
            {
                osg::Matrixd cmt = mWindow->getOsgEngine()->GetViewer()->getCameraManipulator()->getMatrix();
                UserMatrixData::setCurrentMatrixObject(this, cmt);
                mWindow->clearSceneData();
            }
        }

        void ProductionWgt::closeEvent(QCloseEvent* event)
        {
            std::ostringstream oss;
            oss << "ProductionWgt/close.";
            //LOGI(oss.str());
        }

        CircularProgressWgt::CircularProgressWgt(QWidget* parent)
            : QWidget(parent)
        {
            percent = 0;
            timerPercent = new QTimer(this);
            connect(timerPercent, &QTimer::timeout, this, &CircularProgressWgt::Slot_Percent_Timeout);

            //timerPercent->start(200);
        }

        CircularProgressWgt::~CircularProgressWgt()
        {
        }

        QSize CircularProgressWgt::sizeHint() const
        {
            return QSize(120, 120);
        }

        QSize CircularProgressWgt::minimumSizeHint() const
        {
            return QSize(120, 120);         
        }

        void CircularProgressWgt::paintEvent(QPaintEvent* event)
        {
            QPainter painter(this);
            painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

            //std::cout << "inside cpw pe:" << rect().x() << " " << rect().y() << " " << rect().width() << " " << rect().height() << std::endl;
            QPixmap pix(":/new/prefix1/skin/progbar.png");      

            QRect rec = rect();
            int rw = rec.width();
            int rh = rec.height();

            int minwh = qMin(rect().width(), rect().height());

            QPixmap pix2 = pix.scaled(QSize(minwh,minwh), Qt::KeepAspectRatio);
            painter.drawPixmap(width() / 2 - minwh / 2, height() / 2 - minwh / 2, pix2);

            //painter.drawPixmap(rect(), pix);
            
            painter.translate(width() / 2, height() / 2);

            painter.setPen(QPen(QColor(66,137,140,99),1.0));
            painter.setFont(QFont("Arial", 24));

            painter.drawEllipse(-46, -46, 92, 92);

            //painter.setPen(QPen(QColor(146,231,197), 2.0));
            ///painter.setPen(QPen(QColor(146, 231, 97), 2.0));
            ///painter.drawEllipse(-56, -56, 112, 112);

            //painter.setPen(QPen(QColor(146,231,197), 5.0));
        
            int spanAngles = (int)(percent * 360.0 / 100.0);
            if (percent == 100)
                spanAngles = 360;

            spanAngles = -spanAngles;

            //painter.drawArc(width() / 2 - 115/2, height() / 2 - 115/2, 115, 115,0 * 16,spanAngles * 16);
            if (percent > 0)
            {
                // start angle:3 clock is zero. positive:reverse clockwise. negative: clockwise.
                // span angles::  positive:reverse clockwise   negative:clockwise.
                //painter.drawArc(-57, -57, 114, 114, 0 * 16 + 90 * 16 , spanAngles * 16 + 90 * 16);

                QPen pen(QColor(146, 231, 197), 5.0, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(pen);
                painter.drawArc(-57, -57, 114, 114, (90) * 16, spanAngles * 16);
            }

            painter.setPen(QPen(Qt::white));
            painter.drawText(-40,-40,80,80, Qt::AlignCenter,QString("%1%").arg(percent));
        }

        void CircularProgressWgt::Slot_Percent_Timeout()
        {
            percent++;
            if (percent > 100)
                percent = 0;

            //std::cout << "inside percent timerout:" << percent << std::endl;

            update();           
        }

        void CircularProgressWgt::setPercent(int value)
        {
            if (value < 0)
                value = 0;
            else if (value > 100)
                value = 100;

            percent = value;

            update();               
        }

        ProgBarContainer::ProgBarContainer(QWidget* parent)
            : QWidget(parent)
        {
            setStyleSheet("padding:0px;margin:0px;border-bottom:1px solid rgb(60,60,60);");
            setFixedHeight(50);

            hlProgBar = new QHBoxLayout();
            hlProgBar->setContentsMargins(0, 0, 0, 0);

            pProgBar = new QProgressBar(parent);
            pProgBar->setAttribute(Qt::WA_StyledBackground);
            pProgBar->setMinimum(0);
            pProgBar->setMaximum(100);
            pProgBar->setFixedWidth(140);
            pProgBar->setFixedHeight(4);

            pProgBar->setTextVisible(false);

            pLblProg = new QLabel(parent);
            pLblProg->setAlignment(Qt::AlignLeft);

            hlProgBar->setSpacing(12);
            hlProgBar->addWidget(pProgBar);
            hlProgBar->addWidget(pLblProg);
            hlProgBar->addStretch(1);

            setLayout(hlProgBar);
        }
        
        int MWindow::iOsgEngineWorkingNum = 0;
        std::set<MWindow*> MWindow::setOsgEngineWorking;

        MWindow::MWindow(QWidget* parent, Qt::WindowFlags flags,bool bInsideProduction,bool bInsideBlockAT,bool bInsideConstruction,bool bUseLaterSize,int forceWidth,
            int forceHeight)
            : QWidget(parent,flags)
        {

            bInited = false;
            bAllowEdit = false;
            this->bInsideProduction = bInsideProduction;
            this->bInsideBlockAT = bInsideBlockAT;
            this->bInsideConstruction = bInsideConstruction;
            setStyleSheet("#MWindow { background-color:gray; }");

            meshfiles_.clear();

            QVBoxLayout* vlMain = new QVBoxLayout();
            //InitData();
            mainLayout = new QHBoxLayout;
            viewerWindow = new ViewerQT(0,0,0,0,bUseLaterSize,forceWidth,forceHeight);
            
            // move the following code into resizeEvent.
            if (bUseLaterSize && forceWidth > 0 && forceHeight > 0)
            {
                viewerWindow->setFixedWidth(forceWidth);
                viewerWindow->setFixedHeight(forceHeight);
            }
            else
            {
                viewerWindow->setFixedWidth(1024);
                viewerWindow->setFixedHeight(1024);
            }
            viewerWindow->setStyleSheet("background-color:red;");

            mainLayout->addWidget(viewerWindow, 1, Qt::AlignCenter);
            mainLayout->setContentsMargins(0, 0, 0, 0);

#if 0
            osg::Camera* camera = viewerWindow->getCamera();//获得渲染器中的相机
            camera->setClearColor(osg::Vec4(128.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0, 0.8));//设置清除缓存区背景的颜色.RGBA格式.

            QPalette p;
            p.setColor(QPalette::Background, QColor(0, 0, 0));
            viewerWindow->setCameraManipulator(new osgGA::DriveManipulator);//  osgGA::TrackballManipulator

            viewerWindow->setPalette(p);


            //setLayout(mainLayout);

    //窗口大小变化事件
            //viewerWindow->addEventHandler(new osgGA::StateSetManipulator(viewerWindow->getCamera()->getOrCreateStateSet()));
            //viewerWindow->addEventHandler(new osgViewer::WindowSizeHandler);
            //viewerWindow->addEventHandler(new osgViewer::StatsHandler);

            //添加操作器

            osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

            keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());

            keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
            keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
            keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());

            //viewerWindow->setCameraManipulator(keyswitchManipulator.get());


            //添加路径记录
            ///viewerWindow->addEventHandler(new osgViewer::RecordCameraPathHandler);
#endif
            isFirst = true;
            isSemiautoFirst = true;

            QHBoxLayout* hlSaveLayout = new QHBoxLayout();
             butSave = new QPushButton(parent);
             butCancel = new QPushButton(parent);
            butSave->setText("Save");
            butCancel->setText("Cancel");
            butSave->setObjectName("Save");
            butCancel->setObjectName("Cancel");
            //butSave->setFlat(true);
            //butCancel->setFlat(true);
            butSave->setStyleSheet("color:red;");
            butCancel->setStyleSheet("color:green;");
            hlSaveLayout->addStretch(1);
            hlSaveLayout->addWidget(butSave);
            hlSaveLayout->addSpacing(100);
            hlSaveLayout->addWidget(butCancel);
            hlSaveLayout->addStretch(1);

            vlMain->addLayout(mainLayout,1);
            vlMain->addLayout(hlSaveLayout);

            QHBoxLayout* hlTop = new QHBoxLayout();
            hlTop->setSpacing(0);
            hlTop->setContentsMargins(0, 0, 0, 0);

            QVBoxLayout* vlLeft = new QVBoxLayout();
            //QVBoxLayout* vlCenter = new QVBoxLayout();
            QVBoxLayout* vlRight = new QVBoxLayout();

            QPushButton* butLeft1 = new QPushButton(parent);
            QPushButton* butLeft2 = new QPushButton(parent);
            QPushButton* butLeft3 = new QPushButton(parent);
            QPushButton* butLeft4 = new QPushButton(parent);

            butLeft1->setText("Front");
            butLeft1->setObjectName("Front");
            //butLeft2->setText("Left2");
            //butLeft2->setIcon(QPixmap(":/new/prefix1/skin/return_top.png"));
            butLeft2->setStyleSheet("QPushButton { image:url(\":/new/prefix1/skin/return_top.png\");}"
            "QPushButton:hover { image:url(\":/new/prefix1/skin/return_top_hover.png\");}");
            butLeft2->setObjectName("Left2");
            //butLeft3->setText("Left3");
            //butLeft3->setIcon(QPixmap(":/new/prefix1/skin/translate.png"));
            butLeft3->setStyleSheet("QPushButton {image:url(\":/new/prefix1/skin/translate.png\");}"
            "QPushButton:hover { image:url(\":/new/prefix1/skin/translate_hover.png\");}");
            butLeft3->setObjectName("Left3");
            //butLeft4->setText("Left4");
            //butLeft4->setIcon(QPixmap(":/new/prefix1/skin/rotate.png"));
            butLeft4->setStyleSheet("QPushButton {image:url(\":/new/prefix1/skin/rotate.png\");}"
            "QPushButton:hover {image:url(\":/new/prefix1/skin/rotate_hover.png\");}");
            butLeft4->setObjectName("Left4");

            vlLeft->addSpacing(20);
            vlLeft->setSpacing(20);
            vlLeft->addWidget(butLeft1);
            vlLeft->addWidget(butLeft2);
            vlLeft->addWidget(butLeft3);
            vlLeft->addWidget(butLeft4);
            vlLeft->addStretch(1);

            QPushButton* butMiddle1 = new QPushButton(parent);
            QPushButton* butMiddle2 = new QPushButton(parent);
            QPushButton* butMiddle3 = new QPushButton(parent);

            //butMiddle1->setText("Middle1");
            //butMiddle1->setIcon(QPixmap(":/new/prefix1/skin/edit_roi.png"));
            butMiddle1->setStyleSheet("QPushButton {image:url(\":/new/prefix1/skin/edit_roi.png\");}"
            "QPushButton:hover {image:url(\":/new/prefix1/skin/edit_roi_hover.png\");}");
            butMiddle1->setObjectName("Middle1");

            butMiddle2->setText("Middle2");
            butMiddle2->setObjectName("Middle2");

            butMiddle3->setText("Middle3");
            butMiddle3->setObjectName("Middle3");
            ///butMiddle2->hide();
            ///butMiddle3->hide();

            vlRight->addSpacing(20);
            vlRight->setSpacing(20);
            vlRight->addWidget(butMiddle1);
            vlRight->addWidget(butMiddle2);
            vlRight->addWidget(butMiddle3);
            vlRight->addStretch(1);

            //setLayout(vlMain);
            hlTop->addLayout(vlLeft);
            hlTop->addLayout(vlMain, 1);
            hlTop->addLayout(vlRight);

            setLayout(hlTop);

            blockNumber = 0;            

#if 0
            pOsgEngine = OsgEngine::getInstance();
            //pOsgEngine->initViewer();
            //pOsgEngine->SetViewer(viewerWindow);
            pOsgEngine->initViewer(viewerWindow,0, 0, 1960, 1080);
#else
            bool bNewEngine = false;
            pOsgEngine = OsgEngine::getInstance2((unsigned long)this,&bNewEngine);

            std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " " << std::hex << std::showbase << this << " " << viewerWindow << " " << pOsgEngine << std::dec << std::endl;
            ///if(bUseLaterSize && forceWidth > 0 && forceHeight > 0)
            /// bNewEngine = true;

            if (bNewEngine && pOsgEngine)
            {
                if (bUseLaterSize && forceWidth > 0 && forceHeight > 0)
                {
                    pOsgEngine->initViewer(viewerWindow, 0, 0, forceWidth, forceHeight);
                }
                else
                {
                    pOsgEngine->initViewer(viewerWindow, 0, 0, 942,791);
                }
            }
#endif

            //设置当前要素图层
            
            //回调事件注册
            if (bInsideBlockAT)
            {
                pCallbackEventTest = new CallbackEventTest(this, nullptr, nullptr);
                EventManager::GetInstance()->registerEvent(CALL_BACK_SELECT_PHOTO, pCallbackEventTest,pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_RIGHT_SELECT_PHOTO, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_SELECT_PHOTO_WINDOWS, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_SELECT_TILE, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_ROI_BOX_DRAG, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_ROI_POLYGON_DRAG, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_CAMERA, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_TIEPOINT, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_REMOVE_PHOTO, pCallbackEventTest, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_REMOVE_TIEPOINTS, pCallbackEventTest, pOsgEngine);
                pTilingCallbackEvent = nullptr;
            }

            if (bInsideProduction)
            {
                pTilingCallbackEvent = new TilingCallbackEvent(this, nullptr);
            
                EventManager::GetInstance()->registerEvent(CALL_BACK_ROI_BOX_DRAG, pTilingCallbackEvent, pOsgEngine);
                EventManager::GetInstance()->registerEvent(CALL_BACK_ROI_POLYGON_DRAG, pTilingCallbackEvent, pOsgEngine);
                        
                // Selecting tiles for production definition just before submitting production inside ConstructionWgt.
                ///pCallbackEventTest = nullptr;
                pCallbackEventTest = new CallbackEventTest(this, nullptr, nullptr);
                EventManager::GetInstance()->registerEvent(CALL_BACK_SELECT_TILE, pCallbackEventTest, pOsgEngine);
              /*  pProCallbackEvent = new ProductionCallbackEvent(this, nullptr, nullptr);
                EventManager::GetInstance()->registerEvent(CALL_BACK_OSGB_LOADED, pProCallbackEvent, pOsgEngine);*/
            }
           /* if (bInsideConstruction)
            {
               
                pProCallbackEvent = new ProductionCallbackEvent(this, nullptr, nullptr);
                EventManager::GetInstance()->registerEvent(CALL_BACK_OSGB_LOADED, pProCallbackEvent, pOsgEngine);
            }*/
            


            connect(butLeft1, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft2, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft3, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft4, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);

            connect(butMiddle1, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butMiddle2, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butMiddle3, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);

            connect(butSave, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butCancel, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);


            if (bInsideProduction)
            {
                butLeft1->hide();
                butLeft2->hide();
                butLeft3->hide();
                butLeft4->hide();
                butMiddle1->hide();
                butMiddle2->hide();
                butMiddle3->hide();

                butSave->hide();
                butCancel->hide();
            }

            if (bInsideBlockAT)
            {
                butLeft1->hide();
                butLeft2->hide();
                butLeft3->hide();
                butLeft4->hide();

                butMiddle1->hide();
                butMiddle2->hide();
                butMiddle3->hide();

                butSave->hide();
                butCancel->hide();
            }

            if (bInsideConstruction)
            {
                //butMiddle1->show();
                //butMiddle2->show();
                //butMiddle3->show();
            }
        }

        MWindow::~MWindow()
        {
            mainLayout = NULL;
            viewerWindow = NULL;

            if (setOsgEngineWorking.count(this) > 0)
            {
                setOsgEngineWorking.erase(this);
                if(iOsgEngineWorkingNum > 0)
                    iOsgEngineWorkingNum--;
            }

            bHasSceneData = false;

            OsgEngine::deleteInstance((unsigned long)this);
            delete pOsgEngine;
            pOsgEngine = nullptr;
//          std::cout << " inside MWindow destroyed: " << std::hex << std::showbase << this << std::dec << std::endl;
        }

        void MWindow::init(int w, int h)
        {
//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            return;
            if (bInited)
                return;

            bInited = true;

            meshfiles_.clear();

            QVBoxLayout* vlMain = new QVBoxLayout();

            mainLayout = new QHBoxLayout;
            viewerWindow = new ViewerQT;

            // move the following code into resizeEvent.
            //viewerWindow->setFixedWidth(1200);
            //viewerWindow->setFixedHeight(600);

            viewerWindow->setFixedWidth(w);
            viewerWindow->setFixedHeight(h);

            mainLayout->addWidget(viewerWindow, 1, Qt::AlignCenter);
            mainLayout->setContentsMargins(0, 0, 0, 0);
            //setLayout(mainLayout);


    //窗口大小变化事件
            viewerWindow->addEventHandler(new osgGA::StateSetManipulator(viewerWindow->getCamera()->getOrCreateStateSet()));
            viewerWindow->addEventHandler(new osgViewer::WindowSizeHandler);
            viewerWindow->addEventHandler(new osgViewer::StatsHandler);



            //添加操作器

            osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

            keyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());

            keyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
            keyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
            keyswitchManipulator->addMatrixManipulator('4', "Terrain", new osgGA::TerrainManipulator());

            viewerWindow->setCameraManipulator(keyswitchManipulator.get());


            //添加路径记录
            ///viewerWindow->addEventHandler(new osgViewer::RecordCameraPathHandler);

            isFirst = true;
            isSemiautoFirst = true;

            QHBoxLayout* hlSaveLayout = new QHBoxLayout();
            QPushButton* butSave = new QPushButton(parentWidget());
            QPushButton* butCancel = new QPushButton(parentWidget());
            butSave->setText("Save");
            butCancel->setText("Cancel");
            butSave->setObjectName("Save");
            butCancel->setObjectName("Cancel");
            //butSave->setFlat(true);
            //butCancel->setFlat(true);
            butSave->setStyleSheet("color:red;");
            butCancel->setStyleSheet("color:green;");
            hlSaveLayout->addStretch(1);
            hlSaveLayout->addWidget(butSave);
            hlSaveLayout->addSpacing(100);
            hlSaveLayout->addWidget(butCancel);
            hlSaveLayout->addStretch(1);

            
            vlMain->addLayout(mainLayout, 1);
            vlMain->addLayout(hlSaveLayout);

            QHBoxLayout* hlTop = new QHBoxLayout();
            QVBoxLayout* vlLeft = new QVBoxLayout();
            //QVBoxLayout* vlCenter = new QVBoxLayout();
            QVBoxLayout* vlRight = new QVBoxLayout();

            QPushButton* butLeft1 = new QPushButton(parentWidget());
            QPushButton* butLeft2 = new QPushButton(parentWidget());
            QPushButton* butLeft3 = new QPushButton(parentWidget());
            QPushButton* butLeft4 = new QPushButton(parentWidget());

            butLeft1->setText("Left1");
            butLeft2->setText("Left2");
            butLeft3->setText("Left3");
            butLeft4->setText("Left4");

            vlLeft->addSpacing(20);
            vlLeft->setSpacing(20);
            vlLeft->addWidget(butLeft1);
            vlLeft->addWidget(butLeft2);
            vlLeft->addWidget(butLeft3);
            vlLeft->addWidget(butLeft4);
            vlLeft->addStretch(1);

            QPushButton* butMiddle1 = new QPushButton(parentWidget());
            QPushButton* butMiddle2 = new QPushButton(parentWidget());
            QPushButton* butMiddle3 = new QPushButton(parentWidget());

            butMiddle1->setText("Middle1");
            butMiddle2->setText("Middle2");
            butMiddle3->setText("Middle3");

            vlRight->addSpacing(20);
            vlRight->setSpacing(20);
            vlRight->addWidget(butMiddle1);
            vlRight->addWidget(butMiddle2);
            vlRight->addWidget(butMiddle3);
            vlRight->addStretch(1);


            //setLayout(vlMain);
            hlTop->addLayout(vlLeft);
            hlTop->addLayout(vlMain, 1);
            hlTop->addLayout(vlRight);

            setLayout(hlTop);

            blockNumber = 0;

            pOsgEngine = OsgEngine::getInstance();
            //pOsgEngine->initViewer();
            pOsgEngine->initViewer(viewerWindow,0, 0, w, h);

            //设置当前要素图层
            pOsgEngine->SetElementType(Element_Type::ELEMENT_PHOTOS);
            pOsgEngine->SetSelectType(Select_Type::SELECT_ONE);
            //回调事件注册

            connect(butLeft1, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft2, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft3, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butLeft4, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);

            connect(butMiddle1, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butMiddle2, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);
            connect(butMiddle3, &QPushButton::clicked, this, &MWindow::Slot_OsgViewButtonClicked);

            if (bInsideProduction)
            {
                butLeft1->hide();
                butLeft2->hide();
                butLeft3->hide();
                butLeft4->hide();

                butMiddle1->hide();
                butMiddle2->hide();
                butMiddle3->hide();
                butSave->hide();
                butCancel->hide();
            }
        }

        void MWindow::resizeEvent(QResizeEvent* resizeEvent)
        {
    //      std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << std::hex << std::showbase << (unsigned long)this  << std::dec << " " << resizeEvent->size().width() << " / "
    //          << resizeEvent->size().height() << std::endl;

            if (viewerWindow != nullptr)
            {
                
                //viewerWindow->resize(resizeEvent->size());
                viewerWindow->setFixedSize(resizeEvent->size());
                //viewerWindow->setFixedWidth(qApp->desktop()->width());
                //viewerWindow->setFixedHeight(qApp->desktop()->height());
                init(resizeEvent->size().width(), resizeEvent->size().height());
                emit mwindow_resized();
            }
        }

        int MWindow::getOsgEngineWorkingNum()
        {
            return iOsgEngineWorkingNum;
        }

        void MWindow::dumpOsgEngineInfo()
        {
            std::ostringstream oss;
            oss << "dump osgEngineInfo:" << iOsgEngineWorkingNum << " / " << setOsgEngineWorking.size();
            LOGI(oss.str());
        }   

        void MWindow::loadOsgFile(std::string& fileName)
        {
//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (!pOsgEngine)
                return;
//          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            try
            {
                auto loadedModel = pOsgEngine->LoadOsgModel(fileName);
                osgUtil::Optimizer optimizer;
                optimizer.optimize(loadedModel.get());
                viewerWindow->updateTraversal();
                viewerWindow->setSceneData(loadedModel.get());
            }
            catch (std::exception &ex)
            {
                std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << ",exception occured:" << ex.what() << std::endl;
            }

            setSceneData();
        }

        void MWindow::ResetConstraint()
        {
            if (!pOsgEngine)
                return;
            try
            {
                /*1107pOsgEngine->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_POLYGON);

                AI3D::VIEWER::Tile3DViewInterface interface_(pReconstData, getOsgEngine());
                interface_.BuildConstraintNode();*/

                RenderReconstruction(pReconstData);

            }
            catch (std::exception& ex)
            {
                std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << ",exception occured:" << ex.what() << std::endl;
            }
        }
        //传入根节点名称如果该节点不存在则在目录下找
        void MWindow::RenderModel(std::string filename)
        {
            std::string msg ="RenderModel begin" +  __LINE__ ;
            std::cout << msg << std::endl;
            LOGI(msg);
            
            pOsgEngine->RemoveAll();
            std::vector<std::string> result;
            if (AI3D::CORE::File::ExistsPath(filename)&&AI3D::CORE::File::ExistsFile(filename) )
            {
                result.push_back(filename);
            }
            else
            {
                LOGI(filename + " is not exists.");
                return;
            }
            if(0)
            {
                std::string path = AI3D::CORE::File::GetParentDir(filename);
                
                OsgEngine::GetTileDirCoarseLevelTrees(path, result);
                for (std::vector<std::string>::iterator it = result.begin();
                    it != result.end();/*it++*/)
                {
                    std::string temp = *it;
                    AI3D::CORE::String::StringToLower(&temp);
                    std::string b = "las";
                    string::size_type idx = temp.find(b); //在a中查找b.
                    if (idx != string::npos) //存在.
                    {
                        it = result.erase(it);
                    }
                    else //
                    {
                        it++;
                    }

                }
                //最后过滤一遍
            //统计有几个_，下划线的数量一致就行
                if (!result.empty())
                {
                    auto linestrs =  AI3D::CORE::String::StringSplit(result[0],"_");
                    
                    int len = linestrs.size()-1;

                    for (auto& iter : result)
                    {
                        auto linetemp = AI3D::CORE::String::StringSplit(iter, "_");
                        auto leniter = linetemp.size()-1;

                        if (len > leniter)
                        {
                            len = leniter;
                        }
                    }

                    for (std::vector<std::string>::iterator it = result.begin();
                        it != result.end();)
                    {
                        std::string temp = *it;
                        auto leniter = temp.length();
                        if (len != leniter)
                        {
                            it = result.erase(it);
                        }
                        else //
                        {
                            it++;
                        }


                    }

                }
            }

            msg = "RenderModel " + std::to_string(result.size())+" ";
            msg += __LINE__;
            std::cout << msg << std::endl;
            LOGI(msg);
            int cnt = 0;

            int failedcnt = 0;
            for (auto& file : result)
            {
                std::string name = AI3D::CORE::File::GetFileNameWithoutExtension(file);
                    
                auto node = pOsgEngine->LoadOsgModel(cnt,name,file);
                if(node==nullptr)
                {
                    
                    failedcnt++;
                }
                cnt++;              
            }
            if (failedcnt > 0)
            {
                LOGI("load osg model failed .");
                return;
            }
            auto loadedModel = pOsgEngine->GetRootNode();
            if (loadedModel)
            {
                
                osgUtil::Optimizer optimizer;
                optimizer.optimize(loadedModel.get());
                viewerWindow->updateTraversal();

                viewerWindow->setSceneData(loadedModel.get());
				pOsgEngine->LookAtModel(loadedModel, ModelViewType::MODEL_UP,10000);				
                std::string msg = "RenderModel finished ";
                msg += __LINE__;
                std::cout << msg << std::endl;
                LOGI(msg);
            }
            else
            {
                std::string msg = "RenderModel failed " ;
                msg += __LINE__;
                std::cout << msg << std::endl;
                LOGI(msg);
            }

            setSceneData();
        }
        void  MWindow::ResetROI()
        {
            if (!pOsgEngine)
                return;
            try
            {
                RenderReconstruction(pReconstData);
                


            }
            catch (std::exception& ex)
            {
                std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << ",exception occured:" << ex.what() << std::endl;
            }
        }

        

        void MWindow::ROIEdit()
        {
            if (!pOsgEngine)
                return;
            try
            {
                pOsgEngine->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_ROI);
                pOsgEngine->SetSelectType(SELECT_TYPE::SELECT_ONE);
                pOsgEngine->SetROIStatus(true);
                pOsgEngine->SetSelectElement(ELEMENT_LAYER_TYPE::ELEMENT_ROI, std::vector<int>({ 0 }));
                butSave->setVisible(true);
                butCancel->setVisible(true);
            }
            catch (std::exception& ex)
            {
                std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << ",exception occured:" << ex.what() << std::endl;
            }

            bAllowEdit = true;
        }

        void MWindow::RenderReconstruction( ReconstructionObject* data,bool bSelectTiles)
        {
            if (!pOsgEngine)
                return;

            try
            {
                AI3D::CORE::ReconPerfStage perf_total("RenderReconstruction", "total");
                {
                    AI3D::CORE::ReconPerfStage perf_remove("RenderReconstruction", "RemoveScene");
                    pOsgEngine->RemoveScene();
                }

                const reconstruction_t id = data->GetId();
                pReconstData = data;
                pReconstData->SetId(id);

                // Share block-owned reconstruction with ROI callback (no heap copy).
                if (ReconstructionObject* old_cb = pTilingCallbackEvent->GetReconstructObject()) {
                    if (old_cb != data) {
                        delete old_cb;
                    }
                }
                pTilingCallbackEvent->SetReconstruct(data);

                {
                    AI3D::CORE::ReconPerfStage perf_init("RenderReconstruction", "Tile3DViewInterface_Init");
                    AI3D::VIEWER::Tile3DViewInterface interface_(data, pOsgEngine);
                    interface_.InitWithOutATScene(bSelectTiles);
                }

                auto loadedModel = pOsgEngine->GetRootNode();
                if (loadedModel)
                {
                    {
                        AI3D::CORE::ReconPerfStage perf_opt("RenderReconstruction", "OsgOptimizer");
                        osgUtil::Optimizer optimizer;
                        optimizer.optimize(loadedModel.get());
                    }
                    {
                        AI3D::CORE::ReconPerfStage perf_viewer("RenderReconstruction", "ViewerSetSceneAndLookAt");
                        viewerWindow->updateTraversal();
                        viewerWindow->setSceneData(loadedModel.get());
                        pOsgEngine->LookAtModel(loadedModel, ModelViewType::MODEL_UP);
                    }
                }
                else
                {
                    ReconPerfLog("[ReconPerf] RenderReconstruction | ViewerSetSceneAndLookAt | skipped (root null)");
                }
            }
            catch (std::exception& ex)
            {
                ReconPerfLog(std::string("[ReconPerf] RenderReconstruction | exception | ") + ex.what());
            }

            setSceneData();
        }

        // std::vector<image_t> &images
        void MWindow::RenderBlockWithSelectedImages(const ATData& data, jobsta_e blockstatus, std::vector<image_t>& images)
        {
            auto lastselection_layer = pOsgEngine->GetCurrentElementType();
            pOsgEngine->RemoveAll();
            AI3D::VIEWER::AT3DViewInterface interface_(data, pOsgEngine, blockstatus);
            std::vector<int> ids;

            //此处应该是支持传入多个被选中的影像
//          if (item_select_ != nullptr && *item_select_ != kInvalidImageId)
//          {
//              ids.push_back(*item_select_);
//              interface_.SetSelectedImages(ids);
//          }

            if (lastselection_layer== ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS&&  images.size() > 0)
            {
                for (auto t : images)
                    ids.push_back((int)t);
                interface_.SetSelectedImages(ids);
            }

            //AT_viewer_setting_s settings1;
            //interface_.SetSettings(settings1);
            interface_.Init();
            pOsgEngine->SetElementType(lastselection_layer);
            auto loadedModel = pOsgEngine->GetRootNode();

            /*if (camMat != osg::Matrix())
            {
                pOsgEngine->GetViewer()->getCameraManipulator()->setByMatrix(camMat);
            }
            else*/
            {
                pOsgEngine->LookAtModel(loadedModel, ModelViewType::MODEL_FRONT);
            }
            /*osg::Matrixd camMatnew = pOsgEngine->GetViewer()->getCameraManipulator()->getMatrix();
            SetCamMat(camMatnew);*/
            //pOsgEngine->LookAt(ELEMENT_TIEPOINTS, ModelViewType::MODEL_UP);
            osgUtil::Optimizer optimizer;
            optimizer.optimize(loadedModel.get());
            viewerWindow->updateTraversal();
            viewerWindow->setSceneData(loadedModel.get());

            setSceneData();
        }
        void MWindow::InitData()
        {
            /*block_data = nullptr;
            reconst_data = nullptr;
             pTilingCallbackEvent = nullptr;
             pCallbackEventTest = nullptr;*/
        }
        void MWindow::RenderBlock(const ATData& data, jobsta_e blockstatus)
        {
            return;

            pOsgEngine->RemoveAll();
            AI3D::VIEWER::AT3DViewInterface interface_(data, pOsgEngine, blockstatus);
            std::vector<int> ids;
            //此处应该是支持传入多个被选中的影像
            if (item_select_ !=nullptr && *item_select_ != kInvalidImageId)
            {
                ids.push_back(*item_select_);
                interface_.SetSelectedImages(ids);
            }
            
            interface_.Init();
            auto loadedModel = pOsgEngine->GetRootNode();
            
            /*if (camMat != osg::Matrix())
            {
                pOsgEngine->GetViewer()->getCameraManipulator()->setByMatrix(camMat);
            }
            else*/
            {
                pOsgEngine->LookAtModel(loadedModel, ModelViewType::MODEL_FRONT);
            }
            /*osg::Matrixd camMatnew = pOsgEngine->GetViewer()->getCameraManipulator()->getMatrix();
            SetCamMat(camMatnew);*/
            //pOsgEngine->LookAt(ELEMENT_TIEPOINTS, ModelViewType::MODEL_UP);
            osgUtil::Optimizer optimizer;
            optimizer.optimize(loadedModel.get());
            viewerWindow->updateTraversal();
            viewerWindow->setSceneData(loadedModel.get());

            setSceneData();
        }

        
        void MWindow::on_actionLoadFile_triggered()
        {
        }

        ///void MWindow::SetCamMat(osg::Matrixd mat) { camMat = mat; };
        void MWindow::Slot_OsgViewButtonClicked()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            QPushButton* butSender = dynamic_cast<QPushButton*>(sender());
            if (!butSender)
                return;     
            
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << butSender->text().toStdString() << " " 
            //  <<  butSender->objectName().toStdString() << std::endl;

            //std::cout << "clicked:" << butSender->objectName().toStdString() << std::endl;
            
            if (butSender->objectName() == "Save")
            {
                //获取新的tile
                    std::cout << pReconstData->GetTilesCustomMutual().size()<<" --1" << std::endl;;
                //pTilingCallbackEvent->GetBox();
                    //pReconstData = this->pTilingCallbackEvent->GetReconstructObject();
                 AI3D::CORE::ReconstructionObject *temp = this->pTilingCallbackEvent->GetReconstructObject();
                
                    
                    std::cout << std::hex << std::showbase << temp << "== " << temp->GetTilingDiscriptor() <<" "<< temp ->GetId()<< std::dec << std::endl;
                    std::cout << std::hex << std::showbase << pReconstData << "==--- " << pReconstData->GetTilingDiscriptor() << " " << pReconstData->GetId() << std::dec << std::endl;
                    //*pReconstData = *temp;
                    //把相关修改信息赋值给pReconstData;
                    pReconstData->GetBoundaryCustomMutual() = temp->GetBoundaryCustomMutual();
                    pReconstData->GetBoundingBoxCustomMutual() = temp->GetBoundingBoxCustomMutual();
                    pReconstData->GetTilesCustomMutual() = temp->GetTilesCustomMutual();
                    pReconstData->GetTilingDiscriptorMutual()->GetParamsMutual() = temp->GetTilingDiscriptorMutual()->GetParamsMutual();
                    std::cout << std::hex << std::showbase << temp << "=1= " << temp->GetTilingDiscriptor() << " " << temp->GetId() << std::dec << std::endl;
                    std::cout << std::hex << std::showbase << pReconstData << "=1=--- " << pReconstData->GetTilingDiscriptor() << " " << pReconstData->GetId() << std::dec << std::endl;


                std::cout << pReconstData->GetTilesCustomMutual().size() << std::endl;;
                emit signal_projchanged(pReconstData,true);
                butSave->hide();
                butCancel->hide();
                
                pOsgEngine->SetROIStatus(false);
                bAllowEdit = false;
                emit signal_roiedit_saved();
            }
            else if (butSender->objectName() == "Cancel")
            {
                butSave->hide();
                butCancel->hide();
                
                pOsgEngine->SetROIStatus(false); //modify by zhaobf
                emit signal_projchanged(pReconstData,false);
                bAllowEdit = false;
                emit signal_roiedit_cancelled();
            }
            else if (butSender->objectName() == "Front")//出发选tile块
            {
                //std::string name = pCallbackEventTest->GetBoxName();
                //std::cout << name << std::endl;
                pOsgEngine->SetSelectType(Select_Type::SELECT_ONE);
                pOsgEngine->SetElementType(ELEMENT_LAYER_TYPE::ELEMENT_TILE);
            }
            else if (butSender->objectName() == "Left2")//快速回到顶视图
            {
                auto loadedModel = pOsgEngine->GetRootNode();
                pOsgEngine->LookAtModel(loadedModel, ModelViewType::MODEL_UP);              
            }
            else if (butSender->objectName() == "Left3")//平移
            {
                pOsgEngine->SetSceneOperationType(SECENE_OPERATION_TYPE::SECENE_MODE_MOVE);
            }
            else if (butSender->objectName() == "Left4")//旋转
            {
                pOsgEngine->SetSceneOperationType(SECENE_OPERATION_TYPE::SECENE_MODE_ROTATE);
            }
            else if (butSender->objectName() == "Middle1")
            {
                //std::cout << "middle1 clicked." << std::endl;
                //QMessageBox::information(nullptr, "tiles list", "display tiles list.");
            }
            else if (butSender->objectName() == "Middle2")
            {
                //std::cout << "middle2 clicked." << std::endl;
                OpenOriginSettings(nullptr, nullptr);
            }
            else if (butSender->objectName() == "Middle3")
            {
                //std::cout << "middle3 clicked." << std::endl;
                ///OpenTilesList();
            }
            
        
        }
        void MWindow::ResetSelectLayer(const AI3D::VIEWER::selection_layer_e& layer)
        {
            ATData data;
            jobsta_e blockstatus;
            AI3D::VIEWER::AT3DViewInterface interface_(data, pOsgEngine, blockstatus);
            interface_.ResetSelectLayer(layer);
        }

        void MWindow::ResetSelectionMode(const AI3D::VIEWER::selection_mode_e& mode)
        {
            ATData data;
            jobsta_e blockstatus;
            AI3D::VIEWER::AT3DViewInterface interface_(data, pOsgEngine, blockstatus);
            interface_.ResetSelectionMode(mode);
        }
        void MWindow::ResetImageLayerSeleted(const std::set<AI3D::VIEWER::reconst_element_e>& imageLayerSet)
        {
            
            jobsta_e blockstatus;
            AI3D::VIEWER::Tile3DViewInterface::ResetImageLayer(imageLayerSet,pOsgEngine);
            
        }
        
        void MWindow::ResetImageLayerSeleted(const std::set<AI3D::VIEWER::image_layer_e>& imageLayerSet)
        {
            ATData data;
            jobsta_e blockstatus;
            AI3D::VIEWER::AT3DViewInterface interface_(data, pOsgEngine, blockstatus);
            interface_.ResetImageLayer(imageLayerSet);
        }

        void  MWindow::send_update_overview(ReconstructionObject* object)
        {
            emit signal_update_overview(object);
        }

        void MWindow::send_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names)
        {
            emit signal_delete_photos(ids, names);
        }

        void MWindow::send_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name)
        {
            emit signal_delete_tiepoints(ids, name);
        }

        void MWindow::send_right_selected_images_from_3dview(std::vector<image_t>& images)
        {
            emit signal_right_selected_images_from_3dview(images);
        }

        void MWindow::send_selected_images_from_3dview(std::vector<image_t>& images)
        {
            emit signal_selected_images_from_3dview(images);
        }

        void MWindow::send_selected_tiles(std::vector<image_t>& tiles)
        {
            emit signal_selected_tiles(tiles);
        }

        void MWindow::RemoveItem()
        {
            // std::vector<osg::ref_ptr<CustomNode>>* GetPickedNode() ;
            if (pOsgEngine != nullptr)
            {
                std::vector<osg::ref_ptr<CustomNode>>* pickedNodes = pOsgEngine->GetPickedNode();

            }
        }

        std::vector<point3D_t> MWindow::getPickedNodeId()
        {
            std::vector<point3D_t> pickedNodeId;

            if (pOsgEngine != nullptr)
            {
                std::vector<int> ids;
                pOsgEngine->GetPickedElementIds(ids);
                for (int id : ids)
                {
                    pickedNodeId.push_back(static_cast<point3D_t>(id));
                }
            }

            return pickedNodeId;
        }

        std::vector<image_t> MWindow::getPickedPhotoNodeId()
        {
            std::vector<image_t> pickedPhotoNodeId;
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
            //  " " << this << std::dec << std::endl;

            if (pOsgEngine != nullptr)
            {
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                //  " " << this << std::dec << std::endl;

                std::vector<int> tmpID;
                pOsgEngine->GetPickedPhotoIds(tmpID);

#if 0
                std::vector<osg::ref_ptr<CustomNode>>* pickedNodes = pOsgEngine->GetPickedNode();
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                //  " " << this << std::dec << std::endl;

                if (pickedNodes != nullptr)
                {
                    for (auto t : *pickedNodes)
                    {
                    //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                    //      " imgid:" << t->m_iID << " " << t->m_strName << std::dec << std::endl;
                        if (t->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
                        {
                    //      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                    //          " imgid:" << t->m_iID << " " << t->m_strName << std::dec << std::endl;

                ///         if (t->m_iID != kInvalidImageId)
                            {
                    //          std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                    //              " imgid:" << t->m_iID << " " << t->m_strName << std::dec << std::endl;

                                pickedPhotoNodeId.push_back(t->m_iID);
                            }
                ///         else
                ///         {
                ///             std::cout << "inside/invalid " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << std::showbase <<
                ///                 " imgid:" << t->m_iID << " " << t->m_strName << " " << kInvalidImageId << std::dec << std::endl;

                ///         }
                        }
                    }
                }
#else
                for (auto& t : tmpID)
                {
                    pickedPhotoNodeId.push_back(t);
                }
#endif
            }

            return pickedPhotoNodeId;
        }

        bool MWindow::getPickedPhotoNodeId2(std::vector<image_t>& pickedPhotoNodeId)
        {
            if (pOsgEngine != nullptr)
            {
                std::vector<int> tmpID;
                if (!pOsgEngine->GetPickedPhotoIds(tmpID))
                {
                    return false;
                }

#if 0
                std::vector<osg::ref_ptr<CustomNode>>* pickedNodes = pOsgEngine->GetPickedNode();
                if (pickedNodes != nullptr)
                {
                    for (auto t : *pickedNodes)
                    {
                        if (t->GetElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
                        {
                            ///if(t->m_iID!= kInvalidImageId)
                                pickedPhotoNodeId.push_back(t->m_iID);
                        }
                    }
                }
#else
                for (auto& t : tmpID)
                {
                    pickedPhotoNodeId.push_back(t);
                }
#endif
            }
            else
                return false;

            return true;
        }

        std::vector<image_t> MWindow::getPickedTileNodeId()
        {
            std::vector<image_t> pickedPhotoNodeId;

            if (pOsgEngine != nullptr)
            {
                std::vector<int> tileIds;
                pOsgEngine->GetPickedTileIds(tileIds);
                for (int id : tileIds)
                {
                    pickedPhotoNodeId.push_back(static_cast<image_t>(id));
                }
            }

            return pickedPhotoNodeId;
        }

        bool MWindow::getPickedTileNodeId2(std::vector<image_t>& pickedTileNodeId)
        {
            if (pOsgEngine == nullptr)
            {
                return false;
            }
            std::vector<int> tileIds;
            if (!pOsgEngine->GetPickedTileIds(tileIds))
            {
                return false;
            }
            for (int id : tileIds)
            {
                pickedTileNodeId.push_back(static_cast<image_t>(id));
            }
            return true;
        }

        int MWindow::GetNumofNode()
        {
            return pOsgEngine->GetPickedNode()->size();
        }

        OsgEngine* MWindow::getOsgEngine()
        {
            return pOsgEngine;
        }

        void MWindow::Run2()
        {
            if(pOsgEngine)
                pOsgEngine->Run2();
        }

        bool MWindow::hasSceneData()
        {
            return bHasSceneData;
        }

        void MWindow::setSceneData()
        {
            bHasSceneData = true;
            if (setOsgEngineWorking.count(this) <= 0)
            {
                setOsgEngineWorking.insert(this);
                iOsgEngineWorkingNum++;

                {
                    std::ostringstream oss;
                    oss << "mw/setSceneData:" << iOsgEngineWorkingNum;
                    LOGI(oss.str());
                }
            }
            else
            {
                {
                    std::ostringstream oss;
                    oss << "mw/setSceneData(already existed):" << iOsgEngineWorkingNum;
                    LOGI(oss.str());
                }
            }
        }

        void MWindow::clearSceneData()
        {
            bHasSceneData = false;
            //pOsgEngine->RemoveScene();
            viewerWindow->setSceneData(nullptr);
            viewerWindow->frame();                                 

            
            if (setOsgEngineWorking.count(this) > 0)
            {
                setOsgEngineWorking.erase(this);
                if (iOsgEngineWorkingNum > 0)
                    iOsgEngineWorkingNum--;
                {
                    std::ostringstream oss;
                    oss << "mw/clearSceneData:" << iOsgEngineWorkingNum;
                    LOGI(oss.str());
                }
            }
            else
            {
                std::ostringstream oss;
                oss << "mw/clearSceneData(not exist):" << iOsgEngineWorkingNum;
                LOGI(oss.str());
            }
        }


        UserMatrixData::UserMatrixData()
        {
            bInitial = false;
        }

        UserMatrixData::~UserMatrixData()
        {

        }

        // note:check whether it is right in actual situation.
        void UserMatrixData::setCurrentMatrix(osg::Matrixd& matrixd)
        {
            // can be set to true directly?
            bInitial = true;
            lastMatrix = matrixd;
        }

        osg::Matrixd UserMatrixData::getLastMatrix(bool& lastMatrixIsValid)
        {
            lastMatrixIsValid = bInitial;
            return lastMatrix;
        }

        void UserMatrixData::setCurrentMatrixObject(QObject* pObject, osg::Matrixd& matrixd)
        {
            if (!pObject)
                return;

            UserMatrixData* pUserMatrixData = nullptr;
            QObjectUserData* pUserData = pObject->userData(Qt::UserRole + 100);
            if (!pUserData)
            {
            //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                pUserMatrixData = new UserMatrixData();
            }
            else
            {
                ///std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                //pUserMatrixData = dynamic_cast<UserMatrixData*>(pUserData);
                pUserMatrixData = static_cast<UserMatrixData*>(pUserData);
            //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << pObject << std::dec << " " << pUserData << " " << pUserMatrixData << std::endl;
                if (!pUserMatrixData)
                {
                //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    return;
                }
            }

            pUserMatrixData->setCurrentMatrix(matrixd);
            if (!pUserData)
            {
            //  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::hex << pObject << std::dec << " " << pUserMatrixData << std::endl;
                pObject->setUserData(Qt::UserRole + 100, pUserMatrixData);
            }
            else
            {
///             std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            }
        }

        osg::Matrixd UserMatrixData::getCurrentMatrixObject(QObject* pObject, bool& lastMatrixIsValid)
        {
            lastMatrixIsValid = false;
            if (!pObject)
            {
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                return osg::Matrixd();
            }

            UserMatrixData* pUserMatrixData = nullptr;
            QObjectUserData* pUserData = pObject->userData(Qt::UserRole + 100);
            if (!pUserData)
            {
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                return osg::Matrixd();
            }

            ///pUserMatrixData = dynamic_cast<UserMatrixData*>(pUserData);
            pUserMatrixData = static_cast<UserMatrixData*>(pUserData);

            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << std::hex << pObject << std::dec << " " << pUserData << " " << pUserMatrixData << std::endl;

            if (!pUserMatrixData || !pUserMatrixData->bInitial)
            {
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                return osg::Matrixd();
            }

            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " lastMatrix should be valid." << std::endl;

            lastMatrixIsValid = true;
            return pUserMatrixData->lastMatrix;
        }


        void TilingCallbackEvent::callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
        {
            //可拖任意面
            if (type == CALL_BACK_ROI_BOX_DRAG)
            {
                std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();

                for (auto it : *stPhoto)
                {
                    box_ = ABBox3d();
                    box_.min().x() = it.bbox.xMin();
                    box_.max().x() = it.bbox.xMax();
                    box_.min().y() = it.bbox.yMin();
                    box_.max().y() = it.bbox.yMax();
                    box_.min().z() = it.bbox.zMin();
                    box_.max().z() = it.bbox.zMax();
                    //std::cout << "ROI: " << it.ID << " " << it.name << " , XYZ: " << it.bbox.xMax() << " " << it.bbox.yMax() << " " << it.bbox.zMax() << std::endl;
                }

                if (this->pObject != nullptr)
                {
                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        {
                            std::cout << "======================Reseting boundingbox waiting=================" << std::endl;
                            OpenLoadingPromptV4("Reseting boundingbox ");
                            bool bRunFinished = false;

                                                                                                                                                     
                            auto savefunc = [&, this]()
                            {

                                int ret = AI3D::CORE::ReconstructionCommandSet::ResetBoundingBox(this->pObject, box_);

                                bRunFinished = true;                               

                                return ret;
                            };

                            QFuture<int> f1 = QtConcurrent::run(savefunc);

                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                            while (!bRunFinished)
                            {

                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                            int ret = f1.result();
                            //modify by zhaobf
                            CloseLoadingPromptV4();  //上移，否则影响tile和ROI的渲染

                            if (ret == AI3D_SUCCESS)
                            {

                                if (this->pMWindow != nullptr)
                                {

                                    //this->pMWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_ROI);
                                    this->pMWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_TILE);
                                    //AI3D::VIEWER::Tile3DViewInterface interface_(this->pObject, this->pMWindow->getOsgEngine());   //影响性能
                                    //interface_.BuildTilesNode();
                                    //interface_.BuildROINode();
                                    
                                    //modify by zhaobf
                                    {

                                        auto  tiles_custom_ = this->pObject->GetTilesCustom();
                                        std::set<std::string> tileset;
                                        for (auto& iter : tiles_custom_)
                                        {
                                            if (!iter.second.isempty)
                                            {
                                                tileset.insert(iter.first);
                                            }
                                        }                                        
                                    
                                        if(tileset.size())
                                        {

                                            std::vector<ST_BOUNDINGBOX> box(tileset.size());
                                            for (auto& iter : tileset)                                                                                    
                                            {
                                                if (!tiles_custom_.count(iter))
                                                    continue;

                                                auto tile = tiles_custom_.at(iter);
                                                ST_BOUNDINGBOX bbtemp;
                                                bbtemp.ID = tile.index_;

                                                bbtemp.name = tile.name_;
                                                bbtemp.type = 2;

                                                auto bbtile = tile.bb_;
                                                bbtemp.minXYZ.x() = bbtile.min().x();
                                                bbtemp.minXYZ.y() = bbtile.min().y();
                                                bbtemp.minXYZ.z() = bbtile.min().z();
                                                bbtemp.maxXYZ.x() = bbtile.max().x();
                                                bbtemp.maxXYZ.y() = bbtile.max().y();
                                                bbtemp.maxXYZ.z() = bbtile.max().z();
                                                box.push_back(bbtemp);
                                               
                                            }
                                          
                                            this->pMWindow->getOsgEngine()->RenderTiles(box);
                                        }
                                    
                                    }                                    

                                    this->pMWindow->send_update_overview(this->pObject);
                                }
                            }
                        }
                      
                        std::cout << "======================Reseting boundingbox ,end=================" << std::endl;
                    }
                    else
                    {
                        std::cout << "======================Reseting boundingbox waiting=================" << std::endl;
                        int ret = AI3D::CORE::ReconstructionCommandSet::ResetBoundingBox(this->pObject, box_);
                        if (ret == AI3D_SUCCESS)
                        {

                            if (this->pMWindow != nullptr)
                            {


                                this->pMWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_ROI);
                                this->pMWindow->getOsgEngine()->RemoveAll(ELEMENT_LAYER_TYPE::ELEMENT_TILE);
                                AI3D::VIEWER::Tile3DViewInterface interface_(this->pObject, this->pMWindow->getOsgEngine());
                                interface_.BuildROINode();
                                interface_.BuildTilesNode();
                                this->pMWindow->send_update_overview(this->pObject);

                            }

                        }
                        std::cout << "======================Reseting boundingbox ,end=================" << std::endl;
                    }
                }
            }
            else if (type == CALL_BACK_ROI_POLYGON_DRAG)
            {
                std::vector<PolygonBox>* stPhoto = (std::vector<PolygonBox>*)info.getEventInfo();

                for (auto it : *stPhoto)
                {
                    roi_height_range_.first = -DBL_MAX;
                    roi_height_range_.second = -DBL_MAX;
                    roi_height_range_.first = it.minHeight;
                    roi_height_range_.second = it.maxHeight;
                }
            }

        }
    }

}