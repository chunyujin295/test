#include "Gui/PosSigmaDia.h"
#include <QRadioButton>
#include <QDebug>
#include <QPainter>
#include <QBitmap>
#include <QMessageBox>
#include "Gui/message_box.h"
#include <qstandarditemmodel.h>
#include "Util/TaskProcess.h"
#define ENABLESTYLE "color:#FFFFFF"
#define UNENABLESTYLE "color:#A5A5A5"

#define RADIOBUTTONSTYLE QString::fromUtf8( \
"QRadioButton:disabled\n" \
"{ color:gray;				}\n"\
"QRadioButton::indicator\n"\
"{ width:10px;\n"\
"height:10px;\n"\
"border-radius:10px; }QRadioButton::indicator:checked\n"\
"{\n"\
"width:10px;\n"\
"height:10px;\n"\
"background-color:qradialgradient(spread : pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,\n"\
"stop : 0 rgba(4,156,232,255),stop : 0.6 rgba(4,156,232,255),stop : 0.65 rgba(255,255,255,255),\n"\
"stop : 0.8 rgba(255,255,255,255),stop : 0.85 rgba(4,156,232,255),stop : 1 rgba(4,156,232,255));\n"\
"border:2px solid rgb(4,156,232);\n"\
"border-radius:6px; }QRadioButton::indicator:checked:disabled\n"\
"{\n"\
"width:10px;\n"\
"height:10px;\n"\
"background-color:qradialgradient(spread : pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,\n"\
"stop : 0 rgba(169,169,169,255),stop : 0.6 rgba(169,169,169,255),stop : 0.65 rgba(255,255,255,255),\n"\
"stop : 0.8 rgba(255,255,255,255),stop : 0.85 rgba(169,169,169,255),stop : 1 rgba(169,169,169,255));\n"\
"border:2px solid rgb(169,169,169);\n"\
"border-radius:6px; }QRadioButton::indicator:unchecked\n"\
"{width:12px;\n"\
"height:12px;\n"\
"background-color:white;\n"\
"boder: 2px solid rgb(66, 66, 66);border-radius:6px;\n"\
" }QRadioButton::indicator:unchecked:disabled\n"\
"{width:12px;\n"\
"height:12px;\n"\
"background-color:rgb(169, 169, 169);\n"\
"boder: 2px solid rgb(200,200, 200);border-radius:6px;}\n"\
)

#define RADIOBUTTONSTYLE2 QString::fromUtf8( \
"QRadioButton::indicator:checked\n"\
"{image: url(:/new/prefix1/skin/radioButtonSelect1x.png);\n"\
"color:#FFFFFF;}\n"\
"QRadioButton::indicator:checked\n"\
"{image: url(:/new/prefix1/skin/radioButton1x.png);\n"\
"color:#A5A5A5;}\n")



using namespace AI3D::CORE;
namespace AI3D
{
	namespace GUI
	{
		

		


		PosSigmaDia::PosSigmaDia(QDialog* parent)
			: QDialog(parent),
			ui(new Ui_PosSigma),
			
			featureNum(0)
		{
			AI3D::CORE::sfmsettings_s sigma;(0);
			sigmaX = sigma.pos_sigma.x();
			sigmaY = sigma.pos_sigma.y();
			sigmaZ = sigma.pos_sigma.z();
			ui->setupUi(this);
		
			//this->setWindowTitle("SubmitAT");
			this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
			ui->widget->setWindowOpacity(0.5);
			ui->radbtn_gps->setStyleSheet(RADIOBUTTONSTYLE);
			ui->radbtn_gcp->setStyleSheet(RADIOBUTTONSTYLE);
			ui->radbtn_highsigma->setStyleSheet(RADIOBUTTONSTYLE);
			ui->radbtn_lowsigma->setStyleSheet(RADIOBUTTONSTYLE);
			ui->radbtn_normalsigma->setStyleSheet(RADIOBUTTONSTYLE);
			ui->radbtn_arbitrary->setStyleSheet(RADIOBUTTONSTYLE);
			ui->widget_posmode->setEnabled(true);
			ui->lbl_pos->setStyleSheet(ENABLESTYLE);
			ui->radbtn_arbitrary->setEnabled(false);
			ui->radbtn_arbitrary->setChecked(false);
			
		
				ui->radbtn_gps->setEnabled(false);
				ui->radbtn_gps->setChecked(false);
				
			
				ui->radbtn_gcp->setEnabled(false);
				ui->radbtn_gcp->setChecked(false);

			
			SetKeyDensityVisibale(true);
			ui->lbl_pairselection_mode->setVisible(true);
			ui->le_pairselection_mode->setVisible(true);

			if (BlockObject::isChineseVersion())
			{
				ui->le_pairselection_mode->setItemData(0, str2qstr(std::string("默认")), Qt::DisplayRole);
				ui->le_pairselection_mode->setItemData(1, str2qstr(std::string("影像序列")), Qt::DisplayRole);
				
			}
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui->label_2->setText(str2qstr(std::string("参数设置")));
				ui->label_3->setText(str2qstr(std::string("区块名称")));
				ui->label_4->setText(str2qstr(std::string("名称")));
				ui->lbl_arbitrary->setText(str2qstr(std::string("自由坐标系")));
				ui->lbl_pos->setText(str2qstr(std::string("使用POS元数据")));
				ui->lbl_gcp->setText(str2qstr(std::string("使用控制点")));
				
					ui->lbl_atsettings->setText(str2qstr(std::string("空三参数")));
				ui->lbl_keydensity->setText(str2qstr(std::string("特征点数量")));
				ui->lbl_pairselection_mode->setText(str2qstr(std::string("像对选择模式")));
				
				ui->label_11->setText(str2qstr(std::string("POS精度")));

				ui->lbl_keydensity->setAlignment(Qt::AlignRight);
				ui->lbl_pairselection_mode->setAlignment(Qt::AlignRight);
				ui->label_11->setAlignment(Qt::AlignRight);

				ui->lbl_posmode->setText(str2qstr(std::string("定位模式")));

				ui->lbl_gcpmarked_result->setText(str2qstr(std::string("刺点状态")));
				ui->lbl_totalgcpinfo->setText(str2qstr(std::string("控制点数量")));
				ui->lbl_gcpmarked->setText(str2qstr(std::string("刺点完成数量")));
				ui->lbl_gcpmarkedphoto->setText(str2qstr(std::string("刺点总影像数")));

				ui->esitmitationpolicytext->setText(str2qstr(std::string("解算策略")));
				ui->le_tiepointspolicy->setText(str2qstr(std::string("连接点")));
				ui->le_posepolicy->setText(str2qstr(std::string("位姿")));
				ui->le_fpolicy->setText(str2qstr(std::string("焦距")));
				ui->le_ppapolicy->setText(str2qstr(std::string("主点")));
				ui->le_rdispolicy->setText(str2qstr(std::string("径向畸变")));
				ui->le_tdispolicy->setText(str2qstr(std::string("切向畸变")));
				ui->BtnOK->setText(str2qstr(std::string("确定")));
				ui->BtnCancle->setText(str2qstr(std::string("取消")));
				
			}
			connect(ui->radbtn_normalsigma, &QRadioButton::clicked, this, &PosSigmaDia::slot_RadioButton);
			connect(ui->radbtn_highsigma, &QRadioButton::clicked, this, &PosSigmaDia::slot_RadioButton);
			
			connect(ui->radbtn_gps, &QRadioButton::clicked, this, &PosSigmaDia::slot_ControlRadioButton);
			connect(ui->radbtn_gcp, &QRadioButton::clicked, this, &PosSigmaDia::slot_ControlRadioButton);
			connect(ui->radbtn_arbitrary, &QRadioButton::clicked, this, &PosSigmaDia::slot_ControlRadioButton);
			connect(ui->radbtn_lowsigma, &QRadioButton::clicked, this, &PosSigmaDia::slot_RadioButton);
			connect(ui->BtnOK, &QPushButton::clicked, this, &PosSigmaDia::slot_getParam);
			connect(ui->BtnCancle, &QPushButton::clicked, this, &PosSigmaDia::slot_getParam);
			at_options_.sfmsettings.pos_sigma.x() = sigmaX;
			at_options_.sfmsettings.pos_sigma.y() = sigmaY;
			at_options_.sfmsettings.pos_sigma.z() = sigmaZ;
			//自由参数设置值限定在0-100之间
			QDoubleValidator *pDoubleValidator = new QDoubleValidator(0,99,2);
			pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
			
		}

		PosSigmaDia::~PosSigmaDia()
		{
			delete ui;
		}

		void PosSigmaDia::paintEvent(QPaintEvent* p1)
		{
			//绘制样式
			QStyleOption opt;
			opt.initFrom(this);
			QPainter p(this);
			style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式

			QBitmap bmp(this->size());
			bmp.fill();
			QPainter painter(&bmp);
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::black);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawRoundedRect(bmp.rect(), 6, 6);
			setMask(bmp);

		}
		void PosSigmaDia::setName(const QString& fileName)
		{
			ui->TaskName->setText(fileName);
			//add chy

			///at_options_.at_name = qstr2str(fileName);
			at_options_.at_name = fileName.QString::toStdString();

		}

		void PosSigmaDia::SetATOptions(AI3D::CORE::ATOptions& options)
		{
			at_options_ = options;
		}

		AI3D::CORE::ATOptions PosSigmaDia::GetATOptions()const
		{
			return at_options_;
		}

		AI3D::CORE::ATOptions& PosSigmaDia::GetATOptionsMutual()
		{
			return at_options_;
		}
	
		
		//任意模式是时刻存在的
		void PosSigmaDia::setPosModeRadioChecked(alignmode_e mode)
		{
			ui->radbtn_arbitrary->setEnabled(true);
			ui->radbtn_arbitrary->setCheckable(true);

			switch(mode)
			{
			case ALIGN_ARBITRARY:
			{

				ui->radbtn_arbitrary->setEnabled(true);
				ui->radbtn_arbitrary->setChecked(true);
				SetWidgetStatusByRadioCheked(alignmode_e::ALIGN_ARBITRARY);
				/*setGCPDataEnable(false);*/
				break;
			}
			case ALIGN_WITHPOS:
			{
				ui->radbtn_gps->setEnabled(true);
				ui->radbtn_gps->setChecked(true);
				ui->lbl_pos->setStyleSheet(ENABLESTYLE);
				ui->lbl_gcp->setStyleSheet(UNENABLESTYLE);
				SetWidgetStatusByRadioCheked(alignmode_e::ALIGN_WITHPOS);
				/*setGCPDataEnable(false);*/
				break;
			}
			case ALIGN_WITHGCP:
			{
				ui->radbtn_gcp->setEnabled(true);
				ui->radbtn_gcp->setChecked(true);
			
				ui->lbl_pos->setStyleSheet(UNENABLESTYLE);
				ui->lbl_gcp->setStyleSheet(ENABLESTYLE);

				SetWidgetStatusByRadioCheked(alignmode_e::ALIGN_WITHGCP);
				//setGCPDataEnable(true);
				break;
			}
			case ALIGN_WITHGCP_POS:
			{
				ui->radbtn_gcp->setEnabled(true);
				ui->radbtn_gcp->setChecked(true);
				ui->radbtn_gps->setEnabled(true);
				ui->radbtn_gps->setCheckable(true);
				ui->radbtn_gps->setChecked(false);

				
				ui->lbl_pos->setStyleSheet(ENABLESTYLE);
				ui->lbl_gcp->setStyleSheet(ENABLESTYLE);
				SetWidgetStatusByRadioCheked(alignmode_e::ALIGN_WITHGCP);
				//setGCPDataEnable(true);
				break;
			}
			case ALIGN_WITHGCP_ARBITRARY:
			{
				ui->radbtn_gcp->setEnabled(true);
				ui->radbtn_gcp->setChecked(true);
				ui->radbtn_arbitrary->setEnabled(true);				
				ui->radbtn_arbitrary->setChecked(false);
				ui->lbl_pos->setStyleSheet(UNENABLESTYLE);
				SetWidgetStatusByRadioCheked(alignmode_e::ALIGN_WITHGCP);
				/*setGCPDataEnable(false);*/
				break;
			}

			}

		}

		void PosSigmaDia::SetKeyDensityVisibale(bool bIsVis)
		{
			ui->le_keydensity->setVisible(bIsVis);
			ui->lbl_keydensity->setVisible(bIsVis);
			
		}
		void PosSigmaDia::SetPairSelectionModeEnable(bool bIsEnable)
		{
			QStandardItemModel* pItemModel = qobject_cast<QStandardItemModel*>(ui->le_pairselection_mode->model());
			if (bIsEnable)
			{
				auto index = ui->le_pairselection_mode->currentIndex();
				pItemModel->item(index)->setForeground(QBrush(QColor(255, 255, 255)));
				ui->lbl_pairselection_mode->setStyleSheet(ENABLESTYLE);
			}
			else
			{
				auto index = ui->le_pairselection_mode->currentIndex();
				pItemModel->item(index)->setForeground(QBrush(QColor(165, 165, 165)));
				ui->lbl_pairselection_mode->setStyleSheet(UNENABLESTYLE);
			}
		}
		void PosSigmaDia::SetATSettingLabelEnable(bool bIsEnable)
		{
			
			ui->widget_atsetting->setEnabled(bIsEnable);
			if (bIsEnable)
			{
				ui->lbl_atsettings->setStyleSheet(ENABLESTYLE);
			}
			else
			{
				ui->lbl_atsettings->setStyleSheet(UNENABLESTYLE);
			}
		}

		void PosSigmaDia::SetPOSaccuracyEnable(bool bIsEnable)
		{
			if (bIsEnable)
			{
				ui->radbtn_normalsigma->setEnabled(true);
				/*ui->radbtn_normalsigma->setChecked(true);*/
				ui->radbtn_lowsigma->setChecked(true);
				ui->radbtn_highsigma->setEnabled(true);
				ui->radbtn_lowsigma->setEnabled(true);
				ui->lbl_normal_sigma->setStyleSheet(ENABLESTYLE);
				ui->lbl_highsigma->setStyleSheet(ENABLESTYLE);
				ui->lbl_lowsigma->setStyleSheet(ENABLESTYLE);

			}
			else
			{
				ui->radbtn_normalsigma->setEnabled(false);
				ui->radbtn_normalsigma->setChecked(false);

				ui->radbtn_highsigma->setEnabled(false);
				ui->radbtn_highsigma->setChecked(false);

				ui->radbtn_lowsigma->setEnabled(false);
				ui->radbtn_lowsigma->setChecked(false);

				
				ui->lbl_normal_sigma->setStyleSheet(UNENABLESTYLE);
				ui->lbl_highsigma->setStyleSheet(UNENABLESTYLE);
				ui->lbl_lowsigma->setStyleSheet(UNENABLESTYLE);
			}
		}
		
		void PosSigmaDia::setATSettingWidgetEnable(bool bIsEnable, bool gcpmode)
		{
			QStandardItemModel* pItemModel = qobject_cast<QStandardItemModel*>(ui->le_keydensity->model());
			if (bIsEnable)
			{
				/*ui->widget_atsetting->setEnabled(true);*/

				
				ui->radbtn_normalsigma->setEnabled(true);				
				ui->radbtn_normalsigma->setChecked(true);				
				ui->radbtn_highsigma->setEnabled(true);		
				ui->radbtn_lowsigma->setEnabled(true);			

				ui->lbl_atsettings->setStyleSheet(ENABLESTYLE);
				ui->lbl_normal_sigma->setStyleSheet(ENABLESTYLE);
				ui->lbl_highsigma->setStyleSheet(ENABLESTYLE);
				
				ui->lbl_lowsigma->setStyleSheet(ENABLESTYLE);
				auto index = ui->le_keydensity->currentIndex();
				pItemModel->item(index)->setForeground(QBrush(QColor(255, 255, 255)));
				
				
			}
			else
			{
				
				
				ui->radbtn_normalsigma->setEnabled(false);
				ui->radbtn_normalsigma->setChecked(false);
			
				ui->radbtn_highsigma->setEnabled(false);
				ui->radbtn_highsigma->setChecked(false);
				
				ui->radbtn_lowsigma->setEnabled(false);
				ui->radbtn_lowsigma->setChecked(false);
				
				ui->lbl_atsettings->setStyleSheet(UNENABLESTYLE);
				ui->lbl_normal_sigma->setStyleSheet(UNENABLESTYLE);
				ui->lbl_highsigma->setStyleSheet(UNENABLESTYLE);
				
				ui->lbl_lowsigma->setStyleSheet(UNENABLESTYLE);
				auto index = ui->le_keydensity->currentIndex();
				pItemModel->item(index)->setForeground(QBrush(QColor(165, 165, 165)));
				
			}
		}

		void PosSigmaDia::setGcpModeStatus(bool bIsSelect)
		{
			ui->radbtn_gcp->setChecked(bIsSelect);
			if (bIsSelect)
			{
				ui->radbtn_gcp->setEnabled(true);
			}
			
		}

		void PosSigmaDia::setPosModeStatus(bool bIsSelect)
		{
			ui->radbtn_gps->setChecked(bIsSelect);
			if (bIsSelect)
			{
				ui->radbtn_gcp->setEnabled(true);
			}
			
		}

		

		void PosSigmaDia::setGCPResult(int nGCPTotal, int nMarkedGCP, int nGCPMarkedPhoto)
		{
			nGCPTotalNum = nGCPTotal<0 ? 0 : nGCPTotal;
			nMarkedGCPNum = nMarkedGCP < 0 ? 0 : nMarkedGCP;
			nGCPMarkedPhotoNum = nGCPMarkedPhoto < 0 ? 0 : nGCPMarkedPhoto;
			QString tempStr;
			
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				tempStr = str2qstr(std::string("控制点数量:") );
				tempStr += QString::number(nGCPTotalNum);
			}
			else
				tempStr = "GCP total:" + QString::number(nGCPTotalNum);
			    ui->lbl_totalgcpinfo->setText(tempStr);
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				tempStr = str2qstr(std::string("已刺点数量:")) + QString::number(nMarkedGCPNum, 10);
				
			}
			else
				tempStr = "Marked GCP:" + QString::number(nMarkedGCPNum, 10);
			ui->lbl_gcpmarked->setText(tempStr); 
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				tempStr = str2qstr(std::string("刺点总影像数:")) + QString::number(nGCPMarkedPhotoNum, 10);
			}
			else
				tempStr = "GCP Marked Photos:" + QString::number(nGCPMarkedPhotoNum, 10);
			ui->lbl_gcpmarkedphoto->setText(tempStr);
		}
		void PosSigmaDia::print()
		{
			
			std::cout << "ui->radbtn_gcp->isEnabled() " << ui->radbtn_gcp->isEnabled() << " ui->radbtn_gcp->isChecked()  " << ui->radbtn_gcp->isChecked() << std::endl;
			std::cout << "ui->radbtn_gps->isEnabled() " << ui->radbtn_gps->isEnabled() << " ui->radbtn_gps->isChecked()  " << ui->radbtn_gps->isChecked() << " " << ui->radbtn_gps->isChecked() << std::endl;
			std::cout << "ui->radbtn_highsigma->isEnabled() " << ui->radbtn_highsigma->isEnabled() << " ui->radbtn_highsigma->isChecked()  " << ui->radbtn_highsigma->isChecked() << std::endl;
			std::cout << "ui->radbtn_lowsigma->isEnabled() " << ui->radbtn_lowsigma->isEnabled() << " ui->radbtn_lowsigma->isChecked()  " << ui->radbtn_lowsigma->isChecked() << std::endl;
			std::cout << "ui->radbtn_normalsigma->isEnabled() " << ui->radbtn_normalsigma->isEnabled() << " ui->radbtn_normalsigma->isChecked()  " << ui->radbtn_normalsigma->isChecked() << std::endl;
		
		}
		void PosSigmaDia::setGCPDataEnable(bool bIsEnable)
		{
			if (bIsEnable)
			{
				/*QString tempStr;*/
				ui->lbl_gcpmarked_result->setStyleSheet(ENABLESTYLE);
				ui->lbl_totalgcpinfo->setEnabled(true);
				ui->lbl_totalgcpinfo->setStyleSheet(ENABLESTYLE);
			/*	tempStr = "GCP total:" + QString::number(nGCPTotalNum);
				ui->lbl_totalgcpinfo->setText(tempStr);*/
				ui->lbl_gcpmarked->setStyleSheet(ENABLESTYLE);
				ui->lbl_gcpmarked->setEnabled(true);
				/*tempStr = "Marked GCP:" + QString::number(nMarkedGCPNum, 10);
				ui->lbl_gcpmarked->setText(tempStr);*/
				ui->lbl_gcpmarkedphoto->setStyleSheet(ENABLESTYLE);
				ui->lbl_gcpmarkedphoto->setEnabled(true);
				/*tempStr = "GCP Marked Photos:" + QString::number(nGCPMarkedPhotoNum, 10);
				ui->lbl_gcpmarkedphoto->setText(tempStr);*/
				ui->widget_gcpmarkedinfo->setEnabled(true);
			}
			else
			{
				ui->lbl_gcpmarked_result->setStyleSheet(UNENABLESTYLE);
				/*ui->lbl_totalgcpinfo->setText("GCP total:0");*/
				ui->lbl_totalgcpinfo->setStyleSheet(UNENABLESTYLE);
				ui->lbl_totalgcpinfo->setEnabled(false);
				ui->lbl_gcpmarked->setStyleSheet(UNENABLESTYLE);
				/*ui->lbl_gcpmarked->setText("Marked GCP:0");*/
				ui->lbl_gcpmarked->setEnabled(false);
				ui->lbl_gcpmarkedphoto->setStyleSheet(UNENABLESTYLE);
				/*ui->lbl_gcpmarkedphoto->setText("GCP Marked Photos:0");*/
				ui->lbl_gcpmarkedphoto->setEnabled(false);
				ui->widget_gcpmarkedinfo->setEnabled(false);
			}
		}

		void PosSigmaDia::SetWidgetStatusByRadioCheked(alignmode_e mode)
		{
			bool posaccenbale = false;
			bool pairselecmodeenbale = false;
			bool gcplabelenable = false;
			if (mode== alignmode_e::ALIGN_WITHPOS)
			{
				gcplabelenable = false;
				posaccenbale = true;
				pairselecmodeenbale = true;

			}
			else if (mode == alignmode_e::ALIGN_WITHGCP)
			{
				gcplabelenable = true;

				posaccenbale = false;
				pairselecmodeenbale = false;


			}
			else if (mode == alignmode_e::ALIGN_ARBITRARY)
			{
				gcplabelenable = false;
 
				posaccenbale = false;
				pairselecmodeenbale = true;


			}
			setGCPDataEnable(gcplabelenable);
			SetPairSelectionModeEnable(pairselecmodeenbale);
			SetATSettingLabelEnable(posaccenbale | pairselecmodeenbale);
			SetPOSaccuracyEnable(posaccenbale);
		}

		void PosSigmaDia::slot_ControlRadioButton()
		{
			alignmode_e mode;
			if (ui->radbtn_gps == dynamic_cast<QRadioButton*>(sender()))
			{
				mode = alignmode_e::ALIGN_WITHPOS;
				
			}
			else if (ui->radbtn_gcp == dynamic_cast<QRadioButton*>(sender()))
			{
				
				mode = alignmode_e::ALIGN_WITHGCP;
				
			}
			else if (ui->radbtn_arbitrary == dynamic_cast<QRadioButton*>(sender()))
			{
				
				mode = alignmode_e::ALIGN_ARBITRARY;
				
			}
			SetWidgetStatusByRadioCheked(mode);
		}
		void PosSigmaDia::Initpolicy(QComboBox* cmb, AI3D::CORE::ATDefinition& definition, const policies_object_e& object)
		{
			std::tuple<policies_e, std::vector<std::pair<policies_e, bool>>> policy =
				definition.GetPolicy(object);

			

			policies_e default = std::get<0>(policy);
			auto list = std::get<1>(policy);
			int current_index = -1;
			for (int idx = 0; idx < list.size(); idx++)
			{
				if (list[idx].first == default)
				{
					current_index = idx;
					break;
				}
			}
			if (current_index == -1)
			{
				//致命错误
			}
			
			for (int index = 0; index < list.size(); index++)
			{
				auto iter = list[index];
				policies_e policy = iter.first;
				bool can_itemused = iter.second;
				std::string stringforshow = Policy_String.at(policy);
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					stringforshow = Policy_String_Chinese.at(policy);
					
				}
				
				cmb->addItem(str2qstr(stringforshow));
				if (!can_itemused)
				{
					QVariant zerov(0);
					cmb->setItemData(index, zerov, Qt::UserRole - 1);
					cmb->setItemData(index, QBrush(QColor(192, 192, 192)), Qt::BackgroundRole);
				}
			}
			bool allinvalid = std::all_of(list.begin(), list.end(), [](std::pair<policies_e, bool> item) {return !item.second; });
			if (allinvalid)
			{
				cmb->setEnabled(false);
			}

			cmb->setCurrentIndex(current_index);
		}
		

		void PosSigmaDia::InitEstimationPolicies(AI3D::CORE::ATDefinition& definition)
		{
			
			//connect(ui->cmb_tiepointspolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_TiepointsPolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_tiepointspolicy, definition, policies_object_e::PO_OBJ_TIEPOINTS);

			//connect(ui->cmb_Posepolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_PosePolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_Posepolicy, definition, policies_object_e::PO_OBJ_POSE);

			//connect(ui->cmb_fpolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_FPolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_fpolicy, definition, policies_object_e::PO_OBJ_F);

			//connect(ui->cmb_ppapolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_PPAPolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_ppapolicy, definition, policies_object_e::PO_OBJ_PPA);

			//connect(ui->cmb_rdispolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_RdisPolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_rdispolicy, definition, policies_object_e::PO_OBJ_RDIS);

			//connect(ui->cmb_tdispolicy, &QComboBox::currentTextChanged, this, &PosSigmaDia::Slot_TdisPolicyChanged, Qt::QueuedConnection);
			Initpolicy(ui->cmb_tdispolicy, definition, policies_object_e::PO_OBJ_TDIS);

			

		}
		


		void PosSigmaDia::slot_RadioButton()
		{

			if (ui->radbtn_normalsigma == dynamic_cast<QRadioButton*>(sender()))
			{
				//ui->widget_Custom->hide(); 
				at_options_.sfmsettings.pos_sigma.x() = 2;
				at_options_.sfmsettings.pos_sigma.y() = 2;
				at_options_.sfmsettings.pos_sigma.z() = 5;
				ui->radbtn_highsigma->setChecked(false);
				ui->radbtn_lowsigma->setChecked(false);

			}
			else if (ui->radbtn_highsigma == dynamic_cast<QRadioButton*>(sender()))
			{
				//ui->widget_Custom->hide();
				at_options_.sfmsettings.pos_sigma.x() = 0.05;
				at_options_.sfmsettings.pos_sigma.y() = 0.05;
				at_options_.sfmsettings.pos_sigma.z() = 0.1;
				ui->radbtn_normalsigma->setChecked(false);
				ui->radbtn_lowsigma->setChecked(false);

			}
			else if (ui->radbtn_lowsigma == dynamic_cast<QRadioButton*>(sender()))
			{
				//ui->widget_Custom->hide();
				at_options_.sfmsettings.pos_sigma.x() = 10;
				at_options_.sfmsettings.pos_sigma.y() = 10;
				at_options_.sfmsettings.pos_sigma.z() = 10;
				ui->radbtn_normalsigma->setChecked(false);
				ui->radbtn_highsigma->setChecked(false);
			}
			

		}

		void PosSigmaDia::slot_getParam()
		{
			if (ui->BtnOK == dynamic_cast<QPushButton*>(sender())) 
			{
				

				//自由网点选
				if (ui->radbtn_gps->isChecked())
				{
					at_options_.align_mode = sfm_align_mode_e::ALIGN_WITHPOS;
					
				}
				else if (ui->radbtn_gcp->isChecked())
				{
					
					at_options_.align_mode = sfm_align_mode_e::ALIGN_WITHGCP;
				}
				else if (ui->radbtn_arbitrary->isChecked())
				{
					at_options_.align_mode = ALIGN_ARBITRARY;
				}

			
				at_options_.feature_num = ui->le_keydensity->currentText().toInt();
				
				at_options_.reconstruct_mode = (pair_selection_mode_e)ui->le_pairselection_mode->currentIndex();
				
				auto& policies = at_options_.sfmsettings.bapolicies;
			
				std::string f_text = qstr2str(ui->cmb_fpolicy->currentText());
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.f_policy_ = String_Policy_Chinese.at(f_text);
				}else
					policies.f_policy_ = String_Policy.at(f_text);
				
				std::string tiepoints_text = qstr2str(ui->cmb_tiepointspolicy->currentText());//.QString::toStdString();
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.tiepoints_policy_ = String_Policy_Chinese.at(tiepoints_text);
				}
				else
					policies.tiepoints_policy_ = String_Policy.at(tiepoints_text);
				std::string pose_text = qstr2str(ui->cmb_Posepolicy->currentText());// .QString::toStdString();
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.pos_policy_ = String_Policy_Chinese.at(pose_text);
				}
				else
					policies.pos_policy_ = String_Policy.at(pose_text);
				std::string ppa_text = qstr2str(ui->cmb_ppapolicy->currentText());//.QString::toStdString();
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.ppa_policy_ = String_Policy_Chinese.at(ppa_text);
					
				}
				else
					policies.ppa_policy_ = String_Policy.at(ppa_text);
				std::string rdis_text = qstr2str(ui->cmb_rdispolicy->currentText());// .QString::toStdString();
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.rdis_policy_ = String_Policy_Chinese.at(rdis_text);
				}
				else
					policies.rdis_policy_ = String_Policy.at(rdis_text);
				std::string tdis_text = qstr2str(ui->cmb_tdispolicy->currentText());// .QString::toStdString();
				
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					policies.tdis_policy_ = String_Policy_Chinese.at(tdis_text);
				}
				else
					policies.tdis_policy_ = String_Policy.at(tdis_text);
				this->accept();
			}
			else if (ui->BtnCancle == dynamic_cast<QPushButton*>(sender()))
			{
				this->close();
			}

			this->hide();

		}

		std::tuple<double, double, double, int> PosSigmaDia::getData()
		{
			return std::make_tuple(sigmaX, sigmaY, sigmaZ, featureNum);
		}

	}
}