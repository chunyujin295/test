#include "Gui/ProjectInfoWgt.h"
#include "qdir.h"
#include "qmessagebox.h"
#include "qfiledialog.h"

#include <QScrollBar>
#include <QDesktopServices>
#include "Gui/ProjectManager.h"
#include "Util/TaskProcess.h"

namespace AI3D
{
	namespace GUI
	{
		ProjectInfoWgt::ProjectInfoWgt(QString proname, QString propath, QWidget* parent) :
			QWidget(parent),
			ui(new Ui::CProjectInfoWgt)
		{
			ui->setupUi(this);
			pro_path_ = propath;
			pro_name_ = proname;
			ui->label_ProPath_2->setEnabled(false);
			ui->label_ProName_2->setEnabled(false);
			/// init tablewidget
			Init_Widget();
			/// update blocks information
			read_block_info_time = new QTimer(this);
			connect(read_block_info_time, &QTimer::timeout, this, &ProjectInfoWgt::Slot_Read_Block_Info_Time);


			// alke
			/*if (_projectData->masterJobqueue.isEmpty())
			{
				_projectData->masterJobqueue = TRIGLobal::jobQueuePath + "/"+ Tri_configuration_name;

			}
			else {
				QDir dir(_projectData->masterJobqueue);
				dir.cdUp();
			}*/

			//QString absolutepath = pro_path_ + "/" + pro_name_;
			QString absolutepath = pro_path_;
			absolutepath.replace("\\","/");
			ShowProjectPath(absolutepath);
			ShowProjectName(pro_name_.replace("\\", "/"));
			//projPath = _projectData->projectPath;

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui->label_ProName->setText("工程名称");
				ui->label_ProPath->setText("工程路径");
			}
		}




		void ProjectInfoWgt::ShowProjectPath(QString str)
		{
			ui->label_ProPath_2->setText(str);
		}

		void ProjectInfoWgt::ShowProjectName(QString str)
		{
			ui->label_ProName_2->setText(str);
		}


		ProjectInfoWgt::~ProjectInfoWgt()
		{
			// alka 1.4.231
			read_block_info_time->stop();
			// alke

			delete ui;
		}

		void ProjectInfoWgt::SetProjectInfo(std::unordered_map<std::string, std::vector<int>> projinfo)
		{
			/*while (ui->tableWidget->rowCount() > 0)
				ui->tableWidget->removeRow(0);*/

			ui->tableWidget->clearContents();
			ui->tableWidget->setRowCount(0);
			ProjectManager* promanage = ProjectManager::GetInstance();
			
			for (auto& it : projinfo)
			{
				
				int iRow = ui->tableWidget->rowCount();
				ui->tableWidget->setRowCount(iRow + 1);
				for (int i = 0; i < 5; i++) {

					QTableWidgetItem* item = new QTableWidgetItem();
					item->setTextAlignment(Qt::AlignCenter);
					if (i == 0)
					{
						///item->setText(QString::fromStdString(it.first));
						std::string tmpstr = it.first;
						item->setText(str2qstr(tmpstr));
					}
					else if (i == 1)
						item->setText(QString::number(it.second.at(0)));
					else if (i == 2)
						item->setText(QString::number(it.second.at(1)));
					else if (i == 3)
						item->setText(QString::number(it.second.at(2)));
					else if (i == 4)
						item->setText(QString::number(it.second.at(3)));
					
					ui->tableWidget->setItem(iRow, i, item);

				}
			}
			ui->tableWidget->sortByColumn(0,Qt::AscendingOrder);
			return;
		}
		// alka 1.4.231
		
		void ProjectInfoWgt::Init_Widget() {
			//设置table头
			ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
			//ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
			//ui->tableWidget->verticalHeader()->setDefaultSectionSize(20);
			ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			ui->tableWidget->horizontalHeader()->setHighlightSections(false);
			ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
			ui->tableWidget->setShowGrid(false);
			ui->tableWidget->setStyleSheet("QTableWidget::item:hover{background-color:#333333;}"\
				"QTableWidget::item{ font: 14px 'Arial';background-color:#222222;\
		color:#FFFFFF; border-bottom:1px solid #3E3E3E; min-height:28px; max-height:28px; }"
				"QTableWidget::item:selected{background-color:#333333}"
				"QHeaderView::section,QTableCornerButton:section{ \
            font: 12px 'Arial';padding:3px; margin:0px; color:#DCDCDC;  border:1px solid #242424; \
    border-left-width:0px; border-right-width:0px; border-top-width:0px; border-bottom-width:0px; \
background:#333333; }"\
"QTableWidget#tableWidget{background-color:#222222; font: 14px 'Arial'; color:#FFFFFF;border:none;}");
			QStringList strList;
			QString strBlockIDtr(tr("BlockName"));
			QString strPhotos(tr("Photos"));
			QString strRegPhotos(tr("Registered photos"));
			QString strCPoint(tr("Control points"));
			QString strTiePoint(tr("Automatic tie points"));


			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				strBlockIDtr = "区块名称";
				strPhotos = "影像数";
				strRegPhotos = "解算成功影像数";
				strCPoint = "控制点数";
				strTiePoint = "连接点数";
			}
			
			strList << strBlockIDtr << strPhotos << strRegPhotos << strCPoint << strTiePoint;
			//set the horizontal title
			ui->tableWidget->setColumnCount(5);
			ui->tableWidget->setHorizontalHeaderLabels(strList);
			//ui->tableWidget->horizontalHeader()->setMinimumHeight(50);
			ui->tableWidget->verticalHeader()->setHidden(true);
			//set the alignment way
			ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
			//ui->tableWidget->horizontalHeader()->setStyleSheet(QString::fromUtf8("font: 16pt '华文细黑'"));
			ui->tableWidget->horizontalScrollBar()->setDisabled(true);
			QObject::connect(ui->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnBlockIconDoubleClicked(QTableWidgetItem*)));
		}

		void ProjectInfoWgt::Slot_Read_Block_Info_Time() {

			read_block_info_time->stop();
			//// read blocks information
			///// photos 、control points and automatic tie ponits
			int n = ui->tableWidget->rowCount();
			//int n_ = _projectData->list_blocks.size();

			// remove
			for (int i = n; i > 0; i--) {
				ui->tableWidget->removeRow(0);
			}
			// add
			QStringList infos;
			QStringList indexs;
			///TrimotionWgt::getInstance()->getBlocksInfo(infos, indexs);
			//for (int i = 0;i < infos.size();i++) 
				//add_block_info(i,infos[i], indexs[i]);


		}

		void ProjectInfoWgt::Update_Block_Info() 
		{
			
			//ui->tableWidget->clear();
			while (ui->tableWidget->rowCount() > 0)
				ui->tableWidget->removeRow(0);
			ProjectManager* promanage = ProjectManager::GetInstance();
			auto setids = promanage->GetProject()->GetBlocksMutual();// GetBlockIds();
			/*auto projInfo = promanage->GetProject()->GetBlocksStatisics();
			std::unordered_map<block_t, std::vector<int>> mapinfo;
			for (auto it : projInfo)
			{
				mapinfo.insert(std::pair<block_t, std::vector<int>>(it.first, it.second));
			}*/
			for (auto& it: setids)
			{
				auto block = promanage->GetProject()->GetBlock(it.first);
				if (block->GetCurrentAT() == nullptr)
				{
					continue;
				}
				int iRow = ui->tableWidget->rowCount();
				ui->tableWidget->setRowCount(iRow + 1);
				
				for (int i = 0; i < 5; i++)
				{
					QTableWidgetItem* item = new QTableWidgetItem();
					item->setTextAlignment(Qt::AlignCenter);
					int imagenum, regimagenum, gcpnum;
					
					{
						imagenum = block->GetNumImages();
						regimagenum = block->GetCurrentATMutual()->GetNumRegImages();
						gcpnum = block->GetCurrentATMutual()->GetNumControlPoints();
					}

					if (i == 0)
					{
						///item->setText(QString::fromStdString(block->GetTaskInfo().blockString));
						item->setText(str2qstr(block->GetTaskInfo().blockString));
					}
					else if (i == 1)
					{
						/*int*/
						item->setText(QString::number(imagenum));
					}
					else if (i == 2)
					{
						/*int*/ 
						item->setText(QString::number(regimagenum));
					}
					else if (i == 3)
					{
						/*int*/ 
						item->setText(QString::number(gcpnum));
					}
					else if (i == 4)
					{
						int tieptnum = block->GetTiepointStatus() ? block->GetCurrentATMutual()->GetNumPoints3D() : block->GetTaskInfo().statisticinfo_.tiepointnum;
						item->setText(QString::number(tieptnum));
					}
					ui->tableWidget->setItem(iRow, i, item);
				}

				
			}
			ui->tableWidget->sortByColumn(0, Qt::AscendingOrder);
		}

		QString ProjectInfoWgt::GetProjectPath()
		{
			return projPath;
		}

		void ProjectInfoWgt::OnBlockIconDoubleClicked(QTableWidgetItem* item) {

			//20210917ly 屏蔽
			return;
			int row = item->row();
			QString str = ui->tableWidget->item(row, 0)->text();
			QStringList strs = str.split("_");
			//MohackerWgt::getInstance()->setTreeViewByIndex(strs[1].toInt());

		}
	}
}
// alke