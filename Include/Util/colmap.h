#ifndef COLMAP_SRC_BASE_RECONSTRUCTION_H_
#define COLMAP_SRC_BASE_RECONSTRUCTION_H_

#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Core/Types.h"
#include <Eigen/Core>
#include "Core/ATData.h"
namespace colmap 
{




class Reconstruction 
{
 public:
	 Reconstruction() {};
  Reconstruction(const AI3D::CORE::ATData& data);
  const AI3D::CORE::ATData& GetATData() { return atdata_; };
  
  void Read(const std::string& path);
  void Write(const std::string& path) const;

  
  void ReadText(const std::string& path);
  void ReadBinary(const std::string& path);



  void ReadCamerasText(const std::string& path);
  void ReadImagesText(const std::string& path);
  void ReadPoints3DText(const std::string& path);
  void ReadCamerasBinary(const std::string& path);
  void ReadImagesBinary(const std::string& path);
  void ReadPoints3DBinary(const std::string& path);

  
  void WriteText(const std::string& path) const;
  void WriteBinary(const std::string& path) const;
  void WriteCamerasText(const std::string& path) const;
  void WriteImagesText(const std::string& path) const;
  void WritePoints3DText(const std::string& path) const;
  void WriteCamerasBinary(const std::string& path) const;
  void WriteImagesBinary(const std::string& path) const;
  void WritePoints3DBinary(const std::string& path) const;


private:
  EIGEN_STL_UMAP(camera_t, AI3D::CORE::Camera) cameras_;
  EIGEN_STL_UMAP(image_t, AI3D::CORE::Image) images_;
  EIGEN_STL_UMAP(point3D_t, AI3D::CORE::Point3D) points3D_;
  AI3D::CORE::ATData atdata_;
  std::vector<image_t> reg_image_ids_;
};


}  

#endif  
