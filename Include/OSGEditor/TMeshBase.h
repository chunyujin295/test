
#include <vcg/complex/complex.h>

#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export.h>
#include <vcg/complex/algorithms/local_optimization.h>
#include <vcg/complex/algorithms/local_optimization/tri_edge_collapse_quadric.h>
#include <wrap/io_trimesh/import_obj.h>
#include <wrap/gl/glu_tessellator_cap.h>
#include <vcg/complex/algorithms/intersection.h>

#include <wrap/io_trimesh/import.h>
#include <wrap/io_trimesh/export.h>

#ifndef T_MESHBASE_H
#define T_MESHBASE_H

#define EZVS_NAMESPACE_MESH_BEGIN namespace MESH {
#define EZVS_NAMESPACE_MESH_END }

class MyVertex; class MyEdge; class MyFace; class MyEdgeMesh;
struct MyUsedTypes : public vcg::UsedTypes<
	vcg::Use<MyVertex>::AsVertexType,
	vcg::Use<MyEdge>  ::AsEdgeType,
	vcg::Use<MyFace>  ::AsFaceType   > {};

class MyVertex : public vcg::Vertex<MyUsedTypes, vcg::vertex::Coord3d, vcg::vertex::Normal3d, vcg::vertex::VFAdj, vcg::vertex::VEAdj, vcg::vertex::Qualityd, vcg::vertex::Mark, vcg::vertex::BitFlags, vcg::vertex::Color4b> {};
class MyFace : public vcg::Face<  MyUsedTypes, vcg::face::VertexRef, vcg::face::Normal3d, vcg::face::FFAdj, vcg::face::VFAdj, vcg::face::WedgeTexCoord2d, vcg::face::Mark, vcg::face::BitFlags, vcg::face::Color4b> {};
class MyEdge : public vcg::Edge<  MyUsedTypes, vcg::edge::VertexRef, vcg::edge::BitFlags, vcg::edge::VEAdj, vcg::edge::EEAdj> {};
class MyMesh : public vcg::tri::TriMesh< std::vector<MyVertex>, std::vector<MyFace>, std::vector<MyEdge> > {};
class MyEdgeMesh : public vcg::tri::TriMesh <std::vector<MyVertex>, std::vector<MyEdge>> {};




#endif T_MESHBASE_H
