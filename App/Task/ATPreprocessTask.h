#ifndef _AI3D_ATPREPROCESSTASK_H_
#define _AI3D_ATPREPROCESSTASK_H_
#include <string>
#include "Constants.h"
#include <Reconstruction/Reconstruct.h>




extern "C" int AI3D_API RunBatchPrePare(const std::string& json_str, const ReconstructCallBack& call_back);


#endif