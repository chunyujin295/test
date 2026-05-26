#include "ATPreprocessTask.h"
#include "Core/BlockObject.h"
#include "Core/ReturnCode.h"

#include "Core/TaskDef.h"
#include <QDebug>

int RunBatchPrePare(const std::string& json_str, const ReconstructCallBack& call_back)
{
	if (CheckUsingNoChinesePathVersion())
	{
		try
		{
			
			ATTaskInfo task_tile(nlohmann::json::parse(json_str.begin(), json_str.end()));

			std::string blockpath = AI3D::CORE::File::GetParentDir(task_tile.projectFile_) + "/" + task_tile.blockItem_ + "/";
			AI3D::CORE::BlockObject block(blockpath);
			std::string blkfile = "";
			if (BLK_USE_BIN) {
				blkfile = blockpath + task_tile.blockItem_ + BLOCKBINFILE;
				
			}
			else {
				blkfile = blockpath + task_tile.blockItem_ + BLOCKFILE;
				
			}
			
			block.Load(blkfile, false);


			int* progress = new int(0);
			std::thread batchThread(std::bind(&AI3D::CORE::BlockObject::BatchPrePare, &block, task_tile.ATJson_, std::ref(progress)));
			batchThread.detach();
			int lastValue = 0;
			while (true)
			{
				call_back.cb_progress_((float)*progress);
				if (*progress == 100)
				{
					call_back.cb_progress_((float)*progress);
					break;
				}
			}
		}
		catch (std::exception& ex)
		{
			std::ostringstream oss;
			oss << " exception:" << ex.what();
			
			qDebug() << QString::fromStdString(oss.str());
		}
	}
	else
	{
		
		ATTaskInfo task_tile(json_str,0);

		std::string blockpath = AI3D::CORE::File::GetParentDir(task_tile.projectFile_) + "/" + task_tile.blockItem_ + "/";
		AI3D::CORE::BlockObject block(blockpath);
		std::string blkfile = "";
		if (BLK_USE_BIN) {
			blkfile = blockpath + task_tile.blockItem_ + BLOCKBINFILE;
			
		}
		else {
			blkfile = blockpath + task_tile.blockItem_ + BLOCKFILE;
			
		}
		
		block.Load(blkfile, false);

		int* progress = new int(0);
		std::thread batchThread(std::bind(&AI3D::CORE::BlockObject::BatchPrePare, &block, task_tile.ATJson_, std::ref(progress)));
		batchThread.detach();
		int lastValue = 0;
		while (true)
		{
			call_back.cb_progress_((float)*progress);
			if (*progress == 100)
			{
				call_back.cb_progress_((float)*progress);
				break;
			}
		}
	}

	std::cout << " " << 0 << std::endl;
	return AI3D_SUCCESS;
}


