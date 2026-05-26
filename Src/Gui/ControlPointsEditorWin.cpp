

#include "Gui/ControlPointsEditorWin.h"
#include "Gui/MohackerWin.h"
#include "Core/CoordinateSystem.h"
#include <QScrollBar>
#include <QtConcurrent>
#include <QClipBoard>
#include "Core/File.h"
#include "Core/Timer.h"
#include "Util/TaskProcess.h"
#include "3DViewer/model_viewer_widget.h"
#include <QStandardItemModel>
#include "3DViewer/3dview.h"
#include "Core/ATCommandSet.h"
#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"

#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Gui/ProjectionSelectionTreeWidget.h"
//#include "ProjCore/qgscoordinatereferencesystem.h"
//#include "Gui/qgsprojectionselectiontreewidget.h"

#endif // USE_AI3D_PROJ
#define DISABLEDSTYLE "background-color: gray;"
#define SELECTEDSTYLE "background-color: white;"
#define DEFAULTSTYLE "background-color: #222222;"
#define IMGNUM 10

// Gcp PreviewListView 按照缩略图预测点与缩略图中心点的距离,进行升序排列.
bool xyLessThan(const gcp_measurement_list_item_st& lhs, const gcp_measurement_list_item_st& rhs)
{
	double estimated_x = lhs.estimated_x_;
	double estimated_y = lhs.estimated_y_;
	double width = (double)lhs.width;
	double height = (double)lhs.height;

	double estimated_x2 = width / 2;
	double estimated_y2 = height / 2;

	double restimated_x = rhs.estimated_x_;
	double restimated_y = rhs.estimated_y_;

	double rwidth = (double)rhs.width;
	double rheight = (double)rhs.height;

	double restimated_x2 = rwidth / 2;
	double restimated_y2 = rheight / 2;

	double distance = sqrt(pow((estimated_x - estimated_x2), 2) + pow((estimated_y - estimated_y2), 2));
	double rdistance = sqrt(pow((restimated_x - restimated_x2), 2) + pow((restimated_y - restimated_y2), 2));

	// lhs.color_ > 5 表明是特殊的预览图节点,每一个分组的分组占位Item,位于每一组的第一项,并不是真正的缩略图.
	if (lhs.color_ > 5)
		return true;
	else if (rhs.color_ > 5)
		return false;

	int lhsArea = 4;
	double lhsDistanceX = 0.0;
	double lhsDistanceY = 0.0;

	int rhsArea = 4;
	double rhsDistanceX = 0.0;
	double rhsDistanceY = 0.0;

	lhsDistanceX = fabs(estimated_x - estimated_x2);
	lhsDistanceY = fabs(estimated_y - estimated_y2);

	rhsDistanceX = fabs(restimated_x - restimated_x2);
	rhsDistanceY = fabs(restimated_y - restimated_y2);

	if (lhsDistanceX < estimated_x2 * 0.1 && lhsDistanceY < estimated_y2 * 0.3)
		lhsArea = 0;
	else if (lhsDistanceX < estimated_x2 * 0.3 && lhsDistanceY < estimated_y2 * 0.6)
		lhsArea = 1;
	else if (lhsDistanceX < estimated_x2 * 0.6 && lhsDistanceY < estimated_y2 * 0.6)
		lhsArea = 2;
	else if (lhsDistanceX < estimated_x2 * 0.9 && lhsDistanceY < estimated_y2 * 0.9)
		lhsArea = 3;

	if (rhsDistanceX < restimated_x2 * 0.1 && rhsDistanceY < restimated_y2 * 0.3)
		rhsArea = 0;
	else if (rhsDistanceX < restimated_x2 * 0.3 && rhsDistanceY < restimated_y2 * 0.6)
		rhsArea = 1;
	else if (rhsDistanceX < restimated_x2 * 0.6 && rhsDistanceY < restimated_y2 * 0.6)
		rhsArea = 2;
	else if (rhsDistanceX < restimated_x2 * 0.9 && rhsDistanceY < restimated_y2 * 0.9)
		rhsArea = 3;

	if (lhsArea < rhsArea)
		return true;
	else if (lhsArea > rhsArea)
		return false;
	else if (lhsDistanceX < rhsDistanceX)
		return true;
	else if (lhsDistanceX > rhsDistanceX)
		return false;
	else if (lhsDistanceY <= rhsDistanceY)
		return true;
	else
		return false;
}


bool gcp_measurement_list_item_st::compareLessThan(const gcp_measurement_list_item_st& t1, const gcp_measurement_list_item_st& t2)
{
	return xyLessThan(t1, t2);
}
namespace AI3D
{
	namespace GUI
	{

		
		ControlPointsEditorWin::ControlPointsEditorWin(AI3D::CORE::BlockObject* block,  QWidget* parent, AI3D::GUI::ViewWidget* viewWidget) :
			QMainWindow(parent), blockdata_(nullptr),
			point_(0,0),	
			ui(new Ui_ControlPointsWin2)
		{
			ui->setupUi(this);
			// no need to create new object.
			//?chy
			///blockdata_ = new AI3D::CORE::BlockObject;
			blockdata_ = block;
			block_path_ = blockdata_->GetPath();//chy当去掉block时此处改为传入参数
			this->viewWidget = viewWidget;

			bChangingSrs = false;
			Init();

			QString stylesheet = "QPushButton{"
				"height:22px;border-radius:4px;color:#A5A5A5;border:1px solid #5B5B5B;padding:3px 10px;margin:3px;background-color:#282828;font:14px \"Arial\";}"
				"QPushButton:checked{background-color:#2E598A;color:white;border:1px solid #3572B8;}"
				"QPushButton:disabled{background-color:gray;}";
			
			ui->btn_epipolarline->setStyleSheet(stylesheet);

			bshow_epipolarline_ = true;
			ui->btn_epipolarline->setCheckable(true);//可以选中
			ui->btn_epipolarline->setChecked(true);//已被选中


			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui->btn_AllPho->setText("所有影像");
				ui->btn_MarkPho->setText("已刺点影像");
				ui->btn_MatchPho->setText("预测影像");
				ui->btn_epipolarline->setText("显示核线");

				ui->label->setText("控制点列表");
				ui->label_2->setText("已刺点:");
				ui->label_4->setText("观测值");
			}
		}

		ControlPointsEditorWin::~ControlPointsEditorWin()
		{
			delete ui;

			for (auto& item : previewListMap.values())
			{
				if (item != nullptr)
					delete item;
			}

			previewListMap.clear();
		}

		void ControlPointsEditorWin::InitPhotosButtons()
		{
			QString stylesheet = "QPushButton{"
				"height:22px;border-radius:4px;color:#A5A5A5;border:1px solid #5B5B5B;padding:3px 10px;margin:3px;background-color:#282828;font:14px \"Arial\";}"
				"QPushButton:checked{background-color:#2E598A;color:white;border:1px solid #3572B8;}";

			ui->btn_AllPho->setStyleSheet(stylesheet);
			ui->btn_MatchPho->setStyleSheet(stylesheet);
			ui->btn_MarkPho->setStyleSheet(stylesheet);
			ui->btn_AllPho->setEnabled(false);
			ui->btn_MatchPho->setEnabled(false);
			ui->btn_MarkPho->setEnabled(false);

			UpdatePreviewListViewBottonStatus();
		}

		void ControlPointsEditorWin::Init()
		{
			qRegisterMetaType<QPointF>("MyPointF");

			InitSrss();

			ParseDefaultSrs();

			InitGcpListView();

			InitPreviewListView();//
			InitMeasuringView();

			CreateConnect();
			//获取需要生成缩略图的影像
			//InitImageSet();

			ui->gcplistview->setContextMenuPolicy(Qt::CustomContextMenu);

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui_action_deletegcp_ = new QAction(tr("删除"), ui->gcplistview);
				/*ui_action_addgcp_ = new QAction(tr("插入"), ui->gcplistview);*/
				ui_action_copygcp_ = new QAction(tr("复制"), ui->gcplistview);
			}
			else
			{
				ui_action_deletegcp_ = new QAction(tr("Delete"), ui->gcplistview);
				/*ui_action_addgcp_ = new QAction(tr("Insert"), ui->gcplistview);*/
				ui_action_copygcp_ = new QAction(tr("Copy"), ui->gcplistview);
			}

			ui_menu_rightClick_selectRows = new QMenu(ui->gcplistview);
			ui_menu_rightClick_selectRows->addAction(ui_action_deletegcp_);
			/*ui_menu_rightClick_selectRows->addAction(ui_action_addgcp_);*/
			ui_menu_rightClick_selectRows->addAction(ui_action_copygcp_);

			ui->measurementsview->setContextMenuPolicy(Qt::CustomContextMenu);
			ui_menu_rightClickMeasure_selectRows = new QMenu(ui->measurementsview);
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui_action_deletemeausurement_ = new QAction(tr("删除"), ui->measurementsview);
			}
			else
			{
				ui_action_deletemeausurement_ = new QAction(tr("Delete"), ui->measurementsview);
			}
			ui_menu_rightClickMeasure_selectRows->addAction(ui_action_deletemeausurement_);
				SetEditable(!blockdata_->HasReconstructions());
			connect(ui_action_deletegcp_, &QAction::triggered, this, &ControlPointsEditorWin::Slot_DeleteGcp);
			connect(ui_action_copygcp_, &QAction::triggered, this, &ControlPointsEditorWin::Slot_CopyGcp);
			if (this->viewWidget != nullptr)
			{
				connect(this->viewWidget, &AI3D::GUI::ViewWidget::signal_add_user_tie_point, this, &ControlPointsEditorWin::Slot_add_user_tie_point);
				//connect(this->viewWidget, &AI3D::GUI::ViewWidget::signal_insert_gcp_tab, this, &ControlPointsEditorWin::Sig_InsertGCPTab);
			}
		}
		void ControlPointsEditorWin::SetEditable(bool be)
		{
			MakeGcplistViewUneditable(be);
			//屏蔽相关按钮
			
			if(ui_action_deletegcp_)
				ui_action_deletegcp_->setEnabled(be);
			if (ui_action_deletemeausurement_ )
				ui_action_deletemeausurement_->setEnabled(be);
			// 
			//屏蔽刺点操作
		}

		void ControlPointsEditorWin::InitImageSet()
		{
			imageids_forgenpreview_ = blockdata_->GetCurrentATMutual()->GetImagesIdSet();
		}

		void ControlPointsEditorWin::CreateConnect()
		{
		
			QObject::connect(ui->btn_AllPho,      &QPushButton::clicked, this, &ControlPointsEditorWin::Slot_AllPhotos_Clicked);
			QObject::connect(ui->btn_MatchPho,    &QPushButton::clicked, this, &ControlPointsEditorWin::Slot_MatchedPhotos_Clicked);
			QObject::connect(ui->btn_MarkPho,     &QPushButton::clicked, this, &ControlPointsEditorWin::Slot_MarkedPhotos_Clicked);	

			QObject::connect(ui->previewlistview, &MoListWidget::previewImg, this, &ControlPointsEditorWin::displayImage);

			QObject::connect(measuringview_, SIGNAL(PosPoint(int, QPointF)), this, SLOT(Slot_Measuring_Clicked(int, QPointF)), Qt::QueuedConnection);
			QObject::connect(measuringview_, SIGNAL(ChangeScale(double)), this, SLOT(Slot_SetCurrentScale(double)), Qt::QueuedConnection);
			//chy?
			QObject::connect(measuringview_, SIGNAL(MousePoint(QPointF)), this, SLOT(Slot_SetCurrentOffset(QPointF)), Qt::QueuedConnection);

			QObject::connect(ui->gcplistview, &QTableView::customContextMenuRequested, this, &ControlPointsEditorWin::Slot_QTableView_CustomContextMenuRequested);
			QObject::connect(ui->gcplistview, &MoTableWidget::itemModified, this, &ControlPointsEditorWin::Slot_itemModified);
			QObject::connect(ui->gcplistview, &QTableView::clicked, this, &ControlPointsEditorWin::tableviewClick);		

			if (ui->gcplistview->model())
			{
				///QObject::connect(qobject_cast<QStandardItemModel*>(ui->gcplistview->model()), &QStandardItemModel::itemChanged, this, &ControlPointsEditorWin::Slot_ItemDataChanged);
			}

			QObject::connect(ui->measurementsview, &QTableWidget::customContextMenuRequested, this, &ControlPointsEditorWin::Slot_QTableWidget_CustomContextMenuRequested);
			QObject::connect(ui->measurementsview, &QTableView::clicked, this, &ControlPointsEditorWin::measurement_tableviewClick);

			QObject::connect(ui->comboBox_srs,     &QComboBox::currentTextChanged, this, &ControlPointsEditorWin::Slot_SrsItemChanged, Qt::QueuedConnection);				
			QObject::connect(ui->btn_epipolarline, &QPushButton::clicked, this, &ControlPointsEditorWin::Slot_ShowEpi);
			
			
		}

		bool ControlPointsEditorWin::PostProcessAfterClickingGcpListView(const QModelIndex& index)
		{
			QString strItem = ui->gcplistview->model()->index(index.row(), CATEGORY2_COL).data(Qt::EditRole).toString();

			// using setItemData(index,xx) / itemData(index) to retrieve chinese strings if needed.
			// whether to set chinese strings directly or to still set english strings but with chinese information as additional item data inside Chinese enviroment?
		
			if (strItem == "User Tiepoint")
			{
				ui->btn_AllPho->setEnabled(true);
				ui->btn_AllPho->setChecked(true);
				ui->btn_MarkPho->setChecked(false);
				ui->btn_MatchPho->setChecked(false);

				return true;
			}
			else
			{
				ui->btn_AllPho->setEnabled(false);
				ui->btn_AllPho->setChecked(false);

				return false;
			}

			// note:temporarily force to enable the following two buttons to avoid the disabled state after this simulated click.
			// note:forcing to enabled state for the next two buttons here may bring mysterious crash.
			//ui->btn_MatchPho->setEnabled(true);
			//ui->btn_MarkPho->setEnabled(true);

		///	std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << strItem.toStdString() << std::endl;

///			emit ui->btn_AllPho->clicked();
		}

		void ControlPointsEditorWin::Slot_ItemDataChanged(QStandardItem* item)
		{
			std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " " << item->text().toStdString() << std::endl;
		}

		void ControlPointsEditorWin::InitGcpListViewHeader()
		{
			// 根据选择的坐标类别, 显示不同的表头
			if (str2qstr(default_srs_.definition).left(3) == "WGS")
			{
				ui->gcplistview->setHeaderLabelsMode(true);
			}
			else
			{

				ui->gcplistview->setHeaderLabelsMode(false);

			}

		}
		void ControlPointsEditorWin::InitGcpListViewBottonStatus()
		{

			UpdateLabelRecoder();
		}

		void ControlPointsEditorWin::InitPreviewListViewHeader()
		{
			ui->previewlistview->clearData();
		}
		
		void ControlPointsEditorWin::InitSrss(bool bSetCurrentItem4Recent)
		{
			//设置默认焦点
			ui->comboBox_srs->setFocus();
			ui->comboBox_srs->setStyleSheet("{background-color: rgb(18, 18, 18);color: rgb(255, 255, 255);font: 14px 'Arial'; }");
			ui->comboBox_srs->clear();
			QStringList listCoords_default;
			QStringList listCoords_Recent;
			QStringList listCoords_More;			

#ifdef USE_AI3D_PROJ
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				listCoords_default << "默认";
				listCoords_Recent << "最近";
				listCoords_More << "更多";
				listCoords_More << (AI3D::GUI::MohackerWin::prependIndentation() + "空间参考系统数据库");
			}
			else
			{
				listCoords_default << "Default";
				listCoords_Recent << "Recent";
				listCoords_More << "More";
				listCoords_More << (AI3D::GUI::MohackerWin::prependIndentation() + "Spatial reference system database");
			}
			AI3D::PROJ::CoordinateReferenceSystem localcrs(std::string("Local:0"));
			AI3D::PROJ::CoordinateReferenceSystem wgscrs(std::string("EPSG:4326"));
			listCoords_default << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(localcrs.GetDescription())) 
				<< (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(wgscrs.GetDescription() + "(" + wgscrs.GetAuthID() + ")"));
			//chy 
			AI3D::PROJ::CoordinateReferenceSystem gcpcrs;
			if (!bSetCurrentItem4Recent && blockdata_->GetCurrentATMutual()->HasControlPoints())
			{
				
				QString strsrsdef = str2qstr(blockdata_->GetCurrentATMutual()->GetControlPoints().begin()->second.GetSrs().definition);
				gcpcrs.createFromString(strsrsdef);
				AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->InsertRecent(gcpcrs);
				
				
			}
			
			auto lists = AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->GetRecentCrs();

			QList< AI3D::PROJ::CoordinateReferenceSystem> filteredcrs;
			for (auto iter : lists)
			{

				AI3D::PROJ::CoordinateReferenceSystem crs(iter.GetAuthID());
				if (crs.isValid())
				{
					if (crs == localcrs || crs == wgscrs)
					{
						std::cout << " same" << iter.GetAuthID() << std::endl;
					}
					else
					{
						filteredcrs << iter;
					}
				}

			}

			int count = 0;
			//std::cout << lists.size() << std::endl;;
			for (auto iter : filteredcrs)
			{
				QString descrip = QString::fromStdString(iter.GetDescription());
				descrip.toUpper();
///				if (descrip.contains("ENU"))
///				{
///					listCoords_Recent << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription()));
///				}
///				else
					listCoords_Recent << (AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")"));

				//std::cout << " iter " << iter.GetDescription() + "(" + iter.GetAuthID() + ")" << std::endl;
				if (count == 7)
				{
					break;
				}
				count++;
			}

			ui->comboBox_srs->setEditable(false);
			ui->comboBox_srs->addItems(listCoords_default);
			ui->comboBox_srs->addItems(listCoords_Recent);
			ui->comboBox_srs->addItems(listCoords_More);

///			int retint = ui->comboBox_srs->findText(QString::fromStdString(gcpcrs.GetDescription()), Qt::MatchStartsWith);//暂时选用以开始
			int retint = ui->comboBox_srs->findText((AI3D::GUI::MohackerWin::prependIndentation() + QString::fromStdString(gcpcrs.GetDescription())), Qt::MatchStartsWith);//暂时选用以开始
///			std::cout << retint << std::endl;

			if (bSetCurrentItem4Recent && listCoords_Recent.size() > 1)
			{
				ui->comboBox_srs->blockSignals(true);
				ui->comboBox_srs->setCurrentIndex(listCoords_default.size() + 1);
				ui->comboBox_srs->blockSignals(false);
///				previous_srs = listCoords_Recent.at(1);
				previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(listCoords_Recent.at(1));
			}
			else
			{
//				std::cout << "gcp line:" << __LINE__ << " " << retint << std::endl;
				if (retint >= 0)
				{
					ui->comboBox_srs->blockSignals(true);
					ui->comboBox_srs->setCurrentIndex(retint);
					ui->comboBox_srs->blockSignals(false);

//					std::string str= "(" + gcpcrs.GetAuthID() + ")";
//					previous_srs = QString::fromStdString(str);

					previous_srs = ui->comboBox_srs->itemData(retint, Qt::DisplayRole).toString();
					previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(previous_srs);
				}
				else
				{
					//attention Add by chy 此处还没有逻辑因为不知道加啥逻辑
				}
			}
#else

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				listCoords_default << "默认";
				listCoords_Recent << "通用";
				listCoords_More << "更多";
			}
			else
			{
				listCoords_default << "Default";
				listCoords_Recent << "Common";
				listCoords_More << "More";
			}
			auto src_map = AI3D::CORE::CoordinateTransformer::CSG_coordinateSystem_Global();
			for (auto it = src_map.begin(); it != src_map.end(); it++)
			{
				if (it->first == "Default")
				{
					for (auto itsrcname : it->second)
					{
						listCoords_default << str2qstr(itsrcname.name);
					}

				}
				else if (it->first == "Common")
				{
					for (auto itsrcname : it->second)
					{
						listCoords_Recent << str2qstr(itsrcname.name);
					}
				}
				else if (it->first == "More")
				{
					for (auto itsrcname : it->second)
					{
						listCoords_More << str2qstr(itsrcname.name);
					}

				}
			}

			//listCoords_More << "ENU"; 
			ui->comboBox_srs->setEditable(false);
			ui->comboBox_srs->addItems(listCoords_default);
			ui->comboBox_srs->addItems(listCoords_Recent);
			ui->comboBox_srs->addItems(listCoords_More);

#endif

			//默认84坐标系
			//ui->comboBox_srs->setCurrentIndex(1);

			QModelIndex index_default = ui->comboBox_srs->model()->index(0, 0);
			QVariant v_0(0);
			ui->comboBox_srs->model()->setData(index_default, v_0, Qt::UserRole - 1);
			QModelIndex index_recent = ui->comboBox_srs->model()->index(listCoords_default.size(), 0);
			QVariant v_2(0);
			ui->comboBox_srs->model()->setData(index_recent, v_2, Qt::UserRole - 1);
			QModelIndex index_more = ui->comboBox_srs->model()->index(listCoords_default.size() + listCoords_Recent.size(), 0);
			QVariant v_12(0);
			ui->comboBox_srs->model()->setData(index_more, v_12, Qt::UserRole - 1);
			
			QStandardItemModel* pItemModel = qobject_cast<QStandardItemModel*>(ui->comboBox_srs->model());
			QFont fontText;
			fontText.setPixelSize(14);
			fontText.setFamily(QStringLiteral("Arial"));
			fontText.setBold(false);
			for (int i = 0; i < ui->comboBox_srs->count(); i++) {
				pItemModel->item(i)->setFont(fontText);
			}
			QFont fontTitle = fontText;
			fontTitle.setBold(true);
			pItemModel->item(0)->setFont(fontTitle);
			pItemModel->item(listCoords_default.size())->setFont(fontTitle);
			pItemModel->item(listCoords_default.size() + listCoords_Recent.size())->setFont(fontTitle);
			ui->widget_showGcp->setStyleSheet({ "background-color: #333333; color:#919191;" });


		}

		void ControlPointsEditorWin::pushRecentCRS(QString& crs)
		{
#ifdef USE_AI3D_PROJ
			AI3D::PROJ::CoordinateReferenceSystem newcrs;

			newcrs.createFromString(crs);
//			std::cout << "cpew:" << crs.toStdString() << std::endl;
			//AI3D::PROJ::coordinateReferenceSystemRegistry()->InsertRecent(new_crs);
			//QProj::coordinateReferenceSystemRegistry()->InsertRecent(newcrs);
			AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->InsertRecent(newcrs);
#endif
		}

		void ControlPointsEditorWin::SetDefaultSrs(srs_s srs)
		{
			default_srs_ = srs;
		}

		/*设置默认的坐标系统也即导入数据时选定的坐标系统来设置，
		1：列表中没有enu，但如果导入的数据有enu则追加进来
		2：如果是xml形式，有可能每个控制点有一个srs则取第一个，其余的需要转换，可以统一调用controlpoints类中的转换到basecoor；//由于gcp显示是现用现转化的因此不需要转变坐标系
		3：如果控制点有不同的坐标系也即2成立则需要转换并按转换后的显示*/
		void ControlPointsEditorWin::ParseDefaultSrs()
		{
			if (blockdata_->GetCurrentATMutual()->HasControlPoints())
			{
				auto iter = blockdata_->GetCurrentATMutual()->GetControlPoints().begin();
				default_srs_ = iter->second.GetSrs();
			}
			else//@liyue
			{


				srsid_t srs_id = blockdata_->ExistSRS(GEO84SRS);
				if (srs_id != kInvalidSrsId)
				{
					default_srs_ = blockdata_->GetSRSs().at(srs_id);
				}
				else
				{
					default_srs_ = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(GEO84SRS);
				}

			}
			SetComBoxCurrentSrs();
		}
		void ControlPointsEditorWin::SetComBoxCurrentSrs()
		{
			//设置坐标系为当前导入数据坐标系
			//SetComBoxCurrentSrs(default_srs_);
			//需考虑增加ENU
#ifdef USE_AI3D_PROJ

			
#else
			if (blockdata_->GetCurrentATMutual()->HasControlPoints()) 
			{
				std::string oristr_DE = default_srs_.definition;
				QString strenu = str2qstr(blockdata_->GetCurrentATMutual()->GetControlPoints().begin()->second.GetSrs().definition);
				QString strenu_ = strenu.left(3);
				if (!strenu_.compare("ENU", Qt::CaseInsensitive))
				{
					ui->comboBox_srs->addItem(str2qstr(default_srs_.name));
				}
			}
#endif
			SetCurrentSrs(default_srs_);

			///int retint = ui->comboBox_srs->findText(QString(default_srs_.name.c_str()), Qt::MatchStartsWith);//暂时选用以开始
			int retint = ui->comboBox_srs->findText((AI3D::GUI::MohackerWin::prependIndentation() + QString(default_srs_.name.c_str())), Qt::MatchStartsWith);//暂时选用以开始
			ui->comboBox_srs->setCurrentIndex(retint);

			//坐标转换

		}

		srs_s ControlPointsEditorWin::GetSelectSrs(QString srsname)
		{
			//从界面上获取用户选择的并解析成srs_s赋给current_srs；
			//pasregui
			srs_s current_srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(srsname.toStdString());

			return current_srs;

		}
		srs_s ControlPointsEditorWin::GetCurrentSrs()
		{
			return current_srs_;
		}

		//
		void ControlPointsEditorWin::SetCurrentSrs(srs_s srs)
		{
			current_srs_ = srs;

		}

		//==============GCP列表操作相关========begin======
		void ControlPointsEditorWin::Slot_GivenXYZChanged(int id, QString itemtext)
		{
			//Slot_GcpListItem_SingleClicked();
			if (currentgcp_id_ == kInvalidPoint3DId)
			{
				return;
			}
			auto& controlpoint = blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_];
			double value = itemtext.toDouble();
			measurement_error_map_.at(currentgcp_id_).clear();
			blockdata_->GetCurrentATMutual()->UpdateGivenGCP(currentgcp_id_, id, value, measurement_error_map_.at(currentgcp_id_));
			
			//atdata->getgcp(id)->updategivenx();//此函数改变了gcplistview的值和measurement；但没有更新matchphoto；
			////看看是否需要跟之前的 值做比较，也就是没改变的话其实不需要更新
			//if (value diff)
			//{
			
			//	//已经重新计算error 3d;和measurement error；
				UpdatePreviewListView();
				UpdateGcpListView();
				UpdateMeasuringView();//此处当没有匹配到图片时，就是显示空白的所以此处需要多梳理
				UpdateMeasurementsView();
			//5. 触发工程已被编辑
			//}
		}

		void ControlPointsEditorWin::DeleteGCP()
		{
			if (!blockdata_->GetCurrentATMutual()->HasControlPoints())
				return;


			QModelIndex index = ui->gcplistview->currentIndex();
			if (!index.isValid())
			{
				currentgcpindex = QModelIndex();
				return;
			}
			//auto currentgcp_id_ = ui->gcplistview->getGcpIdByRow(index.row());
			if (!globalid_to_localid_map_.count(currentgcp_id_))
			{
				return;
			}
			if (!IsCurrentSelectionUserType(currentgcp_id_))
			{
				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;				
				blockdata_->GetCurrentATMutual()->DeleteGCP(gcpid);
			}
			else
			{
				point3D_t userid = globalid_to_localid_map_.at(currentgcp_id_).first;
				blockdata_->GetCurrentATMutual()->DeleteUserPt(userid);
			}
			
			
			

			gcps_show_.erase(currentgcp_id_);

			ui->gcplistview->removeOneRow(index.row());

			currentgcpindex = ui->gcplistview->currentIndex();
			currentgcp_id_ = ui->gcplistview->getGcpIdByRow(index.row());
			UpdateSurveyListView();
			ShowSelectedGcpView();
			

			emit Sig_ModifiedTrue();

			if (ui->gcplistview->model()->rowCount() <= 0)
			{
				ui->previewlistview->clearData();
				ui->measurementsview->clearData();
				measuringview_->clear();
			}
		}
		void ControlPointsEditorWin::DeleteUserpoint()
		{

		}

		//接口不变内容变
		void ControlPointsEditorWin::Slot_DeleteGcp()
		{
		// note:delete user tie point.
			DeleteGCP();
			
		}

		void ControlPointsEditorWin::Slot_CopyGcp()
		{

			if (!blockdata_->GetCurrentATMutual()->HasControlPoints())
				return;


			QModelIndex index = ui->gcplistview->currentIndex();
			if (!index.isValid())
			{
				currentgcpindex = QModelIndex();
				return;
			}


			QString value = index.model()->data(index, Qt::EditRole).toString();
			//QMessageBox::information(nullptr,"info",value);
		
			QClipboard* clipBoard = QGuiApplication::clipboard();
			clipBoard->setText(value);

#if 0
			//auto currentgcp_id_ = ui->gcplistview->getGcpIdByRow(index.row());
			bool exists = blockdata_->GetCurrentATMutual()->ExistsPoint3D(currentgcp_id_);
			size_t cpsize = blockdata_->GetCurrentATMutual()->GetControlPoints().size();
			blockdata_->GetCurrentATMutual()->DeleteGCP(currentgcp_id_);
			gcps_show_.erase(currentgcp_id_);

			ui->gcplistview->removeOneRow(index.row());

			currentgcpindex = ui->gcplistview->currentIndex();
			currentgcp_id_ = ui->gcplistview->getGcpIdByRow(index.row());
			UpdateGcpListView();
			ShowSelectedGcpView();


			emit Sig_ModifiedTrue();

			if (ui->gcplistview->model()->rowCount() <= 0)
			{
				ui->previewlistview->clearData();
				ui->measurementsview->clearData();
				measuringview_->clear();
			}
#endif
		}


		void ControlPointsEditorWin::PrepareGcpListData(bool Is_UpdateCurrentGCP)//用于更新
		{
			
			AI3D::CORE::ControlPoints gcps;
			if (Is_UpdateCurrentGCP)
			{
				auto current_gcp = blockdata_->GetCurrentAT()->GetControlPointsMutual()[currentgcp_id_] ;
				gcps.ADDPoint(current_gcp);
			}
			else
			{
				gcps.GetPointsMutual() = blockdata_->GetCurrentAT()->GetControlPoints();
			}
			gcps.TransformPoints(current_srs_.definition);


			//1:获取基本的数值信息；如果是追加的怎么处理@liyue，应该是信息按某种排序存入
			for (auto& it : gcps.GetPoints())
			{
		
				
				
				AI3D::CORE::ControlPoint gcp = it.second;
				gcp_toshow_s show;
				show.id_ = gcp.GetId();
				show.name_ = gcp.GetName();
				if (gcp.GetType() & (int)GCP_CONTROL_HV)
				{
					show.type_ = sv_type_e::SURVEYSHOW_GCP;
				}
				else if (gcp.GetType() & (int)GCP_CHECK_HV)
				{
					show.type_ = sv_type_e::SURVEYSHOW_GCPCHECK;
				}
				std::pair<point3D_t, sv_type_e> finder = (std::make_pair(it.first, show.type_));
				if (!localid_to_globalid_map_.count(finder))
				{
					continue;
				}
				auto globalid = localid_to_globalid_map_.at(finder);


				show.photos_ = gcp.GetObjectPoint().GetTrack().Length();

				//判断坐标转换是否成功
				bool bconvertfailed = false;
				if (fabs(gcp.GetGivenXYZ().y()) > INVALIDY)
				{
					bconvertfailed = true;
				}

				show.rms_pix_ = gcp.GetObjectPointMutual().GetPixelRMS();
				show.color_ = 3;
				if (!(show.rms_pix_ == kInvalidError || show.rms_pix_ == -DBL_MAX))
				{
					if (show.rms_pix_ <= RMSGOOD)
					{
						show.color_ = 0;
					}
					else if (show.rms_pix_ >= RMSBAD)
					{
						show.color_ = 2;
					}
					else 
					{
						show.color_ = 1;
					}
				}

				show.rms_dis_ = gcp.GetObjectPointMutual().GetDistRMS();
				//需要重新计算
				//当前坐标系界面选择的坐标
				

				show.error_3d_ = gcp.Get3DError();
				
				if (!bconvertfailed)
				{
					show.xyz_ = gcp.GetGivenXYZ();
					show.esitmated_xyz_ = gcp.GetEstimatedXYZ();
					bool bvalidxyz = (show.xyz_.x() != -DBL_MAX);
					bool bvalidetimatedxyz = (show.esitmated_xyz_.x() != -DBL_MAX);
					if (bvalidxyz && bvalidetimatedxyz)
					{
						show.error_3d_z_ = gcp.GetZ3DError();// show.esitmated_xyz_.z() - show.xyz_.z();
						show.error_3d_xy_ = gcp.GetXY3DError();// std::sqrt(show.error_3d_ * show.error_3d_ - show.error_3d_z_ * show.error_3d_z_);
						//if (current_srs_.type == GEOGRAPHIC)
						if (!AI3D::CORE::CoordinateTransformer::IsSame(current_srs_.definition, BASESRS))

						{
							show.error_3d_z_ = gcp.GetEstimatedXYZ().z() - gcp.GetGivenXYZ().z();
							show.error_3d_xy_ = std::sqrt(show.error_3d_ * show.error_3d_ - show.error_3d_z_ * show.error_3d_z_);
						}
					}
				}
				auto idinfo = std::make_pair(it.first, show.type_);
				auto showid = localid_to_globalid_map_.at(idinfo);
				gcps_show_[globalid] = show;
			}
			
		}
	
		void ControlPointsEditorWin::UpdateGcpListViewBottonStatus()
		{
			UpdateLabelRecoder();
		}
			
		void ControlPointsEditorWin::ShowGcpListData()
		{
			
			if (gcps_show_.empty())
			{
				
				InitBlankGcpListView();
				
				return;
			}
			
			
			// 暂时关闭Gcp ListView刷新显示能力.
			ui->gcplistview->setUpdatesEnabled(false);

			ui->gcplistview->clearData();

			for (auto it = gcps_show_.rbegin();it != gcps_show_.rend();it++)
			{
				gcp_list_item_st gcpListItem;

				auto gcp = it->second;
							
				gcpListItem.color_ = gcp.color_;

				

				gcpListItem.ControlpointsID = it->first;
				gcpListItem.name_ = str2qstr(gcp.name_);
				gcpListItem.photos_ = gcp.photos_; 

				sv_type_e type = gcp.type_;
				/// note:
				if (type == sv_type_e::SURVEYSHOW_GCP)
				{

					gcpListItem.category_ = "Control point";
				}
				else if (type == sv_type_e::SURVEYSHOW_GCPCHECK)
				{

					gcpListItem.category_ = "Check point";
				}
				else if (type == sv_type_e::SURVEYSHOW_USERPOINT)
				{

					gcpListItem.category_ = "User Tiepoint";
				}
				else
				{

					gcpListItem.category_ = "Control point";
				}
				
				int precision_xy_num = LONG_ERROR_PRECISION;
				int precision_z_num = OBJECT_ERROR_PRECISION;
				if (current_srs_.type != GEOGRAPHIC)
				{
					precision_xy_num = OBJECT_ERROR_PRECISION;
				}
				if (gcp.xyz_.x() != -DBL_MAX)
				{
					Eigen::Vector3d dstgiven3d(gcp.xyz_);

					gcpListItem.given_x_ = dstgiven3d.x();
					gcpListItem.given_y_ = dstgiven3d.y();
					gcpListItem.given_z_ = dstgiven3d.z();

					gcpListItem.str_given_x_ = QString::number(dstgiven3d.x(), 'f', precision_xy_num);
					gcpListItem.str_given_y_ = QString::number(dstgiven3d.y(), 'f', precision_xy_num);
					gcpListItem.str_given_z_ = QString::number(dstgiven3d.z(), 'f', precision_z_num);
				}
				if (gcp.esitmated_xyz_.x() != -DBL_MAX)
				{
					{
						Eigen::Vector3d dstestimate3d(gcp.esitmated_xyz_);


						gcpListItem.esitmated_x_ = dstestimate3d.x();
						gcpListItem.esitmated_y_ = dstestimate3d.y();
						gcpListItem.esitmated_z_ = dstestimate3d.z();

						gcpListItem.str_esitmated_x_ = QString::number(dstestimate3d.x(), 'f', precision_xy_num);
						gcpListItem.str_esitmated_y_ = QString::number(dstestimate3d.y(), 'f', precision_xy_num);
						gcpListItem.str_esitmated_z_ = QString::number(dstestimate3d.z(), 'f', precision_z_num);

					}
				}
				if (!(gcp.rms_pix_ == kInvalidError || gcp.rms_pix_ == -DBL_MAX))
				/*if (gcp.rms_pix_ != kInvalidError)*/
				{

					gcpListItem.rms_pix_ = gcp.rms_pix_;
					gcpListItem.str_rms_pix_ = QString::number(gcp.rms_pix_, 'f', IMAGE_ERROR_PRECISION);
				}
				if (!(gcp.rms_dis_ == kInvalidError || gcp.rms_dis_ == -DBL_MAX))
				/*if (gcp.rms_dis_ != kInvalidError)*/
				{

					gcpListItem.rms_dis_ = gcp.rms_dis_;
					gcpListItem.str_rms_dis_ = QString::number(gcp.rms_dis_, 'f', OBJECT_ERROR_PRECISION);
				}
				if (!(gcp.error_3d_ == kInvalidError || gcp.error_3d_ == -DBL_MAX))
				/*if (gcp.error_3d_ != kInvalidError)*/
				{

					gcpListItem.error_3d_ = gcp.error_3d_;
					gcpListItem.str_error_3d_ = QString::number(gcp.error_3d_, 'f', OBJECT_ERROR_PRECISION);
				}
				if (!(gcp.error_3d_xy_ == kInvalidError || gcp.error_3d_xy_ == -DBL_MAX))
				/*if (gcp.error_3d_xy_ != kInvalidError)*/
				{


					gcpListItem.error_3d_xy_ = gcp.error_3d_xy_;
					gcpListItem.error_3d_z_ = gcp.error_3d_z_;

					gcpListItem.str_error_3d_xy_ = QString::number(gcp.error_3d_xy_, 'f', OBJECT_ERROR_PRECISION);
					gcpListItem.str_error_3d_z_ = QString::number(gcp.error_3d_z_, 'f', OBJECT_ERROR_PRECISION);
				}

				// 向GcpListView 添加一行数据.
				ui->gcplistview->appendRowData(gcpListItem);
	
			}

			// 打开Gcp ListView刷新显示能力,并触发刷新显示.
			ui->gcplistview->setUpdatesEnabled(true);

			//按照名字排序
			//
			
			UpdateGcpListViewBottonStatus();
			
		}	
		void ControlPointsEditorWin::tableviewClick(QModelIndex index)
		{

			///QTime gcpListViewClickTime;
			///gcpListViewClickTime.start();

			Slot_GcpListItem_SingleClicked(index, index);

			iPreviousGcpListViewRow = index.row();

			///PostProcessAfterClickingGcpListView(index);
		}

		// 在GcpListView,高亮显示当前选中行.
		void ControlPointsEditorWin::ShowSelectedGcpView()
		{
			//return;
			int row = 0;
			auto showgcps = gcps_show_;
			
			if (showgcps.size() <= 0)
			{
				return;
			}

			
			ui->gcplistview->selectOneRowByGcpId(currentgcp_id_);

		}
		//==============GCP列表操作相关   ========end======


		//==============缩略图列表操作相关========begin======
		void ControlPointsEditorWin::ShowPreviewHighLight()
		{
			// 高亮显示当前选中状态的缩略图.
			ui->previewlistview->selectOneRowByImageId(currentimage_id_);
		}

		void ControlPointsEditorWin::PreparePreviewListData()
		{
			///QTime preparePreviewListDataTime;
			//?chy 怎么没有下文了
			// 此处代码为测时目的,现不再使用.
			///preparePreviewListDataTime.start();
			std::set<image_t > images_show;
			std::map<image_t, Eigen::Vector2d > estimated_xys;
			estimated_xys.clear();
			images_show.clear();
			images_show_.clear();

			if (currentimage_id_ == kInvalidImageId)
			{
				if (measurement_error_map_.count(currentgcp_id_))
				{
					if (!measurement_error_map_.at(currentgcp_id_).empty())
					{
						
						{
							currentimage_id_ = GetFirstImageId();//chy
						}
						
					}
				}
				else
				{

					return;
				}
			}
			if (preview_show_mode_ == 1)
			{
				if (!blockdata_->GetCurrentATMutual()->HasImages())
				{
					return;
				}
				if (currentgcp_id_ != kInvalidPoint3DId)
				{
					if (!IsCurrentSelectionUserType(currentgcp_id_))
					{
						images_show = blockdata_->GetCurrentATMutual()->GetImagesIdSet();
					}
					else
					{
						//获取guideimage的前后20张
						if (!globalid_to_localid_map_.count(currentgcp_id_))
							return;
						auto userid = globalid_to_localid_map_.at(currentgcp_id_).first;
						auto userpoint = blockdata_->GetCurrentATMutual()->GetUserPoints3DMutual().at(userid);
						image_t imgid = userpoint.image_for_userptguide_;
						if (imgid == kInvalidImageId)
						{//应该抛出异常
							return;
						}

						image_t baseid = imgid;
						//统计出其前后20张

						std::set<image_t> images_set = blockdata_->GetCurrentATMutual()->GetImagesIdSet();

						int iTotalImagesCount = blockdata_->GetCurrentATMutual()->GetNumImages();

						if (iTotalImagesCount > 41)
						{
							int baseIdIndex = 0;
							for (auto& img : images_set)
							{
								if (img == baseid)
								{
									break;
								}

								baseIdIndex++;
							}

							if (baseIdIndex <= 20)
							{
								baseIdIndex = 0;
							}
							else if(iTotalImagesCount - baseIdIndex < 20)
							{
								baseIdIndex = iTotalImagesCount - 41;
							}
							else
							{
								baseIdIndex = baseIdIndex - 20;
							}

							int baseIdx = 0;
							int gotNum = 0;

							for (auto& img : images_set)
							{
								if (baseIdx == baseIdIndex)
								{
									gotNum = 1;
									images_show.insert(img);
								}
								else if(gotNum > 0 && gotNum < 41)
								{
									gotNum++;
									images_show.insert(img);
									if (gotNum == 41)
										break;
								}

								baseIdx++;
							}
						}
						else
							images_show = images_set;					
					}
				}
				else
				{
					images_show = blockdata_->GetCurrentATMutual()->GetImagesIdSet();
				}


				//显示所有;是否需要排序@liyue			
			}
			else
			{
				//int matchnum = 0;
				MatchPhotos(estimated_xys);
				if (preview_show_mode_ == 2)
				{
					//1:匹配影像，并将id以及算出来的值付给,同时赋给markedimage

					if (gcps_matched_[currentgcp_id_].empty())
					{
						MatchPhotos(images_show);
						gcps_matched_[currentgcp_id_] = images_show;
					}
					else
					{
						images_show = gcps_matched_[currentgcp_id_];
					}
					//matchnum = images_show.size();
				}

				if (!measurement_error_map_.at(currentgcp_id_).empty())
				{
					for (auto& it : measurement_error_map_.at(currentgcp_id_))
					{
						images_show.insert(it.first);
					}
				}
			}
			//？chy 啥
			// 旧代码,未使用
			int maxVisiblePreviewItem = ui->previewlistview->getVisibleRow() * ui->previewlistview->getVisibleCol();
			if (maxVisiblePreviewItem <= 0)
				maxVisiblePreviewItem = 30;
			ui->previewlistview->setBlockPath(blockdata_->GetPath());
			std::set<image_t> ids;

			QStringList need2GenPriviewImageList;

			// 是否所有缩略图都已经生成.
			bool bAllPreviewReady = true;

			if (preview_show_mode_ != 1)
			{
				for (auto& it : images_show)
				{
					image_t id = it;
					AI3D::CORE::Image  image = blockdata_->GetCurrentAT()->GetImage(id);
					std::string path = image.GetPath();
					std::string name = image.GetName();
					std::string img_name_ = path + "/" + name;
					QString qimg_name_ = QString::fromStdString(img_name_);
					ids.insert(it);

					// 缩略图文件列表.
					need2GenPriviewImageList.append(qimg_name_);

					if (!blockdata_->GetCurrentATMutual()->GetImageMutual(it).ExistsPreviewImage(block_path_))
					{
						// 有未生成的缩略图,因此不是所有缩略图都已经准备好.
						bAllPreviewReady = false;
					}
				}
			}
			else
			{
				for (auto& it : images_show)
				{
					image_t id = it;
					AI3D::CORE::Image  image = blockdata_->GetCurrentAT()->GetImage(id);
					std::string path = image.GetPath();
					std::string name = image.GetName();
					std::string img_name_ = path + "/" + name;
					QString qimg_name_ = QString::fromStdString(img_name_);
					ids.insert(it);

					// 缩略图文件列表.
					need2GenPriviewImageList.append(qimg_name_);

					if (!blockdata_->GetCurrentATMutual()->GetImageMutual(it).ExistsPreviewImage(block_path_))
					{
						// 有未生成的缩略图,因此不是所有缩略图都已经准备好.
						bAllPreviewReady = false;
					}
				}
			}

			if (need2GenPriviewImageList.size() > 0)
			{
				// 在Gcp PreviewListView 有缩略图需要显示.
				///std::cout << " need2genpriv :" << need2GenPriviewImageList.size() << std::endl;
				if (bAllPreviewReady)
				{
					// 所有缩略图都已经有缩略图生成,所以不需要弹出加载进程对话框提示用户等待GcpPreviewList刷新显示.
					///std::cout << " all preview ready." << std::endl;
					bNeedLoadingPrompt = false;
				}
				else
				{
					// 不是所有缩略图都有缩略图生成,所以需要弹出加载进程对话框提示用户等待GcpPreviewList刷新显示.
					///std::cout << " not all preview ready." << std::endl;
					bNeedLoadingPrompt = true;
				}
			}
			else
			{
				///std::cout << " not need2genpriv :" << need2GenPriviewImageList.size() << std::endl;
				// 在Gcp PreviewList中没有缩略图需要显示.
				bNeedLoadingPrompt = false;
			}

			// 在Gcp PreviewListView 中设置缩略图需要显示的图形列表.
			ui->previewlistview->setImageFileList(need2GenPriviewImageList);

			for (auto& it : images_show)
			{
				image_t id = it;
				img_toshow_s img;
				img.id_ = id;
				AI3D::CORE::Image  image = blockdata_->GetCurrentAT()->GetImage(id);
				std::string path = image.GetPath();
				std::string name = image.GetName();

				img.name_ = path + "/" + name;

				// 获取缩略图图片对应的PhotogroupId,便于缩略图ListView后续可以分组显示.
				group_t grpId = image.GetPhotoGroupID();
				img.groupId = grpId;

				Eigen::Vector2d estimated_xy;
				std::map<image_t, Eigen::Vector2d >::iterator iter;
				iter = estimated_xys.find(id);
				if (iter != estimated_xys.end())
				{
					estimated_xy = iter->second;
				}
				else
				{
					estimated_xy.x() = -DBL_MAX;
					estimated_xy.y() = -DBL_MAX;
				}

				img.estimated_x_ = estimated_xy.x();
				img.estimated_y_ = estimated_xy.y();
				img.width = image.GetWidth();
				img.height = image.GetHeight();

				Eigen::Vector2d xy;
				point3D_t localid = globalid_to_localid_map_.at(currentgcp_id_).first;
				if (IsCurrentSelectionUserType(currentgcp_id_))
				{
					xy = blockdata_->GetCurrentATMutual()->GetImageMutual(id).GetPoints2DUserPt(localid);
				}
				else
				{
					
					xy = blockdata_->GetCurrentATMutual()->GetImageMutual(id).GetPoints2DGCP(localid);
				}
				if (xy.x() != -DBL_MAX)
				{
					img.x_ = xy.x();
					img.y_ = xy.y();
					img.check_ = true;
				}
				else
				{
					img.check_ = false;
				}
				if (measurement_error_map_.at(currentgcp_id_).count(id))
				{
					img.rms_pix_ = measurement_error_map_.at(currentgcp_id_)[id].second.first;
					img.rms_dis_ = measurement_error_map_.at(currentgcp_id_)[id].second.second;
					if (img.rms_pix_ >= 0)
					{
						if (img.rms_pix_ <= RMSGOOD)
						{
							img.color_ = 0;
						}
						else if (img.rms_pix_ >= RMSBAD)
						{
							img.color_ = 2;
						}
						else
						{
							img.color_ = 1;
						}
					}
				}
				else
				{
					img.color_ = 3;
				}
				images_show_[id] = img;
			}
		}

		//?chy---逻辑重点
		void ControlPointsEditorWin::ShowPreviewListData()
		{
			///QTime showPreviewListDataTime;

			// 缩略图显示计时,调试用,现未用,可关.
			///showPreviewListDataTime.start();
			UpdatePreviewListViewBottonStatus();
			if (images_show_.empty())
			{
				InitBlankPreviewListView();
				return;
			}
			if (currentimage_id_ != kInvalidImageId)
			{
			}


			// 遍历当前blockdata_所有PhotoGroup,构建组Id和组Name的PhotoGroup Map对象.
			photoGroups.clear();
			for (auto& it : blockdata_->GetPhotoGroups())
			{
				group_t groupId = it.first;;
				AI3D::CORE::PhotoGroup& group = blockdata_->GetGroup(groupId);
				QString groupName = QString::fromStdString(group.GetName());
				photoGroups.insert(groupId, groupName);
			}

			QMap<QString, QList<gcp_measurement_list_item_st>*>::Iterator iterator;
			QList<gcp_measurement_list_item_st>* grpPreviewListData = nullptr;

			// 缩略图分组对象grpPreviewListData清空
			for (iterator = previewListMap.begin(); iterator != previewListMap.end(); iterator++)
			{
				grpPreviewListData = iterator.value();
				if (grpPreviewListData != nullptr)
					delete grpPreviewListData;
			}

			previewListMap.clear();


			// 暂时关闭previewListView显示刷新能力.
			ui->previewlistview->setUpdatesEnabled(false);

			// 清空缩略图Gcp ListView数据.
			ui->previewlistview->clearData();

			// 遍历将在Gcp PreviewListView中显示的缩略图列表.
			for (auto& it : images_show_)
			{
				//it.second.color_;//chy分颜色显示@liyue
				image_t id = it.first;
				gcp_measurement_list_item_st gcpMeasurementListItem;

				QString imageFullName = str2qstr((it.second.name_));

				// 获取图片的组Id: groupId
				group_t groupId = it.second.groupId;

				grpPreviewListData = nullptr;
				// 获取图片的组名:groupName.
				QString groupName = photoGroups.value(groupId);
				if (!groupName.isEmpty())
				{
					// 按照组名groupName对所有图片进行分组.同样的组名归入一个List列表对象(grpPreviewListData).
					// 根据组名groupName及对应的List对象(grpPreviewListData),构建分组聚合Map对象:previewListMap
					grpPreviewListData = previewListMap.value(groupName, nullptr);
					if (grpPreviewListData == nullptr)
					{
						grpPreviewListData = new QList<gcp_measurement_list_item_st>();
						previewListMap.insert(groupName, grpPreviewListData);

						// 在Gcp PreviewListView中,每一个独立的分组(即同样的组名),在第一项不显示任何缩略图.
						// 在第一项显示的对应分组的组名,因此这是一个特殊的占位Item,该特殊的_photo_name不是具体的图片名字,而是分组的组名.同时,
						// 用该Item的color_设定一个特殊的值:99,表明这是一个分组Item.
						gcp_measurement_list_item_st gcpMeasurementListItemFirst;
						gcpMeasurementListItemFirst._photo_name = groupName;
						gcpMeasurementListItemFirst.color_ = 99;

						grpPreviewListData->append(gcpMeasurementListItemFirst);
					}
				}
				else
				{
					std::cout << " ??? something should be eror ???" << std::endl;
				}

				gcpMeasurementListItem._photo_name = imageFullName;
				gcpMeasurementListItem.ControlpointsImageID = id;

				gcpMeasurementListItem.estimated_x_ = it.second.estimated_x_;
				gcpMeasurementListItem.estimated_y_ = it.second.estimated_y_;
				gcpMeasurementListItem.width = it.second.width;
				gcpMeasurementListItem.height = it.second.height;


				if (it.second.preview_name_ == "")
				{
					std::string name = blockdata_->GetCurrentATMutual()->GetImageMutual(id).GetPriviewFileFullName();
					if (AI3D::CORE::File::ExistsPath(name))
					{
						it.second.preview_name_ = name;
					}
				}

				gcpMeasurementListItem.preview_name_ = str2qstr(it.second.preview_name_);

				gcpMeasurementListItem.color_ = it.second.color_;
				gcpMeasurementListItem.check_ = it.second.check_;

				/// todo:test it later,modify for grouping previewlist thumbnails.

				grpPreviewListData->append(gcpMeasurementListItem);
			}

			// previewListMap: 根据组名groupName及对应的List对象(grpPreviewListData)构建.

			QMapIterator<QString, QList<gcp_measurement_list_item_st>*> iter(previewListMap);
			while (iter.hasNext())
			{
				iter.next();
				QString groupName = iter.key();
				QList<gcp_measurement_list_item_st>* grpData = iter.value();
				QList<gcp_measurement_list_item_st> grpDataCopy;
				// 对Gcp PreviewList中每一个基于唯一的GroupName进行分组的组内显示,根据compareLessThan比较器,
				// 对组内图片根据预测点与图片中心点的距离,进行升序排序.
				qSort(grpData->begin(), grpData->end(), gcp_measurement_list_item_st::compareLessThan);
			}

			ui->previewlistview->appendData(previewListMap); // groupData.

			// 打开Gcp PreviewListView 刷新显示开关.
			ui->previewlistview->setUpdatesEnabled(true);

			// 设置bAllowUpdate为true,表明所有缩略图图片已经append完成,后续可以Update,目前实际用途已不大.
			ui->previewlistview->startGenPreviewFileWatchTimer();
		}
		//==============缩略图列表操作相关========end======
		QColor ControlPointsEditorWin::GetColor(int color)
		{
			QColor indexcolor;
			switch (color)
			{
			case 2:
				indexcolor.setRgb(RED_COLOR_R, RED_COLOR_G, RED_COLOR_B);
				break;
			case 1:
				indexcolor.setRgb(YELLOW_COLOR_R, YELLOW_COLOR_G, YELLOW_COLOR_B);
				break;
			case 0:
				indexcolor.setRgb(GREEN_COLOR_R, GREEN_COLOR_G, GREEN_COLOR_B);
				break;
			default:
				indexcolor.setRgb(DEFAULT_LIST_COLOR_R, DEFAULT_LIST_COLOR_G, DEFAULT_LIST_COLOR_B);
				break;
			}
			return indexcolor;
		}
		QColor ControlPointsEditorWin::GetSelectColor(int color)
		{
			QColor indexcolor;
			switch (color)
			{
			case 2:
				indexcolor.setRgb(SELECT_RED_COLOR_R, SELECT_RED_COLOR_G, SELECT_RED_COLOR_B);
				break;
			case 1:
				indexcolor.setRgb(SELECT_YELLOW_COLOR_R, SELECT_YELLOW_COLOR_G, SELECT_YELLOW_COLOR_B);
				break;
			case 0:
				indexcolor.setRgb(SELECT_GREEN_COLOR_R, SELECT_GREEN_COLOR_G, SELECT_GREEN_COLOR_B);
				break;
			default:
				break;
			}
			return indexcolor;
		}
	
		image_t ControlPointsEditorWin::GetFirstImageId()
		{
			if (measurement_error_map_.count(currentgcp_id_)&&!measurement_error_map_.at(currentgcp_id_).empty())
			{
				if (!IsCurrentSelectionUserType(currentgcp_id_))
				{				
					return measurement_error_map_.at(currentgcp_id_).begin()->first;
				}
				else
				{				
					//按设计这个地方应该是往前推20个
					point3D_t userptid = globalid_to_localid_map_.at(currentgcp_id_).first;
					auto userpoint = blockdata_->GetCurrentATMutual()->GetUserPoints3DMutual().at(userptid);

					///return userpoint.image_for_userptguide_;
					if (userpoint.image_for_userptguide_ == kInvalidImageId)
						return userpoint.image_for_userptguide_;					

					image_t first_image_id = kInvalidImageId;

					image_t baseid = userpoint.image_for_userptguide_;

					std::set<image_t> images_set = blockdata_->GetCurrentATMutual()->GetImagesIdSet();

					int iTotalImagesCount = blockdata_->GetCurrentATMutual()->GetNumImages();

					if (iTotalImagesCount > 41)
					{
						int baseIdIndex = 0;
						for (auto& img : images_set)
						{
							if (img == baseid)
							{
								break;
							}

							baseIdIndex++;
						}

						if (baseIdIndex <= 20)
						{
							baseIdIndex = 0;
						}
						else if (iTotalImagesCount - baseIdIndex < 20)
						{
							baseIdIndex = iTotalImagesCount - 41;
						}
						else
						{
							baseIdIndex = baseIdIndex - 20;
						}

						int baseIdx = 0;
						int gotNum = 0;

						for (auto& img : images_set)
						{
							if (baseIdx == baseIdIndex)
							{
								gotNum = 1;
								first_image_id = img;
								break;
							}

							baseIdx++;
						}
					}
					else
						first_image_id = *(images_set.begin());

					///if (userpoint.image_for_userptguide_ >= 20)
					///	first_image_id = userpoint.image_for_userptguide_ - 20 + 1;

					return first_image_id;
				}
				
			}
			return kInvalidImageId;
		}
	
		void ControlPointsEditorWin::UpdateMeasuringViewBottonStatus()
		{
			if (bshow_epipolarline_)
				ui->btn_epipolarline->setChecked(true);
			else
				ui->btn_epipolarline->setChecked(false);
		}

		point3D_t ControlPointsEditorWin::GetSelectGcpId()//获取GCP的iD，用于对GcplistView单机双击操作时
		{
			//gui上解析gcp信息赋值给 gcp_id
			//point3D_t gcp_id = ui->gcplistview->currentIndex().data(CustomRole::CRControlpointsID).toInt();
			// change it based on new ui.
			QModelIndex current = currentgcpindex;
			point3D_t gcp_id = kInvalidPoint3DId;
			if (current.isValid())
			{
				gcp_id = ui->gcplistview->getGcpIdByRow(current.row());
			}
			return gcp_id;
		}

		/*point3D_t ControlPointsEditorWin::GetCurrentGcpId()
		{
			return currentgcp_id_;
		}*/

		void ControlPointsEditorWin::UpdatePreviewListViewBottonStatus()
		{		
			if (preview_show_mode_ == 1)
			{		
				/// todo:comment it temporarily until furthur solving the relevant crash bug.
				ui->btn_AllPho->setChecked(true);
				ui->btn_MatchPho->setChecked(false);
				ui->btn_MarkPho->setChecked(false);
			}
			if (preview_show_mode_ == 2)
			{

				ui->btn_AllPho->setChecked(false);
				ui->btn_MatchPho->setChecked(true);
				ui->btn_MarkPho->setChecked(false);

			}
			if (preview_show_mode_ == 3)
			{
				ui->btn_AllPho->setChecked(false);
				ui->btn_MatchPho->setChecked(false);
				ui->btn_MarkPho->setChecked(true);
			}
					
		}


		void ControlPointsEditorWin::InitGcpListView()//原始状态；
		{
			//设置GCP表头信息
			InitGcpListViewHeader();
			//设置表头的按钮信息
			InitGcpListViewBottonStatus();
			//设置表的编辑状态
			MakeGcplistViewUneditable();					
		}

		void ControlPointsEditorWin::InitMeasuringViewHeader()
		{
			measuringview_ = new control_point_GUI::GraphicsView(this);
			ui->measuringview->layout()->addWidget(measuringview_);
		}
		
		void ControlPointsEditorWin::InitPreviewListView()//
		{
			//是否需要考虑保存切换之前的状态，如果不需要就是InitBlankPreviewListView();
			
			InitPreviewListViewHeader();
			InitPhotosButtons();

		}
		void ControlPointsEditorWin::InitMeasuringView()
		{
			//是否需要考虑保存切换之前的状态，如果不需要就是InitBlankMeasuringView();
			InitMeasuringViewHeader();		
		}	

		void ControlPointsEditorWin::InitBlankGcpListView()
		{			
			ui->gcplistview->clearData();
			currentgcpindex = QModelIndex();
			gcps_show_.clear();
			currentgcp_id_ = kInvalidPoint3DId;
			currentimage_id_ = kInvalidImageId;
			InitGcpListViewBottonStatus();
		}		
		
		void ControlPointsEditorWin::UpdateGcpSelected()
		{			
			QTime updateGcpSelectedTime;
			updateGcpSelectedTime.start();

			if (gcps_show_.count(currentgcp_id_))
			{
				ShowSelectedGcpView();

				
			}

			UpdatePreviewListView();
			
			UpdateMeasurementsView();
			//?chy 啥意思
			// 此处是旧代码,应该没啥用处了.
			isexectableview = false;
			//?chy 啥意思
			// 将MeasuringView刷新显示放在在200毫秒后的下一轮事件循环中进行刷新,缩短当前调用的时间占用.
			QTimer::singleShot(200, this, &ControlPointsEditorWin::Slot_RefreshMeasuringView);
			
		}

		void ControlPointsEditorWin::Slot_RefreshMeasuringView()
		{
			
			UpdateMeasuringView();
			
		}
		void  ControlPointsEditorWin::InitSurveyData()
		{
			if (!blockdata_->HasSurveyPoints())
			{
				return;
			}

			//计算相机的无畸变边界参数和fov
			for (auto& cam : blockdata_->GetCurrentATMutual()->GetCamerasMutual())
			{
				if (cam.second.GetUndistortBorder()[0] == kInvalideNum)
				{
					//无畸变边界
					double left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y;
					cam.second.GetValidUndistortBorder(left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y);
					double undistortedborder[8] = { left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y };
					cam.second.SetUndistortBorder(undistortedborder);
				}
				if (cam.second.GetFov() == kInvalideNum)
				{
					//fov1
					Eigen::Vector2f diagonal(cam.second.GetWidth(), cam.second.GetHeight());
					double fov = diagonal.norm() / (cam.second.GetMeanFocalLength() * 2);
					fov = atan(fov) * 180 / M_PI;
					cam.second.SetFov(fov);
				}
			}
			measurement_error_map_.clear();
			point3D_t index = 0;
			std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_measurement_error_map, user_measurement_error_map;
			if (blockdata_->HasControlPoints())
			{
				blockdata_->GetCurrentATMutual()->UpdataGCPGlobalErrorInfo(gcp_measurement_error_map, btopredict_);
				//赋值给measurement_error_map_;
				
				for (auto& iter : gcp_measurement_error_map)
				{
					if (!blockdata_->GetCurrentATMutual()->GetControlPoints().count(iter.first))
					{
						LOGE(" Has error,but point not exists.");
						continue;
					}
					sv_type_e svtype;					

					auto gcptype = blockdata_->GetCurrentATMutual()->GetControlPoints().at(iter.first).GetType();
					if (gcptype & (int)gpt_e::GCP_CONTROL_HV)
					{
						svtype = sv_type_e::SURVEYSHOW_GCP;
					}
					else if(gcptype & (int)gpt_e::GCP_CHECK_HV)
					{
						svtype = sv_type_e::SURVEYSHOW_GCPCHECK;
					}
					else
					{
						LOGE(" Has error,but point type is wrong.");
						continue;
					}
					std::pair< point3D_t, sv_type_e > info = std::make_pair(iter.first, svtype);
					globalid_to_localid_map_[index] = info;
					measurement_error_map_[index] = iter.second;
					localid_to_globalid_map_[info] = index;
					index++;
				}
			}
			if (blockdata_->HasUserTiePoints())
			{
				blockdata_->GetCurrentATMutual()->UpdataUserTiepointsGlobalErrorInfo(user_measurement_error_map, btopredict_);

				for (auto& iter : user_measurement_error_map)
				{
					std::pair< point3D_t, sv_type_e > info = std::make_pair(iter.first, sv_type_e::SURVEYSHOW_USERPOINT);
					globalid_to_localid_map_[index] = info;
					measurement_error_map_[index] = iter.second;
					localid_to_globalid_map_[info] = index;
					index++;
				}
			}
			UpdateSurveyListView();
		}

		void ControlPointsEditorWin::ShowSurveyListData()
		{
			ShowGcpListData();
		}

		void  ControlPointsEditorWin::PrepareUserPointListData(bool Is_UpdateCurrentData)
		{
			AI3D::CORE::ControlPoints gcps;
			auto userpoints = blockdata_->GetCurrentAT()->GetUserPoints3D();
			if (Is_UpdateCurrentData)
			{
				auto currentgcp_id = globalid_to_localid_map_.at(currentgcp_id_).first;
				auto current_point = userpoints.at(currentgcp_id);
				AI3D::CORE::ControlPoint gcp;
				gcp.SetId(current_point.GetId());
				gcp.SetEstimatedXYZ(current_point.GetXYZMutual());
				gcp.SetObjectPoint(current_point);
				gcp.SetName(current_point.GetName());
				auto srs_block = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(blockdata_->GetCurrentAT()->GetLocalSrs());
				gcp.SetSrs(srs_block);
				gcps.ADDPoint(gcp);
			}
			else
			{
				for (auto& iter : userpoints)
				{
					AI3D::CORE::ControlPoint gcp;
					gcp.SetId(iter.second.GetId());
					gcp.SetObjectPoint(iter.second);
					gcp.SetName(iter.second.GetName());
					auto srs_block = AI3D::CORE::CoordinateDescriptor::GetSRSFromDefinition(blockdata_->GetCurrentAT()->GetLocalSrs());
					gcp.SetSrs(srs_block);
					gcp.SetEstimatedXYZ(iter.second.GetEstimatedXYZMutual());
					gcps.ADDPoint(gcp);
				}
				
			}
			gcps.TransformPoints(current_srs_.definition);


			//
			for (auto& it : gcps.GetPoints())
			{

				std::pair<point3D_t, sv_type_e> finder = (std::make_pair(it.first, sv_type_e::SURVEYSHOW_USERPOINT));
				if (!localid_to_globalid_map_.count(finder))
				{
					continue;
				}
				auto globalid = localid_to_globalid_map_.at(finder);
				AI3D::CORE::ControlPoint gcp = it.second;
				gcp_toshow_s show;
				show.id_ = gcp.GetId();
				show.name_ = gcp.GetName();
				
				show.type_ = sv_type_e::SURVEYSHOW_USERPOINT;
				
				show.photos_ = gcp.GetObjectPoint().GetTrack().Length();

				//判断坐标转换是否成功
				/*bool bconvertfailed = false;
				if (fabs(gcp.GetGivenXYZ().y()) > INVALIDY)
				{
					bconvertfailed = true;
				}*/

				show.rms_pix_ = gcp.GetObjectPointMutual().GetPixelRMS();
				show.color_ = 3;
				if (!(show.rms_pix_ == kInvalidError || show.rms_pix_ == -DBL_MAX))
				{
					if (show.rms_pix_ <= RMSGOOD)
					{
						show.color_ = 0;
					}
					else if (show.rms_pix_ >= RMSBAD)
					{
						show.color_ = 2;
					}
					else
					{
						show.color_ = 1;
					}
				}

				show.rms_dis_ = gcp.GetObjectPointMutual().GetDistRMS();
				//需要重新计算
				//当前坐标系界面选择的坐标


				//show.error_3d_ = gcp.Get3DError();

				//if (!bconvertfailed)
				//{
				//	show.xyz_ = gcp.GetGivenXYZ();
					show.esitmated_xyz_ = gcp.GetEstimatedXYZ();
				//	bool bvalidxyz = (show.xyz_.x() != -DBL_MAX);
				//	bool bvalidetimatedxyz = (show.esitmated_xyz_.x() != -DBL_MAX);
				//	if (bvalidxyz && bvalidetimatedxyz)
				//	{
				//		show.error_3d_z_ = gcp.GetZ3DError();// show.esitmated_xyz_.z() - show.xyz_.z();
				//		show.error_3d_xy_ = gcp.GetXY3DError();// std::sqrt(show.error_3d_ * show.error_3d_ - show.error_3d_z_ * show.error_3d_z_);
				//		//if (current_srs_.type == GEOGRAPHIC)
				//		if (!AI3D::CORE::CoordinateTransformer::IsSame(current_srs_.definition, BASESRS))

				//		{
				//			show.error_3d_z_ = gcp.GetEstimatedXYZ().z() - gcp.GetGivenXYZ().z();
				//			show.error_3d_xy_ = std::sqrt(show.error_3d_ * show.error_3d_ - show.error_3d_z_ * show.error_3d_z_);
				//		}
				//	}
				//}
//获取全局id
					
					
					
				gcps_show_[globalid] = show;
			}
		}

		void  ControlPointsEditorWin::PrepareSurveyListData(bool Is_UpdateCurrentData )
		{
			PrepareGcpListData(Is_UpdateCurrentData);
			PrepareUserPointListData(Is_UpdateCurrentData);
		}
		void ControlPointsEditorWin::UpdateSurveyListView(bool Is_UpdateCurrentSurveyData)
		{
			isexectableview = true;
			PrepareSurveyListData(Is_UpdateCurrentSurveyData);//准备数据，除值之外还需要红框绿框等
			ShowSurveyListData();
			isexectableview = false;
		}
		void ControlPointsEditorWin::InitGcpData()
		{
			InitSurveyData();
			//if (blockdata_->GetCurrentATMutual()->HasControlPoints())
			//{						
			//	blockdata_->GetCurrentATMutual()->UpdataGCPGlobalErrorInfo(measurement_error_map_, btopredict_);							
			//	//计算相机的无畸变边界参数和fov
			//	for (auto& cam : blockdata_->GetCurrentATMutual()->GetCamerasMutual())
			//	{
			//		if (cam.second.GetUndistortBorder()[0] == kInvalideNum)
			//		{
			//			//无畸变边界
			//			double left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y;
			//			cam.second.GetValidUndistortBorder(left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y);
			//			double undistortedborder[8] = { left_min_x, left_max_x, right_min_x, right_max_x, top_min_y, top_max_y, bottom_min_y, bottom_max_y };
			//			cam.second.SetUndistortBorder(undistortedborder);
			//		}
			//		if (cam.second.GetFov() == kInvalideNum)
			//		{
			//			//fov1
			//			Eigen::Vector2f diagonal(cam.second.GetWidth(), cam.second.GetHeight());
			//			double fov = diagonal.norm() / (cam.second.GetMeanFocalLength() * 2);
			//			fov = atan(fov) * 180 / M_PI;
			//			cam.second.SetFov(fov);
			//		}
			//	}
			//	UpdateGcpListView();
			//}
			
		}
		
		void ControlPointsEditorWin::UpdateGcpListView(bool Is_UpdateCurrentGCP)
		{	
			isexectableview = true;
			PrepareGcpListData(Is_UpdateCurrentGCP);//准备数据，除值之外还需要红框绿框等
			ShowGcpListData();			
			isexectableview = false;
		}
		
		void ControlPointsEditorWin::ShowSelectedMeasurementView()
		{
			//measurement_highlight_ = kInvalidImageId;
			if (measurement_error_map_.at(currentgcp_id_).count(currentimage_id_))
			{
				measurement_highlight_ = currentimage_id_;
			}


			if (measurement_highlight_ != kInvalidImageId)
			{
				ui->measurementsview->selectOneRowByImageId(measurement_highlight_);
				ShowPreviewHighLight();
			}

		}
		void ControlPointsEditorWin::UpdatePreviewListView()
		{

			PreparePreviewListData();//准备数据，除值之外还需要红框绿框等
			ShowPreviewListData();

		}
		void ControlPointsEditorWin::UpdateMeasuringView(bool isImage)
		{
			
			if (!bMeasuringClicked)
			{
				PrepareMeasuringData();//准备数据，除值之外还需要红框绿框等

				ShowMeasuringData(isImage);
			}
			
		}

	
		void ControlPointsEditorWin::UpdateMeasurementsView()
		{

			PrepareMeasurementsData();//准备数据，除值之外还需要红框绿框等
			ShowMeasurementsData();

		}
		void ControlPointsEditorWin::PrepareMeasuringData()
		{
			//1：获取当前影像，
			if (currentimage_id_ == kInvalidImageId)
			{
				return;
			}

			
			//2：获取预测点信息
			auto& gcp = blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_];
			
			AI3D::CORE::Image image = blockdata_->GetCurrentATMutual()->GetImage(currentimage_id_);			
			blockdata_->GetCurrentATMutual()->PredictGCPMeasurement(currentgcp_id_, currentimage_id_, candidate_xy_show_,true, btopredict_);
			if (blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_].GetObjectPointMutual().HasElement())
			{
				//3：获取曾经是否刺过信息
				measured_xy_show_ = image.GetPoints2DGCP(currentgcp_id_);
			}
			
			//4：获取核线信息
			epipolarlines_show_.clear();
			
			blockdata_->GetCurrentATMutual()->GetEpipolarLines(currentimage_id_, currentgcp_id_, epipolarlines_show_);
			
		}

		

		void ControlPointsEditorWin::ShowMeasuringData(bool isImage)
		{	
			
			if (currentimage_id_ == kInvalidImageId)
			{
			
				InitBlankMeasuringView();
			
				return;
			}
			
///			float scale = 1.0;
			//获取基本信息
///			if (currentimage_id_ == old_image_scale_ratio_.first)
///			{
///				scale = old_image_scale_ratio_.second;
///			}

			
			AI3D::CORE::Image  image = blockdata_->GetCurrentAT()->GetImage(currentimage_id_);
			std::string path = image.GetPath();
			std::string name = image.GetName();
			std::string imageName = path + "/" + name;

			

			measuringview_->setUpdatesEnabled(false);



			
				measuringview_->clear();

			

			if (AI3D::CORE::File::IsFileExistent(imageName))
			{
				/// dump gcpview info.
				//LOGI("dump gcpview info.");
				measuringview_->dumpInfo();

					measuringview_->addImage(QString::fromStdString(imageName), currentimage_id_, 0, 0, true);

					//LOGI("dump gcpview info.");
					measuringview_->dumpInfo();

					/// dump gcpview info.
						measuringview_->setScale(scale_);
						/// dump gcpview info.
					//std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " scale:" << scale_ << std::endl;
				
																		
			}
			else
			{
				measuringview_->addImage(QString::fromStdString(imageName), currentimage_id_, 0, 0, true);
				measuringview_->setScale(scale_);
				
				
				measuringview_->setUpdatesEnabled(true);
			
				return;//不出现预测点绘制
			}
	
				//有预测点显示圆圈；
			if (candidate_xy_show_.x() != -DBL_MAX)
			{
				//std::cout << __FILE__ << " " << __FUNCTION__ << " "
				measuringview_->addCircleCross({ candidate_xy_show_.x(), candidate_xy_show_.y() });
				//LOGI("addCircleCross:" + std::to_string(candidate_xy_show_.x()) + " / " + std::to_string(candidate_xy_show_.y()));
				//LOGI("dump gcpview info.");
				measuringview_->dumpInfo();

				/// dump gcpview info.
			}

			//有刺过显示十字丝
			if (measured_xy_show_.x() != -DBL_MAX /* && !bMeasuringClicked*/)
			{				
				measuringview_->addNode({ measured_xy_show_.x(),measured_xy_show_ .y()});
				//LOGI("addNode:" + std::to_string(measured_xy_show_.x()) + " / " + std::to_string(measured_xy_show_.y()));
				//LOGI("dump gcpview info.");
				measuringview_->dumpInfo();

				/// dump gcpview info.
			}

			//有核线显示核线
			if (bshow_epipolarline_)
			{
				if (!epipolarlines_show_.empty())
				{
					for (auto it : epipolarlines_show_)
					{
					
						measuringview_->AddLine({ it.second.first.x(),it.second.first.y() },
								{ it.second.second.x(),it.second.second.y() });
			
					//	LOGI("addLine: from " + std::to_string(it.second.first.x()) + " / " + std::to_string(it.second.first.y()) 
					//		+std::to_string(it.second.second.x()) + " / " + std::to_string(it.second.second.y()));
					}
					/// dump gcpview info.
					//LOGI("dump gcpview info.");
					measuringview_->dumpInfo();

				}
			}

			
			{
				if (measured_xy_show_.x() != -DBL_MAX)
				{
					
					// 若已有刺点,按刺点居中显示Gcp大图.
					//LOGI("before centerAt measured_xy:" + std::to_string(measured_xy_show_.x()) + " / " + std::to_string(measured_xy_show_.y()));

					measuringview_->centerAt({ measured_xy_show_.x(),measured_xy_show_.y() });

					//LOGI("after centerAt mearsured_xy:" + std::to_string(measured_xy_show_.x()) + " / " + std::to_string(measured_xy_show_.y()));
					//LOGI("dump gcpview info.");
					measuringview_->dumpInfo();

					///
					/// dump gcpview info.
				}
				else if (candidate_xy_show_.x() != -DBL_MAX)
				{
					
					// 若无刺点,当有预测点,按预测点居中显示Gcp大图.
					//LOGI("before centerAt candidate_xy:" + std::to_string(candidate_xy_show_.x()) + " / " + std::to_string(candidate_xy_show_.y()));

					measuringview_->centerAt({ candidate_xy_show_.x(), candidate_xy_show_.y() });

					//LOGI("after centerAt candidate_xy:" + std::to_string(candidate_xy_show_.x()) + " / " + std::to_string(candidate_xy_show_.y()));
					//LOGI("dump gcpview info.");
					measuringview_->dumpInfo();

					/// dump gcpview info.
				}
			}
			
			measuringview_->setUpdatesEnabled(true);

			//LOGI("dump gcpview info.");
			measuringview_->dumpInfo();

			UpdateMeasuringViewBottonStatus();
			
		}

		void ControlPointsEditorWin::PrepareMeasurementsData()
		{

			InitBlankMeasurementsView();

		}
		void ControlPointsEditorWin::ShowMeasurementsData()
		{
			

			if (measurement_error_map_.at(currentgcp_id_).empty())
			{
				//设置删除不可用
				InitBlankMeasurementsView();
				return;
			}


			ui->measurementsview->setUpdatesEnabled(false);

			
			int rownum = 0;
			for (auto& it : measurement_error_map_.at(currentgcp_id_))
			{
				int c = images_show_[it.first].color_;//分颜色显示chy@liyue
				img_toshow_s showitem = images_show_[it.first];
				{
					gcp_measurement_list_item_st gcpMeasurementListItem;

					///QColor color = GetColor(showitem.color_);
					gcpMeasurementListItem.color_ = showitem.color_;
					
					gcpMeasurementListItem.ControlpointsImageID = showitem.id_;
					gcpMeasurementListItem._photo_name = str2qstr(showitem.name_);
					gcpMeasurementListItem.str_x_ = QString::number(showitem.x_, 'f', MEASUREMENT_PRECISION);
					gcpMeasurementListItem.str_y_ = QString::number(showitem.y_, 'f', MEASUREMENT_PRECISION);
					//无空三刺点情况
					if (!(showitem.rms_pix_ == kInvalidError || showitem.rms_pix_ == -DBL_MAX))
					//if (!(showitem.rms_pix_ == -DBL_MAX /*&& showitem.rms_dis_ == -DBL_MAX*/))
					{								
						gcpMeasurementListItem.str_rms_pix_ = QString::number(showitem.rms_pix_, 'f', IMAGE_ERROR_PRECISION);
						/*gcpMeasurementListItem.str_rms_dis_ = QString::number(showitem.rms_dis_, 'f', OBJECT_ERROR_PRECISION);*/
					}

					ui->measurementsview->appendRowData(gcpMeasurementListItem);

				}
			
				rownum++;

				ui->measurementsview->setUpdatesEnabled(true);
			}
			
			ShowSelectedMeasurementView();
			
		}	
	
		void ControlPointsEditorWin::RecoverMeasurementsView()
		{


			PrepareMeasurementsData();//准备数据，除值之外还需要红框绿框等


			if (measurement_error_map_.at(currentgcp_id_).empty())
			{
				
				InitBlankMeasurementsView();
				return;
			}
			//显示表格信息 + 	//颜色显示

			ui->measurementsview->setUpdatesEnabled(false);

			for (auto& it : measurement_error_map_.at(currentgcp_id_))
			{
				int c = images_show_[it.first].color_;//分颜色显示chy@liyue
				img_toshow_s showitem = images_show_[it.first];

				gcp_measurement_list_item_st gcpMeasurementListItem;
				gcpMeasurementListItem.color_ = showitem.color_;

				{

					
					gcpMeasurementListItem.ControlpointsImageID = showitem.id_;
					gcpMeasurementListItem._photo_name = str2qstr(showitem.name_);
					gcpMeasurementListItem.str_x_ = QString::number(showitem.x_, 'f', MEASUREMENT_PRECISION);
					gcpMeasurementListItem.str_y_ = QString::number(showitem.y_, 'f', MEASUREMENT_PRECISION);

					//无空三刺点情况
					/*if (showitem.rms_pix_ == -DBL_MAX && showitem.rms_dis_ == -DBL_MAX)
					{
					
					}
					else*/
					{
						
						if (!(showitem.rms_pix_ == kInvalidError || showitem.rms_pix_ == -DBL_MAX))
						{
							gcpMeasurementListItem.str_rms_pix_ = QString::number(showitem.rms_pix_, 'f', IMAGE_ERROR_PRECISION);
						}
						//gcpMeasurementListItem.str_rms_dis_ = QString::number(showitem.rms_dis_, 'f', OBJECT_ERROR_PRECISION);
					}
				}

				ui->measurementsview->appendRowData(gcpMeasurementListItem);

			}

			// 加该语句,根据新数据刷新measurementview.若不加,可能该组件相关界面不会反应对应数据.
			// 不加该语句,MeasurementsView可能每加一条记录,可能会频繁刷新显示.该语句是控制增加大量数据后的实际刷新时机.
			ui->measurementsview->setUpdatesEnabled(true);//？chy短时间内是否禁用更新不加会咋样


		}

		void ControlPointsEditorWin::SetCurrentGcpId(point3D_t gcp_id)
		{
			
			currentgcp_id_ = gcp_id;
		}
		

		void ControlPointsEditorWin::SetCurrentImageId(image_t img_id)
		{
			currentimage_id_ = img_id;
		}
		image_t ControlPointsEditorWin::GetCurrentImageId()
		{
			return currentimage_id_;
		}
	
		void ControlPointsEditorWin::UpdateLabelRecoder()
		{
			//获取控制点数量
			//获取刺点图像数量
			int controlnum = blockdata_->GetCurrentATMutual()->GetNumControlPoints();
			int gcpvalidnum = blockdata_->GetCurrentATMutual()->GetNumValidControlPoints();
			int usernum = blockdata_->GetCurrentATMutual()->GetNumUserPoints();
			int uservalidnum = blockdata_->GetCurrentATMutual()->GetNumValidUserPoints();
			
			std::string strgcp = std::to_string(gcpvalidnum) + "/" + std::to_string(controlnum);
			std::string struser = std::to_string(usernum) + "/" + std::to_string(uservalidnum);
			std::string str;
			if(1)
			{
				str = strgcp + " " + struser;
			}
			else
			{
				str = strgcp;
			}
			
			QString recorddata = QString::fromStdString(str);// QString::number(splitnum) + "/" + QString::number(surveynum) + ;
			ui->toolBtn_gcpstatis->setText(recorddata);
		}

		
		//GcpListView相关操作
		void ControlPointsEditorWin::Slot_GcpListItem_SingleClicked(QModelIndex current, QModelIndex previous)
		{
			AI3D::CORE::Timer time;
			time.Start();
			
			if (!current.isValid() || isexectableview)
			{
				if (!current.isValid())
					currentgcpindex = QModelIndex();
				return;
			}	

			
			currentgcpindex = current;
			point3D_t gcpid = GetSelectGcpId();//获取gcpid

			//该控制点列高亮显示
			//HighLightGcp(id);
			//第一步相当于单击该点，并刷新所有显示；
			
			if(current.row() != iPreviousGcpListViewRow)
			{
				if (gcpid != currentgcp_id_ || update_ == true)//是否需要該判斷@liyue
				{
					/// todo:comment it temporarily until furthur solving relevant crash bug.
					///ui->btn_AllPho->setEnabled(true);
					
					bool bInsideUserTiePoint = PostProcessAfterClickingGcpListView(current);

					ui->btn_MatchPho->setEnabled(true);
					ui->btn_MarkPho->setEnabled(true);

					images_show_.clear();
					SetCurrentImageId(kInvalidImageId);//切换了所以当前imageid需要重新赋值
					SetCurrentGcpId(gcpid);
					//measurement_error_map_.clear();

					if(bInsideUserTiePoint)
						preview_show_mode_ = 1;
					else
						preview_show_mode_ = 2;					

					time.Restart();


					UpdateGcpSelected();					

					LOGI(AI3D::CORE::String::StringPrintf("Update single GCP spends: %f", time.ElapsedSeconds()));					
				}
			}
			
			update_ = false;

			LOGD(AI3D::CORE::String::StringPrintf("Single click spends %f s", time.ElapsedSeconds()));
		}

		int ControlPointsEditorWin::get_photos(const AI3D::CORE::Image& image/*, const QString& userTiePointName*/)
		{
			//note:calculate the photos number for specified user tie  point.


			return 0;
		}

		//attention注意一下 如果一开始没有GCP页卡则需要创建GCP页卡，之前 Slot_Btn_AddSigGcp_Clicked 中有这个逻辑，可做一定的参考
		void ControlPointsEditorWin::Slot_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName)
		{
			std::cout << "inside "  << " " << __FUNCTION__ << " " << __LINE__ << " " << userTiePointName.toStdString() << std::endl;
			gcp_list_item_st gcpListItem;
			AI3D::CORE::ATCommandSet::AddUserTiepoint(*blockdata_->GetCurrentATMutual().get(), image.GetImageId(), userTiePointName.toStdString());
			InitGcpData();
			/*gcpListItem.color_ = 0; // gcp.color_;
			gcpListItem.ControlpointsID = 0; // it->first;
			gcpListItem.ControlpointsImageID = image.GetImageId();
			gcpListItem.bHasImageId = true;
			gcpListItem.name_ = userTiePointName; //QString::fromLocal8Bit(gcp.name_.c_str());
			gcpListItem.photos_ = get_photos(image); // gcp.photos_;

			sv_type_e type; // = gcp.type_;
			//if (type == sv_type_e::SURVEYSHOW_GCP)
			{

			//	gcpListItem.category_ = "Control point";
			}
			//else if (type == sv_type_e::SURVEYSHOW_GCPCHECK)
			{

			//	gcpListItem.category_ = "Check point";
			}
			//else if (type == sv_type_e::SURVEYSHOW_USERPOINT)
			{
				gcpListItem.category_ = "User Tiepoint";
			}
			//else
			//{

			//	gcpListItem.category_ = "Control point";
			//}

			gcpListItem.given_x_ = -DBL_MAX;
			gcpListItem.given_y_ = -DBL_MAX;
			gcpListItem.given_z_ = -DBL_MAX;

			gcpListItem.str_given_x_ = "";
			gcpListItem.str_given_y_ = "";
			gcpListItem.str_given_z_ = "";

			gcpListItem.esitmated_x_ = -DBL_MAX;
			gcpListItem.esitmated_y_ = -DBL_MAX;
			gcpListItem.esitmated_z_ = -DBL_MAX;

			gcpListItem.str_esitmated_x_ = "";
			gcpListItem.str_esitmated_y_ = "";
			gcpListItem.str_esitmated_z_ = "";

			gcpListItem.rms_pix_ = -DBL_MAX;
			gcpListItem.str_rms_pix_ = "";

			gcpListItem.rms_dis_ = -DBL_MAX;
			gcpListItem.str_rms_dis_ = "";

			gcpListItem.error_3d_ = -DBL_MAX;
			gcpListItem.str_error_3d_ = "";

			gcpListItem.error_3d_xy_ = -DBL_MAX;
			gcpListItem.error_3d_z_ = -DBL_MAX;

			gcpListItem.str_error_3d_xy_ = "";
			gcpListItem.str_error_3d_z_ = "";

			// 向GcpListView 添加一行数据.

			ui->gcplistview->appendRowData(gcpListItem);*/
		}

		void ControlPointsEditorWin::MakeGcplistViewUneditable(bool bxyzedit_on)
		{
			if (bxyzedit_on)
				ui->gcplistview->setEditTriggers(QAbstractItemView::DoubleClicked);
			else
				ui->gcplistview->setEditTriggers(QAbstractItemView::NoEditTriggers);
		}

		void ControlPointsEditorWin::SetBlockdata(AI3D::CORE::BlockObject* block)
		{
			blockdata_ = block;
		}
	
		void ControlPointsEditorWin::Slot_SrsItemChanged(QString srsname)//用于切换坐标系统
		{

			srsname = AI3D::GUI::MohackerWin::stripPrependIndentation(srsname);

#ifdef USE_AI3D_PROJ

			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << ui->comboBox_srs->currentIndex() << " / "
			//	<< ui->comboBox_srs->count() << " "  << srsname.toStdString() << std::endl;

			if (1 && (srsname == "Spatial reference system database" || srsname == "空间参考系统数据库"))
			{
			//	std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << ui->comboBox_srs->currentIndex() << std::endl;
				///  note!!!: just for test purpose now.
				//AI3D::PROJ::QgsCoordinateReferenceSystem*crs = new AI3D::PROJ::CoordinateReferenceSystem(srsname.toStdString());
				//QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem(srsname);
				//QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem("EPSG:4978");
				///QgsCoordinateReferenceSystem* crs = new QgsCoordinateReferenceSystem("EPSG:4326");

				//QgsCoordinateReferenceSystem crs;
				AI3D::PROJ::CoordinateReferenceSystem crs;
				QString _srsname;

				int idx = ui->comboBox_srs->currentIndex();
				if (idx >= 0 && idx <= ui->comboBox_srs->count())
					_srsname = ui->comboBox_srs->itemData(idx).toString();

				_srsname = previous_srs;


	//			crs.createFromString("EPSG:4413");

				//std::cout << "current.text:" << _srsname.toStdString() << std::endl;

				int startAuthIdPos = _srsname.lastIndexOf("(");
				int endAuthIdPos = _srsname.lastIndexOf(")");

				bool bFoundLocalENU = false;
				bool bFoundValidENUCrs = false;

				///if (_srsname.contains(MohackerWin::localENUPrefix(), Qt::CaseInsensitive))
				///{
				///	bFoundLocalENU = true;
				///}
				if (_srsname.contains(MohackerWin::localSRS(), Qt::CaseInsensitive))
				{
					//std::cout << "current.text1:" << _srsname.toStdString() << std::endl;
					//bFoundLocalENU = true;
					QString authId = "Local:0";
					crs.createFromString(authId);
				}
				else 
				if (startAuthIdPos >= 0 && endAuthIdPos >= 0 && startAuthIdPos < endAuthIdPos)
				{
					QString authId = _srsname.mid(startAuthIdPos + 1, endAuthIdPos - (startAuthIdPos + 1));
					if (authId.contains("ENU", Qt::CaseInsensitive))
					{
						//						std::cout << "current.text21:" << _srsname.toStdString() << std::endl;
						crs.CreateFromENUDefinition(authId);
						if (crs.isValid())
						{
							//							std::cout << "gcp dia/valid enu crs definition:" << crs.description().toStdString() << " authid:" << crs.authid().toStdString() << std::endl;
							bFoundValidENUCrs = true;
						}
					}
					else
					{
						crs.createFromString(authId);
					}
				}
				else
				{
					crs.CreateFromENUDefinition(_srsname);
				}

				///AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound,(bFoundLocalENU ? _srsname : ""));
				//QgsProjectionSelectionTreeWidget* qgsWidget = new QgsProjectionSelectionTreeWidget();
				AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound, (bFoundValidENUCrs ? crs.description() : ""),
					(bFoundValidENUCrs ? crs.authid() : ""));

				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsSelected, this, &ControlPointsEditorWin::Slot_SrsSelected);			
				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsRestore, this, &ControlPointsEditorWin::Slot_SrsRestore);

				if (bFoundLocalENU)
				{
///					qgsWidget->setLastEnuData(_srsname);
				}
				else
				{
					qgsWidget->setCrs(crs);
				}

//				qgsWidget->setFixedSize(578, 650);
				qgsWidget->setFixedSize(1130, 810);
				qgsWidget->show();

				if (bFoundLocalENU)
					qgsWidget->selectCrsByName(QString("Local East-North-Up (ENU)"));

				return;
			}

			previous_srs = srsname;

			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << ui->comboBox_srs->currentIndex() << std::endl;

			srs_s srs = GetSelectSrs(srsname);

			if (current_srs_.definition != srs.definition)
			{
				current_srs_ = srs;
				MakeGcplistViewUneditable((current_srs_.definition == default_srs_.definition));			
				
				if (AI3D::GUI::MohackerWin::stripPrependIndentation(ui->comboBox_srs->currentText()).left(3) == "WGS")
				{
				
					ui->gcplistview->setHeaderLabelsMode(true);
				
				}
				else
				{
				
					ui->gcplistview->setHeaderLabelsMode(false);
				
				}

				// use the flag later.
				bChangingSrs = true;

				UpdateGcpListView();

				ui->previewlistview->clearData();
				ui->measurementsview->clearData();
				measuringview_->clear();

				bChangingSrs = false;
			}
#endif
		}
	
		void ControlPointsEditorWin::Slot_SrsSelected(QString &srs)
		{
			ui->comboBox_srs->blockSignals(true);
			InitSrss(true);
			ui->comboBox_srs->blockSignals(false);
			///previous_srs = ui->comboBox_srs->currentText();
		}

		void ControlPointsEditorWin::Slot_SrsRestore()
		{
//			std::cout << previous_srs.toStdString() << std::endl;
			if (!previous_srs.isEmpty())
			{
				ui->comboBox_srs->blockSignals(true);
				ui->comboBox_srs->setCurrentText((AI3D::GUI::MohackerWin::prependIndentation() + previous_srs));
				ui->comboBox_srs->blockSignals(false);
			}
		}

		void ControlPointsEditorWin::Slot_CategoryChanged(QString itemtext)//控制点改类型与name一样
		{
			//Slot_GcpListItem_SingleClicked();
			if (currentgcp_id_ == kInvalidPoint3DId)
			{
				return;
			}
			// note: support chinese options later for current combobox control when running inside chinese environment.
			if (itemtext == "Control point")//此处改为宏吧
			{
				if (blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_].GetType() == gpt_e::GCP_CHECK_HV)
				{
					blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_].SetType(gpt_e::GCP_CONTROL_HV);
					emit Sig_ModifiedTrue();
				}
					
			}
			else if (itemtext == "Check point")
			{
				if (blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_].GetType() == gpt_e::GCP_CONTROL_HV)
				{
					blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_].SetType(gpt_e::GCP_CHECK_HV);
					
					emit Sig_ModifiedTrue();
					
				}
					
			}
			
			
		}

		
		
		void ControlPointsEditorWin::Slot_DeleteAllGcps()//一键删除；
		{
			//1：调用atdata的deletegcp
			//情况所有view；也即恢复到init状态，但是srs是否恢复，待商议；
			InitBlankGcpListView();
			InitBlankPreviewListView();
			InitBlankMeasuringView();
			InitBlankMeasurementsView();
			
			//设置按钮为不可点击状态
			InitPhotosButtons();		
			emit Sig_ModifiedTrue();		
		}

		
		void ControlPointsEditorWin::InitBlankPreviewListView()
		{
			//PreviewListView列表所有内容被清空；
			UpdatePreviewListViewBottonStatus();
			
			ui->previewlistview->clearData();

			images_show_.clear();
			
		}
		void ControlPointsEditorWin::InitBlankMeasuringView()
		{
			//MeasuringView列表所有内容被清空；
			measuringview_->clear();
			//measuringview_->ClearLine();
			measured_xy_show_ = Eigen::Vector2d{ -DBL_MAX, -DBL_MAX };
			candidate_xy_show_ = Eigen::Vector2d{ -DBL_MAX, -DBL_MAX };
			epipolarlines_show_.clear();
			
			currentimage_id_ = kInvalidImageId;
			old_image_scale_ratio_ = std::make_pair<>(kInvalidImageId, 1.0);//int, float
			
			
		}
		void ControlPointsEditorWin::InitBlankMeasurementsView()
		{
			
			ui->measurementsview->clearData();

			
		}
		
		// 获取当前GcpId对应的图形及关联预测点Map,便于Gcp PreviewListView可以在分组的基础上,按照预测点与中心点的距离进行排序.
		void ControlPointsEditorWin::MatchPhotos(std::map<image_t, Eigen::Vector2d >& estimated_xys)
		{
			if (!globalid_to_localid_map_.count(currentgcp_id_))
			{
				return;
			}
			if (!IsCurrentSelectionUserType(currentgcp_id_))
			{
				
				
					point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
					blockdata_->GetCurrentAT()->PredictGCPMeasurement(gcpid, estimated_xys);
				
			}
			else
			{
				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
				blockdata_->GetCurrentAT()->PredictUserPtMeasurement(gcpid, estimated_xys);
			}
			
		}

		//此函数在用时得注意哪些条件满足计算，哪些会返回false之类的
		void ControlPointsEditorWin::MatchPhotos(std::set<image_t >& images_ids)
		{
			if (!globalid_to_localid_map_.count(currentgcp_id_))
			{
				return;
			}
			//bool isgcp = (globalid_to_localid_map_.at(currentgcp_id_).second == sv_type_e::SURVEYSHOW_GCP ||
			//	globalid_to_localid_map_.at(currentgcp_id_).second == sv_type_e::SURVEYSHOW_GCPCHECK);
			////bool isuserpt = (globalid_to_localid_map_.at(currentgcp_id_).second == sv_type_e::SURVEYSHOW_USERPOINT);
			if (!IsCurrentSelectionUserType(currentgcp_id_))
			{


				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
				blockdata_->GetCurrentAT()->PredictGCPMeasurement(gcpid, images_ids, btopredict_);

			}
			else
			{
				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
				blockdata_->GetCurrentAT()->PredictUserPtMeasurement(gcpid, images_ids, btopredict_);
			}
			
			
		}

		//previewlistview相关操作
		void ControlPointsEditorWin::Slot_AllPhotos_Clicked()
		{
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << "" << __LINE__ << " clicked." << std::endl;

			preview_show_mode_ = 1;
			//分控制点类型和人工点类型，控制点类型不动额外加入人工点的

			UpdatePreviewListViewBottonStatus();

			if (GetCurrentImageId() == kInvalidImageId)
			{
				SetCurrentImageId(GetFirstImageId());
			}

#if 1
			UpdatePreviewListView();
			UpdateMeasuringView();
			//measurementsview高亮显示当前影像
			ShowSelectedMeasurementView();
#endif			
		}
	
		void ControlPointsEditorWin::Slot_itemModified(int row, int col, const QString& text)
		{
			if(col == X2_COL || col == Y2_COL || col == Z2_COL)
			{
				int id = col - X2_COL;
				Slot_GivenXYZChanged(id, text);
				emit Sig_ModifiedTrue();
			}
			else if (col == CATEGORY2_COL)
			{
				Slot_CategoryChanged(text);
			}
			else if (col == NAME2_COL)
			{

			}
		}

		void ControlPointsEditorWin::Slot_MatchedPhotos_Clicked()
		{
			//set matchphotobuttom on 
			//？chy：是否 还存在
			// 重投影计算预判点，此处效率低，需要优化；后续安排；
			
			preview_show_mode_ = 2;			
			UpdatePreviewListViewBottonStatus();

			if (GetCurrentImageId() == kInvalidImageId)
			{
				SetCurrentImageId(GetFirstImageId());
			}
			UpdatePreviewListView();
			UpdateMeasuringView();		
			ShowSelectedMeasurementView();
			
		}

		void ControlPointsEditorWin::Slot_MarkedPhotos_Clicked()
		{
			//set markedphotobuttom on 
			
			preview_show_mode_ = 3;
			
			UpdatePreviewListViewBottonStatus();

			if (currentimage_id_ == kInvalidPoint3DId || (!measurement_error_map_.at(currentgcp_id_).count(currentimage_id_)))
			{
				SetCurrentImageId(GetFirstImageId());
			}
			

			UpdatePreviewListView();
			UpdateMeasuringView();
			
			ShowSelectedMeasurementView();
		
		}

		void ControlPointsEditorWin::displayImage(const QModelIndex& index,QString& imageFile, int specialX, int specialY)
		{

			if (!index.isValid())
				return;

			int row = index.row();
			image_t img_id = index.data(CRControlpointsImageID).toUInt(); 


	
			if (img_id != currentimage_id_)
			{
				SetCurrentImageId(img_id);

				if (images_show_.count(img_id))
				{
					// 显示Gcp刺点大图.
					UpdateMeasuringView();//

					if ( measurement_error_map_.at(currentgcp_id_).count(img_id))
					{
						UpdateMeasurementsView();//此处需要高亮显示被选中的那个；
					}
					else
					{
						//恢复MeasurementsView
						RecoverMeasurementsView();
					}
				}

			}

		}
		
		//measuring 界面相关的操作主要就是shift + 单击
		void ControlPointsEditorWin::Slot_Measuring_Clicked(int imageID, QPointF point)
		{
			AI3D::CORE::Timer time;
			time.Start();
			qreal x = point.x();
			qreal y = point.y();

			bMeasuringClicked = true;

			auto& image = blockdata_->GetCurrentATMutual()->GetImageMutual(imageID);
			Eigen::Vector2d v2d(point.x(), point.y());
			image.SetPoints2DGCP(currentgcp_id_, v2d);
			AI3D::CORE::ControlPoint& currentgcp = blockdata_->GetCurrentATMutual()->GetControlPointsMutual()[currentgcp_id_];

			std::cout << "gv image with:" << image.GetWidth() << "/" << image.GetHeight() << " " << point.x() << "/" << point.y() << std::endl;

			auto& track = currentgcp.GetObjectPointMutual().GetTrackMutual().GetElementByImagIdMutual(image.GetImageId());
			if (track.image_id == imageID && track.point2D_idx== currentgcp_id_ && image.InsideImage(track.xy))
			{
				track.xy = v2d;
			}
			else		
			{
				AI3D::CORE::TrackElement trackElement;
				trackElement.image_id = imageID;
				trackElement.point2D_idx = currentgcp_id_;
				trackElement.xy = v2d;
				currentgcp.GetObjectPointMutual().GetTrackMutual().AddElement(trackElement);
			}
			//触发工程已被编辑
			emit Sig_ModifiedTrue();
			measurement_error_map_.at(currentgcp_id_).clear();
			//无空三刺点的兼容
			auto elements = currentgcp.GetObjectPoint().GetTrack().GetElements();
			for (const auto& ele : elements)
			{
				measurement_error_map_.at(currentgcp_id_)[ele.image_id] = 
					std::pair<Eigen::Vector2d, std::pair<double, double>>(ele.xy, std::pair<double, double>(-DBL_MAX, -DBL_MAX));
			}
			//刷新单个列表
			btopredict_ = true;
			blockdata_->GetTaskInfoMutual().btopredict_ = true;
			blockdata_->GetCurrentATMutual()->UpdataGCPErrorInfo(currentgcp_id_, measurement_error_map_.at(currentgcp_id_), btopredict_);

			UpdateGcpListView(true);
			UpdatePreviewListView();
			UpdateMeasuringView(true);//如十字丝核线等
			UpdateMeasurementsView();
			//设置当前的item gcpview 和previewlist
			ShowPreviewHighLight();
			ShowSelectedGcpView();
			LOGD(AI3D::CORE::String::StringPrintf("Measuring point spends %f s", time.ElapsedSeconds()));
				
			ShowSelectedGcpView();
			bMeasuringClicked = false;			
		}
		//平移缩放等操作略

		//MeasurementsView界面相关操作
		//指某个Item单击或双击时,此處对应itemClickedTableWidget_Measure,但是是否有区别?
		// 点击Item,但只关心行信息.
		void ControlPointsEditorWin::measurement_tableviewClick(QModelIndex index)
		{
			image_t img_id = 0; 
			if (!index.isValid())
				return;
			img_id = ui->measurementsview->getImageIdByRow(index.row());
			if (img_id != currentimage_id_)
			{
				SetCurrentImageId(img_id);

				if (images_show_.count(img_id))
				{
					int color = images_show_[img_id].color_;			
					
					ShowPreviewHighLight();
					UpdateMeasuringView();
				}
			}
		}
		bool ControlPointsEditorWin::IsCurrentSelectionUserType(const point3D_t& id)
		{
			try
			{
				
				if (!globalid_to_localid_map_.count(id))
				{
					return false;
				}
				if (globalid_to_localid_map_.at(id).second == sv_type_e::SURVEYSHOW_USERPOINT)
					return true;
				else
					return false;
			}
			
			catch (std::exception& ex)
			{
				std::ostringstream oss;
				oss << "exception:" << ex.what();
				LOGI(oss.str());
				return false;
			}
			
		}

		//删除某个measurement
		void ControlPointsEditorWin::Slot_MeasurementsItem_Delete()
		{		
			AI3D::CORE::Timer time;
			time.Start();
			measurement_error_map_.at(currentgcp_id_).erase(currentimage_id_);
			image_t next_img_id = GetFirstImageId();		
			measurement_error_map_.at(currentgcp_id_).clear();
			if (!globalid_to_localid_map_.count(currentgcp_id_))
			{
				return;
			}
			if (!IsCurrentSelectionUserType(currentgcp_id_))
			{
				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
				std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_measurement_error_map;
				std::set<point3D_t> tochangedids;
				for (auto& itererror : measurement_error_map_)
				{
					if (globalid_to_localid_map_.count(itererror.first))
					{
						auto gcpinfo = globalid_to_localid_map_.at(itererror.first);
						if (!IsCurrentSelectionUserType(itererror.first))
						{
							gcp_measurement_error_map[gcpinfo.first] = itererror.second;
							tochangedids.insert(itererror.first);
						}
					}
				}
				if (gcp_measurement_error_map.empty() || !gcp_measurement_error_map.count(gcpid))
				{
					return;
				}
				blockdata_->GetCurrentATMutual()->DeleteGCPMeasurement(gcpid, currentimage_id_, gcp_measurement_error_map.at(gcpid));
				//还原到总的
				for (auto& iterset : tochangedids)
				{
					measurement_error_map_.at(iterset) = gcp_measurement_error_map.at(globalid_to_localid_map_.at(iterset).first);
				}
			}
			else
			{
				point3D_t userid = globalid_to_localid_map_.at(currentgcp_id_).first;
				
				point3D_t gcpid = globalid_to_localid_map_.at(currentgcp_id_).first;
				std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >> gcp_measurement_error_map;
				std::set<point3D_t> tochangedids;
				for (auto& itererror : measurement_error_map_)
				{
					if (globalid_to_localid_map_.count(itererror.first))
					{
						auto gcpinfo = globalid_to_localid_map_.at(itererror.first);
						bool isuser = (gcpinfo.second == sv_type_e::SURVEYSHOW_USERPOINT);
						if (isuser)
						{
							gcp_measurement_error_map[gcpinfo.first] = itererror.second;
							tochangedids.insert(itererror.first);
						}
					}
				}
				if (gcp_measurement_error_map.empty() || !gcp_measurement_error_map.count(gcpid))
				{
					return;
				}
				blockdata_->GetCurrentATMutual()->DeleteUserPtMeasurement(gcpid, currentimage_id_, gcp_measurement_error_map.at(gcpid));
				//还原到总的
				for (auto& iterset : tochangedids)
				{
					measurement_error_map_.at(iterset) = gcp_measurement_error_map.at(globalid_to_localid_map_.at(iterset).first);
				}
			}

			
			SetCurrentImageId(next_img_id);
			UpdateGcpListView(true);//此处后续也改为只更新当前的@liyue
			UpdatePreviewListView();
			UpdateMeasuringView();//如十字丝核线等
			UpdateMeasurementsView();
			ShowSelectedGcpView();

			emit Sig_ModifiedTrue();

			LOGD(AI3D::CORE::String::StringPrintf("Delete measurements spends %f s", time.ElapsedSeconds()));
			//触发工程已被编辑
		}

		void ControlPointsEditorWin::Slot_ShowEpi()
		{
			bshow_epipolarline_ = !bshow_epipolarline_;
			ui->btn_epipolarline->setChecked(bshow_epipolarline_);
			
			if (bshow_epipolarline_)
			{
				
				UpdateMeasuringView();
			}
			else
			{
				measuringview_->HideLine();
			}
		}

		void ControlPointsEditorWin::Slot_QTableView_CustomContextMenuRequested(const QPoint& pos)
		{
			
			QModelIndex index = ui->gcplistview->indexAt(pos);
			if (index.isValid())
			{
				ui_menu_rightClick_selectRows->exec(QCursor::pos());
			}

		}

		void ControlPointsEditorWin::Slot_QTableWidget_CustomContextMenuRequested(const QPoint& pos)
		{
#if 1
			
			ui_menu_rightClickMeasure_selectRows->move(cursor().pos());
			int x = pos.x();
			int y = pos.y();

			QModelIndex index = ui->measurementsview->indexAt(QPoint(x, y));

			menuRow = index.row();

			if (menuRow >= 0)
			{
				ui_menu_rightClickMeasure_selectRows->show();
				connect(ui_action_deletemeausurement_, &QAction::triggered, this, &ControlPointsEditorWin::Slot_MeasurementsItem_Delete);

				emit Sig_ModifiedTrue();
			}
#endif			
		}

	
	}
}

