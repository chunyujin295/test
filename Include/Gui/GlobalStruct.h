/**
  * @file      Application.h
  * @brief    全局变量及结构体类
  * @details
  * @author    
  * @attention
  */
#ifndef GLOBAL_STUCT_H
#define GLOBAL_STUCT_H

#include <QMetaType>
#include <QPointF>
#include <QString>
#include <Core/BlockObject.h>
#include <Core/ProductionObject.h>
#include <Core/ReconstructionObject.h>

#pragma execution_character_set("utf-8")		// 支持中文编码
namespace AI3D
{
	namespace GUI
	{
		//#define USE_WIN_DEBUG
		//#define SAVE_CONTROL_POINT_STATUS
	
	
		const std::string VERSION = AI3D::CORE::Application::Getinstance().ParseConfig().version;
		
		const int UPDATATIME(2000);
		const std::string RecentOpenFile("recentopen.xml");
		const int RECENTMOST(10);

		const std::string OPENENGINE("Open Engine ");
		const std::string OPENPROJECT("Open Project ");
		const std::string OPENRECENTPROJECT("Open Recent Project ");
		const std::string NEWPROJECT("New Project ");
		const std::string SAVEPROJECT("Save Project ");
		const std::string NEWBLOCK("New Block ");
		const std::string IMPORTBLOCK("Import Block ");
		const std::string EXPORTBLOCK("Export Block ");
		const std::string RENAMEPROJECT("Rename Project ");
		const std::string RENAMEBLOCK("Rename Block ");
		const std::string DELETEBLOCK("Delete Block ");
		const std::string OPENFOLDER("Open Folder ");

		class ConstructionWgt;
		class ProductionWgt;

		/*enum EErrorCodes
		{
			NoError = 0,
			EmptyFile = -1,
			OpenFileFailed = -2,
			SaveFileFailed = -3,
			ParseFailed = -4
		};*/
		enum ItemType
		{
			ITProject = 0,
			ITBlock,
			ITBlockAT,
			ITReconstruction,
			ITProduction
		};
		Q_DECLARE_METATYPE(ItemType)
		enum CustomRole
		{
			CRBlockIndex = Qt::UserRole + 1,
			CRBlockWgt,
			CRBlockATIndex,
			CRBlockData,
			CRProjectIndex,
			CRProjectWgt,
			CRProjectData,
			CRBlockATData,
			CRItemType,
			CRPhotGroupData,
			CRItemBoundWgt,
			CRReconstructionWgt,
			CRReconstructionData,
			CRProductionData,
			CRProductionTabWgt,

			CRControlpointsImage,
			CRControlpointsTableModelsele,
			CRControlpointsTableModelImageList,
			CRControlpointsDot,
			CRControlpointsID,
			CRControlpointsImageID,
			CanSaveBlock,
			CRIcon,
			CRPhotoGroupID,
			CRImageID,
			CRReconstructionID,
			CRProductionID,
			CRParentBlockData,
		};
		enum ShowGroup
		{
			NoVisible = 0,
			PhotoGroup,
			PhotoDetail
			
		};

		struct Block_Status_s
		{
			bool can_AT = false;
			bool can_resubAT = false;
			bool can_cancle = false;
			bool can_add_pos = true;
			bool can_del_pos = false;
			bool can_add_photo = true;
			bool can_del_photo = false;
			bool can_add_gcp = false;
			bool can_del_gcp = false;
			bool can_submit_rec = false;
			

			int tabbarstate = 0x0001;//默认显示3dview   0x00013dview 0x0010GCP页卡   0x0100Photo页卡  0x1000AT页卡 可组合
			ShowGroup type = ShowGroup::PhotoGroup;
		};
		Q_DECLARE_METATYPE(CustomRole)
		Q_DECLARE_METATYPE(AI3D::CORE::BlockObject*);
		Q_DECLARE_METATYPE(AI3D::CORE::ProductionObject*);
		Q_DECLARE_METATYPE(AI3D::CORE::ReconstructionObject*);
		Q_DECLARE_METATYPE(AI3D::CORE::PhotoGroup);
		Q_DECLARE_METATYPE(AI3D::CORE::Image);
		Q_DECLARE_METATYPE(QPointF);
		//Q_DECLARE_METATYPE(AI3D::GUI::ConstructionWgt);
		//Q_DECLARE_METATYPE(AI3D::GUI::ProductionWgt);

		const  std::string ATTAB = "AT";
		const  std::string PHOTOTAB = "Photos";
		const  std::string GCPTAB = "GCP";
		const  std::string VIEWTAB = "3D View";

		/*const int PENDING = 0;
		const int RUNNING = 1;
		const int COMPLETE = 2;
		const int CANCLE = 3;
		const int FAILED = 4;*/

		const std::string NOTGEOREFERENCED = "Not Georeferenced";
		const std::string  WGS84SRS = "WGS 84";
		#define TABSTRING " "
		//
		struct JobParam
		{
			std::string taskName;
			std::string jobName;
			std::string job_no_httpserver_name;
			std::string operation;
			std::string sourcelistPath;
			std::string createTime;
			std::string ipPort;
		};
		Q_DECLARE_METATYPE(JobParam)
		Q_DECLARE_METATYPE(JobParam*)

			
		
		 inline bool IsPointOrNumber(QString qstrSrc)
		 {
			QByteArray ba = qstrSrc.toLatin1();
			const char* s = ba.data();
			bool bret = true;
			int pointnum = 0;
			while (*s)
			{
				if (*s == '.' || (*s >= '0' && *s <= '9'))
				{
					if (*s == '.')
					{
						pointnum += 1;
						if (pointnum >= 2)//出现两个小数点返回false
						{
							bret = false;
						}
					}
				}
				else
				{
					bret = false;
					break;
				}
				s++;
			}
			return bret;
		 }
	}
}




#endif  // _AI3D_CORE_APPLICATION_H_
