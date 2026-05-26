#include "Gui/BlockWgt.h"
#include <algorithm>

#include <QVariant>
#include <QDateTime>
#include <QtConcurrent>
#include <QStringList>
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
#include "Gui/message_box.h"
#include "Core/Types.h"
#include"Core/BlockObject.h"
#include"Gui/AddSigGcp.h"
#include "Util/Statistic.h"
#include "Util/Settings.h"
#include "Core/File.h"
#include <filesystem>
#include "Util/TaskProcess.h"
//#include "Gui/OTA.h"
#ifdef USE_AI3D_PROJ
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Core/Proj/ProjCore.h"
#endif
//?chy InitGcpData
using namespace AI3D::CORE;

namespace AI3D
{
	namespace GUI
	{
	


		void BlockWgt::InitGcpTabConnections()
		{
		
		//	connect(ui->btn_addgcp_measurements, &QPushButton::clicked, this, &BlockWgt::Slot_Action_ImportMeasurementFromXml, Qt::QueuedConnection);
			//connect(ui->btn_exportgcpmeasurements, &QPushButton::clicked, this, &BlockWgt::Slot_Action_ExportMeasurementToXml, Qt::QueuedConnection);

			connect(ui->btn_Siggcp, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_AddSigGcp_Clicked, Qt::QueuedConnection);
			connect(ui->btn_addgcp, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_AddGcp_Clicked, Qt::QueuedConnection);
			connect(ui->btn_delgcp, &QPushButton::clicked, this, &BlockWgt::Slot_Btn_DelGcp_Clicked, Qt::QueuedConnection);

		}

		void BlockWgt::Slot_Action_ExportMeasurementToXml()
		{

		}

		void BlockWgt::Slot_Action_ImportMeasurementFromXml()
		{
			QFileDialog* pfd = nullptr;
			if (BlockObject::isChineseVersion())
			{
				pfd = new QFileDialog(nullptr, tr("打开刺点文件"), ".", tr("文件类型(*.xml)"));
			}
			else
			{
				pfd = new QFileDialog(nullptr, tr("Open Surveys file"), ".", tr("file(*.xml)"));
			}

			pfd->setAcceptMode(QFileDialog::AcceptOpen);
			pfd->setFileMode(QFileDialog::ExistingFile);
			pfd->setViewMode(QFileDialog::Detail);
			//pfd->setDirectory(QDir(QString(promanager->GetProject()->GetPath().c_str())).absolutePath());
			if (QDialog::Accepted != pfd->exec())
			{
				delete pfd;
				return;
			}
			pfd->close();
			//  emit CloseFirstWgt();

			QString filename = pfd->selectedFiles().first();
			delete pfd;

			try
			{
				///if (!boost::filesystem::is_regular_file(projName.toStdString()))
				if (!std::filesystem::is_regular_file(AI3D::CORE::File::BoostPathFromUtf8(qstr2str(filename))))
				{
					QString dlgTitle = tr("Error");
					QString strInfo = tr("file not exist,please select exist file!");
					///std::cout << "loadproj0:" << qstr2str(projName) << std::endl;
					Message_Box::critical(this, dlgTitle, strInfo);
					return ;
				}
			}
			catch (const std::filesystem::filesystem_error& fse)
			{
				std::ostringstream oss;
				oss << "fse error:" << fse.code() << " " << fse.what() << " " << fse.path1().string() << " " << fse.path2().string();
				LOGI(oss.str());
				return ;
			}
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return ;
			}

			EIGEN_STL_UMAP(srsid_t, srs_s) srs_map;
			EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) cps_map;
			EIGEN_STL_UMAP(image_t, std::string) image_map;
			
			AI3D::CORE::BlockObject::LoadGCPMeasurementsXML1(qstr2str(filename), srs_map, cps_map, image_map);

			auto atdata = block_data_->GetCurrentATMutual();
			std::map<std::string, std::pair<image_t, image_t>> name_ids;
			//导入文件的影像名称 导入的id 
			for (auto& iter : image_map)
			{
				std::string name = File::GetFileNameWithoutExtension(iter.second);
				AI3D::CORE::String::StringToLower(&name);
				name_ids[name].first = iter.first;
				name_ids[name].second = -1;
			}
			//按影像名检测，如果block中有则赋值ID，也就是说map中都有id的也就是block中有的
			for (auto& iter : atdata->GetImages())
			{
				auto name1 = File::GetFileNameWithoutExtension(iter.second.GetName());// iter.second.GetPath() + "/" + iter.second.GetName();;// 
				// name1 = AI3D::CORE::File::EnsureUnifySlash(name1);
				 AI3D::CORE::String::StringToLower(&name1);

				if (name_ids.count(name1))
				{
					name_ids.at(name1).second = iter.first;
					//newimage_map[iter.first] = iter.second.GetPath() + "/" + iter.second.GetName();
				}


			}
			//影像id统一到atdata下
			for (auto& iter : cps_map)
			{
				/*if (iter.second.GetName() == "07350P3-01-002-017")
				{
					std::cout << iter.second.GetId() << std::endl;
				}*/
				std::vector<AI3D::CORE::TrackElement>& elevec = iter.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual();
				for (std::vector<AI3D::CORE::TrackElement>::iterator it = elevec.begin();
					it != elevec.end();)				
				{
					auto& image_id = (*it).image_id;

					std::string imgname = image_map.at(image_id);

					imgname =File::GetFileNameWithoutExtension(imgname);
					AI3D::CORE::String::StringToLower(&imgname);
					if (name_ids.count(imgname)&& name_ids.at(imgname).first != -1 && name_ids.at(imgname).second != -1)
					{
						
						image_id = name_ids.at(imgname).second;
							
							it++;
					}
					else
					{
						it = elevec.erase(it);
					}
				}


			}

			AI3D::CORE::ControlPoints gcps_import;
			gcps_import.SetPoints(cps_map);
			if (cps_map.empty())
				return;
			srs_s basesrs = cps_map.begin()->second.GetSrsMutual();
			
			ImportGCP(gcps_import, basesrs, image_map);
		}
		
		//
		void BlockWgt::ImportGCP(AI3D::CORE::ControlPoints& gcps_import, srs_s& srs,
			EIGEN_STL_UMAP(image_t, std::string)& image_map)
		{
			if (gcps_import.GetGCPCount() <= 0)
				return;
			bool iscover = false;
			//导入的坐标系与之前导入的相同 累加控制点值
			/*if (block_data_->GetCurrentATMutual()->HasControlPoints())
			{
				if (impGcpDialog.GetSrsName().toStdString() != block_data_->GetCurrentATMutual()->GetControlPointsMutual().begin()->second.GetSrsMutual().name)
				{
					block_data_->GetCurrentATMutual()->DeleteGCPs();
					iscover = true;
				}
			}*/
				gcps_import.SetSRS(srs.definition);
				block_data_->UpdateSRSMap(srs);
				srs.ID = block_data_->ExistSRS(srs.definition);
				auto id = block_data_->GetCurrentATMutual()->GenerateValidGCPId();
				EIGEN_STL_UMAP(point3D_t, ControlPoint) gcp_pointstemp;
				for (auto& gcp_it : gcps_import.GetPointsMutual())
				{
					/*if (gcp_it.second.GetName() == "07350P3-01-002-017")
					{
						std::cout << gcp_it.second.GetId() << std::endl;
					}*/
					gcp_it.second.SetId(id);	
					gcp_it.second.GetObjectPointMutual().SetId(id);
					for (auto& ele : gcp_it.second.GetObjectPointMutual().GetTrackMutual().GetElementsMutual())
					{

						AI3D::CORE::Image& img = block_data_->GetCurrentATMutual()->GetImageMutual(ele.image_id);
						
						img.SetPoints2DGCP(id, ele.xy);
						ele.point2D_idx =(point2D_t)id;
					}
					gcp_pointstemp[id] = gcp_it.second;
					id++;
				}
				gcps_import.GetPointsMutual() = gcp_pointstemp;
				//chy add 此处将来需要统一将其他转为GCP坐标系下
				std::string definition = block_data_->GetCurrentATMutual()->GetLocalSrs();
				if (definition == "" || definition == LOCALSRS)
				{
					definition = LOCALSRS;
				}

				/*AI3D::CORE::ControlPoints gcps_import_temp;
				gcps_import_temp.SetSRS(srs.definition);
				for (const auto& iter : gcps_import.GetPointsMutual())
				{
					gcps_import_temp[iter.second.GetId()] = iter.second;
				}*/
				//add by chy 2024 10 28 此处需考虑既有LOCAL又有非local
				gcps_import.TransformPointsToBaseCoordinate(definition);


				//如果有控制点导入，则根据目前是否有控制点再决定GCP页卡的状态；同时需要转换坐标系统
				auto& gcps_recent = block_data_->GetCurrentATMutual()->GetControlPointsMutual();
				for (auto& gcp_it : gcps_import.GetPointsMutual())
				{
					gcps_recent[gcp_it.first] = gcp_it.second;
				}

				if (!block_data_->GetCurrentATMutual()->HasControlPoints())
				{
					return;
				}
				block_data_->GetCurrentATMutual()->SetLocalGcpSrs(gcps_recent.cbegin()->second.GetSrs().definition);
				

				ControlPointsEditorWin* controlPoints_ui = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
				controlPoints_ui->InitGcpData();
				
				
				controlPoints_ui_ = controlPoints_ui;
				connect(controlPoints_ui_, &ControlPointsEditorWin::Sig_ModifiedTrue, this, &BlockWgt::SetModifityXml);
				//更新页卡
				ProjectManager* project = ProjectManager::GetInstance();
				project->AddBlockManager(block_data_);
				std::vector<int> tabvec;
				bool ATFlag = false;
				auto blockstatus = block_data_->GetStatus();
				if (blockstatus == jobsta_e::STATUS_NEW || blockstatus == jobsta_e::STATUS_UNKNOWN)
				{
					ATFlag = false;
				}
				else
				{
					ATFlag = true;
				}
				myTabWidget_[GCPTAB.c_str()] = controlPoints_ui;
				//ExistsTab(ATTAB)
				project->GetBlockManaget(block_data_->GetId())->ChangeTab(ATFlag, block_data_->GetCurrentAT()->HasImages(), \
					block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
				UpdateTabPaper(tabvec);
				if (ui->tabWidget->tabText(0).toStdString() == GCPTAB)
				{
					ui->tabWidget->removeTab(0);
				}
				SetIndexByStr(GCPTAB.c_str());

				BlockManager* blockmanager = project->GetBlockManaget(block_data_->GetId());
				auto blockwgtstatus = blockmanager->GetBlockStatusMutual();

				//blockstatus.can_del_pos = false;
				SetWgtStatus(blockwgtstatus);
				SetModifityXml();
				Update3DView();
			
		}

		
		
		void BlockWgt::InitGcpControlPointWgt()
		{
			if (block_data_->GetCurrentATMutual()->HasControlPoints())
			{
				controlPoints_ui_ = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
				auto srs = controlPoints_ui_->GetCurrentSrs();

			}
			//?chy @zhaokang 此处可以不要了吧
			else
			{
				controlPoints_ui_ = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
				//ui->tabWidget->insertTab(3, (QWidget*)controlPoints_ui_, "GCP");

			}
			connect(controlPoints_ui_, &ControlPointsEditorWin::Sig_ModifiedTrue, this, &BlockWgt::SetModifityXml);

		}

		
		
		void BlockWgt::InsertGCPTab()
		{
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			std::cout << myTabWidget_.size() << std::endl;;

///			if (myTabWidget_.contains(GCPTAB.c_str()))
///			{
///				return;
///			}

			for (int i = 0; i < ui->tabWidget->count(); i++)
			{
				QString title = ui->tabWidget->tabText(i);
				if (title == QString::fromStdString(GCPTAB))
				{
					ui->tabWidget->setCurrentWidget(myTabWidget_[GCPTAB.c_str()]);
					return;
				}
			}

			/// note: insert gcp tab just before 3dview if possible.
			if (ui->tabWidget->count() == 3)
				ui->tabWidget->insertTab(2, myTabWidget_[GCPTAB.c_str()], GCPTAB.c_str());
			else
				ui->tabWidget->addTab(myTabWidget_[GCPTAB.c_str()], GCPTAB.c_str());

			ui->tabWidget->setCurrentWidget(myTabWidget_[GCPTAB.c_str()]);

			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			//ControlPointsEditorWin* controlPoints_ui = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
			//controlPoints_ui->InitSurveyData();			

			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

			////controlPoints_ui->TableView_Load_GcpData(QString::fromLocal8Bit(srs.name.c_str()));
			//controlPoints_ui_ = controlPoints_ui;
			//connect(controlPoints_ui_, &ControlPointsEditorWin::Sig_ModifiedTrue, this, &BlockWgt::SetModifityXml);
			////更新页卡
			//ProjectManager* project = ProjectManager::GetInstance();
			//
			//std::vector<int> tabvec;
			//bool ATFlag = false;
			//auto blockstatus = block_data_->GetStatus();
			//if (blockstatus == jobsta_e::STATUS_NEW || blockstatus == jobsta_e::STATUS_UNKNOWN)
			//{
			//	ATFlag = false;
			//}
			//else
			//{
			//	ATFlag = true;
			//}
			//
			//myTabWidget_[GCPTAB.c_str()] = controlPoints_ui;

			//ExistsTab(ATTAB)
///			bool hassurveydata = block_data_->GetCurrentAT()->HasSurveyPoints();
///			project->GetBlockManaget(block_data_->GetId())->ChangeTab(ATFlag, block_data_->GetCurrentAT()->HasImages(), \
///				hassurveydata, tabvec);

///			std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
///			ui->tabWidget->addTab(myTabWidget_[GCPTAB.c_str()], GCPTAB.c_str());
///			std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
/// 
			//ui->tabWidget->insertTab()

///			UpdateTabPaper(tabvec);
			
			/// ???
///			if (ui->tabWidget->tabText(0).toStdString() == GCPTAB)
///			{
///				ui->tabWidget->removeTab(0);
///			}

///			SetIndexByStr(GCPTAB.c_str());

///			BlockManager* blockmanager = project->GetBlockManaget(block_data_->GetId());
///			auto blockwgtstatus = blockmanager->GetBlockStatusMutual();

			//blockstatus.can_del_pos = false;
///			SetWgtStatus(blockwgtstatus);
///			SetModifityXml();			
///			Update3DView();
		}
		
		void BlockWgt::Slot_Btn_AddSigGcp_Clicked()
		{

			//获取当前的tabwidget是否有gcp页卡
			QTabBar* tabbar = ui->tabWidget->tabBar();
			int num = tabbar->count();
			bool hasgcpbar = false;

			///if (!block_data_->GetCurrentATMutual()->HasControlPoints())
			if (!block_data_->GetCurrentATMutual()->HasSurveyPoints())
			{
				hasgcpbar = false;
			}
			else
			{
				hasgcpbar = true;
			}

			if (hasgcpbar)
			{
				//在gcp页卡新增一栏
				AddSigGcp addgcp;
				//获取原gcpsrs,填写name x y z值
				auto srsgcp = block_data_->GetCurrentATMutual()->GetControlPointsMutual().begin()->second.GetSrsMutual();
				std::string srsname = srsgcp.name;
				AI3D::PROJ::CoordinateReferenceSystem crs(srsgcp.definition);
				AI3D::PROJ::CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem((crs));
				addgcp.Init(QString::fromStdString(srsgcp.definition));

				//addgcp.SetSrsName(srsname.c_str());
				if (QDialog::Accepted != addgcp.exec())
				{
					return;
				}
				QStringList gcpList = addgcp.getPosList();
				int id = block_data_->GetCurrentATMutual()->GenerateValidGCPId();
				AI3D::CORE::ControlPoints gcps_import;
				std::cout << addgcp.GetSrsName().toStdString() << std::endl;

				srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(addgcp.GetSrsName().toStdString());
				srs.ID = block_data_->ExistSRS(srs.definition);
				//for (auto& gcp_it : gcpList)
				{
					AI3D::CORE::ControlPoint gcp;
					gcp.SetSrs(srs);

					gcp.SetName(gcpList.at(0).toStdString());
					//ly 2021109ly添加也即从界面上获取的控制点坐标，此处也可以放在improtgcpdag中 = gcp_it
					Eigen::Vector3d xyz(gcpList.at(1).toDouble(), gcpList.at(2).toDouble(), gcpList.at(3).toDouble());
					gcp.GetGivenXYZMutual() = xyz;
					gcp.SetId(id);
					gcps_import.ADDPoint(gcp);
					//id++;
				}
				std::string definition = block_data_->GetCurrentATMutual()->GetLocalSrs();
				if (definition == "")
				{
					definition = BASESRS;
				}
				gcps_import.TransformPointsToBaseCoordinate(definition);


				//如果有控制点导入，则根据目前是否有控制点再决定GCP页卡的状态；同时需要转换坐标系统
				auto& gcps_recent = block_data_->GetCurrentATMutual()->GetControlPointsMutual();
				for (auto& gcp_it : gcps_import.GetPointsMutual())
				{
					gcps_recent[gcp_it.first] = gcp_it.second;
				}
				controlPoints_ui_->InitGcpData();
			}
			else
			{
				//添加gcp页卡，同导入gcp.txt文件
				AddSigGcp addgcp;
				
				AI3D::PROJ::CoordinateReferenceSystem crs(std::string("Local:0"));
				AI3D::PROJ::CoordinateReferenceSystem::InsertRecentCoordinateReferenceSystem((crs));
				addgcp.Init(QString::fromStdString(crs.GetAuthID()));
				if (QDialog::Accepted != addgcp.exec())
				{
					return;
				}

				QStringList gcpList = addgcp.getPosList();
				bool iscover = false;
				//导入的坐标系与之前导入的相同 累加控制点值
				if (block_data_->GetCurrentATMutual()->HasControlPoints())
				{
					if (addgcp.GetSrsName().toStdString() != block_data_->GetCurrentATMutual()->GetControlPointsMutual().begin()->second.GetSrsMutual().name)
					{
						block_data_->GetCurrentATMutual()->DeleteGCPs();
						iscover = true;
					}
				}

				AI3D::CORE::ControlPoints gcps_import;
				std::cout << addgcp.GetSrsName().toStdString() << std::endl;
				srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(addgcp.GetSrsName().toStdString());
				gcps_import.SetSRS(srs.definition);
				block_data_->UpdateSRSMap(srs);
				srs.ID = block_data_->ExistSRS(srs.definition);
				int id = block_data_->GetCurrentATMutual()->GenerateValidGCPId();
				//for (auto& gcp_it : gcpList)
				{
					AI3D::CORE::ControlPoint gcp;
					gcp.SetSrs(srs);

					gcp.SetName(gcpList.at(0).toStdString());

					//ly 2021109ly添加也即从界面上获取的控制点坐标，此处也可以放在improtgcpdag中 = gcp_it
					Eigen::Vector3d xyz(gcpList.at(1).toDouble(), gcpList.at(2).toDouble(), gcpList.at(3).toDouble());
					gcp.GetGivenXYZMutual() = xyz;
					gcp.SetId(id);
					gcps_import.ADDPoint(gcp);
					//id++;
				}
				std::string definition = block_data_->GetCurrentATMutual()->GetLocalSrs();
				if (definition == "")
				{
					definition = BASESRS;
				}
				gcps_import.TransformPointsToBaseCoordinate(definition);


				//如果有控制点导入，则根据目前是否有控制点再决定GCP页卡的状态；同时需要转换坐标系统
				auto& gcps_recent = block_data_->GetCurrentATMutual()->GetControlPointsMutual();
				for (auto& gcp_it : gcps_import.GetPointsMutual())
				{
					gcps_recent[gcp_it.first] = gcp_it.second;
				}

				if (!block_data_->GetCurrentATMutual()->HasControlPoints())
				{
					return;
				}

				/*---*/
				//接下来需要显示到界面上，需要转换到界面显示的坐标系下
				//ly2021109

				ControlPointsEditorWin* controlPoints_ui = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
				controlPoints_ui->InitGcpData();
				if (iscover)
				{
					//controlPoints_ui->SetSrsCombox(QString::fromLocal8Bit(srs.name.c_str()));
				}

				//controlPoints_ui->TableView_Load_GcpData(QString::fromLocal8Bit(srs.name.c_str()));
				controlPoints_ui_ = controlPoints_ui;
				connect(controlPoints_ui_, &ControlPointsEditorWin::Sig_ModifiedTrue, this, &BlockWgt::SetModifityXml);
				//更新页卡
				ProjectManager* project = ProjectManager::GetInstance();
				project->AddBlockManager(block_data_);
				std::vector<int> tabvec;
				bool ATFlag = false;
				auto blockstatus = block_data_->GetStatus();
				if (blockstatus == jobsta_e::STATUS_NEW || blockstatus == jobsta_e::STATUS_UNKNOWN)
				{
					ATFlag = false;
				}
				else
				{
					ATFlag = true;
				}
				myTabWidget_[GCPTAB.c_str()] = controlPoints_ui;
				//ExistsTab(ATTAB)
				project->GetBlockManaget(block_data_->GetId())->ChangeTab(ATFlag, block_data_->GetCurrentAT()->HasImages(), \
					block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
				UpdateTabPaper(tabvec);
				if (ui->tabWidget->tabText(0).toStdString() == GCPTAB)
				{
					ui->tabWidget->removeTab(0);
				}
				SetIndexByStr(GCPTAB.c_str());

				BlockManager* blockmanager = project->GetBlockManaget(block_data_->GetId());
				auto blockwgtstatus = blockmanager->GetBlockStatusMutual();

				//blockstatus.can_del_pos = false;
				SetWgtStatus(blockwgtstatus);
				SetModifityXml();
				Update3DView();

			}
		}

		void BlockWgt::Slot_Btn_AddGcp_Clicked()
		{
			//std::cout << " inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			ImportGcpDia impGcpDialog;
			//设置上次打开的路径
			impGcpDialog.setOldFileName("C:/Program Files");
			QMap<int, QString> m_RecordData = impGcpDialog.getRecordData();
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			if (QDialog::Accepted != impGcpDialog.exec())
			{
				//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				return;
			}
			//
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

			QList<QStringList> gcpList = impGcpDialog.getPosList();
			if (gcpList.count() > 0)
			{
				bool iscover = false;
				//导入的坐标系与之前导入的相同 累加控制点值
				if (block_data_->GetCurrentATMutual()->HasControlPoints())
				{
					if (impGcpDialog.GetSrsName().toStdString() != block_data_->GetCurrentATMutual()->GetControlPointsMutual().begin()->second.GetSrsMutual().name)
					{
						block_data_->GetCurrentATMutual()->DeleteGCPs();
						iscover = true;
					}
				}

				/*---*/
				//转换到controlpoints;
				AI3D::CORE::ControlPoints gcps_import;
				srs_s srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(impGcpDialog.GetSrsName().toStdString());
				gcps_import.SetSRS(srs.definition);
				block_data_->UpdateSRSMap(srs);
				srs.ID = block_data_->ExistSRS(srs.definition);
				int id = block_data_->GetCurrentATMutual()->GenerateValidGCPId();
				for (auto& gcp_it : gcpList)
				{
					AI3D::CORE::ControlPoint gcp;
					gcp.SetSrs(srs);

					gcp.SetName(gcp_it.at(0).toStdString());
					//ly 2021109ly添加也即从界面上获取的控制点坐标，此处也可以放在importgcpdag中 = gcp_it
					Eigen::Vector3d xyz(gcp_it.at(1).toDouble(), gcp_it.at(2).toDouble(), gcp_it.at(3).toDouble());
					gcp.GetGivenXYZMutual() = xyz;
					gcp.SetId(id);
					gcps_import.ADDPoint(gcp);
					id++;
				}
				//chy add 此处将来需要统一将其他转为GCP坐标系下
				std::string definition = block_data_->GetCurrentATMutual()->GetLocalSrs();
				if (definition == "" || definition == LOCALSRS)
				{
					definition = BASESRS;
				}
				gcps_import.TransformPointsToBaseCoordinate(definition);


				//如果有控制点导入，则根据目前是否有控制点再决定GCP页卡的状态；同时需要转换坐标系统
				auto& gcps_recent = block_data_->GetCurrentATMutual()->GetControlPointsMutual();
				for (auto& gcp_it : gcps_import.GetPointsMutual())
				{
					gcps_recent[gcp_it.first] = gcp_it.second;
				}

				if (!block_data_->GetCurrentATMutual()->HasControlPoints())
				{
					return;
				}
				block_data_->GetCurrentATMutual()->SetLocalGcpSrs(gcps_recent.cbegin()->second.GetSrs().definition);
				/*---*/
				//接下来需要显示到界面上，需要转换到界面显示的坐标系下
				//ly2021109

				ControlPointsEditorWin* controlPoints_ui = new ControlPointsEditorWin(block_data_, viewWidget_ui, viewWidget_ui);
				controlPoints_ui->InitGcpData();
				if (iscover)
				{
					//controlPoints_ui->SetSrsCombox(QString::fromLocal8Bit(srs.name.c_str()));
				}

				//controlPoints_ui->TableView_Load_GcpData(QString::fromLocal8Bit(srs.name.c_str()));
				controlPoints_ui_ = controlPoints_ui;
				connect(controlPoints_ui_, &ControlPointsEditorWin::Sig_ModifiedTrue, this, &BlockWgt::SetModifityXml);
				//更新页卡
				ProjectManager* project = ProjectManager::GetInstance();
				project->AddBlockManager(block_data_);
				std::vector<int> tabvec;
				bool ATFlag = false;
				auto blockstatus = block_data_->GetStatus();
				if (blockstatus == jobsta_e::STATUS_NEW || blockstatus == jobsta_e::STATUS_UNKNOWN)
				{
					ATFlag = false;
				}
				else
				{
					ATFlag = true;
				}
				myTabWidget_[GCPTAB.c_str()] = controlPoints_ui;
				//ExistsTab(ATTAB)
				project->GetBlockManaget(block_data_->GetId())->ChangeTab(ATFlag, block_data_->GetCurrentAT()->HasImages(), \
					block_data_->GetCurrentAT()->HasControlPoints(), tabvec);
				UpdateTabPaper(tabvec);
				if (ui->tabWidget->tabText(0).toStdString() == GCPTAB)
				{
					ui->tabWidget->removeTab(0);
				}
				SetIndexByStr(GCPTAB.c_str());

				BlockManager* blockmanager = project->GetBlockManaget(block_data_->GetId());
				auto blockwgtstatus = blockmanager->GetBlockStatusMutual();

				//blockstatus.can_del_pos = false;
				SetWgtStatus(blockwgtstatus);
				SetModifityXml();
				Update3DView();
			}

		}
		void BlockWgt::Slot_Btn_DelGcp_Clicked()
		{

			if (!block_data_->GetCurrentATMutual()->HasControlPoints())
				return;

			CommonDelDia commondia;
			commondia.SetInfor(tr("Do  you  really want to remove all GCP data?"));
			if (commondia.exec() == QDialog::Rejected)
			{
				return;
			}
			//不需删除gcp页卡
			std::string srs_def = block_data_->GetCurrentATMutual()->GetControlPoints().begin()->second.GetSrs().name;

			//删除数据
			block_data_->GetCurrentATMutual()->DeleteGCPs();

			//controlPoints_ui_->TableView_Load_GcpData(/*QString::fromLocal8Bit(srs_def.c_str())*/);
			controlPoints_ui_->Slot_DeleteAllGcps();
			//更新页卡
			//UpdateAllListItems();
			SetModifityXml();
			Update3DView();
			/*ProjectManager* promanager = ProjectManager::GetInstance();
			Block_Status_s& BlockStatus = promanager->GetBlockManaget(block_data_->GetId())->GetBlockStatusMutual();
			SetWgtStatus(BlockStatus);*/
			ui->btn_delgcp->setEnabled(false);
		}


	}
}
