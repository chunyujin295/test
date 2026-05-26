#include "Gui/BlockWgt.h"
#include "Gui/message_box.h"
#include <algorithm>
#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include <QtConcurrent>
#include <QStringList>
#include <QThreadPool>
#include "Gui/MohackerWin.h"
#include "Core/CoordinateSystem.h"
#include "Gui/BlockManager.h"
#include "Gui/ImportGcpDia.h"
#include "Gui/PosSigmaDia.h"
#include "Core/ControlPoint.h"
#include "Core/CoordinateSystem.h"
#include "Gui/ProjectManager.h"
#include "Core/Timer.h"
#include "Gui/ImportPosDia.h"
#include "Gui/ToolTip.h"
#include "Core/Types.h"
#include"Core/BlockObject.h"
#include"Gui/AddSigGcp.h"
#include "Util/Statistic.h"
#include "Util/Settings.h"
#include "Util/TaskProcess.h"
//#include "Gui/OTA.h"

//?chy InitGcpData
using namespace AI3D::CORE;

namespace AI3D
{
    namespace GUI
    {
    
        void BlockWgt::InitPhotoTabIsEdit()
        {

            ui->le_name->setReadOnly(true);
            ui->le_name->setText(QString());
            ui->le_path->setReadOnly(true);
            ui->le_path->setText(QString());
            ui->le_photo_ser_siz->setReadOnly(true);
            ui->le_photo_ser_siz->setText(QString());
            ui->le_pos_lon->setReadOnly(true);
            ui->le_pos_lon->setText(QString());
            ui->le_pos_lat->setReadOnly(true);
            ui->le_pos_lat->setText(QString());
            ui->le_pos_height->setReadOnly(true);
            ui->le_pos_height->setText(QString());
//#ifdef USE_AI3D_PROJ
//
//          ui->le_rotation->setReadOnly(true);
//          ui->le_rotation->setText(QString());
//          
//#endif


            ui->le_photogroup_name->setReadOnly(true);
            ui->le_photogroup_name->setText(QString());
            ui->le_photogroup_dir->setReadOnly(true);
            ui->le_photogroup_dir->setText(QString());
            ui->le_photogroup_num->setReadOnly(true);
            ui->le_photogroup_num->setText(QString());
            ui->le_photogroup_imagesize->setReadOnly(true);
            ui->le_photogroup_imagesize->setText(QString());
            ui->le_photogroup_camera->setReadOnly(true);
            ui->le_photogroup_camera->setText(QString());
            ///ui->le_k1_2->setReadOnly(true);
            ui->le_k1_2->setText(QString());
            ///ui->le_k2_2->setReadOnly(true);
            ui->le_k2_2->setText(QString());
            ///ui->le_k3_2->setReadOnly(true);
            ui->le_k3_2->setText(QString());
            ///ui->le_p1_2->setReadOnly(true);
            ui->le_p1_2->setText(QString());
            ///ui->le_p2_2->setReadOnly(true);
            ui->le_p2_2->setText(QString());
            ui->le_photogroup_sensorsize->setReadOnly(true);
            ui->le_photogroup_sensorsize->setText(QString());
            ui->le_focalength->setReadOnly(true);
            ui->le_focalength->setText(QString());
        }

        void BlockWgt::InitPhotoTabConnections()
        {
            my_Progress = new ProgressCom(this);
            Qt::WindowFlags flags = Qt::Dialog;
            my_Progress->setWindowFlags(flags);
            QDesktopWidget* desktopWidget = QApplication::desktop();
            QRect applicationRect = desktopWidget->screenGeometry();
            int parentWidth = applicationRect.width();
            int parentHeight = applicationRect.height();
            int proWidth = my_Progress->width();
            int proHeight = my_Progress->height();
            QPoint movePoint(parentWidth / 2 - proWidth / 2, parentHeight / 2 - proHeight / 2);

            my_Progress->move(movePoint);
            my_Progress->hide();

            connect(ui->btn_addsig, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_AddPhotoFile_Clicked, Qt::QueuedConnection);
            connect(ui->btn_adddir, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_AddPhotoDir_Clicked, Qt::QueuedConnection);
            connect(ui->btn_push_removePgtable, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_DelPhoto_Clicked, Qt::QueuedConnection);
            /*if (0)
            {
                connect(viewWidget_ui, &ViewWidget::update_delete_image_, this, &BlockWgt::slot_delete_item);
            }*/
            connect(ui->btn_addpos, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_AddPos_Clicked, Qt::QueuedConnection);
            connect(ui->btn_delpos, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_DelPos_Clicked, Qt::QueuedConnection);
        
            connect(ui->tableView_photogroup, &QTableView::customContextMenuRequested, this, &BlockWgt::Slot_QTableWidgetPhotoGroup_CustomContextMenuRequested);
            connect(ui->tableView_photogroup, &MoPhotoTableWidget::itemModified, this, &BlockWgt::Slot_PhotoGroupItemModified);
            connect(ui->tableView_photogroup, &QTableView::clicked, this, &BlockWgt::Slot_TableView_RealClicked);


            
            connect(ui->tableView_photo_pos, SIGNAL(clicked(QModelIndex)), this, SLOT(Slot_TableWidget_Photo_Pos_RealClicked(QModelIndex)));
            connect(this, &BlockWgt::Signal_Photo_Progress, my_Progress, &ProgressCom::setValue);
            
            
        
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                ui->label_photo_open->setText(QApplication::translate("label_photo_open", "<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline;color:#ffffff;\">打开</span></a></p></body></html>", nullptr));
            }
            else
            {
                ui->label_photo_open->setText(QApplication::translate("label_photo_open", "<html><head/><body><p><a href=\" \"><span style=\" text-decoration: underline;color:#ffffff;\">Open</span></a></p></body></html>", nullptr));
            }
            connect(ui->label_photo_open, &QLabel::linkActivated, this, &BlockWgt::Slot_LinkActivated_Label_Photo_Open);

            connect(ui->le_k1_2, SIGNAL(returnPressed()), this, SLOT(ChangeDistorion1));
            connect(ui->le_k2_2, SIGNAL(returnPressed()), this, SLOT(ChangeDistorion1));
            connect(ui->le_k3_2, SIGNAL(returnPressed()), this, SLOT(ChangeDistorion1));
            connect(ui->le_p1_2, SIGNAL(returnPressed()), this, SLOT(ChangeDistorion1));
            connect(ui->le_p2_2, SIGNAL(returnPressed()), this, SLOT(ChangeDistorion1));
            if (BlockObject::isChineseVersion())
            {
                ui_action_deletephotogroup_ = new QAction(tr("删除"), ui->tableView_photogroup);
                ui_action_clearphotogroup_pose_ = new QAction(tr("清除位姿"), ui->tableView_photogroup);
            }
            else
            {
                ui_action_deletephotogroup_ = new QAction(tr("Delete"), ui->tableView_photogroup);
                ui_action_clearphotogroup_pose_ = new QAction(tr("Clear Pose"), ui->tableView_photogroup);
            }

            
            ui_menuphotogroup_rightClick_selectRows = new QMenu(ui->tableView_photogroup);
            ui_menuphotogroup_rightClick_selectRows->addAction(ui_action_deletephotogroup_);
            ui_menuphotogroup_rightClick_selectRows->addAction(ui_action_clearphotogroup_pose_);
            
            ui_action_clearphotogroup_pose_->setEnabled(false);
            connect(ui_action_deletephotogroup_, &QAction::triggered, this, &BlockWgt::SlotDeletePhotoGroup);
            connect(ui_action_clearphotogroup_pose_, &QAction::triggered, this, &BlockWgt::SlotClearPoseByGroup);

            if (BlockObject::isChineseVersion())
            {
                ui_action_deletephotopos_ = new QAction(tr("删除"), ui->tableView_photo_pos);
                ui_action_clearpos_ = new QAction(tr("清除位姿"), ui->tableView_photo_pos);
                ui->label_4->setText("影像组");
            }
            else
            {
                ui_action_deletephotopos_ = new QAction(tr("Delete"), ui->tableView_photo_pos);
                ui_action_clearpos_ = new QAction(tr("Clear Pose"), ui->tableView_photo_pos);
                // The follow-up operation can be ignored if not dynamically switching.
                // ui->label_4->setText("Photogroup");
            }

            ui_menuphotopos_rightClick_selectRows = new QMenu(ui->tableView_photo_pos);
            ui_menuphotopos_rightClick_selectRows->addAction(ui_action_deletephotopos_);
            ui_menuphotopos_rightClick_selectRows->addAction(ui_action_clearpos_);
            connect(ui_action_deletephotopos_, &QAction::triggered, this, &BlockWgt::SlotDeletePhoto);
            connect(ui_action_clearpos_, &QAction::triggered, this, &BlockWgt::SlotClearPhotoPose);
            ui_action_clearpos_->setEnabled(false);
            
            connect(ui->label_view_report, &QLabel::linkActivated, this, &BlockWgt::slot_linkActivated_label_view_report);
            
            connect(ui->tableView_photo_pos, &QTableWidget::customContextMenuRequested, this, &BlockWgt::Slot_QTableWidgetPhotoPos_CustomContextMenuRequested);
        }
        //需完善的地是多选情形下，
        void BlockWgt::Slot_QTableWidgetPhotoGroup_CustomContextMenuRequested(const QPoint& pos)
        {
            QItemSelectionModel* model_selection = ui->tableView_photogroup->selectionModel();
            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() == 0)
                return;

            std::set<group_t> groupids;
            bool bhaspose = false;//决定是否置灰显示
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            for (auto groupidx : IndexList)
            {
                auto groupid = ui->tableView_photogroup->getGroupIdByRow(groupidx.row());
                if (groupids.count(groupid) == 1)
                {
                    continue;
                }
                groupids.insert(groupid);

                if (block_data_->GetCurrentAT()->HasPositionImages() && !bhaspose)
                {
                    bhaspose = true;
                }

            }

            if (bhaspose)
            {
                ui_action_clearphotogroup_pose_->setEnabled(true);
            }
            
            QModelIndex index = *IndexList.begin();
            Slot_TableView_Clicked(index);
            ui_menuphotogroup_rightClick_selectRows->exec(QCursor::pos());
            
        }

        void BlockWgt::Slot_QTableWidgetPhotoPos_CustomContextMenuRequested(const QPoint& pos)
        {
            QItemSelectionModel* model_selection = ui->tableView_photo_pos->selectionModel();
            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() == 0)
                return;

            std::set<image_t> imageids;
            bool bhaspose = false;

            for (auto imgidx : IndexList)
            {
                auto imageid = ui->tableView_photo_pos->getImageIdByRow(imgidx.row());
                
                auto imageinfo = block_data_->GetCurrentAT()->GetImage(imageid);
                if (imageinfo.HasPosition() && !bhaspose)
                {
                    bhaspose = true;
                    break;
                }

            }
            if (bhaspose)
            {
                ui_action_clearpos_->setEnabled(true);
            }
            ui_menuphotopos_rightClick_selectRows->exec(QCursor::pos());
            /*QModelIndex index = ui->tableView_photo_pos->indexAt(pos);
            if (index.isValid())
            {
                ui_menuphotopos_rightClick_selectRows->exec(QCursor::pos());
            }*/
        }

        void BlockWgt::Slot_PhotoGroupItemModified(int row, int col, const QString& text)
        {
            QModelIndex index = ui->tableView_photogroup->currentIndex();
            if (!index.isValid())
            {
                return;
            }
            auto groupid = ui->tableView_photogroup->getGroupIdByRow(index.row());
            auto group = block_data_->GetGroup(groupid);
            if (col == PGSENSORSIZE_COL)
            {
                if (group.GetCameraMutual().GetCameraName() != "")
                {
                    if(text.contains(UNDEFINEDSTR,Qt::CaseInsensitive))
                        block_data_->UpdateCameraInfo(cam_para_e::SENSOR_SIZE, UNDEFINEDVAL, group.GetId());
                    else                
                        block_data_->UpdateCameraInfo(cam_para_e::SENSOR_SIZE, text.toDouble(), group.GetId());
                    PopulatePhotoGroupTable();
                
                    /*{
                        ui->le_photogroup_sensorsize->setText(text);
                    }*/
                    SetModifityXml();
                    
                }
                
            }
            
            
            if( col == PGFOCALLENGTH_COL)
            {
                int id = col - PGSENSORSIZE_COL;
                    //改写atdata数据
                    if (group.GetCameraMutual().GetCameraName() != "")
                    {
                        block_data_->UpdateCameraInfo(cam_para_e::FOCAL, text.toDouble(), group.GetId());
                        PopulatePhotoGroupTable();
                        //ui->le_focalength->setText(text);
                        SetModifityXml();
                    }

            }
                //UpdatePhotoDetailStatus();
            
        }

        

        void BlockWgt::ChangeDistorion1()
        {
            QObject* obj = qobject_cast<QLineEdit*>(this->sender());
            QString itemstr = ui->le_k1_2->text();
            double itemval = itemstr.toDouble();
            QModelIndex index = ui->tableView_photo_pos->currentIndex();
            if (!index.isValid())
            {

                return;
            }
            auto groupid = ui->tableView_photogroup->getGroupIdByRow(index.row());
            auto group = block_data_->GetGroup(groupid);
            if (obj == ui->le_k1_2)
            {


                block_data_->UpdateCameraInfo(cam_para_e::K1, itemstr.toDouble(), groupid);
                PopulatePhotoGroupTable();
                if (itemstr.toDouble() < 0)
                {
                    ui->le_k1_2->setText(QString(STR(undefined)));
                }
                else
                {
                    ui->le_k1_2->setText(itemstr);
                }
                SetModifityXml();

            }
            else if (obj == ui->le_k2_2)
            {

            }
            else if (obj == ui->le_k3_2)
            {

            }
            else if (obj == ui->le_p1_2)
            {

            }
            else if (obj == ui->le_p2_2)
            {

            }
        }
        void BlockWgt::InitTableViewPhotoGroup()
        {
            ui->tableView_photogroup->viewport()->installEventFilter(new AToolTipper((QObject*)ui->tableView_photogroup));
            ui->tableView_photogroup->horizontalHeader()->viewport()->installEventFilter(new AToolTipper(ui->tableView_photogroup->horizontalHeader()));
            
            ui->tableView_photogroup->SetMode(0);
            ui->tableView_photogroup->InitHeader();
            ui->tableView_photogroup->setContextMenuPolicy(Qt::CustomContextMenu);
            ui->tableView_photogroup->setHeaderLabelsMode();
            
            
            
            
        }
        
        void BlockWgt::InitTableWidgetPosList()
        {
            ui->tableView_photo_pos->viewport()->installEventFilter(new AToolTipper((QObject*)ui->tableView_photo_pos));
            ui->tableView_photo_pos->horizontalHeader()->viewport()->installEventFilter(new AToolTipper(ui->tableView_photo_pos->horizontalHeader()));

            
            ui->tableView_photo_pos->SetMode(1);
            ui->tableView_photo_pos->InitHeader();
            ui->tableView_photo_pos->setContextMenuPolicy(Qt::CustomContextMenu);
            ui->tableView_photo_pos->setHeaderLabelsMode();
            
            
        }
        //删除操作与3dview联动的
        /*void BlockWgt::slot_delete_item(const point3D_t& id,const QString& name)
        {
            item_deleted_happened_in3dview_ = true;
            if (name == "tiepoints")
            {
                block_data_->GetCurrentATMutual()->DeletePoint3D(id);
            
            }
            else
            {
                const std::vector<image_t> ids(1, (image_t)id);
                
                const group_t group_id = block_data_->GetCurrentATMutual()->GetImageMutual(*item_select_).GetPhotoGroupIDMutual();;
                    if (block_data_->PhotoGroupHasElement(group_id))
                    {
                        CommonDelDia commondia;
                        commondia.SetInfor(tr("Some selected photos are referenced in cotrol points.Do you really want to \n remove them?"));
                        if (commondia.exec() == QDialog::Rejected)
                        {
                            return;
                        }
                        
                        isupdategcp = true;
                    }

                    std::set<image_t> remids; 
                    remids.insert(id);
                block_data_->RemoveImages(remids);
            
            }
        
            QString msg = "RegisteredPhotos/Photos: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumRegImages())) + "/"
                + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumImages())) +
                "    Points: " + QString::fromStdString(std::to_string(block_data_->GetCurrentATMutual()->GetNumPoints3D()));
            viewWidget_ui->addTextEditData(msg);
            
            SetModifityXml();
        }*/
        
        void BlockWgt::SetPhotoTabEditable(bool be)
        {
            bool breadonly = !be;
            /*
            if(breadonly)
                ui->tableView_photogroup->setEditTriggers(QAbstractItemView::NoEditTriggers);*/
            if (ui_action_clearphotogroup_pose_ != nullptr)
            {
                ui_action_clearphotogroup_pose_->setDisabled(breadonly);
            }
            if (ui_action_deletephotogroup_ != nullptr)
            {
                ui_action_deletephotogroup_->setDisabled(breadonly);
            }

            if (ui_action_deletephotopos_ != nullptr)
            {
                ui_action_deletephotopos_->setDisabled(breadonly);
            }
            if (ui_action_clearpos_ != nullptr)
            {
                ui_action_clearpos_->setDisabled(breadonly);
            }
            
            ui->le_name->setReadOnly(breadonly);
            
            ui->le_path->setReadOnly(breadonly);
            
            ui->le_photo_ser_siz->setReadOnly(breadonly);
        
            ui->le_pos_lon->setReadOnly(breadonly);
            
            ui->le_pos_lat->setReadOnly(breadonly);
            
            ui->le_pos_height->setReadOnly(breadonly);
//#ifdef USE_AI3D_PROJ
//          ui->le_rotation->setReadOnly(breadonly);        
//#endif

            ui->le_photogroup_name->setReadOnly(breadonly);
            
            ui->le_photogroup_dir->setReadOnly(breadonly);
            
            ui->le_photogroup_num->setReadOnly(breadonly);
            
            ui->le_photogroup_imagesize->setReadOnly(breadonly);
            
            ui->le_photogroup_camera->setReadOnly(breadonly);
            
        /// ui->le_k1_2->setReadOnly(breadonly);
            
        /// ui->le_k2_2->setReadOnly(breadonly);
            
        /// ui->le_k3_2->setReadOnly(breadonly);
            
        /// ui->le_p1_2->setReadOnly(breadonly);
            
        /// ui->le_p2_2->setReadOnly(breadonly);
            
            ui->le_photogroup_sensorsize->setReadOnly(breadonly);
            
            ui->le_focalength->setReadOnly(breadonly);
            
        }
        void BlockWgt::UpdatePhotoDetailStatus()
        {
            
            //
            QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
            QModelIndex currentphotoindex = ui->tableView_photo_pos->currentIndex();

            bool photogroupselected = false;
            bool photoselected = false;
            if (currentgroupindex.isValid())
                photogroupselected = true;;
            if (currentphotoindex.isValid())
                photoselected = true;;
            
            std::shared_ptr<AI3D::CORE::ATData> at_data;
            ui->widget->setVisible(true);
            at_data = block_data_->GetCurrentATMutual();
            if (!photoselected)//默认显示photogroup
            {
            
                // hide photo info
                ui->wgt_photo_priview->setVisible(false);
                // show photogroup info
                ui->wgt_form_photogroup->setVisible(true);
                if (BlockObject::isChineseVersion())
                {
                    ui->label_Group->setText("影像组");
                }
                else
                {
                    ui->label_Group->setText("Photogroup");
                }
                if (photogroupselected)
                {
                    auto groupid = ui->tableView_photogroup->getGroupIdByRow(currentgroupindex.row());
                    auto group = block_data_->GetGroup(groupid);

                    ui->le_photogroup_name->setText(ui->tableView_photogroup->getItem(currentgroupindex.row(), PGNAME_COL)->text());
                    ui->le_photogroup_sensorsize->setText(ui->tableView_photogroup->getItem(currentgroupindex.row(), PGSENSORSIZE_COL)->text());
                    ui->le_focalength->setText(ui->tableView_photogroup->getItem(currentgroupindex.row(), PGFOCALLENGTH_COL)->text());
                    QString groupPath = QString::fromUtf8(at_data->GetImage(*group.GetGroupImageIds().begin()).GetPath().c_str());
                    ui->le_photogroup_dir->setText(groupPath);
                    ui->le_photogroup_num->setText(ui->tableView_photogroup->getItem(currentgroupindex.row(), PGPHOTOCOUNT_COL)->text());
                    AI3D::CORE::Camera& thecamera = const_cast<AI3D::CORE::Camera&> (at_data->GetCamera(group.GetCamera().GetCameraId()));
                    ui->le_photogroup_imagesize->setText(QString("%1 * %2").arg(QString::number(thecamera.GetWidth())).arg(QString::number(thecamera.GetHeight())));
                    ui->le_photogroup_camera->setText(QString::fromUtf8((thecamera.GetMake() + TABSTRING + thecamera.GetMakeModel()).c_str()));
                    auto vec_param = thecamera.GetParams();
                    ui->le_k1_2->setText(QString::number(vec_param.at(4)));
                    ui->le_k2_2->setText(QString::number(vec_param.at(5)));
                    ui->le_k3_2->setText(QString::number(vec_param.at(8)));
                    ui->le_p1_2->setText(QString::number(vec_param.at(6)));
                    ui->le_p2_2->setText(QString::number(vec_param.at(7)));

                }
                else
                    InitPhotoTabIsEdit();
            }
            else
            {
                
                // hide photo info
                ui->wgt_photo_priview->setVisible(true);
                // show photogroup info
                ui->wgt_form_photogroup->setVisible(false);
                if (BlockObject::isChineseVersion())
                {
                    ui->label_Group->setText("影像");
                }
                else
                {
                    ui->label_Group->setText("Photo detail");
                }
                ui->le_name->setText(ui->tableView_photo_pos->getItem(currentphotoindex.row(), PHOTONAME_COL)->text());
                ui->le_path->setText(ui->tableView_photo_pos->getItem(currentphotoindex.row(), PHOTODIR_COL)->text());
                ui->le_photo_ser_siz->setText(ui->tableView_photo_pos->getItem(currentphotoindex.row(), PHOTODIR_COL)->text());
                Slot_Check_Preview(true);
                QString photo_path = QString(ui->le_path->text()).append("/%1").arg(ui->le_name->text());
                QFileInfo info(photo_path);
                if (!info.exists())
                {
                    ui->le_photo_ser_siz->setText("");//"File not exist!!"
                    //return;
                }
                else
                {
                    qint64 sizb = info.size();
                    QString strSiz;
                    int MB = 1024 * 1024;
                    int KB = 1024;
                    if (sizb > MB)
                        strSiz = QString::number(1.0f * sizb / MB, 'f', 2) /*+ QString(" MB")*/;
                    else
                        strSiz = QString::number(1.0f * sizb / KB, 'f', 2) /*+ QString(" KB")*/;
                    if (sizb > 0)
                    {
                        ui->le_photo_ser_siz->setText(strSiz);
                    }
                }
//#ifdef USE_AI3D_PROJ
//              //此处后续需要改为跟cc一致的
//              if (block_data_->GetBlockSRS().type != coord_system_type_e::GEOGRAPHIC)
//              {
//                  ui->label_23->setText("X");
//                  ui->label_24->setText("Y");
//                  ui->label_25->setText("Z");
//                  ui->comboBox->clear();
//                  std::string srstext = block_data_->GetBlockSRS().definition;
//                  QString qsrstext = QString::fromStdString(srstext);
//                  ui->comboBox->addItem(qsrstext);
//          }
//              else
//              {
//                  if (BlockObject::isChineseVersion())
//                  {
//                      ui->label_23->setText("经度");
//                      ui->label_24->setText("纬度");
//                      ui->label_25->setText("高度");
//                  }
//                  else
//                  {
//                      ui->label_23->setText("Longitude");
//                      ui->label_24->setText("Latitude");
//                      ui->label_25->setText("Height");
//                  }
//                  ui->comboBox->clear();
//                  ui->comboBox->addItem(CoordinateDescriptor::GetSRSFromDefinition("EPSG:4326").name.c_str());
//              }
//#else

                //at_data->get
                if (block_data_->GetBlockSRS().type == coord_system_type_e::LOCAL)
                {
                    ui->label_23->setText("X");
                    ui->label_24->setText("Y");
                    ui->label_25->setText("Z");
                    ui->comboBox->clear();
                    ui->comboBox->addItem(QString::fromStdString(NOTGEOREFERENCED));
                }
                else
                {
                    if (BlockObject::isChineseVersion())
                    {
                        ui->label_23->setText("经度");
                        ui->label_24->setText("纬度");
                        ui->label_25->setText("高度");
                    }
                    else
                    {
                        ui->label_23->setText("Longitude");
                        ui->label_24->setText("Latitude");
                        ui->label_25->setText("Height");
                    }
                    ui->comboBox->clear();
                    ui->comboBox->addItem(CoordinateDescriptor::GetSRSFromDefinition("EPSG:4326").name.c_str());
                }
//#endif            

                auto item_photo_pos = ui->tableView_photo_pos->getItem(currentphotoindex.row(), PHOTOPOS_COL);
                if (item_photo_pos != nullptr)
                {
                    QStringList strList = item_photo_pos->text().split(TABSTRING);
                    if (strList.size() >= 3) {
                        ui->le_pos_lon->setText(strList.at(0));
                        ui->le_pos_lat->setText(strList.at(1));
                        ui->le_pos_height->setText(strList.at(2));
                    }
//#ifdef USE_AI3D_PROJ
//                  ui->le_rotation->setReadOnly(breadonly);
//#endif
                }
                //增加3dview显示
                *item_select_ = ui->tableView_photo_pos->getImageIdByRow(currentphotoindex.row());
            }
            
        }
        
        
        void BlockWgt::PopulatePhotoGroupTable()
        {
            
            
            ui->tableView_photogroup->setUpdatesEnabled(false);
            QModelIndex currentindex = ui->tableView_photogroup->currentIndex();
            ui->tableView_photogroup->clearData();
            
            int rownum = 0;
            for (auto& it : block_data_->GetPhotoGroups())
            {
                photogroup_list_item_st infolist;
                group_t i = it.first;
                AI3D::CORE::PhotoGroup& group = block_data_->GetGroup(i);
                infolist.id_ = group.GetId();
                infolist.photogroupname_ = group.GetName();
                infolist.photocount_ = group.GetNumImages();
                infolist.sensorsize_ = group.GetCamera().GetSensorSize();
                infolist.focalmm_ = group.GetCamera().GetFocalLengthMM();
                infolist.focal35mm_ = group.GetCamera().GetFocalLengthIn35mm();
                

                ui->tableView_photogroup->appendRowData(infolist);

            }
            if (currentindex != QModelIndex())
            {
                ui->tableView_photogroup->setCurrentIndex(currentindex);
            }
            ui->tableView_photogroup->setUpdatesEnabled(true);
            ui->tableView_photogroup->update();
            UpdatePhotoDetailStatus();
            //QApplication::processEvents();

        }


        
        void BlockWgt::ChangeDistorion(QString itemstr)
        {
            return;

            
        }

        
        void BlockWgt::SlotDeletePhotoGroup()
        {
            QItemSelectionModel* model_selection = ui->tableView_photogroup->selectionModel();
            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() == 0)
                return;

            std::set<group_t> groupids;
            bool bhasgcp = false;

            for (auto groupidx : IndexList)
            {
                auto groupid = ui->tableView_photogroup->getGroupIdByRow(groupidx.row());
                if (groupids.count(groupid) == 1)
                {
                    continue;
                }
                groupids.insert(groupid);

                if (block_data_->PhotoGroupHasElement(groupid) && !bhasgcp)
                {
                    bhasgcp = true;
                }

            }
            if (bhasgcp)
            {
                //弹窗提示
                CommonDelDia commondia;
                commondia.SetInfor(tr("Some selected photos are referenced in cotrol points.Do you really want to \n remove them?"));
                if (commondia.exec() == QDialog::Rejected)
                {
                    return;
                }
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            std::vector<group_t> groupidsvec(groupids.begin(), groupids.end());
            block_data_->RemovePhotoGroup(groupidsvec);
            ui->tableView_photogroup->setCurrentIndex(QModelIndex());
            ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
            PopulatePhotoGroupTable();
            ui->tableView_photo_pos->clearData();
            UpdateWgtAndProjStatus();
            QApplication::restoreOverrideCursor();
        }
        


        void BlockWgt::SlotClearPoseByGroup()
        {
            QItemSelectionModel* model_selection = ui->tableView_photogroup->selectionModel();
            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() == 0)
                return;
            /*CommonDelDia commondia;
            commondia.resize(682, 161);
            commondia.SetInfor(tr("Do you really want to remove the position?"));
            if (commondia.exec() == QDialog::Rejected)
            {
                return;
            }*/
            

            std::set<group_t> groupids;
            bool bhasgcp = false;

            for (auto groupidx : IndexList)
            {
                auto groupid = ui->tableView_photogroup->getGroupIdByRow(groupidx.row());
                if (groupids.count(groupid) == 1)
                {
                    continue;
                }
                groupids.insert(groupid);

                if (block_data_->PhotoGroupHasElement(groupid) && !bhasgcp)
                {
                    bhasgcp = true;
                }

            }
            

            if (bhasgcp/*block_data_->GetCurrentATMutual()->HasControlPoints()*/)
            {
                isupdategcp = true;
            }
            
            image_t idtemp = 0;
            //std::cout << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetName() << " posbeforeclearpose " << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetPosition() << std::endl;
            
            

            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            std::vector<group_t> groupidsvec(groupids.begin(), groupids.end());
            block_data_->ClearPoses(groupidsvec);
            //std::cout << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetName() << " posafter " << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetPosition() << std::endl;
            auto currentindex = IndexList.at(0);
            
            ui->tableView_photogroup->SetSelectionChanged(true);
            //Slot_TableView_Clicked(currentindex);
            ui->tableView_photogroup->setCurrentIndex(currentindex);
            PopulatePhotoGroupTable();
            auto groupid = ui->tableView_photogroup->getGroupIdByRow(currentindex.row());

            auto& group = block_data_->GetGroup(groupid);
            PopulatePosTableWgt(group);
            UpdateWgtAndProjStatus();
            QApplication::restoreOverrideCursor();
            LOGI(AI3D::CORE::String::StringPrintf("%sDel Pos ", block_data_->GetName()));
        }

        void BlockWgt::SlotDeletePhoto()
        {
            QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
            if (!currentgroupindex.isValid())
            {
                return;
            }
            auto groupid = ui->tableView_photogroup->getGroupIdByRow(currentgroupindex.row());
            
            
            QItemSelectionModel* model_selection = ui->tableView_photo_pos->selectionModel();
            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() == 0)
                return;
        
            std::set<image_t> imageids;
            bool bhasgcp = false;
                 
            for (auto imgidx : IndexList)
            {
                auto imageid = ui->tableView_photo_pos->getImageIdByRow(imgidx.row());
                if (imageids.count(imageid) == 1)
                {
                    continue;
                }
                imageids.insert(imageid);
                
                auto imageinfo = block_data_->GetCurrentAT()->GetImage(imageid);
                if (imageinfo.HasGCPs() && !bhasgcp)
                {
                    bhasgcp =  true;
                }
                
            }
            if (bhasgcp)
            {
                //弹窗提示
                CommonDelDia commondia;
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    commondia.SetInfor(tr("某些已选影像在控制点中有引用，确认需要删除吗？"));
                }
                else
                {
                    commondia.SetInfor(tr("Some selected photos are referenced in cotrol points.Do you really want to \n remove them?"));
                }
                if (commondia.exec() == QDialog::Rejected)
                {
                    return;
                }
            }
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            block_data_->RemoveImages(imageids);
            QApplication::restoreOverrideCursor();
            /*for (auto imageid : imageids)
            {
                block_data_->RemoveImage(imageid);
            }*/
            *item_select_ = kInvalidImageId;
            

            //ui->tableView_photogroup->setCurrentIndex(QModelIndex());
            ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
            PopulatePhotoGroupTable();
            auto& group = block_data_->GetGroup(groupid);
            PopulatePosTableWgt(group);
            LOGI(AI3D::CORE::String::StringPrintf("Delete %s %s", block_data_->GetName(), group.GetName()));
            ////删除选中的tablewidget行
        
            UpdateWgtAndProjStatus();
            *item_select_ = kInvalidImageId;
        }
        void BlockWgt::SlotClearPhotoPose()
        {
            QItemSelectionModel* selections = ui->tableView_photo_pos->selectionModel();
            //  //获取被选中的指针列表
            QModelIndexList IndexList = selections->selectedIndexes();
            if (IndexList.size() == 0)
                return;
            std::set<image_t> image_ids;

            foreach(QModelIndex index, IndexList)
            {
                auto imageid = ui->tableView_photo_pos->getImageIdByRow(index.row());
                image_ids.insert(imageid);
            }
            if (image_ids.empty())
            {
                return;
            }
            QItemSelectionModel* group_model_selection = ui->tableView_photogroup->selectionModel();
            QModelIndexList group_IndexList = group_model_selection->selectedIndexes();
            if (group_IndexList.size() == 0)
                return;
            /*CommonDelDia commondia;
            commondia.resize(682, 161);
            commondia.SetInfor(tr("Do you really want to remove the position?"));
            if (commondia.exec() == QDialog::Rejected)
            {
                return;
            }*/
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            if (block_data_->GetCurrentATMutual()->HasControlPoints())
            {
                isupdategcp = true;
            }
            
            block_data_->ClearPoses(image_ids);

            


            ui->tableView_photo_pos->SetSelectionChanged(true);
            auto currentindex = IndexList.at(0);
            ui->tableView_photo_pos->setCurrentIndex(currentindex);
            auto imageid = ui->tableView_photo_pos->getImageIdByRow(currentindex.row());
            /*std::cout << imageid  << " 888 " << block_data_->GetCurrentATMutual()->GetImage(imageid).GetName() << " image  " << *image_ids.cbegin() 
                <<" 88 " <<  block_data_->GetCurrentATMutual()->GetImage(*image_ids.cbegin()).GetName() << std::endl;*/

            auto groupid = block_data_->GetCurrentATMutual()->GetImage(imageid/**image_ids.cbegin()*/).GetPhotoGroupID();
            ui->tableView_photogroup->SetSelectionChanged(true);
            for (auto groupindex : group_IndexList)
            {
                if (ui->tableView_photogroup->getGroupIdByRow(groupindex.row()) == groupid)
                {
                    
                    ui->tableView_photogroup->setCurrentIndex(groupindex);

                    PopulatePhotoGroupTable();
                    break;
                }
            }
            auto& group = block_data_->GetGroup(groupid);
            PopulatePosTableWgt(group);
            UpdateWgtAndProjStatus();

    
            LOGI(AI3D::CORE::String::StringPrintf("%sDel Pos ", block_data_->GetName()));
        }
        

        //显示pos列表
        void BlockWgt::PopulatePosTableWgt(AI3D::CORE::PhotoGroup& group)
        {
            //std::cout << block_data_->GetNumPhotoGroup() << " " << ui->tableView_photogroup->currentIndex().row() << std::endl;

            if (block_data_->GetNumPhotoGroup() == 0 /*|| ui->tableView_photogroup->currentIndex()== QModelIndex()*/)
            {
                ui->tableView_photo_pos->clearData();
;               return;
            }
            ui->tableView_photo_pos->setUpdatesEnabled(false);
            QModelIndex currentindex = ui->tableView_photo_pos->currentIndex();
            ui->tableView_photo_pos->clearData();
            
            std::shared_ptr<AI3D::CORE::ATData> at_data;
            at_data = block_data_->GetCurrentATMutual();

            auto& images_set = group.GetGroupImageIds();
            auto *images_ = &at_data->GetImages();
            

            QList<QStringList> AllPosList;

            

            std::vector<image_t> image_ids;
            for (const auto& image_id : images_set)
            {
                image_ids.push_back(image_id);
            }

            image_t idtemp = 0;
            //std::cout << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetName() << " line812 " << block_data_->GetCurrentATMutual()->GetImage(idtemp).GetPosition() << std::endl;

            int image_count = image_ids.size();
            std::vector<Eigen::Vector3d> poses(image_count, { -DBL_MAX,-DBL_MAX, -DBL_MAX });
            std::vector<Eigen::Matrix3d> rotations(image_count, Eigen::Matrix3d::Zero());
            for (int i = 0; i < image_ids.size(); i++)
            {
                auto imageinfo = images_->find(image_ids[i])->second;//images_[image_ids[i]];
                auto pos = imageinfo.GetPosition();
                auto rotation = imageinfo.GetRotationMatrix();
                poses[i] = pos;
                rotations[i] = (rotation);
            }
            
            CoordinateTransformer::TransformRotation(image_count, poses, rotations,
                block_data_->GetBlockSRS(),
                CoordinateDescriptor::GetSRSFromDefinition("EPSG:4326"));

            for (int i = 0; i < image_ids.size(); i++)
            {
                auto imageinfo = images_->find(image_ids[i])->second;// images_[image_ids[i]];
                QStringList posList;
                photopose_list_item_st infolist;
                infolist.status_ = pose_status_e::POSE_ST_UNKNOWN;
                //QString posestatus = "Unknown";
                //if (imageinfo.IsRegistered())
                if(imageinfo.HasPosition() && imageinfo.HasRotationMatrix())
                    infolist.status_ = pose_status_e::POSE_ST_COMPLETED;
                infolist.image_id_ = imageinfo.GetImageId();
                infolist.photo_dir_ = imageinfo.GetPath();
                infolist.photo_name_ = imageinfo.GetName();
            
                if (imageinfo.HasPosition() )
                {
                    QString posstr = QString::number(poses[i].x(), 'f', LONG_ERROR_PRECISION) + TABSTRING
                        + QString::number(poses[i].y(), 'f', LONG_ERROR_PRECISION) + \
                        TABSTRING + QString::number(poses[i].z(), 'f');

                    infolist.posvalus_str_ = qstr2str(posstr);

                }
                else
                {
                    QString posstr = " ";

                    infolist.posvalus_str_ = qstr2str(posstr);
                }


                ui->tableView_photo_pos->appendRowData(infolist);
                
            }
            if (currentindex != QModelIndex())
            {
                ui->tableView_photo_pos->setCurrentIndex(currentindex);
            }
            ui->tableView_photo_pos->setUpdatesEnabled(true);
            ui->tableView_photo_pos->update();
            UpdatePhotoDetailStatus();
        }

        
        void BlockWgt::MakePriview()
        {
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            block_data_->GetCurrentATMutual()->GenPreviewImages(block_data_->GetPath());
            return;     
        }

        void BlockWgt::MakePriviewImage()
        {

            auto scaleImage = [this]() {

                block_data_->GetCurrentATMutual()->GenPreviewImages(block_data_->GetPath());

            };
            QFuture<void> f1 = QtConcurrent::run(scaleImage);

            ///f1.waitForFinished();
        }

        // note:triggered by clicked event really.
        void BlockWgt::Slot_TableView_RealClicked(QModelIndex index)
        {
            selectedImages.clear();
            bNeedCheckSelectedImagesLater = true;
            Slot_TableView_Clicked(index);
        }

        void BlockWgt::Slot_TableView_Clicked(QModelIndex index)
        {
            if (!index.isValid() )
            {
                /*if (!index.isValid())
                    current_tableview_index  = QModelIndex();*/
                return;
            }
        
            int currentgroupid = ui->tableView_photogroup->getGroupIdByRow(index.row());
           
            auto& group = block_data_->GetGroup(currentgroupid);
            //ui->btn_push_removePgtable->setEnabled(true);
            //current_tableview_index = index;
            
            ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
            PopulatePosTableWgt(group);
            ProjectManager* promanager = ProjectManager::GetInstance();
            promanager->AddBlockManager(block_data_);
            BlockManager* blockmanager = promanager->GetBlockManaget(block_data_->GetId());
            auto &blockstatus = blockmanager->GetBlockStatusMutual();

            
            SetWgtStatus(blockstatus);
            

        }
        
        // note:triggered by clicked event really.
        void BlockWgt::Slot_TableWidget_Photo_Pos_RealClicked(QModelIndex index)
        {
            selectedImages.clear();
            bNeedCheckSelectedImagesLater = true;
            Slot_TableWidget_Photo_Pos_Clicked(index);
        }

        void BlockWgt::Slot_TableWidget_Photo_Pos_Clicked(QModelIndex index)
        {
            //如果photogroup是多选则需要将其状态改变
            /*ui->tableView_photogroup->SetSelectionChanged(true);
            std::cout << ui->tableView_photogroup->currentIndex().row() << std::endl;
            ui->tableView_photogroup->update();*/
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            UpdatePhotoDetailStatus();
            
            QApplication::restoreOverrideCursor();
        }

        void BlockWgt::Slot_Check_Preview(bool state)
        {
            QModelIndex index = ui->tableView_photo_pos->currentIndex();
            int row = index.row();
            if (row < 0)
            {
                return;
            }
            //preview
            QString text;
            QString photo_path = QString(ui->le_path->text()).append("/%1").arg(ui->le_name->text());
            if (state)
            {
                
#if 1



                //预先生成预览图再加载图像
                int currentimage_id_ = ui->tableView_photo_pos->getImageIdByRow(index.row());
                //std::vector<image_t> ids(1, currentimage_id_);
                /*if (block_data_->GetCurrentATMutual()->GetImageMutual(currentimage_id_).GetPriviewFileFullName() == "")
                {
                    block_data_->GetCurrentATMutual()->GenPreviewImages(block_data_->GetPath(), ids);
                }*/

                AI3D::CORE::Image image = block_data_->GetCurrentATMutual()->GetImagesMutual()[currentimage_id_];
                //if (image.GetPriviewFileFullName() != "")
                std::string imagepath = image.GetPath() + "/" + image.GetName();
                if(AI3D::CORE::File::ExistsFile(imagepath))
                {
                    QPixmap pixmap;
                    AI3D::CORE::Bitmap bitmap;
                    bool ret = bitmap.Read(imagepath/*image.GetPriviewFileFullName()*/);
                    QPixmap temppix = QPixmap::fromImage(AI3D::GUI::BitmapToQImageRGB(bitmap));


                    m_scene.ImagePixmapItem()->setPixmap(temppix);
                    QRect recttemp = temppix.rect();
                    int px_height = 9;
                    recttemp.setRect(recttemp.x(), recttemp.y() + px_height, recttemp.width(), recttemp.height() - 2 * px_height);
                    m_scene.setSceneRect(recttemp);

                    ui->graphics_view_photo->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);



                }
                else
                {
                    text = QString("Loading failed");
                    QPixmap pixmap(":/new/prefix1/skin/default.png");
                    m_scene.ImagePixmapItem()->setPixmap(pixmap);
                    m_scene.setSceneRect(pixmap.rect());

                    ui->graphics_view_photo->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);
                }
#endif // 0 

            }
            else
                text = QString("Preview disabled");

            if (!text.isEmpty()) {

                QFont font;
                //font.setPixelSize(24);
                font.setPointSize(24);
                QFontMetrics fm(font);
                int text_w = fm.width(text);
                int text_h = fm.height();
                int offset_w = text_w * 3 / 2;
                int offset_h = text_h * 3 / 2;

                QPixmap pix(text_w + offset_w * 2, text_h + offset_h * 2);
                QRect rect(offset_w, offset_h, fm.width(text), fm.height());
                pix.fill(Qt::transparent);
                QPainter painter(&pix);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                painter.setFont(font);
                painter.setPen(Qt::gray);
                painter.drawText(rect, text);   
            }
        }


        void BlockWgt::Slot_LinkActivated_Label_Photo_Open()
        {

            QString photo_path = ui->le_path->text() + QString("/%1").arg(ui->le_name->text());
            if (!QFileInfo(photo_path).exists())
                return;

            QString path = QString("file:///") + photo_path;
            bool is_open = QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));

        }

        

        void BlockWgt::beginScaledImage(QStringList& fileNameList, QString& destPath)
        {
            ImageScale* _imageScale = new ImageScale();
            _imageScale->setAutoDelete(true);
            QObject::connect(_imageScale, SIGNAL(finish(QString&, QString&, int&, int&)), this, SLOT(setListWidgetItemIcon(QString&, QString&, int&, int&)), Qt::QueuedConnection);

            _imageScale->setFileName(fileNameList);
            _imageScale->setOutFilePath(destPath);
            QThreadPool::globalInstance()->start(_imageScale, 1);

        }

        

        void BlockWgt::resizeEvent(QResizeEvent* event)
        {
            ui->graphics_view_photo->fitInView(m_scene.sceneRect(), Qt::KeepAspectRatio);

            QWidget::resizeEvent(event);
        }

        //
        void BlockWgt::Slot_Btn_AddPhotoDir_Clicked()
        {
            QString dir;
            QString str= "Open photo directory";
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                str = "打开文件目录";
            }
            QFileDialog fd(nullptr, str, dir);
            fd.setFileMode(QFileDialog::Directory);

            if (QFileDialog::Accepted != fd.exec())
                return;

            if (!dir.isEmpty())
                return;

            QStringList dirs;
            dirs << fd.selectedFiles().first();

            std::string path = qstr2str(fd.selectedFiles().first());

            std::vector<std::string> filenames;
            LOGI("Add images.");
            QTime t;
            t.start();
            QFileInfoList myphotoList;
            QStringList list_suffixs_photos_form = QStringList() <<
                ".arw" << "*.raw" << "*.rw2" << ".jpg" << ".jpeg" << ".png" << ".tiff" << ".tif"/* << ".cr2"*/;
            std::vector<std::string> image_extension;
            for (auto it : list_suffixs_photos_form)
            {
                image_extension.push_back(qstr2str(it));
            }

            t.start();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            block_data_->SearchImages(path, filenames, image_extension);
            //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
            if (filenames.empty())
            {
                LOGI(" No image in " + path);
            //  std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
                QApplication::restoreOverrideCursor();
                return;
            }
            LOGD(AI3D::CORE::String::StringPrintf("SearchImages spends: %f s", t.elapsed() / 1000.0));
            //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
            //统计导入影像数据量
            for (const auto& filename : filenames)
            {
                std::string parentdir = AI3D::CORE::File::GetParentDir(filename);
                parentdir = AI3D::CORE::File::EnsureUnifySlash(parentdir);
                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->PhotosOfDir[parentdir]++;
            //  std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
            }
            //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
            photoAllNum = filenames.size();
            QApplication::restoreOverrideCursor();
            _nTotalPhotos = 0;

            /*block_data_->Addimages_Beta(filenames, &_nTotalPhotos);*/
            //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
            auto addimage = [&]() {return block_data_->Addimages_Beta(filenames, &_nTotalPhotos); };

            QFuture<bool> future = QtConcurrent::run(addimage);

            /*if (_nTotalPhotos == 0)
            {
                if (BlockObject::isChineseVersion())
                {
                    Message_Box::warning(this, "警告", "没检测到影像");
                }
                else
                    Message_Box::warning(this, "Warning", "No images");
            }
            else*/
            {
                //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
///             my_Progress->setMinValue(_nTotalPhotos);
                my_Progress->setMinValue(0);
                my_Progress->setMaxValue(COMPLETE_PROGRESS);
                my_Progress->setLabelText(QString("Adding %1 Photos...").arg(QString::number(photoAllNum)));
                //Sleep(100);
                //my_Progress->setModal(true);
                my_Progress->setWindowModality(Qt::ApplicationModal);
                my_Progress->show();
                int progressnum = 0;
                ///emit Signal_Photo_Progress(_nTotalPhotos);
                emit Signal_Photo_Progress(0);
                //for (int avernum = 0; avernum < filenames.size(); )
                t.start();
                int nTotal = 0;
                //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
                //界面进度条更新
                while (nTotal != 100)
                {
                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
                    if (_nTotalPhotos < 0)
                    {
                        break;
                    }

                    if (nTotal != _nTotalPhotos)
                    {
                        emit Signal_Photo_Progress(_nTotalPhotos);
                        my_Progress->setWindowModality(Qt::ApplicationModal);
                        //my_Progress->setValue(_nTotalPhotos);
                        //LOGD(String::StringPrintf("Adding Images Process: %d ", _nTotalPhotos));
                        nTotal = _nTotalPhotos;
                    }
                    
                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;
                    //不能做其他操作，界面处于忙碌状态，但没卡死
                    qApp->processEvents();
                }
                //std::cout << "inside " << __FILE__ << " " << __LINE__ << " add photos:" << filenames.size() << std::endl;

                //阻塞线程
                future.waitForFinished();
                if (!future)
                {
                    my_Progress->hide();

                }

                LOGD(String::StringPrintf("AddImages spends: %f s. ", t.elapsed() / 1000.0));
                QTimer::singleShot(200, [this]() {my_Progress->hide(); });
                block_data_->SetStatus(jobsta_e::STATUS_NEW);//chy 1125bug此处会出现状态为unkonw

                ui->tableView_photogroup->setCurrentIndex(QModelIndex());
                ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
                PopulatePhotoGroupTable();
                ui->tableView_photo_pos->clearData();
                UpdateWgtAndProjStatus(!ExistsTab(PHOTOTAB));

                LOGI(block_data_->GetName() + "Add imagedir");
            }
        }

        void BlockWgt::Update3DView()
        {
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            //3dview置灰
            //chy@zhaokang加入此逻辑的原因：当导入影像加入pos再将pos删除，此时3dview仍显示有pos所以加入此逻辑
            if (!(block_data_->GetCurrentAT()->HasPositionImages() ||
                block_data_->GetCurrentAT()->HasControlPoints() ||
                block_data_->GetCurrentAT()->HasTiepoints()))
            {
                QTabBar* tabbar = ui->tabWidget->tabBar();
                //tabbar->setTabEnabled(tabbar->count() - 1, false);
            }
            // note:check it later,for it may have some potential risks here if compare with three directly.
            if (current_tab_id_ == 3)//chy add
            {
                ///viewWidget_ui->showAT3dview();
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() != 4)
                {
                    viewWidget_ui->RenderATDataWithSelectedImages(std::vector<image_t>());
                }
            }
        }
        //如果是photogroup则删除整个group，如果是单张影像则删除单张影像
        void BlockWgt::Slot_Btn_DelPhoto_Clicked()
        {
            //

            QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
            QModelIndex currentphotoindex = ui->tableView_photo_pos->currentIndex();

            bool photogroupselected = false;// = ui->tableView_photogroup->currentIndex().isValid();
            bool photoselected = false;// = ui->tableWidget_photo_pos->currentIndex().isValid();
            if (currentgroupindex.isValid())
                photogroupselected = true;;

            if (currentphotoindex.isValid())
                photoselected = true;;
            if (!photogroupselected && !photoselected)
            {
                return;
            }
            if (photoselected)
            {
                SlotDeletePhoto();
            }
            else
            {
                if (photogroupselected)
                {
                    SlotDeletePhotoGroup();
                }
            }
        }

        void BlockWgt::UpdateWgtAndProjStatus(bool bchangetab)
        {
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            ProjectManager* promanager = ProjectManager::GetInstance();
            promanager->AddBlockManager(block_data_);
            BlockManager* blockmanager = promanager->GetBlockManaget(block_data_->GetId());
            auto blockstatus = blockmanager->GetBlockStatusMutual();

            if (bchangetab )
            {
                std::vector<int> tabvec;
                bool ATFlag = false;
                promanager->GetBlockManaget(block_data_->GetId())->ChangeTab(ExistsTab(ATTAB), block_data_->GetCurrentAT()->HasImages(), \
                    block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
                UpdateTabPaper(tabvec);
            }
            SetWgtStatus(blockstatus);

            SetModifityXml();
            Update3DView();
        }

        void BlockWgt::Slot_Btn_AddPos_Clicked()
        {       
            ImportPosDia imposDialog;
            QMap<int, QString> m_RecordData = imposDialog.getRecordData();
            if (QDialog::Accepted != imposDialog.exec())
                return;
            //获取坐标系统

            srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(imposDialog.GetSrsName().toStdString());

            QList<QStringList> Var1 = imposDialog.getPosList();
            std::vector<pose_s> pos_vec;
            for (auto perlist : Var1)
            {
                pose_s posinfo;

                posinfo.name = perlist.at(0).toStdString();
                posinfo.metadata_.center.x() = perlist.at(1).toDouble();
                posinfo.metadata_.center.y() = perlist.at(2).toDouble();
                posinfo.metadata_.center.z() = perlist.at(3).toDouble();

                if (perlist.size() >= 7)
                {

                }

                pos_vec.push_back(posinfo);
            }

            //chy todo:此处理论上如果为false应该有提示，需再设计
            block_data_->AddPoses(srs, pos_vec);
            
            //
            ui->tableView_photogroup->setCurrentIndex(QModelIndex());
            ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
            PopulatePhotoGroupTable();
            ui->tableView_photo_pos->clearData();
            UpdateWgtAndProjStatus();
            
            LOGI(AI3D::CORE::String::StringPrintf("%sAdd Pos file", block_data_->GetName()));
        }

        void BlockWgt::ShowATTab(bool isHideOther)
        {
            if (block_data_->GetCurrentAT() == nullptr)
            {
                return;
            }
            ProjectManager* promanager = ProjectManager::GetInstance();
            std::vector<int> tabvec;
            promanager->GetBlockManaget(block_data_->GetId())->ChangeTab(true, block_data_->GetCurrentAT()->HasImages(), \
                block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
            UpdateTabPaper(tabvec);
            current_tab_id_ = 0;
            if (isHideOther)
            {
                QTabBar* tabbar = ui->tabWidget->tabBar();
                /// todo:tabbar
                tabbar->setEnabled(false);
            }
        }

        void BlockWgt::Slot_Btn_AddPhotoFile_Clicked()
        {
            QStringList files;
            QString _photoDir;
            QString str = "Add photos";
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                str = "导入影像";
            }
            QFileDialog fd(nullptr, str, _photoDir);
            fd.setFileMode(QFileDialog::ExistingFiles);
            QStringList filters;
            filters << "Image files (*.tiff *.jpg *.jpeg *.png *.arw  *.raw *.rw2 *.tif *.cr2)"; //chy 界面导入时显示可支持的影像类型 
            fd.setNameFilters(filters);
            if (QFileDialog::Accepted != fd.exec())
                return;

            files = fd.selectedFiles();
            if (files.isEmpty())
            {
                LOGI("select file is empty!");
                return;
            }

            std::vector<std::string> filenames;
            for (auto it : files)
            {


                filenames.push_back(qstr2str(it));
                //统计导入影像数量
                std::string parentdir = AI3D::CORE::File::GetParentDir(qstr2str(it));

                parentdir = AI3D::CORE::File::EnsureUnifySlash(parentdir);
                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->PhotosOfDir[parentdir]++;

            }
            //block_data_->AddImages(filenames);
            photoAllNum = filenames.size();
            int averagenum = photoAllNum / 10;               
            _nTotalPhotos = 0;
            my_Progress->setMinValue(_nTotalPhotos);
            my_Progress->setMaxValue(COMPLETE_PROGRESS);
            my_Progress->setLabelText(QString("Adding %1 Photos...").arg(QString::number(photoAllNum)));
            //my_Progress->setModal(true);
            my_Progress->setWindowModality(Qt::ApplicationModal);
            my_Progress->show();

            /////////////////////////////////
            emit Signal_Photo_Progress(_nTotalPhotos);
            //for (int avernum = 0; avernum < filenames.size(); )
            int nTotal = 0;
            auto addimage = [&]() {return block_data_->Addimages_Beta(filenames, &_nTotalPhotos); };

            QFuture<bool> future = QtConcurrent::run(addimage);

            while (nTotal != 100)
            {
                if (_nTotalPhotos < 0)
                {
                    break;
                }

                if (nTotal != _nTotalPhotos)
                {
                    emit Signal_Photo_Progress(_nTotalPhotos);
                    ///my_Progress->setValue(_nTotalPhotos);
                    my_Progress->setWindowModality(Qt::ApplicationModal);
                    //LOGD(String::StringPrintf("AddImages spends: %d ", _nTotalPhotos));
                    nTotal = _nTotalPhotos;                 
                }

                qApp->processEvents();
            }
            
            future.waitForFinished();
            if (!future)
            {
                my_Progress->hide();

            }

            //chy   chy modifi 20220830
            QTimer::singleShot(200, [this]() {my_Progress->hide(); });
            ui->tableView_photogroup->setCurrentIndex(QModelIndex());
            ui->tableView_photo_pos->setCurrentIndex(QModelIndex());
            PopulatePhotoGroupTable();
            ui->tableView_photo_pos->clearData();
            UpdateWgtAndProjStatus(!ExistsTab(PHOTOTAB));
            
            LOGI(block_data_->GetName() + "Addimage file");
        }

        void BlockWgt::Slot_Btn_DelPos_Clicked()
        {

            QModelIndex currentgroupindex = ui->tableView_photogroup->currentIndex();
            QModelIndex currentphotoindex = ui->tableView_photo_pos->currentIndex();

            bool photogroupselected = false;
            bool photoselected = false;
            if (currentgroupindex.isValid())
                photogroupselected = true;;

            if (currentphotoindex.isValid())
                photoselected = true;;
            if (!photogroupselected && !photoselected)
            {
                return;
            }
            if (photoselected)
            {
                SlotClearPhotoPose();
            }
            else
            {
                SlotClearPoseByGroup();
            }       
            
        }

    }
}
