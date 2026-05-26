#include"Core/MeasurePoint.h"
#include "Core/CoordinateSystem.h"
#include "Core/ReturnCode.h"
#include "Core/Logging.h"
#include "Core/File.h"
namespace AI3D
{
	namespace CORE
	{
		
		SelectPoint::SelectPoint() :
			imageId_(kInvalidImageId),
			xy_(Eigen::Vector2d(0, 0))
		{

		}

		SelectPoint::SelectPoint(image_t id, Eigen::Vector2d xy) {
			this->imageId_ = id;
			this->xy_ = xy;
		}

		void SelectPoint::SetImageId(image_t id)
		{
			this->imageId_ = id;
		};

		image_t SelectPoint::GetImageId()const
		{
			return imageId_;
		};

		void SelectPoint::Setxy(Eigen::Vector2d xy) {
			this->xy_ = xy;
		}

		Eigen::Vector2d SelectPoint::Getxy()const {
			return xy_;
		}

		

		MeasurePoints::MeasurePoints()
		{
			user_points3D_.clear();
		}


		void MeasurePoints::SetMeasurePoints(EIGEN_STL_UMAP(point3D_t, Point3D)& points) {
			this->user_points3D_ = points;
		}

		EIGEN_STL_UMAP(point3D_t, Point3D) MeasurePoints::GetMeasurePoints()const {
			return user_points3D_;
		}

		bool MeasurePoints::addMeasurePoint(Point3D point) {
			user_points3D_[point.GetId()] = point;
			return true;
		}

		bool MeasurePoints::addMeasurePoint(ATData& data, std::string name) {
			Point3D point;
			point3D_t id = data.GenerateValidUserPtId();
			point.SetId(id);
			point.SetName(name);
			user_points3D_[id] = point;
			return true;
		}

		bool MeasurePoints::deleteMeasurePoint(const point3D_t idx) {
			user_points3D_.erase(idx);
			return true;
		}

		bool MeasurePoints::modifyMeasurePoint(Point3D point) {
			user_points3D_[point.GetId()] = point;
			return true;
		}

		
		ConstraintKV::ConstraintKV() {

		}
		ConstraintKV::ConstraintKV(CONSTRAINT_KEY_TYPE ctype, double cvalue) {
			constraintKeyType = ctype;
			constraintValue = cvalue;
		}

		CONSTRAINT_KEY_TYPE ConstraintKV::getType() {
			return constraintKeyType;
		}

		int ConstraintKV::getIntValue() {
			return static_cast<int>(constraintValue);
		}

		double ConstraintKV::getDoubleValue() {
			return constraintValue;
		}

		
		MeasureConstraint::MeasureConstraint() {
			this->ctype_ = CONSTRAINT_TYPE::CONSTRAINT_UNKNOWN;
		}

		MeasureConstraint::MeasureConstraint(CONSTRAINT_TYPE type) {
			this->ctype_ = type;
		}

		void MeasureConstraint::SetName(std::string name)
		{
			this->name_ = name;
		};

		std::string MeasureConstraint::GetName()const
		{
			return name_;
		};

		void MeasureConstraint::SetId(constraint_t id)
		{
			this->id_ = id;
		};

		constraint_t MeasureConstraint::GetId()const
		{
			return id_;
		};

		void MeasureConstraint::SetType(CONSTRAINT_TYPE type) {
			this->ctype_ = type;
		}

		CONSTRAINT_TYPE MeasureConstraint::GetType()const {
			return ctype_;
		}

		void MeasureConstraint::SetPointList(EIGEN_STL_UMAP(point3D_t, Point3D)& list) {
			this->measurePointList_ = list;
		}

		EIGEN_STL_UMAP(point3D_t, Point3D)& MeasureConstraint::GetPointList() {
			return this->measurePointList_;
		}

		void MeasureConstraint::SetConstraintItemList(std::vector<ConstraintKV>& list) {
			this->constraintItemList_ = list;
		}

		std::vector<ConstraintKV>& MeasureConstraint::GetConstraintItemList() {
			return this->constraintItemList_;
		}

		bool MeasureConstraint::setConstraint(CONSTRAINT_TYPE type, EIGEN_STL_UMAP(point3D_t, Point3D)& pointList, std::vector<ConstraintKV>& constraintList) {
			this->ctype_ = type;
			this->measurePointList_ = pointList;
			this->constraintItemList_ = constraintList;
			return true;
		}

		
		MeasureGroup::MeasureGroup() {
			this->constraintList_.clear();
			
		}

		void MeasureGroup::SetConstraintList(EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& list) {
			this->constraintList_ = list;
		}

		EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& MeasureGroup::GetConstraintList() {
			return this->constraintList_;
		}

		constraint_t MeasureGroup::addConstraintElement(const MeasureConstraint& element) {
			constraint_t id = element.GetId();
			constraintList_[id] = element;
			return id;
		}

		bool MeasureGroup::deleteElementByConstraintId(constraint_t id) {
			
			
			constraintList_.erase(id);
			return true;
		}

		bool MeasureGroup::modifyConstraintItem(constraint_t id, CONSTRAINT_TYPE type, EIGEN_STL_UMAP(point3D_t, Point3D)& pointList, std::vector<ConstraintKV>& constraintList) {
			MeasureConstraint &measureConstraint = constraintList_[id];
			measureConstraint.SetType(type);
			measureConstraint.SetPointList(pointList);
			measureConstraint.SetConstraintItemList(constraintList);
			return true;
		}

		
		
		
		

		
		
		

		

		

		

		

		

		

		

		

		

		

		
		bool MeasureGroup::LoadConstraint(const std::string& path) {
			std::ifstream in = File::OpenIfstreamUtf8(path, std::ios::binary);
			
			if (!in.is_open()) {
				LOGE("Load constraint bin failed!");
				return false;
			}

			ConstraintFile constraintFile;
			constraintFile.Deserialize(in);
			constraintList_.clear();
			bool kvValid = true;
			for (auto& constraintItem : constraintFile.constraintsVec) {
				constraint_t id = constraintItem.id;
				MeasureConstraint measureConstraint;
				measureConstraint.SetId(id);
				measureConstraint.SetName(constraintItem.name);
				CONSTRAINT_TYPE ctype = static_cast<CONSTRAINT_TYPE>(constraintItem.type);
				measureConstraint.SetType(ctype);
				EIGEN_STL_UMAP(point3D_t, Point3D) measurePointList;
				measurePointList.clear();
				for (auto& id : constraintItem.pointIds) {
					point3D_t pid = id;
					Point3D point;
					measurePointList[pid] = point;
				}
				measureConstraint.SetPointList(measurePointList);
				std::vector<double> kvs = constraintItem.values;
				std::vector<ConstraintKV> newkvs;
				newkvs.clear();
				
				if (ctype == CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
					if (kvs.size() < 2) {
						kvValid = false;
						break;
					}
					ConstraintKV dist(CONSTRAINT_KEY_TYPE::KEY_DOUBLE, kvs[0]);
					newkvs.push_back(dist);
					ConstraintKV unit(CONSTRAINT_KEY_TYPE::KEY_INT, kvs[1]);
					newkvs.push_back(unit);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_ORIGIN) {
					if (kvs.size() < 1) {
						kvValid = false;
						break;
					}
					ConstraintKV id(CONSTRAINT_KEY_TYPE::KEY_INDEX, kvs[0]);
					newkvs.push_back(id);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_AXIS) {
					if (kvs.size() < 1) {
						kvValid = false;
						break;
					}
					ConstraintKV axisType(CONSTRAINT_KEY_TYPE::KEY_AXIS, kvs[0]);
					newkvs.push_back(axisType);
				}
				else if (ctype == CONSTRAINT_TYPE::CONSTRAINT_PLAIN) {
					if (kvs.size() < 2) {
						kvValid = false;
						break;
					}
					ConstraintKV axisType(CONSTRAINT_KEY_TYPE::KEY_AXIS, kvs[0]);
					newkvs.push_back(axisType);
					ConstraintKV directType(CONSTRAINT_KEY_TYPE::KEY_DIRECTION, kvs[1]);
					newkvs.push_back(directType);
				}
				else {

				}
				measureConstraint.SetConstraintItemList(newkvs);
				constraintList_[id] = measureConstraint;
			}

			in.close();
			if (!kvValid) {
				return kvValid;
			}
			return true;
		}

		bool MeasureGroup::SaveConstraint(const std::string& outpath) {
			std::ofstream out = File::OpenOfstreamUtf8(outpath, std::ios::binary);
			if (!out.is_open()) {
				LOGE("Save constraint bin failed!");
				return false;
			}
			ConstraintFile constraintFile;
			std::vector<ConstraintData> constraintsVec;
			for (auto& constraintItem : constraintList_) {
				ConstraintData constraintData;
				constraintData.id = constraintItem.second.GetId();
				constraintData.name = constraintItem.second.GetName();
				constraintData.type = static_cast<int>(constraintItem.second.GetType());
				constraintData.pointNum = constraintItem.second.GetPointList().size();
				std::vector<unsigned long long> pointIds;
				for (auto& measureItem : constraintItem.second.GetPointList()) {
					const unsigned long long pointId =
						static_cast<unsigned long long>(measureItem.first);
					pointIds.push_back(pointId);
				}
				constraintData.pointIds = pointIds;
				constraintData.valueNum = constraintItem.second.GetConstraintItemList().size();
				std::vector<double> values;
				for (auto& cvalue : constraintItem.second.GetConstraintItemList()) {
					double tmpValue = cvalue.getDoubleValue();
					values.push_back(tmpValue);
				}
				constraintData.values = values;
				constraintsVec.push_back(constraintData);
			}
			constraintFile.constraintNum = constraintsVec.size();
			constraintFile.constraintsVec = constraintsVec;
			constraintFile.Serialize(out);
			out.close();
			return true;
		}


		namespace {
			
			Eigen::Vector3d ModelPositionForScale(const Point3D& p)
			{
				Point3D q = p;
				if (q.HasEstimatedXYZ()) {
					const Eigen::Vector3d e = q.GetEstimatedXYZ();
					if (e.x() != -DBL_MAX && e.y() != -DBL_MAX && e.z() != -DBL_MAX) {
						return e;
					}
				}
				if (q.HasXYZ()) {
					return q.GetXYZ();
				}
				return Eigen::Vector3d::Zero();
			}
		}

		double MeasureGroup::calAveScale() {
			double scale = 0;
			double sumScale = 0;
			double pointpairNum = 0;
			for (auto& constraintItem : constraintList_) {
				auto ctype = constraintItem.second.GetType();
				if (ctype != CONSTRAINT_TYPE::CONSTRAINT_SCALE) {
					
					scale = -1;
					break;
				}
				MeasureConstraint measureConstraint = constraintItem.second;
				EIGEN_STL_UMAP(point3D_t, Point3D) points = measureConstraint.GetPointList();
				
				if (points.size() < 2) {
					scale = -2;
					break;
				}
				std::vector<Point3D> pointsVec;
				for (auto& point : points) {
					pointsVec.push_back(point.second);
				}
				Point3D point1 = pointsVec[0];
				Point3D point2 = pointsVec[1];
				const Eigen::Vector3d xyz1 = ModelPositionForScale(point1);
				const Eigen::Vector3d xyz2 = ModelPositionForScale(point2);

				Eigen::Vector3d diff = xyz1 - xyz2; 
				double modleDist = diff.norm();
				std::vector<ConstraintKV> kvs = measureConstraint.GetConstraintItemList();
				if (kvs.size() < 2) {
					scale = -3;
					break;
				}
				double oriDist = kvs[0].getDoubleValue();
				SCALE_UNIT_TYPE unit = static_cast<SCALE_UNIT_TYPE>(kvs[1].getIntValue());
				if (unit == SCALE_UNIT_TYPE::UNIT_CENTI) {
					oriDist = oriDist / 100;
				}
				else if (unit == SCALE_UNIT_TYPE::UNIT_KILOMETER) {
					oriDist = oriDist * 1000;
				}
				double tmpScale = modleDist / oriDist;
				sumScale += tmpScale;
				pointpairNum++;
				LOGI("---------oriDist=" + std::to_string(oriDist));
				LOGI("---------modleDist=" + std::to_string(modleDist));
				LOGI("---------tmpScale=" + std::to_string(tmpScale));
			}
			if (scale <0) {
				scale = scale;
			}else if (sumScale == 0) {
				scale = 1;     
			}else {
				scale = sumScale / pointpairNum;   
			}
			LOGI("---------sumScale=" + std::to_string(sumScale));
			LOGI("---------pointpairNum=" + std::to_string(pointpairNum));
			LOGI("---------scale=" + std::to_string(scale));
			return scale;
		}

		void MeasureGroup::updateScaleModle(double scale) {

		}
	}


}