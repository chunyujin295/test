/**
  * @file      Reconstration.h
  * @brief     Mohacker主界面类
  * @details
  * @author    李跃
  * @attention
  */
#ifndef _AI3D_GUI_MOHACKERWGT_H_
#define _AI3D_GUI_MOHACKERWGT_H_
#include <QMainWindow>
#include <QTreeView>
#include <QMenuBar>
#include <QToolBar> 
#include <QStackedWidget>
#include <QSplitter>
#include <QAction>
#include <QMessageBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QMap>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QCollator>
#include <QFileInfo>
#include <QFileDialog>
#include <QFutureWatcher>
#include <Constants.h>
#include <QSortFilterProxyModel>
#include "Core/BlockObject.h"
#include "Gui/ProjectInfoWgt.h"
#include "Gui/BlockWgt.h"
#include "Core/Types.h"
//#include "Gui/Network.h"
#include "Gui/ExportXmlDia.h"
#include "Gui/ExportATColmapDia.h"
#include "Gui/ExportRecColmapDia.h"
#include "ProgressBarCom.h"
#include <QWidgetAction>
#include <QGroupBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QStackedWidget>
#include <QLabel>
#include <QTableWidget>
#include "Util/DeviceInfo.h"
#include "Gui/LoginDialog.h"

//blk如果是在单击时加载，则信息记录需要

namespace AI3D
{
    namespace GUI
    {

        class CSortFilterProxyModel;
        class CTreeView;
        class ImportXml;
        class GUI_API MohackerWin :public QMainWindow
        {
            Q_OBJECT
        public:
            enum ProjectAction_e
            {
                PANew = 0,
                PAOpen,
                PAClose
            };
            enum SaveReturn
            {
                NotNeedSave = 0,
                CancelSave,
                ErrorOccurred,
                Saved
            };
        private:
            MohackerWin();
            ~MohackerWin();
            MohackerWin(const MohackerWin& instance);
            const MohackerWin& operator = (const MohackerWin& instance);

        public:
            static MohackerWin* GetInstance();
            /*{
                static MohackerWin* instance = nullptr;
                if (!instance)
                {
                    instance = new MohackerWin;

                }
                return instance;
            }*/
            void CreateItemModel();

            //bool MohackerWin::event(QEvent* event);
            QStandardItem* NewBlock(AI3D::CORE::BlockObject* block, int index);
            QStandardItem* GetBlockATData(AI3D::CORE::BlockObject* block);
            void SetWindowTitle();

            bool eventFilter(QObject* obj, QEvent* event);
            // 返回值原为void，后改为bool，因加入lock机制，一获取到工程文件名即刻加锁，若打开工程失败则解锁，其余只有关闭工程时才解锁
            bool LoadProject(QString projName);
            void UpdateTreeView();
            void ClearCurrentProject();
            void ClearCurrentProject4LoadProject();
            void SetFileModifiedXml();
            void SetFileModifiedProj();
            void Make_AT_Block(AI3D::CORE::BlockObject* block);
           
            bool HasProject();
            bool ShowBlockWidget(AI3D::CORE::BlockObject* block, QStandardItem* blockItem, bool isInportblock = false);
            bool CheckProjectIsModifyDlg();
            std::set<block_t> GetMultiSelectedBlocks();

            QString GetMasterDir(); // { return file_job_master_dir_; };
            QString GetEngineDir(); // { return file_job_engine_dir_; };

            void BeforeDeleteOneBlock(QModelIndex& index);
            void DeleteOneBlockWidget(QWidget *pWidget);
            void DeleteOneBlockData(AI3D::CORE::BlockObject *blockData);
            void DeleteOneBlock(QModelIndex& index);
            void RemoveWidgetsUnderCurrentBlock(QStandardItem* pBlockItem);
            void RemoveWidgetsUnderCurrentReconstruction(QStandardItem* pBlockItem);
            void CalcBlockStatus();
            void CalcBlockProductionStatus(QStandardItem* pBlockItem);
            void RefreshBlockStatus();
            void ProcessBlockStatus();
            bool IsProjectDirty() { return bProjectDirty;  }
            void TestProjSRS();
            static QString prependIndentation();
            static QString stripPrependIndentation(QString str);
            static QString localENUPrefix();
            static QString localSRS();
            static int disableLevel4ReconstructionPerformanceTest();

        signals:
            void CloseFirstWgt();
            void Signal_Process(int);

        public slots:

            void Slot_Action_OpenEngine();
            void Slot_Action_NewProject();
            void Slot_Action_OpenProject();
            void Slot_Action_SaveProject();
           
            void Slot_Action_SimplifyBlock();
            void Slot_Action_CopyGCPsFromBlock2Block();
            void SaveProject_Wait();
            void Slot_Action_Merge_And_Ajust_blocks();
            void Slot_Action_NewBlock();
            void Slot_Action_ImportBlocks();
            void Slot_Action_ExportToXml();
            void Slot_Action_ExportATToColmap();
            void Slot_Action_ExportRecToColmap();
            void Slot_Action_ImportMeasurementFromXml();
            void Slot_Action_ExportMeasurementToXml();
            void Slot_Action_RenameProject();
            void Slot_Action_RenameBlock();
            //新的通信机制需要更改
            void Slot_Action_DeleteBlock2();
            void Slot_Action_OpenFolder();

            void Slot_Action_Merge_blocks();
            void Slot_Action_UserManual();
            void Slot_Action_About();
            void Slot_Action_DeleteMore();

            //blcok切换
            void Slot_ItemDataChanged(QStandardItem* item);

            //treeview item event
            //单击block
            void TreeViewClicked(const QModelIndex& currentIndex, const QModelIndex& previousIndex);
            void Slot_ProjectTreeView_ItemClicked(const QModelIndex& index);
            //void Slot_ProjectTreeView_ItemChanged(const QModelIndex &currentIndex, const QModelIndex& previous);
            void Slot_ProjectTreeView_ItemDoubleClicked(const QModelIndex& index);
            //treeview's right click menu
            void Slot_TreeView_CustomContextMenuRequested(const QPoint& pos);
            
            void SaveFinished();
           
            

            void SaveXmlFile();
            void SaveXmlFinished();

            void TransferATColmap();
            void TransferRecColmap();
            void TransferATColmapFinished(int result);
            void TransferRecColmapFinished(int result);

            void Set3DViewProgressValue(int num,QString str);
            void FinishImportXML(QString file, bool PasrseSucccess);
            void FinishImportXML2(QString file, bool PasrseSucccess);
            //用于切换分布式和双sever

            void Slot_Action_Settings();
            void Slot_Action_ViewEngineNode();
            void Slot_Action_DataPreProcess();
            void Slot_Action_DataPreProcessRight();
            void Slot_GeneratedXml(std::string xml);
            void Slot_GeneratedXmls(int);

            void Slot_SettingsChanged();
            void Slot_CheckVersion();
            void UpdateBlockStatus();
            void UpdateBlockProductionStatus(QStandardItem* pBlockItem);

            void Slot_CloneBlock();
            void Slot_NewConstruction(AI3D::CORE::BlockObject* block, reconstruction_t reconstructionId);
            void Slot_NewProductionStarted(AI3D::CORE::BlockObject*, reconstruction_t, QStandardItem*);
            void Slot_NewProduction(AI3D::CORE::BlockObject*, reconstruction_t, production_t,QStandardItem*);
            void Slot_ProjModifed();
            void Slot_TreeItemChanged(QStandardItem* item);
            void SetProjectDirty(bool bDirty);

            void Slot_Action_Login();
            void Slot_Action_Logout();
            void Slot_Action_Info();

            void Slot_quit();

        private:                  
            void InitWgt();
            void CreateWgt();
            void CreateActions();
            void CreateConnections();
           // void InitServer();
           
            bool MaybeSave();

            int ShowMessageBox();

            void SaveThisToRecent();

            void RefreshRecentOpenMenu();

            void ProcessMessageBoxResult(int ret, ProjectAction_e action);
            //新建工程时产生的block是自动保存的，剩下的全为手动保存
            bool isnewproject = false;
        protected:
            void closeEvent(QCloseEvent* event);

        private:
            QMenuBar* ui_menuBar;
            QToolBar* ui_toolBar;

            QMenu* ui_menu_project;
            QAction* ui_action_newProject;
            QAction* ui_action_open;

            QMenu* ui_menu_recentOpen;
            QAction* ui_action_save;
            QAction* ui_action_quit;

            QMenu* ui_menu_block;
            QAction* ui_action_newBlock;
            QAction* ui_action_importBlocks;
            QAction* ui_action_exportToXml;
            QAction* ui_action_importGcpMeasurements;
            QAction* ui_action_exportGcpMeasurements;
            

            QMenu* ui_menu_engine;
            QAction* ui_action_engine;

            QMenu* ui_menu_settings;
            QAction* ui_action_set;
            QAction* ui_action_viewEngineNode;

            QMenu* ui_menu_tools;
            QAction* ui_action_dataPreprocess;

            QMenu* ui_menu_help;
            QAction* ui_action_about;
            QAction* ui_action_usermanual;
            QAction* ui_action_at_formatTransfer;
            QAction* ui_action_rec_formatTransfer;
            //right click menu on treeview's project item
            QMenu* ui_menu_rightClick_project;

            //right click menu on treeview's block item
            QMenu* ui_menu_rightClick_block;
            QMenu* ui_menu_rightClick_selectRows;
            QMenu* ui_menu_rightClick_Reconstruction;
            QMenu* ui_menu_rightClick_Production;

            QAction* ui_action_clone_block;
            QAction* ui_action_data_preprocess;

            //both above two right click menu will use the follow two action
            QAction* ui_action_delete;
            QAction* ui_action_rename;
            QAction* ui_action_openFileInExplorer;// 打开工程或block文件所在的路径
            QAction* ui_action_merge_blocks;// del
            QAction* ui_action_merge_and_adjust_blocks;
          //  QAction* ui_action_simplify_block;//抽稀;
            //将后选中的Block中的gcp拷贝到先选中的，主要解决当相同影像在不同软件中刺点时，因为影像id不同，导致GCP刺点结果无法复用;
            QAction* ui_action_copy_gcpmeasurements; 
            // delete more blocks when right-clicking more than one block similarly.
            QAction* ui_action_delete_more; 
            //treeview
            QTreeView* ui_treeView_project;

            //splitter
            QSplitter* ui_splitter_main;
            ProjectInfoWgt* ui_projectWgt_;
            QStackedWidget* ui_stackedWidget;
           // cServer* _server;
            ExportXmlDia* xmldia;
            ExportATColmapDia* atdia;
            ExportRecColmapDia* recdia;

            QMenu* ui_menu_user;
            QAction* ui_action_login;
            QAction* ui_action_info;
            QAction* ui_action_logout;
        private:
            QStandardItemModel* _itemmodel_;
            QStringList _vec_RecentOpens_; //recentopen.xml
            QString _currentSolutionXmlPath_; // .tri
            QMap<QString, QIcon> _map_icon_;
            QMap<QString, Qt::GlobalColor> _map_color_;
            CSortFilterProxyModel* _proxy_;
            QStandardItem* project_root_item_;
            //std::shared_ptr<ProjectObject> project ;
            QModelIndex currentIndex_;
            BlockWgt* ui_blockwidget_;
            savetype_e savetype_;
            QFutureWatcher<int> watcher;
            int processNum;//当前正在执行的.exe的数量
            QStandardItem* treeitem_;
            //QProgressBar* m_pProgressBar;
            ProgressCom* m_pProgressBar;
            QString _currentSolutionXmlPath; // .tri
            QMap<QStandardItem*, jobsta_e> m_mapItemStatusOfProjectTree;
            QMap<QStandardItem*, jobsta_e> m_mapBlockItemsToBeRefreshed;
            QMap<QStandardItem*, jobsta_e> m_mapProductionItemsToBeRefreshed;
            bool bProjectItemsStatusQuerying = false;
            bool bProjectItemsStatusGot = false;
            bool bProjectDirty = false;

        private:
            static MohackerWin* instance;
            static std::once_flag oc_;
            ImportXml* importxml_;
            QModelIndex lastindex_= QModelIndex();//上次单击的那个
            QTimer* pCheckVersionTimer;
            bool bCtrlPressed = false;
            FILE* fpprojectlock = nullptr;
            QTimer* GetRunningInfoTime = nullptr;

        public:
            float persentModel;// 进度

            QWidgetAction* ui_action_about2;
        };

        class CSortFilterProxyModel : public QSortFilterProxyModel
        {
            Q_OBJECT
        public:
            CSortFilterProxyModel(QObject* parent = 0);

        protected:
            virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
        };
        

        
        class CTreeView:public QTreeView
        {
            Q_OBJECT
        public:
            CTreeView(QObject* parent = 0) {};
        signals:
            void Clicked(QModelIndex);
        private:
            void mousePressEvent(QMouseEvent* event)
            {
                QModelIndex index = currentIndex();
                // 如果是鼠标左键按下
                if (event->button() == Qt::LeftButton)
                {
                    emit Clicked(index);
                }
                // 如果是鼠标右键按下
                else if (event->button() == Qt::RightButton)
                {
                    emit Clicked(index);
                }
            }
        };
        class ImportXml :public QThread
        {
            Q_OBJECT
        public:
            ImportXml(QObject* parent = 0){};
            void run();
            void SetFileName(QString str) { filename_ = str; };
        signals:
            void FinishedRead(QString fileName,bool PasrseSucccess);
        private:
            QString filename_ = "";
        };

        struct DataPreprocessTaskData
        {
            QString strPhotoTitle; // not used.
            QString strPhotoDir;
            QString strPoseTitle; // not used.
            QString strPoseDir;
            int taskStatus;
        };

        class DataPreprocessTaskWidget : public QWidget
        {
            Q_OBJECT
        public:
            DataPreprocessTaskWidget(QWidget* parent = nullptr,bool bChooseDir = true);

            QString getFilename();

        public slots:
            void Slot_Dummy(int dummy);
            void Slot_ChooseDir();

        signals:
            void dummySignal(QString str, int val);
            void chooseNewFile(QString str);

        private:
            QLineEdit* leDir;
            QPushButton* butDir;
            bool bChooseDir;
        };

        class DataPreprocessPhotoSetting : public QWidget
        {
            Q_OBJECT
        public:
            DataPreprocessPhotoSetting(QWidget* parent = nullptr);

            void setPhotoDir(QString photoDir);
            void saveOption(AI3D::CORE::BlockObject::DataPreprocessOption& option);

        public slots:
            void Slot_Dummy(int dummy);
            void Slot_RenamePreview();

        signals:
            void dummySignal(QString str, int val);

        private:
            QLabel* lblSettingTitle;

            QGroupBox* gbMain;

            QLabel* lblCentreFocalLength;
            QLineEdit* leCentreFocalLength;
            QLabel* lblObliqueFocalLength;
            QLineEdit* leObliqueFocalLength;
            QLabel* lblNamePrefix;
            QLineEdit* leNamePrefix;
            QLabel* PhotoFolderName;
            QLineEdit* lePhotoFolderName;
            QLabel* lblNameLength;
            QLineEdit* leNameLength;
            QLabel* lblNameStartNo;
            QLineEdit* leNameStartNo;
            QLabel* lblRenamePreview;
            QLineEdit* leRenamePreview;
            QLabel* lblExportFileName;
            QLineEdit* leExportFileName;


            const static int PHOTO_SETTING_NUM = 8;
            QLabel* lblSettingItemTitle[PHOTO_SETTING_NUM];
            QLineEdit* leSettingItem[PHOTO_SETTING_NUM];
            QString photoDir;
        };

        class DataPreprocessPosSetting : public QWidget
        {
            Q_OBJECT
        public:
            DataPreprocessPosSetting(QWidget* parent = nullptr);

            void setPosFile(QString posFile);
            void saveOption(AI3D::CORE::BlockObject::DataPreprocessOption &option);

        public slots:
            void Slot_Dummy(int dummy);

        signals:
            void dummySignal(QString str, int val);

        private:
            void InitSRS(QComboBox *pComboBox);

            QLabel* lblSettingTitle;
            
            QGroupBox* gbMain;
            QLabel* lblPosSRS;
            QComboBox* cbbSRS;

            QLabel* lblPosFileFormat;
            QCheckBox* cbSpace;
            QCheckBox* cbTab;
            QCheckBox* cbComma;
            QCheckBox* cbPoint;
            QCheckBox* cbSemiColon;
            QCheckBox* cbColon;

            QLabel* lblPosDataPreview;

            QComboBox* cbPosImportHead[7];
            QTableWidget* twPosImport;

            QString posFile;
        };

        class DataPreProcess : public QWidget
        {
            Q_OBJECT
        public:
            DataPreProcess(QWidget* parent = 0,AI3D::CORE::BlockObject *pBlockObject = 0);

            void saveOption(int taskId);
            void dumpOption(int taskId);

            int ProgFunc(int taskId, int progress);

        public slots:
            void Slot_Apply();
            void Slot_Ok();
            void Slot_Cancel();
            void Slot_Close();
            void Slot_AddTask();
            void Slot_DelTask();
            void Slot_CancelTask();
            void Slot_ExportDirectory();
            void Slot_ChooseProcessCategory(bool);
            void Slot_TaskItemClicked(QTableWidgetItem* item);
            void Slot_ChooseNewFile(QString);

        signals:
            void dummySignal(QString str,int val);
            void generatedXml(std::string str);
            void generatedXmls(int);

        private:
            AI3D::CORE::BlockObject* pBlockObject;
            QPushButton* butApply;
            QPushButton* butOk;
            QPushButton* butCancel;            
            QPushButton* butClose;
            QGroupBox* gbCategory;
            QRadioButton* rbCategoryDataPreprocessWithAT;
            QRadioButton* rbCategoryDataPreprocess;

            QGroupBox* gbPhotoSettings;
            QGroupBox* gbPosSettings;
            QStackedWidget* swTaskSettings;

            QTextEdit* teTaskMainDummy;
            QTextEdit* teTaskMainDummy2;
            QPushButton* butTaskMainDummy2;

            DataPreprocessPhotoSetting* photoSetting;
            DataPreprocessPosSetting* posSetting;

            QTextEdit* teTaskExtraDummy;

            QGroupBox* gbTaskExtra;
            QLabel* lblExportDirectory;
            QLineEdit* leExportDirectory;
            QPushButton* butExportDirectory;

            QPushButton* butAddTask;
            QPushButton* butDelTask;
            QPushButton* butCancelTask;

            QTableWidget* twTaskList;

            QVector<DataPreprocessTaskData> dataPreprocessTaskData;

            std::vector<AI3D::CORE::BlockObject::DataPreprocessOption> dataPreprocessOptions;
        };
	}
}

#endif
