#pragma once
#include <string>
#include <functional>
#include "Config.h"

struct ReconstructCallBack
{
	ReconstructCallBack(
		const std::function<void(int ret_code)> cb_finish,
		const std::function<void(float cur)> cb_progress,
		const std::function<void(const std::string& message)> cb_message,bool* bStop = nullptr) :
		cb_finish_(cb_finish), cb_progress_(cb_progress), cb_message_(cb_message),should_stop_(bStop)
	{
	
	}

	bool* should_stop_ = nullptr;
	std::function<void(int ret_code)> cb_finish_;
	std::function<void(float cur)> cb_progress_;
	std::function<void(const std::string& message)> cb_message_;
};

extern "C" S3D_RECONSTRUCTION_API const char* RunGetEngineVersion();
extern "C" int S3D_RECONSTRUCTION_API DivideChunks(std::string working_dir,std::string mvs_path, const ReconstructCallBack & call_back,float ram = -1);
//extern "C" int S3D_RECONSTRUCTION_API ExtractBlock(std::string working_dir);

extern "C" int S3D_RECONSTRUCTION_API RunReconstruction(const std::string & json_str, const ReconstructCallBack & call_back);

extern "C" int S3D_RECONSTRUCTION_API Reconstruct3D(const std::string & json_str, const ReconstructCallBack & call_back);

extern "C" int S3D_RECONSTRUCTION_API RunFeatureDetection(const std::string& json_str, const ReconstructCallBack& call_back);

extern "C" int S3D_RECONSTRUCTION_API  RunSfM(const std::string& json_str, const ReconstructCallBack& call_back);
//传入xml 和 gcp.json
//extern "C" int S3D_RECONSTRUCTION_API RunOptimizeAT(const std::string& json_str, const ReconstructCallBack& call_back);
extern "C" int S3D_RECONSTRUCTION_API  RunGenTasks(const std::string&str, const ReconstructCallBack&cb);

extern "C" int S3D_RECONSTRUCTION_API RunMatchPairs(const std::string& json_str, const ReconstructCallBack& call_back);

extern "C" int S3D_RECONSTRUCTION_API RunPairSelection(const std::string& json_str, const ReconstructCallBack& call_back);

extern "C" int S3D_RECONSTRUCTION_API RunCancel(const std::string& json_str, const ReconstructCallBack& call_back);