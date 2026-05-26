
#include "colmap.h"
#include "Core/AlgorithmBase.h"
#include <fstream>
#include "Core/Endian.h"
#include "Core/File.h"
using namespace AI3D::CORE;
namespace colmap 
{

Reconstruction::Reconstruction(const AI3D::CORE::ATData& data)
{
    atdata_ = data;
    cameras_ = data.GetCameras();
    //?????p1,p2???????
    if (1)
    {
        for (auto& iter : cameras_)
        {

            auto& para = iter.second.GetParamsMutual();
            double temp = para[6];
            para[6] = para[7];
            para[7] = temp;

        }
    }
    images_ = data.GetImages();
    points3D_ = data.GetPoints3D();
}

void Reconstruction::Read(const std::string& path)
{
  if (AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "cameras.bin")) &&
      AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "images.bin")) &&
      AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "points3D.bin")))
  {
    ReadBinary(path);
  } else if (AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "cameras.txt")) &&
      AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "images.txt")) &&
      AI3D::CORE::File::ExistsFile(AI3D::CORE::File::JoinPaths(path, "points3D.txt")))
  {
    ReadText(path);
  } 
  else 
  {
    LOG(FATAL) << "cameras, images, points3D files do not exist at " << path;
  }

  //???atdata
  atdata_.SetRegImageIds(reg_image_ids_);
  atdata_.GetCamerasMutual() = cameras_;
  atdata_.GetPoints3DMutual() = points3D_;
  atdata_.GetImagesMutual() = images_;

}

void Reconstruction::Write(const std::string& path) const 
{ 
    WriteBinary(path);
    WriteText(path);
}
//
void Reconstruction::ReadText(const std::string& path) {
  ReadCamerasText(AI3D::CORE::File::JoinPaths(path, "cameras.txt"));
  ReadImagesText(AI3D::CORE::File::JoinPaths(path, "images.txt"));
  ReadPoints3DText(AI3D::CORE::File::JoinPaths(path, "points3D.txt"));
}

void Reconstruction::ReadBinary(const std::string& path) {
  ReadCamerasBinary(AI3D::CORE::File::JoinPaths(path, "cameras.bin"));
  ReadImagesBinary(AI3D::CORE::File::JoinPaths(path, "images.bin"));
  ReadPoints3DBinary(AI3D::CORE::File::JoinPaths(path, "points3D.bin"));
}


//
void Reconstruction::ReadCamerasText(const std::string& path) {
  cameras_.clear();

  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::in);
  CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
      AI3D::CORE::String::StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream(line);

    AI3D::CORE::Camera camera;

    // ID
    std::getline(line_stream, item, ' ');
    camera.SetCameraId(std::stoul(item));

    // MODEL
    std::getline(line_stream, item, ' ');
    camera.SetModelIdFromName(item);

    // WIDTH
    std::getline(line_stream, item, ' ');
    camera.SetWidth(std::stoll(item));

    // HEIGHT
    std::getline(line_stream, item, ' ');
    camera.SetHeight(std::stoll(item));

    // PARAMS
    camera.GetParamsMutual().clear();
    while (!line_stream.eof()) {
      std::getline(line_stream, item, ' ');
      camera.GetParamsMutual().push_back(std::stold(item));
    }
   double temp = camera.GetParamsMutual()[6];
    camera.GetParamsMutual()[6] = camera.GetParamsMutual()[7];
    camera.GetParamsMutual()[7] = temp;
    CHECK(camera.VerifyParams());
   
    cameras_.emplace(camera.GetCameraId(), camera);
  }
}

void Reconstruction::ReadImagesText(const std::string& path) {
  images_.clear();

  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::in);
  CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
    AI3D::CORE::String::StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream1(line);

    // ID
    std::getline(line_stream1, item, ' ');
    const image_t image_id = std::stoul(item);

    AI3D::CORE::Image image;
    image.SetImageId(image_id);

    image.SetRegistered(true);
   
    reg_image_ids_.push_back(image_id);

    // QVEC (qw, qx, qy, qz)
    std::getline(line_stream1, item, ' ');

    Eigen::Vector4d qvec;
    qvec(0) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    qvec(1) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    qvec(2) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    qvec(3) = std::stold(item);
    //std::cout << "before " << qvec << std::endl;
    qvec = AI3D::CORE::AlgorithmBase::NormalizeQuaternion(qvec);
    //std::cout << "after " << qvec << std::endl;
    auto R= AI3D::CORE::AlgorithmBase::QuaternionToRotationMatrix(qvec);
    //auto q1 = AI3D::CORE::AlgorithmBase::RotationMatrixToQuaternion(R);
    //std::cout <<"R " <<R<<" q1 " << q1<<" == "<< AI3D::CORE::AlgorithmBase::NormalizeQuaternion(q1) << std::endl;
    image.SetRotationMatrix(R);

    //std::cout << "-------"<<AlgorithmBase::RotationMatrixToQuaternion(image.GetRotationMatrix())<< " after1 " << image.GetQvec()<< " "<< AI3D::CORE::AlgorithmBase::NormalizeQuaternion(image.GetQvec())<< std::endl;
  
    // TVEC
    Eigen::Vector3d tvec,center;
    std::getline(line_stream1, item, ' ');
    tvec(0) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    tvec(1) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    tvec(2) = std::stold(item);
    
    center = -R.transpose() * tvec;
    image.SetPosition(center);

    // CAMERA_ID
    std::getline(line_stream1, item, ' ');
    image.SetCameraId(std::stoul(item));

    // NAME
    std::getline(line_stream1, item, ' ');
    image.SetName(item);

    // POINTS2D
    if (!std::getline(file, line)) {
      break;
    }

    AI3D::CORE::String::StringTrim(&line);
    std::stringstream line_stream2(line);

    std::vector<Eigen::Vector2d> points2D;
    std::vector<point3D_t> point3D_ids;

    if (!line.empty()) {
      while (!line_stream2.eof()) {
        Eigen::Vector2d point;

        std::getline(line_stream2, item, ' ');
        point.x() = std::stold(item);

        std::getline(line_stream2, item, ' ');
        point.y() = std::stold(item);

        points2D.push_back(point);

        std::getline(line_stream2, item, ' ');
        if (item == "-1") {
          point3D_ids.push_back(kInvalidPoint3DId);
        } else {
          point3D_ids.push_back(std::stoll(item));
        }
      }
    }

    image.SetUp(cameras_.at(image.GetCameraId()));
    image.SetPoints2D(points2D);

   /* for (point2D_t point2D_idx = 0; point2D_idx < image.GetNumPoints2D();
         ++point2D_idx) 
    {
      if (point3D_ids[point2D_idx] != kInvalidPoint3DId) 
      {
        image.SetPoint3DForPoint2D(point2D_idx, point3D_ids[point2D_idx]);
      }
    }*/

    images_.emplace(image.GetImageId(), image);
  }
}

void Reconstruction::ReadPoints3DText(const std::string& path) {
  points3D_.clear();

  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::in);
  CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
    AI3D::CORE::String::StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream(line);

    // ID
    std::getline(line_stream, item, ' ');
    const point3D_t point3D_id = std::stoll(item);

    // Make sure, that we can add new 3D points after reading 3D points
    // without overwriting existing 3D points.
    

    AI3D::CORE::Point3D point3D;

    // XYZ
    std::getline(line_stream, item, ' ');
    point3D.GetXYZMutual(0) = std::stold(item);

    std::getline(line_stream, item, ' ');
    point3D.GetXYZMutual(1) = std::stold(item);

    std::getline(line_stream, item, ' ');
    point3D.GetXYZMutual(2) = std::stold(item);

    // Color
    std::getline(line_stream, item, ' ');
    point3D.GetColorMutual(0) = static_cast<uint8_t>(std::stoi(item));

    std::getline(line_stream, item, ' ');
    point3D.GetColorMutual(1) = static_cast<uint8_t>(std::stoi(item));

    std::getline(line_stream, item, ' ');
    point3D.GetColorMutual(2) = static_cast<uint8_t>(std::stoi(item));

    // ERROR
    std::getline(line_stream, item, ' ');
    point3D.SetPixelRMS(std::stold(item));

    // TRACK
    while (!line_stream.eof()) {
      AI3D::CORE::TrackElement track_el;

      std::getline(line_stream, item, ' ');
      AI3D::CORE::String::StringTrim(&item);
      if (item.empty()) {
        break;
      }
      track_el.image_id = std::stoul(item);

      std::getline(line_stream, item, ' ');
      track_el.point2D_idx = std::stoul(item);
  
      AI3D::CORE::Point2D& point2d = images_.at(track_el.image_id).GetPoint2DMutual(track_el.point2D_idx);
      point2d.SetPoint3DId(point3D_id);
     
    
      track_el.xy = point2d.GetXY();
      auto eles = point3D.GetTrackMutual().GetElements();
      const auto& recindfo = std::find_if(eles.begin(), eles.end(),
          [&](AI3D::CORE::TrackElement a) { return track_el.image_id == a.image_id; });

      if (recindfo == eles.end())
      {
          point3D.GetTrackMutual().AddElement(track_el);
      }
      
    }

    point3D.GetTrackMutual().Compress();

    points3D_.emplace(point3D_id, point3D);
  }
}

void Reconstruction::ReadCamerasBinary(const std::string& path) {
  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::binary);
  CHECK(file.is_open()) << path;

  const size_t num_cameras = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_cameras; ++i) {
    Camera camera;
    camera.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));
    camera.SetModelId(ReadBinaryLittleEndian<int>(&file));
    camera.SetWidth(ReadBinaryLittleEndian<uint64_t>(&file));
    camera.SetHeight(ReadBinaryLittleEndian<uint64_t>(&file));
    ReadBinaryLittleEndian<double>(&file, &camera.GetParamsMutual());
    double temp = camera.GetParamsMutual()[6];
    camera.GetParamsMutual()[6] = camera.GetParamsMutual()[7];
    camera.GetParamsMutual()[7] = temp;
    CHECK(camera.VerifyParams());
    cameras_.emplace(camera.GetCameraId(), camera);
  }
}

void Reconstruction::ReadImagesBinary(const std::string& path) {
  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::binary);
  CHECK(file.is_open()) << path;

  const size_t num_reg_images = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_reg_images; ++i) {
    Image image;

    image.SetImageId(ReadBinaryLittleEndian<image_t>(&file));
    Eigen::Vector4d qvec;
    qvec(0) = ReadBinaryLittleEndian<double>(&file);
    qvec(1) = ReadBinaryLittleEndian<double>(&file);
    qvec(2) = ReadBinaryLittleEndian<double>(&file);
    qvec(3) = ReadBinaryLittleEndian<double>(&file);
    qvec = AI3D::CORE::AlgorithmBase::NormalizeQuaternion(qvec);
    auto R = AI3D::CORE::AlgorithmBase::QuaternionToRotationMatrix(qvec);
    image.SetRotationMatrix(R);

    Eigen::Vector3d tvec, center;
    tvec(0) = ReadBinaryLittleEndian<double>(&file);
    tvec(1) = ReadBinaryLittleEndian<double>(&file);
    tvec(2) = ReadBinaryLittleEndian<double>(&file);
    center = -R.transpose() * tvec;
    image.SetPosition(center);
    image.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));

    char name_char;
    do {
      file.read(&name_char, 1);
      if (name_char != '\0') {
        image.GetNameMutual() += name_char;
      }
    } while (name_char != '\0');

    const size_t num_points2D = ReadBinaryLittleEndian<uint64_t>(&file);
   /* std::string name = "E:/TestData/pingdu_1/newgs/Tile_+005_+020/images/"+image.GetName();
    if (boost::filesystem::exists(name))
    {

    }
    else
    {
        std::cout << image.GetImageId() << std::endl;
    }*/
    std::vector<Eigen::Vector2d> points2D;
    points2D.reserve(num_points2D);
    std::vector<point3D_t> point3D_ids;
    point3D_ids.reserve(num_points2D);
    for (size_t j = 0; j < num_points2D; ++j) {
      const double x = ReadBinaryLittleEndian<double>(&file);
      const double y = ReadBinaryLittleEndian<double>(&file);
      points2D.emplace_back(x, y);
      point3D_ids.push_back(ReadBinaryLittleEndian<point3D_t>(&file));
    }

    image.SetUp(cameras_.at(image.GetCameraId()));
    image.SetPoints2D(points2D);

  

    image.SetRegistered(true);
    reg_image_ids_.push_back(image.GetImageId());

    images_.emplace(image.GetImageId(), image);
  }
}

void Reconstruction::ReadPoints3DBinary(const std::string& path) {
  std::ifstream file = File::OpenIfstreamUtf8(path, std::ios::binary);
  CHECK(file.is_open()) << path;

  const size_t num_points3D = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_points3D; ++i) 
  {
    Point3D point3D;

    const point3D_t point3D_id = ReadBinaryLittleEndian<point3D_t>(&file);
    //num_added_points3D_ = std::max(num_added_points3D_, point3D_id);

    point3D.GetXYZMutual()(0) = ReadBinaryLittleEndian<double>(&file);
    point3D.GetXYZMutual()(1) = ReadBinaryLittleEndian<double>(&file);
    point3D.GetXYZMutual()(2) = ReadBinaryLittleEndian<double>(&file);
    point3D.GetColorMutual(0) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.GetColorMutual(1) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.GetColorMutual(2) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.SetPixelRMS(ReadBinaryLittleEndian<double>(&file));

    const size_t track_length = ReadBinaryLittleEndian<uint64_t>(&file);
    for (size_t j = 0; j < track_length; ++j) 
    {
      const image_t image_id = ReadBinaryLittleEndian<image_t>(&file);
      const point2D_t point2D_idx = ReadBinaryLittleEndian<point2D_t>(&file);
      AI3D::CORE::Point2D& point2d = images_.at(image_id).GetPoint2DMutual(point2D_idx);
      point2d.SetPoint3DId(point3D_id);
      AI3D::CORE::TrackElement ele;
      ele.image_id = image_id;
      ele.point2D_idx = point2D_idx;
      ele.xy = point2d.GetXY();
      
     
      auto eles = point3D.GetTrackMutual().GetElements();
      const auto& recindfo = std::find_if(eles.begin(), eles.end(),
          [&](AI3D::CORE::TrackElement a) { return ele.image_id == a.image_id; });

      if (recindfo == eles.end())
      {
          point3D.GetTrackMutual().AddElement(ele);
      }
    }

    
    point3D.GetTrackMutual().Compress();

    points3D_.emplace(point3D_id, point3D);
  }
  
}
void Reconstruction::WriteText(const std::string& path) const 
{
    WriteCamerasText(AI3D::CORE::File::JoinPaths(path, "cameras.txt"));
    WriteImagesText(AI3D::CORE::File::JoinPaths(path, "images.txt"));
    WritePoints3DText(AI3D::CORE::File::JoinPaths(path, "points3D.txt"));
}

void Reconstruction::WriteBinary(const std::string& path) const {
    WriteCamerasBinary(AI3D::CORE::File::JoinPaths(path, "cameras.bin"));
    WriteImagesBinary(AI3D::CORE::File::JoinPaths(path, "images.bin"));
    WritePoints3DBinary(AI3D::CORE::File::JoinPaths(path, "points3D.bin"));
}
void Reconstruction::WriteCamerasText(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
  CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# Camera list with one line of data per camera:" << std::endl;
  file << "#   CAMERA_ID, MODEL, WIDTH, HEIGHT, PARAMS[]" << std::endl;
  file << "# Number of cameras: " << cameras_.size() << std::endl;

  for (const auto& camera : cameras_) 
  {
    std::ostringstream line;
    line.precision(17);

    line << camera.first << " ";
    line << camera.second.GetModelName() << " ";
    line << camera.second.GetWidth() << " ";
    line << camera.second.GetHeight() << " ";

    for (const double param : camera.second.GetParams()) {
      line << param << " ";
    }

    std::string line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);

    file << line_string << std::endl;
  }
}

void Reconstruction::WriteImagesText(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
  CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# Image list with two lines of data per image:" << std::endl;
  file << "#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, "
          "NAME"
       << std::endl;
  file << "#   POINTS2D[] as (X, Y, POINT3D_ID)" << std::endl;
  file << "# Number of images: " << atdata_.GetNumRegImages()
       << ", mean observations per image: "
       << atdata_.ComputeMeanObservationsPerRegImage() << std::endl;

  for (const auto& image : images_) 
  {
    if (!image.second.IsRegistered()) 
    {
      continue;
    }

    std::ostringstream line;
    line.precision(17);

    std::string line_string;

    line << image.first << " ";
    //normal q??????????
    // QVEC (qw, qx, qy, qz)
    Eigen::Vector4d QVEC = AlgorithmBase::RotationMatrixToQuaternion(image.second.GetRotationMatrix());
    const Eigen::Vector4d normalized_qvec =  AI3D::CORE::AlgorithmBase::NormalizeQuaternion(QVEC);
    line << normalized_qvec(0) << " ";
    line << normalized_qvec(1) << " ";
    line << normalized_qvec(2) << " ";
    line << normalized_qvec(3) << " ";

    // TVEC
    auto Tv = -image.second.GetRotationMatrix() * image.second.GetPosition();
    line << Tv(0) << " ";
    line << Tv(1) << " ";
    line << Tv(2) << " ";

    //??????? 
    /*auto R = image.second.GetRotationMatrix();
    auto C1 = image.second.GetPosition();
    std::cout << "raw "<<R * C1 << std::endl;*/
    //q norm??
  /*  auto Rnq = AlgorithmBase::QuaternionToRotationMatrix(AI3D::CORE::AlgorithmBase::NormalizeQuaternion(QVEC));
    std::cout << "Rnq " << Rnq * C1 << std::endl;*/
    line << image.second.GetCameraId() << " ";

    line << image.second.GetName();
    if (image.second.GetImageId() == 4246)
    {
        std::cout << image.second.GetName() << " " << std::endl;
    }
    file << line.str() << std::endl;

    line.str("");
    line.clear();

    for (const AI3D::CORE::Point2D& point2D : image.second.GetPoints2D()) 
    {
       /* if (!point2D.HasPoint3D())
        {
            continue;
        }*/
      line << point2D.GetX() << " ";
      line << point2D.GetY() << " ";
      if (point2D.HasPoint3D()) 
      {
        line << point2D.GetPoint3DId() << " ";
      } 
      else 
      {
        line << -1 << " ";
      }
    }
    line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);
    file << line_string << std::endl;
  }
}

void Reconstruction::WritePoints3DText(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc);
  CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# 3D point list with one line of data per point:" << std::endl;
  file << "#   POINT3D_ID, X, Y, Z, R, G, B, ERROR, "
          "TRACK[] as (IMAGE_ID, POINT2D_IDX)"
       << std::endl;
  file << "# Number of points: " << points3D_.size()
       << ", mean track length: " << atdata_.ComputeMeanTrackLength() << std::endl;

  for (const auto& point3D : points3D_) {
    file << point3D.first << " ";
    file << point3D.second.GetXYZ()(0) << " ";
    file << point3D.second.GetXYZ()(1) << " ";
    file << point3D.second.GetXYZ()(2) << " ";
    file << static_cast<int>(point3D.second.GetColor(0)) << " ";
    file << static_cast<int>(point3D.second.GetColor(1)) << " ";
    file << static_cast<int>(point3D.second.GetColor(2)) << " ";
    file <<  -1/*point3D.second.GetPixelRMS()*/ << " ";

    std::ostringstream line;
    line.precision(17);

    for (const auto& track_el : point3D.second.GetTrack().GetElements()) {
      line << track_el.image_id << " ";
      line << track_el.point2D_idx << " ";
    }

    std::string line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);

    file << line_string << std::endl;
  }
}

void Reconstruction::WriteCamerasBinary(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  CHECK(file.is_open()) << path;

  AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, cameras_.size());

  for (const auto& camera : cameras_) {
      AI3D::CORE::WriteBinaryLittleEndian<camera_t>(&file, camera.first);
      AI3D::CORE::WriteBinaryLittleEndian<int>(&file, camera.second.GetModelId());
      AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, camera.second.GetWidth());
      AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, camera.second.GetHeight());
    for (const double param : camera.second.GetParams()) 
    {
        AI3D::CORE::WriteBinaryLittleEndian<double>(&file, param);
    }
  }
}

void Reconstruction::WriteImagesBinary(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  CHECK(file.is_open()) << path;

  AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, atdata_.GetNumRegImages());

  for (const auto& image : images_) {
    if (!image.second.IsRegistered()) {
      continue;
    }

    AI3D::CORE::WriteBinaryLittleEndian<image_t>(&file, image.first);

    Eigen::Vector4d QVEC = AlgorithmBase::RotationMatrixToQuaternion(image.second.GetRotationMatrix());
    const Eigen::Vector4d normalized_qvec =   AI3D::CORE::AlgorithmBase::NormalizeQuaternion(QVEC);

   
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, normalized_qvec(0));
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, normalized_qvec(1));
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, normalized_qvec(2));
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, normalized_qvec(3));
    auto Tv = -image.second.GetRotationMatrix() * image.second.GetPosition();
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, Tv(0));
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, Tv(1));
    AI3D::CORE::WriteBinaryLittleEndian<double>(&file, Tv(2));

    AI3D::CORE::WriteBinaryLittleEndian<camera_t>(&file, image.second.GetCameraId());

    const std::string name = image.second.GetName() + '\0';
    if (image.second.GetImageId() == 4246)
    {
        std::cout << name << " " << std::endl;
    }
    file.write(name.c_str(), name.size());

    AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, image.second.GetNumPoints2D());
    for (const AI3D::CORE::Point2D& point2D : image.second.GetPoints2D()) {
        AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point2D.GetX());
        AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point2D.GetY());
        AI3D::CORE::WriteBinaryLittleEndian<point3D_t>(&file, point2D.GetPoint3DId());
    }
  }
}

void Reconstruction::WritePoints3DBinary(const std::string& path) const {
  std::ofstream file = File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  CHECK(file.is_open()) << path;

  AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, points3D_.size());

  for (const auto& point3D : points3D_) {
      AI3D::CORE::WriteBinaryLittleEndian<point3D_t>(&file, point3D.first);
      AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point3D.second.GetXYZ()(0));
      AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point3D.second.GetXYZ()(1));
      AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point3D.second.GetXYZ()(2));
      AI3D::CORE::WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.GetColor(0));
      AI3D::CORE::WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.GetColor(1));
      AI3D::CORE::WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.GetColor(2));
      AI3D::CORE::WriteBinaryLittleEndian<double>(&file, point3D.second.GetPixelRMS());

      AI3D::CORE::WriteBinaryLittleEndian<uint64_t>(&file, point3D.second.GetTrack().Length());
    for (const auto& track_el : point3D.second.GetTrack().GetElements()) {
        AI3D::CORE::WriteBinaryLittleEndian<image_t>(&file, track_el.image_id);
        AI3D::CORE::WriteBinaryLittleEndian<point2D_t>(&file, track_el.point2D_idx);
    }
  }
}

}  // namespace colmap
