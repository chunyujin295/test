#include <QProcess>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QLine>
#include "Gui/MohackerWin.h"
#include "Gui/XmlOper.h"
#include "Util/CatchProcess.h"
#include "Gui/NewProjectDlg.h"
#include "Gui/ProjectManager.h"
#include "Gui/GlobalStruct.h"
#include "Gui/ImportPosDia.h"
#include "Core/Types.h"
#include "Util/OTA.h"
#include "Util/Statistic.h"
#include "Gui/ToolTip.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <QProgressDialog>
#include "Gui/QProgressIndicator.h"

#include <QItemSelectionModel>
#include"Core/Application.h"
#include "Core/File.h"
#include <filesystem>
#include "Gui/message_box.h"
#include "Util/TaskProcess.h"
#include "Core/ReconstructionCommandSet.h"
#include "Gui/ExportXmlDia.h"
#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Gui/ProjectionSelectionTreeWidget.h"

#include "Gui/QComboxTree.h"
#include "Util/User.h"
#include "Util/Software.h"
#include "Util/Transfer.h"
#include "Core/WorkPath.h"

#endif // USE_AI3D_PROJ
namespace AI3D
{
    namespace GUI
    {
#define NEW_MULTI_CHOICE_MODE 1

        
        MohackerWin* MohackerWin::instance = nullptr;
        std::once_flag MohackerWin::oc_;
        static bool bSupportDataPreprocess4TestPurpose = false;

        MohackerWin* MohackerWin::GetInstance()
        {
            std::call_once(oc_, []() {instance = new MohackerWin(); });
            return instance;
        }

        MohackerWin::MohackerWin() :
            _itemmodel_(nullptr),
            _currentSolutionXmlPath_(""),
            project_root_item_(nullptr),
            _proxy_(nullptr)
        {
          
           
            InitWgt();
            qRegisterMetaType<AI3D::CORE::BlockObject*>(); 
            qRegisterMetaType<AI3D::CORE::ProductionObject*>();
            qRegisterMetaType<AI3D::CORE::ReconstructionObject*>();
           

            importxml_ = new ImportXml;
            connect(importxml_, &ImportXml::FinishedRead, this, &MohackerWin::FinishImportXML);
            connect(this, &MohackerWin::Signal_Process, m_pProgressBar, &ProgressCom::setValue, Qt::QueuedConnection);
            GetRunningInfoTime = new QTimer;
            connect(GetRunningInfoTime, &QTimer::timeout, this, &MohackerWin::ProcessBlockStatus);
            
            if (!GetRunningInfoTime->isActive())
                GetRunningInfoTime->start(1000);
            

            QString masterJobPath = Settings::getMasterJobQueue();
            QString engineJobPath = Settings::getEngineJobQueue();

            std::string masterJobPathStr = masterJobPath.toStdString();
            std::string engineJobPathStr = engineJobPath.toStdString();

            
            
            

            //设置默认jobs路径
            //chy@zhaokang
            try
            {
              
                if (!std::filesystem::is_directory(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(masterJobPath))) || !std::filesystem::is_directory(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(engineJobPath))))
                {
             
                    QString path = QCoreApplication::applicationDirPath();
                    path.append("/jobs");
                    QSettings* pSettings = new QSettings("HKEY_CURRENT_USER\\Software\\MoldAI\\JobQueues", QSettings::NativeFormat);
                    pSettings->setValue("master", path);
                    pSettings->setValue("engine", path);
            
                }
            
            }
            catch (const std::filesystem::filesystem_error& fse)
            {
            
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                LOGI(oss.str());
            }
            catch (std::exception& ex)
            {
           
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
            }

            
            ui_treeView_project->viewport()->installEventFilter(new AToolTipper((QObject*)ui_treeView_project));          
        }

        MohackerWin::~MohackerWin()
        {
        
            ClearCurrentProject();
          
            bMainWindowDestroyed = true;
           
        }

        QString MohackerWin::GetMasterDir() 
        { 
            return Settings::getMasterJobQueue();
        }

        QString MohackerWin::GetEngineDir() 
        { 
            return Settings::getEngineJobQueue();
        }

        int MohackerWin::disableLevel4ReconstructionPerformanceTest()
        {
            // can use a optional item in the project config file.
            // 9:disable all key point.
            // 0:no key point disabled.(default behaviour in release version.)
            // 1-8:partly disable some special key point.

            return 9; 
        }

        static bool bCheckingUpdate = false;
        static bool bDownloadingNewVersion = false;
        static bool bGotNewVersion = false;

        // 检查OTA服务器上是否有更新版本并下载到本地
        static void doCheckUpdateAndDownload(MohackerWin* mwin, QStringList& updloadFileList)
        {
            bCheckingUpdate = true;

            std::cout << " checking update..." << std::endl;

            // 判断当前安装目录中是否存在Master及Engine数据统计文件
            QStringList uploadLogs = need2UploadFiles();
            if (uploadLogs.size() > 0)
            {
                std::cout << "has master/engine stats logs needed to upload." << std::endl;
                ///for (auto uploadLog : uploadLogs)
                ///{
                    /// std::cout << "\t" << uploadLog.toStdString();
                ///}

                std::cout << std::endl;
                // 向服务器传输数据统计文件(xxxMaster.json和xxxEngine.json)
                uploadStatsLog(uploadLogs);
            }
            else
            {
                std::cout << "no master/engine stats logs need to upload." << std::endl;
            }

            // 1)see whether need to check update.
            // 2)if need,check update per minute until find newer update.
            // 3)if got newer update,set new-version flag and stop checking,
            // 4)if newer update got,set about state.and do it based on user request.
            // 5)if not,continue to check again per minute.

            if (bGotNewVersion)
            {
                std::cout << " already got new update,so don't need to check update from now on." << std::endl;
            }
            else
            {
                std::cout << " no new update exists on local disk,so need to check update per period until got newer update." << std::endl;

                // prepare a clean environment to do checking update and downloading related files from the cloud server.
                cleanUpOTADownloadEnvironment();
            }

            // 周期性检查服务器上是否有最新版本
            while (!bGotNewVersion)
            {
                std::cout << " check new update now..." << std::endl;

                if (bMainWindowDestroyed)
                {
                    std::cout << " main window destroyed,so quit current thread immediately." << std::endl;
                    break; 
                }

                // if main ui thread,exit current check update thread.and using small time period here later.

                // cleanup every loop.
                // 清空遗留的OTA下载数据,准备环境
                cleanUpOTADownloadEnvironment();

                // 下载服务器上的更新包并解压
                bool checkUpdate = CheckAndDownloadAndUnarchive();
                if (checkUpdate)
                {
                    std::cout << " has found the new update,and download into current disk now." << std::endl;
                    bGotNewVersion = true;                 
                                             break;
                 }
                else
                {

                }

                Sleep(5000);
            }            

            bCheckingUpdate = false;

            std::cout << " check update completed." << std::endl;
        }


        void MohackerWin::InitWgt()
        {

            CatchProcess* catchProgress = new CatchProcess;
            catchProgress->IsProgramRunning(QString("MoldAI.exe"));
            processNum = catchProgress->NumProgramRunning(QString("MoldAI.exe"));
            
            _map_icon_[tr("Project")] = QIcon(":/new/prefix1/skin/file@2x.png");
            _map_icon_[tr("Block")] = QIcon(":/new/prefix1/skin/openfolder@2x.png");

            _map_icon_[tr(JOBPENDINGSTR)]   = QIcon(":/status/skin/pending.png");  //_map_icon_[tr("Block")];// 
            _map_icon_[tr(JOBRUNNINGSTR)]   = QIcon(":/status/skin/running.png");  //_map_icon_[tr("Block")];// 
            _map_icon_[tr(JOBCOMPLETEDSTR)] = QIcon(":/status/skin/complete.png"); //_map_icon_[tr("Block")];// 
            _map_icon_[tr(JOBCANCELLEDSTR)] = QIcon(":/status/skin/cancle.png");   //_map_icon_[tr("Block")];// 
            _map_icon_[tr(JOBFAILEDSTR)] = QIcon(":/status/skin/failed.png");   //_map_icon_[tr("Block")];// 

            // status icon for production item of project tree only.
            _map_icon_[tr(PJOBPENDINGSTR)] = QIcon(":/new/prefix1/skin/production_wait.png");
            _map_icon_[tr(PJOBRUNNINGSTR)] = QIcon(":/new/prefix1/skin/production_run.png");
            _map_icon_[tr(PJOBCOMPLETEDSTR)] = QIcon(":/new/prefix1/skin/production_complete.png");
            _map_icon_[tr(PJOBCANCELLEDSTR)] = QIcon(":/new/prefix1/skin/production_cancel.png");
            _map_icon_[tr(PJOBFAILEDSTR)] = QIcon(":/new/prefix1/skin/production_fail.png");

            /*_map_color_[tr("Block")] = Qt::GlobalColor::;*/
            _map_color_[tr(JOBPENDINGSTR)] = Qt::GlobalColor::yellow;// "#ffd700";
            _map_color_[tr(JOBRUNNINGSTR)] = Qt::GlobalColor::blue;// "#1e90ff";
            _map_color_[tr(JOBCOMPLETEDSTR)] = Qt::GlobalColor::green;// "#228b22";
            _map_color_[tr(JOBCANCELLEDSTR)] = Qt::GlobalColor::magenta;// "#9932cc";
            _map_color_[tr(JOBFAILEDSTR)] = Qt::GlobalColor::red;// "#b22222";

            //_map_color_[tr(JOBRUNNINGSTR)] = _map_color_[tr("Block")];// "#1e90ff";
            CreateWgt();
            CreateActions();
            CreateConnections();
            this->setMinimumSize(1280, 800);
            showMaximized();
            //setWindowIcon(QIcon(":/new/prefix1/skin/moldai32.png"));
            //此处为啥还要在IDE加入 与png啥区别
             setWindowIcon(QIcon("Mohacker32.ico"));

            SetWindowTitle();;
           
            ui_treeView_project->installEventFilter(this);
            //ui_treeView_project->setIconSize(QSize(18, 18));
            
            m_pProgressBar = new ProgressCom();
            Qt::WindowFlags flags = Qt::Dialog;
            m_pProgressBar->setWindowFlags(flags);
            m_pProgressBar->setWindowModality(Qt::WindowModal);
            m_pProgressBar->setWindowFlags(Qt::FramelessWindowHint);
            
            m_pProgressBar->setTitleVisble(false);
            

            // 定时器周期性检查服务器中是否存在最新版本
            // (此时定时尚未启动激活)
            pCheckVersionTimer = new QTimer(this);           

            connect(pCheckVersionTimer, &QTimer::timeout, this, &MohackerWin::Slot_CheckVersion);

            bDownloadingNewVersion = false;
            bGotNewVersion = false;

            // check whether has new version on local disk.if true,don't need to start this timer to check the newer version on the server.
            // for develop environment,it is different with ota upload/download environment.

            //确认好逻辑
            // 检查当前运行目录是否是正式的产品目录路径
            //if (checkOTAInsideProductEnvironment())
            //{
            //    std::cout << " now in product environment,OTA enabled." << std::endl;
            //    // 检查本地是否已经存在可以升级的更新的OTA包. 
            //    if (canBeUpgrade())
            //    {
            //        // 本地已有下载好的更新包,关闭周期性版本检查定时器(如果已激活对应定时器).
            //        bGotNewVersion = true;
            //        Slot_CheckVersion();
            //        std::cout << " has new version on local disk,pls update it at some time." << std::endl;
            //    }
            //    else
            //    {
            //        // 本地尚未有下载好的更新包,激活版本定期性周期性检测相关新版本标志
            //        std::cout << " hasn't new version on local disk,will check the new version on the server later." << std::endl;
            //        pCheckVersionTimer->start(2000);

            //        // 检查OTA服务器上是否有更新版本并下载到本地
            //        QtConcurrent::run(&doCheckUpdateAndDownload, this, QStringList());
            //    }

            //}
            //else
            //{
            //    std::cout << " not in product environment(such as develop env),OTA disabled." << std::endl;
            //}

            //检测系统硬件是否符合标准
            DeviceDetail detail = checkDeviceAvailable();
            QMessageBox msgBox;
            msgBox.setMaximumSize(1000, 1000);
            msgBox.setMinimumSize(1000, 1000);
            msgBox.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            msgBox.setFixedSize(400, 800);
            if (detail.unMatch) {
                msgBox.information(this, "系统检测", detail.errorInfo + "\n" + detail.systemInfo);
            }
            
        }



        //？chy创建树形结构
        void MohackerWin::CreateItemModel()
        {
            _itemmodel_ = new QStandardItemModel;
            connect(_itemmodel_, &QStandardItemModel::itemChanged, this, &MohackerWin::Slot_ItemDataChanged);
           ProjectManager* proManage = ProjectManager::GetInstance();

           ///QStandardItem* root = new QStandardItem(_map_icon_[tr("Project")], proManage->GetProject()->GetName().c_str());

           QString pname2 = str2qstr(const_cast<std::string &>(proManage->GetProject()->GetNameMutual()));
           QStandardItem* root = new QStandardItem(_map_icon_[tr("Project")], pname2);

            root->setData(ItemType::ITProject, CustomRole::CRItemType);
          
            root->setData(0, CustomRole::CRProjectIndex);
            root->setData(QVariant::fromValue((QWidget*)ui_projectWgt_), CustomRole::CRProjectWgt);
            project_root_item_ = root;
           
            _itemmodel_->appendRow(root);

            ///int index = ui_stackedWidget->count();
           
            _proxy_->setSourceModel(_itemmodel_);
           
            ui_treeView_project->setSortingEnabled(true);
         
        }


        QStandardItem* MohackerWin::NewBlock(AI3D::CORE::BlockObject* block,int index)
        {
            block->SetStatus(jobsta_e::STATUS_NEW);
            QStandardItem* blockItem = new QStandardItem(_map_icon_[tr("Block")], QString(block->GetName().c_str()));
            blockItem->setData(index, CustomRole::CRBlockIndex);
            blockItem->setData(QVariant::fromValue(block), CustomRole::CRBlockData);

            blockItem->setData(ItemType::ITBlock, CustomRole::CRItemType);

            std::string tooltip = block->GetTaskInfo().blockString + "\nphotos: 0\ncontrolpoints: 0\ntie points: 0";
            blockItem->setToolTip(tr(tooltip.c_str()));
           
            ProjectManager* promanager = ProjectManager::GetInstance();
            Block_Status_s& BlockStatus = promanager->GetBlockManaget(block->GetId())->GetBlockStatusMutual();
           
                
            BlockWgt* blockWidget = new BlockWgt(block,nullptr);

            blockWidget->SetCurrentExeNum(processNum);

            blockWidget->InitNewWidget();
            blockWidget->SetWgtStatus(BlockStatus);
            connect(blockWidget, &BlockWgt::Signal_Submit_Block, this, &MohackerWin::Make_AT_Block,Qt::DirectConnection);
          
            
            blockItem->setData(QVariant::fromValue((QWidget*)blockWidget), CustomRole::CRBlockWgt);
            project_root_item_->appendRow(blockItem);
            ui_stackedWidget->addWidget(blockWidget);
            if (_proxy_) {
                QModelIndex setIndex = _proxy_->mapFromSource(blockItem->index());
                ui_treeView_project->setCurrentIndex(setIndex);
            }
           
            return blockItem;
        }


        QStandardItem* MohackerWin::GetBlockATData( AI3D::CORE::BlockObject* block)
        {
            ProjectManager* promanager = ProjectManager::GetInstance();

            auto taskinfo = block->GetTaskInfo();
            QIcon icon = _map_icon_[tr(blk_status_str.at(block->GetStatus()).c_str())];

            QStandardItem* blockItem = new QStandardItem(icon, str2qstr(taskinfo.blockString));
            
          
            blockItem->setData(project_root_item_->rowCount() + 1, CustomRole::CRBlockIndex);
            blockItem->setData(QVariant::fromValue(block), CustomRole::CRBlockData);
            blockItem->setData(ItemType::ITBlock, CustomRole::CRItemType);
          
            //std::vector<blk_recontruction_info_s> reconstructions_info_
            
            int recons_index = 0;
            for (auto& recons : taskinfo.reconstructions_info_)
            {
                QStandardItem* recons_item = new QStandardItem(str2qstr(recons.name_));

                recons_item->setData(recons_index, CustomRole::CRBlockIndex);
//                recons_item->setData(QVariant::fromValue(block), CustomRole::CRBlockData);

                recons_item->setData(ItemType::ITReconstruction, CustomRole::CRItemType);

                recons_item->setData(recons.id_, CustomRole::CRReconstructionID);
                recons_item->setData(QVariant::fromValue(block), CustomRole::CRParentBlockData);
                

                blockItem->appendRow(recons_item);

                AI3D::CORE::ReconstructionObject* pReconstructionObject = block->GetReconstruction(recons.id_);
                if (pReconstructionObject)
                {
///                    QStandardItem* pNewConstructionItem = new QStandardItem(str2qstr(const_cast<std::string&>(pReconstructionObject->GetName())));
//                    ui_treeView_project->expandAll();

                    //std::cout << "load:get reconstruction info:" << pReconstructionObject->GetName() << " " << pReconstructionObject->GetNumTiles() << std::endl;
                    recons_item->setData(QVariant::fromValue(pReconstructionObject), CustomRole::CRReconstructionData);

///                    ConstructionWgt* constructionWgt = new ConstructionWgt(block, pReconstructionObject, recons_item);
///                    connect(constructionWgt, &ConstructionWgt::Sig_NewProduction, this, &MohackerWin::Slot_NewProduction);
///                    recons_item->setData(QVariant::fromValue(constructionWgt), CustomRole::CRBlockData);
///                    ui_stackedWidget->addWidget(constructionWgt);
                }

                int product_index = 0;
                for (auto& product_ : recons.production_infos_)
                {
                    AI3D::CORE::ProductionObject* pProductionObject = 0;
                    
                    if(pReconstructionObject)
                        pProductionObject = pReconstructionObject->GetProduction(product_.id_);

                    QStandardItem* product_item = new QStandardItem(QString(product_.name_.c_str()));
                    product_item->setData(product_index, CustomRole::CRBlockIndex);
                    product_item->setData(ItemType::ITProduction, CustomRole::CRItemType);
                    product_item->setData(recons.id_, CustomRole::CRReconstructionID);
                    product_item->setData(QVariant::fromValue(block), CustomRole::CRParentBlockData);
                    product_item->setData(product_.id_, CustomRole::CRProductionID);
                    product_item->setData(QVariant::fromValue(pReconstructionObject), CustomRole::CRReconstructionData);
                    product_item->setFlags(product_item->flags() & ~Qt::ItemIsEditable);

                    if (pProductionObject)
                    {
///                        ProductionWgt* productionWgt = new ProductionWgt(block, pReconstructionObject, recons_item, pProductionObject, product_item);
///                        product_item->setData(QVariant::fromValue(productionWgt), CustomRole::CRBlockData);
///                        ui_stackedWidget->addWidget(productionWgt);
/// 
                        product_item->setData(QVariant::fromValue(pProductionObject), CustomRole::CRProductionData);
                    }

                    recons_item->appendRow(product_item);

                    product_index++;
                }

                recons_index++;
            }

            /*block->GetATData();*/
            
           project_root_item_->appendRow(blockItem);

           return blockItem;
        }

        void MohackerWin::RefreshBlockStatus()
        {
            if (bProjectDirty)
            {
                m_mapProductionItemsToBeRefreshed.clear();
                m_mapBlockItemsToBeRefreshed.clear();
                return;
            }

            if (m_mapProductionItemsToBeRefreshed.size() > 0)
            {
                for (auto it : m_mapProductionItemsToBeRefreshed.toStdMap())
                {
                    if (bProjectDirty)
                    {
                        m_mapProductionItemsToBeRefreshed.clear();
                        m_mapBlockItemsToBeRefreshed.clear();
                        return;
                    }

                    QStandardItem* pProductionItem = it.first;
                    job_status_e production_status = it.second;

                    if (pProductionItem != nullptr)
                    {
                        switch (production_status)
                        {
                        case jobsta_e::STATUS_COMPLETE:
                            pProductionItem->setIcon(_map_icon_[tr(PJOBCOMPLETEDSTR)]);
                            break;
                        case jobsta_e::STATUS_CANCLE:
                            pProductionItem->setIcon(_map_icon_[tr(PJOBCANCELLEDSTR)]);
                            break;
                        case jobsta_e::STATUS_FAILURE:
                            pProductionItem->setIcon(_map_icon_[tr(PJOBFAILEDSTR)]);
                            break;
                        case jobsta_e::STATUS_RUNNING:
                            pProductionItem->setIcon(_map_icon_[tr(PJOBRUNNINGSTR)]);
                            break;
                        case jobsta_e::STATUS_PENDDING:
                        default:
                            pProductionItem->setIcon(_map_icon_[tr(PJOBPENDINGSTR)]);
                            break;
                        }
                    }
                }

                m_mapProductionItemsToBeRefreshed.clear();
            }

            if (m_mapBlockItemsToBeRefreshed.size() > 0)
            {
                for (auto it : m_mapBlockItemsToBeRefreshed.toStdMap())
                {
                    if (bProjectDirty)
                        return;
                    QStandardItem* pBlockItem = it.first;
                    job_status_e block_status = it.second;

                    QIcon icon = _map_icon_[tr(blk_status_str.at(block_status).c_str())];
                    pBlockItem->setIcon(icon);
                }

                m_mapBlockItemsToBeRefreshed.clear();
            }
        }

        QString MohackerWin::prependIndentation()
        {
            return QString(4, QChar(' '));
        }

        QString MohackerWin::stripPrependIndentation(QString str)
        {
            return str.trimmed();
        }

        QString MohackerWin::localENUPrefix()
        {
            return QString("Local East-North-Up (ENU)");
        }

        QString MohackerWin::localSRS()
        {
            return QString("Local coordinate system");
        }

        void MohackerWin::TestProjSRS()
        {
            if(bSupportDataPreprocess4TestPurpose)
            {
                QString strNetPath;
                
                File::CreateDirIfNotExists(strNetPath.toStdString());
                return;
            }

            ///QgsProjectionSelectionWidget dlg;
            ///dlg.show();
            //TestDialog *dlg = new TestDialog(nullptr);
            //dlg->resize(600, 600);
            //dlg->show();

///            std::string srs_path = "D:\worksp\data\srsdata";
            std::string projPath = getenv("PROJ_LIB");
            std::cout << projPath << std::endl;
#ifdef USE_AI3D_PROJ
           AI3D::PROJ::ProjectionSelectionTreeWidget *dlg = new AI3D::PROJ::ProjectionSelectionTreeWidget();
            dlg->resize(800, 600);
            dlg->show();
#endif            
        }

        void MohackerWin::CalcBlockStatus()
        {
            if (bProjectDirty)
                return;

            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "CalcBlockStatus begin.";
                ///LOGI(oss.str());
            }

            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

            bProjectItemsStatusQuerying = true;

            m_mapBlockItemsToBeRefreshed.clear();
            m_mapProductionItemsToBeRefreshed.clear();

            for (auto& itr : promanager->GetProject()->GetBlocksMutual())
            {
                if (bProjectDirty)
                    break;
                AI3D::CORE::BlockObject* block = nullptr; /// new  AI3D::CORE::BlockObject;
                block = promanager->GetProject()->GetBlock(itr.first);

                if (block == nullptr)
                {
                    if (BlockObject::supportTempLogs())
                    {
                        std::ostringstream oss;

                        oss << "UpdateBlockStatus/nullblock inside " << " " << __LINE__ << " " << std::hex << std::showbase
                            << itr.second << " " << std::dec << itr.first;
                        LOGI(oss.str());
                    }

                    continue;
                }
                else
                {
                    if (BlockObject::supportTempLogs())
                    {
                        std::ostringstream oss;

                        oss << "UpdateBlockStatus inside "  << " " << __LINE__ << " " << std::hex << std::showbase
                            << itr.second << " / " << block << " " << std::dec << itr.first << " / " << block->GetId();
                        //LOGI(oss.str());
                    }
                }

                AI3D::CORE::BlockObject::Task_Info taskinfo;

                try
                {
                    taskinfo = block->GetTaskInfoMutual();
                }
                catch (std::exception& ex)
                {
                    std::ostringstream oss;
                    oss << "inside "  << " " << __FUNCTION__ << " " << __LINE__ <<
                        " " << ex.what();
                    LOGI(oss.str());
                    //std::cout << oss.str() << std::endl;
                    continue;
                }

                if (BlockObject::supportTempLogs())
                {
                    std::ostringstream oss;
                    oss << "CalcBlockStatus...";
                    //LOGI(oss.str());
                }

                if (block->GetStatus() <= jobsta_e::STATUS_NEW)
                {
                    BlockWgt::UpdateBlockStatusToProject(block);

                    if (project_root_item_ != nullptr)
                    {
                        if (BlockObject::supportTempLogs())
                        {
                            std::ostringstream oss;
                            oss << "CalcBlockStatus...";
                            ///LOGI(oss.str());
                        }

                        for (auto row = 0; row < project_root_item_->rowCount(); row++)
                        {
                            if (bProjectDirty)
                            {
                                bProjectItemsStatusQuerying = false;
                                return;
                            }

                                QStandardItem* Blockitem = project_root_item_->child(row/*block->GetId()*/);

                                if (Blockitem != nullptr)
                                {
                                    if (Blockitem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetId() == block->GetId())
                                    {
                                        _job_status_e block_status = block->GetStatus();
                                        switch (block_status)
                                        {
                                        case jobsta_e::STATUS_COMPLETE:
                                        case jobsta_e::STATUS_CANCLE:
                                        case jobsta_e::STATUS_FAILURE:
                                        case jobsta_e::STATUS_RUNNING:
                                        case jobsta_e::STATUS_PENDDING:
                                            break;
                                        default:
                                            block_status = jobsta_e::STATUS_PENDDING;
                                            break;
                                        }

                                        if (m_mapItemStatusOfProjectTree.count(Blockitem) > 0)
                                        {
                                            job_status_e saved_block_status = m_mapItemStatusOfProjectTree[Blockitem];
                                            switch (saved_block_status)
                                            {
                                            case jobsta_e::STATUS_COMPLETE:
                                            case jobsta_e::STATUS_CANCLE:
                                            case jobsta_e::STATUS_FAILURE:
                                            case jobsta_e::STATUS_RUNNING:
                                            case jobsta_e::STATUS_PENDDING:
                                                break;
                                            default:
                                                saved_block_status = jobsta_e::STATUS_PENDDING;
                                                break;
                                            }

                                            if (block_status != saved_block_status)
                                            {
                                                m_mapItemStatusOfProjectTree.insert(Blockitem, block_status);
                                                m_mapBlockItemsToBeRefreshed.insert(Blockitem, block_status);
                                            }
                                        }
                                        else
                                        {
                                            m_mapItemStatusOfProjectTree.insert(Blockitem, block_status);
                                            m_mapBlockItemsToBeRefreshed.insert(Blockitem, block_status);
                                        }
                                    }

                                    if (BlockObject::supportTempLogs())
                                    {
                                        std::ostringstream oss;
                                        oss << "CalcBlockStatus...";
                                        ///LOGI(oss.str());
                                    }

                                    CalcBlockProductionStatus(Blockitem);
                                    if (BlockObject::supportTempLogs())
                                    {
                                        std::ostringstream oss;
                                        oss << "CalcBlockStatus...";
                                        ///LOGI(oss.str());
                                    }

                                }
                        }

                        if (BlockObject::supportTempLogs())
                        {
                            std::ostringstream oss;
                            oss << "CalcBlockStatus...";
                   ///         LOGI(oss.str());
                        }

                    }
                }
            }

            bProjectItemsStatusGot = true;
            bProjectItemsStatusQuerying = false;

            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;

                oss << "CalcBlockStatus end.";
                ///LOGI(oss.str());
            }
        }

        void MohackerWin::CalcBlockProductionStatus(QStandardItem* pBlockItem)
        {
            if (!pBlockItem)
                return;

            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "CalcBlockStatus...";
                ///LOGI(oss.str());
            }

            BlockObject* block = pBlockItem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            if (!block)
                return;

            for (auto row = 0; row < pBlockItem->rowCount(); row++)
            {
                if (bProjectDirty)
                    return;

                QStandardItem* pReconstructionItem = pBlockItem->child(row);
                if (pReconstructionItem != nullptr)
                {
                    ReconstructionObject* pReconstructionObject = pReconstructionItem->data(CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();

                    for (auto col = 0; col < pReconstructionItem->rowCount(); col++)
                    {
                        if (bProjectDirty)
                            return;

                        QStandardItem* pProductionItem = pReconstructionItem->child(col);
                        if (pProductionItem != nullptr)
                        {
                            ProductionObject* pProductionObject = pProductionItem->data(CustomRole::CRProductionData).value<AI3D::CORE::ProductionObject*>();
                            if (block && pReconstructionObject && pProductionObject)
                            {
                                if (BlockObject::supportTempLogs())
                                {
                                    std::ostringstream oss;
                                    oss << "CalcBlockStatus...";
                                    ///LOGI(oss.str());
                                }

                                job_status_e production_status = ProductionWgt::GetProductionStatus(block, pReconstructionObject, pProductionObject);
                                if (bProjectDirty)
                                    return;

                                if (BlockObject::supportTempLogs())
                                {
                                    std::ostringstream oss;
                                    oss << "CalcBlockStatus...";
                                    ///LOGI(oss.str());
                                }

                                switch (production_status)
                                {
                                case jobsta_e::STATUS_COMPLETE:
                                case jobsta_e::STATUS_CANCLE:
                                case jobsta_e::STATUS_FAILURE:
                                case jobsta_e::STATUS_RUNNING:
                                case jobsta_e::STATUS_PENDDING:
                                    break;
                                default:
                                    production_status = jobsta_e::STATUS_PENDDING;
                                    break;
                                }

                                if (m_mapItemStatusOfProjectTree.count(pProductionItem) > 0)
                                {
                                    job_status_e saved_production_status = m_mapItemStatusOfProjectTree[pProductionItem];
                                    switch (saved_production_status)
                                    {
                                    case jobsta_e::STATUS_COMPLETE:
                                    case jobsta_e::STATUS_CANCLE:
                                    case jobsta_e::STATUS_FAILURE:
                                    case jobsta_e::STATUS_RUNNING:
                                    case jobsta_e::STATUS_PENDDING:
                                        break;
                                    default:
                                        saved_production_status = jobsta_e::STATUS_PENDDING;
                                        break;
                                    }

                                    if (production_status != saved_production_status)
                                    {
                                        m_mapItemStatusOfProjectTree.insert(pProductionItem,production_status);
                                        m_mapProductionItemsToBeRefreshed.insert(pProductionItem,production_status);
                                    }
                                }
                                else
                                {
                                    m_mapItemStatusOfProjectTree.insert(pProductionItem, production_status);
                                    m_mapProductionItemsToBeRefreshed.insert(pProductionItem, production_status);
                                }
                            }
                        }
                    }
                }
            }
            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "CalcBlockStatus...";
                ///LOGI(oss.str());
            }

        }

        void MohackerWin::ProcessBlockStatus()
        {
            ///if (MohackerWin::disableLevel4ReconstructionPerformanceTest()) 
            ///    return;

            GetRunningInfoTime->stop();

            if (bProjectItemsStatusGot || bProjectItemsStatusQuerying)
            {
                if (bProjectItemsStatusGot)
                {
                    RefreshBlockStatus();
                    bProjectItemsStatusGot = false;
                }
///             else
///                 return;
            }
            else
            {
                auto savefunc = [&, this]()
                {
                    CalcBlockStatus();
                    return 0;
                };

                QtConcurrent::run(savefunc);
            }

            GetRunningInfoTime->start(1000);
        }

        void MohackerWin::UpdateBlockStatus()
        {
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

                for (auto& itr : promanager->GetProject()->GetBlocksMutual())
                {
                    AI3D::CORE::BlockObject* block = new  AI3D::CORE::BlockObject;
                    block = promanager->GetProject()->GetBlock(itr.first);
                    if (block == nullptr)
                    {
                        std::string msg = std::to_string(itr.first) + " block is nullptr ";
                        LOGE(msg);
                        continue;
                    }
                    auto taskinfo = block->GetTaskInfoMutual();

                    if (block->GetStatus() <= jobsta_e::STATUS_NEW)
                    {
                         BlockWgt::UpdateBlockStatusToProject(block);                               

                        if (project_root_item_ != nullptr)
                        {                           
                            for (auto row = 0; row < project_root_item_->rowCount(); row++)
                            {
                                //(row == block->GetId()-1)
                                {
                                   
                                    QStandardItem* Blockitem = project_root_item_->child(row/*block->GetId()*/);

                                    
                                        if (Blockitem != nullptr)
                                        {
                                           /* std::cout << block->GetStatus() << " " << block->GetId() << " " << row << " " <<  << std::endl;*/
                                            if (Blockitem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetId() == block->GetId())
                                            {
                                                QIcon icon = _map_icon_[tr(blk_status_str.at(block->GetStatus()).c_str())];
                                                
                                                Blockitem->setIcon(icon);
                                                
                                                /*  std::cout << block->GetStatus() << " " << _map_color_[tr(blk_status_str.at(block->GetStatus()).c_str())] << std::endl;*/
                                               /* Blockitem->setBackground(_map_color_[tr(blk_status_str.at(block->GetStatus()).c_str())]);*/
                                            }

                                            UpdateBlockProductionStatus(Blockitem);
                                        }
                                }
                            }
                        }
                    }
                }
        }

        void MohackerWin::UpdateBlockProductionStatus(QStandardItem* pBlockItem)
        {
            if(!pBlockItem)
                return;
           
            BlockObject *block = pBlockItem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            if (!block)
            {
                if (BlockObject::supportTempLogs())
                {
                    std::ostringstream oss;
                    oss << "UpdateBlockProductionStatus/nullblock " << std::hex << std::showbase << block << std::dec;
                    LOGI(oss.str());
                }

                return;
            }
            else
            {
                if (BlockObject::supportTempLogs())
                {
                    std::ostringstream oss;
                    oss << "UpdateBlockProductionStatus/okblock " << std::hex << std::showbase << block << std::dec;
                    LOGI(oss.str());
                }
            }

//            auto taskinfo = block->GetTaskInfo();

            for (auto row = 0; row < pBlockItem->rowCount(); row++)
            {
                QStandardItem* pReconstructionItem = pBlockItem->child(row);
                if (pReconstructionItem != nullptr)
                {
                    ReconstructionObject* pReconstructionObject = pReconstructionItem->data(CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject *>();

                    for (auto col = 0; col < pReconstructionItem->rowCount(); col++)
                    {
                        QStandardItem* pProductionItem = pReconstructionItem->child(col);
                        if (pProductionItem != nullptr)
                        {
                            ProductionObject* pProductionObject = pProductionItem->data(CustomRole::CRProductionData).value<AI3D::CORE::ProductionObject *>();
                            if (block && pReconstructionObject && pProductionObject)
                            {
                                job_status_e production_status = ProductionWgt::GetProductionStatus(block, pReconstructionObject, pProductionObject);
                                switch (production_status)
                                {
                                case jobsta_e::STATUS_COMPLETE:
                                    pProductionItem->setIcon(QIcon(QPixmap(":/new/prefix1/skin/production_complete.png")));
                                    break;
                                case jobsta_e::STATUS_CANCLE:
                                    pProductionItem->setIcon(QIcon(QPixmap(":/new/prefix1/skin/production_cancel.png")));
                                    break;
                                case jobsta_e::STATUS_FAILURE:
                                    pProductionItem->setIcon(QIcon(QPixmap(":/new/prefix1/skin/production_fail.png")));
                                    break;
                                case jobsta_e::STATUS_RUNNING:
                                    pProductionItem->setIcon(QIcon(QPixmap(":/new/prefix1/skin/production_run.png")));
                                    break;
                                case jobsta_e::STATUS_PENDDING:
                                default:
                                    pProductionItem->setIcon(QIcon(QPixmap(":/new/prefix1/skin/production_wait.png")));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        void MohackerWin::RemoveWidgetsUnderCurrentBlock(QStandardItem* pBlockItem)
        {
            if (!pBlockItem)
                return;

            QVector<QWidget*> vecWidgets;

            for (auto row = 0; row < pBlockItem->rowCount(); row++)
            {
                QStandardItem* pReconstructionItem = pBlockItem->child(row);
                if (pReconstructionItem != nullptr)
                {
                    QWidget* pReconstructionWgt = dynamic_cast<QWidget*>(pReconstructionItem->data(CustomRole::CRBlockData).value<QWidget*>());
                    ConstructionWgt* reconstructionWgt = dynamic_cast<ConstructionWgt*>(pReconstructionItem->data(CustomRole::CRBlockData).value<ConstructionWgt*>());
                    ConstructionWgt* reconstructionWgt2 = dynamic_cast<ConstructionWgt*>(pReconstructionWgt);
        
                    for (auto col = 0; col < pReconstructionItem->rowCount(); col++)
                    {
                        QStandardItem* pProductionItem = pReconstructionItem->child(col);
                        if (pProductionItem != nullptr)
                        {
                            QWidget* pProductionWgt = dynamic_cast<QWidget*>(pProductionItem->data(CustomRole::CRBlockData).value<QWidget*>());
                            ProductionWgt* productionWgt = dynamic_cast<ProductionWgt*>(pProductionItem->data(CustomRole::CRBlockData).value<ProductionWgt*>());
                            ProductionWgt* productionWgt2 = dynamic_cast<ProductionWgt*>(pProductionWgt);
        
                            if (productionWgt)
                            {
                                // note:check it later.
                                ///productionWgt->mWindow->viewerWindow->setSceneData(nullptr);
                                productionWgt->mWindow->clearSceneData();
            //                    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                            }
                            else if (productionWgt2)
                            {
                                ///productionWgt2->mWindow->viewerWindow->setSceneData(nullptr);
                                productionWgt2->mWindow->clearSceneData();
          //                      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                            }
        //                    else if (productionWgt3)
         //                   {
         //                       productionWgt3->mWindow->viewerWindow->setSceneData(nullptr);
         //                       std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
         //                   }

                            if (pProductionWgt != nullptr)                            
                            {
                                vecWidgets.append(pProductionWgt);
              //                  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << vecWidgets.size() << std::endl;

                                ui_stackedWidget->removeWidget(pProductionWgt);
//                                delete pProductionWgt;
                            }
                        }
                    }

                    if (reconstructionWgt && reconstructionWgt->getMWindow())
                    {
                        // note:check it later.
                        ///reconstructionWgt->getMWindow()->viewerWindow->setSceneData(nullptr);
                        reconstructionWgt->getMWindow()->clearSceneData();
                //        std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    }
                    else if (reconstructionWgt2 && reconstructionWgt2->getMWindow())
                    {
                        //reconstructionWgt2->getMWindow()->viewerWindow->setSceneData(nullptr);
                        reconstructionWgt2->getMWindow()->clearSceneData();
                  //      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    }
                    //else if (reconstructionWgt3 && reconstructionWgt3->getMWindow())
                    //{
                     //   reconstructionWgt3->getMWindow()->viewerWindow->setSceneData(nullptr);
                     //   std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    //}

                    if (pReconstructionWgt != nullptr)
                    {
                        
                        vecWidgets.append(pReconstructionWgt);
                      //  std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << vecWidgets.size() << std::endl;
                        ui_stackedWidget->removeWidget(pReconstructionWgt);
  //                      delete pReconstructionWgt;
                    }


                }
            }

            for (auto& t : vecWidgets)
            {
                if (t != nullptr)
                {
                    
                    delete t;
                }
            }
        }

        void MohackerWin::RemoveWidgetsUnderCurrentReconstruction(QStandardItem* pReconstructionItem)
        {
            if (!pReconstructionItem)
                return;

            QVector<QWidget*> vecWidgets;

            for (auto col = 0; col < pReconstructionItem->rowCount(); col++)
            {
                QStandardItem* pProductionItem = pReconstructionItem->child(col);
                if (pProductionItem != nullptr)
                {
                    QWidget* pProductionWgt = dynamic_cast<QWidget*>(pProductionItem->data(CustomRole::CRBlockData).value<QWidget*>());
                    ProductionWgt* productionWgt = dynamic_cast<ProductionWgt*>(pProductionItem->data(CustomRole::CRBlockData).value<ProductionWgt*>());

                    if (productionWgt != nullptr)
                    {
                        //productionWgt->mWindow->viewerWindow->setSceneData(nullptr);
                        productionWgt->mWindow->clearSceneData();
                    }

                    if (pProductionWgt != nullptr)
                    {
                        vecWidgets.append(pProductionWgt);
                        ui_stackedWidget->removeWidget(pProductionWgt);
                        //delete pProductionWgt;
                    }
                }
            }

            for (auto& t : vecWidgets)
            {
                if (t != nullptr)
                    delete t;
            }
        }

        bool MohackerWin::LoadProject(QString projName)
        {
            //std::string lockfile = projName.toStdString() + LOCKFILE_POSTFIX;
            //
            ////if (fpprojectlock == NULL)
            //if (boost::filesystem::exists(lockfile))
            //{
            //    QString dlgTitle = tr("Error");
            //    QString tips = "Project has Opened. ";
            //    QString strInfo = tr(tips.toStdString().c_str());
            //    Message_Box::critical(this, dlgTitle, strInfo);
            //    return false;
            //}
            //else
            //{
            //    fpprojectlock = _fsopen(lockfile.c_str(), "wt", _SH_DENYWR);
            //}
            QSettings settings;
            QStringList projectionsAuthId = settings.value(QStringLiteral("UI/recentProjectionsAuthId")).toStringList();
            std::cout << projectionsAuthId.size() << std::endl;
         
            try
            {
                ///if (!boost::filesystem::is_regular_file(projName.toStdString()))
                if (!std::filesystem::is_regular_file(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(projName))))
                {
                    QString dlgTitle = tr("Error");
                    QString strInfo = tr("file not exist,please select exist path!");
                    ///std::cout << "loadproj0:" << qstr2str(projName) << std::endl;
                    Message_Box::critical(this, dlgTitle, strInfo);
                    return false;
                }
            }
            catch (const std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                LOGI(oss.str());
                return false;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }

           
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

          

            std::string logpath = qstr2str(projName);

            int num = logpath.find_last_of("/");
            std::string projectpath = logpath.substr(0, num + 1);

            ///if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "load project:" << logpath;
                LOGI(oss.str());
                std::cout << oss.str() << std::endl;
            }

            ResetLog(AI3D::CORE::File::GetParentDir(logpath), AI3D::CORE::File::GetPathBaseName(projectpath));
            // note:check it later.@240321
#if 1
            if(!currentVersionName.isEmpty())
                LOGI(AI3D::CORE::String::StringPrintf("Software Version: %s", currentVersionName.toStdString()));
            else
                LOGI(AI3D::CORE::String::StringPrintf("Software Version: %s", AI3D::GUI::VERSION.c_str()));
#endif

#if 000            
            _currentSolutionXmlPath_.clear();
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            promanager->GetProject()->Clear();
#else
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            ClearCurrentProject4LoadProject();
#endif
            ///bool loadflag = promanager->GetProject()->Load(projName.toStdString());

            bool loadflag = promanager->GetProject()->Load(qstr2str(projName));

            if (!loadflag)
            {
                QApplication::restoreOverrideCursor();
                QString dlgTitle = tr("Error");
                QString strInfo = tr("file not exist,please select exist path!");
                std::cout << "loadproj1:" << qstr2str(projName) << std::endl;
                Message_Box::critical(this, dlgTitle, strInfo);
                
                return false;
            }
            std::unordered_map<std::string, std::vector<int>> mapinfo;//分别代表blockstring/photosnum/registed photos num/control points num/Automactic tie points num

            for (auto& itr : promanager->GetProject()->GetBlocksMutual())
            {

                AI3D::CORE::BlockObject* block = nullptr; // new  AI3D::CORE::BlockObject;
                block = promanager->GetProject()->GetBlock(itr.first);
                std::string blockstring = block->GetTaskInfoMutual().blockString;
                if (blockstring == "")
                    continue;
                std::vector<int> infos;
               
                {
                    infos.push_back(block->GetNumImages());
                    infos.push_back(block->GetCurrentAT()->GetRegImageIds().size());
                    infos.push_back(block->GetCurrentAT()->GetNumControlPoints());
                }

                infos.push_back(block->GetTiepointStatus()? block->GetCurrentAT()->GetPoint3DIds().size():block->GetTaskInfoMutual().statisticinfo_.tiepointnum);
                mapinfo.insert(std::pair<std::string, std::vector<int>>(blockstring, infos));

            }

            //std::cout << "dump projname2:" << std::endl;

            std::string pname0 = promanager->GetProject()->GetName();
            //DumpStdStr(pname0);
            QString pname1 = str2qstr(promanager->GetProject()->GetName());
           // DumpQString(pname1);
            QString pname2 = str2qstr(pname0);
           // DumpQString(pname2);

            ui_projectWgt_ = new ProjectInfoWgt(str2qstr(promanager->GetProject()->GetName()), str2qstr(promanager->GetProject()->GetPath()), this);

            ui_projectWgt_->SetProjectInfo(mapinfo);
            //打开工程后切换背景颜色为黑色，默认qss中设置#333333        
            ui_stackedWidget->setStyleSheet(" background-color:#000000");
            ui_stackedWidget->addWidget(ui_projectWgt_);
           
            CreateItemModel();
           
            for (auto& itr : promanager->GetProject()->GetBlocksMutual())
            {

                AI3D::CORE::BlockObject* block = nullptr;/// new  AI3D::CORE::BlockObject;
                block = promanager->GetProject()->GetBlock(itr.first);

                if (block->GetTaskInfoMutual().blockString == "")
                    continue;               

                promanager->AddBlockManager(block);
                QStandardItem* item = GetBlockATData(block);
                jobsta_e  status = block->GetStatusMutual();
               
                /*if (status < jobsta_e::STATUS_NEW)
                {
                    if (!GetRunningInfoTime->isActive())
                        GetRunningInfoTime->start(1000);
                }*/
                //item->setText(block->GetTaskInfo().blockString.c_str());
                //QApplication::processEvents();
            }

           
            ui_stackedWidget->setCurrentWidget(ui_projectWgt_);
            ui_treeView_project->expandAll();
            _currentSolutionXmlPath_ = projName;
            RefreshRecentOpenMenu();
            ui_menu_block->setEnabled(true);
           /* if (!singleblock)*/
            QApplication::restoreOverrideCursor();
            return true;
        }

        bool MohackerWin::eventFilter(QObject* obj, QEvent* event)
        {
            Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
            if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
            {
                modifiers = static_cast<const QKeyEvent*>(event)->modifiers();
                if (modifiers & Qt::ControlModifier)
                    bCtrlPressed = true;
                else
                    bCtrlPressed = false;
            }
/*
            else if (event->type() == QEvent::KeyRelease)
            {
                modifiers = static_cast<const QKeyEvent*>(event)->modifiers();
                if (modifiers & Qt::ControlModifier)
                    bCtrlPressed = true;
                else
                    bCtrlPressed = false;

            } */
            else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
            {
                modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
                if (modifiers & Qt::ControlModifier)
                    bCtrlPressed = true;
                else
                    bCtrlPressed = false;
            }
/*
            else if (event->type() == QEvent::MouseButtonRelease)
            {
                modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
                if (modifiers & Qt::ControlModifier)
                    bCtrlPressed = true;
                else
                    bCtrlPressed = false;
            } */
            
            
            if(bCtrlPressed)
                ui_treeView_project->setEditTriggers(QTreeView::NoEditTriggers);
            else
                ui_treeView_project->setEditTriggers(QTreeView::DoubleClicked);

            return QMainWindow::eventFilter(obj,event);
        }

        void MohackerWin::UpdateTreeView()
        {


        }

        void MohackerWin::ClearCurrentProject()
        {
            bProjectDirty = true;

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            if (bProjectItemsStatusGot || bProjectItemsStatusQuerying)
            {
                int try_times = 20;
                while (bProjectItemsStatusGot || bProjectItemsStatusQuerying)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    try_times--;
                    if (try_times <= 0)
                        break;
                }
            }

            if(!bProjectItemsStatusGot && !bProjectItemsStatusQuerying)
            {
                ///m_mapItemStatusOfProjectTree.clear();
                ///m_mapBlockItemsToBeRefreshed.clear();
            }

            AI3D::GUI::ProjectManager::GetInstance()->GetProject()->Clear();
            if (_itemmodel_)
            {
                _itemmodel_->clear();
                delete _itemmodel_;
                _itemmodel_ = nullptr;
            }           
            
            if(project_root_item_)
                project_root_item_->clearData();

            int countWidget = ui_stackedWidget->count();
            for (int i = 0; i < countWidget; ++i)
            {
                QWidget* widget = ui_stackedWidget->widget(0);
                ui_stackedWidget->removeWidget(widget);
                widget->hide();

//                delete widget;

            }

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            bProjectDirty = false;
        }

        void MohackerWin::ClearCurrentProject4LoadProject()
        {
            bProjectDirty = true;

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            if (bProjectItemsStatusGot || bProjectItemsStatusQuerying)
            {
                int try_times = 20;
                while (bProjectItemsStatusGot || bProjectItemsStatusQuerying)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    try_times--;
                    if (try_times <= 0)
                        break;
                }
            }

            if (!bProjectItemsStatusGot && !bProjectItemsStatusQuerying)
            {
                ///m_mapItemStatusOfProjectTree.clear();
                ///m_mapBlockItemsToBeRefreshed.clear();
            }

            AI3D::GUI::ProjectManager::GetInstance()->GetProject()->Clear();
            if (_itemmodel_)
            {
                ///_itemmodel_->clear();
                ///delete _itemmodel_;
                _itemmodel_ = nullptr;
            }

            ///if (project_root_item_)
            ///    project_root_item_->clearData();

            int countWidget = ui_stackedWidget->count();
            for (int i = 0; i < countWidget; ++i)
            {
                QWidget* widget = ui_stackedWidget->widget(0);
                ui_stackedWidget->removeWidget(widget);
                ///delete widget;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            bProjectDirty = false;
        }

        void MohackerWin::SetFileModifiedXml()
        {
            ui_action_save->setEnabled(true);
            savetype_ = savetype_e::XML_SAVED;
        }

        void MohackerWin::SetProjectDirty(bool bDirty)
        {
            bProjectDirty = bDirty;
        }

        void MohackerWin::SetFileModifiedProj()
        {            
            ui_action_save->setEnabled(true);
            savetype_ = savetype_e::PROJECT_SAVED;
        }

        void MohackerWin::Make_AT_Block(AI3D::CORE::BlockObject* block)
        {
            if (!HasProject())
            {
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a project first"));
                return;
            }


            //设置所有rootitem均可点击
            for (auto row = 0; row < project_root_item_->rowCount(); row++)
            {
                QStandardItem* Blockitem = project_root_item_->child(row);

                if (Blockitem->data(CustomRole::CanSaveBlock).toInt() == 2)
                {
                    Blockitem->setData(1, CustomRole::CanSaveBlock);
                }

            }


            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();   
           
            auto& taskinfo = block->GetTaskInfoMutual();
           /* taskinfo.isSunblock = true;*/
            
            promanager->AddBlockManager(block);

            ///int rowCount = _itemmodel_->rowCount(); 
            
            
           /* auto func = [&]() */
            {
                QApplication::processEvents();
                QStandardItem* item_ = GetBlockATData(block);
                if (item_ == nullptr)
                {
                    LOGE("get block at data item null");
                    return;
                }

                block->GetTaskInfoMutual().isLoaded = false;//chy?此处应该为load ,如果去掉会出问题
                if (!ShowBlockWidget(block, item_, true))
                {
                    return;
                }
                

                block->GetTaskInfoMutual().isLoaded = true; //chy
                //item_->setText(block->GetTaskInfo().blockString.c_str());
                ui_stackedWidget->setCurrentWidget(item_->data(CustomRole::CRBlockWgt).value<QWidget*>());
                
                auto item_new = item_->data(CustomRole::CRBlockWgt).value<QWidget*>();
                BlockWgt* blockwgt = (BlockWgt*)item_->data(CustomRole::CRBlockWgt).value<QWidget*>();
                if (blockwgt == nullptr)
                {
                    LOGE("block wgt null");
                    return;
                }

                Block_Status_s& BlockStatus = promanager->GetBlockManaget(block->GetId())->GetBlockStatusMutual();
         
                blockwgt->SetWgtStatus(BlockStatus);
                //显示AT页卡
                blockwgt->ShowATTab(true);              
            };  

          

        }
      

        bool MohackerWin::HasProject()
        {
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            
            std::string projectname = promanager->GetProject()->GetName();
            if (projectname == "")
                return false;
            return true;
        }

        bool MohackerWin::ShowBlockWidget(AI3D::CORE::BlockObject* block, QStandardItem* blockItem, bool isInportblock)
        {
            ProjectManager* promanager = ProjectManager::GetInstance();
            m_pProgressBar->setValue(0);
            try
            {
                if (block->GetTaskInfo().isLoaded)
                {
                    if (_proxy_)
                    {
                        QModelIndex setIndex = _proxy_->mapFromSource(blockItem->index());
                        ui_treeView_project->setCurrentIndex(setIndex);
                    }
                    return true;//原为 false chy
                }
                else
                {
                    block->GetTaskInfoMutual().isLoaded = true;
                }
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return false;
            }
            m_pProgressBar->setValue(10);
            if (!isInportblock) 
            {
               bool issucess =  promanager->GetProject()->LoadBlockData(block->GetId());
              
               
               if (!issucess)
               {
                   QString dlgTitle = tr("Error");
                   QString strInfo = tr("LoadBlockData failed! Check Block File is created?");
                   Message_Box::critical(this, dlgTitle, strInfo);
                   m_pProgressBar->hide();
                   return false;
               }
              

                
            }
            m_pProgressBar->setValue(30);
            
            //emit Signal_Process(30);
            //获取blockobject
            auto block_temp = promanager->GetProject()->GetBlock(block->GetId());
            if (block_temp == nullptr)
            {
                return false;
            }


            promanager->AddBlockManager(block_temp);
            Block_Status_s& BlockStatus = promanager->GetBlockManaget(block_temp->GetId())->GetBlockStatusMutual();
            blockItem->setData(QVariant::fromValue(block_temp), CustomRole::CRBlockData);
            if (block_temp->GetId() == 11)
            {
                std::cout << "++++++=" << BlockStatus.can_submit_rec  << __FUNCTION__<< " ++++ "<< __LINE__ << std::endl;
            }
            

            m_pProgressBar->setValue(50);
            //emit Signal_Process(50);
            ui_blockwidget_ = new BlockWgt(block_temp, nullptr);
            //?chy @zhaokang saveatdata直接返回了
         
            connect(ui_blockwidget_, &BlockWgt::Signal_Submit_Block, this, &MohackerWin::Make_AT_Block,Qt::DirectConnection);
            connect(ui_blockwidget_, &BlockWgt::Sig_NewConstruction, this, &MohackerWin::Slot_NewConstruction);

            ui_blockwidget_->InitLoadWidget();
            //zk 设置图片状态为不可删除
         /*   BlockStatus.can_del_photo = false;*/
            ui_blockwidget_->SetWgtStatus(BlockStatus);
            
            ui_blockwidget_->SetCurrentTabId();
            //if (ui_blockwidget_->GetCurrentTabId() == 1)
            {
                ui_blockwidget_->PopulatePhotoGroupTable();
            }
            if (block->GetStatus() < jobsta_e::STATUS_NEW)
            {
                ui_blockwidget_->ShowATTab(true);
            }
            m_pProgressBar->setValue(70);
            
            ui_stackedWidget->addWidget(ui_blockwidget_);
            ui_stackedWidget->setCurrentWidget(ui_blockwidget_);
            if (_proxy_)
            {
                QModelIndex setIndex = _proxy_->mapFromSource(blockItem->index());
                ui_treeView_project->setCurrentIndex(setIndex);
            }
            blockItem->setData(QVariant::fromValue((QWidget*)ui_blockwidget_), CustomRole::CRBlockWgt);
            m_pProgressBar->setValue(100);
           
            return true;
        }

        bool MohackerWin::CheckProjectIsModifyDlg()
        {
            
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (ui_action_save->isEnabled())
            {
                QString dlgTitle = tr("Warning");
                QString strInfo = tr("The project '%1' has been modified.\n do you want to save your changes?").arg(promanager->GetProject()->GetName().c_str());

                QMessageBox::StandardButton defaultBtn = QMessageBox::NoButton;
                QMessageBox::StandardButton result;

                result = Message_Box::question(this, dlgTitle, strInfo, Message_Box_Type::Question_Yes_No_Cancel);

                if (result == QMessageBox::Yes)
                {
                    //保存工程
                    SaveProject_Wait();
                    QString str("保存工程");
                    LOGI(qstr2str(str));
                    return true;
                }
                else if (result == QMessageBox::No)
                {
                    //不保存工程
                    return true;
                }
                else if (result == QMessageBox::Cancel)
                {
                    return false;
                }

            }
            return true;
        }
        

        void MohackerWin::CreateWgt()
        {
            ui_menuBar = menuBar();
            ui_menuBar->resize(50, 0);
            ui_toolBar = addToolBar(tr("Project"));
            ui_toolBar->setMovable(false);

            ui_treeView_project = new QTreeView(this);
            ui_treeView_project->setContextMenuPolicy(Qt::CustomContextMenu);
            //不开启会带来什么问题
            ui_treeView_project->setMouseTracking(true);

            ui_treeView_project->setStyleSheet("QTreeView::item:hover{background-color:#333333}"
            "QTreeView::item:selected{background-color:#5B5B5B}"
            "QTreeView::item:selected::hover{background-color:#696969}"
            );
            
            ui_treeView_project->setEditTriggers(QTreeView::DoubleClicked);

            //ui_treeView_project->setStyleSheet("QTreeView::item:hover{background-color:#FF0000}"
            //    "QTreeView::item:selected{background-color:#0000FF}"
            //    "QTreeView::item:selected::hover{background-color:#00FF00}"
            //);
 
            ui_treeView_project->resize(150,1000);
            ui_treeView_project->setMinimumWidth(150);
            ui_treeView_project->setMaximumWidth(400);
            ui_treeView_project->setHeaderHidden(true);
            ui_treeView_project->setSelectionMode(QAbstractItemView::ExtendedSelection);

            _proxy_ = new CSortFilterProxyModel(ui_treeView_project);
            _proxy_->setSourceModel(_itemmodel_);
            ui_treeView_project->setModel(_proxy_);

            ui_treeView_project->setItemsExpandable(true);

            ui_stackedWidget = new QStackedWidget(this);
            ui_stackedWidget->setMinimumWidth(350);
            ui_stackedWidget->setMinimumHeight(138);

           
            //堆栈窗体在没有工程的情况下，会setvisiable false;


            //ui_blockwgt = new CBlockWgt(this);
            //ui_stackedWidget->addWidget(ui_blockwgt);

            ui_splitter_main = new QSplitter(this);
            ui_splitter_main->addWidget(ui_treeView_project);
            ui_splitter_main->addWidget(ui_stackedWidget);
            ui_splitter_main->setStretchFactor(1, 8);
            //ui_splitter_main->setStretchFactor(1, 6);
            ui_splitter_main->setChildrenCollapsible(false);
            this->setCentralWidget(ui_splitter_main);
            xmldia = new ExportXmlDia;
            atdia = new ExportATColmapDia;
            recdia = new ExportRecColmapDia;
        }

        void MohackerWin::CreateActions()
        {
            //Project Menu
            if (BlockObject::isChineseVersion())
            {
                  ui_menu_project = new QMenu(tr("工程"), this);
            }
            else
            {
                ui_menu_project = new QMenu(tr("&Project"), this);
            }
            //?chy 图片没找到

#if 0
            ui_action_newProject = new QAction(QIcon(":/new/prefix1/skin/filex1.png"), tr("&New Project..."), this);
            ui_action_open = new QAction(QIcon(":/new/prefix1/skin/folderx1.png"), tr("&Open..."), this);
            ui_menu_recentOpen = new QMenu(tr("Recent open..."), this);
#else
            // note:consider using special or customized function for conversion instead later.
            if (BlockObject::isChineseVersion())
            {
                ui_action_newProject = new QAction(QIcon(":/new/prefix1/skin/filex1.png"), tr("新建工程..."), this);
                ui_action_newProject->setShortcut(QKeySequence(Qt::ALT + Qt::Key_N));
                ui_action_open = new QAction(QIcon(":/new/prefix1/skin/folderx1.png"), "打开工程...", this);
                ui_action_open->setShortcut(QKeySequence(Qt::ALT + Qt::Key_O));
                ui_menu_recentOpen = new QMenu(tr("最近打开工程..."), this);
            }
            else
            {
                ui_action_newProject = new QAction(QIcon(":/new/prefix1/skin/filex1.png"), tr("&New Project..."), this);
                ui_action_open = new QAction(QIcon(":/new/prefix1/skin/folderx1.png"), tr("&Open..."), this);
                ui_menu_recentOpen = new QMenu(tr("Recent open..."), this);
            }
#endif
            ///read recent open xml

            XmlOper xmlOper;
            QString errText;
            if (!xmlOper.readRecentOpen(QString("%1/%2")
                .arg(QCoreApplication::applicationDirPath())
                .arg(RecentOpenFile.c_str()),
                _vec_RecentOpens_, errText) || _vec_RecentOpens_.isEmpty())
            {
                ui_menu_recentOpen->setEnabled(false);
            }

            for (auto iter = _vec_RecentOpens_.end(); iter != _vec_RecentOpens_.begin(); )
            {
                QAction* action = new QAction(*(--iter));
                ui_menu_recentOpen->addAction(action);
                connect(action, &QAction::triggered, [=]()
                    {
                        QString strFile = action->text();
                        LoadProject(strFile);
                        //LOGI(OPENRECENTPROJECT + strFile.toStdString());
                    });
            }

            if (BlockObject::isChineseVersion())
            {
                ui_action_save = new QAction(QIcon(":/new/prefix1/skin/savex1.png"), tr("保存"), this);
            }
            else
            {
                ui_action_save = new QAction(QIcon(":/new/prefix1/skin/savex1.png"), tr("&Save"), this);
            }

            ui_action_save->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
            ui_action_save->setEnabled(false);

            if (BlockObject::isChineseVersion())
            {
                ui_action_quit = new QAction(QIcon(":/new/prefix1/skin/quitx1.png"), tr("退出"), this);
            }
            else
            {
                ui_action_quit = new QAction(QIcon(":/new/prefix1/skin/quitx1.png"), tr("&Quit"), this);
            }
            ui_action_quit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

            ui_menu_project->addAction(ui_action_newProject);
            ui_menu_project->addAction(ui_action_open);
            ui_menu_project->addMenu(ui_menu_recentOpen);
            ui_menu_project->addAction(ui_action_save);
            ui_menu_project->addSeparator();
            ui_menu_project->addAction(ui_action_quit);

            ui_toolBar->addAction(ui_action_newProject);
            ui_toolBar->addAction(ui_action_open);
            ui_toolBar->addAction(ui_action_save);

           
            //Block menu

            if (BlockObject::isChineseVersion())
            {
                ui_menu_block = new QMenu(tr("区块"), this);
                ui_action_newBlock = new QAction(tr("新建区块"), this);
                ui_action_importBlocks = new QAction(tr("导入区块"), this);
              /*  ui_action_exportMeasurementToXml = new QAction(tr("导出刺点文件"), this);
                ui_action_importMeasurementToXml = new QAction(tr("导入刺点文件"), this);*/
            }
            else
            {
                ui_menu_block = new QMenu(tr("Block"), this);
                ui_action_newBlock = new QAction(tr("New &Block"), this);
                ui_action_importBlocks = new QAction(tr("Import &Block"), this);
             /*   ui_action_exportMeasurementToXml = new QAction(tr("export &Measurements"), this);
                ui_action_importMeasurementToXml = new QAction(tr("Import &Measurements"), this);*/
            }

            ui_menu_block->setEnabled(false);
            
            ui_menu_block->addAction(ui_action_newBlock);
            ui_menu_block->addAction(ui_action_importBlocks);
            

            //ui_menu_block->addAction(ui_action_exportToXml);

            //Engine menu
            if (BlockObject::isChineseVersion())
            {
                ui_menu_engine = new QMenu(tr("引擎"), this);
                ui_action_engine = new QAction(tr("启动引擎"), this);
            }
            else
            {
                ui_menu_engine = new QMenu(tr("Engine"), this);
                ui_action_engine = new QAction(tr("Start engine"), this);
            }

            ui_menu_engine->addAction(ui_action_engine);
            ui_menu_engine->setEnabled(true);

            if (1)//add by chy 2025/4/11 
            {
                if (BlockObject::isChineseVersion())
                {
                    ui_menu_settings = new  QMenu(tr("任务"), this);
                    ui_action_set = new QAction(tr("Job队列设置"), this);
                    if (AI3D::CORE::Application::Getinstance().GetDistribution() >0)
                         ui_action_viewEngineNode = new QAction(tr("查看引擎节点"), this);
                }
                else
                {
                    ui_menu_settings = new  QMenu(tr("Job"), this);
                    ui_action_set = new QAction(tr("Job setting"), this);
                    if (AI3D::CORE::Application::Getinstance().GetDistribution() > 0)
                          ui_action_viewEngineNode = new QAction(tr("View engine node"), this);

                }


                ui_menu_settings->addAction(ui_action_set);
                //ui_menu_settings->addAction(ui_action_viewEngineNode);
            }
            if(0)//add by chy 2025/4/11 
            {
                if (BlockObject::isChineseVersion())
                {
                    ui_menu_tools = new QMenu(tr("工具"), this);
                    ui_action_dataPreprocess = new QAction(tr("数据预处理"), this);
                }
                else
                {
                    ui_menu_tools = new QMenu(tr("Tools"), this);
                    ui_action_dataPreprocess = new QAction(tr("Data preprocess"), this);
                }

                if(bSupportDataPreprocess4TestPurpose)
                { 
                    // note: make it invisible for no use now.
                    LOGI("add data preprocess action.");
                    ui_menu_tools->addAction(ui_action_dataPreprocess);
                }
                else
                {
                    LOGI("not add data preprocess action.");
                }
                //}
            }

            //Help menu
            if (BlockObject::isChineseVersion())
            {
                ui_menu_help = new QMenu(tr("帮助"), this);
                ui_action_usermanual = new QAction(tr("用户手册"), this);
                ui_action_about = new QAction(tr("关于"), this);
                ui_action_at_formatTransfer = new QAction(tr("空三colmap转换"), this);
                ui_action_rec_formatTransfer = new QAction(tr("重建colmap转换"), this);
            }
            else
            {
                ui_menu_help = new QMenu(tr("Help"), this);
                ui_action_usermanual = new QAction(tr("User Manual"), this);
                ui_action_about = new QAction(tr("About"), this);
                ui_action_at_formatTransfer = new QAction(tr("AT Format Transfer"), this);
                ui_action_rec_formatTransfer = new QAction(tr("Rec Format Transfer"), this);
            }

            ui_menu_help->addAction(ui_action_usermanual);
            ui_menu_help->addAction(ui_action_about);
            ui_menu_help->addAction(ui_action_at_formatTransfer);
            ui_menu_help->addAction(ui_action_rec_formatTransfer);

            //ui_action_about->setIcon(QIcon("yescorner.png"));

            //ui_menu_about->setStyleSheet("QMenu::icon{ position:absolute;right:10px;}");

            //ui_menu_help->setIcon(QIcon("yescorner.png"));
            
            //ui_menu_help->setStyleSheet("QMenu::icon{position:absolute;left:10px;}");

            //ui_action_about2 = new QWidgetAction(ui_menu_help);

            //QLabel* lblUpdate = new QLabel("About2",this);
            ////lblUpdate->setAlignment(Qt::AlignLeft);
            ////lblUpdate->setPixmap(QPixmap("yescorner.png"));

            //ui_action_about2->setDefaultWidget(lblUpdate);
            //ui_menu_help->addAction(ui_action_about2);

            //Right click menu on treeview's item
            ui_menu_rightClick_project = new QMenu(this);

            ui_action_importBlocks->setEnabled(true);

            if (BlockObject::isChineseVersion())
            {
                ui_action_exportToXml = new QAction(tr("空三导出"), this);
                ui_action_importGcpMeasurements = new QAction(tr("导入刺点成果"), this);
                ui_action_exportGcpMeasurements = new QAction(tr("导出刺点成果"), this);
                ui_action_rename = new QAction(tr("重命名"), this);
                ui_action_delete = new QAction(tr("删除"), this);
                ui_action_openFileInExplorer = new QAction(tr("打开文件夹..."), this);
            }
            else
            {
                ui_action_exportToXml = new QAction(tr("Export block"), this);
                ui_action_importGcpMeasurements = new QAction(tr("Import GCP measurements"), this);
                ui_action_exportGcpMeasurements = new QAction(tr("Export GCP measurements"), this);
                ui_action_rename = new QAction(tr("Rename"), this);
                ui_action_delete = new QAction(tr("Delete"), this);
                ui_action_openFileInExplorer = new QAction(tr("Open folder..."), this);
            }

            ui_menu_rightClick_project->addAction(ui_action_newBlock);
           ui_menu_rightClick_project->addAction(ui_action_importBlocks);
           //ui_menu_rightClick_project->addAction(ui_action_rename);
           ui_menu_rightClick_project->addAction(ui_action_openFileInExplorer);

          /* ui_menu_rightClick_project->addAction(ui_action_exportMeasurementToXml);
           ui_menu_rightClick_project->addAction(ui_action_importMeasurementToXml);*/

            ui_menu_rightClick_block = new QMenu(this);
            if (BlockObject::isChineseVersion())
            {
                ui_action_merge_and_adjust_blocks = new QAction(tr("空三合并优化"), this);
                ui_action_delete_more = new QAction(tr("删除"), this);
                ui_action_merge_blocks = new QAction(tr("空三合并"), this);
                ui_action_clone_block = new QAction(tr("复制"), this);
                ui_action_copy_gcpmeasurements = new QAction(tr("拷贝GCP"), this);
               // ui_action_simplify_block = new QAction(tr("空三简化"), this);
            }
            else
            {
                ui_action_merge_and_adjust_blocks = new QAction(tr("Merge And Adjust Blocks"), this);
                ui_action_delete_more = new QAction(tr("Delete"), this);
                ui_action_merge_blocks = new QAction(tr("Merge blocks"), this);
                ui_action_clone_block = new QAction(tr("Clone"), this);
                ui_action_copy_gcpmeasurements = new QAction(tr("Copy GCPs"), this);
               // ui_action_simplify_block = new QAction(tr("Simplify block"), this);
            }

            //ui_action_data_preprocess = new QAction(tr("Data Preprocess"),this);

          //  ui_action_simplify_block->setEnabled(false);
            //chy 
            //ui_menu_rightClick_block->addAction(ui_action_exportToXml);
            ui_menu_rightClick_block->addAction(ui_action_openFileInExplorer);
            ui_menu_rightClick_block->addAction(ui_action_rename);
            //ui_action_rename->setSeparator(true);
            ui_menu_rightClick_block->addAction(ui_action_exportToXml);
            /*ui_menu_rightClick_block->addAction(ui_action_importGcpMeasurements);
            ui_menu_rightClick_block->addAction(ui_action_exportGcpMeasurements);*/
            //ui_action_exportGcpMeasurements->setSeparator(true);
            ui_menu_rightClick_block->setStyleSheet("QMenu{QMenu::item:!enabled{color:grey;}");
            //ui_menu_rightClick_block->addAction(ui_action_simplify_block);
            ui_menu_rightClick_block->addAction(ui_action_clone_block);
            ui_menu_rightClick_block->addAction(ui_action_delete);
            
            //ui_menu_rightClick_block->addAction(ui_action_data_preprocess);

            ui_menu_rightClick_selectRows = new QMenu(this);
            //ui_menu_rightClick_selectRows->addAction(ui_action_merge_and_adjust_blocks);
            ui_menu_rightClick_selectRows->addAction(ui_action_merge_blocks);
            ui_menu_rightClick_selectRows->addAction(ui_action_copy_gcpmeasurements);
            /// note:for deleting more blocks simultaneously.
            ///ui_menu_rightClick_selectRows->addAction(ui_action_delete_more);

            ui_menu_rightClick_Reconstruction = new QMenu(this);
            ui_menu_rightClick_Reconstruction->addAction(ui_action_openFileInExplorer);
            ui_menu_rightClick_Reconstruction->addAction(ui_action_rename);
            ui_menu_rightClick_Reconstruction->addAction(ui_action_clone_block);
            ui_menu_rightClick_Reconstruction->setStyleSheet("QMenu{QMenu::item:!enabled{color:grey;}");
            ui_menu_rightClick_Reconstruction->addAction(ui_action_delete);       
            


            ui_menu_rightClick_Production = new QMenu(this);
            ui_menu_rightClick_Production->addAction(ui_action_openFileInExplorer);
            ui_menu_rightClick_Production->addAction(ui_action_rename);
            ui_menu_rightClick_Production->setStyleSheet("QMenu{QMenu::item:!enabled{color:grey;}");
            ui_menu_rightClick_Production->addAction(ui_action_delete);

            ////个人中心
            //if (BlockObject::isChineseVersion())
            //{
            //    ui_menu_user = new QMenu(tr("个人中心"), this);
            //    ui_action_login = new QAction(tr("登录"), this);
            //    ui_action_info = new QAction(tr("账号"), this);
            //    ui_action_logout = new QAction(tr("登出"), this);
            //}
            //else
            //{
            //    ui_menu_user = new QMenu(tr("User"), this);
            //    ui_action_login = new QAction(tr("Login"), this);
            //    ui_action_info = new QAction(tr("Info"), this);
            //    ui_action_logout = new QAction(tr("Logout"), this);
            //}

            //ui_menu_user->addAction(ui_action_login);
            //ui_menu_user->addAction(ui_action_logout);
            //ui_menu_user->addAction(ui_action_info);

            //menubar append menu
            ui_menuBar->addMenu(ui_menu_project);
            ui_menuBar->addMenu(ui_menu_block);
            ui_menuBar->addMenu(ui_menu_engine);
            //comment by chy 2025/04/11
          ui_menuBar->addMenu(ui_menu_settings);         
           // ui_menuBar->addMenu(ui_menu_tools);

            ui_menuBar->addMenu(ui_menu_help);

            //ui_menuBar->addMenu(ui_menu_user);
            
        }

        void MohackerWin::Slot_NewProductionStarted(AI3D::CORE::BlockObject* block, reconstruction_t recons_id, QStandardItem* pConstructionItem)
        {
            SaveProject_Wait();
        }

        void MohackerWin::Slot_NewProduction(AI3D::CORE::BlockObject* block, reconstruction_t recons_id, production_t production_id,QStandardItem *pConstructionItem)
        {
            if (!pConstructionItem)
                return;

            ConstructionWgt* pConstructionWgt = nullptr;
            pConstructionWgt = dynamic_cast<ConstructionWgt*>(pConstructionItem->data(CustomRole::CRBlockWgt).value<QWidget*>());
            if (pConstructionWgt)
            {
                std::ostringstream oss;
                oss << "inside " <<  " " << __FUNCTION__ << " " << __LINE__ << " got constructionwgt:" << 
                    std::hex << std::showbase << pConstructionWgt << std::dec;
                LOGI(oss.str());
            }
            else
            {
                std::ostringstream oss;
                oss << "inside " <<  " " << __FUNCTION__ << " " << __LINE__ << " didn't get constructionwgt(nulptr).";
                LOGI(oss.str());
            }

            AI3D::CORE::ReconstructionObject* pReconstructionObject = block->GetReconstruction(recons_id);
            if (pReconstructionObject)
            {
                AI3D::CORE::ProductionObject* pProductionObject = pReconstructionObject->GetProduction(production_id);
                if (pProductionObject)
                {
                    QStandardItem* pNewProductionItem = new QStandardItem(str2qstr(const_cast<std::string&>(pProductionObject->GetName())));
                    pNewProductionItem->setData(ItemType::ITProduction, CustomRole::CRItemType);
                    pNewProductionItem->setData(QVariant::fromValue(block), CustomRole::CRParentBlockData);
                    pNewProductionItem->setData(QVariant::fromValue(pReconstructionObject), CustomRole::CRReconstructionData);
                    pNewProductionItem->setData(QVariant::fromValue(pProductionObject), CustomRole::CRProductionData);

                    pNewProductionItem->setFlags(pNewProductionItem->flags() & ~Qt::ItemIsEditable);

                    pConstructionItem->appendRow(pNewProductionItem);
                    ui_treeView_project->expandAll();

                    ProductionWgt* productionWgt = new ProductionWgt(block, pReconstructionObject, pConstructionItem,pProductionObject,pNewProductionItem);
                    pNewProductionItem->setData(QVariant::fromValue(productionWgt), CustomRole::CRBlockData);
                    pNewProductionItem->setData(QVariant::fromValue((QWidget *)productionWgt), CustomRole::CRBlockWgt);

                    ui_stackedWidget->addWidget(productionWgt);

                    QModelIndex newCurrentIndex = _proxy_->mapFromSource(pNewProductionItem->index());
                    ui_treeView_project->setCurrentIndex(newCurrentIndex);

                    ui_stackedWidget->setCurrentWidget(productionWgt);

                    if (pConstructionWgt != nullptr)
                    {
                        productionWgt->pConstructionWgt = pConstructionWgt;
                        connect(productionWgt, &ProductionWgt::signal_delete_production_done, pConstructionWgt, &ConstructionWgt::Slot_Delete_Production_Done);
                    }
                }
            }

            SaveProject_Wait();
        }

        void MohackerWin::Slot_TreeItemChanged(QStandardItem* item)
        {

        }

        // check the positive insert pos when adding new reconstruction item.
        void MohackerWin::Slot_NewConstruction(AI3D::CORE::BlockObject* block, reconstruction_t recons_id)
        {
            BlockWgt* pBlockWgt = static_cast<BlockWgt*>(sender());
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            //std::cout << "inside Slot_NewConstruction:" << currentIndex.row() << std::endl;

            if (!block)
                return;

            AI3D::CORE::BlockObject* pCurrentBlock = nullptr;
            if (currentIndex.data(CustomRole::CRItemType).value<ItemType>() == ItemType::ITBlock)
            {
                pCurrentBlock = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            }

            SetFileModifiedProj();

            if (pCurrentBlock != block)
            {
                pCurrentBlock = block;
            }

            if (pCurrentBlock == block)
            {
                ProjectManager* manager = ProjectManager::GetInstance();
                
                //std::cout << "mhw:" << currentIndex.row() << std::endl;

                QStandardItem* pFoundStandardItem = nullptr;
                AI3D::CORE::BlockObject* pFoundBlock = nullptr;

                for (int i = 0; i < project_root_item_->rowCount(); i++)
                {
                    QStandardItem* pTmpItem = project_root_item_->child(i);
                    pFoundBlock = pTmpItem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();

                    if (pFoundBlock && pFoundBlock == block)
                    {
                        pFoundStandardItem = pTmpItem;
                        pFoundBlock = block;
                        
                        break;
                    }
                }

                QStandardItem *pCurrentStandardItem = project_root_item_->child(currentIndex.row());
                if (pCurrentStandardItem != pFoundStandardItem && pFoundStandardItem)
                {
                    pCurrentStandardItem = pFoundStandardItem;
                }

                if (pCurrentStandardItem)
                {
                    AI3D::CORE::ReconstructionObject* pReconstructionObject = block->GetReconstruction(recons_id);
                    if (pReconstructionObject)
                    {
                        QStandardItem* pNewConstructionItem = new QStandardItem(str2qstr(const_cast<std::string&>(pReconstructionObject->GetName())));

                        //std::cout << "append reconst:" << recons_id << " " << pReconstructionObject->GetId() << " " << pReconstructionObject->GetName() << std::endl;

                        pNewConstructionItem->setData(ItemType::ITReconstruction, CustomRole::CRItemType);

                        pCurrentStandardItem->appendRow(pNewConstructionItem);
                        ui_treeView_project->expandAll();                            
                        
                        ConstructionWgt* constructionWgt = new ConstructionWgt(block,pReconstructionObject,pNewConstructionItem);
                        connect(constructionWgt, &ConstructionWgt::Sig_NewProduction, this, &MohackerWin::Slot_NewProduction);
                        connect(constructionWgt, &ConstructionWgt::Sig_ProjModifed, this, &MohackerWin::Slot_ProjModifed);
                        
                        pNewConstructionItem->setData(QVariant::fromValue(block), CustomRole::CRParentBlockData);
                        pNewConstructionItem->setData(QVariant::fromValue(pReconstructionObject), CustomRole::CRReconstructionData);
                        pNewConstructionItem->setData(QVariant::fromValue(constructionWgt), CustomRole::CRBlockData);
                        pNewConstructionItem->setData(QVariant::fromValue((QWidget*)constructionWgt), CustomRole::CRBlockWgt);

                        ui_stackedWidget->addWidget(constructionWgt);

                        //QModelIndex newCurrentIndex = _itemmodel_->indexFromItem(pNewConstructionItem);                        

                        QModelIndex newCurrentIndex = _proxy_->mapFromSource(pNewConstructionItem->index());
                        ui_treeView_project->setCurrentIndex(newCurrentIndex);
                        ui_stackedWidget->setCurrentWidget(constructionWgt);
                        
                        //ui_treeView_project->setCurrentIndex(newCurrentIndex);
                    }
                }
            }
            else if (!pCurrentBlock)
            {
                // Q:if current selected item from the project tree is not block type or has not chosen any item,what will happen?consider it later in depth.
                // A:aparently,if current selected item is not block type,the submit button labeled "New Reconstruction" can not be seen and clicked,
                //   so it is impossible to enter here under other item type. 
                if (currentIndex.isValid())
                {

                }
                else
                {

                }
            }
        }

        void MohackerWin::Slot_ProjModifed()
        {
            SetFileModifiedProj();
        }

        void MohackerWin::Slot_Action_SimplifyBlock()
        {

            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>())
            {
            case ItemType::ITProject:
            {

            }
            break;
            case ItemType::ITBlock:
            {



                AI3D::CORE::BlockObject* block_data_ = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                ProjectManager* manager = ProjectManager::GetInstance();
                auto project_ptr = manager->GetProject();
               //先弹出对话框设置抽稀参数
                
                AI3D::CORE::ATData::SimplifyOptions simopts;
               
                std::string configStr = AI3D::CORE::Application::Getinstance().GetConfigPath();
                AI3D::CORE::String::StringTrim(configStr, "/");
                std::string inifile = AI3D::CORE::File::EnsureUnifySlash(configStr + "/Simplify.ini");
                if (AI3D::CORE::File::ExistsFile(inifile))
                {
                    simopts.Load(inifile);
                }
                else
                {
                    simopts.max_overlap_ = -1;
                    simopts.min_overlap_ = 3;
                    simopts.max_proj_error_ = 1.2;
                    simopts.max_tiepoint_count_ = 200000;
                }
                block_data_->LoadTiepoints();
                /*if (!block_data_->GetTiepointStatus())
                {
                    if (block_data_->GetCurrentATMutual()->HasRegImages())
                    {
                        if (block_data_->LoadTiepointsBinary(block_data_->GetTaskInfo().Tiepoints, block_data_->GetCurrentATMutual()))
                        {
                            block_data_->SetTiepointStatus(true);
                        }
                    }
                }*/
                int num_tiepoints = (block_data_->GetTiepointStatus() ? block_data_->GetCurrentAT()->GetPoints3D().size() : block_data_->GetTaskInfo().statisticinfo_.tiepointnum);

                if (num_tiepoints > 0)
                {
                    block_data_->GetCurrentATMutual()->Simplify(simopts);
                    //此处的保存会涉及到三个文件，chy
                    SetFileModifiedProj();
                    manager->SetProejctModified(true);
                }
            }
            }
        }

        void MohackerWin::Slot_Action_CopyGCPsFromBlock2Block()
        {
            QItemSelectionModel* model_selection = ui_treeView_project->selectionModel();

            QModelIndexList IndexList = model_selection->selectedIndexes();
            std::set<block_t> blockids;
            std::vector<block_t> blockidvec;
            QModelIndex index_target = *IndexList.begin();
            if (IndexList.size() > 1)//可以是多个，后边所有的刺点结果放到第一个且不负责检测gcp是否相同
            {
                foreach(QModelIndex index, IndexList)
                {
                    if (!index.isValid()) return;
                    if (index.data(CustomRole::CRItemType).value<ItemType>() != ItemType::ITBlock)
                    {
                        return;
                    }
                    else
                    {
                        AI3D::CORE::BlockObject* block = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                        blockids.insert(block->GetId());
                        //std::cout << block->GetId() << std::endl;
                        blockidvec.push_back(block->GetId());
                    }
                }
            }


            //project
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            auto project_ptr = promanager->GetProject();
            AI3D::CORE::BlockObject* block_new = project_ptr->GetBlock(blockidvec[0]);
            AI3D::CORE::BlockObject* block_target = new AI3D::CORE::BlockObject();
            *block_target = *block_new;

            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create ob:" << std::hex << std::showbase << block_target << std::dec;
                LOGI(oss.str());
            }

            EIGEN_STL_UMAP(point3D_t, class AI3D::CORE::ControlPoint) controlpoints;
            auto atdata_target = block_target->GetCurrentATMutual();//基准
           //chy @attention此处会有一个Bug就是一个以上的block会存在gcp在不同坐标系，这个还没测试完全
            point3D_t gcpindex = 0;
            for (int i=1;i<blockidvec.size();i++)
            {
                AI3D::CORE::BlockObject* block_src = project_ptr->GetBlock(blockidvec[i]);
                auto atdata_withgcp = block_src->GetCurrentATMutual();
               
                std::map<int, int> imgids;
                for (auto iterimg : atdata_target->GetImages())
                {
                    std::string fullname = iterimg.second.GetPath() + "/"+iterimg.second.GetName();
                    fullname = AI3D::CORE::File::EnsureUnifySlash(fullname);
                    auto img = atdata_withgcp->FindImageWithFullName(fullname);// atdata_withgcp->FindImageWithName(iterimg.second.GetName(), atdata_withgcp->GetImagesIds());
                    if (img != nullptr)
                    {
                        imgids[img->GetImageId()] = iterimg.first;
                    }
                }

                //去除控制点
                block_target->GetCurrentATMutual()->GetControlPointsMutual().clear();
                //获取原始控制点
               
                auto& gcps = atdata_withgcp->GetControlPointsMutual();
                for (auto& itergcp : gcps)
                {

                    int id = block_target->ExistSRS(itergcp.second.GetSrs().definition);
                    if (id == kInvalidSrsId)//说明有
                    {
                        //插入一个
                        block_target->UpdateSRSMap(itergcp.second.GetSrs());
                        id = block_target->ExistSRS(itergcp.second.GetSrs().definition);
                        //适应从Core导出的block_Absolute_xml中GCP没有srs_id的情况

                    }
                    itergcp.second.SetSrs(block_target->GetSRSsMutual()[id]);
                    for (auto& measure : itergcp.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
                    {
                        if (imgids.count(measure.image_id))
                        {
                            measure.image_id = imgids.at(measure.image_id);
                        }
                    }
                    
                    controlpoints[gcpindex] = itergcp.second;
                    gcpindex++;
                }
                
            }
            block_target->GetCurrentATMutual()->SetControlPoints(controlpoints);
            block_target->GetTaskInfoMutual().isLoaded = false;
            block_target->GetTaskInfoMutual().isSaved = false;
            //QWidget* currentWgt = index_target.data(CustomRole::CRBlockWgt).value<QWidget*>();
           
            //if (currentWgt)
            //{
            //    BlockWgt* Wgt = dynamic_cast<BlockWgt*>(currentWgt);
            //    Wgt->InsertGCPTab();
            //}
            //else
            //{
            //    return;//chy其实应该为抛出异常
            //}
            {
                ProjectManager* manager = ProjectManager::GetInstance();
                auto project_ptr = manager->GetProject();
                //拷贝blk，blk需改名需新建文件夹需更新内容；
               /* block_t id = block_target->GetId();*/
               
                project_ptr->CloneBlock(block_target,"-gcpopy");
                LOGI("Clone Block Finished!");

                AI3D::CORE::BlockObject* newblock = project_ptr->GetBlock(block_target->GetId());


                //待测试
                jobsta_e nextstatus = jobsta_e::STATUS_NEW;
                newblock->SetStatus(nextstatus);
                manager->AddBlockManager(newblock);

                //显示到界面上
                QStandardItem* item_ = GetBlockATData(newblock);
                ShowBlockWidget(newblock, item_, true);


                //保存工程
                QApplication::processEvents();
                newblock->GetTaskInfoMutual().isSaved = false;
                item_->setData(2, CustomRole::CanSaveBlock);
                //此处的保存会涉及到三个文件，chy
                SetFileModifiedProj();
                manager->SetProejctModified(true);
            }
           
            /*item_->setData(2, CustomRole::CanSaveBlock);*/
            SetFileModifiedProj();
            promanager->SetProejctModified(true);
        }

        void MohackerWin::CreateConnections()
        {
            //connect(ui_action_simplify_block, &QAction::triggered, this, &MohackerWin::Slot_Action_SimplifyBlock);
            connect(ui_action_copy_gcpmeasurements, &QAction::triggered, this, &MohackerWin::Slot_Action_CopyGCPsFromBlock2Block);

            connect(ui_action_newProject, &QAction::triggered, this, &MohackerWin::Slot_Action_NewProject);
            connect(ui_action_open, &QAction::triggered, this, &MohackerWin::Slot_Action_OpenProject);
            connect(ui_action_save, &QAction::triggered, this, &MohackerWin::Slot_Action_SaveProject);
            connect(ui_action_quit, &QAction::triggered, this, &QWidget::close);
            
            

            connect(ui_action_newBlock, &QAction::triggered, this, &MohackerWin::Slot_Action_NewBlock);
            connect(ui_action_importBlocks, &QAction::triggered, this, &MohackerWin::Slot_Action_ImportBlocks);
            connect(ui_action_exportToXml, &QAction::triggered, this, &MohackerWin::Slot_Action_ExportToXml);
            connect(ui_action_importGcpMeasurements, &QAction::triggered, this, &MohackerWin::Slot_Action_ImportMeasurementFromXml);
            connect(ui_action_exportGcpMeasurements, &QAction::triggered, this, &MohackerWin::Slot_Action_ExportMeasurementToXml);
            connect(ui_action_rename, &QAction::triggered, this, &MohackerWin::Slot_Action_RenameBlock);
            connect(ui_action_delete, &QAction::triggered, this, &MohackerWin::Slot_Action_DeleteBlock2);
            connect(ui_action_clone_block, &QAction::triggered, this, &MohackerWin::Slot_CloneBlock);
            //connect(ui_action_data_preprocess, &QAction::triggered, this, &MohackerWin::Slot_Action_DataPreProcessRight);

            connect(ui_action_usermanual, &QAction::triggered, this, &MohackerWin::Slot_Action_UserManual);
            connect(ui_action_about, &QAction::triggered, this, &MohackerWin::Slot_Action_About);
            connect(ui_action_at_formatTransfer, &QAction::triggered, this, &MohackerWin::Slot_Action_ExportATToColmap);
            connect(ui_action_rec_formatTransfer, &QAction::triggered, this, &MohackerWin::Slot_Action_ExportRecToColmap);
            connect(ui_action_openFileInExplorer, &QAction::triggered, this, &MohackerWin::Slot_Action_OpenFolder);
            connect(ui_action_engine, &QAction::triggered, this, &MohackerWin::Slot_Action_OpenEngine);
            connect(ui_action_merge_blocks, &QAction::triggered, this, &MohackerWin::Slot_Action_Merge_blocks);
            connect(ui_action_merge_and_adjust_blocks, &QAction::triggered, this, &MohackerWin::Slot_Action_Merge_And_Ajust_blocks);
            connect(ui_action_delete_more, &QAction::triggered, this, &MohackerWin::Slot_Action_DeleteMore);

            /*connect(ui_action_login, &QAction::triggered, this, &MohackerWin::Slot_Action_Login);
            connect(ui_action_info, &QAction::triggered, this, &MohackerWin::Slot_Action_Info);
            connect(ui_action_logout, &QAction::triggered, this, &MohackerWin::Slot_Action_Logout);*/
            
///            QItemSelectionModel* model_selection = ui_treeView_project->selectionModel();
            //connect(model_selection,&QItemSelectionModel::currentChanged,this,&MohackerWin::TreeViewClicked);//ly20220224左右鼠标键均响应替代下边的槽函数

            connect(ui_treeView_project, &QTreeView::clicked, this, &MohackerWin::Slot_ProjectTreeView_ItemClicked);
            connect(ui_treeView_project, &QTreeView::doubleClicked, this, &MohackerWin::Slot_ProjectTreeView_ItemDoubleClicked);
            connect(ui_treeView_project, &QTreeView::customContextMenuRequested, this, &MohackerWin::Slot_TreeView_CustomContextMenuRequested);

            connect(atdia, SIGNAL(WriteATColmapFile()), this, SLOT(TransferATColmap()));
            connect(recdia, SIGNAL(WriteRecColmapFile()), this, SLOT(TransferRecColmap()));

            connect(xmldia, SIGNAL(WriteXmlFile()), this,SLOT(SaveXmlFile()));
            //add by chy 2025/04/11 
            if(1)
            {
                connect(ui_action_set, &QAction::triggered, this, &MohackerWin::Slot_Action_Settings);
                if (AI3D::CORE::Application::Getinstance().GetDistribution() > 0)
                    connect(ui_action_viewEngineNode, &QAction::triggered, this, &MohackerWin::Slot_Action_ViewEngineNode);
             //  connect(ui_action_dataPreprocess, &QAction::triggered, this, &MohackerWin::Slot_Action_DataPreProcess);
            }
           

        }
      
        void MohackerWin::Slot_CloneBlock()
        {
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) 
            {
            case ItemType::ITProject:
            {

            }
            break;
            case ItemType::ITBlock:
            {
                

              
                AI3D::CORE::BlockObject* block_data_ = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                ProjectManager* manager = ProjectManager::GetInstance();
                auto project_ptr = manager->GetProject();
                //拷贝blk，blk需改名需新建文件夹需更新内容；
                block_t id = block_data_->GetId();
               /* LOGI(String::StringPrintf("Cloning %s(%s)", block_data_->GetName().c_str(), block_data_->GetTaskInfo().blockString.c_str()));*/
             //chy @todo 是否需要加QFuture 
                project_ptr->CloneBlock(id, false);
                LOGI("Clone Block Finished!");
                
                
                
                AI3D::CORE::BlockObject* newblock = project_ptr->GetBlock(id);
                
               
                //待测试
               /* jobsta_e nextstatus = jobsta_e::STATUS_NEW;
                newblock->SetStatus(nextstatus);*/
                manager->AddBlockManager(newblock);

                //显示到界面上
                QStandardItem* item_ = GetBlockATData(newblock);
                ShowBlockWidget(newblock, item_, true);
               
              
                //保存工程
                QApplication::processEvents();
                newblock->GetTaskInfoMutual().isSaved = false;
                item_->setData(2, CustomRole::CanSaveBlock);
                //此处的保存会涉及到三个文件，chy
                SetFileModifiedProj();
                manager->SetProejctModified(true);
              
            }
            break;
            case ItemType::ITReconstruction:
            {
                LOGI("Start clone reconstruction.");
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();

                QStandardItem* pFoundStandardItem = nullptr;
                for (int i = 0; i < project_root_item_->rowCount(); i++)
                {
                    QStandardItem* pTmpItem = project_root_item_->child(i);
                    AI3D::CORE::BlockObject* pFoundBlock = pTmpItem->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();

                    if (pFoundBlock && pFoundBlock == blockData)
                    {
                        pFoundStandardItem = pTmpItem;
                        break;
                    }
                }

                ConstructionWgt* constructWgt = currentIndex.data(CustomRole::CRBlockData).value<ConstructionWgt*>();
                if (constructWgt && pFoundStandardItem)
                {
                    AI3D::CORE::ReconstructionObject* recons_object = constructWgt->getReconstructionObject();
                    group_t new_reconstruction_id;
                    blockData->CloneReconstruction(recons_object->GetId(), new_reconstruction_id);

                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();                   
                    block_t id = blockData->GetId();                

                    AI3D::CORE::ReconstructionObject* newdata = blockData->GetReconstruction(new_reconstruction_id);

                    //显示到界面上

                    QStandardItem* item_;// = Getrec(newdata);

                    item_ = new QStandardItem(str2qstr(const_cast<std::string&>(newdata->GetName())));
                    item_->setData(ItemType::ITReconstruction, CustomRole::CRItemType);

                    ConstructionWgt* constructionWgt = new ConstructionWgt(blockData, newdata,item_);
                    connect(constructionWgt, &ConstructionWgt::Sig_NewProduction, this, &MohackerWin::Slot_NewProduction);
                    connect(constructionWgt, &ConstructionWgt::Sig_ProjModifed, this, &MohackerWin::Slot_ProjModifed);

                    item_->setData(QVariant::fromValue(blockData), CustomRole::CRParentBlockData);
                    item_->setData(QVariant::fromValue(newdata), CustomRole::CRReconstructionData);

                    item_->setData(QVariant::fromValue(constructionWgt), CustomRole::CRBlockData);
                    item_->setData(QVariant::fromValue((QWidget *)constructionWgt), CustomRole::CRBlockWgt);

                    ui_stackedWidget->addWidget(constructionWgt);

                    pFoundStandardItem->appendRow(item_);
                    ui_treeView_project->expandAll();

                    QModelIndex newCurrentIndex = _proxy_->mapFromSource(item_->index());
                    ui_treeView_project->setCurrentIndex(newCurrentIndex);
                    ui_stackedWidget->setCurrentWidget(constructionWgt);

                    //保存工程
                    QApplication::processEvents();
                    blockData->GetTaskInfoMutual().isSaved = false;
                    item_->setData(2, CustomRole::CanSaveBlock);//@attetion此处不知道要不要改
                 
                    SetFileModifiedProj();
                    manager->SetProejctModified(true);
                    LOGI("End clone reconstruction.");
                }
            }
            break;
            case ItemType::ITProduction:
            {
               //这个类型没有clone
            }
            break;
            default:
                break;
            }          
        }
      
        void MohackerWin::closeEvent(QCloseEvent* event)
        {          
            if (MaybeSave())
            {
                int ret = ShowMessageBox();
                ProcessMessageBoxResult(ret, ProjectAction_e::PAClose);
                if (QMessageBox::Cancel == ret)
                {                 
                    event->ignore();
                    return;
                }
                else if(QMessageBox::No == ret)
                {
                   
                    SaveThisToRecent();
                                   
                    event->accept();
                    bMainWindowDestroyed = true;                 
                }
                else if (QMessageBox::Yes == ret)
                {
                    ProjectManager* manager = ProjectManager::GetInstance();
                    bool bSaveFinished = false;

                    auto savefunc = [&,this](ProjectManager* manager)
                    {
                        LOGI("preparing to save project...");

                        ///std::this_thread::sleep_for(std::chrono::milliseconds(16000));                       
                        
                        ///return manager->GetProject()->Save(savetype_e::XML_SAVED);
                        int res = manager->GetProject()->Save(savetype_e::XML_SAVED);
                        
                        LOGI("saved project.");
 ///                       std::this_thread::sleep_for(std::chrono::milliseconds(20000));

                        bSaveFinished = true;
                        return res;
                    };
                    
                    LOGI("OpenLoadingPrompt:Saving project now, pls wait for a while...");
                    QString infostr = "Saving project now,pls wait for a while...";
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        infostr = tr("正在保存，请稍等");
                    }
                    OpenLoadingPromptV4(infostr);

                    QFuture<int> f1 = QtConcurrent::run(savefunc, manager);
                                    
                    while (!bSaveFinished)
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
                    
                    CloseLoadingPromptV4();
                    LOGI("Closed Loading Prompt here.");
   ///                 QApplication::restoreOverrideCursor();

                    // auto close the information dialog which has been just popped up.
///                    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                    ///f1.waitForFinished();
                   
                    event->accept();
                    bMainWindowDestroyed = true;
                }
            }
            else
            {
                SaveThisToRecent();
                
                event->accept();
                bMainWindowDestroyed = true;
            }

            ProjectManager* manager = ProjectManager::GetInstance();
            std::string projectname = manager->GetProject()->GetFullName();
           /* std::string lockfile = projectname + LOCKFILE_POSTFIX;
            if (boost::filesystem::exists(lockfile))
            {
                LOGI("start quit projelock ");
                if (fpprojectlock != NULL)
                {
                   int fid =  fclose(fpprojectlock);
                    LOGI("start quit projelock closed "+std::to_string(fid));
                }
                boost::filesystem::remove(lockfile);
            }*/
            //记录软件使用情况


            ///std::string apppath = QApplication::applicationDirPath().toStdString();

            /*std::string apppath = qstr2str(QApplication::applicationDirPath());*/
            std::string apppath = GetWorkPath();

            std::string machineCode = MasterInfo::Getinstance().GetMachineCode();
            std::string masterjsonpath = apppath + PATH_SEPARATOR_STR + machineCode + "Master.json";

            MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->QuitTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toStdString();
            MasterInfo::Getinstance().ExportMasterInfoJson(masterjsonpath);
            event->accept();

        }

        bool MohackerWin::MaybeSave()
        {  
            return ui_action_save->isEnabled();
        }

        int MohackerWin::ShowMessageBox()
        {
            int ret;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                ret = Message_Box::question(this, "", tr("当前工程已经修改，需要保存修改吗？"), Message_Box_Type::Question_Yes_No_Cancel);
            }
            else
            {
                ret = Message_Box::question(this, "", tr("The project has been modified, do you want to save your changes?"), Message_Box_Type::Question_Yes_No_Cancel);
            }
            return ret;
        }

        void MohackerWin::SaveThisToRecent()
        {
            
            if (/*_vec_RecentOpens_.contains(_currentSolutionXmlPath_) ||*/ _currentSolutionXmlPath_.isEmpty())
                return;

           
            if (_currentSolutionXmlPath_.split(".").last() != PROJECTPOSTFIX && _currentSolutionXmlPath_.split(".").last() != BINPROJECTPOSTFIX)
            {
                return;
            }

            if (_vec_RecentOpens_.contains(_currentSolutionXmlPath_))
            {
                _vec_RecentOpens_.removeOne(_currentSolutionXmlPath_);
            }
            else
            {
                if (_vec_RecentOpens_.size() >= RECENTMOST)
                    _vec_RecentOpens_.removeFirst();               
            }
            _vec_RecentOpens_.push_back(_currentSolutionXmlPath_);//顺序插入，反序读出

            XmlOper xmlOper;
            QString errText;
            xmlOper.writeRecentOpen(QString("%1/%2")
                .arg(QCoreApplication::applicationDirPath())
                .arg(RecentOpenFile.c_str()),
                _vec_RecentOpens_, errText);
        }

        void MohackerWin::RefreshRecentOpenMenu()
        {
            SaveThisToRecent();
            ui_menu_recentOpen->clear();

            for (auto iter = _vec_RecentOpens_.end(); iter != _vec_RecentOpens_.begin(); )
            {
                if (!ui_menu_recentOpen->isEnabled())
                {
                    ui_menu_recentOpen->setEnabled(true);
                }
                QAction* action = new QAction(*(--iter));
                ui_menu_recentOpen->addAction(action);
                connect(action, &QAction::triggered, [=]() {
                    SaveThisToRecent();
                    QString strFile = action->text();
                    LoadProject(strFile);
                    });

            }
        }


        void MohackerWin::ProcessMessageBoxResult(int ret, ProjectAction_e action)
        {
            auto processAction = [=]()
            {
                switch (action) {
                case ProjectAction_e::PANew:
                    //newProject();
                    break;
                case ProjectAction_e::PAOpen:
                    //loadProject();
                    break;
                case ProjectAction_e::PAClose:
                    //            this->close();
                    break;
                default:
                    break;
                }
            };

            switch (ret) {
            case QMessageBox::Cancel:
                return;
                break;
            case QMessageBox::No:
            {
                processAction();
            }
            break;

            case QMessageBox::Save:
            {
               Slot_Action_SaveProject();
                processAction();
            }
            break;
            default:
                break;
            }
        }


        
        void MohackerWin::Slot_Action_OpenEngine()
        {
            
            {
                //分布式启动另一个Engine
                CatchProcess mcatchProcess;
                ///if (!mcatchProcess.IsProgramRunning("engine.exe"))
                if (!mcatchProcess.IsProgramRunning("MoldAINode.exe"))
                {
                    QString path = QCoreApplication::applicationDirPath();

                    ///std::string pathstring = path.toStdString();

                    std::string pathstring = qstr2str(path);

                    /// 如果不存在engine.bat则输出此文件
                    /// 
                    std::string batfile = pathstring + "/moldainode.bat";
                    if (AI3D::CORE::File::ExistsFile(batfile))
                    {

                        std::ofstream ofs = AI3D::CORE::File::OpenOfstreamUtf8(batfile, std::ios::out);
                        if (!ofs.fail())
                        {

                            ofs << "start  %~dp0/MoldAINode.exe" << "\n";
                            ofs << "ECHO Run complete. Please press enter to continue." << "\n";
                            ofs.close();
                        }
                    }
                 
                  
                    QString workdirectory = path;
                    QStringList argumentList= QStringList();
                   
                    path.append("/moldainode.bat");

                    ///std::cout << " ===++++ fbs:" << path.toStdString() << " " << workdirectory.toStdString() << std::endl;

                    QProcess::startDetached(path, QStringList(), workdirectory, nullptr);
                    //                   QProcess::startDetached(path, QStringList());
                    //?chy
                    LOGI(OPENENGINE);
                    //QProcess::start(path, QStringList(), workdictory, nullptr);
                }
            }
                 
        }
       
       
        void MohackerWin::Slot_Action_NewProject()
        {
            VersionInfo versionInfo = checkSoftWareVersion();
            QString message;
            if (!versionInfo.checkReturn) {
                //请求出错
                message = "网络问题，请联网后再试";
                QMessageBox errBox;
                errBox.warning(this, "软件版本检测", message);
                return;
            }
            else if (!versionInfo.isValid) {
                //当前版本不合法
                message = "当前软件不可用，请联系软件开发商更新版本";
                QMessageBox errBox;
                errBox.warning(this, "软件版本检测", message);
                return;
            }

            //qt 1：实现弹窗用户用户指定路径
            //2：用户写个名字；
            //check重名
            // ok
            //tri文件，
            //创建ProjecObject
            //project.Save(t.tri);
            
            //preojct.NewBlock()
            //show BlockWgt ui
            //SetStatus();
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() != "")
            {
                if (!CheckProjectIsModifyDlg())
                    return;
            }

            QString prePathName = "";
            NewProjectDlg dlg(prePathName);
            if (QDialog::Accepted != dlg.exec())
                return;

            ClearCurrentProject();
            promanager->GetProject()->Clear();
            _currentSolutionXmlPath_.clear();
            
            ui_menu_block->setEnabled(true);


            emit CloseFirstWgt();
            prePathName = dlg.GetProjectPath();

            //LOGI(OPENPROJECT + prePathName.toStdString());

            promanager->InitBlockManager();
            promanager->SetProejctModified(true);

///            promanager->GetProject()->NewProject( dlg.GetProjectName().toStdString(), dlg.GetProjectPath().toStdString());
///            ui_projectWgt_ = new ProjectInfoWgt(dlg.GetProjectName(), dlg.GetProjectPath(), this);
            
            std::string projName = qstr2str(const_cast<QString &>(dlg.GetProjectName()));
            std::string projPath = qstr2str(const_cast<QString &>(dlg.GetProjectPath()));
            promanager->GetProject()->NewProject(projName,projPath);

            ui_projectWgt_ = new ProjectInfoWgt(dlg.GetProjectName(), dlg.GetProjectPath(), this);
           
            ui_stackedWidget->addWidget(ui_projectWgt_);
            ui_stackedWidget->setCurrentWidget(ui_projectWgt_);
            CreateItemModel();
            ui_treeView_project->expand(project_root_item_->index());
            ui_treeView_project->expandAll();
            //UpdateTreeView();
            //show();
            isnewproject = true;
            Slot_Action_NewBlock();
            isnewproject = false;
            std::string logpath = promanager->GetProject()->GetFullName();
            int num = logpath.find_last_of("/");
            std::string projectpath = logpath.substr(0, num + 1);
            ResetLog(AI3D::CORE::File::GetParentDir(promanager->GetProject()->GetFullName()), projectpath);
            /// note:check it later.(@240321)
#if 1
            if(!currentVersionName.isEmpty())
                LOGI(AI3D::CORE::String::StringPrintf("Software Version: %s", qstr2str(currentVersionName).c_str()));
            else
                LOGI(AI3D::CORE::String::StringPrintf("Software Version: %s", AI3D::GUI::VERSION.c_str()));
#endif
            ///_currentSolutionXmlPath_ = QString(promanager->GetProject()->GetFullName().c_str());
            _currentSolutionXmlPath_ = str2qstr(promanager->GetProject()->GetFullName());
            RefreshRecentOpenMenu();

        }

     

        void MohackerWin::Slot_Action_OpenProject()
        {
           //获取tri；
           /// project.Open();           
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() != "")
            {
                if (!CheckProjectIsModifyDlg())
                    return;
            }          

#if 0
            QFileDialog fd(nullptr, tr("Open project file"), ".", tr("file(*.mai)"));
            fd.setAcceptMode(QFileDialog::AcceptOpen);
            fd.setFileMode(QFileDialog::ExistingFile);
            fd.setViewMode(QFileDialog::Detail);
            fd.setDirectory(QDir(QString(promanager->GetProject()->GetPath().c_str())).absolutePath());
            if (QDialog::Accepted != fd.exec())
                return;
            emit CloseFirstWgt();

            QString filename = fd.selectedFiles().first();
#else
            QFileDialog* pfd = nullptr;
            if (BlockObject::isChineseVersion())
            {
                pfd = new QFileDialog(nullptr, tr("打开工程文件"), ".", tr("文件类型(*.mai)"));
            }
            else
            {
                pfd = new QFileDialog(nullptr, tr("Open project file"), ".", tr("file(*.mai)"));
            }

            pfd->setAcceptMode(QFileDialog::AcceptOpen);
            pfd->setFileMode(QFileDialog::ExistingFile);
            pfd->setViewMode(QFileDialog::Detail);
            pfd->setDirectory(QDir(QString(promanager->GetProject()->GetPath().c_str())).absolutePath());
            if (QDialog::Accepted != pfd->exec())
            {
                delete pfd;
                return;
            }

            emit CloseFirstWgt();

            QString filename = pfd->selectedFiles().first();
            delete pfd;
#endif
            
            LoadProject(filename);
            
            
        }

        
        void MohackerWin::Slot_Action_SaveProject()
        {
            if (!ui_menu_rightClick_block->isEnabled())
            {
                ui_menu_rightClick_block->setEnabled(true);
            }

            for (auto row = 0; row < project_root_item_->rowCount(); row++)
            {
                QStandardItem* Blockitem = project_root_item_->child(row);
                if (Blockitem->data(CustomRole::CanSaveBlock).toInt() == 2)
                {
                    Blockitem->setData(1, CustomRole::CanSaveBlock);
                }
               
            }
            ProjectManager* manager = ProjectManager::GetInstance();
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
            {
                bool bSaveFinished = false;
              
                auto savefunc = [&, this](ProjectManager* manager)
                {
                    int ret = manager->GetProject()->Save(savetype_e::XML_SAVED);
                    bSaveFinished = true;
                    return ret;
                };
                //  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                QFuture<int> f1 = QtConcurrent::run(savefunc, manager);
                LOGI("OpenLoadingPrompt:save block now, pls wait for a while");
                if (BlockObject::isChineseVersion())
                {
                    OpenLoadingPromptV4("正在保存工程，请稍等");
                }
                else
                {
                    OpenLoadingPromptV4("Save block now,pls wait for a while");
                }

                while (!bSaveFinished)
                {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                ui_action_save->setEnabled(false);
                CloseLoadingPromptV4();
            }
            else
            {

                auto savefunc = [this](ProjectManager* manager) {
                    return manager->GetProject()->Save(savetype_e::XML_SAVED);
                };
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                QFuture<int> f1 = QtConcurrent::run(savefunc, manager);
            
                connect(&watcher, SIGNAL(finished()), this, SLOT(SaveFinished()));
                watcher.setFuture(f1);


                ui_action_save->setEnabled(false);

                QApplication::restoreOverrideCursor();
            }
            LOGI(SAVEPROJECT + manager->GetProject()->GetName());
          
        }
        
        void MohackerWin::SaveProject_Wait()
        {
            if (!ui_menu_rightClick_block->isEnabled())
            {
                ui_menu_rightClick_block->setEnabled(true);
            }
            for (auto row = 0; row < project_root_item_->rowCount(); row++)
            {
                QStandardItem* Blockitem = project_root_item_->child(row);
                if (Blockitem->data(CustomRole::CanSaveBlock).toInt() == 2)
                {
                    Blockitem->setData(1, CustomRole::CanSaveBlock);
                }

            }
            ProjectManager* manager = ProjectManager::GetInstance();
            if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() > 2)
            {

                auto savefunc = [this](ProjectManager* manager) {
                    return manager->GetProject()->Save(savetype_e::XML_SAVED);
                };
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                QFuture<int> f1 = QtConcurrent::run(savefunc, manager);
                f1.waitForFinished();
                QApplication::restoreOverrideCursor();
            }
            else
            {
               
                bool bSaveFinished = false;               

                auto savefunc = [&, this](ProjectManager* manager)
                {
                    LOGI("preparing to save project...");

                    int res = manager->GetProject()->Save(savetype_e::XML_SAVED);

                    LOGI("saved project.");
                    

                    bSaveFinished = true;
                    return res;
                };
                //if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() == 2)
                {
                    LOGI("OpenLoadingPrompt:Saving project now, pls wait for a while...");
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        OpenLoadingPromptV4("正在保存工程，请稍等");
                    }
                    else
                    {
                        OpenLoadingPromptV4("Saving project now,pls wait for a while");
                    }
                    
                    QFuture<int> f1 = QtConcurrent::run(savefunc, manager);

                    while (!bSaveFinished)
                    {


                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    CloseLoadingPromptV4();
                }
                /*else
                {
                    savefunc(manager);
                }*/
                LOGI("load end.");
            }
            ui_action_save->setEnabled(false);
            LOGI(SAVEPROJECT + manager->GetProject()->GetName());
            /*connect(&watcher, SIGNAL(finished()), this, SLOT(SaveFinished()));
            watcher.setFuture(f1);*/
        }
       
        void MohackerWin::Slot_Action_NewBlock()
        {

            if (!HasProject())
            {
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a project first"));
                return;
            }
            //project
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
          
            AI3D::CORE::BlockObject* block = new AI3D::CORE::BlockObject(promanager->GetProject()->GetPath());
   
            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create ob:" << std::hex << std::showbase << block << std::dec;
                LOGI(oss.str());
            }

            block->SetStatus(jobsta_e::STATUS_NEW);
          
            promanager->GetProject()->AddBlock(block);
           
            std::string postFix = "";
            if (PROJECT_USE_BIN) {
                postFix = BINDOTPROJECTPOSTFIX;
            }
            else {
                postFix = DOTPROJECTPOSTFIX;
            }
            block->GetTaskInfoMutual().projectfile_ = promanager->GetProject()->GetPath() + "/" + promanager->GetProject()->GetName() + postFix;
            
            LOGI("New Block");
            promanager->AddBlockManager(block);

            //显示到界面上

            //emit A
            

            /*填充block
            
            
            */
            LOGI(NEWBLOCK + block->GetName());
            int rowCount = _itemmodel_->rowCount();
            block->GetTaskInfoMutual().isSaved = false;
            block->GetTaskInfoMutual().isLoaded = true;
            //新建工程不保存
            if (isnewproject)
            {
               
                if (promanager->GetProject()->Save(savetype_e::XML_SAVED) == AI3D_SUCCESS)
                {
                    //加锁
                   /* std::string lockfile = promanager->GetProject()->GetPath() + PATH_SEPARATOR_STR + promanager->GetProject()->GetName() + PROJECTFILE + LOCKFILE_POSTFIX;
                    fpprojectlock = _fsopen(lockfile.c_str(), "wt", _SH_DENYWR);*/
                }
                else
                    return;
            }
            else
            {
               
                SetFileModifiedXml();
            }
            
            QStandardItem* item_ = NewBlock(block, rowCount + 1);
            if (block->GetId() > 1) //注意bloclid从1开始
            {
                item_->setData(2, CustomRole::CanSaveBlock);//chy add cansaveblock ：2 表示没有保存可以保存
            }
            item_->setText(block->GetTaskInfo().blockString.c_str());
            
            ui_stackedWidget->setCurrentWidget(item_->data(CustomRole::CRBlockWgt).value<QWidget*>());
        }
        
        void MohackerWin::Slot_Action_ImportBlocks()
        {          
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() == "")
            {
                /// note: add chinese version.
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a block first"));
                return;
            }
            // note: adjust the following code to adapt chinese environment later..
            QFileDialog fd(nullptr, tr("Import blocks"), ".", tr("Blocks import files(*.xml *.xls *.xlsx);;BlocksExchange XML format(*.xml);;MS Excel block definition(*.xls *.xlsx)"));
            fd.setAcceptMode(QFileDialog::AcceptOpen);
            fd.setFileMode(QFileDialog::ExistingFile);
            fd.setViewMode(QFileDialog::Detail);
          
            fd.setDirectory(QDir(QString(promanager->GetProject()->GetPath().c_str())).absolutePath());
            if (QDialog::Accepted != fd.exec())
                return;
            QString fileName = fd.selectedFiles().first();    
           
          
            std::string file = qstr2str(fileName);
            

            
                if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                {
                    

                        bool bSaveFinished = false;

                        ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

                        auto savefunc = [&, this](ProjectManager* manager)
                        {
                            LOGI("preparing to import block...");


                            bool IsImportBlock = promanager->GetProject()->ImportBlock(file);


                            LOGI("importing block.");

                            bSaveFinished = true;
                            return IsImportBlock;
                        };
                    LOGI("OpenLoadingPrompt:import block now, pls wait for a while");
                    if (BlockObject::isChineseVersion())
                    {
                        OpenLoadingPromptV4("正在导入，请稍等");
                    }
                    else
                        OpenLoadingPromptV4("Import block now,pls wait for a while");

                    QFuture<bool> f1 = QtConcurrent::run(savefunc, promanager);

                    while (!bSaveFinished)
                    {


                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    // 接下来那段代码是不是也可以放在savefun里，@chy comment here @20231213
                    if (!f1.result())
                    {

                        Message_Box::warning(nullptr, tr("Warning"), tr("Parse block file failed!"));

                        LOGE(IMPORTBLOCK + (file)+"Parse block file failed!");

                        CloseLoadingPromptV4();
                        return;

                    }
                    else
                    {



                        promanager->SetProejctModified(true);


                        LOGI(IMPORTBLOCK + (file));
                        // m_pProgressBar->setValue(0);

                        ///AI3D::CORE::BlockObject* block = new AI3D::CORE::BlockObject;
                        AI3D::CORE::BlockObject* block;
                        block = promanager->GetProject()->GetCurrentBlock();

                        promanager->AddBlockManager(block);


                        //统计处理数据量
                        for (const auto& img : block->GetCurrentAT()->GetImages())
                        {
                            std::string imgpath = img.second.GetPath();
                            AI3D::CORE::String::StringToLower(&imgpath);
                            std::string parentdir = AI3D::CORE::File::EnsureUnifySlash(imgpath);
                            MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->PhotosOfDir[parentdir]++;

                        }
                        std::string msg = "import xml " + file;
                        LOGI(msg);
                        for (auto& iter : MasterInfo::Getinstance().GetAPPUseInfosMutual())
                        {
                            auto count = iter.PhotosOfDir.size();
                            std::string msg = std::to_string(count);
                            LOGI(msg);
                        }

                        QStandardItem* item_ = GetBlockATData(block);
                        ShowBlockWidget(block, item_, true);

                        item_->setData(2, CustomRole::CanSaveBlock);
                        SetFileModifiedProj();
                    }

                    CloseLoadingPromptV4();
                    LOGI("Closed Loading Prompt here.");
                }
                else
                {

                    m_pProgressBar->setWindowModality(Qt::ApplicationModal);
                    m_pProgressBar->show();

                    m_pProgressBar->setValue(0);
                    m_pProgressBar->setMaxValue(0);
                    m_pProgressBar->setMinValue(0);
                    importxml_->SetFileName(fileName);
                    importxml_->start();
                }


            

        }



        void MohackerWin::Slot_Action_ExportMeasurementToXml()
        {

        }

        void MohackerWin::Slot_Action_ImportMeasurementFromXml()
        {
            QFileDialog* pfd = nullptr;
            if (BlockObject::isChineseVersion())
            {
                pfd = new QFileDialog(nullptr, tr("打开刺点文件"), ".", tr("文件类型(*.xml)"));
            }
            else
            {
                pfd = new QFileDialog(nullptr, tr("Open Surveys file"), ".", tr("file(*.xml)"));
            }

            pfd->setAcceptMode(QFileDialog::AcceptOpen);
            pfd->setFileMode(QFileDialog::ExistingFile);
            pfd->setViewMode(QFileDialog::Detail);
            //pfd->setDirectory(QDir(QString(promanager->GetProject()->GetPath().c_str())).absolutePath());
            if (QDialog::Accepted != pfd->exec())
            {
                delete pfd;
                return;
            }
            pfd->close();
            //  emit CloseFirstWgt();

            QString filename = pfd->selectedFiles().first();
            delete pfd;

            try
            {
                ///if (!boost::filesystem::is_regular_file(projName.toStdString()))
                if (!std::filesystem::is_regular_file(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(filename))))
                {
                    QString dlgTitle = tr("Error");
                    QString strInfo = tr("file not exist,please select exist file!");
                    ///std::cout << "loadproj0:" << qstr2str(projName) << std::endl;
                    Message_Box::critical(this, dlgTitle, strInfo);
                    return;
                }
            }
            catch (const std::filesystem::filesystem_error& fse)
            {
                std::ostringstream oss;
                oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                LOGI(oss.str());
                return;
            }
            catch (std::exception& ex)
            {
                std::ostringstream oss;
                oss << "exception:" << ex.what();
                LOGI(oss.str());
                return;
            }

            EIGEN_STL_UMAP(srsid_t, srs_s) srs_map;
            EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) cps_map;
            EIGEN_STL_UMAP(image_t, std::string) image_map;

            AI3D::CORE::BlockObject::LoadGCPMeasurementsXML1(qstr2str(filename), srs_map, cps_map, image_map);
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            AI3D::CORE::BlockObject* blockData = new AI3D::CORE::BlockObject();
            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create ob:" << std::hex << std::showbase << blockData << std::dec;
                LOGI(oss.str());
            }
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITBlock:
            {
                //删除
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            }
            default:break;
            }
            auto atdata = blockData->GetCurrentATMutual();
            std::map<std::string, std::pair<image_t, image_t>> name_ids;
            //导入文件的影像名称 导入的id 
            for (auto& iter : image_map)
            {
                std::string name = File::GetFileNameWithoutExtension(iter.second);
                AI3D::CORE::String::StringToLower(&name);
                name_ids[name].first = iter.first;
                name_ids[name].second = -1;
            }
            //按影像名检测，如果block中有则赋值ID，也就是说map中都有id的也就是block中有的
            for (auto& iter : atdata->GetImages())
            {
                auto name1 = File::GetFileNameWithoutExtension(iter.second.GetName());// iter.second.GetPath() + "/" + iter.second.GetName();;// 
                // name1 = AI3D::CORE::File::EnsureUnifySlash(name1);
                AI3D::CORE::String::StringToLower(&name1);

                if (name_ids.count(name1))
                {
                    name_ids.at(name1).second = iter.first;
                    //newimage_map[iter.first] = iter.second.GetPath() + "/" + iter.second.GetName();
                }


            }
            //影像id统一到atdata下
            for (auto& iter : cps_map)
            {
                /*if (iter.second.GetName() == "07350P3-01-002-017")
                {
                    std::cout << iter.second.GetId() << std::endl;
                }*/
                std::vector<AI3D::CORE::TrackElement>& elevec = iter.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual();
                for (std::vector<AI3D::CORE::TrackElement>::iterator it = elevec.begin();
                    it != elevec.end();)
                {
                    auto& image_id = (*it).image_id;

                    std::string imgname = image_map.at(image_id);

                    imgname = File::GetFileNameWithoutExtension(imgname);
                    AI3D::CORE::String::StringToLower(&imgname);
                    if (name_ids.count(imgname) && name_ids.at(imgname).first != -1 && name_ids.at(imgname).second != -1)
                    {

                        image_id = name_ids.at(imgname).second;

                        it++;
                    }
                    else
                    {
                        it = elevec.erase(it);
                    }
                }


            }

            AI3D::CORE::ControlPoints gcps_import;
            gcps_import.SetPoints(cps_map);
            if (cps_map.empty())
                return;
            srs_s basesrs = cps_map.begin()->second.GetSrsMutual();

          //  ImportGCP(gcps_import, basesrs, image_map);
        }

        //?chy @zhaokang 应该是需要加入是否要导入tiepoint的逻辑，目前是否需要分开存储，进展如何
        //tiepoint：tiepoint.bin
        void MohackerWin::Slot_Action_ExportToXml()
        {           
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            AI3D::CORE::BlockObject* blockData = new AI3D::CORE::BlockObject();
            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create ob:" << std::hex << std::showbase << blockData << std::dec;
                LOGI(oss.str());
            }
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITBlock:
            {
                //删除
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            }
            default:break;
            }
            
            std::string projectpath = File::GetDirName(blockData->GetPathMutual());
            projectpath = File::GetParentDir(blockData->GetTaskInfo().projectfile_);
            xmldia->SetInitFileName(str2qstr((const_cast<std::string&>(projectpath))), str2qstr(blockData->GetName() + "-export"));
            
///            strfile = AI3D::CORE::File::EnsureUnifySlash(strfile);           
///            xmldia->SetOutFileFullName(strfile.c_str());

            auto srs = blockData->ComputeEnuSRS();
            xmldia->SetEnuId(srs.ID);
            if (srs.type ==  coord_system_type_e::LOCAL)
            {
                //设置坐标系选取下拉列表置灰,填充NOTGEOREFERENCED
                xmldia->InitSrss(QString::fromStdString(NOTGEOREFERENCED));
            }
            else
            {
                xmldia->InitSrss(QString::fromStdString(srs.definition));
            }
           
            xmldia->setWindowModality(Qt::ApplicationModal);
            xmldia->SetTabWidgetIndex(0);
            bool canselecttiepoint = false;
            if (blockData->GetCurrentAT()->HasTiepoints())
            {
                canselecttiepoint = true;
            }
            else if(blockData->GetTaskInfo().statisticinfo_.tiepointnum>0)
            {
                canselecttiepoint = true;
            }
            xmldia->SetCanSelectTiePoint(canselecttiepoint);

            if (xmldia->exec() == QDialog::Accepted)
            {
                               
            }
            else
            {
                return;
            }
        }

        //打开空三转换对话框
        void MohackerWin::Slot_Action_ExportATToColmap()
        {
      
            atdia->SetTabWidgetIndex(0);
            if (atdia->exec() == QDialog::Accepted)
            {

            }
            else
            {
                return;
            }
        }

        //打开重建转换对话框
        void MohackerWin::Slot_Action_ExportRecToColmap()
        {
            
            recdia->SetTabWidgetIndex(0);
            if (recdia->exec() == QDialog::Accepted)
            {

            }
            else
            {
                return;
            }
        }

        //执行空三转换
        void MohackerWin::TransferATColmap() {
            qDebug() << "call TransferATColmap ==========" << endl;

            std::string inputfilename = qstr2str(atdia->GetInFileFullName());
            qDebug() << "inputfilename :" << atdia->GetInFileFullName() << endl;
            std::string outputfilename = qstr2str(atdia->GetOutFileFullName());
            qDebug() << "outputfilename :" << atdia->GetOutFileFullName() << endl;

#if 1
            /*  bool flag = false;
              while (!flag)*/
            {
                atdia->ShowProgress();
                //blockData->ExportATXML(strfile, opt);
            }
#endif

            /*  blockData->ExportATXML(strfile, opt);
              SaveXmlFinished();*/

            //QThread::sleep(2);
            std::string input_path = inputfilename;
            std::string outimgput_path = outputfilename;
            std::string outputsfm_path1 = outputfilename;
           /* ToColmapForGS(input_path, outimgput_path, outputsfm_path1);
            TransferATColmapFinished();*/

            auto transferColmap = [=](std::string input_path, std::string outimgput_path, std::string outputsfm_path1) {  return ToColmapForGS(input_path, outimgput_path, outputsfm_path1); };//20220224ly改为引用会出bug?
            QFuture<int> f2 = QtConcurrent::run(transferColmap, input_path, outimgput_path, outputsfm_path1);
            watcher.setFuture(f2);

            connect(&watcher,
                &QFutureWatcher<int>::finished,
                [&]() {  //捕获列表：[&]，表示捕获当前作用域中的所有变量的引用。
                    // 任务完成时的槽函数
                    int result = watcher.result(); // 获取任务的结果
                    qDebug() << "resulte :" << result << endl;
                    TransferATColmapFinished(result);
                });

            //connect(&watcher, SIGNAL(finished()), this, SLOT(TransferATColmapFinished(watcher.result())));
        }

        //执行重建转换
        void MohackerWin::TransferRecColmap() {
            qDebug() << "call TransferRecColmap ==========" << endl;

            std::string inputfilename = qstr2str(recdia->GetInFileFullName());
            qDebug() << "inputfilename :" << recdia->GetInFileFullName() << endl;
            std::string outputfilename = qstr2str(recdia->GetOutFileFullName());
            qDebug() << "outputfilename :" << recdia->GetOutFileFullName() << endl;

            std::string input_path = inputfilename;
            std::string output_path = outputfilename;

#if 1
            /*  bool flag = false;
              while (!flag)*/
            {
                recdia->ShowProgress();
                //blockData->ExportATXML(strfile, opt);
            }
#endif

            /*  blockData->ExportATXML(strfile, opt);
              SaveXmlFinished();*/

           /* QThread::sleep(2);
            TransferRecColmapFinished();*/
            auto transferColmap = [=](std::string input_path, std::string output_path) {  return RunParsePointcloudVpc(input_path, output_path); };//20220224ly改为引用会出bug?
            QFuture<int> f2 = QtConcurrent::run(transferColmap, input_path, output_path);
            watcher.setFuture(f2);

            connect(&watcher,
                &QFutureWatcher<int>::finished,
                [&]() {  //捕获列表：[&]，表示捕获当前作用域中的所有变量的引用。
                    // 任务完成时的槽函数
                    int result = watcher.result(); // 获取任务的结果
                    qDebug() << "resulte :" << result << endl;
                    TransferRecColmapFinished(result);
                });
            //connect(&watcher, SIGNAL(finished()), this, SLOT(TransferRecColmapFinished(watcher.result())));
        }

        void MohackerWin::SaveXmlFile()
        {
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            AI3D::CORE::BlockObject* blockData = new AI3D::CORE::BlockObject();
            if (BlockObject::supportTempLogs())
            {
                std::ostringstream oss;
                oss << "create ob:" << std::hex << std::showbase << blockData << std::dec;
                LOGI(oss.str());
            }
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITBlock:
            {
                //删除
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            }
            default:
                break;
            }

            ///std::string strfile = blockData->GetPath() + "/" + blockData->GetName() + "-export.xml";
            //std::string initExportPath = 
            ///xmldia->SetInitFileName(str2qstr((const_cast<std::string&>(blockData->GetPath()))), str2qstr(blockData->GetName() + "-export"));
            //xmldia->SetOutFileFullName(strfile.c_str());

            ///std::string finalfilename = xmldia->GetOutFileFullName().toStdString();
            std::string finalfilename = qstr2str(xmldia->GetOutFileFullName());

            AI3D::CORE::BlockObject::BlockExportOptions opt;
            //导出xml,根据参数判断是否导出tiepoint
            opt.export_tiepoint_ = xmldia->isSelectTiePoint();

            if (opt.export_tiepoint_ && !blockData->GetTiepointStatus())
            {
                blockData->LoadTiepointsBinary(blockData->GetTaskInfo().Tiepoints,blockData->GetCurrentATMutual());
            }

            opt.export_controlpoint_ = true;
            opt.export_not_registered_ = true;
            opt.srs_ = xmldia->GetSelectSrs_s();

#if 1
          /*  bool flag = false;
            while (!flag)*/
            {
               xmldia->ShowProgress();
               //blockData->ExportATXML(strfile, opt);
            }
#endif

          /*  blockData->ExportATXML(strfile, opt);
            SaveXmlFinished();*/
           
           auto savexml = [=](std::string strfile, AI3D::CORE::BlockObject::BlockExportOptions opt){  return blockData->ExportATXML(strfile, opt); };//20220224ly改为引用会出bug?
           QFuture<int> f2 = QtConcurrent::run(savexml, finalfilename, opt);
           watcher.setFuture(f2);
         
          
           connect(&watcher, SIGNAL(finished()), this, SLOT(SaveXmlFinished()));         
           
        }

        void MohackerWin::TransferATColmapFinished(int result)
        {
            atdia->FinhshWriteXml(result);
        }

        void MohackerWin::TransferRecColmapFinished(int result)
        {
            recdia->FinhshWriteXml(result);
        }

        void MohackerWin::SaveXmlFinished()
        {
            xmldia->FinhshWriteXml();
        }

        void MohackerWin::Set3DViewProgressValue(int num, QString str)
        {
            m_pProgressBar->show();
            m_pProgressBar->setTitleVisble(true);
            m_pProgressBar->setValue(num);
            
            //m_pProgressBar->setFormat(str);

            if (num == 100)
            {
                m_pProgressBar->hide();
                m_pProgressBar->setTitleVisble(false);
            }
                
        }

        void MohackerWin::FinishImportXML(QString file, bool PasrseSucccess)
        {
           
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (PasrseSucccess)
            {
                promanager->SetProejctModified(true);                                
            }
            else
            {
                Message_Box::warning(nullptr, tr("Warning"), tr("Parse block file failed!"));

                LOGE(IMPORTBLOCK + qstr2str(file) + "Parse block file failed!");

                m_pProgressBar->hide();
                return;
            }
            LOGI(IMPORTBLOCK + qstr2str(file));
           
            AI3D::CORE::BlockObject* block;
            block = promanager->GetProject()->GetCurrentBlock();
           
            promanager->AddBlockManager(block);

          
            //统计处理数据量
            for (const auto& img : block->GetCurrentAT()->GetImages())
            {
                std::string imgpath = img.second.GetPath();
                AI3D::CORE::String::StringToLower(&imgpath);
                std::string parentdir = AI3D::CORE::File::EnsureUnifySlash(imgpath);
                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->PhotosOfDir[parentdir]++;

            }
            std::string msg = "import xml " + file.QString::toStdString();
            LOGI(msg);
            for (auto& iter : MasterInfo::Getinstance().GetAPPUseInfosMutual())
            {
                auto count = iter.PhotosOfDir.size();
                std::string msg = std::to_string(count);
                LOGI(msg);
            }

            QStandardItem* item_ = GetBlockATData(block);
            ShowBlockWidget(block, item_, true);
          
            item_->setData(2, CustomRole::CanSaveBlock);
            SetFileModifiedProj();
            m_pProgressBar->hide();
           
        }       

        void MohackerWin::FinishImportXML2(QString file, bool PasrseSucccess)
        {

            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (PasrseSucccess)
            {

                //promanager->GetProject()->Save(savetype_e::PROJECT_SAVED);

                {
                    promanager->SetProejctModified(true);
                    //设置保存按钮为可点击
                    //ui_action_save->setEnabled(true);
                    //仿CC 窗口标题+*
                    //this->setWindowTitle(QString("Mohacker" + QString(w.GetProject()->GetName().c_str()) +"*"));
                }

            }
            else
            {
                Message_Box::warning(nullptr, tr("Warning"), tr("Parse block file failed!"));

                ///LOGE(IMPORTBLOCK + file.toStdString() + "Parse block file failed!");

                LOGE(IMPORTBLOCK + qstr2str(file) + "Parse block file failed!");

                m_pProgressBar->hide();  
                return;
            }
            LOGI(IMPORTBLOCK + qstr2str(file));
           
            AI3D::CORE::BlockObject* block;
               
            block = promanager->GetProject()->GetBlockByImportFilename(qstr2str(file));
        
           
            {
                bool tiepointstatus = block->GetTiepointFullStatus();;


                if (block->GetCurrentATMutual()->GetNumRegImages() > 2 && tiepointstatus)
                {
                    block->SetStatus(jobsta_e::STATUS_COMPLETE);
                }
                else
                {
                    //chy 导入的xml有可能是已经在外边做完空三的了；这个时候状态应该是complete;
                    block->SetStatus(jobsta_e::STATUS_NEW);
                }
            }
            promanager->AddBlockManager(block);

            //导入xml生成缩略图

            //auto addimage_tmp = [&]() { return block->GetCurrentAT()->GenPreviewImages(block->GetPath()); };
   //         QFuture<void> future_tmp = QtConcurrent::run(addimage_tmp);
   //         future_tmp.waitForFinished();
            // m_pProgressBar->setValue(0);
            //QApplication::processEvents();
            //统计处理数据量
            for (const auto& img : block->GetCurrentAT()->GetImages())
            {
                std::string imgpath = img.second.GetPath();
                std::string parentdir = AI3D::CORE::File::EnsureUnifySlash(imgpath);
                MasterInfo::Getinstance().GetAPPUseInfosMutual().rbegin()->PhotosOfDir[parentdir]++;
            }
            QStandardItem* item_ = GetBlockATData(block);
            ShowBlockWidget(block, item_, true);
            /* block->GetTaskInfoMutual().isLoaded = true;*/
            item_->setData(2, CustomRole::CanSaveBlock);
            SetFileModifiedProj();
            m_pProgressBar->hide();

        }

        void MohackerWin::SetWindowTitle()
        {
            /// note:check it later.@240321
#if 1
            if (!currentVersionName.isEmpty())
                setWindowTitle(QString(QObject::tr("%1 %2")).arg("MoldAI").arg(currentVersionName));
            else
                setWindowTitle(QString(QObject::tr("%1 %2")).arg("MoldAI").arg(VERSION.c_str()));
#endif
        }
        void MohackerWin::Slot_SettingsChanged()
        {
            SetWindowTitle();
        }

        void MohackerWin::Slot_Action_Settings()
        {
              
                SettingsWgt* pSettingsWindow = new SettingsWgt(nullptr);
               
                connect(pSettingsWindow, &SettingsWgt::SettingsChanged, this, &MohackerWin::Slot_SettingsChanged);

                pSettingsWindow->setWindowModality(Qt::ApplicationModal);
                pSettingsWindow->setAttribute(Qt::WA_DeleteOnClose);
         //       pSettingsWindow->resize(480, 230);
                pSettingsWindow->resize(600, 290);
                pSettingsWindow->show();
        }

        void MohackerWin::Slot_Action_ViewEngineNode()
        {          
            OpenEngineNodeView();
        }

        void MohackerWin::Slot_Action_DataPreProcessRight()
        {
            std::cout << "right clicked one block item of the project tree,start one data preprocess for related block object." << std::endl;
            std::cout << "dpp sender:" << qstr2str(sender()->objectName()) << std::endl;

            AI3D::CORE::BlockObject* newblock = nullptr;
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>())
            {
                case ItemType::ITProject:
                {

                }
                break;
                case ItemType::ITBlock:
                {
                    AI3D::CORE::BlockObject* block_data_ = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();

                    block_t id = block_data_->GetId();

///                    project_ptr->CloneBlock(id, false);
                    LOGI("Clone Block Finished!");

                    newblock = project_ptr->GetBlock(id);

                }
            }

            DataPreProcess* pDataPreProcess = new DataPreProcess(nullptr,newblock);

            connect(pDataPreProcess, &DataPreProcess::generatedXml, this, &MohackerWin::Slot_GeneratedXml);
            connect(pDataPreProcess, &DataPreProcess::generatedXmls, this, &MohackerWin::Slot_GeneratedXmls);

            pDataPreProcess->setWindowModality(Qt::ApplicationModal);
            pDataPreProcess->resize(1300, 700);
            pDataPreProcess->show();
        }

        void MohackerWin::Slot_GeneratedXml(std::string xml)
        {
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() == "")
            {
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a block first"));
                return;
            }

            std::cout << "process xml:" << xml << std::endl;


///            importxml_->SetFileName(str2qstr(xml));
///            importxml_->start();

            ImportXml* importxml2_ = new ImportXml;
            connect(importxml2_, &ImportXml::FinishedRead, this, &MohackerWin::FinishImportXML2);

            importxml2_->SetFileName(str2qstr(xml));
            importxml2_->run();
        }

        void MohackerWin::Slot_GeneratedXmls(int hasDoneNum)
        {
            if (hasDoneNum <= 0)
                return;

            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() == "")
            {
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a block first"));
                return;
            }

            Slot_Action_SaveProject();
        }

        void MohackerWin::Slot_Action_DataPreProcess()
        {
            // note!!: set boolean flag to be zero in release version,for being one just for test purpose.
            if (bSupportDataPreprocess4TestPurpose)
            {
                std::cout << "inside data pre process action." << std::endl;
                TestProjSRS();
                return;
            }
            
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager->GetProject()->GetName() == "")
            {
                Message_Box::warning(nullptr, tr("warning"), tr("Please new/open a block first"));
                return;
            }

            std::cout << "has clicked menu item for data preprocess." << std::endl;
            std::cout << "sender:" << qstr2str(sender()->objectName()) << std::endl;
            DataPreProcess* pDataPreProcess = new DataPreProcess(nullptr);

            connect(pDataPreProcess, &DataPreProcess::generatedXml, this, &MohackerWin::Slot_GeneratedXml);
            connect(pDataPreProcess, &DataPreProcess::generatedXmls, this, &MohackerWin::Slot_GeneratedXmls);

            pDataPreProcess->setWindowModality(Qt::ApplicationModal);
            pDataPreProcess->resize(1300, 700);
            pDataPreProcess->show();
        }

        void MohackerWin::Slot_Action_RenameProject()
        {
        
        }

        void MohackerWin::Slot_Action_RenameBlock()
        {
            
            //defferentiate the item project or block or...by ItemType
            AI3D::CORE::BlockObject* blockData = nullptr;
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITProject:
            {
                ui_treeView_project->edit(currentIndex);
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                LOGI(RENAMEPROJECT);
            }
            break;
            case ItemType::ITBlock:
            {
                ui_treeView_project->edit(currentIndex);          
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                ///ProjectManager* managet = ProjectManager::GetInstance();          
            }
            break;
            case ItemType::ITBlockAT:
            {
                blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
            }
            break;
            case ItemType::ITReconstruction:
            {
                ui_treeView_project->edit(currentIndex);
                blockData = currentIndex.data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();
            }
            break;
            case ItemType::ITProduction:
            {
///                ui_treeView_project->edit(currentIndex);
                blockData = currentIndex.data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();
            }
            break;
            default:
                break;
            }

            if (blockData)
            {
                blockData->GetTaskInfoMutual().isSaved = false;
                blockData->GetTaskInfoMutual().savetype_ = SaveType_e::PROJECT_SAVED;
                ui_action_save->setEnabled(true);
            }
        }
        //删除Block首先是要cancle掉
        void MohackerWin::Slot_Action_DeleteBlock2()
        {           
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITProject:
            {

            }
            break;
            case ItemType::ITBlock:
            {
                if (QMessageBox::No == Message_Box::question(this, "delete", "Are you sure to delete the current block!", Message_Box_Type::Question_Yes_No))
                {
                    return;
                }
               
                //删除
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                std::string name = blockData->GetName();
                //Pending状态下则移动job到cancel文件夹
                if (blockData->GetStatus() == _job_status_e::STATUS_PENDDING)
                {
                    std::string jobname = blockData->GetTaskInfo().job_;
                    std::string postFix = "";
                    if (JOB_INFO_USE_BIN) {
                        postFix = BINFILE_POSTFIX;
                    }
                    else {
                        postFix = JSONFILE_POSTFIX;
                    }
                    std::string pendingjobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + jobname + SC_POSTFIX + postFix;
                    std::string canclejobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Cancelled" + PATH_SEPARATOR_STR + jobname + SC_POSTFIX + postFix;

                    try
                    {
                        const std::filesystem::path pendingPath = AI3D::CORE::File::BoostPathFromUtf8(pendingjobpath);
                        const std::filesystem::path cancelPath = AI3D::CORE::File::BoostPathFromUtf8(canclejobpath);
                        if (std::filesystem::is_regular_file(pendingPath))
                        {
                            LOGI("saved to job cancelled dir inside MohackerWin");
                            std::filesystem::copy_file(pendingPath, cancelPath, std::filesystem::copy_options::overwrite_existing);
                            std::filesystem::remove(pendingPath);
                        }
                    }
                    catch (const std::filesystem::filesystem_error& fse)
                    {
                        std::ostringstream oss;
                        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                        LOGI(oss.str());
                    }
                    catch (std::exception& ex)
                    {
                        std::ostringstream oss;
                        oss << "exception:" << ex.what();
                        LOGI(oss.str());
                    }
                }
                blockData->GetTaskInfoMutual().isCancleOrDelete = 1;
                //?chy@zhangyunfen 找到另外实现的部分 暂且不支持running状态下删除block，待后期放开
                if (blockData->GetStatus() == _job_status_e::STATUS_RUNNING )
                {
                        //执行取消
                        QString str1;
                        int errornum;
                        QString jobname = QString(blockData->GetTaskInfo().job_.c_str()).split(PATH_SEPARATOR_STR).last();
                       //chy 0901需要看一下原来的代码是怎么处理等待的

///                        bool flag = doCancelJob2(blockData->GetPath(), jobname.toStdString(), errornum);
                        bool flag = doCancelJob2(blockData->GetPath(), qstr2str(jobname), errornum);
                        if (!flag)
                        {
                            LOGW("Please wait engine cancle operation!");
                        }

                    //设置标志位
                   
                }

                //add by chy 20230801因为重建的加入，需要考虑当block为complete状态时的删除功能，
                //DeleteBlock 已经删除了数据，所以这里只需要删除job和文件夹
                
                //获取要终止的job
                std::vector<std::pair<std::string, std::string> > jobs_to_delete;
                
              //  int errornum;
                if (blockData->GetStatus() == _job_status_e::STATUS_COMPLETE)
                {
                    //执行取消
                    
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集
                    
                    ReconstructionCommandSet::GetJobsToCancelled(*blockData,jobs_to_delete);
                   
                   
                }

                ProjectManager* manager = ProjectManager::GetInstance();
               

                {
                    bool bSaveFinished = false;

                    auto savefunc = [&, this](ProjectManager* manager)
                    {
                        LOGI("delete block...");

                        doCancelJobs(jobs_to_delete);
                        manager->GetProject()->DeleteBlock(blockData->GetId());
                        manager->DeleteBlockManager(blockData->GetId());

                        LOGI("delete block finished.");
                        bSaveFinished = true;
                        return ;
                    };
                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {
                        LOGI("OpenLoadingPrompt:Saving project now, pls wait for a while...");
                        if (AI3D::CORE::BlockObject::isChineseVersion()) {
                            OpenLoadingPromptV4("正在保存工程，请稍等");
                        }
                        else {
                            OpenLoadingPromptV4("Saving project now,pls wait for a while");
                        }

                        QFuture<void> f1 = QtConcurrent::run(savefunc, manager);

                        while (!bSaveFinished)
                        {
                            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }

                        CloseLoadingPromptV4();
                        LOGI("Closed Loading Prompt here.");
                    }
                    else
                    {
                        savefunc(manager);
                    }
                }


                QWidget* currentWgt = currentIndex.data(CustomRole::CRBlockWgt).value<QWidget*>();
                BlockWgt* blockWgt = currentIndex.data(CustomRole::CRBlockWgt).value<BlockWgt*>();



                //zk 2022年4月8日20:34:14 删除block需要调用引擎的操作.
                if (currentWgt)
                {

                    ui_stackedWidget->removeWidget(currentWgt);
                    BlockWgt* blockWgt2 = dynamic_cast<BlockWgt*>(currentWgt);
                    

                    if (blockWgt)
                    {
                        //std::cout << "clear the scene data first before destroying related parent container widget." << std::endl;
                        ///blockWgt->viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                        blockWgt->viewWidget_ui->mWindow->clearSceneData();
                    }
                    else if (blockWgt2)
                    {
                        //std::cout << "clear2 the scene data first before destroying related parent container widget." << std::endl;
                        //blockWgt2->viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                        blockWgt2->viewWidget_ui->mWindow->clearSceneData();
                    }
                    else
                    {
                        //std::cout << "can't clear the scene data first before destroying related parent container widget." << std::endl;
                    }

                    delete currentWgt;
                    currentWgt = nullptr;
                }

                // alka 1.4.231 maptosource itemindex
                QModelIndex currentIndex_ = _proxy_->mapToSource(currentIndex);
                // alke

                QStandardItem* item = _itemmodel_->itemFromIndex(currentIndex_);

                // delete reconstruction/production widget from stackedWidget under current block item.
                RemoveWidgetsUnderCurrentBlock(item);

                int rowCount = item->rowCount();
                for (int row = rowCount - 1; row >= 0; row--)
                {
                    item->removeRow(row);
                }

                QStandardItem* parentItem = item->parent();
                parentItem->removeRow(currentIndex_.row());
                
                //定位到根节点
                QModelIndex rootIndex1 = ui_treeView_project->rootIndex();
                QModelIndex selindex = _itemmodel_->index(0, 0, rootIndex1);
                QModelIndex rootIndex = selindex;// currentIndex_.parent();               
              
                if (_proxy_) {
           ///         QModelIndex setIndex = _proxy_->mapFromSource(rootIndex);
                   
                }
                
                
                    ProjectInfoWgt* projectWgt = dynamic_cast<ProjectInfoWgt*>(_itemmodel_->data(rootIndex, CustomRole::CRProjectWgt).value<QWidget*>());
                    if (projectWgt)
                    {
                       //ui_stackedWidget->setCurrentWidget(projectWgt);//如果加这一行则会显示info界面
                        projectWgt->Update_Block_Info();
                    }
              /*  ui_treeView_project->setCurrentIndex(rootIndex);*/
                LOGI(DELETEBLOCK + name);
            }
            break;
            case ItemType::ITBlockAT:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current block!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();



            }
            break;

            case ItemType::ITReconstruction:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current reconstruction!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }

                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();

                ConstructionWgt* constructWgt = currentIndex.data(CustomRole::CRBlockData).value<ConstructionWgt*>();
                if (constructWgt)
                {

                    AI3D::CORE::ReconstructionObject* recons_object = constructWgt->getReconstructionObject();

                    //执行取消
                    QString str1;
                   
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集
                    std::vector<std::pair<std::string, std::string> > jobs_to_delete;
                    ReconstructionCommandSet::GetJobsToCancelled(*recons_object, jobs_to_delete);
                    doCancelJobs(jobs_to_delete);

                    int ret = ReconstructionCommandSet::DeleteReconstruction(blockData, recons_object->GetId());
                    if (ret != AI3D_SUCCESS)
                    {
                        return;
                    }

                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();
                    project_ptr->Save(savetype_e::XML_SAVED);
                    QModelIndex currentIndex_ = _proxy_->mapToSource(currentIndex);
                    QStandardItem* item = _itemmodel_->itemFromIndex(currentIndex_);
                    RemoveWidgetsUnderCurrentReconstruction(item);
                    //RemoveWidgetsUnderCurrentBlock(item);

                    int rowCount = item->rowCount();
                    for (int row = rowCount - 1; row >= 0; row--)
                    {
                        item->removeRow(row);
                    }

                    QStandardItem* parentItem = item->parent(); // get block item.
                    if (parentItem != nullptr)
                    {
                        //                        std::cout << __FILE__ << " " << __LINE__ << std::endl;
                        parentItem->removeRow(currentIndex_.row());
                    }

                    ui_stackedWidget->removeWidget(constructWgt);
                    delete constructWgt;
                    constructWgt = nullptr;

                    //设置标志位
                   
                   
                }
            }
            break;

            case ItemType::ITProduction:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current reconstruction!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }
               
                ProductionWgt* productionWgt = currentIndex.data(CustomRole::CRBlockData).value<ProductionWgt*>();
                if (productionWgt)
                {
                    AI3D::CORE::ProductionObject* production_object = productionWgt->getProductionObject();

                    SetProjectDirty(true);

                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    //执行取消
                    QString str1;
                   
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集

                    std::vector<std::pair<std::string, std::string> > jobs_to_delete;
                    ReconstructionCommandSet::GetJobsToCancelled(*production_object, jobs_to_delete);

                    doCancelJobs(jobs_to_delete);
                    QModelIndex currentIndex_ = _proxy_->mapToSource(currentIndex);

                    QStandardItem* item = _itemmodel_->itemFromIndex(currentIndex_);

                    AI3D::CORE::ReconstructionObject* pReconstructionObject = _itemmodel_->data(currentIndex_, CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();
                    AI3D::CORE::BlockObject* blockData = _itemmodel_->data(currentIndex_, CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();

                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    //设置标志位
                    if (blockData != nullptr)
                        blockData->GetTaskInfoMutual().isCancleOrDelete = 1;
                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                    int ret = ReconstructionCommandSet::DeleteProduction(blockData, pReconstructionObject->GetId(), production_object->GetId());
                    if (ret != AI3D_SUCCESS)
                    {
                        return;
                    }
                    if (blockData != nullptr)
                        blockData->GetTaskInfoMutual().isSaved = false;
                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();
                    project_ptr->Save(savetype_e::XML_SAVED);
                    QStandardItem* parentItem = item->parent();
                    if (parentItem != nullptr)
                    {
                        //std::cout << __FILE__ << " " << __LINE__ << std::endl;
                        parentItem->removeRow(currentIndex_.row());
                    }

                    //productionWgt->mWindow->viewerWindow->setSceneData(nullptr);
                    productionWgt->mWindow->clearSceneData();

                    ui_stackedWidget->removeWidget(productionWgt);
                    delete productionWgt;

                    SetProjectDirty(false);

                    //定位到根节点
                    QModelIndex rootIndex1 = ui_treeView_project->rootIndex();
                    QModelIndex selindex = _itemmodel_->index(0, 0, rootIndex1);
                    QModelIndex rootIndex = selindex;// currentIndex_.parent();               

                    ProjectInfoWgt* projectWgt = dynamic_cast<ProjectInfoWgt*>(_itemmodel_->data(rootIndex, CustomRole::CRProjectWgt).value<QWidget*>());
                    if (projectWgt)
                    {
                        //ui_stackedWidget->setCurrentWidget(projectWgt);//如果加这一行则会显示info界面
                        projectWgt->Update_Block_Info();
                    }



                    //std::cout << "inside " << __FILE__ << " " << __LINE__ << std::endl;
                }
            
            }
                break;

            default:
                break;
            }

        }

        void MohackerWin::BeforeDeleteOneBlock(QModelIndex &currentIndex)
        {
///            if (QMessageBox::No == Message_Box::question(this, "delete", "Are you sure to delete the current block!", Message_Box_Type::Question_Yes_No))
///          {
///                return;
///            }
 
            if (currentIndex.data(CustomRole::CRItemType).value<ItemType>() == ItemType::ITBlock)
            {
                //删除
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                std::string name = blockData->GetName();
                //Pending状态下则移动job到cancel文件夹
                if (blockData->GetStatus() == _job_status_e::STATUS_PENDDING)
                {
                    std::string jobname = blockData->GetTaskInfo().job_;
                    std::string postFix = "";
                    if (JOB_INFO_USE_BIN) {
                        postFix = BINFILE_POSTFIX;
                    }
                    else {
                        postFix = JSONFILE_POSTFIX;
                    }
                    std::string pendingjobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Pending" + PATH_SEPARATOR_STR + jobname + SC_POSTFIX + postFix;
                    std::string canclejobpath = qstr2str(Settings::getMasterJobQueue()) + PATH_SEPARATOR_STR + "Cancelled" + PATH_SEPARATOR_STR + jobname + SC_POSTFIX + postFix;

                    try
                    {
                        const std::filesystem::path pendingPath = AI3D::CORE::File::BoostPathFromUtf8(pendingjobpath);
                        const std::filesystem::path cancelPath = AI3D::CORE::File::BoostPathFromUtf8(canclejobpath);
                        if (std::filesystem::is_regular_file(pendingPath))
                        {
                            LOGI("saved to job cancelled dir inside MohackerWin");
                            std::filesystem::copy_file(pendingPath, cancelPath, std::filesystem::copy_options::overwrite_existing);
                            std::filesystem::remove(pendingPath);
                        }
                    }
                    catch (const std::filesystem::filesystem_error& fse)
                    {
                        std::ostringstream oss;
                        oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
                        LOGI(oss.str());
                    }
                    catch (std::exception& ex)
                    {
                        std::ostringstream oss;
                        oss << "exception:" << ex.what();
                        LOGI(oss.str());
                    }
                }

                //?chy@zhangyunfen 找到另外实现的部分 暂且不支持running状态下删除block，待后期放开
                if (blockData->GetStatus() == _job_status_e::STATUS_RUNNING)
                {
                    //执行取消
                    QString str1;
                    int errornum;
                    QString jobname = QString(blockData->GetTaskInfo().job_.c_str()).split(PATH_SEPARATOR_STR).last();
                    //chy 0901需要看一下原来的代码是怎么处理等待的

///                        bool flag = doCancelJob2(blockData->GetPath(), jobname.toStdString(), errornum);
                    bool flag = doCancelJob2(blockData->GetPath(), qstr2str(jobname), errornum);
                    if (!flag)
                    {
                        LOGW("Please wait engine cancle operation!");
                    }

                    //设置标志位
                    blockData->GetTaskInfoMutual().isCancleOrDelete = 1;
                }

                //add by chy 20230801因为重建的加入，需要考虑当block为complete状态时的删除功能，
                //DeleteBlock 已经删除了数据，所以这里只需要删除job和文件夹

                if (blockData->GetStatus() == _job_status_e::STATUS_COMPLETE)
                {
                    //执行取消
                    QString str1;
                    int errornum;
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集由

                    for (auto& iterrec : blockData->GetReconstructions())
                    {
                        for (auto& iterpro : iterrec.second->GetProductions())
                        {
                            for (auto& itertile : iterpro.second->GetTiles())
                            {
                                if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                                    itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                                {
                                    std::string jobstr = itertile.second.jobstr_;
                                    QString jobname = QString(jobstr.c_str()).split(PATH_SEPARATOR_STR).last();
                                    //feedback path
                                    std::string feedbackpath = iterpro.second->GetPath() + "/" + itertile.second.name_ + "/";
                                    feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                                    bool flag = doCancelJob2(feedbackpath, qstr2str(jobname), errornum);
                                    if (!flag)
                                    {
                                        LOGW("Please wait engine cancle operation!");
                                    }
                                }
                            }
                        }
                    }
                    //设置标志位
                    blockData->GetTaskInfoMutual().isCancleOrDelete = 1;
                }

            }
            else if (currentIndex.data(CustomRole::CRItemType).value<ItemType>() == ItemType::ITBlockAT)
            {


            }
            else
            {
                // ignore.
                return;
            }

///            QStandardItem* parentItem;/// = item->parent();

#if 0
            QModelIndex currentIndex = ui_treeView_project->currentIndex();
            switch (currentIndex.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITProject:
            {

            }
            break;
            case ItemType::ITBlock:
            {
                //删除
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                std::string name = blockData->GetName();
                //Pending状态下则移动job到cancel文件夹

                ProjectManager* managet = ProjectManager::GetInstance();
                managet->GetProject()->DeleteBlock(blockData->GetId());
                managet->DeleteBlockManager(blockData->GetId());

                QWidget* currentWgt = currentIndex.data(CustomRole::CRBlockWgt).value<QWidget*>();
                //zk 2022年4月8日20:34:14 删除block需要调用引擎的操作.
                if (currentWgt)
                {
                    ui_stackedWidget->removeWidget(currentWgt);
                    delete currentWgt;
                    currentWgt = nullptr;
                }

                // alka 1.4.231 maptosource itemindex
                QModelIndex currentIndex_ = _proxy_->mapToSource(currentIndex);
                // alke

                QStandardItem* item = _itemmodel_->itemFromIndex(currentIndex_);

                // delete reconstruction/production widget from stackedWidget under current block item.
                RemoveWidgetsUnderCurrentBlock(item);

                int rowCount = item->rowCount();
                for (int row = 0; row < rowCount; ++row)
                {
                    item->removeRow(row);
                }

                QStandardItem* parentItem = item->parent();
                parentItem->removeRow(currentIndex_.row());


                //定位到根节点
                QModelIndex rootIndex1 = ui_treeView_project->rootIndex();
                QModelIndex selindex = _itemmodel_->index(0, 0, rootIndex1);
                QModelIndex rootIndex = selindex;// currentIndex_.parent();               

                if (_proxy_) {
                    ///         QModelIndex setIndex = _proxy_->mapFromSource(rootIndex);

                }

                ProjectInfoWgt* projectWgt = dynamic_cast<ProjectInfoWgt*>(_itemmodel_->data(rootIndex, CustomRole::CRProjectWgt).value<QWidget*>());
                if (projectWgt)
                {
                    //ui_stackedWidget->setCurrentWidget(projectWgt);//如果加这一行则会显示info界面
                    projectWgt->Update_Block_Info();
                }
                /*  ui_treeView_project->setCurrentIndex(rootIndex);*/
                LOGI(DELETEBLOCK + name);
            }
            break;
            case ItemType::ITBlockAT:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current block!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();



            }
            break;

            case ItemType::ITReconstruction:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current reconstruction!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }
                AI3D::CORE::BlockObject* blockData = currentIndex.data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();


                ConstructionWgt* constructWgt = currentIndex.data(CustomRole::CRBlockData).value<ConstructionWgt*>();
                if (constructWgt)
                {

                    AI3D::CORE::ReconstructionObject* recons_object = constructWgt->getReconstructionObject();

                    //执行取消
                    QString str1;
                    int errornum;
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集由

                    for (auto& iterrec : blockData->GetReconstructions())
                    {
                        for (auto& iterpro : iterrec.second->GetProductions())
                        {
                            for (auto& itertile : iterpro.second->GetTiles())
                            {
                                if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                                    itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                                {
                                    std::string jobstr = itertile.second.jobstr_;
                                    QString jobname = QString(jobstr.c_str()).split(PATH_SEPARATOR_STR).last();
                                    //feedback path
                                    std::string feedbackpath = iterpro.second->GetPath() + "/" + itertile.second.name_ + "/";
                                    feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                                    bool flag = doCancelJob2(feedbackpath, qstr2str(jobname), errornum);
                                    if (!flag)
                                    {
                                        LOGW("Please wait engine cancle operation!");
                                    }
                                }
                            }
                        }
                    }
                    //设置标志位
                    blockData->GetTaskInfoMutual().isCancleOrDelete = 1;


                    blockData->DeleteReconstruction(recons_object->GetId());
                    blockData->GetTaskInfoMutual().isSaved = false;
                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();
                    project_ptr->SaveJson(project_ptr->GetPath(), savetype_e::PROJECT_SAVED);
                }
            }
            break;

            case ItemType::ITProduction:
            {
                if (QMessageBox::Cancel == QMessageBox::question(this, "delete", "Are you sure to delete the current reconstruction!", QMessageBox::Ok, QMessageBox::Cancel))
                {
                    return;
                }

                ProductionWgt* productionWgt = currentIndex.data(CustomRole::CRBlockData).value<ProductionWgt*>();
                if (productionWgt)
                {
                    AI3D::CORE::ProductionObject* production_object = productionWgt->getProductionObject();


                    //执行取消
                    QString str1;
                    int errornum;
                    //add by chy先获取 待cancled的项，也就是该block下所有pending 和running的任务，是否需要将doCancelJob2 改成处理job集由

                    for (auto& itertile : production_object->GetTiles())
                    {
                        if (itertile.second.status_ == jobsta_e::STATUS_RUNNING ||
                            itertile.second.status_ == jobsta_e::STATUS_PENDDING)
                        {
                            std::string jobstr = itertile.second.jobstr_;
                            QString jobname = QString(jobstr.c_str()).split(PATH_SEPARATOR_STR).last();

                            std::string feedbackpath = production_object->GetPath() + "/" + itertile.second.name_ + "/";
                            feedbackpath = AI3D::CORE::File::EnsureUnifySlash(feedbackpath);
                            bool flag = doCancelJob2(feedbackpath, qstr2str(jobname), errornum);
                            if (!flag)
                            {
                                LOGW("Please wait engine cancle operation!");
                            }
                        }
                    }

                    AI3D::CORE::ReconstructionObject* pReconstructionObject = _itemmodel_->data(currentIndex, CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();
                    AI3D::CORE::BlockObject* blockData = _itemmodel_->data(currentIndex, CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();

                    //设置标志位
                    blockData->GetTaskInfoMutual().isCancleOrDelete = 1;


                    pReconstructionObject->DeleteProduction(production_object->GetId());
                    blockData->GetTaskInfoMutual().isSaved = false;
                    ProjectManager* manager = ProjectManager::GetInstance();
                    auto project_ptr = manager->GetProject();
                    project_ptr->SaveJson(project_ptr->GetPath(), savetype_e::PROJECT_SAVED);
                }

            }
            break;

            default:
                break;
            }
#endif
        }

        void MohackerWin::DeleteOneBlockWidget(QWidget* currentWgt)
        {
            if (!currentWgt)
                return;

            if (currentWgt)
            {
                ui_stackedWidget->removeWidget(currentWgt);
                delete currentWgt;
                currentWgt = nullptr;
            }
        }

        void MohackerWin::DeleteOneBlockData(AI3D::CORE::BlockObject* blockData)
        {
            if (!blockData)
                return;

            std::string name = blockData->GetName();

            ProjectManager* managet = ProjectManager::GetInstance();
            managet->GetProject()->DeleteBlock(blockData->GetId());
            managet->DeleteBlockManager(blockData->GetId());
        }

        void MohackerWin::DeleteOneBlock(QModelIndex& currentIndex)
        {


        }

        void MohackerWin::Slot_Action_OpenFolder()
        {
            ProjectManager* managet = ProjectManager::GetInstance();
            QModelIndex index = ui_treeView_project->currentIndex();
            QString path;
            switch (index.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITProject:
            {
                path = str2qstr(managet->GetProject()->GetPath());
                
            }
            break;
            case ItemType::ITBlock:
            {
                AI3D::CORE::BlockObject* blockData = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                std::string projectpath =  AI3D::CORE::File::EnsureTrailingSlash(AI3D::CORE::File::EnsureUnifySlash(managet->GetProject()->GetPath()));             
                path = str2qstr(projectpath) + "Block_" + QString::number((blockData->GetId()));               
            }
            break;
            case ItemType::ITBlockAT:
                break;

            case ItemType::ITReconstruction:
            {
                // open reconstruction path.
             ///   path = QString(QString::fromLocal8Bit(managet->GetProject()->GetPath().c_str()));
                ConstructionWgt* constructWgt = index.data(CustomRole::CRBlockData).value<ConstructionWgt*>();
                if (constructWgt)
                {
                    //AI3D::CORE::ReconstructionObject *recons_object = constructWgt->get
                    AI3D::CORE::ReconstructionObject* recons_object = constructWgt->getReconstructionObject();
                    path = str2qstr(recons_object->GetPath());
                }

                break;
            }
            case ItemType::ITProduction:
            {
                // open production path.
//                path = QString(QString::fromLocal8Bit(managet->GetProject()->GetPath().c_str()));
                ProductionWgt* productionWgt = index.data(CustomRole::CRBlockData).value<ProductionWgt*>();
                if (productionWgt)
                {
                    AI3D::CORE::ProductionObject* production_object = productionWgt->getProductionObject();
                    
                   
                    path =QString::fromStdString( production_object->GetOptions().destination_);
                }

                break;
            }
            default:
                break;
            }

            if (!path.isEmpty())
            {
                path.replace("/", "\\");//将地址中的"/"替换为"\"，因为在Windows下使用的是"\".
                QProcess::startDetached("explorer", QStringList() << path);
                ///LOGI(OPENFOLDER + path.toStdString());
            }
        }
        void MohackerWin::Slot_Action_Merge_And_Ajust_blocks()
        {
            std::set<block_t>& blockids = GetMultiSelectedBlocks();
           
            //project
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            promanager->GetProject()->MergeAndAdjustBlocks(blockids);
           /* qApp->processEvents();
            auto mergeblocks = [&]() {return promanager->GetProject()->MergeAndAdjustBlocks(blockids); };
            QFuture<bool> future = QtConcurrent::run(mergeblocks);
            future.waitForFinished();*/

            block_t MergeBlockId = *promanager->GetProject()->GetBlockIds().rbegin();
            AI3D::CORE::BlockObject* MergeBlock = promanager->GetProject()->GetBlock(MergeBlockId);

            LOGI("Merging Blocks");
            promanager->AddBlockManager(MergeBlock);

            //显示到界面上
            QStandardItem* item_ = GetBlockATData(MergeBlock);
            ShowBlockWidget(MergeBlock, item_, true);

            MergeBlock->GetTaskInfoMutual().isSaved = false;
            item_->setData(2, CustomRole::CanSaveBlock);
            SetFileModifiedProj();
            promanager->SetProejctModified(true);
        }

        std::set<block_t> MohackerWin::GetMultiSelectedBlocks()
        {
            QItemSelectionModel* model_selection = ui_treeView_project->selectionModel();

            QModelIndexList IndexList = model_selection->selectedIndexes();
            std::set<block_t> blockids;
            if (IndexList.size() > 1) {
                foreach(QModelIndex index, IndexList)
                {
                    if (!index.isValid()) 
                        return std::set<block_t>();
                    if (index.data(CustomRole::CRItemType).value<ItemType>() != ItemType::ITBlock)
                    {
                        /// check it later for verifying the correctness of related logic,
                        ///      especially not bringing extra error for existing functions or logics.
                        /// may need to modify it for suitting construction and production item.
                        /// some suggestions:just ignore current selected item instead of directly returning.
                        
                        ///return std::set<block_t>();
                    }
                    else
                    {
                        AI3D::CORE::BlockObject* block = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                        blockids.insert(block->GetId());
                    }
                }
            }
            return blockids;
        }


        void MohackerWin::Slot_Action_Merge_blocks()
        {
            
            std::set<block_t> blockids = GetMultiSelectedBlocks();
            //project
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

            qApp->processEvents();
            auto mergeblocks = [&]() {return promanager->GetProject()->MergeBlocks(blockids); };
            QFuture<bool> future = QtConcurrent::run(mergeblocks);
            future.waitForFinished();
            
            block_t MergeBlockId = *promanager->GetProject()->GetBlockIds().rbegin();
            AI3D::CORE::BlockObject* MergeBlock = promanager->GetProject()->GetBlock(MergeBlockId);
      
            LOGI("Merging Blocks");
            promanager->AddBlockManager(MergeBlock);

            //显示到界面上
            QStandardItem* item_ = GetBlockATData(MergeBlock);
            ShowBlockWidget(MergeBlock, item_, true);
      
            MergeBlock->GetTaskInfoMutual().isSaved = false;
            item_->setData(2, CustomRole::CanSaveBlock);
            SetFileModifiedProj();
            promanager->SetProejctModified(true);
        }

        void MohackerWin::Slot_Action_DeleteMore()
        {
            return;
            std::cout << "delete more blocks." << std::endl;
       ///     std::set<block_t> blockids = GetMultiSelectedBlocks();
            // project
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

            QItemSelectionModel* itemSelectionModel = ui_treeView_project->selectionModel();

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (itemSelectionModel != nullptr)
            {
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                QModelIndexList modelIndexList = itemSelectionModel->selectedIndexes();
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                if (modelIndexList.size() > 1)
                {
                    QVector<QWidget*> vecWidgets;
                    QVector<QStandardItem*> vecItems;
                    QVector<AI3D::CORE::BlockObject*> vecBlockDatas;
                    std::set<int> setItemRows;

                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    QStandardItem* parentItem = nullptr;

                    //                    QStandardItem* parentItem = item->parent();
                    //                   parentItem->removeRow(currentIndex_.row());

                    for (QModelIndex index : modelIndexList)
                    {
                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << index.row() << std::endl;
                        setItemRows.insert(index.row());

                        QWidget* currentWgt = index.data(CustomRole::CRBlockWgt).value<QWidget*>();
                        vecWidgets.append(currentWgt);

                        QModelIndex currentIndex_ = _proxy_->mapToSource(index);
                        QStandardItem* item = _itemmodel_->itemFromIndex(currentIndex_);
                        vecItems.append(item);

                        if (!parentItem)
                            parentItem = item->parent();

                        AI3D::CORE::BlockObject* blockData = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                        vecBlockDatas.append(blockData);

                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        BeforeDeleteOneBlock(index);
                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    }

                    for (auto& t : vecBlockDatas)
                    {
                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        DeleteOneBlockData(t);
                    }

                    for (auto& t : vecWidgets)
                    {
                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        DeleteOneBlockWidget(t);
                    }

                    for (auto& t : vecItems)
                    {
                        // delete reconstruction/production widget from stackedWidget under current block item.
                        //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        RemoveWidgetsUnderCurrentBlock(t);
                    }

                    if (parentItem != nullptr)
                    {
                        for (auto& t = setItemRows.rbegin(); t != setItemRows.rend(); t++)
                        {
                            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << *t << std::endl;
                            parentItem->removeRow(*t);
                        }
                    }

                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    //定位到根节点
                    QModelIndex rootIndex1 = ui_treeView_project->rootIndex();
                    QModelIndex selindex = _itemmodel_->index(0, 0, rootIndex1);
                    QModelIndex rootIndex = selindex;// currentIndex_.parent();               

                    ProjectInfoWgt* projectWgt = dynamic_cast<ProjectInfoWgt*>(_itemmodel_->data(rootIndex, CustomRole::CRProjectWgt).value<QWidget*>());
                    if (projectWgt)
                    {
                        projectWgt->Update_Block_Info();
                    }

                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                }
                else
                    return;
            }
            else
                return;

#if 0
            qApp->processEvents();
            auto deleteblocks = [&]() {
                ///return promanager->GetProject()->MergeBlocks(blockids); 

                return true;
            };
            QFuture<bool> future = QtConcurrent::run(deleteblocks);
            future.waitForFinished();

            ///block_t MergeBlockId = *promanager->GetProject()->GetBlockIds().rbegin();
            ///AI3D::CORE::BlockObject* MergeBlock = promanager->GetProject()->GetBlock(MergeBlockId);

            LOGI("Deleting Blocks");
            ///promanager->AddBlockManager(MergeBlock);

            //显示到界面上
            ///QStandardItem* item_ = GetBlockATData(MergeBlock);
            ///ShowBlockWidget(MergeBlock, item_, true);

            ///MergeBlock->GetTaskInfoMutual().isSaved = false;
            ///item_->setData(2, CustomRole::CanSaveBlock);
            
#endif
            SetFileModifiedProj();
            promanager->SetProejctModified(true);
        }

        void  MohackerWin::Slot_Action_UserManual()
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile("MoldAI User Manual.pdf"));
        }
        void MohackerWin::Slot_Action_About()
        {
            QString strVersion;
            // note: check it later. @240321
#if 1
            if(!currentVersionName.isEmpty())
                strVersion = QString(tr("%1 %2")).arg("MoldAI ").arg(currentVersionName);
            else
                strVersion = QString(tr("%1 %2")).arg("MoldAI ").arg(VERSION.c_str());
#endif
            ///Message_Box::about(this, QStringLiteral("about"), strVersion);

            if (bGotNewVersion)
            {
                Message_Box::aboutUpdate(this, QStringLiteral("about"), strVersion);
            }
            else
            {
                Message_Box::about(this, QStringLiteral("about"), strVersion);
            }
                
                     
        }

        void MohackerWin::Slot_ItemDataChanged(QStandardItem* item)
        {

            if (item->text().isEmpty())
            {
                QVariant var = item->data(CustomRole::CRItemType);
                if (var.isValid())
                {
                    ItemType itemtype = var.value<ItemType>();
                    switch (itemtype) {
                    case ItemType::ITBlock:
                        item->setText(str2qstr(item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetName()));
                        item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetTaskInfoMutual().isSaved = false;
                        break;
                    case ItemType::ITProject:
                        //item->setText(item->data(CustomRole::CRProjectData).value< Project* >()->project_name);
                        break;

                    case ItemType::ITReconstruction:
                        //item->setText(QString::fromLocal8Bit(item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetName().c_str()));
                        //item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->GetTaskInfoMutual().isSaved = false;

                        break;

                    case ItemType::ITProduction:
                        break;

                    default:
                        break;
                    }
                }
            }
            else
            {

                QVariant var = item->data(CustomRole::CRItemType);
                if (var.isValid())
                {
                    ItemType itemtype = var.value<ItemType>();
                    if (ItemType::ITBlock == itemtype) {

///                        item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->ReName(item->text().toStdString());

                        item->data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>()->ReName(qstr2str(item->text()));

                    }
                    else if (ItemType::ITReconstruction == itemtype)
                    {
                        // static int ReNameReconstruction(BlockObject * block, group_t reconstruction_id, const std::string & name);
                        AI3D::CORE::BlockObject* block = item->data(CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();
                        AI3D::CORE::ReconstructionObject* pReconstructionObject = item->data(CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();                       
                        if (block && pReconstructionObject)
                        {
                            try
                            {
                                AI3D::CORE::ReconstructionCommandSet::ReNameReconstruction(block, pReconstructionObject->GetId(), qstr2str(item->text()));
                                SetFileModifiedProj();
                            }
                            catch (std::exception& ex)
                            {
                                std::cout << "inside " <<  ",exception occured:" << ex.what() << std::endl;
                            }
                        }
                    }

                   /* else if (ItemType::ITProject == itemtype)
                        item->data(CustomRole::CRProjectData).value< Project* >()->project_name = item->text();*/

                    
                    
                }
            }

        }

        void MohackerWin::TreeViewClicked(const QModelIndex &currentIndex, const QModelIndex& previousIndex)
        {
            
            if(currentIndex.isValid())
                Slot_ProjectTreeView_ItemClicked(currentIndex);
        }
   

       
        void MohackerWin::Slot_ProjectTreeView_ItemClicked(const QModelIndex& clickedindex)
        {
            if (bCtrlPressed)
                return;
            QModelIndex index = clickedindex;
            if (!index.isValid())
                return;

            QModelIndex index_ = _proxy_->mapToSource(index);
            if (index_.isValid())
                index = index_;

            currentIndex_ = index;

            switch (index.data(CustomRole::CRItemType).value<ItemType>()) {
            case ItemType::ITProject:
            {
                ProjectInfoWgt* projectWgt = dynamic_cast<ProjectInfoWgt*>(_itemmodel_->data(index, CustomRole::CRProjectWgt).value<QWidget*>());
                if (projectWgt) {
                    ui_stackedWidget->setCurrentWidget(projectWgt);        
                    projectWgt->Update_Block_Info();
                }
            }
            break;

            case ItemType::ITBlockAT:
            {
                QStandardItem* item = _itemmodel_->itemFromIndex(index);
                QStandardItem* itemParent = item->parent();
                index = _itemmodel_->indexFromItem(itemParent);
            }
            break;
            case ItemType::ITBlock:
            {              
                m_pProgressBar->show();
                
                QCoreApplication::processEvents();
                //Sleep(2000);
                QStandardItem* item = _itemmodel_->itemFromIndex(index);

                
             
                AI3D::CORE::BlockObject* block_data = _itemmodel_->data(index, CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                
                ShowBlockWidget(block_data,item,true);//chy


                //读取xml文件  刺点流程有多个xml文件，需注意区分
                BlockWgt* currentWgt = dynamic_cast<BlockWgt*>(_itemmodel_->data(index, CustomRole::CRBlockWgt).value<QWidget*>());
                if (currentWgt)
                {
                    BlockWgt* lastWgt = dynamic_cast<BlockWgt*>(_itemmodel_->data(lastindex_, CustomRole::CRBlockWgt).value<QWidget*>());
                    if (lastWgt)
                    {
                    //    lastWgt->viewWidget_ui->mWindow->viewerWindow->setSceneData(nullptr);
                        lastWgt->viewWidget_ui->mWindow->clearSceneData();
                    }
                    ui_stackedWidget->setCurrentWidget(currentWgt);
                    currentWgt->viewWidget_ui->mWindow->viewerWindow->setSceneData(currentWgt->viewWidget_ui->mWindow->getOsgEngine()->GetRootNode());
                    lastindex_ = currentIndex_;
                }

                m_pProgressBar->hide();
              

            }
            break;

            case ItemType::ITReconstruction:
            {
                QWidget* currentWgt = dynamic_cast<QWidget*>(_itemmodel_->data(index, CustomRole::CRBlockData).value<QWidget*>());
                if (currentWgt)
                {
                    ui_stackedWidget->setCurrentWidget(currentWgt);
                    /* lastindex_ = currentIndex_;*/
                }
               else
                {
                    QStandardItem* recons_item = _itemmodel_->itemFromIndex(index);
                    QStandardItem* itemParent = recons_item->parent();


                    AI3D::CORE::BlockObject* block = _itemmodel_->data(index, CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();
                    ///std::cout << block->GetTaskInfo().statisticinfo_.tiepointnum << std::endl;;
                   
                    AI3D::CORE::ReconstructionObject* pReconstructionObject = _itemmodel_->data(index, CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();

                    

                    {
                        bool bSaveFinished = false;

                        auto savefunc = [&, this]()
                        {
                            LOGI("preparing to load reconstruction...");

                            
                                AI3D::CORE::BlockObject blockload;
                                auto atdata = std::make_shared<ATData>();
                                //@attention此处双方还未统一好GCP这一块，所以暂不用Bin

                                std::string atbin = pReconstructionObject->GetPath() + "/" + PRODUCTIONVIEWIDSBIN;
                                std::string atxml = pReconstructionObject->GetPath() + "/views.xml";
                                atbin = AI3D::CORE::File::EnsureUnifySlash(atbin);
                                atxml = AI3D::CORE::File::EnsureUnifySlash(atxml);
                                auto res =  blockload.LoadATBinary(atbin, atdata);
                              // auto res = blockload.LoadATXML(atxml, atdata);
                               if (res)
                               {
                                   pReconstructionObject->SetATData(*atdata.get());
                                   srs_s atcustomsrs = pReconstructionObject->GetCustomSrs();
                                   AI3D::CORE::ATData atdata_custom = *atdata.get();
                                   atdata_custom.TransFormATData(atcustomsrs.definition);
                                   atdata_custom.ComputeDepths();
                                   pReconstructionObject->SetATDataCustom(atdata_custom);
                               }

                            

                            LOGI("load reconstruction finished.");
                           
                            bSaveFinished = true;
                            //LOGI("res:" + res);

                            return res;
                            //return true;
                        };
                        bool ret = false;
                        if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                        {
                            LOGI("OpenLoadingPrompt:Loading recontruction now, pls wait for a while...");
                            if (BlockObject::isChineseVersion())
                            {
                                OpenLoadingPromptV4("正在加载重建，请稍等");
                            }
                            else
                            {
                                OpenLoadingPromptV4("Loading recontruction now,pls wait for a while");
                            }


                            QFuture<bool> f1 = QtConcurrent::run(savefunc);

                            while (!bSaveFinished)
                            {


                                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }
                            ret = f1.result();
                            //LOGI("ret:" + ret);
                        }
                        else
                        {
                           ret = savefunc();
                           //LOGI("ret2:" + ret);
                           
                        }
                        if (ret)
                        {
                           
                            ConstructionWgt* constructionWgt = new ConstructionWgt(block, pReconstructionObject, recons_item);
                            connect(constructionWgt, &ConstructionWgt::Sig_NewProduction, this, &MohackerWin::Slot_NewProduction);
                            connect(constructionWgt, &ConstructionWgt::Sig_ProjModifed, this, &MohackerWin::Slot_ProjModifed);

                            recons_item->setData(QVariant::fromValue(constructionWgt), CustomRole::CRBlockData);
                            recons_item->setData(QVariant::fromValue((QWidget *)constructionWgt), CustomRole::CRBlockWgt);

                            ui_stackedWidget->addWidget(constructionWgt);

                            ui_stackedWidget->setCurrentWidget(constructionWgt);
                            LOGI("load end.");
                        }
                        else
                        {
                            LOGE("load reconstruction failed.");
                        }
                       
                    }


                    if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
                    {

                        CloseLoadingPromptV4();
                    }
                }

            }
            break;

            case ItemType::ITProduction:
            {
                QWidget* currentWgt = dynamic_cast<QWidget*>(_itemmodel_->data(index, CustomRole::CRBlockData).value<QWidget*>());
                if (currentWgt)
                {
                    ui_stackedWidget->setCurrentWidget(currentWgt);
                    /* lastindex_ = currentIndex_;*/
                }
                else
                {
                    QStandardItem* product_item = _itemmodel_->itemFromIndex(index);
                    QStandardItem* recons_item = product_item->parent();
                    ///index = _itemmodel_->indexFromItem(itemParent);

                    AI3D::CORE::BlockObject* block = _itemmodel_->data(index, CustomRole::CRParentBlockData).value<AI3D::CORE::BlockObject*>();
                    
                    ///if (!block->GetTaskInfo().isLoaded)
                    ///{
                    ///    ProjectManager* promanager = ProjectManager::GetInstance();                     
                    ///}

                    ConstructionWgt* pConstructionWgt = nullptr;
                    if (recons_item != nullptr)
                    {
                        pConstructionWgt = dynamic_cast<ConstructionWgt *>(recons_item->data(CustomRole::CRBlockWgt).value<QWidget*>());
                        if (pConstructionWgt != nullptr)
                        {
                            std::ostringstream oss;
                            oss << "inside " << __FUNCTION__ << " " << __LINE__ << " got constructionwgt:"
                                << std::hex << std::showbase << pConstructionWgt << std::dec;
                            //LOGI(oss.str());
                        }
                        else
                        {
                            std::ostringstream oss;
                            oss << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " didn't get constructionwgt(nullptr).";
//                            LOGI(oss.str());
                        }
                    }

                    AI3D::CORE::ReconstructionObject* pReconstructionObject = _itemmodel_->data(index, CustomRole::CRReconstructionData).value<AI3D::CORE::ReconstructionObject*>();
                    AI3D::CORE::ProductionObject* pProductionObject = _itemmodel_->data(index, CustomRole::CRProductionData).value<AI3D::CORE::ProductionObject*>();

                    ProductionWgt* productionWgt = new ProductionWgt(block, pReconstructionObject, recons_item, pProductionObject, product_item);
                    product_item->setData(QVariant::fromValue(productionWgt), CustomRole::CRBlockData);
                    product_item->setData(QVariant::fromValue((QWidget *)productionWgt), CustomRole::CRBlockWgt);
                    ui_stackedWidget->addWidget(productionWgt);

                    ui_stackedWidget->setCurrentWidget(productionWgt);

                    if (pConstructionWgt != nullptr)
                    {
                        connect(productionWgt, &ProductionWgt::signal_delete_production_done, pConstructionWgt, &ConstructionWgt::Slot_Delete_Production_Done);
                        productionWgt->pConstructionWgt = pConstructionWgt;
                    }
                }
            }
            break;

            default:
                break;
            }
        }
        void MohackerWin::Slot_ProjectTreeView_ItemDoubleClicked(const QModelIndex& index)
        {
            QVariant var = index.data(CustomRole::CRItemType);

            if (bCtrlPressed)
                return;

            if (!var.isValid())
                return;
            switch (var.value<ItemType>()) 
            {
            case ItemType::ITProject:
            {
                //?chy @zhaokang 
                Q_UNUSED(index);
                break;
            }
            case ItemType::ITBlock:
            case ItemType::ITBlockAT:
            {
                AI3D::CORE::BlockObject* blockData = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                blockData->GetTaskInfoMutual().isSaved = false;
                blockData->GetTaskInfoMutual().savetype_ = SaveType_e::PROJECT_SAVED;
                ui_action_save->setEnabled(true);
                break;
            }

            case ItemType::ITReconstruction:
                break;

            case ItemType::ITProduction:
                break;

            default:
                break;
            }
        }
       
        void MohackerWin::Slot_TreeView_CustomContextMenuRequested(const QPoint& pos)
        {
            QItemSelectionModel* model_selection = ui_treeView_project->selectionModel();

            QModelIndexList IndexList = model_selection->selectedIndexes();
            if (IndexList.size() > 1) {
                foreach(QModelIndex index, IndexList)
                {
                    if (!index.isValid()) return;
                    ///  note: may be 
                    if (index.data(CustomRole::CRItemType).value<ItemType>() != ItemType::ITBlock) return;
                }
                ui_menu_rightClick_selectRows->exec(QCursor::pos());
                return;
            }

            QModelIndex index = ui_treeView_project->indexAt(pos);
           
            QVariant var = index.data(CustomRole::CRItemType);
            if (!var.isValid())
                return;
            switch (var.value<ItemType>())
            {
            case ItemType::ITProject:
            {     
                Slot_ProjectTreeView_ItemClicked(index);
               /* auto runfunc = [=]() {Slot_ProjectTreeView_ItemClicked(index); };
            runfunc();*/
            ui_menu_rightClick_project->exec(QCursor::pos());
            ui_action_openFileInExplorer->setEnabled(true);
            }
                break;
            case ItemType::ITBlock:
                if (index.data(CustomRole::CanSaveBlock).toInt() == 2)
                {
                    ui_action_openFileInExplorer->setEnabled(false);
                 }
                else
                {                   
                    ui_action_openFileInExplorer->setEnabled(true);
                }
                {
                    AI3D::CORE::BlockObject* blockData = index.data(CustomRole::CRBlockData).value<AI3D::CORE::BlockObject*>();
                    
                   
                    //if(blockData->GetTiepointFullStatus()) //(blockData->GetCurrentAT()->HasTiepoints())
                    //{
                    //    ui_action_simplify_block->setEnabled(true);
                    //}

                    ui_action_rename->setEnabled(blockData->GetStatus() == jobsta_e::STATUS_RUNNING?false:true);
                    ui_action_delete->setEnabled(true);
                    ui_action_exportToXml->setEnabled((blockData->GetStatus() == jobsta_e::STATUS_COMPLETE
                        || blockData->GetStatus() == jobsta_e::STATUS_NEW)?true :false);
                    ui_action_clone_block->setEnabled((blockData->GetStatus() == jobsta_e::STATUS_CANCLE
                        || blockData->GetStatus() == jobsta_e::STATUS_FAILURE) ? false : true);
                    bool cansimplify = false;
                    if (!blockData->HasReconstructions() && blockData->GetStatus() == jobsta_e::STATUS_COMPLETE)
                    {
                        cansimplify = true;
                    }
                    //ui_action_simplify_block->setEnabled(cansimplify);*/
                   ProjectManager* promanager = ProjectManager::GetInstance();
                    
                    Block_Status_s  BlockStatus = promanager->GetBlockManaget(blockData->GetId())->GetBlockStatusMutual();                  
                    ui_action_importGcpMeasurements->setEnabled(BlockStatus.can_add_gcp);
                    ui_action_exportGcpMeasurements->setEnabled(blockData->HasControlPoints());
                    auto runfunc = [=]() {Slot_ProjectTreeView_ItemClicked(index); };
                    
                    runfunc();
                    ui_menu_rightClick_block->exec(QCursor::pos());

                }
               
                break;

            case ItemType::ITReconstruction:
                {
                    // to open folder now only under a reconstruction item.
                    ui_action_openFileInExplorer->setEnabled(true);
                    ui_action_rename->setEnabled(true);
                    ui_action_delete->setEnabled(true);
                    ui_action_clone_block->setEnabled( true);
                    auto runfunc = [=]() {Slot_ProjectTreeView_ItemClicked(index); };

                    runfunc();
                    ui_menu_rightClick_Reconstruction->exec(QCursor::pos());
                }
                break;

            case ItemType::ITProduction:
            {
                // to open folder now only under a production item.
                ui_action_openFileInExplorer->setEnabled(true);
                ui_action_rename->setEnabled(false);
                ui_action_delete->setEnabled(true);
                auto runfunc = [=]() {Slot_ProjectTreeView_ItemClicked(index); };

                runfunc();
                ui_menu_rightClick_Production->exec(QCursor::pos());
            }
            break;

            default:
                break;
            }
        }

        void  MohackerWin::SaveFinished()
        {
            ProjectManager* managet = ProjectManager::GetInstance();
            ui_action_save->setEnabled(false);
            QApplication::restoreOverrideCursor();
        }        

        void MohackerWin::Slot_CheckVersion()
        {
            //std::cout << " mw/checkVersion timeout." << std::endl;
            // 判断当前程序是否检测到已经下载到更新版本到当前机器.
            if (bGotNewVersion)
            {
                // 如已将更新版本下载到当前机器上,停掉新版本检测定时器
                pCheckVersionTimer->stop();

                //ui_action_about->setIcon(QIcon("new.png"));
                //QPixmap pixmap(":/new/prefix1/skin/default.png");

                // 在about菜单项前显示New图标.
                ///new/prefix1/可以看做一个命名空间
                ui_action_about->setIcon(QIcon(":/new/prefix1/skin/new.png"));
                ui_menu_help->setStyleSheet("QMenu::icon{position:absolute;left:10px;}");

                std::cout << " new update got,stop the check timer." << std::endl;
            }
        }

        //用户中心
        void  MohackerWin::Slot_Action_Login()
        {
            AI3D::GUI::LoginDialog* loginDialog = new AI3D::GUI::LoginDialog;
            QObject::connect(loginDialog, &AI3D::GUI::LoginDialog::closeAll, this, &AI3D::GUI::MohackerWin::Slot_quit);
            if (isLogin())
            {
                QMessageBox errBox;
                errBox.warning(this, "登录", "当前账号已登录，如需切换账号，请先退出当前账号");
            }
            else {
                loginDialog->show();
            }
            
        }
        void MohackerWin::Slot_Action_Info()
        {
            HttpReply reply = getAccount();
            if (reply.code == 0) {
                QJsonObject info = reply.data.value("data").toObject();
                QString username = info.value("username").toString();
                int level = info.value("level").toInt();
                qDebug() << "level" << level << "\n";
                QString text = username + "\n" + "等级" + QString::number(level);
                ui_action_info->setText(text);
            }

        }

        void MohackerWin::Slot_Action_Logout()
        {
            if (isLogin()) {
                //已登录
                if (userLogout()) {
                    //登出
                    ui_action_info->setText("个人信息");
                }
            }

        }

        void MohackerWin::Slot_quit() {
            this->close();
            QApplication::quit();
        }

        CSortFilterProxyModel::CSortFilterProxyModel(QObject* parent)
            :QSortFilterProxyModel(parent)
        {

        }

        bool CSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
        {
            if (!left.isValid() || !right.isValid())
                return false;

            QVariant leftData = sourceModel()->data(left);
            QVariant rightData = sourceModel()->data(right);
            QCollator collator;
            collator.setNumericMode(true);
            collator.setCaseSensitivity(Qt::CaseSensitive);
            collator.setIgnorePunctuation(false);

            if (leftData.canConvert<QString>() && rightData.canConvert<QString>())
            {
                QString strLeft = leftData.toString();
                QString strRight = rightData.toString();
                return collator.compare(strLeft, strRight) > 0;
            }

            return QSortFilterProxyModel::lessThan(left, right);
        }
        //?chy@zhaokang 
        void ImportXml::run()
        {
            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();

            ///bool IsImportBlock = promanager->GetProject()->ImportBlock(filename_.toStdString());

            bool IsImportBlock = promanager->GetProject()->ImportBlock(qstr2str(filename_));

            emit FinishedRead(filename_, IsImportBlock);


        }

        DataPreProcess::DataPreProcess(QWidget* parent,AI3D::CORE::BlockObject *pBlockObject)
            : QWidget(parent)
        {
            this->setWindowFlags(Qt::FramelessWindowHint);
            this->setWindowTitle("Data preprocess");
            this->pBlockObject = pBlockObject;
            setStyleSheet("background-color:rgb(101,101,101);");
            setContentsMargins(2, 2, 2, 2);            

            QVBoxLayout* vlMain = new QVBoxLayout();
            vlMain->setContentsMargins(0, 0, 0, 0);
            vlMain->setMargin(0);

            QHBoxLayout* hlTitle = new QHBoxLayout();
            hlTitle->setContentsMargins(0, 0, 0, 0);

            QLabel* lblTitle = new QLabel(this);
            butClose = new QPushButton(this);

            lblTitle->setText("Data preprocess");
            lblTitle->setStyleSheet("color:white;margin-left:20px;font: 14px \"Arial\";");

            //butClose->setText("Close");
            //butClose->setIcon(QPixmap(":/new/prefix1/skinbutton/sclose.png"));
            
            //butClose->setIcon(QPixmap(":/new/button/skinbutton/sclose.png"));

//            QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
            //QPixmap closePix(":/new/prefix1/skinbutton/sclose.png");
 //           butClose->setIcon(closePix);
            butClose->setIcon(QPixmap(":/new/prefix1/skin/closelogo.png"));
            //butClose->setText("X");
            butClose->setStyleSheet("background-color:rgb(101,101,101);border:none;color:white;height:30px;margin-right:20px;");

            hlTitle->addWidget(lblTitle);
            hlTitle->addStretch(1);
            hlTitle->addWidget(butClose,0,Qt::AlignRight|Qt::AlignVCenter);

            QWidget* pBottomWidget = new QWidget(this);
            pBottomWidget->setStyleSheet("background-color:rgb(32,33,36);height:80px;border:none;margin:0px;padding:0px;");
            pBottomWidget->setContentsMargins(0, 0, 0, 0);

            QHBoxLayout* hlBottom = new QHBoxLayout();
            hlBottom->setContentsMargins(0, 5, 0, 5);

            butApply = new QPushButton(this);
            butOk = new QPushButton(this);
            butCancel = new QPushButton(this);
           
            butApply->setText("Apply");
            butOk->setText("OK");
            butCancel->setText("Cancel");

            butApply->setVisible(false);

            //butOk->setStyleSheet("QPushButton {background-color:rgb(0x16,0x9b,0xd5,0xff);color:white;border-radius:6px;}");
            butApply->setStyleSheet("border-radius:6px;height:22px;border:2px solid darkgrey;");
            butOk->setStyleSheet("QPushButton {background-color:rgb(22,155,213,255);color:white;border-radius:6px;width:120px;height:26px;font: 14px \"Arial\";}"
                "QPushButton:disabled {background-color:rgb(128,128,128,255);color:white;}"
            );

            //butOk->setStyleSheet("border-radius:6px;height:22px;border:2px solid darkgrey;");
            butCancel->setStyleSheet("border-radius:6px;width:120px;height:22px;border:2px solid darkgrey;background-color:white;color:black;");
            butCancel->setVisible(false);
            
            hlBottom->addStretch(1);
            hlBottom->addWidget(butOk,0,Qt::AlignRight|Qt::AlignVCenter);
            hlBottom->addWidget(butCancel,0,Qt::AlignRight|Qt::AlignVCenter);
            hlBottom->setContentsMargins(0, 15, 30, 15);
            hlBottom->setSpacing(10);

            QFrame* lineUnderTitle = new QFrame(this);
            lineUnderTitle->setFrameShape(QFrame::HLine);
            lineUnderTitle->setFrameShadow(QFrame::Plain);
            lineUnderTitle->setLineWidth(1);
            lineUnderTitle->setStyleSheet("background-color:rgb(181,176,176);border:none;color:rgb(181,176,176);height:1px;");

            QHBoxLayout* hlCategory = new QHBoxLayout();

            QLabel* lblCategory = new QLabel(this);

            gbCategory = new QGroupBox(this);

            rbCategoryDataPreprocessWithAT = new QRadioButton(gbCategory);
            rbCategoryDataPreprocess = new QRadioButton(gbCategory);

            rbCategoryDataPreprocessWithAT->setEnabled(false);

            lblCategory->setText("Processing type");
            lblCategory->setStyleSheet("color:white;margin-left:45px;");

            rbCategoryDataPreprocessWithAT->setText("Data preprocess and submit AT");
            rbCategoryDataPreprocessWithAT->setStyleSheet("color:white;");
            rbCategoryDataPreprocess->setText("Data Preprocess");
            rbCategoryDataPreprocess->setChecked(true);
            rbCategoryDataPreprocess->setStyleSheet("color:white;");

            hlCategory->addWidget(lblCategory);
            hlCategory->addWidget(rbCategoryDataPreprocessWithAT);
            hlCategory->addWidget(rbCategoryDataPreprocess);

            gbCategory->setStyleSheet("border:none;");
            gbCategory->setLayout(hlCategory);
            
            gbCategory->setVisible(false);

            QHBoxLayout* hlTitle2 = new QHBoxLayout();
            QLabel* lblTitle2 = new QLabel(this);
            lblTitle2->setText("Preprocess task list");
            lblTitle2->setStyleSheet("color:white;margin-left:15px;font: 14px \"Arial\";");
            hlTitle2->addWidget(lblTitle2, 0, Qt::AlignLeft);

            QHBoxLayout* hlTaskButtonLayout = new QHBoxLayout();
            
            butAddTask = new QPushButton(this);
            butAddTask->setText("Add task");
            //butAddTask->setIcon(QPixmap(":/new/prefix1/skin/fileunclickable1x.png"));
            butAddTask->setIcon(QPixmap(":/new/prefix1/skin/tianjia.png"));
            butAddTask->setStyleSheet("background-color:rgb(101,101,101);color:white;border:none;font: 14px \"Arial\";");
            //butAddTask->setFlat(true);

            QStyle* pStyle = QApplication::style();
            QIcon icon = pStyle->standardIcon(QStyle::SP_TitleBarMinButton);

            butDelTask = new QPushButton(this);
            butDelTask->setText("Delete task");
            butDelTask->setStyleSheet("background-color:rgb(101,101,101);color:white;border:none;font: 14px \"Arial\";");
            //butDelTask->setIcon(QPixmap(":/new/prefix1/skin/closelogo.png"));
            //butDelTask->setIcon(icon);
            //butDelTask->setFlat(true);
            butDelTask->setIcon(QPixmap(":/new/prefix1/skin/shanchu.png"));

            icon = pStyle->standardIcon(QStyle::SP_TitleBarCloseButton);
            butCancelTask = new QPushButton(this);
            butCancelTask->setText("Cancel task");
            butCancelTask->setIcon(icon);
            butCancelTask->setEnabled(false);
            butCancelTask->setStyleSheet("background-color:rgb(101,101,101);color:white;border:none;");
            butCancelTask->setVisible(false);

            hlTaskButtonLayout->addWidget(butAddTask);
            hlTaskButtonLayout->addWidget(butDelTask);
            hlTaskButtonLayout->addWidget(butCancelTask);
            hlTaskButtonLayout->setSpacing(30);

            hlTaskButtonLayout->addStretch(1);

            hlTaskButtonLayout->setContentsMargins(15, 10, 0, 0);

            QHBoxLayout* hlTaskMainArea = new QHBoxLayout();
            twTaskList = new QTableWidget(this);

            QStringList slTitle;
            slTitle << "ID" << "Preprocess data" << "Task status";

            twTaskList->setColumnCount(3);
            //twTaskList->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color:white;color:black;}");
            twTaskList->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color:rgb(157,150,150);color:white;font: 14px \"Arial\";}");
            twTaskList->setHorizontalHeaderLabels(slTitle);
            //twTaskList->horizontalHeader()->setStretchLastSection(true);
            twTaskList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            twTaskList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
            twTaskList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
            twTaskList->verticalHeader()->hide();
            twTaskList->setSelectionMode(QAbstractItemView::SingleSelection);
            twTaskList->setEditTriggers(QAbstractItemView::NoEditTriggers);
            twTaskList->setSelectionBehavior(QAbstractItemView::SelectRows);
            twTaskList->setTextElideMode(Qt::ElideNone);
            twTaskList->horizontalHeader()->setFixedHeight(35);
            //twTaskList->setStyleSheet("QTableWidget {background-color:rgb(90,90,90);} QTableWidget::item {selection-background-color:rgb(79,135,161);selection-color:white;background-color:rgb(123,116,116);color:white;}");
            
            //twTaskList->setStyleSheet("QTableWidget::item { background-color:#FF0000;color:#00FF00;selection-background-color:green;selection-color:yellow; }"
             //   "");
            
//            twTaskList->setStyleSheet("QTableWidget { background-color:yellow;}"
//                        "QTableWidget::item { background-color:#FF0000;color:#00FF00;selection-background-color:blue;selection-color:white;}");

            twTaskList->setStyleSheet("QTableWidget { background-color:rgb(101,101,101);font: 14px \"Arial\";}"
                    "QTableWidget::item { background-color:rgb(123,116,116);color:white;}"
                    "QTableWidget::item:selected { background-color:rgb(79,135,161);color:white;}");

///            twTaskList->setStyleSheet("QTableWidget::item { background-color:#FF0000;color:#00FF00;selection-background-color:blue;selection-color:white; }"
///                "");

            twTaskList->setColumnWidth(0, 140);
            twTaskList->setColumnWidth(2, 100);

            //teTaskMainDummy = new QTextEdit(this);

            //teTaskMainDummy2 = new QTextEdit(this);
            //butTaskMainDummy2 = new QPushButton(this);
            //butTaskMainDummy2->setText("task main2");

            QWidget* taskSettingsContainer = new QWidget(this);
            taskSettingsContainer->setStyleSheet("border:1px solid darkgrey;");

            QVBoxLayout* vlTaskSettings = new QVBoxLayout();
            vlTaskSettings->setContentsMargins(0, 0, 0, 0);
            vlTaskSettings->setMargin(0);

            QHBoxLayout* hlTaskSettingsTitle = new QHBoxLayout();
            hlTaskSettingsTitle->setContentsMargins(15, 0, 30, 0);
            hlTaskSettingsTitle->setMargin(0);

            QLabel *lblSettingTitle = new QLabel(this);
            lblSettingTitle->setText("Preprocessing rules setting");
            lblSettingTitle->setFixedHeight(35);
            lblSettingTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            lblSettingTitle->setStyleSheet("background-color:rgb(157,150,150);color:white;padding:0px;margin:0px;font: 14px \"Arial\";");
            lblSettingTitle->setContentsMargins(0, 0, 0, 0);
            lblSettingTitle->setMargin(0);
            hlTaskSettingsTitle->addWidget(lblSettingTitle);

            vlTaskSettings->addLayout(hlTaskSettingsTitle,0);

            swTaskSettings = new QStackedWidget(this);
            swTaskSettings->setStyleSheet("border:none;");

            vlTaskSettings->addWidget(swTaskSettings, 1);

            photoSetting = nullptr;
            posSetting = nullptr;

            //photoSetting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            //posSetting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            //swTaskSettings->setContentsMargins(0, 0, 0, 0);
            //swTaskSettings->setStyleSheet("background-color:rgb(90,90,90);");

            //hlTaskMainArea->addWidget(teTaskMainDummy, 1);

            hlTaskMainArea->addWidget(twTaskList, 1);
///            hlTaskMainArea->addWidget(teTaskMainDummy2, 1);

            //hlTaskMainArea->addWidget(swTaskSettings, 1);           
            //hlTaskMainArea->addWidget(swTaskSettings,1);
            taskSettingsContainer->setLayout(vlTaskSettings);
            taskSettingsContainer->setContentsMargins(1, 1, 1, 1);

            //hlTaskMainArea->addLayout(vlTaskSettings,1);
            hlTaskMainArea->addWidget(taskSettingsContainer, 1);
            

            hlTaskMainArea->setContentsMargins(5, 0, 5, 0);
            hlTaskMainArea->setSpacing(5);

            gbTaskExtra = new QGroupBox(this);
            gbTaskExtra->setFlat(true);
            gbTaskExtra->setStyleSheet("border:none;");
            gbTaskExtra->setContentsMargins(0, 0, 0, 0);

            QVBoxLayout* vlTaskExtraArea = new QVBoxLayout();
            QHBoxLayout* hlTaskExtraArea = new QHBoxLayout();

            lblExportDirectory = new QLabel(this);
            lblExportDirectory->setText("Export directory");
            lblExportDirectory->setStyleSheet("color:white;margin-left:10px;font: 14px \"Arial\";");

            leExportDirectory = new QLineEdit(this);
            //leExportDirectory->setEnabled(false);
            leExportDirectory->setReadOnly(true);
            leExportDirectory->setStyleSheet("border-radius:6px;height:26px;border:2px solid darkgrey;background-color:white;color:black;font: 14px \"Arial\";");

            ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();
            if (promanager)
            {
                std::string projPath = promanager->GetProject()->GetPath();
                leExportDirectory->setText(str2qstr(projPath));
            }

            butExportDirectory = new QPushButton(this);
            butExportDirectory->setText("...");
            butExportDirectory->setStyleSheet("border-radius:6px;border:2px solid darkgrey;width:26px;height:26px;color:white;font: 14px \"Arial\";");

            hlTaskExtraArea->addWidget(lblExportDirectory);
            hlTaskExtraArea->addWidget(leExportDirectory,2);
            hlTaskExtraArea->addWidget(butExportDirectory);            
            
//          teTaskExtraDummy = new QTextEdit(this);
//          hlTaskExtraArea->addWidget(teTaskExtraDummy, 1);

            hlTaskExtraArea->addStretch(1);

            hlTaskExtraArea->setContentsMargins(0, 0, 0, 0);

            vlTaskExtraArea->addStretch(1);
            vlTaskExtraArea->addLayout(hlTaskExtraArea,1);
            vlTaskExtraArea->addStretch(4);

            gbTaskExtra->setLayout(vlTaskExtraArea);
            
            //gbTaskExtra->setStyleSheet("border:none;margin:0px;padding:0px;");           

            vlMain->addLayout(hlTitle);
            vlMain->addWidget(lineUnderTitle);
            vlMain->addWidget(gbCategory);

            vlMain->addLayout(hlTitle2);
            vlMain->addLayout(hlTaskButtonLayout);

            //vlMain->addStretch(1);

            vlMain->addLayout(hlTaskMainArea, 2);
            ///vlMain->addLayout(hlTaskExtraArea, 1);           
            vlMain->addWidget(gbTaskExtra,1);

            pBottomWidget->setLayout(hlBottom);
            //vlMain->addLayout(hlBottom);
            vlMain->addWidget(pBottomWidget);

            connect(butApply, &QPushButton::clicked, this, &DataPreProcess::Slot_Apply);
            connect(butOk, &QPushButton::clicked, this, &DataPreProcess::Slot_Ok);
            connect(butCancel, &QPushButton::clicked, this, &DataPreProcess::Slot_Cancel);
            connect(butClose, &QPushButton::clicked, this, &DataPreProcess::Slot_Close);
            
            connect(butAddTask, &QPushButton::clicked, this, &DataPreProcess::Slot_AddTask);
            connect(butDelTask, &QPushButton::clicked, this, &DataPreProcess::Slot_DelTask);
            connect(butCancelTask, &QPushButton::clicked, this, &DataPreProcess::Slot_CancelTask);

            connect(butExportDirectory, &QPushButton::clicked, this, &DataPreProcess::Slot_ExportDirectory);

            connect(twTaskList, &QTableWidget::itemClicked, this, &DataPreProcess::Slot_TaskItemClicked);

            setLayout(vlMain);           
        }

        void DataPreProcess::saveOption(int taskId)
        {
            int taskCount = twTaskList->rowCount() / 2;

            if (taskCount <= 0 || taskId < 0 || taskId >= taskCount)
                return;

            AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption;

            dataPreprocessOption = dataPreprocessOptions.at(taskId);

            //if (!dataPreprocessOption.photoDir.empty() || !dataPreprocessOption.posFile.empty())
            if (!dataPreprocessOption.photoDir.empty())
            {
                std::cout << taskId << " " << dataPreprocessOption.photoDir << "/" << dataPreprocessOption.posFile << std::endl;

                int iPhotoSettingIndex = taskId * 2;
                int iPosSettingIndex = taskId * 2 + 1;

                // process photo settings
                //if(!dataPreprocessOption.photoDir.empty())
                {
                    DataPreprocessPhotoSetting* pRuleSetting = static_cast<DataPreprocessPhotoSetting*>(swTaskSettings->widget(iPhotoSettingIndex));
                    pRuleSetting->saveOption(dataPreprocessOption);
                    if (dataPreprocessOption.exportFileName.empty())
                        dataPreprocessOption.exportFileName = qstr2str(QString("%1.xml").arg(taskId+1,3,10,QLatin1Char('0')));
                }

                // process pose settings
                if (!dataPreprocessOption.posFile.empty())
                {
                    DataPreprocessPosSetting* pRuleSetting = static_cast<DataPreprocessPosSetting*>(swTaskSettings->widget(iPosSettingIndex));
                    pRuleSetting->saveOption(dataPreprocessOption);
                }

                if (!leExportDirectory->text().isEmpty())
                    dataPreprocessOption.exportDirectory = qstr2str(leExportDirectory->text());

                dataPreprocessOptions.at(taskId) = dataPreprocessOption;
            }
        }

        void DataPreProcess::dumpOption(int taskId)
        {
            int taskCount = twTaskList->rowCount() / 2;

            if (taskCount <= 0 || taskId < 0 || taskId >= taskCount)
                return;

            AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption;

            dataPreprocessOption = dataPreprocessOptions.at(taskId);

            // print the content of all the fields of one data preprocess option item.
        }

        int DataPreProcess::ProgFunc(int taskId, int progress)
        {
            if (twTaskList->rowCount() <= 0 || taskId < 0 || taskId >= (twTaskList->rowCount() / 2))
                return -1;

            QTableWidgetItem* pTableWidgetItem = twTaskList->item(taskId * 2, 2);

            if (progress == 100)
                pTableWidgetItem->setText(QString("Pass"));
            else if(progress == -1)
                pTableWidgetItem->setText(QString("Failed"));
            else
                pTableWidgetItem->setText(QString("Processing %1%%").arg(progress));

            return 0;
        }

        static int DataPreProcess_ProgFunc(void *thisObj,int taskId, int progress)
        {
            std::cout << "inside " << __FUNCTION__ << " " << taskId << " " << progress << std::endl;

            if (thisObj)
            {
                DataPreProcess* pDataPreProcess = static_cast<DataPreProcess*>(thisObj);
                if (pDataPreProcess)
                {
                    pDataPreProcess->ProgFunc(taskId,progress);
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }

            return 0;
        }

        // change Slot_Ok based on Slot_Apply.
        void DataPreProcess::Slot_Apply()
        {
 //            if (fname.isEmpty())
 //               return;

            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            int row = -1;
            int hasDoneNum = 0;
            for (int i = 0; i < twTaskList->rowCount(); i+= 2)
            {
                row = i;
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption;
                int iTaskId = row / 2;

                dataPreprocessOption = dataPreprocessOptions.at(iTaskId);

                // todo:simplify

                //if (!dataPreprocessOption.photoDir.empty() || !dataPreprocessOption.posFile.empty())
                if (!dataPreprocessOption.photoDir.empty())
                {
                    std::cout << i << " " << dataPreprocessOption.photoDir << "/" << dataPreprocessOption.posFile << std::endl;

                    saveOption(iTaskId);


                    dataPreprocessOption = dataPreprocessOptions.at(iTaskId);
                    dumpOption(iTaskId);

                    if (pBlockObject)
                    {
                        std::cout << "doing data preprocess by right clicking one block item from the project tree." << std::endl;
                        int* progress = new int(0);
///                        srs_s blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition("EPSG:4326");
                        srs_s blocksrs;
                        
                        if (dataPreprocessOption.SRS.empty())
                            blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName("EPSG:4326");
                        else
                            blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(dataPreprocessOption.SRS); ;// AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(dataPreprocessOption.SRS);

                        std::string genereated_xml;
                        
                        pBlockObject->UpdateSRSMap(blocksrs);
                        blocksrs.ID = pBlockObject->ExistSRS(blocksrs.definition);
                        
                        pBlockObject->BatchPreProcess(dataPreprocessOption,genereated_xml ,dataPreprocessOption.photoDir, progress, dataPreprocessOption.namePrefix, blocksrs, "", 
                            dataPreprocessOption.posFile, dataPreprocessOption.nameLength, dataPreprocessOption.nameStartNo);

                        emit generatedXml(genereated_xml);
                    }
                    else
                    {
                
                        std::cout << "doing data preprocess by clicking tool menu item from the main menubar." << std::endl;

                        ProjectManager* promanager = AI3D::GUI::ProjectManager::GetInstance();


///                        AI3D::CORE::BlockObject* block = new AI3D::CORE::BlockObject(promanager->GetProject()->GetPath());
                        AI3D::CORE::BlockObject* block = new AI3D::CORE::BlockObject(!dataPreprocessOption.exportDirectory.empty() ? dataPreprocessOption.exportDirectory: promanager->GetProject()->GetPath());

                        block->SetStatus(jobsta_e::STATUS_NEW);

///                        promanager->GetProject()->AddBlock(block);
///                        block->GetTaskInfoMutual().projectfile_ = promanager->GetProject()->GetPath() + "/" + promanager->GetProject()->GetName() + PROJECTFILE;

                        int* progress = new int(0);
                        ///                        srs_s blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition("EPSG:4326");
                        srs_s blocksrs;
                        
                        if (dataPreprocessOption.SRS.empty())
                            blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName("EPSG:4326");
                        else
                            blocksrs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(dataPreprocessOption.SRS);
                       
                            block->UpdateSRSMap(blocksrs);
                            blocksrs.ID = block->ExistSRS(blocksrs.definition);
                        std::string generated_xml;


                        block->BatchPreProcess(dataPreprocessOption, generated_xml, dataPreprocessOption.photoDir, progress, dataPreprocessOption.namePrefix, blocksrs, "",
                            dataPreprocessOption.posFile, dataPreprocessOption.nameLength, dataPreprocessOption.nameStartNo, DataPreProcess_ProgFunc,(void *)this,iTaskId);

///                        promanager->GetProject()->DeleteBlock(block->GetId());
///                        delete block;

                        std::cout << "generated xml:" << generated_xml << std::endl;

                        if (generated_xml.empty())
                        {

                            DataPreProcess_ProgFunc((void *)this,iTaskId,-1);

                        }
                        else
                        {
                            emit generatedXml(generated_xml);
                        }

                        hasDoneNum++;

                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                        delete progress;
                    }
                }
                else
                {
                    std::cout << i << " " << dataPreprocessOption.photoDir << "//" << dataPreprocessOption.posFile << std::endl;
                }
            }

            emit generatedXmls(hasDoneNum);
        }

        void DataPreProcess::Slot_Ok()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            //butOk->setStyleSheet("background-color:rgb(128,128,128);");
            butOk->setEnabled(false);

            //QMessageBox::information(nullptr,"info","ok clicked.");

            //QString strExportDirectory = leExportDirectory->text();
            //if (strExportDirectory.isEmpty())
            //{
                //QMessageBox::information(nullptr, "info", "no export directory set.");
            //}
            //else
            //{
                //QMessageBox::information(nullptr, "info", "export directory:" + strExportDirectory);
            //}

            Slot_Apply();

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            ///close();
        }
        
        void DataPreProcess::Slot_Cancel()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            //QMessageBox::information(nullptr,"info","cancel clicked.");
            close();
        }

        void DataPreProcess::Slot_Close()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            //QMessageBox::information(nullptr, "info", QString::fromUtf8(__FUNCTION__) + " clicked.");
            close();
        }

        void DataPreProcess::Slot_AddTask()
        {
//            std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

            if (dataPreprocessTaskData.count() >= 100)
                return;


            twTaskList->insertRow(twTaskList->rowCount());
            twTaskList->insertRow(twTaskList->rowCount());

            photoSetting = new DataPreprocessPhotoSetting(this);
            posSetting = new DataPreprocessPosSetting(this);

            swTaskSettings->addWidget(photoSetting);
            swTaskSettings->addWidget(posSetting);

            DataPreprocessTaskData dataPreprocessTaskDataItem;
            dataPreprocessTaskDataItem.taskStatus = 0;

            for (int i = twTaskList->rowCount() - 2; i < twTaskList->rowCount(); i++)
            {
                for (int j = 0; j < twTaskList->columnCount(); j++)
                {
                    if (j == 0)
                    {
                        QString colTitle;
                        if (i % 2 == 0)
                        {
                            colTitle = QString::number(twTaskList->rowCount() / 2) + "\t" + "Photo";
                            dataPreprocessTaskDataItem.strPhotoDir = "";
                            dataPreprocessTaskDataItem.strPhotoTitle = colTitle;
                        }
                        else
                        {
                            colTitle = QString(" ") + "\t" + QString("Pos");
                            dataPreprocessTaskDataItem.strPoseDir = "";
                            dataPreprocessTaskDataItem.strPoseTitle = colTitle;
                        }

                        QTableWidgetItem* pItem = new QTableWidgetItem(colTitle);
                        twTaskList->setItem(i, j, pItem);
                    }
                    else
                    {
                        if (j == 1)
                        {
                            if (i % 2 == 0)
                            {
                                DataPreprocessTaskWidget* pDataPreprocessTaskWidget = new DataPreprocessTaskWidget(this, true);
                                connect(pDataPreprocessTaskWidget, &DataPreprocessTaskWidget::chooseNewFile, this, &DataPreProcess::Slot_ChooseNewFile);
                                twTaskList->setCellWidget(i, j, pDataPreprocessTaskWidget);
                            }
                            else
                            {
                                DataPreprocessTaskWidget* pDataPreprocessTaskWidget = new DataPreprocessTaskWidget(this, false);
                                connect(pDataPreprocessTaskWidget, &DataPreprocessTaskWidget::chooseNewFile, this, &DataPreProcess::Slot_ChooseNewFile);
                                twTaskList->setCellWidget(i, j, pDataPreprocessTaskWidget);
                            }
                        }
                        else
                        {
                            QTableWidgetItem* pItem = new QTableWidgetItem();
                            twTaskList->setItem(i, j, pItem);
                        }
                    }
                }
            }

            dataPreprocessTaskData.append(dataPreprocessTaskDataItem);

            AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption;
            dataPreprocessOptions.push_back(dataPreprocessOption);
        }

        void DataPreProcess::Slot_DelTask()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            //QMessageBox::information(nullptr, "info", "del task clicked.");

            if (twTaskList->rowCount() <= 0)
                return;

            int nCurrRow = twTaskList->currentRow();
            if (nCurrRow < 0 || nCurrRow >= twTaskList->rowCount())
                return;

            int nTaskId = nCurrRow / 2;
            dataPreprocessTaskData.remove(nTaskId);

            twTaskList->removeRow(nTaskId * 2 + 1);
            twTaskList->removeRow(nTaskId * 2);

            for (int i = 0; i < twTaskList->rowCount(); i++)
            {
                if (i % 2 == 0)
                {
                    QTableWidgetItem* pPhotoItem = twTaskList->item(i, 0);
                    QString colTitle = QString::number(i / 2) + "\t" + "Photo";
                    pPhotoItem->setText(colTitle);
                }
            }

            swTaskSettings->removeWidget(swTaskSettings->widget(nTaskId * 2 + 1));
            swTaskSettings->removeWidget(swTaskSettings->widget(nTaskId * 2));

            //dataPreprocessOptions.erase(dataPreprocessOptions.begin() + nTaskId * 2, dataPreprocessOptions.begin() + (nTaskId + 1) * 2);
            dataPreprocessOptions.erase(dataPreprocessOptions.begin() + nTaskId, dataPreprocessOptions.begin() + (nTaskId + 1));
        }

        void DataPreProcess::Slot_CancelTask()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            QMessageBox::information(nullptr, "info", "cancel task clicked.");
        }

        void DataPreProcess::Slot_ExportDirectory()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            //QString txtExportDirectory = leExportDirectory->text();
            //if (txtExportDirectory.isEmpty())
            //    return;

            ///QMessageBox::information(nullptr,"info","export directory clicked:" + txtExportDirectory);

            QString strSelectedDir = QFileDialog::getExistingDirectory(nullptr, "选择Export Directory:", "./", QFileDialog::ShowDirsOnly);
            if (!strSelectedDir.isEmpty())
            {
                strSelectedDir.replace(QRegExp("\\"), "/");
                leExportDirectory->setText(strSelectedDir);
            }
        }

        void DataPreProcess::Slot_ChooseProcessCategory(bool bToggle)
        {
            if (bToggle)
            {
                QRadioButton* pRadioButton = static_cast<QRadioButton*>(sender());
                std::cout << qstr2str(pRadioButton->text()) << " choosed." << std::endl;
            }
        }

        void DataPreProcess::Slot_ChooseNewFile(QString fname)
        {
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (fname.isEmpty())
                return;

            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            int row = -1;
            DataPreprocessTaskWidget* pSenderDataPreprocessTaskWidget = static_cast<DataPreprocessTaskWidget*>(sender());

            for (int i = 0; i < twTaskList->rowCount(); i++)
            {
                row = i;
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                DataPreprocessTaskWidget* pDataPreprocessTaskWidget = static_cast<DataPreprocessTaskWidget*>(twTaskList->cellWidget(row, 1));
                if (pSenderDataPreprocessTaskWidget == pDataPreprocessTaskWidget)
                {
                    //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    twTaskList->selectRow(row);
                    swTaskSettings->setCurrentIndex(row);

//                    twTaskList->setCurrentCell(row, 1);

                    //std::vector<AI3D::CORE::BlockObject::DataPreprocessOption> dataPreprocessOptions;

                    AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption;
//                    AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption2;
//                    AI3D::CORE::BlockObject::DataPreprocessOption dataPreprocessOption3;

                    int iTaskId = row / 2;
                    dataPreprocessOption = dataPreprocessOptions.at(iTaskId);
                    //dataPreprocessOption2 = dataPreprocessOptions.at(iTaskId);
                    //dataPreprocessOption3 = dataPreprocessOptions.at(iTaskId);

                    //std::vector<AI3D::CORE::BlockObject::DataPreprocessOption> dataPreprocessOptions;

                    if (row % 2 == 0)
                    {
                        QString photoDir = pDataPreprocessTaskWidget->getFilename();
                        DataPreprocessPhotoSetting* pPhotoWidget = static_cast<DataPreprocessPhotoSetting*>(swTaskSettings->currentWidget());
                        if (pPhotoWidget)
                            pPhotoWidget->setPhotoDir(photoDir);

                        dataPreprocessOption.photoDir = qstr2str(photoDir);
                        twTaskList->item(row, 2)->setText("Ready");


                    }
                    else
                    {
                        QString posFile = pDataPreprocessTaskWidget->getFilename();
                        DataPreprocessPosSetting* pPosWidget = static_cast<DataPreprocessPosSetting*>(swTaskSettings->currentWidget());
                        if (pPosWidget)
                            pPosWidget->setPosFile(posFile);

                        dataPreprocessOption.posFile = qstr2str(posFile);
                    }                   

                    //std::cout << "ChooseNewFile:" << dataPreprocessOption.photoDir << " " << dataPreprocessOption.posFile << std::endl;

                    //dataPreprocessOption2 = dataPreprocessOptions.at(iTaskId);

                    //std::cout << "ChooseNewFile2:" << dataPreprocessOption2.photoDir << " " << dataPreprocessOption2.posFile << std::endl;

                    dataPreprocessOptions.at(iTaskId) = dataPreprocessOption;

                    //dataPreprocessOption3 = dataPreprocessOptions.at(iTaskId);

                    //std::cout << "ChooseNewFile3:" << dataPreprocessOption3.photoDir << " " << dataPreprocessOption3.posFile << std::endl;

                    break;
                }
            }

        }

        void DataPreProcess::Slot_TaskItemClicked(QTableWidgetItem *item)
        {
            int row = item->row();
            int col = item->column();

            QString infoItemClicked = QString("row:%1 col:%2").arg(row).arg(col);
            //QMessageBox::information(nullptr,"tw info",infoItemClicked);

            swTaskSettings->setCurrentIndex(row);

            DataPreprocessTaskWidget* pDataPreprocessTaskWidget = static_cast<DataPreprocessTaskWidget*>(twTaskList->cellWidget(row, 1));

            if (row % 2 == 0)
            {
                    QString photoDir = pDataPreprocessTaskWidget->getFilename();
                    DataPreprocessPhotoSetting* pPhotoWidget = static_cast<DataPreprocessPhotoSetting*>(swTaskSettings->currentWidget());
                    if (pPhotoWidget)
                        pPhotoWidget->setPhotoDir(photoDir);
            }
            else
            {
                    QString posFile = pDataPreprocessTaskWidget->getFilename();
                    DataPreprocessPosSetting* pPosWidget = static_cast<DataPreprocessPosSetting*>(swTaskSettings->currentWidget());
                    if (pPosWidget)
                        pPosWidget->setPosFile(posFile);

            }

        }

        DataPreprocessTaskWidget::DataPreprocessTaskWidget(QWidget* parent,bool bChooseDir)
            : QWidget(parent)
        {
            this->bChooseDir = bChooseDir;

            QHBoxLayout* hlLayout = new QHBoxLayout();

            leDir = new QLineEdit(this);
            leDir->setReadOnly(true);
            leDir->setStyleSheet("background-color:white;color:black;font: 14px \"Arial\";");

            butDir = new QPushButton(this);
            butDir->setText("...");
            //butDir->setContentsMargins(0, 0, 0, 0);
            butDir->setFixedWidth(26);
            butDir->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");

            hlLayout->addWidget(leDir, 1);
            hlLayout->addWidget(butDir, 0);

            hlLayout->setContentsMargins(3, 3, 3, 3);
            hlLayout->setSpacing(3);
            hlLayout->setMargin(0);

            connect(butDir, &QPushButton::clicked, this, &DataPreprocessTaskWidget::Slot_ChooseDir);

            setLayout(hlLayout);
        }

        QString DataPreprocessTaskWidget::getFilename()
        {
            if (!leDir)
                return "";

            return leDir->text();
        }

        // add:file or dir.
        void DataPreprocessTaskWidget::Slot_ChooseDir()
        {
            if (bChooseDir)
            {
                QString strSelectedDir = QFileDialog::getExistingDirectory(nullptr, "Choose Photo Directory", "./", QFileDialog::ShowDirsOnly);
                if (strSelectedDir.isEmpty())
                    return;

                leDir->setText(strSelectedDir);
        //        std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                emit chooseNewFile(strSelectedDir);
            }
            else
            {
                //QString oldStr = QFileInfo(oldFileName).absolutePath();
                QFileDialog fd(nullptr, tr("Choose Pos File"), "./", tr("pos file(*.txt)"));
                fd.setAcceptMode(QFileDialog::AcceptOpen);
                fd.setFileMode(QFileDialog::ExistingFile);
                fd.setViewMode(QFileDialog::Detail);

                if (QDialog::Accepted != fd.exec())
                {
                    return;
                }

                QString posFile_path = fd.selectedFiles().first();
                if (!posFile_path.isEmpty())
                {
                    leDir->setText(posFile_path);
          //          std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    emit chooseNewFile(posFile_path);
                }
            }
        }

        void DataPreprocessTaskWidget::Slot_Dummy(int dummy)
        {

        }

        QString PhotoSettingNames[] =
        {
            //"Centre focallength",
            "Nadir focallength(mm)",
            "Oblique focallength(mm)",
            "Name prefix",
            "Photo folder name",
            "Name length",
            "Name start No.",
            "Rename preview",
            //"Export file name(.xls)",
            "Export file name(.xml)",
        };

        QString PosDataColumnNames[] =
        {
            "Name",
            "Longitude",
            "Latitude",
            "Height",
            "Yaw",
            "Pitch",
            "Roll",
            "Ignore",
        };

        DataPreprocessPhotoSetting::DataPreprocessPhotoSetting(QWidget* parent)
            : QWidget(parent)
        {
            //setStyleSheet("background-color:yellow;");
            setStyleSheet("background-color:white;");
            ///setStyleSheet("background-color:rgb(90,90,90);");
            QVBoxLayout* vlTop = new QVBoxLayout();

            //gbMain = new QGroupBox("Photo Setting");

            //vlTop->addWidget(gbMain,1);
            vlTop->setContentsMargins(10, 10, 10, 10);
            vlTop->setMargin(0);
#if 0
            QHBoxLayout* hlTitle = new QHBoxLayout();
            hlTitle->setContentsMargins(10, 0, 30, 0);
            hlTitle->setMargin(0);

            lblSettingTitle = new QLabel(this);
            lblSettingTitle->setText("Preprocessing rules setting");
            lblSettingTitle->setFixedHeight(35);
            lblSettingTitle->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            lblSettingTitle->setStyleSheet("background-color:rgb(157,150,150);color:white;padding:0px;margin:0px;");
            lblSettingTitle->setContentsMargins(0, 0, 0, 0);
            lblSettingTitle->setMargin(0);
            //lblSettingTitle->setContentsMargins(0, 0, 0, 0);
            //lblSettingTitle->setMargin(0);
            hlTitle->addWidget(lblSettingTitle);            

            vlTop->addLayout(hlTitle);
#endif

            QHBoxLayout* hlLayout[8];

            vlTop->addStretch(1);

            for (int i = 0; i < PHOTO_SETTING_NUM; i++)
            {
                hlLayout[i] = new QHBoxLayout();
                hlLayout[i]->setContentsMargins(0, 0, 20, 0);
                //hlLayout[i]->setMargin(0);

                vlTop->addLayout(hlLayout[i],1);
                //vlTop->addLayout(hlLayout[i]);

                lblSettingItemTitle[i] = new QLabel(PhotoSettingNames[i],this);
///                lblSettingItemTitle[i]->setStyleSheet("background-color:rgb(90,90,90);color:white;");
                lblSettingItemTitle[i]->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
                //lblSettingItemTitle[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                lblSettingItemTitle[i]->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

                leSettingItem[i] = new QLineEdit(this);
                leSettingItem[i]->setAlignment(Qt::AlignRight);
///                leSettingItem[i]->setStyleSheet("background-color:white;color:black;");

                //leSettingItem[i]->setStyleSheet("background-color:green;");
                //leSettingItem[i]->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

                if (i < 2)
                {
                    ///    leSettingItem[i]->setInputMask("xxxxxxx mm");
                        QRegExp regexp("((\\d)*(\\.)?(\\d){0,3})");
                        QRegExpValidator *regexpValidator = new QRegExpValidator(regexp);
                        leSettingItem[i]->setValidator(regexpValidator);
                    //leSettingItem[i]->setInputMask("dd9.000 mm;_");
                        if (i == 0)
                        {
                        //    leSettingItem[i]->setInputMask("d25.000 mm;0");
                            leSettingItem[i]->setText("25.000");
                        }
                        else
                        {
                        //    leSettingItem[i]->setInputMask("d35.000 mm;0");
                            leSettingItem[i]->setText("35.000");
                        }
                }
                else if (i == 4)
                {
                    QRegExp regexp("([1-4])");
                    QRegExpValidator* regexpValidator = new QRegExpValidator(regexp);
                    leSettingItem[i]->setValidator(regexpValidator);
                    leSettingItem[i]->setText("4");
                    connect(leSettingItem[i], &QLineEdit::textEdited, this, &DataPreprocessPhotoSetting::Slot_RenamePreview);
                }
                else if (i == 5)
                {
                    ///QRegExp regexp("(([1-9](\\d)+)|(\\d))");
                    //QRegExp regexp("(([1-9](\\d){1,3})|[1-9]");
                    QRegExp regexp("([1-9](\\d){0,3})");
                    QRegExpValidator* regexpValidator = new QRegExpValidator(regexp);
                    leSettingItem[i]->setValidator(regexpValidator);
                    leSettingItem[i]->setText("1");
                    connect(leSettingItem[i], &QLineEdit::textEdited, this, &DataPreprocessPhotoSetting::Slot_RenamePreview);
                }
                else if (i == 3 || i == 6)
                {
                    leSettingItem[i]->setReadOnly(true);

///                    leSettingItem[i]->setStyleSheet("background-color:rgb(192,192,192);color:white;border:none;");
                    if (i == 3)
                    {
                        ; // leSettingItem[i]->setPlaceholderText("0,1,2,3,4");
                        leSettingItem[i]->setStyleSheet("background-color:rgb(123,116,116);color:white;");
                    }
                    else
                    {
                        ; // leSettingItem[i]->setText("BD01001");
                        leSettingItem[i]->setStyleSheet("background-color:rgb(101,101,101);color:white;border:none;");
                    }
                }
                else if (i == 2)
                {
                    leSettingItem[i]->setText("BD01");
                    connect(leSettingItem[i], &QLineEdit::textEdited, this, &DataPreprocessPhotoSetting::Slot_RenamePreview);
                }
                else
                    ; // leSettingItem[i]->setPlaceholderText("Input Item " + QString::number(i));
                //hlLayout[i]->addWidget(lblSettingItemTitle[i],2,Qt::AlignRight);
                //hlLayout[i]->addWidget(leSettingItem[i],3,Qt::AlignLeft);

                hlLayout[i]->addWidget(lblSettingItemTitle[i], 1); // , Qt::AlignCenter);
                hlLayout[i]->addWidget(leSettingItem[i], 2); // , Qt::AlignCenter);
            }            

            vlTop->addStretch(1);

            setLayout(vlTop);
        }

        void DataPreprocessPhotoSetting::setPhotoDir(QString photoDir)
        {
            QStringList dirList;

            this->photoDir = photoDir;

            QDir dir(photoDir);
            
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << qstr2str(photoDir) << std::endl;

            if (!photoDir.isEmpty() && dir.exists())
            {
                dir.setFilter(QDir::Dirs);
                //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << qstr2str(photoDir) << std::endl;
                foreach(QFileInfo fullDir, dir.entryInfoList())
                {
                    if (fullDir.fileName() == "." || fullDir.fileName() == "..")
                    {
                        continue;
                    }

                    dirList << fullDir.fileName();
                    //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << qstr2str(photoDir) << std::endl;
                }
            }

            QString dirStr = dirList.join(",");
            //std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << qstr2str(photoDir) << " " << qstr2str(dirStr) << std::endl;
            leSettingItem[3]->setText(dirStr);


            Slot_RenamePreview();
        }

        void DataPreprocessPhotoSetting::saveOption(AI3D::CORE::BlockObject::DataPreprocessOption& option)
        {
            if (option.photoDir.empty())
                return;


            QString strRadirFocalLength = leSettingItem[0]->text();
            QString strObliqueFocalLength = leSettingItem[1]->text();

            QString strNamePrefix = leSettingItem[2]->text();
            QString strPhotoFolderName = leSettingItem[3]->text();

            QString strNameLength = leSettingItem[4]->text();
            QString strNameStartNo = leSettingItem[5]->text();

            QString strExportFilename = leSettingItem[7]->text();


            bool bOk = false;
            double dRadirFocalLength = strRadirFocalLength.toDouble(&bOk);
            if (!bOk)
                dRadirFocalLength = 25.0;

            bOk = false;
            double dObliqueFocalLength = strObliqueFocalLength.toDouble(&bOk);
            if (!bOk)
                dObliqueFocalLength = 35.0;

            option.centreFocalLength = dRadirFocalLength;
            option.obliqueFocalLength = dObliqueFocalLength;

            option.namePrefix = qstr2str(strNamePrefix);
            if (option.namePrefix.empty())
                option.namePrefix = "BD01";

            option.photoFolderNames.clear();

            if (!strPhotoFolderName.isEmpty())
            {
                if (!strPhotoFolderName.contains(","))
                    option.photoFolderNames.push_back(qstr2str(strPhotoFolderName));
                else
                {
                    QStringList photoFolderNameList = strPhotoFolderName.split(",");
                    for (QString str : photoFolderNameList)
                    {
                        option.photoFolderNames.push_back(qstr2str(str));
                    }
                }
            }

            int iNameLength = 4;
            iNameLength = strNameLength.toInt();
            if (iNameLength < 1 || iNameLength > 4)
                iNameLength = 4;

            int iNameStartNo = 1;
            iNameStartNo = strNameStartNo.toInt();
            if (iNameStartNo < 1 || iNameStartNo > 999)
                iNameStartNo = 1;

            option.nameLength = iNameLength;
            option.nameStartNo = iNameStartNo;

            if (strExportFilename.isEmpty())
                ;
            else
            {
                if (!strExportFilename.contains("."))
                    strExportFilename += ".xml";

                option.exportFileName = qstr2str(strExportFilename);
            }
        }

        void DataPreprocessPhotoSetting::Slot_Dummy(int dummy)
        {

        }

        void DataPreprocessPhotoSetting::Slot_RenamePreview()
        {
            // 2,4,5
            QString strNamePrefix = leSettingItem[2]->text();
            QString strPhotoFolderName = leSettingItem[3]->text();
            QString strNameLength = leSettingItem[4]->text();
            QString strNameStartNo = leSettingItem[5]->text();

            //if (strNamePrefix.isEmpty() || strPhotoFolderName.isEmpty() || strNameLength.isEmpty() || strNameStartNo.isEmpty())
            //    return;
            if (strPhotoFolderName.isEmpty())
                return;
              
            if (strNamePrefix.isEmpty())
            {
                strNamePrefix = "BD01";
                leSettingItem[2]->setText(strNamePrefix);
            }

            if (strPhotoFolderName.contains(","))
            {
                QStringList photoFolderNameList = strPhotoFolderName.split(",");
                for (QString str : photoFolderNameList)
                {
                    strPhotoFolderName = str;
                    break;
                }
            }

            int iNameLength = 4;
            iNameLength = strNameLength.toInt();
            if (iNameLength < 1 || iNameLength > 4)
                iNameLength = 4;

            int iNameStartNo = 1;
            iNameStartNo = strNameStartNo.toInt();
            if (iNameStartNo < 1 || iNameStartNo > 999)
                iNameStartNo = 1;

            QString strRenamePreview = QString("%1%2%3").arg(strNamePrefix).arg(strPhotoFolderName).arg(iNameStartNo,iNameLength,10,QLatin1Char('0'));
            leSettingItem[6]->setText(strRenamePreview);
        }

        DataPreprocessPosSetting::DataPreprocessPosSetting(QWidget* parent)
            : QWidget(parent)
        {
            ///this->setAttribute(Qt::WA_NoSystemBackground, false);
            setStyleSheet("background-color:white;");
            ///setStyleSheet("background-color:rgb(90,90,90);");
            QVBoxLayout* vlTop = new QVBoxLayout();
            
            //gbMain = new QGroupBox("Pos Setting");
            //vlTop->addWidget(gbMain, 1);
            vlTop->setContentsMargins(10, 10, 10, 10);

#if 0
            QHBoxLayout* hlTitle = new QHBoxLayout();
            hlTitle->setContentsMargins(0, 0, 0, 0);
            hlTitle->setMargin(0);

            lblSettingTitle = new QLabel(this);
            lblSettingTitle->setText("Preprocessing rules setting");
            //lblSettingTitle->setMargin(0);
            //lblSettingTitle->setContentsMargins(0, 0, 0, 0);
            lblSettingTitle->setFixedHeight(35);
            lblSettingTitle->setStyleSheet("background-color:rgb(157,150,150);color:white;");
            lblSettingTitle->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            hlTitle->addWidget(lblSettingTitle);

            vlTop->addLayout(hlTitle);
#endif

            QHBoxLayout* hlSRS = new QHBoxLayout();
            lblPosSRS = new QLabel(this);
            lblPosSRS->setText("SRS");
///            lblPosSRS->setStyleSheet("color:white;");
            lblPosSRS->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            lblPosSRS->setFixedWidth(120);

            cbbSRS = new QComboBox(this);
            
            //cbbSRS->addItem("SRS 1");
            //cbbSRS->addItem("SRS 2");
            //cbbSRS->addItem("SRS 3");
            InitSRS(cbbSRS);

            hlSRS->addWidget(lblPosSRS);
            hlSRS->addWidget(cbbSRS);
            hlSRS->addStretch(1);
            
            QHBoxLayout* hlDelimeter = new QHBoxLayout();
            lblPosFileFormat = new QLabel(this);
            lblPosFileFormat->setText("Pos File Format");
///            lblPosFileFormat->setStyleSheet("color:white;");
            lblPosFileFormat->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            lblPosFileFormat->setFixedWidth(120);

            cbSpace = new QCheckBox(this);
            cbSpace->setText("Space [ ]");
///            cbSpace->setStyleSheet("color:white;");
            cbSpace->setChecked(true);
            cbSpace->setEnabled(false);
            cbSpace->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");

            cbTab = new QCheckBox(this);
            cbTab->setText("Tab [   ]");
 ///           cbTab->setStyleSheet("color:white;");
            cbTab->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            cbTab->setChecked(true);
            cbTab->setEnabled(false);

            cbComma = new QCheckBox(this);
            cbComma->setText("Comma [,]");
 ///           cbComma->setStyleSheet("color:white;");
            cbComma->setEnabled(false);
            cbComma->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");

            hlDelimeter->addWidget(lblPosFileFormat);
            hlDelimeter->addWidget(cbSpace);
            hlDelimeter->addWidget(cbTab);
            hlDelimeter->addWidget(cbComma);
            //hlDelimeter->addStretch(1);

            QHBoxLayout* hlDelimeter2 = new QHBoxLayout();

            QLabel* lblPosFileFormat2 = new QLabel(this);
            lblPosFileFormat2->setText("    ");
            lblPosFileFormat2->setFixedWidth(120);
            lblPosFileFormat2->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
 ///           lblPosFileFormat2->setStyleSheet("color:rgb(90,90,90);");

            cbPoint = new QCheckBox(this);
            cbPoint->setText("Point [.]");
 ///           cbPoint->setStyleSheet("color:white;");
            cbPoint->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            cbPoint->setEnabled(false);

            cbSemiColon = new QCheckBox(this);
            cbSemiColon->setText("SemiColon [;]");
 ///           cbSemiColon->setStyleSheet("color:white;");
            cbSemiColon->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            cbSemiColon->setEnabled(false);

            cbColon = new QCheckBox(this);
            cbColon->setText("Colon [:]");
 ///           cbColon->setStyleSheet("color:white;");
            cbColon->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
            cbColon->setEnabled(false);

            hlDelimeter2->addWidget(lblPosFileFormat2);
            hlDelimeter2->addWidget(cbPoint);
            hlDelimeter2->addWidget(cbSemiColon);
            hlDelimeter2->addWidget(cbColon);
            //hlDelimeter2->addStretch(1);

            QHBoxLayout* hlPosDataPreview = new QHBoxLayout();
            lblPosDataPreview = new QLabel(this);
            lblPosDataPreview->setText("Pos data preview");
            lblPosDataPreview->setFixedWidth(120);
            lblPosDataPreview->setStyleSheet("background-color:rgb(101,101,101);color:white;font: 14px \"Arial\";");
 ///           lblPosDataPreview->setStyleSheet("color:white;");

            hlPosDataPreview->addWidget(lblPosDataPreview);
            hlPosDataPreview->addStretch(1);

            QHBoxLayout* hlPosImportHeader = new QHBoxLayout();
            hlPosImportHeader->setContentsMargins(0, 0, 0, 0);
            hlPosImportHeader->setMargin(0);
            hlPosImportHeader->setSpacing(0);
            for (int i = 0; i < 7; i++)
            {
                cbPosImportHead[i] = new QComboBox(this);
                cbPosImportHead[i]->setContentsMargins(0, 0, 0, 0);
                
                hlPosImportHeader->addWidget(cbPosImportHead[i], 1);
                
                for (int j = 0; j < 8; j++)
                {
                    cbPosImportHead[i]->addItem(PosDataColumnNames[j]);
                }

                cbPosImportHead[i]->setCurrentIndex(i);
            }

            twPosImport = new QTableWidget(this);

            QStringList slPosImport;
            slPosImport << "Name" << "Logitude" << "Latitude" << "Height" << "Yaw" << "Pitch" << "Roll";

            twPosImport->setColumnCount(7);
            twPosImport->setHorizontalHeaderLabels(slPosImport);
            twPosImport->horizontalHeader()->setStyleSheet("QHeaderView::section {background-color:white;color:black;font: 14px \"Arial\";}");
            twPosImport->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            twPosImport->horizontalHeader()->hide();
            twPosImport->verticalHeader()->hide();
            twPosImport->setSelectionBehavior(QAbstractItemView::SelectRows);

            //twPosImport->insertRow(twPosImport->rowCount());
            //twPosImport->insertRow(twPosImport->rowCount());

            vlTop->addLayout(hlSRS);
            vlTop->addLayout(hlDelimeter);
            vlTop->addLayout(hlDelimeter2);
            vlTop->addLayout(hlPosDataPreview);
            vlTop->addLayout(hlPosImportHeader);
            vlTop->addWidget(twPosImport,1);

            setLayout(vlTop);
        }

        void DataPreprocessPosSetting::setPosFile(QString posFile)
        {
            this->posFile = posFile;

            if (!posFile.isEmpty() && QFileInfo(posFile).exists())
            {
                QVector<QStringList> v_StringList = ImportPosDia::readPosFile(posFile);
                std::cout << qstr2str(posFile) << ":" << v_StringList.count() << std::endl;

                twPosImport->clear();

                int j = 0;
                for(QStringList sl : v_StringList)
                {
                    twPosImport->insertRow(twPosImport->rowCount());
                    for (int i = 0; i < sl.count() && i < twPosImport->columnCount(); i++)
                    {
                        if (j < 5)
                        {
                            //std::cout << j << " " << i << " " << qstr2str(const_cast<QString&>(sl.at(i))) << " slcount:" << sl.count() << std::endl;
                        }

                        twPosImport->setItem(j, i, new QTableWidgetItem(sl.at(i)));
                    }
                    j++;
                }
            }
            else
            {
                std::cout << qstr2str(posFile) << ":" << "null or doesn't exist." << std::endl;
            }
        }

        // dump all the options after getting all the input fields.
        void DataPreprocessPosSetting::saveOption(AI3D::CORE::BlockObject::DataPreprocessOption &option)
        {
            QString currentSRS = cbbSRS->currentText();
            int currentId = cbbSRS->currentIndex();
            int totalCount = cbbSRS->count();

            if (!currentSRS.isEmpty())
            {
                if (currentSRS == "Default")
                {
                    if (currentId + 1 < totalCount)
                        currentSRS = cbbSRS->itemText(currentId + 1);
                }
                else if (currentSRS == "Common")
                {
                    if (currentId + 1 < totalCount)
                        currentSRS = cbbSRS->itemText(currentId + 1);
                }
                else if (currentSRS == "More")
                {
                    if (currentId + 1 < totalCount)
                        currentSRS = cbbSRS->itemText(currentId + 1);
                }

                std::cout << "get srs item:" << qstr2str(currentSRS) << std::endl;
                option.SRS = qstr2str(currentSRS);
            }

            option.posFileFormat.clear();
            
//            QStringList fileFormatDelimeters;
            if (cbSpace->isChecked())
            {
//                fileFormatDelimeters << " ";
                option.posFileFormat.push_back(" ");
            }

            if (cbTab->isChecked())
            {
 //               fileFormatDelimeters << "\t";
                option.posFileFormat.push_back("\t");
            }

            if (cbComma->isChecked())
            {
 //               fileFormatDelimeters << ",";
                option.posFileFormat.push_back(",");
            }

            if (cbPoint->isChecked())
            {
 //               fileFormatDelimeters << ".";
                option.posFileFormat.push_back(".");
            }

            if (cbSemiColon->isChecked())
            {
 //               fileFormatDelimeters << ";";
                option.posFileFormat.push_back(";");
            }

            if (cbColon->isChecked())
            {
 //               fileFormatDelimeters << ":";
                option.posFileFormat.push_back(":");
            }

            for (int i = 0; i < 7; i++)
            {
                QString posDataTitleField = cbPosImportHead[i]->currentText();
                option.posDataFields.push_back(qstr2str(posDataTitleField));
            }
        }

        void DataPreprocessPosSetting::Slot_Dummy(int dummy)
        {

        }

        void DataPreprocessPosSetting::InitSRS(QComboBox* pComboBox)
        {
            if (!pComboBox)
                pComboBox->clear();
            QStringList slDefault_Coords;
            QStringList slRecent_Coords;
            QStringList slMore_Coords;
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                slDefault_Coords << "默认";
                slRecent_Coords << "最近";
                slMore_Coords << "更多";
                slMore_Coords << "空间参考系统数据库";
            }
            else
            {
                slDefault_Coords << "Default";
                slRecent_Coords << "Recent";
                slMore_Coords << "More";
                slMore_Coords << "Spatial reference system database";
            }
#ifdef USE_AI3D_PROJ

            //QStringList listCoords_default;
            /*listCoords_default << "Default";*/
            //参照cc,此处纯显示不加入库中；
            //@attention chy 把local加入 库中
            AI3D::PROJ::CoordinateReferenceSystem localcrs(std::string("Local:0"));
            AI3D::PROJ::CoordinateReferenceSystem wgscrs(std::string("EPSG:4326"));

            slDefault_Coords << QString::fromStdString(localcrs.GetAuthID()) << QString::fromStdString(wgscrs.GetAuthID());
            //QStringList listCoords_Recent;
            //listCoords_Recent << "Recent";
            auto lists = AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->GetRecentCrs();

            QList< AI3D::PROJ::CoordinateReferenceSystem> filteredcrs;
            for (auto iter : lists)
            {
                AI3D::PROJ::CoordinateReferenceSystem crs(iter.GetAuthID());
                if (crs == localcrs || crs == wgscrs)
                {
                }
                else
                {
                    filteredcrs << iter;
                }
            }
            int count = 0;
            std::cout << lists.size() << std::endl;;
            for (auto iter : filteredcrs)
            {

                slRecent_Coords << iter.description();
                if (count == 7)
                {
                    break;
                }
                count++;
            }



#else

            auto src_map = AI3D::CORE::CoordinateTransformer::CSG_coordinateSystem_Global();
            for (auto it = src_map.begin(); it != src_map.end(); it++)
            {
                if (it->first == "Default")
                {
                    for (auto itsrcname : it->second)
                    {
                        slDefault_Coords << str2qstr(itsrcname.name);
                    }
                }
                else if (it->first == "Common")
                {
                    for (auto itsrcname : it->second)
                    {
                        slRecent_Coords << str2qstr(itsrcname.name);
                    }
                }
                else if (it->first == "More")
                {
                    for (auto itsrcname : it->second)
                    {
                        slMore_Coords << str2qstr(itsrcname.name);
                    }
                }
            }
#endif
            pComboBox->setEditable(false);
            pComboBox->addItems(slDefault_Coords);
            pComboBox->addItems(slRecent_Coords);
            pComboBox->addItems(slMore_Coords);

            if(pComboBox->count() > 0)
                pComboBox->setCurrentIndex(1);
        }
    }
}
