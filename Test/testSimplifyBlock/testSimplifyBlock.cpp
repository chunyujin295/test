
#include "Core/ATData.h"
#include "Core/BlockObject.h"
#include "Core/CoordinateSystem.h"


using namespace AI3D::CORE;


//解决当时合并完之后空三过不去的问题
int main(int argc, char** argv)
{

	//判断两个文件的影像id是否一致，
	//map<int,int> IDMAPS; first为s3d的，second为mok的；
	//
	

	std::string xml_filepath_withgcp = argv[1];//gcptest/block_AT-TY2500.xml";// "D:/TestData/testwu/cc.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "C:/data/Projects/Newrrr/Block_2/Block_2.xml";// "D:/TestData/gcptest/AT1-geo.xml";// "D:/DOC/gongzuo/1AT.xml";
	BlockObject block_withgcp;
	auto atdata_withgcp = std::make_shared<ATData>();

	block_withgcp.LoadATXML(xml_filepath_withgcp, atdata_withgcp,false);
	
	block_withgcp.SetATData(atdata_withgcp);
	
		ATData::SimplifyOptions simopts;
		
	BlockObject::BlockExportOptions opt;
	opt.export_controlpoint_ = true;
	opt.export_not_registered_ = true;
	opt.export_tiepoint_ = true;
	//simopts.max_overlap_ = -1;
	//simopts.min_overlap_ = 2;
	simopts.max_proj_error_ = 3.0;
	simopts.max_tiepoint_count_ = 100000000;
	std::cout << block_withgcp.GetCurrentATMutual()->GetNumPoints3D() << " before " << std::endl;
	block_withgcp.GetCurrentATMutual()->Simplify(simopts);
	std::cout << block_withgcp.GetCurrentATMutual()->GetNumPoints3D() << "after" << std::endl;
	block_withgcp.ExportATXML(xml_filepath_withgcp+"_w.xml", opt);

	
	
	return 1;
}

