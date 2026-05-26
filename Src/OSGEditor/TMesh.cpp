#include <osgUtil/MeshOptimizers>

#include "OSGEditor/TMesh.h"
#include "OSGEditor/TMeshBase.h"
#include "OSGEditor/TModelIO.h"

TMesh::TMesh()
{
	geometry_ = NULL;
	isParseAtt_ = false;
}

TMesh::TMesh(bool isparseAtt)
{
	geometry_ = NULL;
	isParseAtt_ = isparseAtt;
}


TMesh::~TMesh()
{
	std::for_each(edgeset_.begin(), edgeset_.end(), dereference_clear());
	std::for_each(triangleset_.begin(), triangleset_.end(), dereference_clear());
	std::for_each(pointset_.begin(), pointset_.end(), dereference_clear());
	std::for_each(originalpointlist_.begin(), originalpointlist_.end(), dereference_clear());

}

void TMesh::SetGeometry(osg::Geometry* geometry)
{
	/*const */std::vector<unsigned int> emptyIndexs;
	SetGeometry(geometry, emptyIndexs);
}

void TMesh::SetGeometry(osg::Geometry* geometry, const std::vector<unsigned int>& protectedPoints)
{
	geometry_ = geometry;

	/**<复制共享的属性数组，主要是解决多个顶点共用同一个属性数组  */
	if (geometry_->containsSharedArrays())
	{
		// removing coord indices
		//OSG_INFO << "EdgeCollapse::setGeometry(..): Duplicate shared arrays" << std::endl;
		geometry_->duplicateSharedArrays();
	}
	/**<获得顶点数组中元素个数  */
	unsigned int numVertices = geometry->getVertexArray()->getNumElements();
	originalpointlist_.resize(numVertices);

	/**< 获得几何体顶点数组   */
	CopyVertexArrayToPointsVisitor copyVertexArrayToPoints(originalpointlist_);      /**< 顶点访问器   */
	geometry_->getVertexArray()->accept(copyVertexArrayToPoints);

	if (isParseAtt_)
	{
		/**< 顶点属性访问器   */
		CopyArrayToPointsVisitor        copyArrayToPoints(originalpointlist_);

		/**< 获取顶点纹理坐标   */
		for (unsigned int ti = 0; ti < geometry_->getNumTexCoordArrays(); ++ti)
		{
			if (geometry_->getTexCoordArray(ti))
			{
				geometry->getTexCoordArray(ti)->accept(copyArrayToPoints);
			}
		}

		/**< 获取顶点法向量   */
		if (geometry_->getNormalArray() &&
			geometry_->getNormalArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry->getNormalArray()->accept(copyArrayToPoints);
		}

		/**< 获取顶点颜色   */
		if (geometry_->getColorArray() &&
			geometry_->getColorArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry->getColorArray()->accept(copyArrayToPoints);
		}

		/**< 获取顶点第二颜色   */
		if (geometry_->getSecondaryColorArray() &&
			geometry_->getSecondaryColorArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry->getSecondaryColorArray()->accept(copyArrayToPoints);
		}

		/**< 获取雾坐标   */
		if (geometry_->getFogCoordArray() &&
			geometry_->getFogCoordArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry->getFogCoordArray()->accept(copyArrayToPoints);
		}

		/**< 获取其他属性   */
		for (unsigned int vi = 0; vi < geometry_->getNumVertexAttribArrays(); ++vi)
		{
			if (geometry_->getVertexAttribArray(vi) &&
				geometry_->getVertexAttribArray(vi)->getBinding() == osg::Array::BIND_PER_VERTEX)
			{
				geometry->getVertexAttribArray(vi)->accept(copyArrayToPoints);
			}

		}
	}

	/**< 设置需要保护的顶点   */
	for (auto pitr = protectedPoints.begin();
	pitr != protectedPoints.end();
		++pitr)
	{
		originalpointlist_[*pitr]->protected_ = true;
	}
	/**< 根据顶点索引获得三角形   */
	//CollectTriangleIndexFunctor collectTriangles;
	//collectTriangles.setMesh(this);

	//geometry_->accept(collectTriangles);*/
	CollectTriange();


}

void TMesh::CopyBackToGeometry()
{
	/**< 更新原始的顶点数组   */
	originalpointlist_.clear();
	std::copy(pointset_.begin(), pointset_.end(), std::back_inserter(originalpointlist_));

	/**<更新几何体中的顶点数组   */
	CopyPointsToVertexArrayVisitor copyVertexArrayToPoints(originalpointlist_);
	geometry_->getVertexArray()->accept(copyVertexArrayToPoints);

	if (isParseAtt_)
	{
		/**<更新顶点属性   */
		CopyPointsToArrayVisitor  copyArrayToPoints(originalpointlist_);

		/**<更新顶点纹理坐标   */
		for (unsigned int ti = 0; ti < geometry_->getNumTexCoordArrays(); ++ti)
		{
			if (geometry_->getTexCoordArray(ti))
			{
				geometry_->getTexCoordArray(ti)->accept(copyArrayToPoints);
			}

		}

		/**<更新顶点法线向量   */
		if (geometry_->getNormalArray() && geometry_->getNormalArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry_->getNormalArray()->accept(copyArrayToPoints);

			NormalizeArrayVisitor nav;
			geometry_->getNormalArray()->accept(nav);
		}

		/**<更新顶点颜色   */
		if (geometry_->getColorArray() &&
			geometry_->getColorArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry_->getColorArray()->accept(copyArrayToPoints);
		}

		/**<更新顶点第二颜色   */
		if (geometry_->getSecondaryColorArray() &&
			geometry_->getSecondaryColorArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			geometry_->getSecondaryColorArray()->accept(copyArrayToPoints);
		}

		/**<更新雾坐标   */
		if (geometry_->getFogCoordArray() && geometry_->getFogCoordArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
			geometry_->getFogCoordArray()->accept(copyArrayToPoints);

		/**<更新其他属性   */
		for (unsigned int vi = 0; vi < geometry_->getNumVertexAttribArrays(); ++vi)
		{
			if (geometry_->getVertexAttribArray(vi) &&
				geometry_->getVertexAttribArray(vi)->getBinding() == osg::Array::BIND_PER_VERTEX)
			{
				geometry_->getVertexAttribArray(vi)->accept(copyArrayToPoints);
			}

		}

	}
	else
	{
		/**< 清除顶点纹理坐标   */
		for (unsigned int ti = 0; ti < geometry_->getNumTexCoordArrays(); ++ti)
		{
			if (geometry_->getTexCoordArray(ti))
			{
				//geometry->getTexCoordArray(ti)->resizeArray(0);
				geometry_->setTexCoordArray(ti, NULL);
				osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
				geometry_->setStateSet(stateset.get());
			}
		}

		/**< 清除顶点法向量   */
		if (geometry_->getNormalArray() &&
			geometry_->getNormalArray()->getBinding() == osg::Array::BIND_PER_VERTEX)
		{
			//geometry->getNormalArray()->resizeArray(0);
			geometry_->setNormalArray(NULL);
		}
	}

	typedef std::set< osg::ref_ptr<TMesh::Triangle>, dereference_less >    TrianglesSorted;
	TrianglesSorted trianglesSorted;
	for (TMesh::TriangleSet::iterator itr = triangleset_.begin();
	itr != triangleset_.end();
		++itr)
	{
		trianglesSorted.insert(*itr);
	}

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt(GL_TRIANGLES, trianglesSorted.size() * 3);
	unsigned int pos = 0;
	for (TrianglesSorted::iterator titr = trianglesSorted.begin();
	titr != trianglesSorted.end();
		++titr)
	{
		const TMesh::Triangle* triangle = (*titr).get();
		(*primitives)[pos++] = triangle->p1_->index_;
		(*primitives)[pos++] = triangle->p2_->index_;
		(*primitives)[pos++] = triangle->p3_->index_;
	}

	geometry_->getPrimitiveSetList().clear();
	geometry_->addPrimitiveSet(primitives);

	if (isParseAtt_)
	{
		osgUtil::SmoothingVisitor::smooth(*(geometry_.get()));
	}

}

TMesh::Triangle* TMesh::AddTriangle(unsigned int p1, unsigned int p2, unsigned int p3)
{
	//OSG_NOTICE<<"addTriangle("<<p1<<","<<p2<<","<<p3<<")"<<std::endl;

	// detect if triangle is degenerate.
	if (p1 == p2 || p2 == p3 || p1 == p3) return 0;

	TMesh::Triangle* triangle = new TMesh::Triangle;
	TMesh::Point* points[3];
	points[0] = TMesh::AddPoint(triangle, p1);
	points[1] = TMesh::AddPoint(triangle, p2);
	points[2] = TMesh::AddPoint(triangle, p3);
	// find the lowest value point in the list.
	unsigned int lowest = 0;
	if (dereference_check_less(points[1], points[lowest])) lowest = 1;
	if (dereference_check_less(points[2], points[lowest])) lowest = 2;

	triangle->p1_ = points[lowest];
	triangle->p2_ = points[(lowest + 1) % 3];
	triangle->p3_ = points[(lowest + 2) % 3];

	triangle->e1_ = AddEdge(triangle, triangle->p1_.get(), triangle->p2_.get());
	triangle->e2_ = AddEdge(triangle, triangle->p2_.get(), triangle->p3_.get());
	triangle->e3_ = AddEdge(triangle, triangle->p3_.get(), triangle->p1_.get());

	triangle->update();

	triangleset_.insert(triangle);

	return triangle;
}

TMesh::Triangle* TMesh::AddTriangle(TMesh::Point* p1, TMesh::Point* p2, TMesh::Point* p3)
{
	// detect if triangle is degenerate.
	if (p1 == p2 || p2 == p3 || p1 == p3)
	{
		return 0;
	}

	TMesh::Triangle* triangle = new TMesh::Triangle;

	TMesh::Point* points[3];
	points[0] = TMesh::AddPoint(triangle, p1);
	points[1] = TMesh::AddPoint(triangle, p2);
	points[2] = TMesh::AddPoint(triangle, p3);

	// find the lowest value point in the list.
	unsigned int lowest = 0;
	if (dereference_check_less(points[1], points[lowest])) lowest = 1;
	if (dereference_check_less(points[2], points[lowest])) lowest = 2;

	triangle->p1_ = points[lowest];
	triangle->p2_ = points[(lowest + 1) % 3];
	triangle->p3_ = points[(lowest + 2) % 3];

	triangle->e1_ = AddEdge(triangle, triangle->p1_.get(), triangle->p2_.get());
	triangle->e2_ = AddEdge(triangle, triangle->p2_.get(), triangle->p3_.get());
	triangle->e3_ = AddEdge(triangle, triangle->p3_.get(), triangle->p1_.get());

	triangle->update();

	triangleset_.insert(triangle);

	return triangle;
}

void TMesh::RemoveTriangle(TMesh::Triangle* triangle)
{
	if (triangle->p1_.valid()) RemovePoint(triangle, triangle->p1_.get());
	if (triangle->p2_.valid()) RemovePoint(triangle, triangle->p2_.get());
	if (triangle->p3_.valid()) RemovePoint(triangle, triangle->p3_.get());
							   
	if (triangle->e1_.valid()) RemoveEdge(triangle, triangle->e1_.get());
	if (triangle->e2_.valid()) RemoveEdge(triangle, triangle->e2_.get());
	if (triangle->e3_.valid()) RemoveEdge(triangle, triangle->e3_.get());

	triangleset_.erase(triangle);
}

void TMesh::ReplaceTrianglePoint(TMesh::Triangle* triangle, TMesh::Point* pOriginal, TMesh::Point* pNew)
{
	if (triangle->p1_ == pOriginal || triangle->p2_ == pOriginal || triangle->p3_ == pOriginal)
	{
		// fix the corner points to use the new point
		if (triangle->p1_ == pOriginal) triangle->p1_ = pNew;
		if (triangle->p2_ == pOriginal) triangle->p2_ = pNew;
		if (triangle->p3_ == pOriginal) triangle->p3_ = pNew;

		// fixes the edges so they point to use the new point
		triangle->e1_ = ReplaceEdgePoint(triangle->e1_.get(), pOriginal, pNew);
		triangle->e2_ = ReplaceEdgePoint(triangle->e2_.get(), pOriginal, pNew);
		triangle->e3_ = ReplaceEdgePoint(triangle->e3_.get(), pOriginal, pNew);

		// remove the triangle form the original point, and possibly the point if its the last triangle to use it
		RemovePoint(triangle, pOriginal);

		// add the triangle to that point
		AddPoint(triangle, pNew);
	}
}

unsigned int TMesh::TestTriangle(TMesh::Triangle* triangle)
{
	unsigned int result = 0;
	if (!(triangle->p1_))
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p1_==NULL" << std::endl;
		++result;
	}
	else if (triangle->p1_->triangles_.count(triangle) == 0)
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p1_->triangles_ does not contain triangle" << std::endl;
		++result;
	}

	if (!(triangle->p2_))
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p2_==NULL" << std::endl;
		++result;
	}
	else if (triangle->p2_->triangles_.count(triangle) == 0)
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p2_->triangles_ does not contain triangle" << std::endl;
		++result;
	}

	if (!(triangle->p3_))
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p3_==NULL" << std::endl;
		++result;
	}
	else if (triangle->p3_->triangles_.count(triangle) == 0)
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") p3_->triangles_ does not contain triangle" << std::endl;
		++result;
	}

	if (TestEdge(triangle->e1_.get()))
	{
		++result;
		OSG_NOTICE << "testTriangle(" << triangle << ") e1_ test failed" << std::endl;
	}

	if (TestEdge(triangle->e2_.get()))
	{
		++result;
		OSG_NOTICE << "testTriangle(" << triangle << ") e2_ test failed" << std::endl;
	}

	if (TestEdge(triangle->e3_.get()))
	{
		OSG_NOTICE << "testTriangle(" << triangle << ") e3_ test failed" << std::endl;
		++result;
	}

	return result;
}

unsigned int TMesh::TestAllTriangles()
{
	unsigned int numErrors = 0;
	for (TMesh::TriangleSet::iterator itr = triangleset_.begin();
	itr != triangleset_.end();
		++itr)
	{
		numErrors += TestTriangle(const_cast<TMesh::Triangle*>(itr->get()));
	}
	return numErrors;
}

TMesh::Edge* TMesh::AddEdge(TMesh::Triangle* triangle, TMesh::Point* p1, TMesh::Point* p2)
{

	osg::ref_ptr<TMesh::Edge> edge = new TMesh::Edge;
	if (dereference_check_less(p1, p2))
	{
		edge->p1_ = p1;
		edge->p2_ = p2;
	}
	else
	{
		edge->p1_ = p2;
		edge->p2_ = p1;
	}

	edge->setErrorMetric(0.0);

	TMesh::EdgeSet::iterator itr = edgeset_.find(edge);
	if (itr == edgeset_.end())
	{
		edgeset_.insert(edge);
	}
	else
	{
		edge = *itr;
	}

	edge->addTriangle(triangle);

	return edge.get();
}

void TMesh::RemoveEdge(TMesh::Triangle* triangle, TMesh::Edge* edge)
{
	TMesh::EdgeSet::iterator itr = edgeset_.find(edge);
	if (itr != edgeset_.end())
	{
		edge->triangles_.erase(triangle);
		if (edge->triangles_.empty())
		{
			edge->p1_ = 0;
			edge->p2_ = 0;

			// edge no longer in use, so need to delete.
			edgeset_.erase(itr);
		}
	}
}

TMesh::Edge* TMesh::ReplaceEdgePoint(TMesh::Edge* edge, TMesh::Point* pOriginal, TMesh::Point* pNew)
{
	if (edge->p1_ == pOriginal || edge->p2_ == pOriginal)
	{
		TMesh::EdgeSet::iterator itr = edgeset_.find(edge);
		if (itr != edgeset_.end())
		{
			// remove the edge from the list, as its positoin in the list
			// may need to change once its values have been amended
			edgeset_.erase(itr);
		}

		// modify its values
		if (edge->p1_ == pOriginal) edge->p1_ = pNew;
		if (edge->p2_ == pOriginal) edge->p2_ = pNew;

		if (dereference_check_less(edge->p2_, edge->p1_))
		{
			edge->p1_.swap(edge->p2_);
		}

		itr = edgeset_.find(edge);
		if (itr != edgeset_.end())
		{
			// reuse existing edge.
			edge = const_cast<TMesh::Edge*>(itr->get());
		}
		else
		{
			// put it back in.
			edgeset_.insert(edge);
		}
		return edge;
	}
	else
	{
		return edge;
	}
}

unsigned int TMesh::TestEdge(TMesh::Edge* edge)
{
	unsigned int numErrors = 0;
	for (TMesh::TriangleSet::iterator teitr = edge->triangles_.begin();
	teitr != edge->triangles_.end();
		++teitr)
	{
		TMesh::Triangle* triangle = const_cast<TMesh::Triangle*>(teitr->get());
		if (!(triangle->e1_ == edge || triangle->e2_ == edge || triangle->e3_ == edge))
		{
			OSG_NOTICE << "testEdge(" << edge << "). triangle != point back to this edge" << std::endl;
			OSG_NOTICE << "                     triangle->e1_==" << triangle->e1_.get() << std::endl;
			OSG_NOTICE << "                     triangle->e2_==" << triangle->e2_.get() << std::endl;
			OSG_NOTICE << "                     triangle->e3_==" << triangle->e3_.get() << std::endl;
			++numErrors;
		}
	}

	if (edge->triangles_.empty())
	{
		OSG_NOTICE << "testEdge(" << edge << ").triangles_ is empty" << std::endl;
		++numErrors;
	}
	return numErrors;
}

unsigned int TMesh::TestAllEdges()
{
	unsigned int numErrors = 0;
	for (TMesh::EdgeSet::iterator itr = edgeset_.begin();
	itr != edgeset_.end();
		++itr)
	{
		numErrors += TestEdge(const_cast<TMesh::Edge*>(itr->get()));
	}
	return numErrors;
}

unsigned int TMesh::ComputeNumBoundaryEdges()
{
	unsigned int numBoundaryEdges = 0;
	for (TMesh::EdgeSet::iterator itr = edgeset_.begin();
	itr != edgeset_.end();
		++itr)
	{
		if ((*itr)->IsBoundaryEdge()) ++numBoundaryEdges;
	}
	return numBoundaryEdges;
}

TMesh::Point* TMesh::AddPoint(TMesh::Triangle* triangle, unsigned int p1)
{
	return TMesh::AddPoint(triangle, originalpointlist_[p1].get());
}

TMesh::Point* TMesh::AddPoint(TMesh::Triangle* triangle, TMesh::Point* point)
{
	TMesh::PointSet::iterator itr = pointset_.find(point);
	if (itr == pointset_.end())
	{
		pointset_.insert(point);
	}
	else
	{
		point = const_cast<TMesh::Point*>(itr->get());
	}

	point->triangles_.insert(triangle);

	return point;
}

void TMesh::RemovePoint(TMesh::Triangle* triangle, TMesh::Point* point)
{
	TMesh::PointSet::iterator itr = pointset_.find(point);
	if (itr != pointset_.end())
	{
		point->triangles_.erase(triangle);

		if (point->triangles_.empty())
		{
			// point no longer in use, so need to delete.
			pointset_.erase(itr);
		}
	}
}

unsigned int TMesh::TestPoint(TMesh::Point* point)
{
	unsigned int numErrors = 0;

	for (TMesh::TriangleSet::iterator itr = point->triangles_.begin();
	itr != point->triangles_.end();
		++itr)
	{
		TMesh::Triangle* triangle = const_cast<TMesh::Triangle*>(itr->get());
		if (!(triangle->p1_ == point || triangle->p2_ == point || triangle->p3_ == point))
		{
			OSG_NOTICE << "testPoint(" << point << ") error, triangle " << triangle << " does not point back to this point" << std::endl;
			OSG_NOTICE << "             triangle->p1_ " << triangle->p1_.get() << std::endl;
			OSG_NOTICE << "             triangle->p2_ " << triangle->p2_.get() << std::endl;
			OSG_NOTICE << "             triangle->p3_ " << triangle->p3_.get() << std::endl;
			++numErrors;
		}
	}

	return numErrors;
}

unsigned int TMesh::TestAllPoints()
{
	unsigned int numErrors = 0;
	for (TMesh::PointSet::iterator itr = pointset_.begin();
	itr != pointset_.end();
		++itr)
	{
		numErrors += TestPoint(const_cast<TMesh::Point*>(itr->get()));
	}
	return numErrors;
}

int TMesh::SetBoundaryEdgeProtect()
{
	int n = 0;
	for (auto iter = edgeset_.begin();
	iter != edgeset_.end();
		iter++)
	{
		if ((*iter)->IsAdjacentToBoundary())
		{
			n++;
			(*iter)->isboundaryedge_ = true;
		}
	}

	return n;
}

int TMesh::BuildBoundyEdgeMap()
{
	TMesh::EdgeSet::iterator iter = edgeset_.begin();
	for (iter; iter != edgeset_.end(); ++iter)
	{
		if (!(*iter)->isboundaryedge_) continue;
		for (TMesh::EdgeSet::iterator iter_next = iter;
		iter_next != edgeset_.end();
			++iter_next)
		{
			if (iter_next == iter) continue;
			if (!(*iter_next)->isboundaryedge_) continue;
			osg::Vec3 v1 = (*iter)->p1_->vertex_ - (*iter_next)->p1_->vertex_;
			error_type l1 = v1.length2();

			if (l1 < FLT_MIN)
			{
				osg::Vec3 v2 = (*iter)->p2_->vertex_ - (*iter_next)->p2_->vertex_;
				error_type l2 = v2.length2();

				if (l2 < FLT_MIN)
				{
					boundyedgemap_.insert(std::make_pair((*iter), (*iter_next)));
				}

			}

		}

	}

	return boundyedgemap_.size();
}

int TMesh::RemoveSuspensionTriangle()
{
	int ntrisize = triangleset_.size();
	if (ntrisize == 0) return 0;

	TriangleList tempTriList;

	for (auto iter = triangleset_.begin();
	iter != triangleset_.end();
		iter++)
	{
		tempTriList.push_back((*iter));
	}

	int nlone = 0;
	for (auto iter = tempTriList.begin();
	iter != tempTriList.end();)
	{
		if ((*iter)->IsLoneTriangle())
		{
			iter = tempTriList.erase(iter);
			nlone++;
		}
		else
		{
			iter++;
		}
	}

	triangleset_.clear();
	for (auto iter = tempTriList.begin();
	iter != tempTriList.end();
		iter++)
	{
		triangleset_.insert((*iter));
	}

	return nlone;
}

void TMesh::DrawArraysType(GLenum mode, GLint first, GLsizei count)
{
	switch (mode)
	{
		case GL_TRIANGLES :
		{
			unsigned int pos = first;
			for (GLsizei i = 2; i < count; i += 3, pos += 3)
			{
				this->AddTriangle(pos, pos + 1, pos + 2);
			}
			break;
		}
		case GL_TRIANGLE_STRIP :
		{
			unsigned int pos = first;
			for (GLsizei i = 2; i < count; ++i, ++pos)
			{
				if ((i % 2)) this->AddTriangle(pos, pos + 2, pos + 1);
				else         this->AddTriangle(pos, pos + 1, pos + 2);
			}
			break;
		}
		case GL_TRIANGLE_FAN :
		{
			unsigned int pos = first + 1;
			for (GLsizei i = 2; i < count; ++i, ++pos)
			{
				this->AddTriangle(first, pos, pos + 1);
			}
			break;
		}
		default:
			// can't be converted into to triangles.
			break;
	}
}

void TMesh::DrawElementsUByteType(GLenum mode, GLsizei count, const GLubyte* indices)
{
	if (indices == 0 || count == 0) return;

	typedef GLubyte Index;
	typedef const Index* IndexPointer;

	switch (mode)
	{
		case(GL_TRIANGLES) :
		{
			IndexPointer ilast = &indices[count];
			for (IndexPointer iptr = indices; iptr < ilast; iptr += 3)
			{
				this->AddTriangle(*iptr, *(iptr + 1), *(iptr + 2));
			}

			break;
		}
		case(GL_TRIANGLE_STRIP) :
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				if ((i % 2)) this->AddTriangle(*(iptr), *(iptr + 2), *(iptr + 1));
				else       this->AddTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
			}
			break;
		}
		case(GL_TRIANGLE_FAN) :
		{
			IndexPointer iptr = indices;
			Index first = *iptr;
			++iptr;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				this->AddTriangle(first, *(iptr), *(iptr + 1));
			}
			break;
		}
		default:
			// can't be converted into to triangles.
			break;
	}
}

void TMesh::DrawElementsUShortType(GLenum mode, GLsizei count, const GLushort* indices)
{
	if (indices == 0 || count == 0) return;

	typedef GLushort Index;
	typedef const Index* IndexPointer;

	switch (mode)
	{
		case(GL_TRIANGLES) :
		{
			IndexPointer ilast = &indices[count];
			for (IndexPointer iptr = indices; iptr < ilast; iptr += 3)
				this->AddTriangle(*iptr, *(iptr + 1), *(iptr + 2));
			break;
		}
		case(GL_TRIANGLE_STRIP) :
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				if ((i % 2)) this->AddTriangle(*(iptr), *(iptr + 2), *(iptr + 1));
				else         this->AddTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
			}
			break;
		}
		case(GL_TRIANGLE_FAN) :
		{
			IndexPointer iptr = indices;
			Index first = *iptr;
			++iptr;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				this->AddTriangle(first, *(iptr), *(iptr + 1));
			}
			break;
		}
		default:
			// can't be converted into to triangles.
			break;
	}
}

void TMesh::DrawElementsUIntType(GLenum mode, GLsizei count, const GLuint* indices)
{
	if (indices == 0 || count == 0) return;

	typedef GLuint Index;
	typedef const Index* IndexPointer;

	switch (mode)
	{
		case(GL_TRIANGLES) :
		{
			IndexPointer ilast = &indices[count];
			for (IndexPointer iptr = indices; iptr < ilast; iptr += 3)
				this->AddTriangle(*iptr, *(iptr + 1), *(iptr + 2));
			break;
		}
		case(GL_TRIANGLE_STRIP) :
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				if ((i % 2)) this->AddTriangle(*(iptr), *(iptr + 2), *(iptr + 1));
				else         this->AddTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
			}
			break;
		}
		case(GL_TRIANGLE_FAN) :
		{
			IndexPointer iptr = indices;
			Index first = *iptr;
			++iptr;
			for (GLsizei i = 2; i < count; ++i, ++iptr)
			{
				this->AddTriangle(first, *(iptr), *(iptr + 1));
			}
			break;
		}
		default:
			// can't be converted into to triangles.
			break;
	}
}


void TMesh::CollectTriange()
{
	osg::Geometry::PrimitiveSetList primitiveSetList = geometry_->getPrimitiveSetList();
	//std::cout << "nsize = " << primitiveSetList.size() << std::endl;
	for (unsigned int i = 0; i < primitiveSetList.size(); i++)
	{
		osg::ref_ptr<osg::PrimitiveSet> test = primitiveSetList.at(i);
		osg::PrimitiveSet::Type test_type = test->getType();
		switch (test_type)
		{
			case osg::PrimitiveSet::DrawArraysPrimitiveType:
			{
				osg::DrawArrays* draw_arrays = dynamic_cast<osg::DrawArrays*>(test.get());
				GLint first_pos = draw_arrays->getFirst();
				GLsizei count_num = draw_arrays->getCount();
				GLenum draw_model = draw_arrays->getMode();

				this->DrawArraysType(draw_model, first_pos, count_num);

				////根据draw model解析
				//std::cout << "first_pos = " << first_pos << std::endl;
				//std::cout << "count_num = " << count_num << std::endl;
				//std::cout << "DrawArraysPrimitiveType" << std::endl;
				//this->addTriangle(first_pos, first_pos + 1, first_pos + 2);

				break;
			}
			case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
			{
				osg::DrawArrayLengths* draw_arrays = dynamic_cast<osg::DrawArrayLengths*>(test.get());
				GLint first_pos = draw_arrays->getFirst();
				GLenum draw_model = draw_arrays->getMode();
				for (osg::DrawArrayLengths::vector_type::const_iterator itr = draw_arrays->begin();
				itr != draw_arrays->end();
					++itr)
				{
					this->DrawArraysType(draw_model, first_pos, *itr);
					first_pos += *itr;
				}


				//std::cout << "DrawArrayLengthsPrimitiveType" << std::endl;
				break;
			}
			case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
			{
				osg::DrawElementsUByte* draw_element = dynamic_cast<osg::DrawElementsUByte*>(test.get());
				GLenum draw_model = draw_element->getMode();

				if (!draw_element->empty())
					this->DrawElementsUByteType(draw_model, draw_element->size(), &(draw_element->front()));



				//std::cout << "DrawElementsUBytePrimitiveType" << std::endl;

				break;
			}
			case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
			{
				osg::DrawElementsUShort* draw_element = dynamic_cast<osg::DrawElementsUShort*>(test.get());
				GLenum draw_model = draw_element->getMode();
				if (!draw_element->empty())
					this->DrawElementsUShortType(draw_model, draw_element->size(), &(draw_element->front()));


				//std::cout << "DrawElementsUShortPrimitiveType" << std::endl;
				break;
			}
			case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
			{
				osg::DrawElementsUInt* draw_element = dynamic_cast<osg::DrawElementsUInt*>(test.get());
				GLenum draw_model = draw_element->getMode();
				if (!draw_element->empty())
					this->DrawElementsUIntType(draw_model, draw_element->size(), &(draw_element->front()));


				//std::cout << "DrawElementsUIntPrimitiveType" << std::endl;
				break;
			}
			default:
				break;

		}

	}
}

/*-------------------------------------------------------------------------------------*/
osg::Geometry* TMeshConvUtil::ConvTriangleToGeode(TMesh::TriangleSet& triset)
{
	if (0 == triset.size()) return NULL;

	//osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertexs = new osg::Vec3Array();        /**< 顶点坐标数组   */
	osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array();       /**< 纹理坐标数组   */

	typedef std::set<osg::ref_ptr<TMesh::Triangle>, dereference_less>    TrianglesSorted;
	TrianglesSorted trianglesSorted;                                    /**< 三角形排序   */
	for (auto iter = triset.begin();
	iter != triset.end();
		iter++)
	{
		trianglesSorted.insert(*iter);
	}

	TMesh::PointSet PointsSorted;                            /**< 顶点排序,剔除重复点   */

	for (auto iter = trianglesSorted.begin();
	iter != trianglesSorted.end();
		iter++)
	{
		PointsSorted.insert((*iter)->p1_);
		PointsSorted.insert((*iter)->p2_);
		PointsSorted.insert((*iter)->p3_);
	}
	unsigned int pos = 0;
	for (auto iter = PointsSorted.begin();                   /**< 重新对顶点索引赋值   */
	iter != PointsSorted.end();
		iter++)
	{
		(*iter)->index_ = pos;
		vertexs->push_back((*iter)->vertex_);
		if ((*iter)->attributes_.size() >= 2)
		{
			texcoord->push_back(osg::Vec2((*iter)->attributes_[0], (*iter)->attributes_[1]));
		}

		pos++;
	}


	osg::ref_ptr<osg::DrawElementsUInt> primitives = new osg::DrawElementsUInt(GL_TRIANGLES, trianglesSorted.size() * 3);

	pos = 0;
	for (auto iter = trianglesSorted.begin();
	iter != trianglesSorted.end();
		iter++)
	{
		primitives->at(3 * pos + 0) = (*iter)->p1_->index_;
		primitives->at(3 * pos + 1) = (*iter)->p2_->index_;
		primitives->at(3 * pos + 2) = (*iter)->p3_->index_;
		pos++;
	}
	geometry->setVertexArray(vertexs.get());
	if (texcoord->size() == vertexs->size()) geometry->setTexCoordArray(0, texcoord.get());
	geometry->getPrimitiveSetList().clear();
	geometry->addPrimitiveSet(primitives.get());

	//osgUtil::SmoothingVisitor::smooth(*(geometry.get()));

	return geometry.release();
}

osg::Image* TMeshConvUtil::ConvCvMat2OsgImage(cv::Mat& mat)
{
	osg::Image *result = new osg::Image();
	int mat_cols = mat.cols;
	int mat_rows = mat.rows;

	result->setOrigin(osg::Image::TOP_LEFT);
	result->setImage(mat_cols, mat_rows, 1, GL_BGR, GL_BGR, GL_UNSIGNED_BYTE, mat.data, osg::Image::USE_NEW_DELETE);
	result->setWriteHint(osg::Image::EXTERNAL_FILE);
	if (result)
	{
		return result;
	}
	else
	{
		return NULL;
	}

}

cv::Mat TMeshConvUtil::ConvOsgImage2CvMat(osg::Image *image)
{
	int image_width = image->s();
	int image_height = image->t();
	osg::Image::Origin origin = image->getOrigin();
	GLenum image_pixformat = image->getPixelFormat();
	GLenum image_datatype = image->getDataType();
	cv::Mat result;
	result.create(image_height, image_width, CV_8UC3);

	//为MAT赋值
	switch (origin)
	{
		case osg::Image::BOTTOM_LEFT:
		{
			unsigned char *imgdata = image->data();
			for (int i = 0; i < image_height; i++)
			{
				for (int j = 0; j < image_width; j++)
				{
					result.at<cv::Vec3b>(image_height - i - 1, j)[0] = imgdata[(image_width * i + j) * 3 + 2];
					result.at<cv::Vec3b>(image_height - i - 1, j)[1] = imgdata[(image_width * i + j) * 3 + 1];
					result.at<cv::Vec3b>(image_height - i - 1, j)[2] = imgdata[(image_width * i + j) * 3];
				}
			}
		}
		break;
		case osg::Image::TOP_LEFT:
		{
			unsigned char *imgdata = image->data();
			for (int i = 0; i < image_height; i++)
			{
				for (int j = 0; j < image_width; j++)
				{
					result.at<cv::Vec3b>(i, j)[0] = imgdata[(image_width * i + j) * 3 + 2];
					result.at<cv::Vec3b>(i, j)[1] = imgdata[(image_width * i + j) * 3 + 1];
					result.at<cv::Vec3b>(i, j)[2] = imgdata[(image_width * i + j) * 3];
				}
			}
		}
		break;
	}

	return result;
}

void TMeshConvUtil::ConvTriangle2VcgMesh(TMesh::TriangleSet& triset, MyMesh& result)
{
	typedef std::set<osg::ref_ptr<TMesh::Triangle>, dereference_less>    TrianglesSorted;
	TrianglesSorted trianglesSorted;
	for (auto iter = triset.begin();
	iter != triset.end();
		iter++)
	{
		trianglesSorted.insert(*iter);
	}

	TMesh::PointSet PointsSorted;                            /**< 顶点排序,剔除重复点   */
	TMesh::PointList pointlist;
	for (auto iter = trianglesSorted.begin();
	iter != trianglesSorted.end();
		iter++)
	{
		PointsSorted.insert((*iter)->p1_);
		PointsSorted.insert((*iter)->p2_);
		PointsSorted.insert((*iter)->p3_);
		//pointlist.push_back((*iter)->p1_);
		//pointlist.push_back((*iter)->p2_);
		//pointlist.push_back((*iter)->p3_);

	}


	unsigned int pos = 0;
	for (auto iter = PointsSorted.begin();                     /**< 重新对顶点索引赋值   */
	iter != PointsSorted.end();
		iter++)
	{
		(*iter)->index_ = pos;
		pointlist.push_back(*iter);
		pos++;
	}

	int vsize = pointlist.size();
	MyMesh::VertexIterator vi = vcg::tri::Allocator<MyMesh>::AddVertices(result, vsize);

	for (int i = 0; i < vsize; i++)
	{
		MyVertex::CoordType& P((*vi).P());
		P[0] = pointlist[i]->vertex_.x();
		P[1] = pointlist[i]->vertex_.y();
		P[2] = pointlist[i]->vertex_.z();
		++vi;
	}

	vi = result.vert.begin();
	std::vector<MyMesh::VertexPointer> indices(result.vert.size());
	for (MyMesh::VertexPointer& idx : indices)
	{
		idx = &*vi;
		++vi;
	}


	MyMesh::FaceIterator fi = vcg::tri::Allocator<MyMesh>::AddFaces(result, trianglesSorted.size());
	for (auto iter = trianglesSorted.begin();
	iter != trianglesSorted.end();
		iter++)
	{
		//if (indices[3 * pos + 0] && indices[3 * pos + 1] && indices[3 * pos + 2])
		{
			//(*fi).V(0) = indices[3 * pos + 0];
			//(*fi).V(1) = indices[3 * pos + 1];
			//(*fi).V(2) = indices[3 * pos + 2];
			(*fi).V(0) = indices[(*iter)->p1_->index_];
			(*fi).V(1) = indices[(*iter)->p2_->index_];
			(*fi).V(2) = indices[(*iter)->p3_->index_];


			if ((*iter)->p1_->attributes_.size() >= 2 &&
				(*iter)->p2_->attributes_.size() >= 2 &&
				(*iter)->p3_->attributes_.size() >= 2)
			{
				(*fi).WT(0) = vcg::TexCoord2d((*iter)->p1_->attributes_[0], (*iter)->p1_->attributes_[1]);
				(*fi).WT(1) = vcg::TexCoord2d((*iter)->p2_->attributes_[0], (*iter)->p2_->attributes_[1]);
				(*fi).WT(2) = vcg::TexCoord2d((*iter)->p3_->attributes_[0], (*iter)->p3_->attributes_[1]);
			}
		}

		++fi;
		//++pos;
	}

}

void TMeshConvUtil::ConvVcgMesh2TriangleSet(MyMesh& vcgmesh, TMesh& result)
{
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertexs = new osg::Vec3Array();
	osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array();

	osg::DrawElementsUInt* primitives = new osg::DrawElementsUInt(GL_TRIANGLES, vcgmesh.FN() * 3);

	unsigned int pos = 0;
	for (MyMesh::FaceIterator fi = vcgmesh.face.begin();
	fi != vcgmesh.face.end();
		++fi)
	{
		if (fi->IsD()) continue;

		//MyMesh::FacePointer fp(&(*fi));
		//indices[fp->cV(0)];
		vertexs->push_back(osg::Vec3((*fi).V(0)->P().X(), (*fi).V(0)->P().Y(), (*fi).V(0)->P().Z()));
		vertexs->push_back(osg::Vec3((*fi).V(1)->P().X(), (*fi).V(1)->P().Y(), (*fi).V(1)->P().Z()));
		vertexs->push_back(osg::Vec3((*fi).V(2)->P().X(), (*fi).V(2)->P().Y(), (*fi).V(2)->P().Z()));

		if ((*fi).HasWedgeTexCoord)
		{
			texcoord->push_back(osg::Vec2((*fi).WT(0).U(), (*fi).WT(0).V()));
			texcoord->push_back(osg::Vec2((*fi).WT(1).U(), (*fi).WT(1).V()));
			texcoord->push_back(osg::Vec2((*fi).WT(2).U(), (*fi).WT(2).V()));
		}

		(*primitives)[3 * pos + 0] = 3 * pos + 0;
		(*primitives)[3 * pos + 1] = 3 * pos + 1;
		(*primitives)[3 * pos + 2] = 3 * pos + 2;
		//cout << "pos = " << pos << endl;
		pos++;
	}

	geometry->setVertexArray(vertexs.get());
	if (texcoord->size() > 0)
		geometry->setTexCoordArray(0, texcoord.get());
	geometry->addPrimitiveSet(primitives);

	//TMesh* osgmesh = new TMesh(true);
	result.SetGeometry(geometry.get());

	//result.insert(osgmesh->triangleset_.begin(), osgmesh->triangleset_.end());
	//return result;
}

/*-------------------------------------------------------------------------------------*/

cv::Rect2f TMeshCalcUtil::CalcCoordRect(std::vector<cv::Point2f> coords)
{
	cv::Rect2f result;
	int nsize = coords.size();
	if (nsize < 3) return result;
	float maxx = -FLT_MAX;
	float maxy = -FLT_MAX;
	float minx = FLT_MAX;
	float miny = FLT_MAX;
	for (int i = 0; i < nsize; i++)
	{
		if (coords[i].x > maxx) maxx = coords[i].x;
		if (coords[i].y > maxy) maxy = coords[i].y;
		if (coords[i].x < minx) minx = coords[i].x;
		if (coords[i].y < miny) miny = coords[i].y;
	}

	result.x = minx;
	result.y = miny;

	result.width = maxx - minx;
	result.height = maxy - miny;

	return result;
}

int TMeshCalcUtil::CalcLayerNum1(osg::Node* node, float gsd)
{
	if (!node)
		return -1;
	osg::BoundingSphere nodeBS = node->computeBound();

	float lg2 = log(2.0);
	float noderadius = nodeBS.radius();
	float tt = noderadius / float(PIXELNODE);
	//int nlevel = floor(tt);
	float nlevelf = log(tt) / lg2;
	int nlevel = floor(nlevelf);


	float invgsd = 1.0 / gsd;
	float lginvgsd = log(invgsd);
	float result = lginvgsd / lg2;
	int gsdlevel = int(result);

	return nlevel + gsdlevel + 1;
}

int TMeshCalcUtil::CalcLayerNum(osg::Node* node, float gsd)
{
	if (!node) 
		return -1;
	osg::BoundingSphere nodeBS = node->computeBound();

	float lg2 = log(2.0);
	float noderadius = nodeBS.radius();
	float tt = noderadius / float(PIXELNODE);
	//int nlevel = floor(tt);
	float nlevelf = log(tt) / lg2;
	int nlevel = floor(nlevelf);


	float invgsd = 1.0 / gsd;
	float lginvgsd = log(invgsd);
	float result = lginvgsd / lg2;
	int gsdlevel = int(result);

	return nlevel + gsdlevel + 1;
}


float TMeshCalcUtil::CalcGSDFromModel(osg::Node* node, unsigned int gsd_type /* = 0 */)
{
	if (!node) return -1.0f;

	CollectGeom collect_geom;
	node->accept(collect_geom);
	GeomArr geom_arr = collect_geom.GetGeomArr();

	unsigned int num_geom = geom_arr.size();
	std::vector<float> edge_lengths;
	std::vector<float> edge_pixel_lengths;
	std::cout << num_geom << std::endl;
	for (unsigned int i = 0; i < num_geom; i++)
	{
		osg::ref_ptr<osg::StateSet> stateset = geom_arr[i]->getStateSet();
		//if (!stateset.valid())  return -1.0f;
		if (!stateset.valid())  continue;
		
		osg::StateSet::TextureAttributeList& tex_att_list = stateset->getTextureAttributeList();
		//if (tex_att_list.size() == 0) return -1.0f;
		if (tex_att_list.size() == 0) continue;

		osg::ref_ptr<osg::Texture2D> tex2d =
			dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
		//if (!tex2d.valid()) return -1.0f;
		if (!tex2d.valid()) continue;;
		osg::ref_ptr<osg::Image> tex_image = tex2d->getImage();
		//if (!tex_image.valid()) return -1.0f;
		if (!tex_image.valid()) continue;;

		int image_width = tex_image->s();
		int image_height = tex_image->t();

		TMesh* pMesh = new TMesh(true);
		pMesh->SetGeometry(geom_arr[i].get());

		for (auto iter = pMesh->edgeset_.begin(); iter != pMesh->edgeset_.end(); iter++)
		{
			osg::Vec3f line = (*iter)->p1_->vertex_ - (*iter)->p2_->vertex_;
			float line_length = line.length();

			osg::Vec2f line_pixel = osg::Vec2f(
				((*iter)->p1_->attributes_[0] - (*iter)->p2_->attributes_[0]) * image_width,
				((*iter)->p1_->attributes_[1] - (*iter)->p2_->attributes_[1]) * image_height);
			float line_pixel_length = line_pixel.length();

			//std::cout << "line_pixel_length = " << line_pixel_length << std::endl;

			edge_lengths.push_back(line_length);
			edge_pixel_lengths.push_back(line_pixel_length);

		}

		if (pMesh)
		{
			delete pMesh;
			pMesh = NULL;
		}

	}

	unsigned int num_edge = edge_lengths.size();
	//std::cout << "num_edge = " << num_edge << std::endl;
	float avg = 0.0f;
	float max_gsd = -FLT_MAX;
	float min_gsd = FLT_MAX;
	unsigned int pos = 0;
	for (unsigned int i = 0; i < num_edge; i++)
	{
		if (fabs(edge_pixel_lengths[i]) > 0.0000001)
		{
			float temp_gsd = (edge_lengths[i] / edge_pixel_lengths[i]);
			avg += temp_gsd;
			if (temp_gsd > max_gsd) max_gsd = temp_gsd;
			if (temp_gsd < min_gsd) min_gsd = temp_gsd;
			pos++;
		}

	}

	float result = -1.0f;
	if (pos > 0)
	{
		if (0 == gsd_type)
			result = avg / float(pos);
		else if (1 == gsd_type)
			result = min_gsd;
		else if (2 == gsd_type)
			result = max_gsd;
	}
	//result = 0.05f;//LiMingci
	return result;
}


bool TMeshCalcUtil::IsPointInPolygon(osg::Vec3 point, std::vector<osg::Vec3> poly)
{
	int np = poly.size();

	double angle = 0.0;
	bool inside = false;

	//std::cout << "+++++++++"<< point[0] << "  " << point[1] << "  " << point[2] << std::endl;

	for (int i = 0; i < np - 1; i++)
	{
		if (fabs(poly[i].x() - point.x()) < 0.0000001 && fabs(poly[i].y() - point.y()) < 0.0000001)
		{
			inside = true;
			break;
		}

		double x1, y1, x2, y2;
		x1 = poly[i].x() - point.x();
		y1 = poly[i].y() - point.y();

		x2 = poly[(i + 1)].x() - point.x();
		y2 = poly[(i + 1)].y() - point.y();

		double radian = atan2(y1, x1) - atan2(y2, x2);

		if (radian > osg::PI)
		{
			radian = radian - 2 * osg::PI;
		}
		else if (radian < -osg::PI)
		{
			radian = radian + 2 * osg::PI;
		}

		angle += radian;

	}

	if (fabs(2 * osg::PI - fabs(angle)) < 0.0000001)
	{
		inside = true;
	}

	return inside;
}