# Core 层分析文档（MoldAIData.dll）

## 一、概览

Core 层编译为 **MoldAIData.dll**，是整个 MoldAI 系统的共享基础库。它为上层（Engine、Task、GUI）提供：

| 类别 | 内容 | 文件数 |
|------|------|--------|
| **基础类型** | ID 别名、枚举、Eigen 类型、BBox/坐标结构 | ~1117行 (Types.h) |
| **文件 I/O** | 路径工具、UTF-8 文件流、文件锁、资源管理 | File.h + File.cpp |
| **数学/几何** | 统计函数、浮点比较、三角化、投影矩阵变换 | Math.h, AlgorithmBase.h |
| **数据结构** | ATData, BlockObject, ProjectObject, Point3D, Image, Camera, Track | ~15个头文件 |
| **任务系统** | TaskDescriptor, ATTaskInfo, task_info_s, 命令集类 | TaskDef.h, TaskInfo.h, ATCommandSet.h |
| **坐标系统** | PROJ 封装、SRS 管理、坐标变换 | CoordinateSystem.h + Proj/ (23个文件) |
| **序列化** | JSON/BIN 双格式、异或加密二进制、文件魔术头校验 | DataStruct.h, Rapidjson.h |
| **日志/配置** | glog 封装、CHECK 宏、Application 单例 | Logging.h, Application.h |
| **三维重建** | 重建/生产管线命令、分块、模型导出 | ReconstructionCommandSet.h, Tiling.h |

**编译**: CMake 构建，依赖 Eigen3、PROJ、GDAL、GEOS、glog、RapidJSON、OpenCV、pugixml、exiv2、SQLite、Boost、Qt6 等。

---

## 二、基础类型系统 (Types.h)

### 2.1 ID 别名体系

```
image_t          = uint32_t    图片ID
camera_t         = uint32_t    相机ID
point2D_t        = uint32_t    2D特征点ID
point3D_t        = uint32_t    3D点ID
group_t          = int         照片组ID
block_t          = int         Block ID
reconstruction_t = int         重建ID
production_t     = int         生产输出ID
srsid_t          = int         坐标参考系ID
```

**无效值哨兵**:
```cpp
kInvalidImageId    = std::numeric_limits<image_t>::max()      // 无效图片
kInvalidCameraId   = std::numeric_limits<camera_t>::max()     // 无效相机
kInvalidGroupId    = -1                                       // 无效组
kInvalidSrsId      = std::numeric_limits<unsigned int>::max() // 无效SRS
kInvalidPoint3DId  = ...                                      // 无效3D点
kNaN               = std::numeric_limits<double>::quiet_NaN() // 无效浮点数
```

### 2.2 Eigen 类型别名

```cpp
typedef Eigen::Matrix<double, 3, 4>      Matrix3x4d;   // 投影矩阵
typedef Eigen::Matrix<unsigned char, 3, 1> Vector3ub;  // RGB 颜色
typedef Eigen::Matrix<unsigned char, 4, 1> Vector4ub;  // RGBA 颜色
typedef Eigen::Matrix<double, 6, 1>      Vector6d;     // 6DOF 位姿
```

### 2.3 核心枚举

**Job 类型** (`jobtype_e`):
```cpp
JOB_AT    = 0  // 空三任务（AT pipeline: Feature→Match→SfM→Recon）
JOB_TILE  = 1  // 分块重建任务
JOB_BATCH = 2  // 批量预处理任务
```

**Job 状态** (`jobsta_e`):
```cpp
STATUS_PENDDING  // 等待执行
STATUS_RUNNING   // 执行中
STATUS_COMPLETE  // 完成
STATUS_CANCLE    // 取消
STATUS_FAILURE   // 失败
STATUS_UNKNOWN   // 未知
```

**任务类型常量**:
```cpp
ATSTARTTYPE             = 1  // 空三起始（需拆分）
ATRUNNINGTYPE           = 2  // 空三运行中
ATLASTTASKTYPE          = 3  // 空三最后任务
ATCOMPLETETYPE          = 0  // 完成标记
RECONSTRUCTIONSTARTTYPE = 4  // 重建起始
BATCHSTARTTYPE          = 5  // 批量预处理起始
```

**相机模型类型** (`CameraModelType_e`):
```
SIMPLE_PINHOLE, PINHOLE, SIMPLE_RADIAL, RADIAL,
OPENCV, OPENCV_FISHEYE, FULL_OPENCV, FOV,
SIMPLE_RADIAL_FISHEYE, RADIAL_FISHEYE, THIN_PRISM_FISHEYE
```

**SFM 模式** (`sfm_mode_e`):
```cpp
SFM_INCREMENTAL  // 增量式 SFM
SFM_GLOBAL       // 全局式 SFM
```

### 2.4 包围盒与空间结构

```cpp
struct bbox_s {
    double xmin_, ymin_, zmin_, xmax_, ymax_, zmax_;
    bool isValid();
    ABBox3d toABBox3d();
    void CreateJson / ParseJson;  // JSON 序列化
};

template<typename T> using ABBox2 = Eigen::AlignedBox<T, 2>;
template<typename T> using ABBox3 = Eigen::AlignedBox<T, 3>;
typedef ABBox2<double> ABBox2d;
typedef ABBox3<double> ABBox3d;
typedef ABBox2<float>  ABBox2f;
typedef ABBox3<float>  ABBox3f;

// 包围盒工具函数
PointsToBox();  ExtendBoundingBox();  ExtendBoundingBoxByRatio();
IsBoundingBoxValid();  MakeBoundingBoxValid();
BoundingBoxToInt();  BoundingBoxToPresicion();
```

### 2.5 SRS 定义

```cpp
struct srs_s {
    std::string definition;   // WKT/PROJ 定义字符串
    std::string name;         // 显示名
    int type;                // coord_system_type_e
    std::string CreateJson/ParseJson;  // JSON 序列化
};
```

### 2.6 BIN/JSON 格式切换宏

```
BLK_USE_BIN             → block 文件格式
TASK_USE_BIN            → task_def_N 文件格式
JOB_INFO_USE_BIN        → job 文件格式
JOB_FEEDBACK_USE_BIN    → feedback 文件格式
ENGINE_USE_BIN          → 引擎注册文件格式
STAT_USE_BIN            → 统计文件格式
SOURCEDATA_USE_BIN      → 源数据文件格式
```

---

## 三、文件 I/O 层 (File.h)

### 3.1 File 类（全静态方法）

| 类别 | 方法 | 用途 |
|------|------|------|
| **路径工具** | `EnsureTrailingSlash`, `EnsureUnifySlash`, `GetParentDir`, `GetFileName`, `GetFileNameWithoutExtension`, `GetFileExtension`, `SplitFileExtension` | 路径标准化 |
| **存在检查** | `ExistsFile`, `ExistsDir`, `ExistsPath` | 文件/目录存在性 |
| **删除** | `RemoveFile`, `RemoveFiles`, `Remove` (递归), `MakeDirEmpty` | 文件/目录清理 |
| **复制** | `CopyDirectory`, `CopySingleFile`, `CopyFiles`, `BackupFile` | 复制/备份 |
| **目录** | `CreateDirIfNotExists`, `GetFileList`, `GetRecursiveFileList`, `GetDirList`, `GetRecursiveDirList` | 目录扫描 |
| **UTF-8 流** | `OpenIfstreamUtf8`, `OpenOfstreamUtf8`, `ReadBinaryFileUtf8` | 跨平台 UTF-8 路径打开 |
| **文件锁** | `FopenDenyWriteLockUtf8` | Windows deny-write 排他锁 |
| **模板方法** | `JoinPaths` (可变参数), `CSVToVector`, `VectorToCSV`, `ReadBinaryBlob`, `WriteBinaryBlob` | 通用工具 |

### 3.2 文件锁机制

```cpp
// Windows: fopen + _SH_DENYWR → 独占写访问
static FILE* FopenDenyWriteLockUtf8(const std::string& path, const char* mode);
```

这是在文件系统层面实现多 Engine 实例互斥的关键基础设施。

### 3.3 RapidJsonCore (Rapidjson.h)

```cpp
class RapidJsonCore {
    static int ReadFile(path, string&);   // 读文本文件
    static int SaveFile(path, string&);   // 写文本文件
    static int SaveFile(path, Document&); // 写 JSON Document
    static std::string UTF8ToANSI(s);     // 编码转换
};
```

---

## 四、数学与算法基类

### 4.1 Math.h — 统计与工具函数

**浮点比较**（处理 NaN）:
```cpp
DoubleNearSig(a, b, digits)   // 基于有效位数的比较
NumberNear(a, b, epsilon)     // 基于 epsilon 的比较
NanCompatibleEquals(a, b)     // NaN-safe 相等
```

**统计函数**:
```cpp
Median(), Mean(), Variance(), StdDev(), Percentile()
```

**数值工具**:
```cpp
RadToDeg(), DegToRad()       // 角度弧度转换
Clip(), SignOfNumber()       // 裁剪/符号
Sigmoid(), ScaleSigmoid()    // 激活函数
NextCombination()            // 组合生成器
NChooseK()                   // 组合数
TruncateCast()               // 安全类型转换
```

**绘制工具**（OpenCV）:
```cpp
GetArrowPoints()  // 生成箭头多边形点集
DrawArrow()       // 在 cv::Mat 上绘制箭头
```

### 4.2 AlgorithmBase — 3D 视觉基础运算

```cpp
class AlgorithmBase {
    // 坐标系变换
    ProjectionCenterFromMatrix()     // 从投影矩阵提取光心
    ComposeProjectionMatrix(R, C)    // 从旋转+光心合成投影矩阵
    InvertProjectionMatrix()         // 反转投影矩阵
    QuaternionToRotationMatrix()     // 四元数→旋转矩阵
    RotationMatrixToQuaternion()     // 旋转矩阵→四元数
    ComposeIdentityQuaternion()      // 单位四元数

    // 投影与重投影
    TransformPointW2C(proj_matrix, pt)   // 世界→相机坐标
    TransformPointC2I(cam_matrix, pt)    // 相机→图像坐标
    TransformPointW2Iz(proj_matrix, cam_mat, pt)  // 世界→像素坐标
    CalculateReprojectionError()         // 重投影误差
    CalculateSquaredReprojectionError()  // 平方重投影误差
    CalculateNormalizedAngularError()    // 归一化角度误差

    // 三角化
    TriangulatePoint(proj1, proj2, pt1, pt2)         // 双视图三角化
    TriangulateMultiViewPoint(projs, pts)            // 多视图三角化
    TriangulateNViewAlgebraic()                      // N视图代数三角化
    CalculateTriangulationAngle()                    // 三角化夹角

    // 对极几何
    F_from_P(P1, P2)              // 从投影矩阵计算基础矩阵
    CalcEpipolarLine()            // 计算对极线
    LineToEndPoints()             // 对极线→图像端点

    // OPK 角
    ConvertOPK2Rotmat()           // Omega/Phi/Kappa → 旋转矩阵
    ConvertRotmat2OPK()           // 旋转矩阵 → OPK

    // 相似变换
    FindRTS()                     // 找相似变换 (Eigen::umeyama)
    Refine_RTS()                  // LM 精化相似变换
};
```

**Functor 模板**：通用的 Eigen LM 优化问题框架，用于 Ceres-like 的非线性最小二乘。

### 4.3 SimilarityTransform3 — 三维相似变换

```cpp
class SimilarityTransform3 {
    // 7参数: scale + quaternion(qx,qy,qz,qw) + translation(tx,ty,tz)
    SimilarityTransform3(scale, qvec, tvec);
    Estimate<kEstimateScale>(src, dst);  // 从对应点估计
    Inverse();
    TransformPoint(xyz*);
    TransformPose(qvec*, tvec*);
    Matrix();  Scale();  Rotation();  Translation();
};
```

`SimilarityTransformEstimator<kDim, kEstimateScale>` 模板类配合 RANSAC 使用。

---

## 五、核心领域数据模型

### 5.1 Camera — 相机模型

```cpp
class Camera {
    camera_t camera_id_;
    int model_id_;                    // 相机模型类型
    size_t width_, height_;           // 分辨率
    vector<double> params_;           // 内参+畸变参数
    bool prior_focal_length_;         // 是否有先验焦距
    double focal_length_, focal_lengthIn35mm_;
    std::string cam_maker_, cam_makermodel_;
    double sensor_size_, pixel_size_;
    double undistortedborder_[8];     // 去畸变后的有效边界
};
```

**关键方法**:
- `GetCalibrationMatrix()`: 3x3 内参矩阵 K
- `ImageToWorld/WorldToImage()`: 畸变校正后的像素↔归一化坐标变换
- `UndistortPixel()`: 单点去畸变
- `UndistortCamera()`: 生成去畸变后的新相机
- `InitializeWithId/InitializeWithName()`: 按相机模型初始化参数
- `HasBogusParams()`: 检测异常参数（焦距/畸变超出合理范围）
- `Rescale()`: 图像缩放后的参数调整

### 5.2 Image — 图像与位姿

```cpp
class Image {
    image_t image_id_;
    camera_t camera_id_;
    std::string name_, path_, preview_name_;
    bool registered_;                   // 是否已注册（成功空三）
    // 位姿
    Eigen::Vector3d center_;            // 投影中心 C
    Eigen::Matrix3d rotation_matrix_;   // 旋转矩阵 R
    // 先验位姿（GPS/POS）
    Eigen::Vector3d center_prior_;
    Eigen::Matrix3d rotation_matrix_prior_;
    srs_s prior_srs_def_;
    // 2D 特征点
    vector<Point2D> points2D_;
    // GCP 测量
    map<point3D_t, Eigen::Vector2d> points2D_gcp_;
    map<point3D_t, Eigen::Vector2d> points2D_userpt_;
    // 深度/视锥
    Eigen::Vector3d depth_;
    vector<Eigen::Vector3d> frustum_;
    // 图像属性
    int width_, height_;
    ExifInfo exifinfo_;
    XmpData xmpdata_;
    Eigen::Vector3d colorparam_;       // 色彩校正参数
};
```

**关键方法**:
- `GetProjectionMatrix()`: P = K[R | -RC] (3x4 投影矩阵)
- `InverseProjectionMatrix()`: P⁻¹
- `GetProjectionCenter()`: 光心 C
- `ViewingDirection()`: 视线方向
- `IsVisible(pt, cam_mat)`: 判断 3D 点是否在图像可见区域内
- `GenPreviewImage()`: 生成预览缩略图
- `ParseExif()`: 解析 EXIF/XMP 元数据（焦距、GPS、姿态等）

### 5.3 Point2D — 2D 特征点

```cpp
class Point2D {
    Eigen::Vector2d xy_;             // 图像坐标
    point3D_t point3D_id_;          // 关联的 3D 点 ID
    cv::Vec3b color_;               // 颜色 (BGR)
};
```

### 5.4 Point3D — 3D 点

```cpp
class Point3D {
    Eigen::Vector3d xyz_;            // 坐标
    Eigen::Vector3i color_;          // RGB 颜色
    Track track_;                    // 观测轨迹（跨多张图像的2D匹配）
    point3D_t id_;
    std::string name_;
    double pixel_rms_;              // 像素重投影 RMS
    double dist_rms_;               // 3D 距离 RMS
    ptt_e type_;                    // 点类型 (PT_NONE/TIE_POINT/GCP/USER_POINT)
    Eigen::Vector3d estimated_xyz_; // 估计坐标（用于 GCP）
};
```

### 5.5 Track — 观测轨迹

```cpp
struct TrackElement {
    image_t image_id;                // 观测图像
    point2D_t point2D_idx;          // 在该图像中的 2D 点索引
    Eigen::Vector2d xy;             // 像素坐标
};

class Track {
    vector<TrackElement> elements_;  // 多视图观测
    Length();                        // 轨迹长度（被多少图像观测到）
    AddElement/DeleteElement();      // 增删观测
    FindElementByImageId();          // 在特定图像中查找
};
```

### 5.6 ControlPoint — 控制点 (GCP)

```cpp
class ControlPoint {
    point3D_t id_;
    std::string name_;
    gpt_e type_;                     // FULL_XYZ / XY_ONLY / Z_ONLY
    Eigen::Vector3d xyz_;           // 给定大地坐标
    Eigen::Vector3d estimated_xyz_; // 空三平差后坐标
    Eigen::Vector2d weight_;        // 权重
    srs_s origin_srs_;             // 坐标参考系
    double error_3d_, error_3d_xy_, error_3d_z_;  // 3D/XY/Z 误差
};
```

**ControlPoints 容器**:
- `LoadXML/SaveXML()`: BlockExchange XML 格式
- `LoadText/SaveText()`: 文本格式
- `LoadJson/SaveJson()`: JSON 格式
- `TransformPoints()`: 坐标变换（PROJ/GDAL）
- `GetGCPCount/GetCheckPointCount()`: 检查点/控制点统计

### 5.7 ATData — 空三核心数据

```cpp
class ATData {
    // 核心集合
    vector<Camera> cameras_;
    vector<Image> images_;
    vector<Point3D> points3D_;
    vector<Point3D> user_points3D_;
    EIGEN_STL_UMAP(point3D_t, ControlPoint) controlpoints_;
    vector<Constraint> constraintList_;
    vector<ImagePair> image_pairs_;
    EIGEN_STL_UMAP(string, image_t) image_path_to_id_;

    // 双向索引（加速查询）
    vector<vector<point2D_t>> point_views_;    // 3D点 → 观测它的2D点列表
    vector<vector<point3D_t>> view_points_;    // 图像 → 其包含的3D点列表
    vector<image_t> reg_image_ids_;            // 已注册图像ID列表

    // 坐标参考系
    srs_s origin_srs_definition_;
    srs_s local_srs_definition_;
    srs_s local_gcp_srs_definition_;
};
```

**关键方法**:

| 类别 | 方法 | 说明 |
|------|------|------|
| **稳健对齐** | `AlignRobust<kEstimateScale>()` | LORANSAC + SimilarityTransform 对齐 |
| **简化** | `Simplify(options)` | 按选项过滤/优化空三结果 |
| **变换** | `Transform(similarity)`, `Normalize()` | 空间变换/归一化 |
| **预测** | `Predict()`, `PredictGCPMeasurement()` | 预测图像位姿/GCP测量 |
| **三角化** | `TriangulateTiePoints()` | 多视图三角化 |
| **去畸变** | `UndistortData()`, `UndistortImages()` | 去畸变处理 |
| **场景信息** | `GetSceneScale()`, `GetGSD()`, `ComputeAvgResolution()` | 场景尺度 |
| **包围盒** | `ComputeBoundsAndCentroid()`, `ComputeTileBoundingBox()` | 计算场景范围 |
| **导出** | `WritePoints3DText()`, `WriteImageText()` | 文本导出 |
| **质量** | `ComputeNumObservations()`, `ComputeMeanTrackLength()`, `ComputeMeanReprojectionError()` | 质量统计 |
| **GCP** | `ComputeSquaredReprojectionErrorForGCP()`, `Compute3DErrorForGCP()` | GCP 精度评估 |
| **提取** | `ExtractATDataByImages()`, `ExtractATDataByTiepoints()` | 子集提取 |
| **变换** | `TransformATData()`, `TransFormTiepoints()`, `TransFormImages()`, `TransFormGCPs()` | 坐标变换 |
| **预览** | `GenPreviewImages()` | 生成预览图 |

### 5.8 BlockObject — 数据区块

```cpp
class BlockObject {
    block_t id_;
    std::string name_, path_, description_;
    block_type_e type_;            // TYPE_NONE / CLONED / MERGED
    blk_status_e status_;          // BLKSTS_PENDDING ... BLKSTS_EMPTY
    // 照片组
    EIGEN_STL_UMAP(group_t, PhotoGroup) photogroups_;
    vector<PhotoGroup> ATGroups_;  // 空三分组
    // 空三数据
    shared_ptr<ATData> ATData_;
    // 重建
    EIGEN_STL_UMAP(reconstruction_t, ReconstructionObject) reconstructions_;
    // SRS
    srsid_t blockSRS_id_;
    EIGEN_STL_UMAP(srsid_t, srs_s) srs_map_;
    Eigen::Vector3d position_offset_;
    // 照片ID
    set<image_t> image_ids_;
};
```

**关键方法**:
- **图像管理**: `Addimages_Beta`, `RemoveImages`, `SearchImages`, `DeleteImages`
- **持久化**: `Load()`, `Save()`, `LoadBlockATData()`, `ExportBlockATData()`
- **AT I/O**: `LoadATBinary/ExportATBinary`, `LoadATXML/ExportATXML`
- **重建**: `AddReconstruction`, `GetReconstruction`, `CloneReconstruction`, `DeleteReconstruction`
- **SRS**: `GetBlockSRS`, `SetBlockSRS`, `ComputeEnuSRS`, `GenerateValidSrsId`
- **GCP**: `AddGCPs`, `ParseControlPoints`, `LoadGCPMeasurementsXML`
- **POS**: `AddPoses`, `ClearPoses`, `LoadPoseTxt`, `LoadPoseXLSX`
- **合并**: `MergeBlocks`, `MergeAndAdjustBlocks`, `InterSectionAdjustment`
- **批量处理**: `BatchPrePare`, `BatchPreProcess`, `ImagesRename`
- **去畸变**: `UndistortBlock`
- **AT 报告**: `ParseATReport`, `ExportATReport`, `GenerateATReportPicture`

### 5.9 ProjectObject — 工程容器

```cpp
class ProjectObject {
    std::string name_, path_;
    EIGEN_STL_UMAP(block_t, BlockObject*) blocks_;  // 所有 Block
    set<block_t> blockids_;
    set<srsid_t> srs_ids_;
};
```

**关键方法**: `NewProject`, `AddBlock`, `ImportBlock`, `DeleteBlock`, `CloneBlock`, `MergeBlocks`, `Load/Save` (JSON/BIN)

### 5.10 ReconstructionObject & ProductionObject

**ReconstructionObject**: 管理一个重建实例（分块信息、处理设置、生产输出列表）

**ProductionObject**: 管理一个生产输出（格式、目标路径、SRS、瓦片列表）

---

## 六、任务系统

### 6.1 TaskDescriptor — 任务定义

```cpp
struct TaskDescriptor {
    int id_, type_, fatherId_;
    std::string fun_name_;           // "RunFeatureDetection", "RunSfM" 等
    std::string msg_, name_;         // 显示名
    set<int> depends_;               // 依赖任务 ID
    vector<int> imgIds_;             // 分配的图片
    int key_maximage_num_;           // 关键点提取最大图数
    int match_maximage_num_;         // 匹配最大图数
    int sfm_task_num_, match_task_num_;  // 子任务拆分数量
    int sfmId_, match_id_;
    vector<int> matchIds_;           // 匹配子任务 ID 列表
    int sfmmem_;                      // SFM 内存预估
};
```

### 6.2 ATTaskInfo — Task 文件顶层结构

```cpp
struct ATTaskInfo {
    std::string job_;            // Job JSON
    std::string blockItem_;      // Block 路径
    std::string projectFile_;    // 项目文件路径
    std::string ATJson_;         // AT 数据 JSON 路径
    std::string GCPJson_;        // GCP 数据 JSON 路径
    TaskDescriptor task_;        // 任务描述

    // 序列化: LoadJson/LoadBin → 读取 disk task_def_N
    //          SaveJson/SaveBin → 写入 disk task_def_N
};
```

### 6.3 task_info_s — 任务信息（含配置）

```cpp
struct task_info_s {
    task_base_info_s base_info_;       // blockItem, job, projectPath, sdebug
    ATOptions at_options_;             // 空三参数
    task_metadata_s task_metadata_;    // id, type, function, msg, keyMaxImgNum, matchMaxImgNum
    ProductionOptions production_options_;  // 重建参数
};
```

**序列化**: `WriteToBin` → 使用 `SPTaskInfoFile` 二进制格式；`WriteToJson` → JSON 格式

### 6.4 ATCommandSet — 空三命令集

```cpp
class ATCommandSet {
    // 用户数据 I/O
    AddUserTiepoint, WriteUserTiepointsJson, ReadUserTiepointsJson;
    // GCP I/O
    WriteGCPMeasurementsJson/XML, ReadGCPMeasurementsJson/XML;
    // POS I/O
    WritePOSJson/Bin, ReadPOSJson/Bin;
    // 源数据 I/O
    SaveSourceDataJson/Binary, LoadSourceDataJson/Binary;
    // AT 文件
    LoadATBinary, ExportATBinary;
    // 创建
    CreateATFiles, CreateATTaskInfo;
    // 状态
    GetATCompleteStatus;
    // Block
    LoadBlock;
};
```

### 6.5 ReconstructionCommandSet — 重建命令集

```cpp
class ReconstructionCommandSet {
    SubmitReconstruction();              // 提交重建任务
    SubmitProduction();                  // 提交生产任务
    ResubmitProductionJob();             // 重新提交
    CanSubmitProduction/CanCancelProduction/CanResubmitProduction();
    GetJobsToCancelled();               // 获取待取消的 Job 列表
};
```

### 6.6 TaskCommandSet — 通用任务命令集

处理任务状态变更、取消、重试等通用任务操作。

---

## 七、坐标系统 (CoordinateSystem + Proj/)

### 7.1 CoordinateDescriptor

```cpp
class CoordinateDescriptor {
    coord_system_type_e type_;   // LOCAL / GEOGRAPHIC / PROJECTED / Unsupported
    int epsg_code_;
    std::string wkt_;
    Eigen::Vector3d origin_point_;
    GetSRSFromDefinition/wkt;     // WKT 字符串 → srs_s
    GetSRSFromName(name);         // 名称 → srs_s (查询内置DB)
    GetCGCS2000Code(lat_lon_alt); // 经纬度 → CGCS2000 投影代码
    IsGeode();                     // 是否是地理/GEO坐标系
};
```

### 7.2 CoordinateTransformer

```cpp
class CoordinateTransformer {
    Transform(x,y,z, src, dst);           // 单点变换
    Transform(numPoints, x[],y[],z[]);    // 批量点变换
    TransformBBox(box, src, dst);         // 包围盒变换
    Transform(poses, src, dst);           // 位姿变换
    TransformRotation(pose, rot, src, dst); // 旋转矩阵变换
    TransformByEnu();                     // 通过 ENU 中间系变换
    TransformByEpsgCode();                // 通过 EPSG 代码变换
    IsSame(src, dst);                     // 判断是否同一坐标系
};
```

**底层**: 使用 GeographicLib::LocalCartesian (ENU 本地系) + PROJ (全局坐标变换)

### 7.3 Proj/ 子模块（23 个文件）

封装了一套完整的 PROJ/GDAL 坐标系统:
- `CoordinateReferenceSystem`: CRS 定义管理
- `CoordinateReferenceSystemRegistry`: CRS 注册表
- `CoordinateTransform`: 坐标变换执行
- `CoordinateTransformContext`: 变换上下文
- `DatumTransform`: 基准面变换
- `ProjUtils`, `OgcUtils`: 工具函数
- `SqliteUtils`: PROJ 数据库查询 (proj.db)
- `QProj`: Qt 集成接口

---

## 八、配置与选项系统

### 8.1 ATOptions — 空三参数

```cpp
struct ATOptions {
    point2D_t feature_num = 20000;       // 特征点数量
    pair_selection_mode_e reconstruct_mode; // 匹配对选择模式
    sfm_align_mode_e align_mode;         // 对齐模式
    sfmsettings_s sfmsettings;           // SFM 设置
    AT_saveoption_s saveoptions;         // 保存选项
    int maxthreads_num;                   // 最大线程数
};

struct sfmsettings_s {
    BA_estimation_polices_s bapolicies;  // BA 策略
    int grid_count_1, grid_count_2;      // BA 网格参数
    int max_feature_count_1, max_feature_count_2;
    sfm_mode_e sfm_mode;                 // Global / Incremental
    Eigen::Vector3d pos_sigma;           // POS 先验精度
};

struct BA_estimation_polices_s {
    // 控制哪些参数参与优化: Compute / Adjust / Keep
    policies_e tiepoints_policy_, pos_policy_, f_policy_;
    policies_e ppa_policy_, rdis_policy_, tdis_policy_;
    bool use_gcp_, use_user_tiepoints_, use_constraints_, use_image_position_;
};
```

### 8.2 Application 单例

```cpp
class Application {
    static Application& Getinstance();
    // 路径
    GetAPPPath(), GetCameraDBPath(), GetProjDBPath(), GetGDALPath();
    GetLogPath(), GetConfigPath(), GetEnginePath();
    // 环境
    SetUpGDALSettings(), SetProjLibENV();
    // 配置
    GetConfigFile(), ExportConfig(), ParseConfig();
    // 功能开关
    isGDGSEnable(), isBaseGSEnable();    // Gaussian Splatting
    isTDOM_DSMEnable();                   // TDOM/DSM
};
```

### 8.3 日志系统 (Logging.h)

```cpp
// 基于 glog 的封装
LOGW/LOGE/LOGI/LOGD/LOGV/LOGF(message)
LogFile(message, level)

// 运行时检查宏 (release 模式也生效)
CHECK_OPTION(condition);  CHECK_OPTION_EQ/NE/LE/LT/GE/GT();
CHECK_LOG(condition);     CHECK_LOG_EQ/NE/LE/LT/GE/GT();
CHECK_OPTION_NOTNULL(val);
```

### 8.4 String 工具类 (String.h)

```cpp
class String {
    LocaleToUtf8/Utf8ToLocale();     // Boost locale 编码转换
    ReadFileToString/SaveFileFromString();
    StringPrintf(format, ...);        // printf-format 字符串
    StringReplace/Split/Trim/ToUpper/ToLower/StartsWith/Contains();
    ToSHA256();                       // SHA-256 哈希
};
```

---

## 九、数据结构关系总图

```
┌─────────────────────────────────────────────────────────────┐
│                     Core Layer (MoldAIData.dll)              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────────────┐       │
│  │ Types.h  │    │ File.h   │    │ DataStruct.h      │       │
│  │ ID/枚举   │    │ 路径/IO   │    │ BIN序列化格式      │       │
│  │ Eigen类型 │    │ 文件锁    │    │ (SPTaskInfoFile,  │       │
│  │ BBox/SRS  │    │ UTF-8流  │    │  FeedBackFile,    │       │
│  └──────────┘    └──────────┘    │  PIDFile...)      │       │
│                                  └──────────────────┘       │
│  ┌──────────────────────────────────────────┐               │
│  │          领域数据模型                       │               │
│  │  Camera ←─ Image ←─ Point2D               │               │
│  │     ↓          ↓         ↘                │               │
│  │  Track ───→ Point3D ←── ControlPoint       │               │
│  │     ↓                                      │               │
│  │  ATData (全部数据的聚合容器)                  │               │
│  │     ↓                                      │               │
│  │  BlockObject (区块: 含 AT + 重建 + 多SRS)    │               │
│  │     ↓                                      │               │
│  │  ProjectObject (项目: 含多个 Block)          │               │
│  └──────────────────────────────────────────┘               │
│                                                              │
│  ┌──────────────────────────────────────────┐               │
│  │          任务/管线层                        │               │
│  │  TaskDescriptor / ATTaskInfo              │               │
│  │  ATCommandSet / ReconstructionCommandSet  │               │
│  │  task_info_s (含 ATOptions + ProductionOptions) │        │
│  └──────────────────────────────────────────┘               │
│                                                              │
│  ┌──────────────────────────────────────────┐               │
│  │          数学/算法层                        │               │
│  │  Math.h (统计/比较/量化)                    │               │
│  │  AlgorithmBase (投影/三角化/对极)            │               │
│  │  SimilarityTransform3 (7参数相似变换)        │               │
│  └──────────────────────────────────────────┘               │
│                                                              │
│  ┌──────────────────────────────────────────┐               │
│  │          基础设施                           │               │
│  │  CoordinateSystem + Proj/ (坐标变换)        │               │
│  │  Tiling (分块策略: None/Regular2D/3D/Adaptive)│            │
│  │  ExifIO (Exif/XMP 解析)                   │               │
│  │  CameraDatabase (相机传感器DB)              │               │
│  │  Application (单例: 路径/配置/环境)          │               │
│  │  Logging (glog), String (工具), RapidJson   │               │
│  └──────────────────────────────────────────┘               │
└─────────────────────────────────────────────────────────────┘
```

---

## 十、依赖关系

```
MoldAIData.dll 依赖:
├── Eigen3          → 所有 3D 数学运算
├── PROJ            → 坐标参考系定义与变换
├── GDAL            → 地理空间数据 (OGRSpatialReference)
├── GEOS            → 几何运算
├── RapidJSON       → JSON 解析/生成
├── glog            → 日志输出
├── OpenCV          → 图像处理/预览
├── pugixml         → XML 配置/BlockExchange 读写
├── exiv2           → EXIF/XMP 元数据解析
├── GeographicLib   → ENU 本地坐标系
├── SQLite          → PROJ 数据库/srs 缓存
├── Boost           → locale (编码转换), filesystem
├── Qt6             → Core, Gui, Widgets, Sql, Xml, Network, OpenGL
├── OpenSSL         → 加密 (通过 libcurl)
├── libcurl         → OTA 更新
├── Ceres Solver    → (间接依赖，用于 ATData::AlignRobust)
└── KML             → KML 导出
```

---

## 十一、代码质量问题

### 严重

1. **DataStruct.h 超过 4000 行**: 所有二进制格式在一个文件中，包含 30+ struct 的 Serialize/Deserialize，维护困难
2. **三套序列化代码**: 每个结构体同时维护 nlohmann JSON + RapidJSON (string) + RapidJSON (Value&) + BIN 四套代码
3. **GBK 残留**: 大量 `field2_` 备份和已注释的 `UTF82GBK/GBK2UTF8` 调用未清理
4. **异或加密形同虚设**: `SOURCE_XOR_KEY = 0xAB` 硬编码，无实际安全意义
5. **ATData 类 816 行**: 违反单一职责，同时负责数据存储、变换、导出、预览
6. **BlockObject 类 575 行**: 既有数据容器又有业务逻辑，包含 10+ 嵌套 struct

### 中等

1. **TaskDef.h 1576 行**: 单个文件包含 TaskDescriptor + ATTaskInfo + GBK 转换 + 多种序列化代码
2. **Math.h 使用匿名 namespace 套匿名 namespace**: 所有实现在头文件中，导致多个编译单元重复实例化
3. **PointCloud.h 语法不完整**: 类体残缺 (`PointCloud` 缺少 `{};`)
4. **Proj/ 独立封装**: 自成体系的 Proj 封装与 CoordinateSystem.h 中的 CoordinateTransformer 功能重叠
5. **缺少统一错误处理**: 有时用 returnCode，有时用 LOG(ERROR) + return false

### 轻度

1. **中文/英文注释混用**
2. **命名风格不统一**: `ATData` vs `BlockObject` vs `ATTaskInfo` (全大写 vs CamelCase vs 混合)
3. **Test/Core/testPreview/** 几乎为空，Core 层缺少单元测试覆盖
4. **json.h 过大 (494KB)**: 包含完整的 nlohmann json 单头文件实现

---

## 十二、总体评价

**优点**:
- 领域建模完整：从相机模型、图像、特征点、3D点、Track、GCP 到 Block/Project 层次清晰
- 双格式序列化（JSON 可调试 + BIN 高性能）设计思路合理
- 坐标系统支持完善，PROJ/GDAL/GeographicLib 三套体系互补
- 数学算法层质量较好，三角化/投影变换/相似变换实现符合摄影测量标准

**主要风险**:
- DataStruct.h 的序列化代码是三套 JSON + 一套 XOR BIN 的维护噩梦
- ATData 和 BlockObject 过于庞大，修改风险高
- GBK 编码历史残留虽然已注释，但污染了几乎每个 struct
- Core 层承担了太多职责（数据 + 算法 + I/O + 坐标变换 + 序列化）
