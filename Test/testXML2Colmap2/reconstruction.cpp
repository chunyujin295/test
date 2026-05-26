// Copyright (c) 2018, ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: Johannes L. Schoenberger (jsch-at-demuc-dot-de)

#include "reconstruction.h"
#include "camera_models.h"
#include <fstream>
#include <filesystem>
#include "undistortion.h"
#include <pugixml.hpp>
//#include "similarity_transform.h"
//#include "base/database_cache.h"
//#include "base/gps.h"
#include "pose.h"
#include "Core/File.h"
//#include "base/projection.h"
//#include "base/triangulation.h"
//#include "util/bitmap.h"
//#include "util/misc.h"
//#include "util/ply.h"

#include "camera.h"
#include "misc.h"
#include "Core/File.h"
namespace colmap {

Reconstruction::Reconstruction()
    : correspondence_graph_(nullptr), num_added_points3D_(0) {}

std::unordered_set<point3D_t> Reconstruction::Point3DIds() const {
  std::unordered_set<point3D_t> point3D_ids;
  point3D_ids.reserve(points3D_.size());

  for (const auto& point3D : points3D_) {
    point3D_ids.insert(point3D.first);
  }

  return point3D_ids;
}

//void Reconstruction::Load(const DatabaseCache& database_cache) {
//  correspondence_graph_ = nullptr;
//
//  // Add cameras.
//  cameras_.reserve(database_cache.NumCameras());
//  for (const auto& camera : database_cache.Cameras()) {
//    if (!ExistsCamera(camera.first)) {
//      AddCamera(camera.second);
//    }
//    // Else: camera was added before, e.g. with `ReadAllCameras`.
//  }
//
//  // Add images.
//  images_.reserve(database_cache.NumImages());
//
//  for (const auto& image : database_cache.Images()) {
//    if (ExistsImage(image.second.ImageId())) {
//      class Image& existing_image = Image(image.second.ImageId());
//      CHECK_EQ(existing_image.Name(), image.second.Name());
//      if (existing_image.NumPoints2D() == 0) {
//        existing_image.SetPoints2D(image.second.Points2D());
//      } else {
//        CHECK_EQ(image.second.NumPoints2D(), existing_image.NumPoints2D());
//      }
//      existing_image.SetNumObservations(image.second.NumObservations());
//      existing_image.SetNumCorrespondences(image.second.NumCorrespondences());
//    } else {
//      AddImage(image.second);
//    }
//  }
//
//  // Add image pairs.
//  for (const auto& image_pair :
//       database_cache.CorrespondenceGraph().NumCorrespondencesBetweenImages()) {
//    ImagePairStat image_pair_stat;
//    image_pair_stat.num_total_corrs = image_pair.second;
//    image_pair_stats_.emplace(image_pair.first, image_pair_stat);
//  }
//}

void Reconstruction::SetUp(const CorrespondenceGraph* correspondence_graph) {
  //CHECK_NOTNULL(correspondence_graph);
  for (auto& image : images_) {
    image.second.SetUp(Camera(image.second.CameraId()));
  }
  correspondence_graph_ = correspondence_graph;

  // If an existing model was loaded from disk and there were already images
  // registered previously, we need to set observations as triangulated.
  for (const auto image_id : reg_image_ids_) {
    const colmap::Image& image = Image(image_id);
    for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
         ++point2D_idx) {
      if (image.Point2D(point2D_idx).HasPoint3D()) {
        const bool kIsContinuedPoint3D = false;
       /* SetObservationAsTriangulated(image_id, point2D_idx,
                                     kIsContinuedPoint3D);*/
      }
    }
  }
}

void Reconstruction::TearDown() {
  correspondence_graph_ = nullptr;
  image_pair_stats_.clear();

  // Remove all not yet registered images.
  std::unordered_set<camera_t> keep_camera_ids;
  for (auto it = images_.begin(); it != images_.end();) {
    if (it->second.IsRegistered()) {
      keep_camera_ids.insert(it->second.CameraId());
      it->second.TearDown();
      ++it;
    } else {
      it = images_.erase(it);
    }
  }

  // Remove all unused cameras.
  for (auto it = cameras_.begin(); it != cameras_.end();) {
    if (keep_camera_ids.count(it->first) == 0) {
      it = cameras_.erase(it);
    } else {
      ++it;
    }
  }

  // Compress tracks.
  for (auto& point3D : points3D_) {
    point3D.second.Track().Compress();
  }
}

void Reconstruction::AddCamera(const class Camera& camera) {
  ////CHECK();
    if (ExistsCamera(camera.CameraId()))
        return;
    if (!camera.VerifyParams())
        return;
  cameras_.emplace(camera.CameraId(), camera);
}

void Reconstruction::AddImage(const colmap::Image& image) {
    if (ExistsImage(image.ImageId()))
        return;
    images_[image.ImageId()] = image;
}

point3D_t Reconstruction::AddPoint3D(const Eigen::Vector3d& xyz,
                                     const Track& track,
                                     const Eigen::Vector3ub& color) {
  const point3D_t point3D_id = ++num_added_points3D_;
  if (ExistsPoint3D(point3D_id))
  {
      return kInvalidPoint3DId;
  }

  class Point3D& point3D = points3D_[point3D_id];

  point3D.SetXYZ(xyz);
  point3D.SetTrack(track);
  point3D.SetColor(color);

  for (const auto& track_el : track.Elements()) 
  {
      colmap::Image& image = Image(track_el.image_id);
    
   /* if (image.Point2D(track_el.point2D_idx).HasPoint3D())
    {
        return kInvalidPoint3DId;;
    }*/
    image.SetPoint3DForPoint2D(track_el.point2D_idx, point3D_id);
    //CHECK_LE(image.NumPoints3D(), image.NumPoints2D());
  }

  const bool kIsContinuedPoint3D = false;

  for (const auto& track_el : track.Elements()) {
   /* SetObservationAsTriangulated(track_el.image_id, track_el.point2D_idx,
                                 kIsContinuedPoint3D);*/
  }

  return point3D_id;
}

void Reconstruction::AddObservation(const point3D_t point3D_id,
                                    const TrackElement& track_el) {
    colmap::Image& image = Image(track_el.image_id);
  ////CHECK(!image.Point2D(track_el.point2D_idx).HasPoint3D());

  image.SetPoint3DForPoint2D(track_el.point2D_idx, point3D_id);
  //CHECK_LE(image.NumPoints3D(), image.NumPoints2D());

  class Point3D& point3D = Point3D(point3D_id);
  point3D.Track().AddElement(track_el);

  const bool kIsContinuedPoint3D = true;
  /*SetObservationAsTriangulated(track_el.image_id, track_el.point2D_idx,
                               kIsContinuedPoint3D);*/
}
//
//point3D_t Reconstruction::MergePoints3D(const point3D_t point3D_id1,
//                                        const point3D_t point3D_id2) {
//  const class Point3D& point3D1 = Point3D(point3D_id1);
//  const class Point3D& point3D2 = Point3D(point3D_id2);
//
//  const Eigen::Vector3d merged_xyz =
//      (point3D1.Track().Length() * point3D1.XYZ() +
//       point3D2.Track().Length() * point3D2.XYZ()) /
//      (point3D1.Track().Length() + point3D2.Track().Length());
//  const Eigen::Vector3d merged_rgb =
//      (point3D1.Track().Length() * point3D1.Color().cast<double>() +
//       point3D2.Track().Length() * point3D2.Color().cast<double>()) /
//      (point3D1.Track().Length() + point3D2.Track().Length());
//
//  Track merged_track;
//  merged_track.Reserve(point3D1.Track().Length() + point3D2.Track().Length());
//  merged_track.AddElements(point3D1.Track().Elements());
//  merged_track.AddElements(point3D2.Track().Elements());
//
//  DeletePoint3D(point3D_id1);
//  DeletePoint3D(point3D_id2);
//
//  const point3D_t merged_point3D_id =
//      AddPoint3D(merged_xyz, merged_track, merged_rgb.cast<uint8_t>());
//
//  return merged_point3D_id;
//}
////
//void Reconstruction::DeletePoint3D(const point3D_t point3D_id) {
//  // Note: Do not change order of these instructions, especially with respect to
//  // `Reconstruction::ResetTriObservations`
//
//  const class Track& track = Point3D(point3D_id).Track();
//
//  const bool kIsDeletedPoint3D = true;
//
//  for (const auto& track_el : track.Elements()) {
//    ResetTriObservations(track_el.image_id, track_el.point2D_idx,
//                         kIsDeletedPoint3D);
//  }
//
//  for (const auto& track_el : track.Elements()) {
//    class Image& image = Image(track_el.image_id);
//    image.ResetPoint3DForPoint2D(track_el.point2D_idx);
//  }
//
//  points3D_.erase(point3D_id);
//}
//
//void Reconstruction::DeleteObservation(const image_t image_id,
//                                       const point2D_t point2D_idx) {
//  // Note: Do not change order of these instructions, especially with respect to
//  // `Reconstruction::ResetTriObservations`
//
//  class Image& image = Image(image_id);
//  const point3D_t point3D_id = image.Point2D(point2D_idx).Point3DId();
//  class Point3D& point3D = Point3D(point3D_id);
//
//  if (point3D.Track().Length() <= 2) {
//    DeletePoint3D(point3D_id);
//    return;
//  }
//
//  point3D.Track().DeleteElement(image_id, point2D_idx);
//
//  const bool kIsDeletedPoint3D = false;
//  ResetTriObservations(image_id, point2D_idx, kIsDeletedPoint3D);
//
//  image.ResetPoint3DForPoint2D(point2D_idx);
//}
//
//void Reconstruction::DeleteAllPoints2DAndPoints3D() {
//  points3D_.clear();
//  for (auto& image : images_) {
//    class Image new_image;
//    new_image.SetImageId(image.second.ImageId());
//    new_image.SetName(image.second.Name());
//    new_image.SetCameraId(image.second.CameraId());
//    new_image.SetRegistered(image.second.IsRegistered());
//    new_image.SetNumCorrespondences(image.second.NumCorrespondences());
//    new_image.SetQvec(image.second.Qvec());
//    new_image.SetQvecPrior(image.second.QvecPrior());
//    new_image.SetTvec(image.second.Tvec());
//    new_image.SetTvecPrior(image.second.TvecPrior());
//    image.second = new_image;
//  }
//}

void Reconstruction::RegisterImage(const image_t image_id) {
    colmap::Image& image = Image(image_id);
  if (!image.IsRegistered()) {
    image.SetRegistered(true);
    reg_image_ids_.push_back(image_id);
  }
}
//
//void Reconstruction::DeRegisterImage(const image_t image_id) {
//  class Image& image = Image(image_id);
//
//  for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
//       ++point2D_idx) {
//    if (image.Point2D(point2D_idx).HasPoint3D()) {
//      DeleteObservation(image_id, point2D_idx);
//    }
//  }
//
//  image.SetRegistered(false);
//
//  reg_image_ids_.erase(
//      std::remove(reg_image_ids_.begin(), reg_image_ids_.end(), image_id),
//      reg_image_ids_.end());
//}
SimilarityTransform3::SimilarityTransform3(const double scale,
    const Eigen::Vector4d& qvec,
    const Eigen::Vector3d& tvec) {
    Eigen::Matrix4d matrix = Eigen::MatrixXd::Identity(4, 4);
    matrix.topLeftCorner<3, 4>() = ComposeProjectionMatrix(qvec, tvec);
    matrix.block<3, 3>(0, 0) *= scale;
    transform_.matrix() = matrix;
}

void SimilarityTransform3::TransformPoint(Eigen::Vector3d* xyz) const {
    *xyz = transform_ * *xyz;
}

void SimilarityTransform3::TransformPose(Eigen::Vector4d* qvec,
    Eigen::Vector3d* tvec) const {
    // Projection matrix P1 projects 3D object points to image plane and thus to
    // 2D image points in the source coordinate system:
    //    x' = P1 * X1
    // 3D object points can be transformed to the destination system by applying
    // the similarity transformation S:
    //    X2 = S * X1
    // To obtain the projection matrix P2 that transforms the object point in the
    // destination system to the 2D image points, which do not change:
    //    x' = P2 * X2 = P2 * S * X1 = P1 * S^-1 * S * X1 = P1 * I * X1
    // and thus:
    //    P2' = P1 * S^-1
    // Finally, undo the inverse scaling of the rotation matrix:
    //    P2 = s * P2'

    Eigen::Matrix4d src_matrix = Eigen::MatrixXd::Identity(4, 4);
    src_matrix.topLeftCorner<3, 4>() = ComposeProjectionMatrix(*qvec, *tvec);
    Eigen::Matrix4d dst_matrix =
        src_matrix.matrix() * transform_.inverse().matrix();
    dst_matrix *= Scale();

    *qvec = RotationMatrixToQuaternion(dst_matrix.block<3, 3>(0, 0));
    *tvec = dst_matrix.block<3, 1>(0, 3);
}
Eigen::Matrix4d SimilarityTransform3::Matrix() const {
    return transform_.matrix();
}
double SimilarityTransform3::Scale() const {
    return Matrix().block<1, 3>(0, 0).norm();
}
void Reconstruction::Normalize(const double extent, const double p0,
                               const double p1, const bool use_images) {
  //CHECK_GT(extent, 0);

  if ((use_images && reg_image_ids_.size() < 2) ||
      (!use_images && points3D_.size() < 2)) {
    return;
  }

  auto bound = ComputeBoundsAndCentroid(p0, p1, use_images);

  // Calculate scale and translation, such that
  // translation is applied before scaling.
  const double old_extent = (std::get<1>(bound) - std::get<0>(bound)).norm();
  double scale;
  if (old_extent < std::numeric_limits<double>::epsilon()) {
    scale = 1;
  } else {
    scale = extent / old_extent;
  }

  SimilarityTransform3 tform(scale, ComposeIdentityQuaternion(),
                             -scale * std::get<2>(bound));
  Transform(tform);
}





void Reconstruction::Transform(const SimilarityTransform3& tform) {
    for (auto& image : images_) {
        tform.TransformPose(&image.second.Qvec(), &image.second.Tvec());
    }
    for (auto& point3D : points3D_) {
        tform.TransformPoint(&point3D.second.XYZ());
    }
}
Eigen::Vector3d Reconstruction::ComputeCentroid(const double p0,
                                                const double p1) const {
  return std::get<2>(ComputeBoundsAndCentroid(p0, p1, false));
}

std::pair<Eigen::Vector3d, Eigen::Vector3d> Reconstruction::ComputeBoundingBox(
    const double p0, const double p1) const {
  auto bound = ComputeBoundsAndCentroid(p0, p1, false);
  return std::make_pair(std::get<0>(bound), std::get<1>(bound));
}

std::tuple<Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d>
Reconstruction::ComputeBoundsAndCentroid(const double p0, const double p1,
                                         const bool use_images) const {
  /*CHECK_GE(p0, 0);
  CHECK_LE(p0, 1);
  CHECK_GE(p1, 0);
  CHECK_LE(p1, 1);
  CHECK_LE(p0, p1);*/

  const size_t num_elements =
      use_images ? reg_image_ids_.size() : points3D_.size();
  if (num_elements == 0) {
    return std::make_tuple(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, 0),
                           Eigen::Vector3d(0, 0, 0));
  }

  // Coordinates of image centers or point locations.
  std::vector<float> coords_x;
  std::vector<float> coords_y;
  std::vector<float> coords_z;
  if (use_images) {
    coords_x.reserve(reg_image_ids_.size());
    coords_y.reserve(reg_image_ids_.size());
    coords_z.reserve(reg_image_ids_.size());
    for (const image_t im_id : reg_image_ids_) {
      const Eigen::Vector3d proj_center = Image(im_id).ProjectionCenter();
      coords_x.push_back(static_cast<float>(proj_center(0)));
      coords_y.push_back(static_cast<float>(proj_center(1)));
      coords_z.push_back(static_cast<float>(proj_center(2)));
    }
  } else {
    coords_x.reserve(points3D_.size());
    coords_y.reserve(points3D_.size());
    coords_z.reserve(points3D_.size());
    for (const auto& point3D : points3D_) {
      coords_x.push_back(static_cast<float>(point3D.second.X()));
      coords_y.push_back(static_cast<float>(point3D.second.Y()));
      coords_z.push_back(static_cast<float>(point3D.second.Z()));
    }
  }

  // Determine robust bounding box and mean.

  std::sort(coords_x.begin(), coords_x.end());
  std::sort(coords_y.begin(), coords_y.end());
  std::sort(coords_z.begin(), coords_z.end());

  const size_t P0 = static_cast<size_t>(
      (coords_x.size() > 3) ? p0 * (coords_x.size() - 1) : 0);
  const size_t P1 = static_cast<size_t>(
      (coords_x.size() > 3) ? p1 * (coords_x.size() - 1) : coords_x.size() - 1);

  const Eigen::Vector3d bbox_min(coords_x[P0], coords_y[P0], coords_z[P0]);
  const Eigen::Vector3d bbox_max(coords_x[P1], coords_y[P1], coords_z[P1]);

  Eigen::Vector3d mean_coord(0, 0, 0);
  for (size_t i = P0; i <= P1; ++i) {
    mean_coord(0) += coords_x[i];
    mean_coord(1) += coords_y[i];
    mean_coord(2) += coords_z[i];
  }
  mean_coord /= P1 - P0 + 1;

  return std::make_tuple(bbox_min, bbox_max, mean_coord);
}

Reconstruction Reconstruction::Crop(
    const std::pair<Eigen::Vector3d, Eigen::Vector3d>& bbox) const {
  // add all cameras and images. Only the registered images will be used.
  Reconstruction reconstruction;
  for (const auto& camera_el : cameras_) {
    reconstruction.AddCamera(camera_el.second);
  }
  for (const auto& image_el : images_) {
    reconstruction.AddImage(image_el.second);
    auto& image = reconstruction.Image(image_el.first);
    image.SetRegistered(false);
    for (point2D_t pid = 0; pid < image.NumPoints2D(); ++pid) {
      image.ResetPoint3DForPoint2D(pid);
    }
  }
  for (const auto& point_el : points3D_) {
    const auto& point = point_el.second;
    if ((point.XYZ().array() >= bbox.first.array()).all() &&
        (point.XYZ().array() <= bbox.second.array()).all()) {
      for (const auto& track_el : point.Track().Elements()) {
        reconstruction.RegisterImage(track_el.image_id);
      }
      reconstruction.AddPoint3D(point.XYZ(), point.Track(), point.Color());
    }
  }
  return reconstruction;
}

const colmap::Image* Reconstruction::FindImageWithName(
    const std::string& name) const {
  for (const auto& image : images_) {
    if (image.second.Name() == name) {
      return &image.second;
    }
  }
  return nullptr;
}

std::vector<image_t> Reconstruction::FindCommonRegImageIds(
    const Reconstruction& reconstruction) const {
  std::vector<image_t> common_reg_image_ids;
  for (const auto image_id : reg_image_ids_) {
    if (reconstruction.ExistsImage(image_id) &&
        reconstruction.IsImageRegistered(image_id)) {
      //CHECK_EQ(Image(image_id).Name(), reconstruction.Image(image_id).Name());
      common_reg_image_ids.push_back(image_id);
    }
  }
  return common_reg_image_ids;
}

//void Reconstruction::TranscribeImageIdsToDatabase(const Database& database) {
//  std::unordered_map<image_t, image_t> old_to_new_image_ids;
//  old_to_new_image_ids.reserve(NumImages());
//
//  EIGEN_STL_UMAP(image_t, class Image) new_images;
//  new_images.reserve(NumImages());
//
//  for (auto& image : images_) {
//    if (!database.ExistsImageWithName(image.second.Name())) {
//      LOG(FATAL) << "Image with name " << image.second.Name()
//                 << " does not exist in database";
//    }
//
//    const auto database_image = database.ReadImageWithName(image.second.Name());
//    old_to_new_image_ids.emplace(image.second.ImageId(),
//                                 database_image.ImageId());
//    image.second.SetImageId(database_image.ImageId());
//    new_images.emplace(database_image.ImageId(), image.second);
//  }
//
//  images_ = std::move(new_images);
//
//  for (auto& image_id : reg_image_ids_) {
//    image_id = old_to_new_image_ids.at(image_id);
//  }
//
//  for (auto& point3D : points3D_) {
//    for (auto& track_el : point3D.second.Track().Elements()) {
//      track_el.image_id = old_to_new_image_ids.at(track_el.image_id);
//    }
//  }
//}

size_t Reconstruction::ComputeNumObservations() const {
  size_t num_obs = 0;
  for (const image_t image_id : reg_image_ids_) {
    num_obs += Image(image_id).NumPoints3D();
  }
  return num_obs;
}

double Reconstruction::ComputeMeanTrackLength() const {
  if (points3D_.empty()) {
    return 0.0;
  } else {
    return ComputeNumObservations() / static_cast<double>(points3D_.size());
  }
}

double Reconstruction::ComputeMeanObservationsPerRegImage() const {
  if (reg_image_ids_.empty()) {
    return 0.0;
  } else {
    return ComputeNumObservations() /
           static_cast<double>(reg_image_ids_.size());
  }
}

double Reconstruction::ComputeMeanReprojectionError() const {
  double error_sum = 0.0;
  size_t num_valid_errors = 0;
  for (const auto& point3D : points3D_) {
    if (point3D.second.HasError()) {
      error_sum += point3D.second.Error();
      num_valid_errors += 1;
    }
  }

  if (num_valid_errors == 0) {
    return 0.0;
  } else {
    return error_sum / num_valid_errors;
  }
}

void Reconstruction::Read(const std::string& path) {
  if (ExistsFile(JoinPaths(path, "cameras.bin")) &&
      ExistsFile(JoinPaths(path, "images.bin")) &&
      ExistsFile(JoinPaths(path, "points3D.bin"))) {
    ReadBinary(path);
  } else if (ExistsFile(JoinPaths(path, "cameras.txt")) &&
             ExistsFile(JoinPaths(path, "images.txt")) &&
             ExistsFile(JoinPaths(path, "points3D.txt"))) {
    ReadText(path);
  } else {
    //LOG(FATAL) << "cameras, images, points3D files do not exist at " << path;
  }
}

void Reconstruction::Write(const std::string& path) const 
{ 
    WriteBinary(path); 
    WriteText(path);
}

void Reconstruction::ReadText(const std::string& path) {
  ReadCamerasText(JoinPaths(path, "cameras.txt"));
  ReadImagesText(JoinPaths(path, "images.txt"));
  ReadPoints3DText(JoinPaths(path, "points3D.txt"));
}

void Reconstruction::ReadBinary(const std::string& path) {
  ReadCamerasBinary(JoinPaths(path, "cameras.bin"));
  ReadImagesBinary(JoinPaths(path, "images.bin"));
  ReadPoints3DBinary(JoinPaths(path, "points3D.bin"));
}

void Reconstruction::WriteText(const std::string& path) const {
  WriteCamerasText(JoinPaths(path, "cameras.txt"));
  WriteImagesText(JoinPaths(path, "images.txt"));
  WritePoints3DText(JoinPaths(path, "points3D.txt"));
}

void Reconstruction::WriteBinary(const std::string& path) const {
  WriteCamerasBinary(JoinPaths(path, "cameras.bin"));
  WriteImagesBinary(JoinPaths(path, "images.bin"));
  WritePoints3DBinary(JoinPaths(path, "points3D.bin"));
}

void Reconstruction::CreateImageDirs(const std::string& path) const {
  std::unordered_set<std::string> image_dirs;
  for (const auto& image : images_) {
    const std::vector<std::string> name_split =
        StringSplit(image.second.Name(), "/");
    if (name_split.size() > 1) {
      std::string dir = path;
      for (size_t i = 0; i < name_split.size() - 1; ++i) {
        dir = JoinPaths(dir, name_split[i]);
        image_dirs.insert(dir);
      }
    }
  }
  for (const auto& dir : image_dirs) {
    CreateDirIfNotExists(dir);
  }
}

void Reconstruction::ReadCamerasText(const std::string& path) {
  cameras_.clear();

  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::in);
  ////CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
    StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream(line);

    class Camera camera;

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
    camera.Params().clear();
    while (!line_stream.eof()) {
      std::getline(line_stream, item, ' ');
      camera.Params().push_back(std::stold(item));
    }

    //CHECK(camera.VerifyParams());

    cameras_.emplace(camera.CameraId(), camera);
  }
}

void Reconstruction::RunUndistort(Reconstruction rec,std::string image_path,std::string dense_path)
{
    UndistortCameraOptions undistortion_options;
    undistortion_options.max_image_size =-1;
    COLMAPUndistorter undistorter(undistortion_options,
        rec,
        image_path, dense_path);
    active_thread_ = &undistorter;
    undistorter.Start();
    undistorter.Wait();
    active_thread_ = nullptr;
}

void Reconstruction::ReadImagesText(const std::string& path) {
  images_.clear();

  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::in);
  //CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
    StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream1(line);

    // ID
    std::getline(line_stream1, item, ' ');
    const image_t image_id = std::stoul(item);

    colmap::Image image;
    image.SetImageId(image_id);

    image.SetRegistered(true);
    reg_image_ids_.push_back(image_id);

    // QVEC (qw, qx, qy, qz)
    std::getline(line_stream1, item, ' ');
    image.Qvec(0) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    image.Qvec(1) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    image.Qvec(2) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    image.Qvec(3) = std::stold(item);

    image.NormalizeQvec();

    // TVEC
    std::getline(line_stream1, item, ' ');
    image.Tvec(0) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    image.Tvec(1) = std::stold(item);

    std::getline(line_stream1, item, ' ');
    image.Tvec(2) = std::stold(item);

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

    StringTrim(&line);
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

    image.SetUp(Camera(image.CameraId()));
    image.SetPoints2D(points2D);

    for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
         ++point2D_idx) {
      if (point3D_ids[point2D_idx] != kInvalidPoint3DId) {
        image.SetPoint3DForPoint2D(point2D_idx, point3D_ids[point2D_idx]);
      }
    }

    images_.emplace(image.ImageId(), image);
  }
}

void Reconstruction::ReadPoints3DText(const std::string& path) {
  points3D_.clear();

  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::in);
  //CHECK(file.is_open()) << path;

  std::string line;
  std::string item;

  while (std::getline(file, line)) {
    StringTrim(&line);

    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::stringstream line_stream(line);

    // ID
    std::getline(line_stream, item, ' ');
    const point3D_t point3D_id = std::stoll(item);

    // Make sure, that we can add new 3D points after reading 3D points
    // without overwriting existing 3D points.
    num_added_points3D_ = std::max(num_added_points3D_, point3D_id);

    class Point3D point3D;

    // XYZ
    std::getline(line_stream, item, ' ');
    point3D.XYZ(0) = std::stold(item);

    std::getline(line_stream, item, ' ');
    point3D.XYZ(1) = std::stold(item);

    std::getline(line_stream, item, ' ');
    point3D.XYZ(2) = std::stold(item);

    // Color
    std::getline(line_stream, item, ' ');
    point3D.Color(0) = static_cast<uint8_t>(std::stoi(item));

    std::getline(line_stream, item, ' ');
    point3D.Color(1) = static_cast<uint8_t>(std::stoi(item));

    std::getline(line_stream, item, ' ');
    point3D.Color(2) = static_cast<uint8_t>(std::stoi(item));

    // ERROR
    std::getline(line_stream, item, ' ');
    point3D.SetError(std::stold(item));

    // TRACK
    while (!line_stream.eof()) {
      TrackElement track_el;

      std::getline(line_stream, item, ' ');
      StringTrim(&item);
      if (item.empty()) {
        break;
      }
      track_el.image_id = std::stoul(item);

      std::getline(line_stream, item, ' ');
      track_el.point2D_idx = std::stoul(item);

      point3D.Track().AddElement(track_el);
    }

    point3D.Track().Compress();

    points3D_.emplace(point3D_id, point3D);
  }
}

void Reconstruction::ReadCamerasBinary(const std::string& path) {
  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::binary);
  //CHECK(file.is_open()) << path;

  const size_t num_cameras = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_cameras; ++i) {
    class Camera camera;
    camera.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));
    camera.SetModelId(ReadBinaryLittleEndian<int>(&file));
    camera.SetWidth(ReadBinaryLittleEndian<uint64_t>(&file));
    camera.SetHeight(ReadBinaryLittleEndian<uint64_t>(&file));
    ReadBinaryLittleEndian<double>(&file, &camera.Params());
    //CHECK(camera.VerifyParams());
    cameras_.emplace(camera.CameraId(), camera);
  }
}

void Reconstruction::ReadImagesBinary(const std::string& path) {
  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::binary);
  //CHECK(file.is_open()) << path;

  const size_t num_reg_images = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_reg_images; ++i) {
      colmap::Image image;

    image.SetImageId(ReadBinaryLittleEndian<image_t>(&file));

    image.Qvec(0) = ReadBinaryLittleEndian<double>(&file);
    image.Qvec(1) = ReadBinaryLittleEndian<double>(&file);
    image.Qvec(2) = ReadBinaryLittleEndian<double>(&file);
    image.Qvec(3) = ReadBinaryLittleEndian<double>(&file);
    image.NormalizeQvec();

    image.Tvec(0) = ReadBinaryLittleEndian<double>(&file);
    image.Tvec(1) = ReadBinaryLittleEndian<double>(&file);
    image.Tvec(2) = ReadBinaryLittleEndian<double>(&file);

    image.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));

    char name_char;
    do {
      file.read(&name_char, 1);
      if (name_char != '\0') {
        image.Name() += name_char;
      }
    } while (name_char != '\0');

    const size_t num_points2D = ReadBinaryLittleEndian<uint64_t>(&file);

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

    image.SetUp(Camera(image.CameraId()));
    image.SetPoints2D(points2D);

    for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
         ++point2D_idx) {
      if (point3D_ids[point2D_idx] != kInvalidPoint3DId) {
        image.SetPoint3DForPoint2D(point2D_idx, point3D_ids[point2D_idx]);
      }
    }

    image.SetRegistered(true);
    reg_image_ids_.push_back(image.ImageId());

    images_.emplace(image.ImageId(), image);
  }
}

void Reconstruction::ReadPoints3DBinary(const std::string& path) {
  std::ifstream file = AI3D::CORE::File::OpenIfstreamUtf8(path, std::ios::binary);
  //CHECK(file.is_open()) << path;

  const size_t num_points3D = ReadBinaryLittleEndian<uint64_t>(&file);
  for (size_t i = 0; i < num_points3D; ++i) {
    class Point3D point3D;

    const point3D_t point3D_id = ReadBinaryLittleEndian<point3D_t>(&file);
    num_added_points3D_ = std::max(num_added_points3D_, point3D_id);

    point3D.XYZ()(0) = ReadBinaryLittleEndian<double>(&file);
    point3D.XYZ()(1) = ReadBinaryLittleEndian<double>(&file);
    point3D.XYZ()(2) = ReadBinaryLittleEndian<double>(&file);
    point3D.Color(0) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.Color(1) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.Color(2) = ReadBinaryLittleEndian<uint8_t>(&file);
    point3D.SetError(ReadBinaryLittleEndian<double>(&file));

    const size_t track_length = ReadBinaryLittleEndian<uint64_t>(&file);
    for (size_t j = 0; j < track_length; ++j) {
      const image_t image_id = ReadBinaryLittleEndian<image_t>(&file);
      const point2D_t point2D_idx = ReadBinaryLittleEndian<point2D_t>(&file);
      point3D.Track().AddElement(image_id, point2D_idx);
    }
    point3D.Track().Compress();

    points3D_.emplace(point3D_id, point3D);
  }
}

void Reconstruction::WriteCamerasText(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc);
  //CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# Camera list with one line of data per camera:" << std::endl;
  file << "#   CAMERA_ID, MODEL, WIDTH, HEIGHT, PARAMS[]" << std::endl;
  file << "# Number of cameras: " << cameras_.size() << std::endl;

  for (const auto& camera : cameras_) {
    std::ostringstream line;
    line.precision(17);

    line << camera.first << " ";
    line << camera.second.ModelName() << " ";
    line << camera.second.Width() << " ";
    line << camera.second.Height() << " ";

    for (const double param : camera.second.Params()) {
      line << param << " ";
    }

    std::string line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);

    file << line_string << std::endl;
  }
}

void Reconstruction::WriteImagesText(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc);
  //CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# Image list with two lines of data per image:" << std::endl;
  file << "#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, "
          "NAME"
       << std::endl;
  file << "#   POINTS2D[] as (X, Y, POINT3D_ID)" << std::endl;
  file << "# Number of images: " << reg_image_ids_.size()
       << ", mean observations per image: "
       << ComputeMeanObservationsPerRegImage() << std::endl;

  for (const auto& image : images_) 
  {
    /*if (!image.second.IsRegistered()) {
      continue;
    }*/

    std::ostringstream line;
    line.precision(17);

    std::string line_string;

    line << image.first << " ";

    // QVEC (qw, qx, qy, qz)
    const Eigen::Vector4d normalized_qvec =
        NormalizeQuaternion(image.second.Qvec());
    line << normalized_qvec(0) << " ";
    line << normalized_qvec(1) << " ";
    line << normalized_qvec(2) << " ";
    line << normalized_qvec(3) << " ";

    // TVEC
    line << image.second.Tvec(0) << " ";
    line << image.second.Tvec(1) << " ";
    line << image.second.Tvec(2) << " ";

    line << image.second.CameraId() << " ";

    line << image.second.Name();

    file << line.str() << std::endl;

    line.str("");
    line.clear();

    for (const Point2D& point2D : image.second.Points2D()) {
      line << point2D.X() << " ";
      line << point2D.Y() << " ";
      if (point2D.HasPoint3D()) {
        line << point2D.Point3DId() << " ";
      } else {
        line << -1 << " ";
      }
    }
    line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);
    file << line_string << std::endl;
  }
}

void Reconstruction::WritePoints3DText(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc);
  //CHECK(file.is_open()) << path;

  // Ensure that we don't loose any precision by storing in text.
  file.precision(17);

  file << "# 3D point list with one line of data per point:" << std::endl;
  file << "#   POINT3D_ID, X, Y, Z, R, G, B, ERROR, "
          "TRACK[] as (IMAGE_ID, POINT2D_IDX)"
       << std::endl;
  file << "# Number of points: " << points3D_.size()
       << ", mean track length: " << ComputeMeanTrackLength() << std::endl;

  for (const auto& point3D : points3D_) {
    file << point3D.first << " ";
    file << point3D.second.XYZ()(0) << " ";
    file << point3D.second.XYZ()(1) << " ";
    file << point3D.second.XYZ()(2) << " ";
    file << static_cast<int>(point3D.second.Color(0)) << " ";
    file << static_cast<int>(point3D.second.Color(1)) << " ";
    file << static_cast<int>(point3D.second.Color(2)) << " ";
    file << point3D.second.Error() << " ";

    std::ostringstream line;
    line.precision(17);

    for (const auto& track_el : point3D.second.Track().Elements()) {
      line << track_el.image_id << " ";
      line << track_el.point2D_idx << " ";
    }

    std::string line_string = line.str();
    line_string = line_string.substr(0, line_string.size() - 1);

    file << line_string << std::endl;
  }
}

void Reconstruction::WriteCamerasBinary(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  //CHECK(file.is_open()) << path;

  WriteBinaryLittleEndian<uint64_t>(&file, cameras_.size());

  for (const auto& camera : cameras_) {
    WriteBinaryLittleEndian<camera_t>(&file, camera.first);
    WriteBinaryLittleEndian<int>(&file, camera.second.ModelId());
    WriteBinaryLittleEndian<uint64_t>(&file, camera.second.Width());
    WriteBinaryLittleEndian<uint64_t>(&file, camera.second.Height());
    for (const double param : camera.second.Params()) {
      WriteBinaryLittleEndian<double>(&file, param);
    }
  }
}

void Reconstruction::WriteImagesBinary(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  //CHECK(file.is_open()) << path;

  WriteBinaryLittleEndian<uint64_t>(&file, reg_image_ids_.size());

  for (const auto& image : images_) {
    if (!image.second.IsRegistered()) {
      continue;
    }

    WriteBinaryLittleEndian<image_t>(&file, image.first);

    const Eigen::Vector4d normalized_qvec =
        NormalizeQuaternion(image.second.Qvec());
    WriteBinaryLittleEndian<double>(&file, normalized_qvec(0));
    WriteBinaryLittleEndian<double>(&file, normalized_qvec(1));
    WriteBinaryLittleEndian<double>(&file, normalized_qvec(2));
    WriteBinaryLittleEndian<double>(&file, normalized_qvec(3));

    WriteBinaryLittleEndian<double>(&file, image.second.Tvec(0));
    WriteBinaryLittleEndian<double>(&file, image.second.Tvec(1));
    WriteBinaryLittleEndian<double>(&file, image.second.Tvec(2));

    WriteBinaryLittleEndian<camera_t>(&file, image.second.CameraId());

    const std::string name = image.second.Name() + '\0';
    file.write(name.c_str(), name.size());

    WriteBinaryLittleEndian<uint64_t>(&file, image.second.NumPoints2D());
    for (const Point2D& point2D : image.second.Points2D()) {
      WriteBinaryLittleEndian<double>(&file, point2D.X());
      WriteBinaryLittleEndian<double>(&file, point2D.Y());
      WriteBinaryLittleEndian<point3D_t>(&file, point2D.Point3DId());
    }
  }
}

void Reconstruction::WritePoints3DBinary(const std::string& path) const {
  std::ofstream file = AI3D::CORE::File::OpenOfstreamUtf8(path, std::ios::trunc | std::ios::binary);
  ////CHECK(file.is_open()) << path;

  WriteBinaryLittleEndian<uint64_t>(&file, points3D_.size());

  for (const auto& point3D : points3D_) {
    WriteBinaryLittleEndian<point3D_t>(&file, point3D.first);
    WriteBinaryLittleEndian<double>(&file, point3D.second.XYZ()(0));
    WriteBinaryLittleEndian<double>(&file, point3D.second.XYZ()(1));
    WriteBinaryLittleEndian<double>(&file, point3D.second.XYZ()(2));
    WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.Color(0));
    WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.Color(1));
    WriteBinaryLittleEndian<uint8_t>(&file, point3D.second.Color(2));
    WriteBinaryLittleEndian<double>(&file, point3D.second.Error());

    WriteBinaryLittleEndian<uint64_t>(&file, point3D.second.Track().Length());
    for (const auto& track_el : point3D.second.Track().Elements()) {
      WriteBinaryLittleEndian<image_t>(&file, track_el.image_id);
      WriteBinaryLittleEndian<point2D_t>(&file, track_el.point2D_idx);
    }
  }
}


bool Reconstruction::LoadFromXMLFileNew(const std::string& path) 
{

   

    // load file
    pugi::xml_document doc;
    if (doc.load_file(path.c_str()).status != pugi::status_ok)
        return false;

    // parse file
    if (!doc.child("BlocksExchange") || !doc.child("BlocksExchange").child("Block"))
        return false;

    // 1. srss
    //QMap<QString, int> srs_map;
    pugi::xml_node srss = doc.child("BlocksExchange").child("SpatialReferenceSystems");
    if (srss) {
        for (pugi::xml_node node = srss.first_child(); node != NULL; node = node.next_sibling()) {
           std::string name(node.name());
           std::string id;
           std::string epsg;
            if (node.child("Id"))
                id = node.child("Id").text().as_string();

            if (node.child("Definition"))
                epsg = node.child("Definition").text().as_string();

            if (id.empty() || epsg.empty() || epsg.size() <= 5)
                continue;

            int epcode = 4978;
            /*if (epsg.left(4).compare("EPSG:") == 0) {
                epcode = epsg.right(epsg.size() - 5).toInt();
            }*/

            //srs_map.insert(id, epcode);
        }
    }

    // 2. block
    pugi::xml_node block = doc.child("BlocksExchange").child("Block");
    if (!block)
        return false;

    // 2.1 srs
    int coord_type = -1;
    /*if (block.child("SRSId"))
        coord_type = srs_map[block.child("SRSId").text().as_string()];*/

    // 2.2 PhotoGroups
    pugi::xml_node groups = block.child("Photogroups");
    if (groups) 
    {

        int temp_id = 0;

        for (pugi::xml_node group = groups.first_child(); group != NULL; group = group.next_sibling()) {

            temp_id++;

            //// 2.2.1 camera information
            std::string camera_name;
            int camera_id = temp_id;
            int image_width;
            int image_height;
            double focal_length;
            double cx, cy;
            double k1, k2, k3, p1, p2;

            // Id
            if (group.child("Id"))
                camera_id = group.child("Id").text().as_int();

            // ImageDimensions
            pugi::xml_node dimensions = group.child("ImageDimensions");
            if (dimensions)
                if (dimensions.child("Width"))
                    image_width = dimensions.child("Width").text().as_int();
            if (dimensions.child("Height"))
                image_height = dimensions.child("Height").text().as_int();
            {
                double focal_lengthmm, ccdsize;
                if (group.child("FocalLength"))
                {
                    focal_lengthmm = group.child("FocalLength").text().as_double();
                    if (group.child("SensorSize"))
                        ccdsize = group.child("SensorSize").text().as_double();
                    focal_length = focal_lengthmm / (ccdsize / std::max(image_width, image_height));
                }
                else {
                    // FocalLengthPixels
                    if (group.child("FocalLengthPixels"))
                        focal_length = group.child("FocalLengthPixels").text().as_double();
                }
            }
            // PrincipalPoint
            pugi::xml_node principalPoint = group.child("PrincipalPoint");
            if (principalPoint)
                if (principalPoint.child("x"))
                    cx = principalPoint.child("x").text().as_int();
            if (principalPoint.child("y"))
                cy = principalPoint.child("y").text().as_int();

            // Distortion
            pugi::xml_node distortion = group.child("Distortion");
            if (distortion)
                if (distortion.child("K1"))
                    k1 = distortion.child("K1").text().as_double();
            if (distortion.child("K2"))
                k2 = distortion.child("K2").text().as_double();
            if (distortion.child("K3"))
                k3 = distortion.child("K3").text().as_double();
            if (distortion.child("P1"))
                p1 = distortion.child("P1").text().as_double();
            if (distortion.child("P2"))
                p2 = distortion.child("P2").text().as_double();

            // camera name
            if (!group.children("Photo").empty()) 
            {
                if (group.children("Photo").begin()->child("ImagePath")) 
                {
                    std::string path = group.children("Photo").begin()->child("ImagePath").text().as_string();
                    //camera_name = path;
                }
            }

            class Camera camera;
            //camera.InitializeWithName("SIMPLE_RADIAL", focal_length, image_width, image_height);
            camera.InitializeWithName("FULL_OPENCV", focal_length, image_width, image_height);
            camera.SetParams({ focal_length,focal_length,cx,cy,k1,k2,p2,p1,k3,0,0,0 });
            camera.SetCameraId(camera_id);
            camera_name = std::to_string(camera_id);
            //camera.setCameraName(camera_name);
            cameras_[camera_id] = camera;


            //// 2.2.2  Photos
            if (group.children("Photo").empty())
                continue;

            for (auto& photo : group.children("Photo")) 
            {

                // info
                int photo_id;
                std::string image_path;
                double near_depth = .0f;
                double median_depth = .0f;
                double far_depth = .0f;

                if (photo.child("Id"))
                    photo_id = photo.child("Id").text().as_int();
                if (photo.child("ImagePath"))
                    image_path = photo.child("ImagePath").text().as_string();
                if (photo.child("NearDepth"))
                    near_depth = photo.child("NearDepth").text().as_double();
                if (photo.child("MedianDepth"))
                    median_depth = photo.child("MedianDepth").text().as_double();
                if (photo.child("FarDepth"))
                    far_depth = photo.child("FarDepth").text().as_double();

                // pos
                Eigen::Matrix3d rotation;
                Eigen::Vector3d center;
                pugi::xml_node node_pose = photo.child("Pose");
                if (node_pose)
                {

                    // Rotation
                    for (int i = 0; i < 3; ++i) {
                        for (int j = 0; j < 3; ++j) {
                            pugi::xml_node node = node_pose.child("Rotation");
                            std::string flag = std::string("M_") + std::to_string(i) + std::to_string(j);
                            if (node && node.child(flag.c_str()))
                                rotation(i, j) = node.child(flag.c_str()).text().as_double();
                        }
                    }

                    // Center
                    pugi::xml_node node_center = node_pose.child("Center");
                    if (node_center) {

                        if (node_center.child("x") && node_center.child("y") && node_center.child("z")) {
                            center(0) = node_center.child("x").text().as_double();
                            center(1) = node_center.child("y").text().as_double();
                            center(2) = node_center.child("z").text().as_double();

                           /* if (coord_type != 4978) 
                            {
                                double tmp_center[3];
                                coord_trans.UTMToEPSG4978(center[0], center[1], center[2], &tmp_center[0], &tmp_center[1], &tmp_center[2], QString::number(coord_type).toStdString().c_str());
                                center[0] = tmp_center[0];
                                center[1] = tmp_center[1];
                                center[2] = tmp_center[2];
                            }*/
                        }

                        // camera z max value
                      /*  if (MaxCamera_z < center(2)) 
                        { MaxCamera_z = center(2); }*/

                    }
                }

                // ColorParameter
                Eigen::Vector3d color_parameter;
                pugi::xml_node node_colorparameter = photo.child("ColorParameter");
                if (node_colorparameter) {

                    if (node_colorparameter.child("P0"))
                        color_parameter(0) = node_colorparameter.child("P0").text().as_double();
                    if (node_colorparameter.child("P1"))
                        color_parameter(1) = node_colorparameter.child("P1").text().as_double();
                    if (node_colorparameter.child("P2"))
                        color_parameter(2) = node_colorparameter.child("P2").text().as_double();

                }

                colmap::Image image;
                image.SetCameraId(camera_id);
                image.SetQvec(RotationMatrixToQuaternion(rotation));
                image.SetTvec(-1 * image.RotationMatrix() * center);
                image.SetImageId(photo_id);
                std::filesystem::path p(image_path);
                std::string filenmae = p.filename().string();
                image.SetName(filenmae);
                images_[photo_id] = image;
                reg_image_ids_.push_back(photo_id);
            }
        }
    }

    // 2.3 TiePoints
    pugi::xml_node tie_points = block.child("TiePoints");
    if (tie_points) {

        for (pugi::xml_node tie_point = tie_points.first_child(); tie_point != nullptr; tie_point = tie_point.next_sibling()) 
        {
            Eigen::Vector3d point;
            Eigen::Matrix<uint8_t, 3, 1> rgb;

            pugi::xml_node position = tie_point.child("Position");
            if (position) 
            {
                if (position.child("x") && position.child("y") && position.child("z")) 
                {
                    point(0) = position.child("x").text().as_double();
                    point(1) = position.child("y").text().as_double();
                    point(2) = position.child("z").text().as_double();

                    double tmp_pos[3];
                   /* if (coord_type != 4978)
                    {
                        coord_trans.UTMToEPSG4978(point[0], point[1], point[2], &tmp_pos[0], &tmp_pos[1], &tmp_pos[2], QString::number(coord_type).toStdString().c_str());
                        point[0] = tmp_pos[0];
                        point[1] = tmp_pos[1];
                        point[2] = tmp_pos[2];
                    }*/

                  /*  if (point(0) > Points_statis.xmax_) { Points_statis.xmax_ = point(0); }
                    if (point(0) < Points_statis.xmin_) { Points_statis.xmin_ = point(0); }
                    if (point(1) > Points_statis.ymax_) { Points_statis.ymax_ = point(1); }
                    if (point(1) < Points_statis.ymin_) { Points_statis.ymin_ = point(1); }
                    if (point(2) > Points_statis.zmax_) { Points_statis.zmax_ = point(2); }
                    if (point(2) < Points_statis.zmin_) { Points_statis.zmin_ = point(2); }*/
                }
            }

            pugi::xml_node color = tie_point.child("Color");
            if (color) 
            {
                if (color.child("Red") && color.child("Green") && color.child("Blue"))
                {

                    rgb(0) = int(color.child("Red").text().as_double() * 255);
                    rgb(1) = int(color.child("Green").text().as_double() * 255);
                    rgb(2) = int(color.child("Blue").text().as_double() * 255);
                }
            }
            class Point3D pt3d;
            pt3d.SetXYZ(point);
            points3D_[num_added_points3D_] = pt3d;
            points3D_[num_added_points3D_].SetTrack(class Track());

            if (tie_point.child("Measurement")) 
            {
                for (auto item : tie_point.children("Measurement")) 
                {

                    if (!item.child("PhotoId") || !item.child("x") || !item.child("y"))
                        continue;

                    int photo_id = item.child("PhotoId").text().as_int();
                    Eigen::Vector2d point;
                    colmap::Image& image = this->Image(photo_id);
                    point(0) = item.child("x").text().as_double();
                    point(1) = item.child("y").text().as_double();
                    point2D_t point_id = image.SetPoint2D(class Point2D(point, num_added_points3D_));

                    points3D_[num_added_points3D_].Track().AddElement(class TrackElement(photo_id, point_id));
                }
            }

            ++num_added_points3D_;
        }
    }

    return true;
}


}  // namespace colmap
