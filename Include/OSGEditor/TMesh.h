

#ifndef T_OSG_MESH_H
#define T_OSG_MESH_H
#include "OSGEditor/Base.h"
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include <list>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <iostream>

#include <osg/TriangleIndexFunctor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TriStripVisitor>
#include <osg/Texture2D>
#include <osg/StateSet>

#include <Eigen/Dense>

#include <opencv2/opencv.hpp>

#define PIXELNODE 32

typedef double error_type;       /**< 对double数据类型的重命名   */

struct dereference_less
{
	/**
	@brief     比较大小的仿函数，比较的对象是后续定义点、边、三角形
			   主要用于定于set中的点、边、三角形的排列顺序
	*/
	template<class T, class U>
	inline bool operator() (const T& lhs, const U& rhs) const
	{
		return *lhs < *rhs;
	}
};

/**
*	@brief	        比较大小的魔板函数，比较的对象是后续定义点、边、三角形
*	@param	[in]    lhs    比较对象1
*	@param	[in]    rhs    比较对象2
*	@return         如果对象1小于对象2返回true，反之返回false
*/
template<class T>
bool dereference_check_less(const T& lhs, const T& rhs)
{
	if (lhs == rhs) return false;
	if (!lhs) return true;
	if (!rhs) return false;
	return *lhs < *rhs;
}

struct dereference_clear
{
	/**
	@brief     清除数据的仿函数，主要用于清除set中的数据
	*/
	template<class T>
	inline void operator() (const T& t)
	{
		T& non_const_t = const_cast<T&>(t);
		non_const_t->clear();
	}
};


class OSGEDITOR_INTERNAL_CLASS TMesh
{
public:
	struct Triangle;
	struct Edge;
	struct Point;

	typedef std::vector<float>                                                  FloatList;
	typedef std::set<osg::ref_ptr<Edge>, dereference_less >                     EdgeSet;
	typedef std::set< osg::ref_ptr<Point>, dereference_less >                   PointSet;
	typedef std::vector< osg::ref_ptr<Point> >                                  PointList;
	typedef std::list< osg::ref_ptr<Triangle> >                                 TriangleList;
	typedef std::set< osg::ref_ptr<Triangle> >									TriangleSet;
	typedef std::map< osg::ref_ptr<Triangle>, unsigned int, dereference_less >  TriangleMap;
	typedef std::map< osg::ref_ptr<Edge>, osg::ref_ptr<Edge> >                  EdgeMap;

	struct Point : public osg::Referenced
	{
		Point() : protected_(false), index_(0), isModified_(false)
		{
			updateDeltaMat_ = true;
			deltaMat_.setZero(4, 4);
		}


		bool                protected_;          /**< 是否是保护点   */
		bool                isModified_;
		unsigned int        index_;              /**< 点索引   */
		osg::Vec3           vertex_;             /**< 点坐标   */
		FloatList           attributes_;         /**< 点属性vector   */
		TriangleSet         triangles_;          /**< 共享该点的三角形   */
		Eigen::MatrixXd     deltaMat_;
		bool                updateDeltaMat_;

		void clear()
		{
			attributes_.clear();
			triangles_.clear();
		}

		bool operator < (const Point& rhs) const
		{
			if (vertex_ < rhs.vertex_) return true;
			if (rhs.vertex_ < vertex_) return false;

			return attributes_ < rhs.attributes_;
		}

		bool IsBoundaryPoint() const
		{
			if (protected_) return true;

			for (TriangleSet::const_iterator itr = triangles_.begin();
			itr != triangles_.end();
				++itr)
			{
				const Triangle* triangle = itr->get();
				if ((triangle->e1_->p1_ == this || triangle->e1_->p2_ == this) && triangle->e1_->IsBoundaryEdge()) return true;
				if ((triangle->e2_->p1_ == this || triangle->e2_->p2_ == this) && triangle->e2_->IsBoundaryEdge()) return true;
				if ((triangle->e3_->p1_ == this || triangle->e3_->p2_ == this) && triangle->e3_->IsBoundaryEdge()) return true;

				//if ((*itr)->isBoundaryTriangle()) return true;
			}
			return false;
		}

	};

	struct Edge : public osg::Referenced
	{
		Edge() :
			errormetric_(0.0),
			maximumdeviation_(1.0),
			isboundaryedge_(false)
		{}

		osg::ref_ptr<Point>             p1_;                       /**< 边的第一个端点   */
		osg::ref_ptr<Point>             p2_;                       /**< 边的第二个端点   */
		TriangleSet                     triangles_;                /**< 与边相关的三角形   */
		error_type                      errormetric_;              /**< 边的误差测度   */
		error_type                      maximumdeviation_;
		osg::ref_ptr<Point>             proposedPoint_;            /**< 边折叠后退化后的点   */
		bool                            isboundaryedge_;           /**< 是否是边界边   */

		void clear()
		{
			p1_ = 0;
			p2_ = 0;
			triangles_.clear();
		}


		void setErrorMetric(error_type errorMetric) { errormetric_ = errorMetric; }
		error_type getErrorMetric() const { return errormetric_; }

		bool operator < (const Edge& rhs) const
		{
			// both error metrics are computed
			if (getErrorMetric() < rhs.getErrorMetric()) return true;
			else if (rhs.getErrorMetric() < getErrorMetric()) return false;

			if (dereference_check_less(p1_, rhs.p1_)) return true;
			if (dereference_check_less(rhs.p1_, p1_)) return false;

			return dereference_check_less(p2_, rhs.p2_);
		}

		bool operator == (const Edge& rhs) const
		{
			if (&rhs == this) return true;
			if (*this < rhs) return false;
			if (rhs < *this) return false;
			return true;
		}

		bool operator != (const Edge& rhs) const
		{
			if (&rhs == this) return false;
			if (*this < rhs) return true;
			if (rhs < *this) return true;
			return false;
		}

		void addTriangle(Triangle* triangle)
		{
			triangles_.insert(triangle);
		}

		bool IsBoundaryEdge() const
		{
			return triangles_.size() <= 1;
		}

		bool IsAdjacentToBoundary() const
		{
			return IsBoundaryEdge() || p1_->IsBoundaryPoint() || p2_->IsBoundaryPoint();
		}

		error_type GetEdgeLength()
		{
			osg::Vec3 detp = p1_->vertex_ - p2_->vertex_;
			return detp.length();
		}


		void UpdateMaxNormalDeviationOnEdgeCollapse()
		{
			//OSG_NOTICE<<"updateMaxNormalDeviationOnEdgeCollapse()"<<std::endl;
			maximumdeviation_ = 0.0f;
			for (TriangleSet::iterator itr1 = p1_->triangles_.begin();
			itr1 != p1_->triangles_.end();
				++itr1)
			{
				if (triangles_.count(*itr1) == 0)
				{
					maximumdeviation_ = osg::maximum(maximumdeviation_, (*itr1)->ComputeNormalDeviationOnEdgeCollapse(this, proposedPoint_.get()));
				}
			}
			for (TriangleSet::iterator itr2 = p2_->triangles_.begin();
			itr2 != p2_->triangles_.end();
				++itr2)
			{
				if (triangles_.count(*itr2) == 0)
				{
					maximumdeviation_ = osg::maximum(maximumdeviation_, (*itr2)->ComputeNormalDeviationOnEdgeCollapse(this, proposedPoint_.get()));
				}
			}
		}

		error_type GetMaxNormalDeviationOnEdgeCollapse() const { return maximumdeviation_; }

	};

	struct Triangle : public osg::Referenced
	{
		Triangle() {}

		osg::ref_ptr<Point>          p1_;               /**< 三角形顶点   */
		osg::ref_ptr<Point>          p2_;
		osg::ref_ptr<Point>          p3_;

		osg::ref_ptr<Edge>           e1_;               /**< 三角形边   */
		osg::ref_ptr<Edge>           e2_;
		osg::ref_ptr<Edge>           e3_;

		osg::Plane                   plane_;            /**< 三角形平面   */

		error_type                   area_;             /**< 三角形面积   */

		void clear()
		{
			p1_ = 0;
			p2_ = 0;
			p3_ = 0;

			e1_ = 0;
			e2_ = 0;
			e3_ = 0;
		}

		inline bool operator < (const Triangle& rhs) const
		{
			if (dereference_check_less(p1_, rhs.p1_)) return true;
			if (dereference_check_less(rhs.p1_, p1_)) return false;


			const Point* lhs_lower = dereference_check_less(p2_, p3_) ? p2_.get() : p3_.get();
			const Point* rhs_lower = dereference_check_less(rhs.p2_, rhs.p3_) ? rhs.p2_.get() : rhs.p3_.get();

			if (dereference_check_less(lhs_lower, rhs_lower)) return true;
			if (dereference_check_less(rhs_lower, lhs_lower)) return false;

			const Point* lhs_upper = dereference_check_less(p2_, p3_) ? p3_.get() : p2_.get();
			const Point* rhs_upper = dereference_check_less(rhs.p2_, rhs.p3_) ? rhs.p3_.get() : rhs.p2_.get();

			return dereference_check_less(lhs_upper, rhs_upper);
		}

		void SetOrderedPoints(Point* p1, Point* p2, Point* p3)
		{
			Point* points[3];
			points[0] = p1;
			points[1] = p2;
			points[2] = p3;

			// find the lowest value point in the list.
			unsigned int lowest = 0;
			if (dereference_check_less(points[1], points[lowest])) lowest = 1;
			if (dereference_check_less(points[2], points[lowest])) lowest = 2;

			p1_ = points[lowest];
			p2_ = points[(lowest + 1) % 3];
			p3_ = points[(lowest + 2) % 3];
		}

		void update()
		{
			plane_.set(p1_->vertex_, p2_->vertex_, p3_->vertex_);
			area_ = Area();

			

		}

		osg::Plane ComputeNewPlaneOnEdgeCollapse(Edge* edge, Point* pNew) const
		{
			const Point* p1 = (p1_ == edge->p1_ || p1_ == edge->p2_) ? pNew : p1_.get();
			const Point* p2 = (p2_ == edge->p1_ || p2_ == edge->p2_) ? pNew : p2_.get();
			const Point* p3 = (p3_ == edge->p1_ || p3_ == edge->p2_) ? pNew : p3_.get();

			return osg::Plane(p1->vertex_, p2->vertex_, p3->vertex_);
		}

		error_type ComputerAngleOnPoint(Point* tPoint) const
		{
			osg::Vec3 endpoint1;
			osg::Vec3 endpoint2;
			if (tPoint == p1_.get())
			{
				endpoint1 = p2_->vertex_;
				endpoint2 = p3_->vertex_;
			}
			if (tPoint == p2_.get())
			{
				endpoint1 = p3_->vertex_;
				endpoint2 = p1_->vertex_;
			}
			if (tPoint == p3_.get())
			{
				endpoint1 = p1_->vertex_;
				endpoint2 = p2_->vertex_;
			}

			osg::Vec3 d1 = endpoint1 - tPoint->vertex_;
			osg::Vec3 d2 = endpoint2 - tPoint->vertex_;

			d1.normalize();
			d2.normalize();
			error_type vvv = acos(d1[0] * d2[0] + d1[1] * d2[1] + d1[2] * d2[2]) / osg::PI;
			if (/*_isnan(vvv)*/std::isnan(vvv))//by zmz
			{
				vvv = 0.333333;
			}
			return vvv;

		}

		// note return 1 - dotproduct, so that deviation is in the range of 0.0 to 2.0, where 0 is coincident, 1.0 is 90 degrees, and 2.0 is 180 degrees.
		error_type ComputeNormalDeviationOnEdgeCollapse(Edge* edge, Point* pNew) const
		{
			const Point* p1 = (p1_ == edge->p1_ || p1_ == edge->p2_) ? pNew : p1_.get();
			const Point* p2 = (p2_ == edge->p1_ || p2_ == edge->p2_) ? pNew : p2_.get();
			const Point* p3 = (p3_ == edge->p1_ || p3_ == edge->p2_) ? pNew : p3_.get();

			osg::Vec3 new_normal = (p2->vertex_ - p1->vertex_) ^ (p3->vertex_ - p2->vertex_);
			new_normal.normalize();

			error_type result = 1.0 - (new_normal.x() * plane_[0] + new_normal.y() * plane_[1] + new_normal.z() * plane_[2]);
			return result;
		}

		error_type Distance(const osg::Vec3& vertex) const
		{
			return error_type(plane_[0])*error_type(vertex.x()) +
				error_type(plane_[1])*error_type(vertex.y()) +
				error_type(plane_[2])*error_type(vertex.z()) +
				error_type(plane_[3]);
		}

		/**
		*	@brief		海伦公式计算三角形面积
		*	@return     三角形面积
		*/
		error_type Area() const
		{
			//if (e1_.get() == NULL || e2_.get() == NULL || e3_.get() == NULL) return 0.0;
			error_type a = 0.0;
			if (e1_.get() != NULL)	a = e1_->GetEdgeLength();

			error_type b = 0.0;
			if (e2_.get() != NULL) b = e2_->GetEdgeLength();

			error_type c = 0.0;
			if (e2_.get() != NULL) c = e3_->GetEdgeLength();

			error_type p = (a + b + c) / 2.0;
			return sqrt(p * (p - a) * (p - b) * (p - c));
		}

		bool IsBoundaryTriangle() const
		{
			return (e1_->IsBoundaryEdge() || e2_->IsBoundaryEdge() || e3_->IsBoundaryEdge());
		}

		/**
		*	@brief		判断三角形是否是狭长三角形
		*               设置的阈值为10度，即三角形中出现小于10度或大于170度
		*               的三角形即认为该三角形为狭长三角形
		*	@param
		*	@return     如果是狭长三角形返回true,否则返回false
		*/
		bool IsLongNarrowTriangle() const
		{


			error_type a = e1_->GetEdgeLength();
			error_type b = e2_->GetEdgeLength();
			error_type c = e3_->GetEdgeLength();
			error_type r = (4 * sqrt(3) * area_) / (a*a + b*b + c*c);

			if (fabs(r) < 0.01)
				return true;
			else
				return false;

		}

		bool IsLoneTriangle() const
		{
			if (p1_->triangles_.size() == 1 ||
				p2_->triangles_.size() == 1 ||
				p3_->triangles_.size() == 1)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	};


public:
	TMesh();
	TMesh(bool isparseAtt);

	~TMesh();

	osg::Geometry *GetGeometry()
	{
		return geometry_.release();
	}

	unsigned int GetNumOfTriangles()
	{
		return triangleset_.size();
	}


	void SetGeometry(osg::Geometry* geometry, const std::vector<unsigned int>& protectedPoints);

	void SetGeometry(osg::Geometry* geometry);

	void CopyBackToGeometry();

	Triangle* AddTriangle(unsigned int p1, unsigned int p2, unsigned int p3);

	Triangle* AddTriangle(Point* p1, Point* p2, Point* p3);

	void RemoveTriangle(Triangle* triangle);

	void ReplaceTrianglePoint(Triangle* triangle, Point* pOriginal, Point* pNew);

	unsigned int TestTriangle(Triangle* triangle);

	unsigned int TestAllTriangles();

	virtual Edge* AddEdge(Triangle* triangle, Point* p1, Point* p2);

	void RemoveEdge(Triangle* triangle, Edge* edge);

	Edge* ReplaceEdgePoint(Edge* edge, Point* pOriginal, Point* pNew);

	unsigned int TestEdge(Edge* edge);

	unsigned int TestAllEdges();

	unsigned int ComputeNumBoundaryEdges();

	Point* AddPoint(Triangle* triangle, unsigned int p1);

	Point* AddPoint(Triangle* triangle, Point* point);

	void RemovePoint(Triangle* triangle, Point* point);

	unsigned int TestPoint(Point* point);

	unsigned int TestAllPoints();

	int SetBoundaryEdgeProtect();

	int BuildBoundyEdgeMap();

	int RemoveSuspensionTriangle();

public:

	typedef std::vector< osg::ref_ptr<osg::Array> > ArrayList;

	EdgeSet                     edgeset_;                      /**< 三角网的所有边集合   */
	TriangleSet                 triangleset_;                  /**< 三角网的所有三角面片集合   */
	PointSet                    pointset_;                     /**< 三角网的所有顶点集合   */
	PointList                   originalpointlist_;            /**< 三角网的原始顶点数组，可能还有重复项   */
	EdgeMap                     boundyedgemap_;                /**< 三角网的所有边集合   */

protected:

	bool                                     isParseAtt_;                   /**< 是否解析三角网的属性信息   */
	osg::ref_ptr<osg::Geometry>              geometry_;                     /**< 与三角网对应的几何体   */

private:
	void CollectTriange();

	void DrawArraysType(GLenum mode, GLint first, GLsizei count);

	void DrawElementsUByteType(GLenum mode, GLsizei count, const GLubyte* indices);

	void DrawElementsUShortType(GLenum mode, GLsizei count, const GLushort* indices);

	void DrawElementsUIntType(GLenum mode, GLsizei count, const GLuint* indices);

};

struct CollectTriangleOperator
{

	CollectTriangleOperator() :mesh_(0) {}


	void setMesh(TMesh* mesh) { mesh_ = mesh;}
	TMesh* mesh_;
	/**<获得三角形顶点索引仿函数   */
	inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
	{
		mesh_->AddTriangle(p1, p2, p3);
	}

};

typedef osg::TriangleIndexFunctor<CollectTriangleOperator> CollectTriangleIndexFunctor;

class CopyArrayToPointsVisitor : public osg::ArrayVisitor
{
public:
	CopyArrayToPointsVisitor(TMesh::PointList& pointList) :pointList_(pointList)
	{
		//
	}

	template<class T>
	void copy(T& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
			pointList_[i]->attributes_.push_back((float)array[i]);
	}

	virtual void apply(osg::Array&) {}
	virtual void apply(osg::ByteArray& array) { copy(array); }
	virtual void apply(osg::ShortArray& array) { copy(array); }
	virtual void apply(osg::IntArray& array) { copy(array); }
	virtual void apply(osg::UByteArray& array) { copy(array); }
	virtual void apply(osg::UShortArray& array) { copy(array); }
	virtual void apply(osg::UIntArray& array) { copy(array); }
	virtual void apply(osg::FloatArray& array) { copy(array); }

	virtual void apply(osg::Vec4ubArray& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			osg::Vec4ub& value = array[i];
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			attributes.push_back((float)value.r());
			attributes.push_back((float)value.g());
			attributes.push_back((float)value.b());
			attributes.push_back((float)value.a());
		}
	}

	virtual void apply(osg::Vec2Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			osg::Vec2& value = array[i];
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			attributes.push_back(value.x());
			attributes.push_back(value.y());
		}
	}

	virtual void apply(osg::Vec3Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			osg::Vec3& value = array[i];
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			attributes.push_back(value.x());
			attributes.push_back(value.y());
			attributes.push_back(value.z());
		}
	}

	virtual void apply(osg::Vec4Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			osg::Vec4& value = array[i];
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			attributes.push_back(value.x());
			attributes.push_back(value.y());
			attributes.push_back(value.z());
			attributes.push_back(value.w());
		}
	}

	TMesh::PointList& pointList_;


protected:

	CopyArrayToPointsVisitor& operator = (const CopyArrayToPointsVisitor&) { return *this; }
};

class CopyVertexArrayToPointsVisitor : public osg::ArrayVisitor
{
public:
	CopyVertexArrayToPointsVisitor(TMesh::PointList& pointList) : pointList_(pointList)
	{
		//
	}

	virtual void apply(osg::Vec2Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i] = new TMesh::Point;
			pointList_[i]->index_ = i;

			osg::Vec2& value = array[i];
			osg::Vec3& vertex = pointList_[i]->vertex_;
			vertex.set(value.x(), value.y(), 0.0f);
		}
	}

	virtual void apply(osg::Vec3Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i] = new TMesh::Point;
			pointList_[i]->index_ = i;

			pointList_[i]->vertex_ = array[i];
		}
	}

	virtual void apply(osg::Vec4Array& array)
	{
		if (pointList_.size() != array.size()) return;

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i] = new TMesh::Point;
			pointList_[i]->index_ = i;

			osg::Vec4& value = array[i];
			osg::Vec3& vertex = pointList_[i]->vertex_;
			vertex.set(value.x() / value.w(), value.y() / value.w(), value.z() / value.w());
		}
	}

	TMesh::PointList& pointList_;

protected:

	CopyVertexArrayToPointsVisitor& operator = (const CopyVertexArrayToPointsVisitor&) { return *this; }

};

class CopyPointsToArrayVisitor : public osg::ArrayVisitor
{
public:
	CopyPointsToArrayVisitor(TMesh::PointList& pointList) :pointList_(pointList), index_(0)
	{
		//
	}
	template<typename T, typename R>
	void copy(T& array, R /*dummy*/)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			if (index_ < pointList_[i]->attributes_.size())
			{
				float val = (pointList_[i]->attributes_[index_]);
				array[i] = R(val);
			}
		}

		++index_;
	}

	// use local typedefs if usinged char,short and int to get round gcc 3.3.1 problem with defining unsigned short()
	typedef unsigned char dummy_uchar;
	typedef unsigned short dummy_ushort;
	typedef unsigned int dummy_uint;

	virtual void apply(osg::Array&) {}
	virtual void apply(osg::ByteArray& array) { copy(array, char()); }
	virtual void apply(osg::ShortArray& array) { copy(array, short()); }
	virtual void apply(osg::IntArray& array) { copy(array, int()); }
	virtual void apply(osg::UByteArray& array) { copy(array, dummy_uchar()); }
	virtual void apply(osg::UShortArray& array) { copy(array, dummy_ushort()); }
	virtual void apply(osg::UIntArray& array) { copy(array, dummy_uint()); }
	virtual void apply(osg::FloatArray& array) { copy(array, float()); }

	virtual void apply(osg::Vec4ubArray& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			array[i].set((unsigned char)attributes[index_],
				(unsigned char)attributes[index_ + 1],
				(unsigned char)attributes[index_ + 2],
				(unsigned char)attributes[index_ + 3]);
		}
		index_ += 4;
	}

	virtual void apply(osg::Vec2Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			if (index_ + 1 < attributes.size()) array[i].set(attributes[index_], attributes[index_ + 1]);
		}
		index_ += 2;
	}

	virtual void apply(osg::Vec3Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			if (index_ + 2 < attributes.size()) array[i].set(attributes[index_], attributes[index_ + 1], attributes[index_ + 2]);
		}
		index_ += 3;
	}

	virtual void apply(osg::Vec4Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			TMesh::FloatList& attributes = pointList_[i]->attributes_;
			if (index_ + 3 < attributes.size()) array[i].set(attributes[index_], attributes[index_ + 1], attributes[index_ + 2], attributes[index_ + 3]);
		}
		index_ += 4;
	}

	TMesh::PointList& pointList_;
	unsigned int index_;

protected:

	CopyPointsToArrayVisitor& operator = (CopyPointsToArrayVisitor&)
	{
		return *this;
	}
};

class NormalizeArrayVisitor : public osg::ArrayVisitor
{
public:
	NormalizeArrayVisitor() {}

	template<typename Itr>
	void normalize(Itr begin, Itr end)
	{
		for (Itr itr = begin;
		itr != end;
			++itr)
		{
			itr->normalize();
		}
	}

	virtual void apply(osg::Vec2Array& array) { normalize(array.begin(), array.end()); }
	virtual void apply(osg::Vec3Array& array) { normalize(array.begin(), array.end()); }
	virtual void apply(osg::Vec4Array& array) { normalize(array.begin(), array.end()); }

};

class CopyPointsToVertexArrayVisitor : public osg::ArrayVisitor
{
public:
	CopyPointsToVertexArrayVisitor(TMesh::PointList& pointList) : pointList_(pointList)
	{
		//
	}

	virtual void apply(osg::Vec2Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i]->index_ = i;
			osg::Vec3& vertex = pointList_[i]->vertex_;
			array[i].set(vertex.x(), vertex.y());
		}
	}

	virtual void apply(osg::Vec3Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i]->index_ = i;
			array[i] = pointList_[i]->vertex_;
		}
	}

	virtual void apply(osg::Vec4Array& array)
	{
		array.resize(pointList_.size());

		for (unsigned int i = 0; i < pointList_.size(); ++i)
		{
			pointList_[i]->index_ = i;
			osg::Vec3& vertex = pointList_[i]->vertex_;
			array[i].set(vertex.x(), vertex.y(), vertex.z(), 1.0f);
		}
	}

	TMesh::PointList& pointList_;

protected:

	CopyPointsToVertexArrayVisitor& operator = (const CopyPointsToVertexArrayVisitor&)
	{
		return *this;
	}
};


class osg::Image;
class MyMesh;
//关于TMesh的一些转换
class  TMeshConvUtil
{
public:
	static osg::Geometry* ConvTriangleToGeode(TMesh::TriangleSet& triset);

	static	osg::Image* ConvCvMat2OsgImage(cv::Mat& mat);

	static	cv::Mat ConvOsgImage2CvMat(osg::Image* image);

	static	void ConvTriangle2VcgMesh(TMesh::TriangleSet& triset, MyMesh& result);

	static	void ConvVcgMesh2TriangleSet(MyMesh& vcgmesh, TMesh& result);

};

//关于TMesh的一些操作
class  TMeshCalcUtil
{
public:
	//计算包围矩形
	static cv::Rect2f CalcCoordRect(std::vector<cv::Point2f> coords);

	//根据三维模型计算纹理分辨率
	//输入模型需纹理
	//gsd_type = 0, 1, 2
	//gsd_type = 0  返回平均值
	//gsd_type = 1  返回最小值
	//gsd_type = 2  返回最大值
	static float CalcGSDFromModel(osg::Node* node, unsigned int gsd_type = 0);
	static int CalcLayerNum(osg::Node* node, float gsd);
	static int CalcLayerNum1(osg::Node* node, float gsd);

	static bool  IsPointInPolygon(osg::Vec3 point, std::vector<osg::Vec3> poly);

};




#endif // !ES_OSG_MESH_H
