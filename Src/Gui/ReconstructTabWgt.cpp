#include "Gui/BlockWgt.h"
#include <algorithm>

#include <QVariant>
#include <QDateTime>
#include <QtConcurrent>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QStringList>
#include <QHostInfo>
#include "Util/TaskProcess.h"
#include <QThreadPool>
#include "Gui/MohackerWin.h"
#include "Core/CoordinateSystem.h"
#include "Gui/BlockManager.h"
#include "Gui/ImportGcpDia.h"
#include "Gui/PosSigmaDia.h"
#include "Core/ControlPoint.h"
#include "Core/CoordinateSystem.h"
#include "Gui/ProjectManager.h"
#include "Core/Timer.h"
#include "Gui/ImportPosDia.h"
//#include "Gui/Network.h"
#include "Core/Types.h"
#include "Core/TaskDef.h"
//#include "Core/TileObject.h"
#include"Gui/AddSigGcp.h"
#include "Util/Statistic.h"
#include "Util/Settings.h"
#include "Util/TaskProcess.h"
//#include "Gui/OTA.h"
#include "Util/JobMonitor.h"
#include "Core/ReconstructionCommandSet.h"
#include "Util/Software.h"

//?chy InitGcpData
using namespace AI3D::CORE;

namespace AI3D
{
	namespace GUI
	{
		

		//blk文件增加
		//注意一旦提交则空三的数据只能查看不能编辑；
		//注意tippoint这个时候需要注意load与否
		//此处注意一定要有坐标偏移而且偏移首先以GCP为准
		void BlockWgt::Slot_Btn_SubmitReconstruct_Clicked()
		{
			VersionInfo versionInfo = checkSoftWareVersion();
			QString message;
			if (!versionInfo.checkReturn) {
				//请求出错
				message = "网络问题，请联网后再试";
				QMessageBox errBox;
				errBox.warning(this, "软件版本检测", message);
				return;
			}
			else if (!versionInfo.isValid) {
				//当前版本不合法
				message = "当前软件不可用，请联系软件开发商更新版本";
				QMessageBox errBox;
				errBox.warning(this, "软件版本检测", message);
				return;
			}
			LOGI( "======================Submit Reconstruction ,waiting=================" );
			std::cout << "======================Submit Reconstruction ,waiting==================" << std::endl;
			struct processing_settings_s settings;
			reconstruction_t newRecounstructionId = kInvalidReconstructionId;
			auto savefunc = [&, this]()
			{
				return ReconstructionCommandSet::SubmitReconstruction(block_data_, newRecounstructionId, settings);
			};

			int ret = AI3D_FAILURE;
			if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
			{
				if (BlockObject::isChineseVersion())
				{
					OpenLoadingPromptV4("已提交重建，请耐心等待");
				}
				else
				{
					OpenLoadingPromptV4("Please be patient and wait.New reconstruction");
				}
				QFuture<int> f1 = QtConcurrent::run(savefunc);
				QEventLoop waitLoop;
				QFutureWatcher<int> watcher;
				QObject::connect(&watcher, &QFutureWatcher<int>::finished, &waitLoop, &QEventLoop::quit);
				watcher.setFuture(f1);
				waitLoop.exec();
				ret = f1.result();
				
			}
			else
			{
				ret = savefunc();
				

			}
			
			if (ret == AI3D_SUCCESS)
			{
				emit Sig_NewConstruction(block_data_, newRecounstructionId);
				std::cout << "======================Submit Reconstruction ,end==================" << std::endl;
				LOGI("======================Submit Reconstruction end.=================");
			}
			else
			{
				LOGI("======================Submit Reconstruction failed and end.=================");
				std::cout << "======================Submit Reconstruction ,failed and end==================" << std::endl;
			}
			
			if (AI3D::CORE::Application::Getinstance().GetReleaseLevel() <= 2)
			{
				CloseLoadingPromptV4();
			}
			
			return;
			
			
		}
		

	}
}
