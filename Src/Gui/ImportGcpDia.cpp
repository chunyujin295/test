#include "Gui/ImportGcpDia.h"
#include <QPushButton>
#include <QTextStream>
#include<QFileDialog>
#include<QTableWidgetItem>
#include<QStandardItemModel>
#include<QTableView>
#include<QHeaderView>
#include<QMessageBox>
#include<QTextCodec>
#include "Core/CoordinateSystem.h"
#include "Gui/GlobalStruct.h"
#include "Gui/message_box.h"
#include "Gui/MohackerWin.h"
#include "Util/TaskProcess.h"

#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"

#include "Gui/ProjectionSelectionTreeWidget.h"
#endif // USE_AI3D_PROJ

namespace AI3D
{
	namespace GUI
	{
		ImportGcpDia::ImportGcpDia(QDialog* parent)
			: QDialog(parent)
		{
			/*QTextCodec* codec = QTextCodec::codecForName("utf-8");
			QTextCodec::setCodecForLocale(codec);*/
			ui.setupUi(this);
			ui.tableView->horizontalHeader()->setHighlightSections(false);
			m_RecordMap.clear();

			ui.splitter->setStretchFactor(0, 1);
			ui.splitter->setStretchFactor(1, 1);
			ui.splitter->setStretchFactor(2, 1);
			ui.splitter->setStretchFactor(3, 1);
			ui.splitter->setVisible(true); 
#ifdef USE_AI3D_PROJ
		/*	ui.comboBox->setMinimumSize(QSize(450, 28));
			ui.comboBox->setMaximumSize(QSize(1000, 28));*/
#endif
			//this->setWindowTitle("SubmitAT");
			//this->setWindowFlags(Qt::Dialog);

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui.Btn_OK->setText(tr("确定"));
				ui.Btn_Cancel->setText(tr("取消"));
				
				ui.label_6->setText(tr("空间参考系统"));
				ui.label_7->setText(tr("导入控制点文件"));

				ui.label_6->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
				ui.label_7->setAlignment(Qt::AlignRight);

				ui.label_3->setText(tr("从文本文件导入控制点"));
				ui.label_2->setText(tr("数据预览："));
				ui.label_4->setText(tr(" 控制点总数："));
			}

			vlist.clear();
			model = nullptr;
			comboxSelf = nullptr;
			this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

			connect(ui.btn_tool_filechoose, &QPushButton::clicked, this, &ImportGcpDia::openFileDialog);
			connect(ui.Btn_Cancel, &QPushButton::clicked, this, &ImportGcpDia::close);
			connect(ui.Btn_OK, &QPushButton::clicked, this, &ImportGcpDia::ReadFile);
			connect(ui.comboBox, &QComboBox::currentTextChanged, this, &ImportGcpDia::Slot_SrsItemChanged, Qt::QueuedConnection);

			InitSrss();					
			InitTableTitleList();
		}

		ImportGcpDia::~ImportGcpDia()
		{

		}

		void ImportGcpDia::InitTableTitleList()
		{
			comboxitemlist.clear();
			comboxitemlist_display.clear();
			p_List.clear();


			p_List.push_back(ui.comboBox_1);
			p_List.push_back(ui.comboBox_2);
			p_List.push_back(ui.comboBox_3);
			p_List.push_back(ui.comboBox_4);

			///comboxitemlist << "Role" << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";
			comboxitemlist << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";

			if (BlockObject::isChineseVersion())
			{
				//comboxitemlist_display << "角色" << "名称" << "精度" << "纬度" << "高程"; // << "忽略";
				comboxitemlist_display << "名称" << "精度" << "纬度" << "高程"; // << "忽略";
				// std::cout << "combobox size:" << ui.comboBox_1->count() << std::endl;
			}
			else
			{
				comboxitemlist_display << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";
			}

			for (int i = 0; i < 4; i++)
			{
				//std::cout << " i: " << i << std::endl;
				QComboBox* pComboBox = p_List.at(i);

				if (pComboBox->count() >= 6)
				{
					pComboBox->removeItem(pComboBox->count() - 1);
				}

				pComboBox->removeItem(0);

				for (int j = 0; j < comboxitemlist.size(); j++)
				{
					pComboBox->setItemData(j, comboxitemlist_display.at(j), Qt::DisplayRole);
					pComboBox->setItemData(j, comboxitemlist.at(j), Qt::UserRole);
					//std::cout << "j:" << j << " / " << comboxitemlist.size() << " / " << comboxitemlist_display.size()
					//	<< " " << comboxitemlist.at(j).toStdString() << " " << comboxitemlist_display.at(j).toStdString() << std::endl;

				}
			}
		}

		void ImportGcpDia::InitSrss(bool bSetCurrentItem4Recent)
		{
			//设置默认焦点
//			std::cout << "import gcp dia c." << std::endl;

			ui.comboBox->blockSignals(true);

			ui.comboBox->setFocus();
			ui.comboBox->setStyleSheet("QComboBox {background-color: rgb(18, 18, 18);color: rgb(255, 255, 255);font: 14px 'Arial'; }");
			ui.comboBox->clear();
			QStringList listCoords_default;
			QStringList listCoords_Recent;
			QStringList listCoords_More;
			
#ifdef USE_AI3D_PROJ
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				listCoords_default << "默认";
				listCoords_Recent << "最近";
				listCoords_More << "更多";
				listCoords_More << (MohackerWin::prependIndentation() + "空间参考系统数据库");
			}
			else
			{
				listCoords_default << "Default";
				listCoords_Recent << "Recent";
				listCoords_More << "More";
				listCoords_More << (MohackerWin::prependIndentation() + "Spatial reference system database");
			}

			AI3D::PROJ::CoordinateReferenceSystem localcrs(std::string("Local:0"));
			AI3D::PROJ::CoordinateReferenceSystem wgscrs(std::string("EPSG:4326"));

			listCoords_default << (MohackerWin::prependIndentation() + QString::fromStdString(localcrs.GetDescription())) 
				<< (MohackerWin::prependIndentation() +QString::fromStdString(wgscrs.GetDescription() + "(" + wgscrs.GetAuthID() + ")"));
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
///					listCoords_Recent << (MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription()));
///				}
///				else
				{
					listCoords_Recent << (MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")"));
				}

				//std::cout << " iter " << iter.GetDescription() + "(" + iter.GetAuthID() + ")" << std::endl;
				if (count == 7)
				{
					break;
				}
				count++;
			}

			ui.comboBox->setEditable(false);
			ui.comboBox->addItems(listCoords_default);
			ui.comboBox->addItems(listCoords_Recent);
			ui.comboBox->addItems(listCoords_More);

			bool bHaveSetCurrentIndex = false;
			int retint = ui.comboBox->findText(MohackerWin::prependIndentation() + QString::fromStdString(localcrs.GetDescription()), Qt::MatchStartsWith);//暂时选用以开始
			///std::cout << retint << std::endl;

			if (bSetCurrentItem4Recent && listCoords_Recent.size() > 1)
			{
//				std::cout << "import gcp dia a." << std::endl;
				ui.comboBox->setCurrentIndex(listCoords_default.size() + 1);
///				previous_srs = listCoords_Recent.at(1).trimmed();
				previous_srs = MohackerWin::stripPrependIndentation(listCoords_Recent.at(1));
				bHaveSetCurrentIndex = true;
			}
			else
			{

//				std::cout << "import gcp dia b." << std::endl;
				if (retint >= 0)
				{
					ui.comboBox->blockSignals(true);
					ui.comboBox->setCurrentIndex(retint);



					///std::string str = "(" + localcrs.GetAuthID() + ")";
					///previous_srs = QString::fromStdString(str);
					/// 
					previous_srs = ui.comboBox->itemData(retint, Qt::DisplayRole).toString();
					previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(previous_srs);


					bHaveSetCurrentIndex = true;
				}
				else
				{
					//attention Add by chy 此处还没有逻辑因为不知道加啥逻辑
				}
			}

			if (!bSetCurrentItem4Recent && !bHaveSetCurrentIndex)
			{
				//默认84坐标系
				// need this previous code according to existing business requirements?


				ui.comboBox->setCurrentIndex(1);

				previous_srs = MohackerWin::stripPrependIndentation(ui.comboBox->itemData(1,Qt::DisplayRole).toString());
			}

			ui.comboBox->blockSignals(false);
#else
			
			listCoords_default << "Default";
			
			listCoords_Recent << "Common";
		
			listCoords_More << "More";
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
				else if (it->first == "Common")//
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
			ui.comboBox->setEditable(false);
			ui.comboBox->addItems(listCoords_default);
			ui.comboBox->addItems(listCoords_Recent);
			ui.comboBox->addItems(listCoords_More);



			//默认84坐标系
			ui.comboBox->setCurrentIndex(1);
#endif


			QModelIndex index_default = ui.comboBox->model()->index(0, 0);
			QVariant v_0(0);
			ui.comboBox->model()->setData(index_default, v_0, Qt::UserRole - 1);
			QModelIndex index_recent = ui.comboBox->model()->index(listCoords_default.size(), 0);
			QVariant v_2(0);
			ui.comboBox->model()->setData(index_recent, v_2, Qt::UserRole - 1);
			QModelIndex index_more = ui.comboBox->model()->index(listCoords_default.size() + listCoords_Recent.size(), 0);
			QVariant v_12(0);
			ui.comboBox->model()->setData(index_more, v_12, Qt::UserRole - 1);

			QStandardItemModel* pItemModel = qobject_cast<QStandardItemModel*>(ui.comboBox->model());
			QFont fontText;
			fontText.setPixelSize(14);
			fontText.setFamily(QStringLiteral("Arial"));
			fontText.setBold(false);
			for (int i = 0; i < ui.comboBox->count(); i++) {
				pItemModel->item(i)->setFont(fontText);
			}
			QFont fontTitle = fontText;
			fontTitle.setBold(true);
			pItemModel->item(0)->setFont(fontTitle);
			pItemModel->item(listCoords_default.size())->setFont(fontTitle);
			pItemModel->item(listCoords_default.size() + listCoords_Recent.size())->setFont(fontTitle);
		}

		void ImportGcpDia::openFileDialog()
		{

			//打开选择文件对话框存在以下问题：在msvc2015 32bit编译，会出现错误
			QString oldStr = QFileInfo(oldFileName).absolutePath();

			QString title = tr("Open solution file");
			QString file_type = tr("pos file(*.txt)");

			if (BlockObject::isChineseVersion())
			{
				title = "打开控制点文件";
				file_type = "控制点文件(*.txt)";
			}

			//QFileDialog fd(nullptr, tr("Open solution file"), oldStr, tr("pos file(*.txt)"));
			QFileDialog fd(nullptr, title, oldStr, file_type);
			fd.setAcceptMode(QFileDialog::AcceptOpen);
			fd.setFileMode(QFileDialog::ExistingFile);
			fd.setViewMode(QFileDialog::Detail);

			if (QDialog::Accepted != fd.exec()) {
				return;
			}

			if (model != nullptr)
			{
				//model->clear();
				int num =  model->rowCount();
				for (int i = model->rowCount();i >= 0 ;i--)
				{
					model->removeRow(0);
				}
			}

			posFile_path = fd.selectedFiles().first();
			ui.le_path->setText(posFile_path);
			showList(posFile_path);				
		}

		void ImportGcpDia::setOldFileName(QString strFile)
		{
			oldFileName = strFile;
		}

		QString ImportGcpDia::getFilePath()
		{
			return posFile_path;
		}

		QString ImportGcpDia::GetSrsName()
		{
			QString str = ui.comboBox->currentText();
			///str = str.trimmed();
			str = MohackerWin::stripPrependIndentation(str);
			return str;
		}

		void  ImportGcpDia::Slot_SrsItemChanged(QString srsname)
		{
//			std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " " << ui.comboBox->currentIndex() << std::endl;
			///srsname = srsname.trimmed();
			srsname = MohackerWin::stripPrependIndentation(srsname);



			if (1 && (srsname == "Spatial reference system database" || srsname == "空间参考系统数据库"))
			{
				

				QString _srsname;

				int idx = ui.comboBox->currentIndex();
				if (idx >= 0 && idx <= ui.comboBox->count())
					_srsname = ui.comboBox->itemData(idx).toString();

				_srsname = previous_srs;

				AI3D::PROJ::CoordinateReferenceSystem crs;
				//			crs.createFromString("EPSG:4413");

				bool bFoundLocalENU = false;
				bool bFoundValidENUCrs = false;


				std::cout << "current.text:" << _srsname.toStdString() << std::endl;

				int startAuthIdPos = _srsname.lastIndexOf("(");
				int endAuthIdPos = _srsname.lastIndexOf(")");

				/*
				if (_srsname.contains("ENU", Qt::CaseInsensitive))
				{
					crs.CreateFromENUDefinition(_srsname);
				}
				else if (_srsname.contains(MohackerWin::localENUPrefix(), Qt::CaseInsensitive))
				{
					std::cout << "current.text1:" << _srsname.toStdString() << std::endl;
					bFoundLocalENU = true;
				}
				else */
				if (_srsname.contains(MohackerWin::localSRS(), Qt::CaseInsensitive))
				{
					//std::cout << "current.text1:" << _srsname.toStdString() << std::endl;
					//bFoundLocalENU = true;
					QString authId = "Local:0";
					crs.createFromString(authId);
				}
				else if (startAuthIdPos >= 0 && endAuthIdPos >= 0 && startAuthIdPos < endAuthIdPos)
				{
//					std::cout << "current.text2:" << _srsname.toStdString() << std::endl;
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
//						std::cout << "current.text22:" << _srsname.toStdString() << std::endl;
						crs.createFromString(authId);

					}

					if (crs.isValid())
					{
//						std::cout << "gcp dia/valid definition:" << crs.description().toStdString() << " authid:" << crs.authid().toStdString() << std::endl;
					}
					else
					{
//						std::cout << "gcp invalid." << std::endl;
					}
				}
				else
				{
//					std::cout << "current.text3:" << _srsname.toStdString() << std::endl;
					crs.CreateFromENUDefinition(_srsname);
				}

				///AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound, (bFoundLocalENU ? _srsname : ""));
				AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound, (bFoundValidENUCrs ? crs.description() : ""),
					(bFoundValidENUCrs ? crs.authid() : ""));

				// crsSelected
				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsSelected, this, &ImportGcpDia::Slot_SrsSelected);
				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsRestore, this, &ImportGcpDia::Slot_SrsRestore);
//				std::cout << "before importing gcp qgswidget." << std::endl;
				if (bFoundLocalENU)
				{
					/// 
				}
				else
				{
					qgsWidget->setCrs(crs);
				}

				///qgsWidget->setFixedSize(qMin(this->width(),578),qMin(this->height(),650));
				///qgsWidget->setFixedSize(578, 650);
				qgsWidget->setFixedSize(1130, 810);

				qgsWidget->show();

				if(bFoundLocalENU)
					qgsWidget->selectCrsByName(QString("Local East-North-Up (ENU)"));

//				std::cout << "after importing gcp qgswidget." << std::endl;
				return;
			}

			previous_srs = srsname;

			comboxitemlist.clear();
			comboxitemlist_display.clear();

			QString currSrs = ui.comboBox->currentText();
			///currSrs = currSrs.trimmed();
			currSrs = MohackerWin::stripPrependIndentation(currSrs);

			///if (ui.comboBox->currentText().left(3) != "WGS")

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				if (currSrs.left(3) != "WGS")
				{

///					comboxitemlist << "Role" << "Name" << "X" << "Y" << "Z";
///					comboxitemlist_display << "角色" << "名称" << "X" << "Y" << "Z"; // << "忽略";
					comboxitemlist << "Name" << "X" << "Y" << "Z";
					comboxitemlist_display << "名称" << "X" << "Y" << "Z"; // << "忽略";
				}
				else
				{
///					comboxitemlist << "Role" << "Name" << "Longitude" << "Latitude" << "Height";
///					comboxitemlist_display << "角色" << "名称" << "经度" << "纬度" << "高程"; // << "忽略";

					comboxitemlist << "Name" << "Longitude" << "Latitude" << "Height";
					comboxitemlist_display << "名称" << "经度" << "纬度" << "高程"; // << "忽略";

				}

			}
			else
			{
				if (currSrs.left(3) != "WGS")
				{

//					comboxitemlist << "Role" << "Name" << "X" << "Y" << "Z";
//					comboxitemlist_display << "Role" << "Name" << "X" << "Y" << "Z";

					comboxitemlist << "Name" << "X" << "Y" << "Z";
					comboxitemlist_display << "名称" << "X" << "Y" << "Z";
				}
				else
				{
///					comboxitemlist << "Role" << "Name" << "Longitude" << "Latitude" << "Height";
///					comboxitemlist_display << "Role" << "Name" << "Longitude" << "Latitude" << "Height";

					comboxitemlist << "Name" << "Longitude" << "Latitude" << "Height";
					comboxitemlist_display << "名称" << "经度" << "维度" << "高程";
				}

			}

			int i = 0;
			for (auto combox : p_List)
			{
				combox->blockSignals(true);
				combox->clear();
				combox->addItems(comboxitemlist);

				for (int j = 0; j < comboxitemlist.size(); j++)
				{
					combox->setItemData(j, comboxitemlist_display.at(j), Qt::DisplayRole);
					combox->setItemData(j, comboxitemlist.at(j), Qt::UserRole);
				}


///				combox->setCurrentIndex(i + 1);
				combox->setCurrentIndex(i);

				combox->blockSignals(false);
				i++;
			}
		}

		void ImportGcpDia::Slot_SrsSelected(QString& srs)
		{
//			std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			ui.comboBox->blockSignals(true);
			InitSrss(true);
			ui.comboBox->blockSignals(false);
		}

		void ImportGcpDia::Slot_SrsRestore()
		{

//			std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

			if (!previous_srs.isEmpty())
			{
				ui.comboBox->blockSignals(true);
				///ui.comboBox->setCurrentText(previous_srs);
				ui.comboBox->setCurrentText((MohackerWin::prependIndentation() + previous_srs));
				ui.comboBox->blockSignals(false);
			}
		}

		bool ImportGcpDia::showList(QString fileName)
		{
			QFile file(fileName);
			if (!file.open(QFile::ReadOnly | QFile::Text))
				return false;

			QTextStream fileStream(&file);
			fileStream.setAutoDetectUnicode(true);
			int numRow = 1;
			QVector<QStringList> v_StringList;
			int errrowNum = 0;
			int specialNum = 0;
///			QRegExp qreg_exp_special("[(、~！@#$&%*()-+={}':\";',\\[\\]【】<>《》?￥……（）—；‘’：“”，.？\^!)]");
			QRegExp qreg_exp_special("[(、~！@#$&%*()-+={}':\";',\\[\\]【】<>《》?￥……（）—；‘’：“”，？\^!)]");
			QRegExp qreg_exp_special_2("[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b]");
			QRegExp qreg_exp_special_3("[\p{P}/u]");
			QRegExp   qreg_special_character_test("[^(\u4E00-\u9FA5A-Za-z0-9_+'.')]");
			QRegExp   qreg_special_character_test2("\W");
			//QRegExp   qreg_special_character_test_1("[^(\u4E00-\u9FA5) | ^]");
			while (!fileStream.atEnd())
			{
				
				QString str = fileStream.readLine();
				str.replace(QRegExp("[\\s]+"), " ");
				str.replace(QRegExp(","), " ");
				str.replace(QRegExp(";"), " ");
				QStringList listalll = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
				///for (auto listitem:listalll)
				///{
				///	QString local = QString::fromLocal8Bit(listitem.toStdString().c_str());
					
				///}
				QStringList list;
				if (listalll.size() > 4)
				{
					list  = listalll.mid(0,4);
				}
				else
				{
					list = listalll;
				}
				if (list.size() < 3)
				{
					errrowNum++;
					continue;
				}
				if (list.size() == 3)
				{
					//判断每项是不是纯（数字+点）
					int itemtrueNum = 0;
					for (auto item: list)
					{
						if (IsPointOrNumber(item))
						{
							itemtrueNum++;
						}
					}
					if (itemtrueNum == 3)
					{
						QString str = QString("Control point %1").arg(numRow);
						list.push_front(str);
					}else if (itemtrueNum < 3)
					{
						continue;
					}
						
					//errrowNum++;
					//continue;
				}
				numRow++;

				bool has_specialflag = false;
				for (auto listitem:list)
				{
					if (/*listitem.contains(qreg_exp_special) || listitem.contains(qreg_exp_special_2) ||*/ listitem.contains(qreg_exp_special))
					{
						specialNum++;
						has_specialflag = true;
						break;
					}
						

				}
				if (!has_specialflag)
				{
					v_StringList.push_back(list);
				}
				
			}
			file.close();
			
			
			if (errrowNum > 0)
			{
				Message_Box::critical(this, tr("Error"), tr("%1 error(s),insufficient number of columns.").arg(errrowNum));
			}
			if (specialNum > 0)
			{
				Message_Box::critical(this, tr("Error"), tr("%1 error(s),There is special character.").arg(specialNum));
			}
			if (v_StringList.isEmpty())
			{
				///ui.lab_posNum->setText(0);
				ui.lab_posNum->setText("");
				return false;
			}
			int count = 4;//4列
			//初始化标头
			showTitle(count);
			//comboxSelf = new QSensorIntDelegate;
			if (!v_StringList.isEmpty())
			{
				QStringList strlist;
				comboxSelf = new QSensorIntDelegate;
				QString srs = GetSrsName();
				srs_s current_srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(srs.toStdString());
				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					if (current_srs.type == GEOGRAPHIC)
					{

						strlist << "影像名" << "经度" << "纬度" << "高程";
					}
					else
					{
						strlist << "影像名" << "X" << "Y" << "Z";

					}
				}
				else
				{
					if (current_srs.type == GEOGRAPHIC)
					{
						strlist << "Name" << "Longitude" << "Latitude" << "Height";
					}
					else
					{
						strlist << "Name" << "X" << "Y" << "Z";
					}
				}
				model = new QStandardItemModel(this);
				model->setHorizontalHeaderLabels(strlist);

				QHeaderView* myHead = new QHeaderView(Qt::Horizontal);
				myHead->setItemDelegate(comboxSelf);

				ui.tableView->setHorizontalHeader(myHead);
				ui.tableView->horizontalHeader()->setVisible(false);
				ui.tableView->horizontalHeader()->setItemDelegate(comboxSelf);

				QItemSelectionModel* itemselection = new QItemSelectionModel(model);
				ui.tableView->setSelectionModel(itemselection);
				ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
				ui.tableView->setItemDelegateForRow(0, comboxSelf);
				
				//插入tablewidget中
				int num = 0;
				for (auto perRow : v_StringList) {

					if (!perRow.isEmpty()) {

						QList<QStandardItem*> items;
						items.clear();
						for (int i = 0; i < v_StringList.first().size(); i++) {


							
							QStandardItem* item = new QStandardItem;
							item->setText(perRow.at(i));
							item->setTextAlignment(Qt::AlignCenter);
							item->setEditable(false);
							items.append(item);
							//model->appendRow(item);


						}
						//model->appendRow(items);
						model->insertRow(model->rowCount(), items);
					}
					else {
						continue;
					}
					num++;
				}
				ui.tableView->verticalHeader()->hide();

				ui.tableView->setModel(model);
				//ui.tableView->sortByColumn(0, Qt::AscendingOrder);

				ui.lab_posNum->setText(QString::number(v_StringList.size()));
				

			}
			return true;


		}

		void ImportGcpDia::showTitle(int num)
		{
			QWidget* w; // = new QWidget;
			while (ui.splitter->count() > 0)
			{
				w = ui.splitter->widget(0);
				w->setParent(nullptr);   // 必须
				w->deleteLater();
			}

			comboxitemlist.clear();
			comboxitemlist_display.clear();

			m_RecordMap.clear();
			p_List.clear();

			if (MohackerWin::stripPrependIndentation(ui.comboBox->currentText()).left(3) != "WGS")
			{
				///comboxitemlist << "Role" << "Name" << "X" << "Y" << "Z"; // << "Ignore";
				comboxitemlist << "Name" << "X" << "Y" << "Z"; // << "Ignore";
				if (BlockObject::isChineseVersion())
				{
					///comboxitemlist_display << "角色" << "名称" << "X" << "Y" << "Z"; // << "忽略";
					comboxitemlist_display << "名称" << "X" << "Y" << "Z"; // << "忽略";
				}
				else
				{
					///comboxitemlist_display << "Role" << "Name" << "X" << "Y" << "Z"; // << "Ignore";
					comboxitemlist_display << "Name" << "X" << "Y" << "Z"; // << "Ignore";

				}
			}
			else
			{

///				comboxitemlist << "Role" << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";
				comboxitemlist << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";
				if (BlockObject::isChineseVersion())
				{
///					comboxitemlist_display << "角色" << "名称" << "经度" << "纬度" << "高程"; // << "忽略";
					comboxitemlist_display << "名称" << "经度" << "纬度" << "高程"; // << "忽略";
				}
				else
				{
///					comboxitemlist_display << "Role" << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";
					comboxitemlist_display << "Name" << "Longitude" << "Latitude" << "Height"; // << "Ignore";

				}
			}

			int count = ui.splitter->count();
		
			for (int i = 0; i < num; i++) {
				QComboBox* combox_New = new QComboBox;
				combox_New->setStyleSheet("font: 14px 'Arial';");

				combox_New->blockSignals(true);
				combox_New->addItems(comboxitemlist);

				for (int j = 0; j < comboxitemlist.size(); j++)
				{
					combox_New->setItemData(j, comboxitemlist_display.at(j), Qt::DisplayRole);
					combox_New->setItemData(j, comboxitemlist.at(j), Qt::UserRole);
				}


///				combox_New->setCurrentIndex(i + 1);
				combox_New->setCurrentIndex(i);


				combox_New->blockSignals(false);

				ui.splitter->addWidget(combox_New);
				p_List.push_back(combox_New);
				///m_RecordMap.insert(i, comboxitemlist.at(i + 1));
				m_RecordMap.insert(i, comboxitemlist.at(i));
			}
			for (int j = 0; j < ui.splitter->count(); j++) {
				ui.splitter->setStretchFactor(j, 1);
			}
			//创建连接
			for (auto perCombox : p_List) {
				
				connect(perCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImportGcpDia::recordIndex);
			}
		}

		void ImportGcpDia::recordIndex(int index)
		{

			QObject* obj = sender(); //返回发出信号的对象，用QObject类型接收
			QComboBox* combox_tmp = qobject_cast<QComboBox*>(obj);
			if (p_List.contains(combox_tmp)) {
				//获取splitter的index及qcombox的index的string
				for (int i = 0; i < ui.splitter->count(); i++) {

					//获取index
					if (combox_tmp = qobject_cast<QComboBox*>(ui.splitter->widget(i))) {
						//QString str = combox_tmp->currentText();
						QString str = combox_tmp->itemData(combox_tmp->currentIndex(), Qt::UserRole).toString();
						m_RecordMap.insert(i, str);
					}

				}
			}
		}

		void ImportGcpDia::ReadFile()
		{
#if 1 
			/*this->accept();
			return;*/
#endif
			QSet<QString> setCategory;
			int iComboBoxCount = 0;

			for (int i = 0; i < ui.splitter->count(); i++)
			{
				QComboBox* pComboBox = qobject_cast<QComboBox*>(ui.splitter->widget(i));
				if (pComboBox != nullptr)
				{
					iComboBoxCount++;
					setCategory.insert(pComboBox->itemData(pComboBox->currentIndex(), Qt::UserRole).toString());
				}
			}

			if (setCategory.count() != iComboBoxCount)
			{
				//std::cout << "category select error." << std::endl;
				for (auto& str : setCategory)
				{
					std::cout << str.toStdString() << std::endl;
				}

				if (BlockObject::isChineseVersion())
				{
					Message_Box::critical(this, tr("错误"), tr("类型选择存在重复."));
				}
				else
				{
					Message_Box::critical(this, tr("Error"), tr("Has duplicated category option(s)."));
				}

				return;
			}

			QFile file(getFilePath());
			if (!file.open(QFile::ReadOnly | QFile::Text))
			{
				//emit error(CBlockWgt::FILE_OPNE_FAIL);
				return;
			}

			QTextStream fileStream(&file);
			int numRow = 0;
			QList<int> sortItem;
			int nameIt = 0, longitudeIt = 0, latitudeIt = 0, heightIt = 0, headingIt = 0, pitchingIt = 0, rollingIt = 0, ignore = 0;
			for (auto it = m_RecordMap.begin(); it != m_RecordMap.end(); it++) {
				if (it.value() == "Name") {
					nameIt = it.key();					
				}
				else if (it.value() == comboxitemlist.at(1)) {
					longitudeIt = it.key();					
				}
				else if (it.value() == comboxitemlist.at(2))
					latitudeIt = it.key();
				else if (it.value() == comboxitemlist.at(3))
					heightIt = it.key();
				else if (it.value() == "Heading")
					headingIt = it.key();
				else if (it.value() == "Pitching")
					pitchingIt = it.key();
				else if (it.value() == "Rolling")
					rollingIt = it.key();
				else if (it.value() == "Role")
				{
					ignore = 100;
				}
				else
				{
					continue;
				}
			}

			sortItem << nameIt << longitudeIt << latitudeIt << heightIt << headingIt << pitchingIt << rollingIt;

			if (model == nullptr || model->rowCount() <= 0 )
			{
				///ui.Btn_OK->setEnabled(false);
				return;
			}

			///ui.Btn_OK->setEnabled(true);

			for (int i = 0; i < model->rowCount(); i++) {
				QStringList tempList;
				for (int j = 0; j < model->columnCount(); j++) {
					tempList << (model->item(i, sortItem.at(j))->text());
				}
				if (tempList[0].indexOf(QRegExp("[A-Za-z]")) == -1) {
					//名字不存在,提示

				}
				vlist.push_back(tempList);
			}

			file.close();
			this->accept();

		}


	}
}


