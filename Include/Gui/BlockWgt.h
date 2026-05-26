/**
  * @file      Reconstration.h
  * @brief     CBlockWgt block对应界面类
  * @details
  * @author    李跃
  * @attention
  */
#ifndef _AI3D_GUI_BLOCKWGT_H_
#define _AI3D_GUI_BLOCKWGT_H_

#include <QWidget>
#include <QStandardItemModel>
#include <QFutureWatcher>
#include<QDoubleValidator>

#include <QFileInfo>
#include <QStyledItemDelegate>
#include <QRegExpValidator>
#include <omp.h>
#include "ui_BlockWgt.h"
//#include "ui_BlockWgtCN2.h"
#include "ui_ConstructionWgt.h"
#include "ui_ProductionWgt.h"
#include "ui_ReConstructionWgt.h"
#include "OSGEditor/AT3DViewInterface.h"
#include "Core/BlockObject.h"
#include "Core/ATData.h"
#include "Core/Types.h"
#include "Gui/BlockManager.h"
#include "Gui/ImageScale.h"
#include "Gui/ControlPointsEditorWin.h"
#include "3DViewer/3dview.h"
#include "Gui/ProgressBarCom.h"
#include "Gui/GlobalStruct.h"
#include "Gui/CommonDelDia.h"
#include "Gui/ImportGcpDia.h"
#include "Gui/MoPhotoWidget.h"

#include "3DViewer/render_options.h"
#include "3DViewer/image_viewer_widget.h"
#include "Core/Types.h"
#include "OSGEditor/OsgEngine.h"
#include "OSGEditor/EventManager.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QReadWriteLock>
#include <QtConcurrent>
#include <QApplication>
#include <osg/ArgumentParser>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>   
#include <osgDB/ReadFile>
#include <QString>
#include <QTimer>
#include <QKeyEvent>

#include <QGLWidget>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QtGui>
using Qt::WindowFlags;
#include <iostream>

#include <QMainWindow>
#include <QMessageBox>
//#include "ui_MainWindow.h"
#include <OSGDB/ConvertUTF>
#include <OSGDB/WriteFile>
//#include "ViewerQT.h"
#include <osg/Point>
#include <osg/LineWidth>
#include <osgUtil/DelaunayTriangulator>
#include <osgGA/TerrainManipulator>
#include "Core/ReconstructionCommandSet.h"
#include "OSGEditor/AT3DViewInterface.h"
#include<QGridLayout>
#include<QProgressBar>

namespace AI3D
{
    namespace GUI
    {
        static const QString s_submitText = QObject::tr("Submit AeroTriangulation");

        static const QString s_reSubmitText = QObject::tr("ReSubmit");
        ///button cancel text
        static const QString s_cancelText = QObject::tr("Cancel");
        class ControlPointWins;

        struct JobStats
        {
            QString blockpath;
            int rtnstr;
            int rtnPercent;
            QString submittime;
            QString finishtime;
            QString rtnTotalTime;
            QString currentStageName;
            QVector<JobStage> vecStages;
            bool bGettingJobInfo = false;
            bool bGotNewJobInfo = false;
        };

        struct ProductionItemInfo
        {
            std::string name_;
            std::string jobFileName_;
            
            jobsta_e initJobStat_;
            jobsta_e lastJobStat_;
            //jobsta_e jobStat_;
            int lastProgress;
            //int currentProgress;

            std::string feedbackFile_;
            std::string Msg;
            std::string submitTime_;
            bool needRefresh_;
        };

        class QtVEditorDoubleValidator :public QDoubleValidator
        {
        public:
            explicit QtVEditorDoubleValidator(QObject* parent = Q_NULLPTR);
            QtVEditorDoubleValidator(double bottom, double top, int decimals, QObject* parent = Q_NULLPTR);
            QValidator::State validate(QString& str, int& i)const;
        };

        class UserMatrixData : QObjectUserData
        {
        public:
            UserMatrixData();
            ~UserMatrixData();

            void setCurrentMatrix(osg::Matrixd &matrixd);
            osg::Matrixd getLastMatrix(bool &lastMatrixIsValid);

        public:
            static void setCurrentMatrixObject(QObject* object, osg::Matrixd& matrixd);
            static osg::Matrixd getCurrentMatrixObject(QObject* object, bool& lastMatrixIsValid);

        private:
            bool bInitial;
            osg::Matrixd lastMatrix;
        };

        class AdapterWidget : public QOpenGLWidget
        {
        public:
            AdapterWidget(QWidget* parent = 0, const char* name = 0, const QOpenGLWidget* shareWidget = 0, WindowFlags f = 0);
            virtual ~AdapterWidget() {}
            osgViewer::GraphicsWindow* getGraphicsWindow() { return _gw.get(); }
            const osgViewer::GraphicsWindow* getGraphicsWindow() const { return _gw.get(); }
            void setKeyboardModifiers(QInputEvent* event);
        protected:

            void init();
            virtual void resizeGL(int width, int height);
            virtual void keyPressEvent(QKeyEvent* event);
            virtual void keyReleaseEvent(QKeyEvent* event);
            virtual void mousePressEvent(QMouseEvent* event);
            virtual void mouseReleaseEvent(QMouseEvent* event);
            virtual void mouseDoubleClickEvent(QMouseEvent* event);
            virtual void mouseMoveEvent(QMouseEvent* event);
            virtual void wheelEvent(QWheelEvent* event);
            virtual void initializeGL();

            osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> _gw;
        };

        //摄像机设置：
//视点，投影方法，图形窗口，

        class ViewerQT : public osgViewer::Viewer, public AdapterWidget
        {
        public: 
            ViewerQT(QWidget* parent = 0, const char* name = 0, const QOpenGLWidget* shareWidget = 0, WindowFlags f = 0, bool bUseLaterSize = false,
                int forceWidth = 0, int forceHeight = 0);                

            ~ViewerQT()
            {
                _timer.stop();
            }

            void showEvent(QShowEvent* event) 
            {
                AdapterWidget::showEvent(event);
                AdapterWidget::makeCurrent();
            }

            void hideEvent(QHideEvent* event)
            {
                AdapterWidget::hideEvent(event);
                AdapterWidget::doneCurrent();                
            }

            void pauseRefresh()
            {
                bPauseRefresh = true;
            }

            void resumeRefresh()
            {
                bPauseRefresh = false;
            }

            virtual void paintGL()
            {
//              std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " this:" << std::hex
//                  << std::showbase << this << std::dec << std::endl;
                if (bPauseRefresh)
                    return;

                // note:may need to check the done status of osgviewer.
                if (isVisible() && !done())
                {
                    frame();
                }
            }

            void refreshGL()
            {
                //updateGL();
                update();
            }

        protected:
            QLabel* butSave;
            //QPushButton* butCancel;
            QTimer _timer;
            bool bUseLaterSize = false;
            bool bPauseRefresh = false;
        };

        class CallbackEventTest;
        //class ProductionCallbackEvent;
        class TilingCallbackEvent;
        class MWindow : public QWidget
        {
            Q_OBJECT
        public:
            MWindow(QWidget* parent = 0, Qt::WindowFlags flags = 0, bool bInsideProduction = false, bool bInsideBlockAT = false, bool bInsideConstruction = false,
                bool bUseLaterSize = false, int forceWidth = 0,int forceHeight = 0);
            ~MWindow();

            enum viewmode_e {
                VIEWMODE_HOME = 0,
                VIEWMODE_TOP = 1,
                VIEWMODE_BOTTOM = 2,
                VIEWMODE_LEFT = 3,
                VIEWMODE_RIGHT = 4,
                VIEWMODE_FRONT = 5,
                VIEWMODE_BACK = 6,
                VIEWMODE_FRONTI = 7,
                VIEWMODE_BACKI = 8,
            };

            void InitData();
            //OsgEngine* getOsgEngine();
            void loadOsgFile(std::string& fileName);
            void send_update_overview(ReconstructionObject* object);
            
            void RenderReconstruction( ReconstructionObject* data,bool bSelectTiles = false);
            void RenderModel(std::string filenmae);

            void init(int w, int h);
            void RenderBlock(const ATData& data, jobsta_e blockstatus);
            void RenderBlockWithSelectedImages(const ATData& data, jobsta_e blockstatus, std::vector<image_t>& images);
            void resizeEvent(QResizeEvent* resizeEvent) override;
            void ROIEdit();
            void ResetConstraint();
            
            void ResetROI();
            void Run2();
            void ResetSelectLayer(const AI3D::VIEWER::selection_layer_e& layer);
            void ResetSelectionMode(const AI3D::VIEWER::selection_mode_e& mode);
            void ResetImageLayerSeleted(const std::set<AI3D::VIEWER::image_layer_e>& imageLayerSet);
            void ResetImageLayerSeleted(const std::set<AI3D::VIEWER::reconst_element_e>& imageLayerSet);
            void RemoveItem();
            int GetNumofNode();
            OsgEngine* getOsgEngine();
            std::vector<point3D_t> getPickedNodeId();
            std::vector<image_t> getPickedPhotoNodeId();
            bool getPickedPhotoNodeId2(std::vector<image_t>& pickedPhotoNodeId);

            std::vector<image_t> getPickedTileNodeId();
            bool getPickedTileNodeId2(std::vector<image_t>& pickedTileNodeId);

            void send_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names);
            void send_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name);
            void send_selected_images_from_3dview(std::vector<image_t>& images);
            void send_right_selected_images_from_3dview(std::vector<image_t>& images);
            void send_selected_tiles(std::vector<image_t>& images);

            bool hasSceneData();
            void setSceneData();
            void clearSceneData();

            static int getOsgEngineWorkingNum();
            static void dumpOsgEngineInfo();
           bool bModelloaded = false;
            image_t* item_select_ = nullptr;
        public:
            ViewerQT* viewerWindow;
        private:
            QHBoxLayout* mainLayout;
            
            OsgEngine* pOsgEngine;

            void addTextEditData(QString str);
            void addTextEidtListStr(QList<QString> str_list);

            bool isFirst;/** the first time construct the structure of scence. main node for mannual.*/
            bool isSemiautoFirst;/** the first time construct the structure of scence. main node for semiauto.*/
            QString baseosgpath;
            std::vector<std::string> str_osgbfilelist_;
            std::vector<std::string> str_lowestlevel_osgbfilelist_;
            std::vector<std::string> str_highestlevel_osgbfilelist_;
            QProgressBar* progressBar;
            std::vector<std::string> meshfiles_;
            void SetView(viewmode_e viewmode);

            QPushButton* butSave;
            QPushButton* butCancel;
        public:
            float polygonHeight;
            osg::ref_ptr<osg::Switch> previewFile;
            int blockNumber;
            float expand;
            bool bInited;

        private slots:
            void on_actionLoadFile_triggered();
            void Slot_OsgViewButtonClicked();
            
        signals:
            void mwindow_resized();
            void signal_projchanged(ReconstructionObject* object,bool bmodified);
            void signal_delete_photos(const std::vector<image_t> &ids,const std::vector<std::string>& names);
            void signal_delete_tiepoints(const std::vector<point3D_t> &ids,std::string& name);
            void signal_selected_images_from_3dview(std::vector<image_t> &images);
            void signal_right_selected_images_from_3dview(std::vector<image_t>& images);
            void signal_update_overview(ReconstructionObject* object);
            void signal_selected_tiles(std::vector<image_t> &tiles);
            void signal_roiedit_saved();
            void signal_roiedit_cancelled();

        private:
            bool bInsideProduction;
            bool bInsideBlockAT;
            bool bInsideConstruction;
            BlockObject* pBlock_data;
            ReconstructionObject* pReconstData;
            TilingCallbackEvent* pTilingCallbackEvent = nullptr;
            CallbackEventTest* pCallbackEventTest = nullptr;
            //ProductionCallbackEvent* pProCallbackEvent = nullptr;
            bool bUseLaterSize = false;
            bool bAllowEdit = false;
            bool bHasSceneData = false;
            static int iOsgEngineWorkingNum;
            static std::set<MWindow*> setOsgEngineWorking;
        };


        //做分块时的回调
        //1：拖动兴趣区则需要反馈新的box范围
        
        class TilingCallbackEvent : public EventBaseServer
        {
        public:
            TilingCallbackEvent(MWindow* pMWindow = nullptr,  void* pUserInfo = nullptr)
            {
                this->pMWindow = pMWindow;
                this->pObject = nullptr;
                /*this->pUserInfo = pUserInfo;*/
            }
            
            ~TilingCallbackEvent() {};

            void SetReconstruct( ReconstructionObject* pObject = nullptr)
            {
                this->pObject = pObject;
                
            }
            virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info);

            ABBox3d GetBox() { return box_; };
            std::pair<double, double> GetROIHeightRange() { return roi_height_range_; };
            ReconstructionObject* GetReconstructObject() { return pObject; };
        private:

            ABBox3d box_;
            std::pair<double, double> roi_height_range_;
            MWindow* pMWindow;
            ReconstructionObject* pObject;
            
        };

        //跟着视图转的那个
        class CamRotationCallbackEvent : public EventBaseServer
        {
        public:
            CamRotationCallbackEvent() {};
            ~CamRotationCallbackEvent() {};

            virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
            {
                

                if (type == CALL_BACK_CAMERA)
                {
                    osg::Matrix* pCameraMT = (osg::Matrix*)info.getEventInfo();
                //  std::cout << "camera R:" << pCameraMT->getRotate().asVec3().x() << " " << pCameraMT->getRotate().asVec3().y() << " " << pCameraMT->getRotate().asVec3().z() << std::endl;
                }
                
            }
            std::string GetBoxName() { return boxname; };
        private:
            std::string boxname;
        };

        //空三的回调：
        //1:选了一些影像，需告知影像的id
        //2：删除了一些点需告知点的id；
        //3：
        class ATCallbackEvent : public EventBaseServer
        {
        public:
            ATCallbackEvent() {};
            ~ATCallbackEvent() {};

            virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
            {
                if (type == CALLBACK_EVENT_TYPE::CALL_BACK_SELECT_PHOTO)
                {
                    image_ids_.clear();

                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stPhoto)
                    {
                        image_ids_.push_back(it.ID);
                        
                    }
                }
            
                else if (type == CALL_BACK_TIEPOINT)
                {
                    point_ids_.clear();
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stPhoto)
                    {
                        point_ids_.push_back(it.ID);
                        
                    }
                }
            }
            
        private:
            std::vector<int> point_ids_;
            std::vector<int> image_ids_;
        };
        
           /* class ProductionCallbackEvent : public EventBaseServer
        {
        public:
            ProductionCallbackEvent(MWindow* pMWindow = nullptr, BlockObject* pBlockData = nullptr, void* pUserInfo = nullptr)
            {
                this->pMWindow = pMWindow;
                this->pBlockData = pBlockData;
                this->pUserInfo = pUserInfo;

                std::cout << "ProductionCallbackEvent:" << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase <<
                    this->pMWindow << " " << this << std::dec << std::endl;

            }


            ~ProductionCallbackEvent() {}

            virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
            {
                if (type == CALLBACK_EVENT_TYPE::CALL_BACK_OSGB_LOADED)
                {
                    
                    if (pMWindow != nullptr)
                    {
                        pMWindow->bModelloaded =true;
                    }

                  
                }

                
            }

            std::string GetBoxName() { return boxname; };
        private:
            std::string boxname;
            MWindow* pMWindow;
            BlockObject* pBlockData;
            void* pUserInfo;
        };*/
        class CallbackEventTest : public EventBaseServer
        {
        public:
            CallbackEventTest(MWindow *pMWindow = nullptr,BlockObject* pBlockData = nullptr, void* pUserInfo = nullptr) 
            { 
                this->pMWindow = pMWindow;
                this->pBlockData = pBlockData; 
                this->pUserInfo = pUserInfo; 

                std::cout << "CallbackEventTest:" << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase <<
                    this->pMWindow << " " << this << std::dec << std::endl;

            }

            
            ~CallbackEventTest() {}

            virtual void callBackEvent(CALLBACK_EVENT_TYPE type, const EventInfo& info)
            {                           
                if (type == CALLBACK_EVENT_TYPE::CALL_BACK_SELECT_PHOTO)
                {
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    std::set<image_t> ids;

                    std::cout << "got selected photo signal from osgengine." << std::endl;
#if 1 
                    for (auto it : *stPhoto)
                    {
                        //std::cout << "CB/Photo selected: " << it.ID << " " << it.name << std::hex <<std::showbase << it.ID << std::dec << std::endl;
                        ids.insert(it.ID);
                    }     

                    std::vector<image_t> pickedImages;
                    //std::cout << "got selected photo signal from osgengine:" << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase <<
                    //  pMWindow << std::dec << std::endl;
                    if (pMWindow != nullptr)
                    {
                    //  std::cout << "got selected photo signal from osgengine:" << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase <<
                    //      pMWindow << std::dec << std::endl;

                        ///pickedImages = pMWindow->getPickedPhotoNodeId();
                        pickedImages.assign(ids.begin(),ids.end());
                        if (pickedImages.size() > 0)
                        {                           
                            //影响3DView中照片选中性能，注释掉目前未发现其他问题，modify by zhaobf
                            pMWindow->send_selected_images_from_3dview(pickedImages);
                        }
                    }

                    //std::cout << "got selected photo signal from osgengine:" << __FILE__ << " " << __LINE__ << " " << std::hex << std::showbase <<
                    //  pMWindow << std::dec << std::endl;
#endif
                }

                if (/*type == CALLBACK_EVENT_TYPE::CALL_BACK_RIGHT_SELECT_PHOTO ||*/ type == CALLBACK_EVENT_TYPE::CALL_BACK_SELECT_PHOTO_WINDOWS)
                {
                    std::vector<image_t> pickedImages;
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();

                    for (auto it : *stPhoto)
                    {
                    /// std::cout << "Photo right selected: " << it.ID << " " << it.name << std::endl;
                        pickedImages.push_back(it.ID);
                        break;
                    }

                    // note:just for test purpose.
                    ///OpenUserTiePoints();

                    if (pMWindow != nullptr)
                    {
///                     pickedImages = pMWindow->getPickedPhotoNodeId();
                        if (pickedImages.size() > 0)
                        {
                            pMWindow->send_right_selected_images_from_3dview(pickedImages);
                        }
                    }
                }

                if (type == CALLBACK_EVENT_TYPE::CALL_BACK_SELECT_TILE)
                {
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stPhoto)
                    {
                        //std::cout << "inside select tile callback:Tile: " << it.ID << " " << it.name << std::endl;
                    }

                    std::vector<image_t> selectedTileNodes;
                    if (pMWindow != nullptr)
                    {
                        std::cout << "inside select tile callback:Tile: " << __LINE__ << std::endl;
                        if (pMWindow->getPickedTileNodeId2(selectedTileNodes))
                        {
                            std::cout << "inside select tile callback:Tile: " << __LINE__ << " " << selectedTileNodes.size() << std::endl;
                            pMWindow->send_selected_tiles(selectedTileNodes);
                        }
                    }

                    std::cout << "inside select tile callback:Tile: " << __LINE__ << std::endl;
                }
                if (type == CALL_BACK_ROI_BOX_DRAG)
                {
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stPhoto)
                    {
                        boxname = "9999";
                        //std::cout << "ROI: " << it.ID << " " << it.name << " , XYZ: " << it.bbox.xMax() << " " << it.bbox.yMax() << " " << it.bbox.zMax() << std::endl;
                    }
                    if (this->pMWindow != nullptr)
                    {
                        //std::cout << " 000 " << std::endl;
                    }
                    if (this->pBlockData != nullptr)
                    {
                        //std::cout << " 9999000 " << std::endl;
                    }
                }
                if (type == CALL_BACK_ROI_POLYGON_DRAG)
                {
                    std::vector<PolygonBox>* stPhoto = (std::vector<PolygonBox>*)info.getEventInfo();
                    for (auto it : *stPhoto)
                    {
                        //std::cout << "ROI_POLYGON: " << it.ID << " " << it.name << " , minH: " << it.minHeight << " maxH" << it.maxHeight << std::endl;
                    }
                }

                if (type == CALL_BACK_CAMERA)
                {
                    osg::Matrix* pCameraMT = (osg::Matrix*)info.getEventInfo();
                    
                    //std::cout << "camera R:" << pCameraMT->getRotate().asVec3().x() << " " << pCameraMT->getRotate().asVec3().y() << " " << pCameraMT->getRotate().asVec3().z() << std::endl;
                }
                
                if (type == CALL_BACK_TIEPOINT)
                {
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhoto = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    
                }

                if (type == CALL_BACK_REMOVE_PHOTO)
                {
                    // delete photos.
                    std::cout << "need to remove photo." << std::endl;

                    std::vector<image_t> vecImages;
                    std::vector<std::string> vecNames;

                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stPhotos = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stPhotos)
                    {
                    //  std::cout << "Photo to be removed: " << it.ID << " " << it.name << std::endl;
                        vecImages.push_back(it.ID);
                        vecNames.push_back(it.name);
                    }

                    if (pMWindow != nullptr)
                    {
                        pMWindow->send_delete_photos(vecImages, vecNames);
                    }
                }

                if (type == CALL_BACK_REMOVE_TIEPOINTS)
                {
                    // delete tiepoints
                    std::vector<point3D_t> vecTiePoints;

                    //std::cout << "need to remove tiepoints." << std::endl;
                    std::vector<ST_CALLBACK_ELEMENT_INFO>* stTiePoints = (std::vector<ST_CALLBACK_ELEMENT_INFO>*)info.getEventInfo();
                    for (auto it : *stTiePoints)
                    {
                        //std::cout << "TiePoint to be removed: " << it.ID << " " << it.name << std::endl;
                        vecTiePoints.push_back(it.ID);
                    }

                    if (pMWindow != nullptr)
                    {
                        pMWindow->send_delete_tiepoints(vecTiePoints, std::string("tiepoints"));
                    }
                }

                if (type == CALL_BACK_REMOVE)
                {
                    // deleting images,tiepoints or etc depends on detail situation.
                    std::cout << "need to remove some elements compared with current image layer selected." << std::endl;

                    // to find out selected elements' detail type via osgengine->getCurrentElementtTpe()
                    if (pMWindow != nullptr && pMWindow->getOsgEngine() != nullptr)
                    {
                        if (pMWindow->getOsgEngine()->GetCurrentElementType() == ELEMENT_LAYER_TYPE::ELEMENT_PHOTOS)
                        {
                            //std::cout << "current selected element type needed to be removed is photos." << std::endl;
                            std::vector<point3D_t> photoIds = pMWindow->getPickedNodeId();
                            for (auto t : photoIds)
                            {
                                ///pMWindow->Send_Delete3DViewItem(t,"images");
                            }
                        }
                        else if (pMWindow->getOsgEngine()->GetCurrentElementType() == ELEMENT_LAYER_TYPE::ELEMENT_TIEPOINTS)
                        {
                            //std::cout << "current selected element type needed to be removed is tiepoints." << std::endl;
                            std::vector<point3D_t> tiePoints = pMWindow->getPickedNodeId();

                            for (auto t : tiePoints)
                            {
                                //emit pMWindow->delete_3dview_item(t, "tiepoints");
                                //pMWindow->Send_Delete3DViewItem(t, "tiepoints");
                                //void delete_3dview_item(point3D_t, QString);
                            }

                        }
                        else
                        {
                            //std::cout << "current selected element type is neither photos nor tiepoints,can't be removed now." << std::endl;
                            // other element type.
                        }
                    }
                }
            }

            std::string GetBoxName() { return boxname; };
        private:
            std::string boxname;
            MWindow* pMWindow;
            BlockObject* pBlockData;
            void* pUserInfo;
        };


        class osgQOpenGLWidget : public QOpenGLWidget,
            protected QOpenGLFunctions
        {
            Q_OBJECT
        protected:
            /*
                        OSGRenderer* m_renderer{ nullptr };
                        */
                        bool _osgWantsToRenderFrame{ true };
                    //  OpenThreads::ReadWriteMutex _osgMutex;                  
            osg::ArgumentParser* _arguments{ nullptr };         
            bool _isFirstFrame{ true };

            //friend class OSGRenderer;

        signals:
            void initialized();

        public:
            osgQOpenGLWidget(QWidget* parent = nullptr);
            osgQOpenGLWidget(osg::ArgumentParser* arguments, QWidget* parent = nullptr);
            virtual ~osgQOpenGLWidget();

        };

        class ConstructionWgt : public QWidget
        {
            Q_OBJECT
        public:
            ConstructionWgt(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject *reconsObject,QStandardItem *recons_item, QWidget* parent = nullptr);
            ~ConstructionWgt();

            AI3D::CORE::ReconstructionObject* getReconstructionObject() { return recons_object_; }
            MWindow* getMWindow() { return mWindow;  }

        public:
            void SetButtonStates();
            void ShowOrHideDetails();
            
            void SetOverviewWarning(QString strWarning);
            void ShowOverviewWarning(bool bShow = true);
            void SetOverviewROIDimension(float x, float y, float z);
            void SetExpectedMaxRamUsage(float maxRam);
            
            void DisconnectSignalMap4ROILimits();
            void ConnectSignalMap4ROIlLimits();
            void RefreshOverviewInfo();
            void TestGeometryContraints();
            void InsertGeometryContraintsItem(QString itemName,QString itemType,QString replacementOption);

            void SetProjectModified();
            void SetRightSideEditable(bool status);
            void DoFirstRenderReconstruction();
            void RefreshEditableState();

            void showEvent(QShowEvent* event) override;
            void hideEvent(QHideEvent* event) override;
            void closeEvent(QCloseEvent* event) override;

            void setTileCategoryExtra();
            void SetLayerType();
        public slots:
            void Slot_SubmitProduction();
            void Slot_CurrentIndexChanged(const QString& text);
            void Slot_TextChanged(const QString& text);
            void Slot_EditingFinished();
            void Slot_ROIEditingFinished();
            void Slot_Refresh_Timeout();
            void Slot_DelayedResizeTilingMode();
            void Slot_DelayedResetTileMAXRamUsage();
            void Slot_DelayedResetTileSize();
            void Slot_Delayed2DMode();
            void Slot_Delayed3DMode();
            void Slot_ROIEdit();
            void Slot_ROIImport();
            
            void Slot_ROIDefault();
            void Slot_TiePoints();
            void Slot_Photos();
            void Slot_Frustum();
            void Slot_ToggleShowOrHideDetails();
            void Slot_LoadOSGBFile();
            void Slot_GeometryContraintsImport();
            void Slot_ItemClicked(QTableWidgetItem* pItem);
            void Slot_MWindowResized();
            void Slot_MoreSettings();
            void Slot_GeometryContraints_CustomContextMenuRequested(const QPoint& pos);
            void Slot_GeometryContraints_Delete();
            void Slot_UpdateROI(ReconstructionObject* object);
            void Slot_UpdateROIBy3DViewEdit(ReconstructionObject* object,bool bmodified);
            void Slot_DoneParamSettings4Production(bool result);
            void Slot_ClickTab(int index);
            void Slot_ROIEdit_Saved();
            void Slot_ROIEdit_Cancelled();
            void Slot_Delete_Production_Done();
            void Slot_SelectTypes();

        signals:
            void Sig_NewProductionStarted(AI3D::CORE::BlockObject*, reconstruction_t, QStandardItem*);
            void Sig_NewProduction(AI3D::CORE::BlockObject*, reconstruction_t, production_t,QStandardItem*);
            void Sig_ProjModifed();
            void Sig_DelayedResizeTilingMode(tiling_mode_e newMode);
            void Sig_DelayedResetTileMAXRamUsage(float newValue);
            void Sig_DelayedResetTileSize(float newValue);
            void Sig_IsModifiedProj();
            void signal_projchanged(ReconstructionObject* object);

        private:
            Ui::CReConstructionWgt* ui;
            AI3D::CORE::ReconstructionObject* recons_object_;
            QStandardItem* recons_item;
            AI3D::CORE::BlockObject* block_data_;
            QComboBox* cbbTileCategory;
            QLabel* lblTileExtra;
            QLineEdit* leTileExtra;
            QLabel* lblCenterRightOverviewDetail;
            QLabel* lblOverviewROIDimension;
            QLabel* lblExpectedMaxRamUsage;
            QLabel* lblOverviewWarningIcon;
            QLabel* lblOverviewWarning;
            QTableWidget* twProductionList;
            QTimer* refresh_timer_;
            QPushButton* butNewProduction;
            QPushButton* butNewProduction2;
            QPushButton* butROIEdit;
            QPushButton* butROIImport;
            QPushButton* butROIDefault;
            float savedTileSize;
            float savedMaxRamUsage;
            tiling_mode_e savedTilingMode;
            int iLastDelayedAction; // -1:init / 0:ResizeTilingMode / 1:ResetTileMAXRamUsage
            bool bShowDetails;
            bool bSupportMoreTileMode;
            QPushButton* butTiePoints;
            QPushButton* butPhotos;
            QPushButton* butFrustum;
            QPushButton* butShowOrHideDetails;
            QPushButton* butIcon4ShowOrHideDetails;

            QLabel* lblROIXUnit;
            QLabel* lblROIXMin;
            QLineEdit* leROIXMin;
            QLabel* lblROIXMax;
            QLineEdit* leROIXMax;

            QLabel* lblROIYUnit;
            QLabel* lblROIYMin;
            QLineEdit* leROIYMin;
            QLabel* lblROIYMax;
            QLineEdit* leROIYMax;

            QLabel* lblROIZUnit;
            QLabel* lblROIZMin;
            QLineEdit* leROIZMin;
            QLabel* lblROIZMax;
            QLineEdit* leROIZMax;
            ///ViewerQT* viewerWindow;
            MWindow* mWindow;
            
            QFrame* lineGeometryContraints;
            QLabel* lblGeometryContraintsTitle;
            QPushButton* butGeometryContraintsImport;

            QCheckBox* cbPhotos;
            QCheckBox* cbTiePoints;
            QCheckBox* cbGCP;
            QCheckBox* cbTiling;
            QCheckBox* cbROI;
            QCheckBox* cbConstraints;

            QTableWidget* twGeometryContraints;

            QStringList slTextureReplacementOption;
            bool bHasRenderedATData;
            QLabel* lblMoreSettings;
            QPushButton* butMoreSettings;
            QMenu* menu_RightClick4GeometryContraints;
            QAction* action_delete4GeometryContraints;
            bool bRenderReconstructionOnce = false;
            bool bROIEditing = false;
            osg::Matrixd savedMatrix;
            bool bInsideOverview;
            QString lastTileExtraValue;
            int lastTileModeIndex;
        };

        class CircularProgressWgt;

        class ProductionWgt : public QWidget
        {
            Q_OBJECT
        public:
            ProductionWgt(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject* recons_object, QStandardItem* recons_item, AI3D::CORE::ProductionObject* production_object, QStandardItem* production_item, QWidget* parent = nullptr);
            ~ProductionWgt();

            AI3D::CORE::ReconstructionObject* getReconstructionObject() { return recons_object_; }
            AI3D::CORE::ProductionObject* getProductionObject() { return production_object_; }

            static job_status_e CalcStatusAndPercent(AI3D::CORE::BlockObject* block, AI3D::CORE::ReconstructionObject* recons_object, AI3D::CORE::ProductionObject* cpo,int& percent);
            static job_status_e GetProductionStatus(BlockObject* block_data, ReconstructionObject* recons_object_, ProductionObject* cpo);

            void setInformationByPurpose(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>> &setInformation,std::vector<std::string> &translated_infos);
            void setInformationOriginal(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation);
            // Purpose:3dmesh
            void setInformationBy3DMesh(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation);
            // Purpose:3d pointlcoud
            void setInformationBy3DPointCloud(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation);
            //,modified  by 
            void SetInformation(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation,std::vector<std::string> &translated_infos);
            // Purpose:TDOM/DSM
            void setInformationBy4D(QWidget* panelBottom, std::vector<std::pair<std::string, std::string>>& setInformation);
            void InitProductionItemInfo();
            void GetProductionItemInfo();
            void RefreshProductionItemInfo();
            void RefreshTileRow(std::string& tile,bool bInsertNewRow = false);

            void showEvent(QShowEvent* event) override;
            void hideEvent(QHideEvent* event) override;
            void closeEvent(QCloseEvent* event) override;

        public slots:
            void Slot_Dummy();
            void Slot_Refresh_Timeout();
            void Slot_CancelProduction();
            void Slot_ResubmitProduction();
            void Slot_ClickTab(int idx);
            void Slot_Refresh_TimeoutV2();

        signals:
            void signal_delete_production_done();

        public:
            bool bCancelled;
            jobsta_e status_;
            MWindow* mWindow;
            ConstructionWgt* pConstructionWgt = nullptr;
            std::vector<ProductionItemInfo> vecProductionItemInfo;
            std::vector<std::string> vecTile;
            std::map<std::string, ProductionItemInfo> mapProductionItemInfo;

        private:
            Ui::CReConstructionWgt* ui;
            AI3D::CORE::ReconstructionObject* recons_object_;
            QStandardItem* recons_item;
            AI3D::CORE::ProductionObject* production_object_;
            QStandardItem* production_item;
            AI3D::CORE::BlockObject* block_data_;
            QTableWidget* twProductionList;
            QTimer* refresh_timer_;
            CircularProgressWgt* cpwLeft;
            QLabel* lblTopLeft;
            QLabel* lblTopRightTop;
            QLabel* lblTopRightBottom;
            QPushButton* butCancelProduction;
            QPushButton* butResubmitProduction;
            
            std::vector<std::string> overview_settings_title; //  size:13
            std::vector<std::pair<std::string, std::string>> vecSettingInformation;

            QLabel* lblID;
            QLabel* lblFormat;
            QLabel* lblDestination;

            QLabel* lblTypeOfLevelOfDetail;
            QLabel* lblOrigin;
            QLabel* lblSpatialReferenceSystem;

            QLabel* lblLevelOfDetailSize;
            QLabel* lblScopeOfLevelOfDetail;

            QLabel* lblTextureCompressionQuality;
            bool bProductionItemInfoFirstRendered;
            bool bProductionItemInfoNeedRendering;
            bool bDestroying;
            bool bResubmitting;
            bool bProductionItemInfoGetting;
            bool bProductionItemInfoGot;
            bool bForceRefreshAll;
            bool bRenderProductionOnce = false;
        };

        class CircularProgressWgt : public QWidget
        {
            Q_OBJECT
        public:
            CircularProgressWgt(QWidget* parent = nullptr);
            ~CircularProgressWgt();
        
            void setPercent(int value);

        public slots:
            void Slot_Percent_Timeout();

        protected:
            virtual void paintEvent(QPaintEvent* event);
            QSize sizeHint() const;
            QSize minimumSizeHint() const;

        private:
            QTimer* timerPercent;
            int percent;        
        };

        class ProgBarContainer : public QWidget
        {
        public:
            ProgBarContainer(QWidget* parent = nullptr);

        public:
            QHBoxLayout* hlProgBar;
            QProgressBar* pProgBar;
            QLabel* pLblProg;
        };

        class BlockWgt : public QWidget
        {
            Q_OBJECT

        public:
            enum TabPage
            {
                TPGeneral = 0,
                TPPhotos,
                TPPos,
                TPControlPos,
                TP3DView
            };
        
        //pos列表
            enum PosList_col_e
            {
                IMAGE_ID_COL = 0,
                IMAGE_NAME_COL,
                IMAGE_PATH_COL,
                POS_STATUS_COL,
                POS_VALUE_COL,
                POSLIST_COUNT
            };

            //BlockWgt(QWidget* parent = Q_NULLPTR);
            BlockWgt(AI3D::CORE::BlockObject* block, QWidget* parent = nullptr);
            ~BlockWgt();
        
            struct InfoForShow_s
            {
                jobsta_e status;
                float progreesvalue;
                std::string progressstylestr;
                QString SubmitTime= "--/--";
                QString EndTime = "--/--";

                QString ATStagetext = "";//对应stage后面的
                std::string ATStagetextstylestr;
                QString ATStatustext = "";//对应canclelog
                std::string ATStatustextstylestr;
                QString ATReporttext = "";
                std::string ATReporttextstylestr;
            };
            
        public:
            //-------
            //上排主按钮
            void InitMainButton();
            void SetMainButtonStatus(Block_Status_s blockManagerStatus);
            
            //ATTab
            void InitATWgt();
            void GetRealTimeInfo();
            void GetRealTimeInfoV2();
            void InitATTabConnections();
            void UpdateNewReconstructionStatus(Block_Status_s blockManagerStatus);
            void UpdateJobStageLists(QVector<JobStage> vec_job);
            void UpdateATTabLabel(InfoForShow_s show);
            static int UpdateEngineStatus();
            int UpdateCompleteJobATFile();
            
            void QuitJobInfoTimer();
            void StartJobInfoTimer(bool bloaded =false);
            static jobsta_e UpdateBlockStatusToProject(AI3D::CORE::BlockObject* block);//仅仅是给project界面展示用
            //实时获取进度信息
            /*void GetRealTimeATJobFullInfo();*/
            //photogroup页卡
            void InitPhotoTabIsEdit();
            void InitPhotoTabConnections();
            //void SetPhotoOrGroupDetailWgtStatus(Block_Status_s blockManagerStatus);
            
            //GCP
            void InitGcpTabConnections();
            //tile
        
            void UpdateWgtAndProjStatus(bool bchangetab = false);
            
            // 设置按钮状态及tab可否点击
            void InitButtonAndLabel();

            //photogroup
            void InitTableViewPhotoGroup();
            //pos列表的
            void InitTableWidgetPosList();

            void CreateConnection();
            void keyPressEvent(QKeyEvent* e) override;
            //默认显示3dview
            void InitNewWidget();
            //根据信息决定显示哪些tab
            void InitLoadWidget();
            //void SlotClickTab(int idx);
            void SetIndexByStr(QString str);
            void InitGcpControlPointWgt();
            
            void SetWgtStatus(Block_Status_s blockManagerStatus);
            void PopulatePhotoGroupTable();
            void ClearModelWithoutHeader(QAbstractItemModel* itemModel);

            void PopulatePosTableWgt(AI3D::CORE::PhotoGroup& group);

            void SetModifityXml();
            void Update3DView();
            //生成预览图
            void MakePriview();
            //可以不用了
            void MakePriviewImage();
            //导入影像时的进度条
            void cbProgress(int value);
            void UpdateTabPaper(std::vector<int> papervec);

            void SetCurrentExeNum(int num) { currentExeNum = num; };

            QString getBlockName() const { return QString(block_data_->GetName().c_str()); };
            QString getBlockPath() const { return QString(block_data_->GetPath().c_str()); };

            
            //双server
            void AddItemContent(int row, int column, QString content);
            
            void ShowATTab(bool isHideOther = false);//控制Tab页卡，photo GCP 3dview等
            
            
            void SetCurrentTabId();
            int GetCurrentTabId();
            

            void InsertGCPTab();
            //void Slot_QTableView_CustomContextMenuRequested();

            void SetPhotoTabEditable(bool be);

            void showEvent(QShowEvent* event) override;
            void hideEvent(QHideEvent* event) override;
            void closeEvent(QCloseEvent* event) override;

            void SetRightSideKxPxEditable(bool status);
            void RefreshRightSideKxPxEditable();
            void ImportGCP(AI3D::CORE::ControlPoints& gcps_import, srs_s& srs,
                EIGEN_STL_UMAP(image_t, std::string)& image_map);
            static QString getChineseString(const char *section, const char *text);

        public slots:
            void Slot_PhotoGroupItemModified(int row, int col, const QString& text);
            void slot_linkActivated_label_view_report(QString link);
            //void slot_delete_item();
            //void slot_delete_item(const point3D_t& id, const QString& name);
            //点击增加Photo按钮
            void Slot_Btn_AddPhotoFile_Clicked();
            void Slot_Btn_AddPhotoDir_Clicked();
            //删除Photo按钮
            void Slot_Btn_DelPhoto_Clicked();

            //add/del pos Button
            void Slot_Btn_AddPos_Clicked();
            void Slot_Btn_DelPos_Clicked();

            //add/del gcp Button
            void Slot_Btn_AddSigGcp_Clicked();
            void Slot_Btn_AddGcp_Clicked();
            void Slot_Btn_DelGcp_Clicked();

            void Slot_Action_ExportMeasurementToXml();
            void Slot_Action_ImportMeasurementFromXml();
            //pushbutton submit clicked
            //新框架下需要更改的地方有：
            // 1：提交空三
            // 2：终止
            // 3：delete
            // 4：进度更新也是
            //的逻辑：1如果是runnging状态，则将cancle操作写进feedback；等待enigne处理，2：pending 状态则直接更新feedback 和移动文件
            void Slot_Btn_SubmitAerotri_Clicked2();         
            void Slot_Btn_Cancle2();            
            void Slot_Btn_Resubmit2();
            //因为需要增加AT的一些参数而加的
            
            bool SubmitATWithDefinition(AI3D::CORE::ATOptions& at_options);
            
            

            //重建的
            // 
            void Slot_Btn_SubmitReconstruct_Clicked();
            


            void Slot_TableView_RealClicked(QModelIndex index);

            //tableview photogroups clicked 照片组控件点击
            void Slot_TableView_Clicked(QModelIndex index);

            //pushbutton choose pos file clicked
        /*  void Slot_Btn_ChoosePosfile_Clicked();*/
            
            void Slot_TableWidget_Photo_Pos_RealClicked(QModelIndex index);
            // alka 1.4.231
            void Slot_TableWidget_Photo_Pos_Clicked(QModelIndex index);

            //progressbar submit value changed
            //void slot_progressBar_submit_valueChanged(int value);
            //void Slot_ItemModel_PhotoGroup_ItemChanged(QStandardItem* item);

            void Slot_LinkActivated_Label_Monitor_Job(QString link) {};
            
            void Slot_Check_Preview(bool state);
            void Slot_LinkActivated_Label_Photo_View() {};
            void Slot_LinkActivated_Label_Photo_Open();

            void Slot_LinkActivated_Label_View_Report(QString link) {};

            //read pos file completed from thread
            void Slot_PosFileReadComplet() {};
            //read pos error
            /*void SetListWidgetItemIcon(QString& oldFileName, QString& fileName, int& num, int& totalNum);*/

            void beginScaledImage(QStringList& fileNameList, QString& destPath);
            /*void handleFinished();*/
    
            bool ExistsTab(std::string page);
            //void GetTabWgt(std::string name);
        /*  void InitTaskList(int row,int coloum, QString jobname);*/
        
            //更新所有的TaskName;
            void UpdateTaskListAll(QVector<JobStage>& jobStage);
            void Show_3DView_Progress(int,QString);
            void ChangeDistorion1();
            void ChangeDistorion(QString);
            void Slot_QTableWidgetPhotoPos_CustomContextMenuRequested(const QPoint& pos);
            void Slot_QTableWidgetPhotoGroup_CustomContextMenuRequested(const QPoint& pos);
            //void Slot_Table_Sig_Pos_Delete();
            void SlotDeletePhotoGroup();
            void SlotClearPoseByGroup();
            void SlotDeletePhoto();
            void SlotClearPhotoPose();
            void Slot_ClickTab(int idx);

            void Slot_delete_photos(const std::vector<image_t>& ids, const std::vector<std::string>& names);
            void Slot_delete_tiepoints(const std::vector<point3D_t>& ids, std::string& name);
            void Slot_selected_images_from_3dview(std::vector<image_t>& images);
            void Slot_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName);
            void Slot_currentChanged(int index);

        signals:
            //修改工程数据
            void Sig_IsModifiedXml();
            //修改blk,没有用到
            void Sig_IsModifiedProj();
            void Signal_Photo_Progress(int);
            void Signal_Submit_Block(AI3D::CORE::BlockObject* block);
            void Signal_Submit_Block_Wgt();//预先加载提交AT的进度条界面
            void Sig_SaveFinished();
            void Sig_BlockStatus(jobsta_e,int);

            void finishedProcess(const QString& msg);
            void Sig_NewConstruction(AI3D::CORE::BlockObject*,group_t);
            void signal_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName);

        public:
            ViewWidget* viewWidget_ui; 
        protected:
            void resizeEvent(QResizeEvent* event);
        private:
            friend class ControlPointsEditorWin;
            bool item_deleted_happened_in3dview_ = false;
            QStringList list_string_pos_header_;
            QMenu* ui_menuphotogroup_rightClick_selectRows;
            QMenu* ui_menuphotopos_rightClick_selectRows;
            QAction* ui_action_deletephotogroup_;
            QAction* ui_action_clearphotogroup_pose_;
            QAction* ui_action_deletephotopos_;
            QAction* ui_action_clearpos_;
            void UpdatePhotoDetailStatus();
            /*MoPhotoTableWidget* photogroup_listview_;
            MoPhotoTableWidget* photopos_listview_;
            QStandardItemModel* itemmodel_photogroup_;
            QStandardItemModel* _itemmodel_pos_;*/
            QStringList list_string_photogroup_header_;
            /*std::shared_ptr<AI3D::CORE::BlockObject>*/ 
            AI3D::CORE::BlockObject* block_data_;
            QItemSelectionModel* theSelection;
            
            QMap<QString,QWidget*> myTabWidget_;//存储tabwidget的01页卡
            //ControlPointWins* controlPoints_ui_;
            ControlPointsEditorWin* controlPoints_ui_;
            ImageViewerGraphicsScene m_scene;
            QFutureWatcher<bool> watcher;
            std::map<int, std::string> tab_wgts_;
            
            /*int current_groupid;
            QModelIndex current_tablewidget_index;*/

            QModelIndex current_tableview_index;
            QModelIndex current_tablewidget_index;
            int photoAllNum;
            int _nTotalPhotos;
            ProgressCom* my_Progress;
            QProgressDialog* progress_bar_;
            int currentExeNum = 0;
            int current_tab_id_ = 3;//默认3DView
            int progress_;
            QString retMsg_;
            std::shared_ptr<AI3D::CORE::ATData> ATdata_;

            bool isupdategcp = false;
            //是否可以修改自由网参数
            bool CanGPSParamsModified_ = true;

            QMenu* menu_rightTableWidget_Pos;
            QAction* ui_action_delete_sigphoto;
            int menuRow = -1;
            //需要两个定时器
            /*1:任务进行时状态的定时器；
            * 2::engine是否在运行时的定时器
            * 目前的定时器开启和关闭都有一定的时间，但实际上应该是一直开启，因为project也需要知道他们每个block的状态
            */
            QTimer* GetRunningInfoTime = nullptr;
            QTimer* GetEngineInfoTime = nullptr;

            QTimer* time = nullptr;
            QTimer* pJobTimer = nullptr;
            QString atBlockPath_ = "";
            image_t* item_select_ = nullptr;;
            /*JobStats jobStats;*/
            bool atreportloaded = false;
            bool bGettingJobInfo = false;
            bool bGotNewJobInfo = false;

            bool bInsideReconstruction = false;
            bool bInsideProduction = false;
        private:
            Ui::CBlockWgt* ui;
            std::vector<image_t> selectedImagesFrom3dviewOnly;
            std::vector<image_t> selectedImages;
            bool bNeedCheckSelectedImagesLater = false;
            std::set<image_t> setSelectedImagesFrom3dview;
            std::map<group_t, std::vector<image_t>> mapSelectedImagesFrom3dview;
            image_t min_selected_image_id = kInvalidImageId;
            group_t min_selected_group_id = kInvalidGroupId;
            bool bHasChangedSelectedImagesInsidePhotosTab = false;
            int iLastTabPos = -1;

        };

        //可修改可编辑
        class ReadOnlyDelegate : public QItemDelegate
            {
            
        public:
            ReadOnlyDelegate(QWidget * parent = NULL) :QItemDelegate(parent)
            {
            }
            QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
            const QModelIndex & index) const override //final
            {
                Q_UNUSED(parent)
                Q_UNUSED(option) 
                Q_UNUSED(index)
                return NULL;
            }
        };

        class QWIntLineDelete :public QStyledItemDelegate
        {
            Q_OBJECT

        public:
            explicit QWIntLineDelete(QObject* parent = nullptr) :QStyledItemDelegate(parent)
            {}

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
            {
                QLineEdit* edit = new QLineEdit(parent);
                edit->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                //edit->setStyleSheet("border-image:url(:/new/prefix1/skin/lineedit.png);");
                edit->setFrame(false);
                edit->setMaxLength(20);
                //
                QtVEditorDoubleValidator* pDoubleValidator = new QtVEditorDoubleValidator(0, 99, 2);
                pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
                //pDoubleValidator->validate('');
                
                //QRegExp rx("^(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*)){1,20}$");
                //QRegExp rx("^[1-9]\d*\.\d + $ | ^ 0\.\d + $ | ^ [1 - 9]\d * $ | ^ 0$");
                //QRegExpValidator* pReg = new QRegExpValidator(rx, nullptr);

                edit->setValidator(pDoubleValidator);

                return edit;
            }

            void setEditorData(QWidget* editor, const QModelIndex& index) const override
            {
                double value = index.model()->data(index, Qt::EditRole).toDouble();
                QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
                


                if (QString::number(value, 'f', 8).size() > 15)
                {
                    QString str = QString::number(value, 'f', 8).left(15) + "...";
                    lineedit->setText(str);
                    
                }
                else
                {
                    lineedit->setText(QString::number(value, 'f', 8));

                }
                

                /*QFontMetrics fontWidth(lineedit->font());
                QString elideNote = fontWidth.elidedText(QString::number(value, 'f', 8),Qt::ElideRight,15);*/
                

            }

            void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
            {
                //将代理组件的数据保存到数据模型中
                QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
                double value;
                if (lineedit->text().size() > 15)
                {
                    value = lineedit->text().left(15).toDouble();
                }
                else if (lineedit->text().size() ==0)
                {
                    value = -1;
                }
                else
                {
                    value = lineedit->text().toDouble();
                }               
            
                model->setData(index, value, Qt::EditRole);
            }

            void  updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
            {

                editor->setGeometry(option.rect);
            }

        };

        struct Location
        {
            double L1, L2, L3, L4, L5, L6, L7, L8;
        };


        void Refresh3DViewOfConstructionWgt();
    }
}


#endif

