#pragma once

#ifndef _AI3D_MEASUREPOINT_H_
#define _AI3D_MEASUREPOINT_H_

#include <fstream>
#include <Constants.h>
#include <unordered_map>

#include "Core/Types.h"
#include "Core/Point3d.h"
#include "Core/Image.h"
#include "Core/ATData.h"

namespace AI3D
{
	namespace CORE
	{
		
		typedef uint64_t unifyid;
		class ATData;
		
		class AI3D_API SelectPoint
		{

		public:

			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			SelectPoint();
			SelectPoint(image_t id, Eigen::Vector2d xy);

		public:
			void SetImageId(image_t id);
			image_t GetImageId()const;

			void Setxy(Eigen::Vector2d xy);
			Eigen::Vector2d Getxy()const;
			
		protected:
			image_t imageId_;
			Eigen::Vector2d xy_;

		};

		class AI3D_API MeasurePoints
		{

		public:

			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			MeasurePoints();

			void SetMeasurePoints(EIGEN_STL_UMAP(point3D_t, Point3D)& points);
			EIGEN_STL_UMAP(point3D_t, Point3D) GetMeasurePoints()const;

			bool addMeasurePoint(Point3D point);  
			bool addMeasurePoint(ATData& data, std::string name);
			bool deleteMeasurePoint(const point3D_t idx);  
			bool modifyMeasurePoint(Point3D point);    


		protected:
			EIGEN_STL_UMAP(point3D_t, Point3D) user_points3D_;
		};

		
		
		
		
		
		
		
		
		
		
		
		

		
		enum class CONSTRAINT_TYPE : uint8_t
		{
			CONSTRAINT_UNKNOWN = 0,        
			CONSTRAINT_SCALE = 1,        
			CONSTRAINT_ORIGIN = 2,       
			CONSTRAINT_AXIS = 3,         
			CONSTRAINT_PLAIN = 4,		 
		};

		
		enum class SCALE_UNIT_TYPE : int
		{
			UNIT_UNIT = 0,        
			UNIT_CENTI = 1,       
			UNIT_METER = 2,         
			UNIT_KILOMETER = 3,		 
		};

		
		enum class AXIS_TYPE : uint8_t
		{
			AXIS_X = 0,       
			AXIS_Y = 1,       
			AXIS_Z = 2,        
		};

		enum class DIRECTION_TYPE : uint8_t
		{
			DIRCTION_X_P = 0,     
			DIRCTION_X_N = 1,     
			DIRCTION_Y_P = 2,     
			DIRCTION_Y_N = 3,     
			DIRCTION_Z_P = 4,     
			DIRCTION_Z_N = 5,     
		};

		enum class CONSTRAINT_KEY_TYPE : uint8_t
		{
			KEY_UNKNOWN = 0,        
			KEY_INT = 1,        
			KEY_DOUBLE = 2,       
			KEY_INDEX = 3,         
			KEY_AXIS = 4,		 
			KEY_DIRECTION = 5,		
		};

		
		class AI3D_API ConstraintKV {
		public:
			ConstraintKV();
			ConstraintKV(CONSTRAINT_KEY_TYPE ctype, double cvalue);

			CONSTRAINT_KEY_TYPE getType();
			int getIntValue();
			double getDoubleValue();

		private:
			CONSTRAINT_KEY_TYPE constraintKeyType;
			double constraintValue;
		};

		
		class AI3D_API MeasureConstraint
		{
		public:
			MeasureConstraint();
			MeasureConstraint(CONSTRAINT_TYPE type);     

			
			void SetName(std::string name);
			std::string GetName()const;

			void SetId(constraint_t id);
			constraint_t GetId()const;

			void SetType(CONSTRAINT_TYPE type);
			CONSTRAINT_TYPE GetType()const;

			
			void SetPointList(EIGEN_STL_UMAP(point3D_t, Point3D)& list);
			EIGEN_STL_UMAP(point3D_t, Point3D)& GetPointList();
			
			void SetConstraintItemList(std::vector<ConstraintKV>& list);
			std::vector<ConstraintKV>& GetConstraintItemList();

			bool setConstraint(CONSTRAINT_TYPE type, EIGEN_STL_UMAP(point3D_t, Point3D)& pointList, std::vector<ConstraintKV>& constraintList);

		private:
			std::string name_;
			constraint_t id_;
			EIGEN_STL_UMAP(point3D_t, Point3D) measurePointList_;
			CONSTRAINT_TYPE ctype_;    
			std::vector<ConstraintKV> constraintItemList_;    
		};

		
		class AI3D_API MeasureGroup
		{

		public:

			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			MeasureGroup();

		public:
			
			void SetConstraintList(EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& list);
			EIGEN_STL_UMAP(constraint_t, MeasureConstraint)& GetConstraintList();

			constraint_t addConstraintElement(const MeasureConstraint& element);   
			bool deleteElementByConstraintId(constraint_t id);    
			bool modifyConstraintItem(constraint_t id, CONSTRAINT_TYPE type, EIGEN_STL_UMAP(point3D_t, Point3D)& pointList, std::vector<ConstraintKV>& constraintList);

			
			
			

			
			
			
			
			

			
			bool LoadConstraint(const std::string& path);
			bool SaveConstraint(const std::string& outpath) ;

			
			double calAveScale();                                       
			void updateScaleModle(double scale);                        

		protected:
			EIGEN_STL_UMAP(constraint_t, MeasureConstraint) constraintList_;
			EIGEN_STL_UMAP(point3D_t, Point3D) measurePointList_;
		};

	}
}
#endif 
