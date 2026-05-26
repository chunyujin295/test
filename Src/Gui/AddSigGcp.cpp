#include "Gui/AddSigGcp.h"

#include <QStandardItemModel>
#include "Core/CoordinateSystem.h"
#include "Core/BlockObject.h"
#include "Util/TaskProcess.h"

#include "Gui/MohackerWin.h"

#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"

#include "Gui/ProjectionSelectionTreeWidget.h"
#endif // USE_AI3D_PROJ

namespace AI3D
{
	namespace GUI
	{
		AddSigGcp::AddSigGcp(QDialog* parent)
			: QDialog(parent)
		{
			ui.setupUi(this);
			this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
			//Init();

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui.label->setText("X");
				ui.label_2->setText("Y");
				ui.label_3->setText("Z");
				//ui.label_4->setText(str2qstr(std::string("控制点名称")));
				//ui.btn_OK->setText(str2qstr(std::string("确定")));
				//ui.btn_Cancle->setText(str2qstr(std::string("取消")));
				ui.label_4->setText(tr("控制点名称"));
				ui.btn_OK->setText(tr("确定"));
				ui.btn_Cancle->setText(tr("取消"));
			}

			connect(ui.btn_OK, &QPushButton::clicked, this, &AddSigGcp::Slot_Btn_Ok);
			connect(ui.btn_Cancle, &QPushButton::clicked, this, &AddSigGcp::Slot_Btn_Cancle);
			connect(ui.comboBox, &QComboBox::currentTextChanged, this, &AddSigGcp::Slot_SrsItemChanged, Qt::QueuedConnection);
		}

		AddSigGcp::~AddSigGcp()
		{

		}
		//重点关注一下GCP页卡中此处的实现

		void AddSigGcp::Init(QString srsdefination, bool bSetCurrentItem4Recent)
		{
			if (!bSetCurrentItem4Recent)
			{
				// being called at the first time.
				saved_srsdefination = srsdefination;
			}
			//设置默认焦点
			ui.comboBox->setFocus();
			ui.comboBox->setStyleSheet("{background-color: rgb(18, 18, 18);color: rgb(255, 255, 255);font: 14px 'Arial'; }");
			ui.comboBox->clear();
			QStringList listCoords_default;
			QStringList listCoords_Recent;
			QStringList listCoords_More;

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				//listCoords_default << str2qstr(std::string("默认"));
				//listCoords_Recent << str2qstr(std::string("最近"));
				//listCoords_More << str2qstr(std::string("更多"));
				//listCoords_More << (AI3D::GUI::MohackerWin::prependIndentation() + str2qstr(std::string("空间参考系统数据库")));
				listCoords_default << tr("默认");
				listCoords_Recent << tr("最近");
				listCoords_More << tr("更多");
				listCoords_More << (AI3D::GUI::MohackerWin::prependIndentation() + tr("空间参考系统数据库"));

			}
			else
			{
				listCoords_default << "Default";
				listCoords_Recent << "Recent";
				listCoords_More << "More";
				listCoords_More << (MohackerWin::prependIndentation() + "Spatial reference system database");
			}

#if 000
			auto src_map = AI3D::CORE::CoordinateTransformer::CSG_coordinateSystem_Global();
			for (auto it = src_map.begin(); it != src_map.end(); it++)
			{
				if (it->first == "Default")
				{
					for (auto itsrcname : it->second)
					{
						listCoords_default << (MohackerWin::prependIndentation() + str2qstr(itsrcname.name));
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
			AI3D::PROJ::CoordinateReferenceSystem localcrs(std::string("Local:0"));
			AI3D::PROJ::CoordinateReferenceSystem wgscrs(std::string("EPSG:4326"));

			
			//if (!srsdefination.isEmpty())
			//{
			//	QString strenu = srsdefination.toUpper();
			//	QString strenu_ = srsdefination.left(3);
			//	if (!strenu_.compare("ENU", Qt::CaseInsensitive))
			//	{
			//		/*currcrs.type
			//		currcrs.CreateFromENUDefinition(srsdefination);*/
			//		listCoords_default << (MohackerWin::prependIndentation() + QString::fromStdString(currcrs.GetDescription()));
			//	}
			//	else
			//	{
			//		listCoords_default << (MohackerWin::prependIndentation() + QString::fromStdString(currcrs.GetDescription() + "(" + currcrs.GetAuthID() + ")"));
			//	}

			//}
			/*listCoords_default << (QString(4, QChar(' ')) + QString::fromStdString(localcrs.GetDescription())) << (QString(4, QChar(' ')) + QString::fromStdString(wgscrs.GetDescription() + "(" + wgscrs.GetAuthID() + ")"));
=======*/

			listCoords_default << (MohackerWin::prependIndentation() + QString::fromStdString(localcrs.GetDescription())) 
				<< (MohackerWin::prependIndentation() + QString::fromStdString(wgscrs.GetDescription() + "(" + wgscrs.GetAuthID() + ")"));

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

//				if (descrip.contains("ENU"))
//				{
//					///					listCoords_Recent << (QString(4, QChar(' ')) +QString::fromStdString(iter.GetDescription()));
//					listCoords_Recent << (MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription()));
//				}
//				else
				{
					///					listCoords_Recent << (QString(4, QChar(' ')) + QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")"));
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

			///			int retint = ui.comboBox->findText(QString(4, QChar(' ')) + QString::fromStdString(localcrs.GetDescription()), Qt::MatchStartsWith);//暂时选用以开始
			AI3D::PROJ::CoordinateReferenceSystem currcrs;
			int retint =-1;
			if (srsdefination.isEmpty())
			{
				currcrs = localcrs;

			}
			else
			{
				currcrs.createFromString(srsdefination);
			}
			//	retint = ui.comboBox->findText(MohackerWin::prependIndentation() + QString::fromStdString(localcrs.GetDescription()), Qt::MatchStartsWith);//暂时选用以开始
			
			bool bHaveSetCurrentIndex = false;
				retint = ui.comboBox->findText(MohackerWin::prependIndentation() + QString::fromStdString(currcrs.GetDescription()), Qt::MatchStartsWith);//
			std::cout << retint << std::endl;
			if (currcrs != localcrs && currcrs != wgscrs)
			{
				bSetCurrentItem4Recent = true;
			}
			if (bSetCurrentItem4Recent && listCoords_Recent.size() > 1)
			{
				//std::cout << "import gcp dia a." << std::endl;
				ui.comboBox->blockSignals(true);

				ui.comboBox->setCurrentIndex(listCoords_default.size() + 1);
				ui.comboBox->blockSignals(false);
				///				previous_srs = listCoords_Recent.at(1).trimmed();
				previous_srs = MohackerWin::stripPrependIndentation(listCoords_Recent.at(1));
			}
			else
			{
				if (retint >= 0)
				{
					ui.comboBox->blockSignals(true);
					ui.comboBox->setCurrentIndex(retint);

					ui.comboBox->blockSignals(false);

					///std::string str = "(" + localcrs.GetAuthID() + ")";
					///previous_srs = QString::fromStdString(str);
					previous_srs = ui.comboBox->itemData(retint, Qt::DisplayRole).toString();
					previous_srs = AI3D::GUI::MohackerWin::stripPrependIndentation(previous_srs);

					//if (!srsdefination.isEmpty())
					//{
					//	std::string str = "(" + currcrs.GetAuthID() + ")";
					//	previous_srs = QString::fromStdString(str);
					//}
					//else {
					//	std::string str = "(" + localcrs.GetAuthID() + ")";
					//	previous_srs = QString::fromStdString(str);
					//}

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
				ui.comboBox->blockSignals(true);
				ui.comboBox->setCurrentIndex(1);
				ui.comboBox->blockSignals(false);
				previous_srs = MohackerWin::stripPrependIndentation(ui.comboBox->itemData(1,Qt::DisplayRole).toString());
			}

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

			QRegExp rx("^(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\\.[0-9]+)|([0-9]*[1-9][0-9]*)){1,20}$");
			QRegExpValidator* pReg = new QRegExpValidator(rx, nullptr);
			ui.lineEdit_X->setValidator(pReg);
			ui.lineEdit_Y->setValidator(pReg);
			ui.lineEdit_Z->setValidator(pReg);
		}

		void AddSigGcp::Slot_Btn_Ok()
		{
			vlist << ui.lineEdit_Name->text() << ui.lineEdit_X->text() << ui.lineEdit_Y->text() << ui.lineEdit_Z->text();
			this->accept();
		}

		void AddSigGcp::Slot_Btn_Cancle()
		{
			this->close();
		}

		QString AddSigGcp::GetSrsName()
		{
			QString str = ui.comboBox->currentText();
			str = MohackerWin::stripPrependIndentation(str);
			return str;
		}

		void AddSigGcp::SetSrsName(QString str)
		{
			str = MohackerWin::stripPrependIndentation(str);
			ui.comboBox->clear();
			ui.comboBox->addItem((MohackerWin::prependIndentation() + str));
		}

	void  AddSigGcp::Slot_SrsItemChanged(QString srsname)
	{
		std::cout << "inside " << " " << __FUNCTION__ << " " << __LINE__ << " " << ui.comboBox->currentIndex() << std::endl;
		///srsname = srsname.trimmed();
		srsname = MohackerWin::stripPrependIndentation(srsname);

		if (1 && (srsname == "Spatial reference system database" || srsname == "空间参考系统数据库"))
		{
			std::cout << "inside " << __FUNCTION__ << " " << __LINE__ << " " << ui.comboBox->currentIndex() << std::endl;
			

			QString _srsname;

			int idx = ui.comboBox->currentIndex();
			if (idx >= 0 && idx <= ui.comboBox->count())
				_srsname = ui.comboBox->itemData(idx).toString();

			_srsname = previous_srs;

			AI3D::PROJ::CoordinateReferenceSystem crs;
			//			crs.createFromString("EPSG:4413");

			std::cout << "current.text:" << _srsname.toStdString() << std::endl;

			int startAuthIdPos = _srsname.lastIndexOf("(");
			int endAuthIdPos = _srsname.lastIndexOf(")");
			if (startAuthIdPos >= 0 && endAuthIdPos >= 0 && startAuthIdPos < endAuthIdPos)
			{
				QString authId = _srsname.mid(startAuthIdPos + 1, endAuthIdPos - (startAuthIdPos + 1));
				crs.createFromString(authId);
			}
			else
			{
				crs.CreateFromENUDefinition(_srsname);
			}

			AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this);

			// crsSelected
			connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsSelected, this, &AddSigGcp::Slot_SrsSelected);
			connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsRestore, this, &AddSigGcp::Slot_SrsRestore);
			std::cout << "before importing gcp qgswidget." << std::endl;
			qgsWidget->setCrs(crs);

			///qgsWidget->setFixedSize(qMin(this->width(),578),qMin(this->height(),650));
			///qgsWidget->setFixedSize(578, 650);
			qgsWidget->setFixedSize(1130, 810);


			qgsWidget->show();
			std::cout << "after importing gcp qgswidget." << std::endl;
			return;
		}

		previous_srs = srsname;


#if 0
		comboxitemlist.clear();

		QString currSrs = ui.comboBox->currentText();
		///currSrs = currSrs.trimmed();
		currSrs = MohackerWin::stripPrependIndentation(currSrs);

		///if (ui.comboBox->currentText().left(3) != "WGS")
		if (currSrs.left(3) != "WGS")
		{
			comboxitemlist << "Role" << "Name" << "X" << "Y" << "Z";
		}
		else
		{
			comboxitemlist << "Role" << "Name" << "Longitude" << "Latitude" << "Height";
		}

		int i = 0;
		for (auto combox : p_List)
		{
			combox->clear();
			combox->addItems(comboxitemlist);
			combox->setCurrentIndex(i + 1);
			i++;
		}
#endif
		if (MohackerWin::stripPrependIndentation(ui.comboBox->currentText()).left(3) != "WGS")
		{
			ui.label->setText("X");
			ui.label_2->setText("Y");
			ui.label_3->setText("Z");
		}
		else
		{
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui.label->setText("精度");
				ui.label_2->setText("纬度");
				ui.label_3->setText("高度");
			}
			else
			{
				ui.label->setText("Longitude");
				ui.label_2->setText("Latitude");
				ui.label_3->setText("Height");
			}
		}
	}

	void AddSigGcp::Slot_SrsSelected(QString& srs)
	{
		//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
		ui.comboBox->blockSignals(true);
		Init(srs,true);
		ui.comboBox->blockSignals(false);
	}

	void AddSigGcp::Slot_SrsRestore()
	{
		//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

		if (!previous_srs.isEmpty())
		{
			ui.comboBox->blockSignals(true);
			///ui.comboBox->setCurrentText(previous_srs);
			///ui.comboBox->setCurrentText(QString(4, QChar(' ')) + previous_srs);
			ui.comboBox->setCurrentText((MohackerWin::prependIndentation() + previous_srs));
			ui.comboBox->blockSignals(false);
		}
	}

}

	/*void AddSigGcp::Slot_SrsSelected(QString& srs)
	{
		std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
		ui.comboBox->blockSignals(true);
		Init(true);
		ui.comboBox->blockSignals(false);
	}

	void AddSigGcp::Slot_SrsRestore()
	{
		std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

		if (!previous_srs.isEmpty())
		{
			ui.comboBox->blockSignals(true);
			ui.comboBox->setCurrentText((MohackerWin::prependIndentation() + previous_srs));
			ui.comboBox->blockSignals(false);
		}
	}*/

}


