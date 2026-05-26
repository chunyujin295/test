#pragma once
#include <qwidget.h>
//#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCloseEvent>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLCDNumber>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QCheckBox>
#include <QRadioButton>

#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QListWidget>
#include <QResizeEvent>
#include <QDialog>
#include <QStackedWidget>

#include <QSortFilterProxyModel>
#include <qdatetime.h>
#include <qtimer.h>
#include "Util/Settings.h"
#include <QOpenGLWidget>						

#include "Gui/GlobalStruct.h"
///#include "Gui/BlockWgt.h"
#include "Core/ReconstructionObject.h"

#define PERCENT_100_QUALITY_JPEG "100% quality JPEG"
#define PERCENT_90_QUALITY_JPEG  "90% quality JPEG"
#define PERCENT_75_QUALITY_JPEG  "75% quality JPEG"
#define PERCENT_50_QUALITY_JPEG  "50% quality JPEG"

#define SAMPLING_POINTS_UNIT_METER "meters"
#define SAMPLING_POINTS_UNIT_PIXEL "pixels" //modified by  add s

#define POINT_CLOUD_FORMAT_LAS "LAS"
#define POINT_CLOUD_FORMAT_PLY "PLY"
#define POINT_CLOUD_FORMAT_OSGB "OSGB"

#define D3_VIEW_SELECTION_MODE_SINGLE_ITEM "Single Item"
#define D3_VIEW_SELECTION_MODE_RECTANGLE "Rectangle"
#define D3_VIEW_SELECTION_MODE_POLYGON "Polygon"

#define D3_VIEW_IMAGE_LAYER_PHOTOS "Photos"
#define D3_VIEW_IMAGE_LAYER_TIEPOINTS "TiePoints"
#define D3_VIEW_IMAGE_LAYER_GCP "GCP"

namespace AI3D
{
    namespace GUI
    {
        class MWindow;
        class ViewWidget;
        class ConstructionWgt;
    }
}

//自定义SortProxy
//? @
class MySortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MySortProxy(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        //排序role，也就是获取model-data时用的role
        setSortRole(Qt::InitialSortOrderRole);
        //本地化，对中文排序有影响
        //在对字符串排序时，是否考虑本地因素，默认false
        setSortLocaleAware(true);
    }
    //... ...
    //重写虚函数接口
    bool lessThan(const QModelIndex& source_left,
        const QModelIndex& source_right) const override
    {
        //参照源码
        QVariant l = (source_left.model() ? source_left.model()->data(source_left, sortRole()) : QVariant());
        QVariant r = (source_right.model() ? source_right.model()->data(source_right, sortRole()) : QVariant());
        return isVariantLessThan(l, r, sortCaseSensitivity(), isSortLocaleAware());
    }

    //修改自源码QAbstractItemModelPrivate
    bool isVariantLessThan(const QVariant& left,
        const QVariant& right,
        Qt::CaseSensitivity cs = Qt::CaseSensitive,
        bool isLocaleAware = false) const
    {
        //修改源码对无效值得判断
        //if (left.userType() == QVariant::Invalid)
        //    left = false;
        //if (right.userType() == QVariant::Invalid)
        //    right = true;
        //无效值作为0来判断，这样就在正负数之间展示
        if ((left.userType() == QVariant::Invalid) ||
            (right.userType() == QVariant::Invalid))
            return left.toDouble() < right.toDouble();
        //下面未改动
        switch (left.userType()) {
        case QVariant::Int:
            return left.toInt() < right.toInt();
        case QVariant::UInt:
            return left.toUInt() < right.toUInt();
        case QVariant::LongLong:
            return left.toLongLong() < right.toLongLong();
        case QVariant::ULongLong:
            return left.toULongLong() < right.toULongLong();
        case QMetaType::Float:
            return left.toFloat() < right.toFloat();
        case QVariant::Double:
            return left.toDouble() < right.toDouble();
        case QVariant::Char:
            return left.toChar() < right.toChar();
        case QVariant::Date:
            return left.toDate() < right.toDate();
        case QVariant::Time:
            return left.toTime() < right.toTime();
        case QVariant::DateTime:
            return left.toDateTime() < right.toDateTime();
        case QVariant::String:
        default:
            if (isLocaleAware)
                return left.toString().localeAwareCompare(right.toString()) < 0;
            else
                return left.toString().compare(right.toString(), cs) < 0;
        }
    }
    //... ...
};
//
class RoundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RoundWidget(QWidget* parent = nullptr);

    void resizeEvent(QResizeEvent* event);
    //protected:
    //    void paintEvent(QPaintEvent *event);
};

class RoundWidget2 : public QWidget
{
    Q_OBJECT
public:
    explicit RoundWidget2(QWidget* parent = nullptr);

    void resizeEvent(QResizeEvent* event);
    //protected:
    //    void paintEvent(QPaintEvent *event);
};

//
class SettingsWgt :
    public RoundWidget //QWidget
{
    Q_OBJECT
public:
    SettingsWgt(QWidget* parent = nullptr);
    virtual ~SettingsWgt();

    /* static QString getMasterJobQueue();
     static QString getEngineJobQueue();*/
    static bool isFenbushi();

public slots:
    void funcUpdate();

    void funcPathMaster();
    void funcPathEngine();
    void funcClose();

signals:
    void SettingsClosed();
    void SettingsChanged();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;
    // settings.
    QLabel* lblJobQueue;

    QLabel* lblMaster;
    QLineEdit* leMaster;

    QLabel* lblEngine;
    QLineEdit* leEngine;

    QPushButton* butPathMaster;
    QPushButton* butPathEngine;

    QPushButton* butClose;
    QPushButton* butUpdate;
    QPushButton* butReset;

    QCheckBox* cbHttpServer;

    QString currMasterPath;
    QString currEnginePath;
    bool bHttpServerMode;
    bool bHasUpdated;
    bool bCloseButtonClicked;

    /*  static QSettings* pSettings;*/
};
// 提示窗口
class SettingsPrompt :
    public RoundWidget
{
    Q_OBJECT
public:
    SettingsPrompt(QWidget* parent = nullptr);
    virtual ~SettingsPrompt();

public slots:
    void funcTimeout();
    void funcClose();

    //signals:
    //    void SettingsClosed();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;

    QLabel* lblPrompt;
    QLabel* lblCloseTime;
    QPushButton* butClose;

    QTimer* pTimer;
    QTime* pTime;
};

class LoadingPrompt :
    public RoundWidget
{
    Q_OBJECT
public:
    LoadingPrompt(QWidget* parent = nullptr);
    virtual ~LoadingPrompt();

public slots:
    void funcTimeout();
    void funcClose();

    //signals:
    //    void SettingsClosed();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;

    QLabel* lblPrompt;
    QLabel* lblCloseTime;
    QPushButton* butClose;

    QTimer* pTimer;
    QTime* pTime;
};

class LoadingPromptV2 :
    public RoundWidget
{
    Q_OBJECT
public:
    LoadingPromptV2(QWidget* parent = nullptr, QString strInformation = "");
    virtual ~LoadingPromptV2();

public slots:
    void funcTimeout();
    void funcClose();

    //signals:
    //    void SettingsClosed();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;

    QLabel* lblPrompt;
    QLabel* lblCloseTime;
    QPushButton* butClose;

    QTimer* pTimer;
    QTime* pTime;
    QString strDefault;
};

void OpenLoadingPromptV2(QString strInformation);
void CloseLoadingPromptV2();

class LoadingPromptV4 :
    public RoundWidget
{
    Q_OBJECT
public:
    LoadingPromptV4(QWidget* parent = nullptr, QString strInformation = "");
    virtual ~LoadingPromptV4();

public slots:
    void funcTimeout();
    void funcClose();

    //signals:
    //    void SettingsClosed();

private:
    void closeEvent(QCloseEvent* evt);

private:
    QVBoxLayout* vlayout;

    QLabel* lblPrompt;
    QLabel* lblCloseTime;
    QPushButton* butClose;

    QTimer* pTimer;
    QTime* pTime;
    QString strDefault;
    int iState;
};

class MoreSettings : public QDialog
{
    Q_OBJECT
public:
    MoreSettings(QWidget* parent = nullptr, AI3D::CORE::ReconstructionObject* recons_object_ = nullptr, bool bReadOnly = false);
    ~MoreSettings();

public:
    void Slot_Ok();
    void Slot_Cancel();
    void Slot_Close();
public:
    void setGeometricPrecisionExtra();
    void setHoleFillingExtra();
    void setUntexturedRegionsExtra();

private:
    AI3D::CORE::ReconstructionObject* recons_object_ = nullptr;
    QPushButton* butClose;
    QPushButton* butOk;
    QPushButton* butCancel;
    QComboBox* cbbGeometricPrecision;
    QComboBox* cbbHoleFilling;
    QComboBox* cbbUntexturedRegions;
    bool bReadOnly = false;
};

void OpenLoadingPromptV4(QString strInformation);
void CloseLoadingPromptV4();

class ParamSettings4Production;

class BasicSettings : public QWidget
{
    Q_OBJECT
public:
    BasicSettings(ParamSettings4Production* parent = nullptr);
    ~BasicSettings();

    void Init();
    void SetErrorInfoVisible(bool bVis);
    void RefreshErrorInfo();
    void ChangeEnabledState4NextButton();

public:
    std::string name_;
    std::string desination_;
public slots:
    void Slot_SetName();
    void Slot_SetDestination();
    void Slot_ChooseFolder();
    void Slot_TextEdited();

public:
    QVBoxLayout* vlMain;
    QLabel* lblProductionTitle;
    QLabel* lblProductionID;
    QLabel* lblName;
    QLineEdit* leName;
    QLabel* lblDestination;
    QLineEdit* leDestination;
    QPushButton* butDestination;
    QLabel* lblError;
    bool bErrorInfoVisible;
    production_t production_id_ = kInvalidProductionId;
    ParamSettings4Production* paramSettings4Production;
};

class Purpose4ProductionDefinition : public QWidget
{
    Q_OBJECT
public:
    Purpose4ProductionDefinition(ParamSettings4Production* parent = nullptr);
    ~Purpose4ProductionDefinition();

    void Init();
    void Reset();

public slots:
    void Slot_ReferenceModel();
    void Slot_Export3DMesh();
    void Slot_Export3DPointCloud();
    void Slot_ExportOrthophotoDSM();
    // void Slot_Export3DMesh4ExternalRetouching();
    void Slot_ExportPiontCloudGS();
    void Slot_ExportOption();

private:
    QLabel* lblTitle;
    QCheckBox* cbReferenceModel;
    QCheckBox* cbExport3DMesh;
    QCheckBox* cbExportPointCloudGS;
    QCheckBox* cbExport3DPointCloud;
    QCheckBox* cbExportOrthophotoDSM;
    // QCheckBox* cbExport3DMesh4ExternalRetouching;
    ParamSettings4Production* paramSettings4Production;
};

class Export3DMesh_FormatWithOptions : public QWidget
{
    Q_OBJECT
public:
    Export3DMesh_FormatWithOptions(ParamSettings4Production* parent = nullptr);
    ~Export3DMesh_FormatWithOptions();

    void Init();
    void Reset();
    void DefaultParams();
    void ValidParams();
    void SetInValid();
    bool IsValid();
    void SwitchLodType();
    void SwitchLodTypeAfterFormatSelection();
    void SwitchLodChecked();
    void SwitchIncTex();

public:
    AI3D::CORE::production_format_e format_ = AI3D::CORE::production_format_e::PRODUCTION_MESH_FORMAT_OSGB;
    bool withAcrossTile_ = false;//@attetnion modified by  20231211
    bool withLod_ = true;
    AI3D::CORE::mesh3d_lod_type_e lodType_ = AI3D::CORE::mesh3d_lod_type_e::MESH3D_LOD_ADAPTIVETREE;
    bool withTexMaps_ = true;
    int texturecompression_ = 75;
    int max_texture_size_ = 8192;
    bool withSparping_ = true;
    bool withSkirt_ = false;
    bool withOvelap_ = true;
    float skirtPix_ = 4.0;
    float tileOverLap_ = -1.0;
    float defaultTileOverlap_;
    int unit_ = 0;//0m,1unit
public slots:
    void Slot_Format(const QString& str);
    void Slot_LodChecked();
    void Slot_AcrossTiles();
    void Slot_LodType(const QString& str);
    void Slot_IncludeTexChecked();
    void Slot_TextureCompress();
    void Slot_MaxTexSize();
    void Slot_BeSharpenning();
    void Slot_BeSkirt();
    void Slot_SkirtValue();
    void Slot_BeOverlap();
    void Slot_OverlapValue();
private:


    QVBoxLayout* vlTop;
    QLabel* lblTitle;

    // format
    QHBoxLayout* hlFormat;
    QLabel* lblFormat;
    QComboBox* cbbFormat;

    // level of detail(LOD)
    QCheckBox* cbLevelOfDetail;
    QHBoxLayout* hlGenerateLODAcrossTiles;
    QLabel* lblGenerateLODAcrossTiles;
    QCheckBox* cbGenerateLODAcrossTiles;

    QHBoxLayout* hlLODType;
    QLabel* lblLODType;
    QComboBox* cbbLODType;

    // include texture maps
    QCheckBox* cbIncludeTextureMaps;
    QHBoxLayout* hlTextureCompression;
    QLabel* lblTextureCompression;
    QComboBox* cbbTextureCompression;

    QHBoxLayout* hlMaximumTextureSize;
    QLabel* lblMaximumTextureSize;
    QLineEdit* leMaximumTextureSize;
    QLabel* lblMaximumTextureSizePixel;

    QHBoxLayout* hlSharpening;
    QLabel* lblSharpening;
    QCheckBox* cbSharpening;

    // skirt
    QHBoxLayout* hlSkirt;
    QCheckBox* cbSkirt;
    QLineEdit* leSkirt;
    QLabel* lblSkirtPixel;

    QHBoxLayout* hlOverlap;
    QCheckBox* cbOverlap;
    QLineEdit* leOverlap;
    QLabel* lblOverlap;

    bool bLodType = true;
    bool bInited = false;
};

class ExportOrthophoto_DSM : public QWidget
{
    Q_OBJECT
public:
    ExportOrthophoto_DSM(ParamSettings4Production* parent = nullptr);
    ~ExportOrthophoto_DSM();

    void Init();
    void Reset();
    bool IsValid() { return valid_; }
    void SetValid(bool valid) { valid_ = valid; }
    void DefaultParams();
    void SwitchDSMChecked();
    void SwitchOrthophotoChecked();

    int max_image_dim_ = 4096;
    bool withTDOM_ = true;

    AI3D::CORE::tdom_format_e tdom_format_ = AI3D::CORE::tdom_format_e::TDOM_FORMAT_TIFFGEOTIFF;
    float sampling_distance_;
    float default_sampling_distance_;
    bool withDSM_ = true;
    AI3D::CORE::dsm_format_e dsm_format_ = AI3D::CORE::dsm_format_e::DSM_FORMAT_TIFFGEOTIFF;
    AI3D::CORE::tdom_mode_e tdommode_ = AI3D::CORE::tdom_mode_e::NORMAL;
public slots:
    void Slot_ModeChanged(const QString&);
    void Slot_DSMFORMATChanged(const QString&);
    void Slot_TDOMFORMATChanged(const QString&);
    void Slot_SetMaxImageDim();
    void Slot_SetRes();
    void Slot_DSMChecked();
    void Slot_OrthophotoChecked();

private:
    QVBoxLayout* vlTop;
    QLabel* lblTitle;
    QHBoxLayout* hlResolution;
    QLabel* lblResolution;
    QLineEdit* leResolution;
    QHBoxLayout* hlMaximumImagePartDimension;
    QLabel* lblMaximumImagePartDimension;
    QLineEdit* leMaximumImagePartDimension;
    QHBoxLayout* hlMode;
    QLabel* lblMode;
    QComboBox* cbbMode;
    QCheckBox* cbOrthophoto;
    QHBoxLayout* hlOrthophotoFormat;
    QLabel* lblOrthophotoFormat;
    QComboBox* cbbOrthophotoFormat;
    QHBoxLayout* hlDSMFormat;
    QLabel* lblDSMFormat;
    QComboBox* cbbDSMFormat;
    QCheckBox* cbDSM;
    bool valid_ = false;
    bool bInited = false;
};

class Export_PointCloud_GS : public QWidget
{
    Q_OBJECT
public:
    Export_PointCloud_GS(ParamSettings4Production* parent = nullptr);
    ~Export_PointCloud_GS();
    void Init();
    void Reset();
    void DefaultParams();
    void ValidParams();
    void SetInValid();
    bool IsValid();
    AI3D::CORE::gs_scene_e scene_type_ = AI3D::CORE::gs_scene_e::GS_SCENE_FLY;
    AI3D::CORE::gs_3d_format_e format_ = AI3D::CORE::gs_3d_format_e::GS_3D_FORMAT_PLY;

public slots:
    void Slot_SceneFly();
    void Slot_SceneIndoor();
    void Slot_SceneObject();
    void Slot_Format(const QString& str);
    
private:
    ParamSettings4Production* paramSettings4Production;
    QVBoxLayout* vlTop;
    QLabel* lblTitle;
    QLabel* paramTitle;
    // format
    QHBoxLayout* hlFormat;
    QLabel* lblFormat;
    QComboBox* cbbFormat;

    QCheckBox* cbSceneFly;
    QCheckBox* cbSceneIndoor;
    QCheckBox* cbSceneObject;

    bool valid_ = false;
    bool bInited = false;
    
};

class Export3D_Point_Cloud : public QWidget
{
    Q_OBJECT
public:
    Export3D_Point_Cloud(ParamSettings4Production* parent = nullptr);
    ~Export3D_Point_Cloud();

    void Init();
    void DefaultParams();
    void Reset();
    bool IsValid() { return valid_; };
    void SetValid(bool valid) { valid_ = valid; };
    AI3D::CORE::production_format_e format_ = AI3D::CORE::production_format_e::PRODUCTION_POINTCLOUD_FORMAT_LAS;
    float samplingDistance_;
    float defaultSamplingDistance_;//需要外部计算好且是一个像素对应的距离
    //int samUnit_ = 0;//米
    int samUnit_ = 1;// 0为pixel 1为meter add by 
public slots:
    void Slot_Format(const QString&);
    void Slot_SamplingPointsUnit(const QString&);
    void Slot_SamplingPoints();

private:
    QVBoxLayout* vlTop;
    QLabel* lblTitle;
    QHBoxLayout* hlFormat;
    QLabel* lblFormat;
    QComboBox* cbbFormat;
    QHBoxLayout* hlSamplingPoints;
    QLabel* lblSamplingPoints;
    QLineEdit* leSamplingPoints;
    QComboBox* cbbSamplingPointsUnit;
    bool valid_ = false;
    bool bInited = false;
};
//
//class Export3DMesh4ExternalRetouching_FormatWithOptions : public QWidget
//{
//    Q_OBJECT
//public:
//    Export3DMesh4ExternalRetouching_FormatWithOptions(ParamSettings4Production* parent = nullptr);
//    ~Export3DMesh4ExternalRetouching_FormatWithOptions();
//
//    void Init();
//    void Reset();
//    void DefaultParams();
//    bool IsValid() { return format_ != ""; }
//
//    void SwitchIncludeTextureMaps();
//
//    std::string format_ = "OBJ";//为""代表无效
//    bool withTexMaps_ = true;
//    int texturecompression_ = 75;
//    int max_texture_size_ = 8192;
//    bool withSparping_ = true;  
//   
//
//public slots:
//    void Slot_TextureCompression(const QString &);
//    void Slot_IncludeTextureMaps();
//    void Slot_SharpeningChecked();
//    void Slot_MaxTextureSize();
//
//private:
//    QVBoxLayout* vlTop;
//    QLabel* lblTitle;
//
//    // format
//    QHBoxLayout* hlFormat;
//    QLabel* lblFormat;
//    ///QComboBox* cbbFormat;
//    QLineEdit* leFormat;
//
//    // include texture maps
//    QCheckBox* cbIncludeTextureMaps;
//    QHBoxLayout* hlTextureCompression;
//    QLabel* lblTextureCompression;
//    QComboBox* cbbTextureCompression;
//
//    QHBoxLayout* hlMaximumTextureSize;
//    QLabel* lblMaximumTextureSize;
//    QLineEdit* leMaximumTextureSize;
//    QLabel* lblMaximumTextureSizePixel;
//
//    QHBoxLayout* hlSharpening;
//    QLabel* lblSharpening;
//    QCheckBox* cbSharpening;  
//    bool bInited = false;
//};

class FormatWithOptions : public QWidget
{
    Q_OBJECT
public:
    FormatWithOptions(ParamSettings4Production* parent = nullptr);
    ~FormatWithOptions();

    void Init();

public slots:
    void Slot_Format(const QString& str);
    void Slot_LevelOfDetail();
    void Slot_IncludeTextureMaps();
    void Slot_Skirt();

private:
    QLabel* lblTitle;
    QLabel* lblFormat;
    QComboBox* cbbFormat;
    QCheckBox* cbLevelOfDetail;
    QCheckBox* cbIncludeTextureMaps;
    QCheckBox* cbSkirt;
    QLineEdit* lePixel;
    QLabel* lblPixel;
};

class SpatialReferenceSystem : public QWidget
{
    Q_OBJECT
public:
    SpatialReferenceSystem(ParamSettings4Production* parent = nullptr);
    ~SpatialReferenceSystem();

    void Init();
    void SetComSrsAsCurrent(QComboBox* pComboBox);
    void InitSRS(QComboBox* pComboBox, bool bSetCurrentItem4Recent = false);
    void Reset();
    void  SetSrsUnEditable();
    void DefaultParams();
    bool IsValid() { return valid_; };
    void SetValid(bool valid) { valid_ = valid; };
    std::string definition_ = srs_s().definition;
    std::set<std::string> srsdefinitionvec_;//为了把enu的加进入临时家的
    std::string default_definition_;
    bool shouldSetOrigin_ = true;
    Eigen::Vector3d coor_origin_ = Eigen::Vector3d{ 0.0,0.0,0.0 };//默认的自动的；
    Eigen::Vector3d coor_origin_custom_ = Eigen::Vector3d{ 0.0,0.0,0.0 };
    bool inAutoMode = true;

public slots:
    void Slot_OriginSetting(const QString&);
    void Slot_SRSChanged(const QString&);
    void Slot_SrsSelected(QString& srs);
    void Slot_SrsRestore();

private:
    QVBoxLayout* vlTop;
    QLabel* lblTitle;
    QLabel* lblSRS;
    QComboBox* cbbSRS;
    QLabel* lblOriginSetting;
    bool valid_;
    QString previous_srs;
};

class TilingRange : public QWidget
{
    Q_OBJECT
public:
    TilingRange(ParamSettings4Production* paramSettings4Production = nullptr, QWidget* parent = nullptr);
    ~TilingRange();

    void Init();
    void Reset();

    //EIGEN_STL_UMAP(std::string, AI3D::CORE::tile_info_s) tiles_;
    std::vector<std::string> tiles_selected_;
    // std::vector<std::string> tiles_;
public slots:
    void Slot_ClickToSelect(const QString&);

private:
    QVBoxLayout* vlTop;
    QLabel* lblTitle;
    QHBoxLayout* hlTiling;
    QLabel* lblTiling;
    QLabel* lblTiles;
    QLabel* lblTilingSuffix;
    QLabel* lblClickToSelect;
    ParamSettings4Production* paramSettings4Production;
};

class TilesList;

class ParamSettings4Production : public QDialog
{
    Q_OBJECT
public:
    ParamSettings4Production(AI3D::CORE::production_option_s options, QWidget* parent = nullptr, QString strTitle = "", AI3D::CORE::BlockObject* block_data_ = nullptr,
        AI3D::CORE::ReconstructionObject* recons_object_ = nullptr);
    ~ParamSettings4Production();

    void closeEvent(QCloseEvent* event);
    void DisplaySelectedTab();

    void DoNextInsideBasicSettings();
    void DoNextInsidePurpose();
    void DoNextInsideFormatWithOptions();

    void DoNextInsideSpatialReferenceSystem();
    void DoNextInsideTilingRange();
    void Submit();
    AI3D::CORE::production_option_s GetOptions() { return options_; }
    static AI3D::CORE::production_option_s GetSavedOptions() { return saved_options_; }
public slots:
    void Slot_Next();
    void Slot_Cancel();
    void Slot_Close();
    void Slot_BasicSettings();
    void Slot_Purpose();
    void Slot_FormatWithOptions();
    void Slot_SpatialReferenceSystem();
    void Slot_RefreshSelectedTab();
    void Slot_TilingRange();

signals:
    void signal_done_options(bool bOkResult = false);

public:
    AI3D::CORE::production_purpose_e production_purpose;
    bool purpose_chosen_dirty; // need to reset related format option page to default state if true.
    AI3D::CORE::ReconstructionObject* recons_object;
    AI3D::CORE::gs_scene_e scene_type;
    bool scene_chosen_dirty;

    //
    QStackedWidget* stackedWidget;

    BasicSettings* basicSettings;
    Purpose4ProductionDefinition* purpose;
    FormatWithOptions* formatWithOptions;
    Export3DMesh_FormatWithOptions* export3DMesh_FormatWithOptions;
    Export3D_Point_Cloud* export3D_Point_Cloud;
    ExportOrthophoto_DSM* exportOrthophoto_DSM;
    // Export3DMesh4ExternalRetouching_FormatWithOptions* export3DMesh4ExternalRetouching_FormatWithOptions;
    Export_PointCloud_GS* export_PointCloud_GS;
    SpatialReferenceSystem* spatialReferenceSystem;
    TilingRange* tilingRange;
    TilesList* tilesList;
    QPushButton* butNext;

private:
    AI3D::CORE::BlockObject* block_data_;
    QLabel* lblTitle;
    QPushButton* butClose;

    QPushButton* butCancel;
    QString strTitle;

    QVector<QWidget*> vecWidget;

    QPushButton* butBasicSettings;
    QPushButton* butPurpose;
    QPushButton* butFormatWithOptions;
    QPushButton* butSpatialReferenceSystem;
    QPushButton* butTilingRange;

    int iSelectedTabPos;
    AI3D::CORE::production_option_s options_;
public:
    static AI3D::CORE::production_option_s saved_options_;
};

int OpenParamSettings4Production(AI3D::CORE::production_option_s& options, QString strTitle = "Product Definition", int w = 900, int h = 570, AI3D::CORE::BlockObject* block_data_ = nullptr,
    AI3D::CORE::ReconstructionObject* recons_object_ = nullptr, AI3D::GUI::ConstructionWgt* construction_wgt_ = nullptr);
void CloseParamSettings4Production();


class SpatialReferenceSystemOriginSetting : public QDialog
{
    Q_OBJECT
public:
    SpatialReferenceSystemOriginSetting(SpatialReferenceSystem* srs, QWidget* parent = nullptr);
    ~SpatialReferenceSystemOriginSetting();

    void closeEvent(QCloseEvent* event);
    void anySet();
    Eigen::Vector3d coor_origin_auto = Eigen::Vector3d::Zero();
    Eigen::Vector3d coor_origin_custom = Eigen::Vector3d::Zero();
    bool inAutoMode = true;
public slots:
    void Slot_OK();
    void Slot_Cancel();
    void Slot_AutoSet();
    void Slot_CustomSet();
    //void Slot_InputValue();
    void Slot_Close();
    void Slot_XYZChanged();

private:
    QString strTitle;
    QLabel* lblTitle;
    QPushButton* butClose;

    QRadioButton* rbAutoSet;
    QLineEdit* leAutoSetX;
    QLineEdit* leAutoSetY;
    QLineEdit* leAutoSetZ;

    QRadioButton* rbCustomSet;
    QLineEdit* leCustomSetX;
    QLineEdit* leCustomSetY;
    QLineEdit* leCustomSetZ;

    QPushButton* butOk;
    QPushButton* butCancel;
    SpatialReferenceSystem* srs;
};

void OpenOriginSettings(SpatialReferenceSystem* srs, QWidget* parent_ = nullptr);

class YesNoCancelDialog : public QDialog
{
    Q_OBJECT
public:
    YesNoCancelDialog(QWidget* parent = nullptr, QString strTitle = "");
    ~YesNoCancelDialog();

    void Init();

public:

public slots:
    void Slot_Yes();
    void Slot_No();
    void Slot_Cancel();
    void Slot_Close();

private:
    QLabel* lblTitle;
    QPushButton* butClose;
    QPushButton* butYes;
    QPushButton* butNo;
    QPushButton* butCancel;
    QString strTitle;
};

int OpenYesNoCancelDialog(QWidget* parent = nullptr, QString strTitle = "", int w = 540, int h = 161);
void CloseYesNoCancelDialog();

class OkDialog : public QDialog
{
    Q_OBJECT
public:
    OkDialog(QWidget* parent = nullptr, QString strTitle = "");
    ~OkDialog();

    void Init();

public:

public slots:
    void Slot_Ok();
    void Slot_Close();

private:
    QLabel* lblTitle;
    QPushButton* butClose;
    QPushButton* butOk;
    QString strTitle;
};

int OpenOkDialog(QWidget* parent = nullptr, QString strTitle = "", int w = 540, int h = 60);
void CloseOkDialog();

///class TilesList : public QDialog
class TilesList : public QWidget
{
    Q_OBJECT
public:
    TilesList(AI3D::CORE::ReconstructionObject* recons_object_ = nullptr, QWidget* parent = nullptr, ParamSettings4Production* paramSettings4Production = nullptr);
    ~TilesList();

    void SetTilesSelected();
    void TilesListClicked();
    void RefreshEditMode();
    void SetLayerType();
    void SetSelectionModeExtra();
    void SetSelectItems();
    void InitTileListItem();
    void ClearOsgData();
    void Reset();

public slots:
    void Slot_Close();

    void Slot_SelectAll(const QString&);
    void Slot_Invert(const QString&);
    void Slot_SelectTypes();
    void Slot_SelectionModeChanged(const QString&);
    void Slot_Display3DView();
    void Slot_SelectedTiles(std::vector<image_t>& tiles);
    void Slot_ItemSelectionChanged();
    void Slot_EditModeChanged();

private:
    AI3D::CORE::ReconstructionObject* recons_object;
    int tilescount_;
    ParamSettings4Production* paramSettings4Production;
    AI3D::GUI::MWindow* mWindow;
    QPushButton* butClose;
    ///    QComboBox* cbbImageLayer;
    QCheckBox* cbPhotos;
    QCheckBox* cbTiePoints;
    QCheckBox* cbGCP;
    QCheckBox* cbTiling;
    QCheckBox* cbROI;
    QCheckBox* cbConstraints;

    QComboBox* cbbSelectionMode;

    QTableWidget* twTiles;
    QLabel* lblTilesSummary;
    QLabel* lblTilesSuffix;
    QLabel* lblSelectAll;
    QLabel* lblInvert;
    QPushButton* butEdit;
    bool bCanBeEditable;
    bool bOsgEngineCleared;
};

class UserTiePoints : public QDialog
{
    Q_OBJECT
public:
    UserTiePoints(AI3D::GUI::ViewWidget* viewWidget, AI3D::CORE::Image& image, QWidget* parent = nullptr);
    ~UserTiePoints();

public slots:
    void Slot_Close();
    void Slot_Add();
    void Slot_Cancel();

signals:
    void signal_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName);
    void signal_insert_gcp_tab();

private:
    QLabel* lblTitle;
    QPushButton* butClose;
    QLabel* lblName;
    QLineEdit* leName;
    QLabel* lblType;
    QComboBox* cbbType;
    QPushButton* butAdd;
    QPushButton* butCancel;
    AI3D::GUI::ViewWidget* viewWidget;
    AI3D::CORE::Image image;
};

int OpenTilesList(ParamSettings4Production* paramSettings4Production, AI3D::CORE::ReconstructionObject* recons_object_ = nullptr, QWidget* parent = nullptr, int w = 1329, int h = 842);
void CloseTilesList();
void HideTilesList();
void OpenMoreSettings(AI3D::CORE::ReconstructionObject* recons_object_ = nullptr, bool bReadOnly = false);
int OpenUserTiePoints(AI3D::GUI::ViewWidget* viewWidget, AI3D::CORE::Image& image, QWidget* parent = nullptr, int w = 375, int h = 240);
void CloseUserTiePoints();

class MoDelegate;

struct gcp_list_item_st
{
    gcp_list_item_st()
    {
        bHasImageId = false;
    }

    int color_;
    int ControlpointsID;
    QString name_;
    int photos_;
    QString category_;

    double given_x_;
    double given_y_;
    double given_z_;
    QString str_given_x_;
    QString str_given_y_;
    QString str_given_z_;

    double esitmated_x_;
    double esitmated_y_;
    double esitmated_z_;
    QString str_esitmated_x_;
    QString str_esitmated_y_;
    QString str_esitmated_z_;

    double rms_pix_;
    double rms_dis_;
    double error_3d_;
    double error_3d_xy_;
    double error_3d_z_;

    QString str_rms_pix_;
    QString str_rms_dis_;
    QString str_error_3d_;
    QString str_error_3d_xy_;
    QString str_error_3d_z_;
    int ControlpointsImageID;
    bool bHasImageId;
};
//gcp_list_item_st;
//gcp_list_item_st;

///typedef 
class gcp_measurement_list_item_st
{
public:
    gcp_measurement_list_item_st() {}
    int color_;
    int ControlpointsImageID;
    QString _photo_name;
    QString preview_name_;
    double x_;
    double y_;
    double rms_pix_;
    double rms_dis_;
    QString str_x_;
    QString str_y_;
    QString str_rms_pix_;
    QString str_rms_dis_;
    bool check_;
    int type_;//0gcp;1;check,2:user
    double estimated_x_ = -DBL_MAX;//预测的坐标
    double estimated_y_ = -DBL_MAX;//预测的坐标
    int width = 0;
    int height = 0;

    static bool compareLessThan(const gcp_measurement_list_item_st& t1, const gcp_measurement_list_item_st& t2);
};
//gcp_measurement_list_item_st;

//bool operator < (const gcp_measurement_list_item_st& t1, const gcp_measurement_list_item_st& t2);

//typedef 
struct gcp_preview_list_item_st
{
    QString _photo_name;
    int color_;
    bool check_;
    int type_;
    QString _photo_title;
    QString _photo_tip;
};
//gcp_preview_list_item_st;

typedef enum column_gcp2_e
{
    COLOR2_COL = 1,
    NAME2_COL,
    PHOTO2_COL,
    CATEGORY2_COL,
    RMS_PIX2_COL,

    ERROR3D2_COL,
    ERROR3D_H2_COL,
    ERROR3D_V2_COL,
    X2_COL,
    Y2_COL,
    Z2_COL,
    EST_X2_COL,
    EST_Y2_COL,
    EST_Z2_COL,
    COUNT_GCPCOL,
}col_gcp2_e;

typedef enum column_measurement2_e
{
    COLOR3_COL = 1,
    NAME3_COL,
    X3_COL,
    Y3_COL,
    RMS_PIX3_COL,
    //RMS_DIST3_COL,
    COUNT_MEASCOL
}col_measurement2_e;
//#endif
//? 
//1:滚动条
//2:表头排序的逻辑，貌似单击photo 和rmx排序结果不太对
class MoTableWidget : public QTableView
{
    Q_OBJECT
public:
    explicit MoTableWidget(QWidget* parent = nullptr, int mode = 0);
    virtual ~MoTableWidget();

    void printInfo();
    bool bLeaved;
    int getMode();

    void clearData();

    int getColCount();
    QStringList& getheaderLabels() { return headerLabels; }

    QList<gcp_list_item_st>& getGcpListData() { return gcpListData; }

    void removeOneRow(int row);
    void selectOneRow(int row);

    void selectOneRowByGcpId(int gcpId);
    void selectOneRowByImageId(uint32_t imageId);

    void appendRowData(gcp_list_item_st& gcpListItem);
    void updateRowData(int row, gcp_list_item_st& gcpListItem);
    int findRowByControlId(uint64_t gcp_id);
    uint64_t getGcpIdByRow(int row);
    uint32_t getImageIdByRow(int row);

    void appendRowData(gcp_measurement_list_item_st& gcpMeasurementItem);

    QList<gcp_measurement_list_item_st>& getGcpMeasurementListData() { return gcpMeasurementListData; }
    void setGcpMeasurementListData(QList<gcp_measurement_list_item_st>& gcpMeasurementListData);

    void setHeaderLabelsMode(bool bWGS84 = false);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

protected:
    void leaveEvent(QEvent* event);
    void updateRow(int row);
    void hideEvent(QHideEvent* event);


public slots:
    void cellEntered(int, int);

    void cellEntered2(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);
    void Slot_itemModified(int, int, const QString&) const;

signals:
    void itemModified(int, int, const QString&) const;

private:
    QColor previousHoverRowBackColor;
    int previousHoverRow;

    QColor origBackColor0; // alternative color0
    QColor origBackColor1; // alternative color1
    QColor origBackColor2; // selection color
    QColor origBackColor3; // selection and hover color
    QColor origBackColor4; // hover color

    void setRowColor(int, QColor);

    int selectedRow;
    int iHoverRow;
    MoDelegate* pItemDelegate;

    QStandardItemModel* pStandardItemModel = nullptr;
    int mode = 0;
    int colCount;
    QStringList headerLabels;

    QList<gcp_list_item_st> gcpListData;
    QList<gcp_measurement_list_item_st> gcpMeasurementListData;
};

class MoDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MoDelegate(QWidget* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

public slots:
    // todo：add leave event?
    void cellEntered(int row, int col);
    void cellEntered2(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);

signals:
    void itemModified(int, int, const QString&) const;

private:
    MoTableWidget* pTableWidget;
    int iHoverRow;
};

class MoHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    MoHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~MoHeaderView();

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
    virtual QSize sizeHint() const;

private:
    MoTableWidget* pTableWidget;
};

class MoListWidget : public QListView
{
    Q_OBJECT
public:
    explicit MoListWidget(QWidget* parent = nullptr);
    virtual ~MoListWidget();

    void init();

    QList<gcp_preview_list_item_st>& getGcpPreviewListData() { return gcpPreviewListData; }
    void setGcpPreviewListData(QList<gcp_preview_list_item_st>& gcpPreviewListData);

    void clearData();
    QStandardItem* appendRowData(gcp_measurement_list_item_st& gcpMeasurementItem);
    void appendData(QMap<QString, QList<gcp_measurement_list_item_st>*>&);

    void selectOneRowByImageId(uint32_t imageId);
    //uint32_t getImageIdByRow(int row);

    virtual void resizeEvent(QResizeEvent* event);
    virtual void hideEvent(QHideEvent* event);
    int getVisibleRow();
    int getVisibleCol();
    void setBlockPath(const std::string& blockPath);
    std::string getBlockPath() { return blockPath; }

    void setImageFileList(QStringList& imageFileList);
    void startGenPreviewFileWatchTimer();

public slots:
    void funcClicked(const QModelIndex& index);

    void funcPreviewFileTimeout();

signals:
    void previewImg(const QModelIndex& index, QString& img, int specialX, int specialY);
    void previewPix(QPixmap& pix, int specialX, int specialY);

public:
    bool bLeaved;
    std::string blockPath;

    QStringList imageFileList;
    QStringList previewFileList;
    //线程中的图形列表是否处理完毕.
    bool bGenPreviewFileCompleted;
    QTimer* pGenPreviewFileTimer;
    int iPreviousGenPreviewFileNum;

    void arrangeGroupItems();

private:
    QList<QString> imagesList;
    QStandardItemModel* pStandardItemModel = nullptr;

    QList<gcp_preview_list_item_st> gcpPreviewListData;
    int previousWidth;
    int previousHeight;
    // 控制gcplistview集中加载图形列表文件,列表文件名准备完后允许后续更新显示.
    // 最新代码中实际用处不大了.
    bool bAllowUpdate;//?
    QMap<QString, QList<gcp_measurement_list_item_st>*> previewListMap;
    int hspace = 0;
    int hnum = 0;
};


#include <QGraphicsView>
#include <QGraphicsItem>



class MoListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MoListItemDelegate(QWidget* parent = nullptr);

    // QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

public slots:
    // todo：add leave event?
    void cellEntered(int row, int col);
    void cellEntered2(const QModelIndex& index);
    void doubleClicked(const QModelIndex& index);

private:
    MoListWidget* pListWidget;

    int iHoverRow;

};

class ChineseDict
{
public:
    static ChineseDict* getInstance();

private:
    ChineseDict();
    ~ChineseDict();

    static ChineseDict* pChineseDict;

};


