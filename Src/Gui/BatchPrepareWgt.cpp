#include "Gui/BatchPrepareWgt.h"
#include <QPushButton>
#include <QDebug>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QTableView>
#include<QHeaderView>
#include "Core/CoordinateSystem.h"
#include "Gui/GlobalStruct.h"
#include <QMessageBox>
#include "Gui/message_box.h"
#include "Util/TaskProcess.h"

namespace AI3D
{
	namespace GUI
	{
		//改进的方向：
		//每次写的焦距可以写进配置文件下一个block可以复用
		//可以根据pos 的name列给prifix赋值并检查

		BatchPrepareDia::BatchPrepareDia(QDialog* parent)
			: QDialog(parent)
		{

			ui.setupUi(this);
			ui.tableView->horizontalHeader()->setHighlightSections(false);
			m_RecordMap.clear();

			ui.le_imgprefix->setPlaceholderText("前缀");
			ui.le_numlength->setPlaceholderText("不超过5");
			ui.le_startnum->setPlaceholderText("数字起始序号");
			// 设置占位符


			// 整数验证器
			QIntValidator* intValidator1 = new QIntValidator(this);
			intValidator1->setRange(1, 10);                 // 设置验证器范围只能是 0 ~ 999
			ui.le_numlength->setValidator(intValidator1);   // 为编辑框设置验证器

			// 整数验证器
			QIntValidator* intValidator2 = new QIntValidator(this);
			intValidator2->setRange(1, 10);                 // 设置验证器范围只能是 0 ~ 999
			ui.le_startnum->setValidator(intValidator2);   // 为编辑框设置验证器

			// 身份证号最多允许输入 18 个字符
		  //  ui->lineEdit_IDNum->setMaxLength(18);

			ui.splitter->setStretchFactor(0, 1);
			ui.splitter->setStretchFactor(1, 1);
			ui.splitter->setStretchFactor(2, 1);
			ui.splitter->setStretchFactor(3, 1);
			ui.splitter->setVisible(true);
			//this->setWindowTitle("SubmitAT");
			//this->setWindowFlags(Qt::Dialog);
			vlist.clear();
			model = nullptr;
			comboxSelf = nullptr;
			
			this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
			connect(ui.btn_posfile_choose, &QPushButton::clicked, this, &BatchPrepareDia::OpenPosFileDialog);
			//connect(ui.btn_gcpfile_choose, &QPushButton::clicked, this, &BatchPrepareDia::OpenGCPFileDialog);
			connect(ui.btn_imagedir_choose, &QPushButton::clicked, this, &BatchPrepareDia::OpenImageDirDialog);
			
			connect(ui.Btn_OK, &QPushButton::clicked, this, &BatchPrepareDia::Slot_GetParam);
			
			//connect(ui.Btn_OK, &QPushButton::clicked, this, &ImportPos::sig_VecString);
			connect(ui.Btn_Cancel, &QPushButton::clicked, this, &BatchPrepareDia::close);
			connect(ui.Btn_OK, &QPushButton::clicked, this, &BatchPrepareDia::Slot_GetParam);
			connect(ui.comboBox, &QComboBox::currentTextChanged, this, &BatchPrepareDia::Slot_SrsItemChanged, Qt::QueuedConnection);
			InitSrss();
			

		}

		BatchPrepareDia::~BatchPrepareDia()
		{

		}


		void BatchPrepareDia::InitSrss()
		{
			//设置默认焦点
			ui.comboBox->setFocus();
			ui.comboBox->setStyleSheet("{background-color: rgb(18, 18, 18);color: rgb(255, 255, 255);font: 14px 'Arial'; }");
			ui.comboBox->clear();
			QStringList listCoords_default;
			listCoords_default << "Default";
			QStringList listCoords_Recent;
			listCoords_Recent << "Common";
			QStringList listCoords_More;
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
			ui.comboBox->setEditable(false);
			ui.comboBox->addItems(listCoords_default);
			ui.comboBox->addItems(listCoords_Recent);
			ui.comboBox->addItems(listCoords_More);

			//默认84坐标系
			ui.comboBox->setCurrentIndex(1);

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

		
		void BatchPrepareDia::OpenGCPFileDialog()
		{
			QString oldStr = ".";// QFileInfo(oldFileName).absolutePath(); */
			QFileDialog fd(nullptr, tr("Open solution file"), oldStr, tr("gcp file(*.txt)"));
			fd.setAcceptMode(QFileDialog::AcceptOpen);
			fd.setFileMode(QFileDialog::ExistingFile);
			fd.setViewMode(QFileDialog::Detail);

			if (QDialog::Accepted != fd.exec())
			{
				return;
			}

			QString file = fd.selectedFiles().first();
			//ui.le_GCPpath->setText(file);
		}
		void BatchPrepareDia::OpenImageDirDialog()
		{
			QFileDialog fd;
			fd.setFileMode(QFileDialog::Directory);
			
			if (fd.exec() != QFileDialog::Accepted)
				return;
			QString path = fd.selectedFiles().first();
			ui.le_image_path->setText(path);
		}

		void BatchPrepareDia::OpenPosFileDialog()
		{

			//打开选择文件对话框存在以下问题：在msvc2015 32bit编译，会出现错误
			QString oldStr = ".";// QFileInfo(oldFileName).absolutePath(); */
			QFileDialog fd(nullptr, tr("Open solution file"), oldStr, tr("pos file(*.txt)"));
			fd.setAcceptMode(QFileDialog::AcceptOpen);
			fd.setFileMode(QFileDialog::ExistingFile);
			fd.setViewMode(QFileDialog::Detail);

			if (QDialog::Accepted != fd.exec())
			{
				return;
			}

			QString file = fd.selectedFiles().first();
			ui.le_posfile->setText(file);


			if (model != nullptr)
			{
				int num = model->rowCount();
				model->clear();
				/*for (int i = model->rowCount(); i >= 0; i--)
				{
					model->removeRow(0);
				}*/
			}
			showList(file);
			
				
		}

		void BatchPrepareDia::setOldFileName(QString strFile)
		{
			oldFileName = strFile;
		}

		/*QString BatchPrepareDia::getFilePath()
		{
			return posFile_path;
		}*/

		std::string BatchPrepareDia::GetImagePath()
		{

			return qstr2str(ui.le_image_path->text());

		}
		std::string BatchPrepareDia::GetPosFile()
		{

			/// todo2:
			return qstr2str(ui.le_posfile->text());

		}
		std::string BatchPrepareDia::GetPrefix()
		{
			return qstr2str(ui.le_imgprefix ->text());
		}
		std::string BatchPrepareDia::GetNumLength()
		{
			return qstr2str(ui.le_numlength->text());
		}
		std::string BatchPrepareDia::GetStartNum()
		{
			return qstr2str(ui.le_startnum->text());
		}
		std::string BatchPrepareDia::GetGCPFile()
		{
			//return ui.le_GCPpath->text().toStdString();
			return "";
		}

		bool BatchPrepareDia::showList(QString fileName)
		{


			QFile file(fileName);
			if (!file.open(QFile::ReadOnly | QFile::Text))
				return false;

			QTextStream fileStream(&file);
			int numRow = 1;
			QVector<QStringList> v_StringList;
			int errrowNum = 0;
			int specialNum = 0;
///			QRegExp qreg_exp_special("[(、~！@#$&%*()-+={}':\";',\\[\\]【】<>《》?￥……（）—；‘’：“”，.？\^!)]");
			QRegExp qreg_exp_special("[(、~！@#$&%*()-+={}':\";',\\[\\]【】<>《》?￥……（）—；‘’：“”，？\^!)]");
			while (!fileStream.atEnd())
			{
				
				QString str = fileStream.readLine();
				str.replace(QRegExp("[\\s]+"), " ");
				str.replace(QRegExp(","), " ");
				str.replace(QRegExp(";"), " ");
				QStringList listall = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
				for (auto listitem : listall)
				{
					QString local = listitem;
					
				}

				QStringList list;
				if (listall.size() > 4)
				{
					list = listall.mid(0, 4);
				}
				else
				{
					list = listall;
				}
				numRow++;
				if (list.size() <= 3)
				{
					LOGW("The Import PosFile %d Row Has Not Enough coloum", numRow - 1);
					errrowNum++;
					continue;
				}
				//if (list.size() == 3)
				//{
				//	//判断每项是不是纯（数字+点）
				//	int itemtrueNum = 0;
				//	for (auto item : list)
				//	{
				//		if (IsPointOrNumber(item))
				//		{
				//			itemtrueNum++;
				//		}
				//	}
				//	if (itemtrueNum == 3)
				//	{
				//		QString str = QString("GPS point %1").arg(numRow);
				//		list.push_front(str);
				//	}
				//	else if (itemtrueNum < 3)
				//	{
				//		continue;
				//	}

				//	//errrowNum++;
				//	//continue;
				//}
				
				bool has_specialflag = false;
				for (auto listitem : list)
				{
					if (listitem.contains(qreg_exp_special))
					{
						LOGW("The Import PosFile Has Special Chracter %d row", numRow - 1);
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
				Message_Box::critical(this, tr("Error"), tr("%1 importation error(s). '\n' The file is insufficient number of columns.").arg(errrowNum));
			}
			if (specialNum > 0)
			{
				Message_Box::critical(this, tr("Error"), tr("%1 importation error(s). '\n' The file has special character.").arg(specialNum));
			}
			if (v_StringList.isEmpty())
			{
				ui.lab_posNum->setText(0);
				return false;
			}


			int count = v_StringList.begin()->size();
			//初始化表头
			showTitle(count);


			//comboxSelf = new QSensorIntDelegate;
			if (!v_StringList.isEmpty())
			{
				QStringList strlist;
				comboxSelf = new QSensorIntDelegate;
				QString srs = GetSrsName();
				srs_s current_srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(srs.toStdString());
				if (current_srs.type == GEOGRAPHIC)
				{
					strlist << "Name" << "Longitude" << "Latitude" << "Height";
				}
				else
				{
					strlist << "Name" << "X" << "Y" << "Z";
				}
				
				model = new QStandardItemModel(this);
				model->setHorizontalHeaderLabels(strlist);

				QHeaderView* myHead = new QHeaderView(Qt::Horizontal);
				myHead->setItemDelegate(comboxSelf);
				//myHead->setho
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

				//ui.tableView->sortByColumn(0,Qt::AscendingOrder);
				ui.lab_posNum->setText(QString::number(v_StringList.size()));
				
			}
			return true;
		}

		void BatchPrepareDia::showTitle(int num)
		{

			QWidget* w = new QWidget;
			while (ui.splitter->count() > 0)
			{
				w = ui.splitter->widget(0);
				w->setParent(nullptr);   // 必须
				w->deleteLater();
			}


			int count = ui.splitter->count();
			
		
			for (int i = 0; i < num; i++) {
				//ui.splitter->
				QComboBox* combox_New = new QComboBox;
				combox_New->setStyleSheet("font: 14px 'Arial';");
				combox_New->addItems(comboxitemlist);
				combox_New->setCurrentIndex(i + 1);
				ui.splitter->addWidget(combox_New);
				p_List.push_back(combox_New);
				m_RecordMap.insert(i, comboxitemlist.at(i + 1));
			}
			for (int j = 0; j < ui.splitter->count(); j++) {
				ui.splitter->setStretchFactor(j, 1);
			}
			//创建连接
			for (auto perCombox : p_List) {

				connect(perCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BatchPrepareDia::recordIndex);
			}
		}

		void BatchPrepareDia::recordIndex(int index)
		{

			QObject* obj = sender(); //返回发出信号的对象，用QObject类型接收
			QComboBox* combox_tmp = qobject_cast<QComboBox*>(obj);
			if (p_List.contains(combox_tmp)) {
				//获取splitter的index及qcombox的index的string
				for (int i = 0; i < ui.splitter->count(); i++) {

					//获取index
					if (combox_tmp = qobject_cast<QComboBox*>(ui.splitter->widget(i))) {
						QString str = combox_tmp->currentText();
						qDebug() << str;
						m_RecordMap.insert(i, str);
					}

				}
			}
		}

		preparetaskinfo_s  BatchPrepareDia::GetParams()
		{
			return params_;
		}

		void BatchPrepareDia::Slot_GetParam()
		{

			if (ui.Btn_OK == dynamic_cast<QPushButton*>(sender()))
			{
			/*	if (!ui.le_GCPpath->text().isEmpty())
				{
					params_.GcpPath = ui.le_GCPpath->text().toStdString();
				}*/
				if (!ui.le_image_path->text().isEmpty())
				{
					params_.ImagePath = qstr2str(ui.le_image_path->text());
				}
				if (!ui.le_posfile->text().isEmpty())
				{
					params_.PosfilePath = qstr2str(ui.le_posfile->text());
				}
				//由于qt不太熟，暂时没考虑输入为空的情形1121chy
				params_.Prefix = qstr2str(ui.le_imgprefix->text());
				params_.NumLength = ui.le_numlength->text().toInt();
				params_.NumStart = ui.le_startnum->text().toInt();
				params_.SRS = GetSrsName().toStdString();
				this->accept();
			}
			else if (ui.Btn_Cancel == dynamic_cast<QPushButton*>(sender()))
			{
				this->close();
			}

			this->hide();

		}

		QString BatchPrepareDia::GetSrsName()
		{
			QString str = ui.comboBox->currentText();
			return str;
		}

		void BatchPrepareDia::Slot_SrsItemChanged()
		{
			
			comboxitemlist.clear();

			if (ui.comboBox->currentText().left(3) != "WGS")
			{
				comboxitemlist <<"Role" <<"Name" <<"X" <<"Y"<<"Z";
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
				combox->setCurrentIndex(i+ 1);
				i++;
			}
			

		}



	}
}

