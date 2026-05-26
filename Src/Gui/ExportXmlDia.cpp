#include "Gui/ExportXmlDia.h"
#include <QPushButton>

#include<QFileDialog>
#include<QTableWidgetItem>
#include<QStandardItemModel>
#include<QHeaderView>
#include<QProcess>
#include<Gui/GlobalStruct.h>
#include<windows.h>
#include<ShellAPI.h>
#include "Util/TaskProcess.h"
#include "Gui/message_box.h"
#include "Gui/MohackerWin.h"

#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "Gui/ProjectionSelectionTreeWidget.h"
#endif // USE_AI3D_PROJ

namespace AI3D
{
	namespace GUI
	{
		ExportXmlDia::ExportXmlDia(QDialog* parent)
			: QDialog(parent)
		{
			ui.setupUi(this);
			
			setWindowModality(Qt::WindowModal);

			ui.label_complete->setVisible(false);
			ui.label_OpenFile->setVisible(false);

			ui.splitter->setStretchFactor(0, 1);
			ui.splitter->setStretchFactor(1, 1);
			ui.splitter->setStretchFactor(2, 1);
			ui.splitter->setStretchFactor(3, 1);
			//ui.splitter->setVisible(false);
			//this->setWindowTitle("SubmitAT");
			//this->setWindowFlags(Qt::Dialog);
			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui.label_3->setText(tr("空三导出"));
				ui.label_6->setText(tr("空间参考系统"));
				ui.label_6->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

				ui.label_7->setText(tr("导出文件"));

				ui.label_3->setText(tr("空三导出"));
				ui.checkBox->setText(tr("导出连接点"));
				ui.Btn_OK->setText(tr("确定"));
				ui.Btn_Cancel->setText(tr("取消"));
			}

			this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
			connect(ui.btn_tool_filechoose, &QPushButton::clicked, this, &ExportXmlDia::openFileDialog);
			//connect(ui.Btn_OK, &QPushButton::clicked, this, &ExportXmlDia::sig_VecString);
			connect(ui.Btn_Cancel, &QPushButton::clicked, this, &ExportXmlDia::close);
			connect(ui.Btn_OK, &QPushButton::clicked, this, [=]() {

				///if (QMessageBox::No == Message_Box::question(this, "delete", "Are you sure to delete the current block!", Message_Box_Type::Question_Yes_No))
				///{
				///	return;
				///}
				emit WriteXmlFile();
				});
			connect(ui.label_OpenFile, &QLabel::linkActivated, this, &ExportXmlDia::Slot_LinkActivated_Label_OpenFolder);
			connect(ui.Btn_Close, &QPushButton::clicked, this, &ExportXmlDia::close);
			connect(ui.le_path, &QLineEdit::editingFinished, this, &ExportXmlDia::Slot_FilePathEdited, Qt::DirectConnection);

			connect(ui.comboBox, &QComboBox::currentTextChanged, this, &ExportXmlDia::Slot_SrsItemChanged, Qt::QueuedConnection);

			ui.le_path->setEnabled(false);

			/// note: which srs definition should be passed on? has been already called by other function in main window class.
			//InitSrss();
			///InitSrss("");

			SetTabWidgetIndex(0);
			EmportEnuID = 0;
			bFilePathHasEdited = false;
		}

		ExportXmlDia::~ExportXmlDia()
		{

		}

		void ExportXmlDia::InitSrss(QString srsdefination, bool bSetCurrentItem4Recent)
		{
			//设置默认焦点
			ui.comboBox->setFocus();
			ui.comboBox->setStyleSheet("QComboBox {background-color: rgb(18, 18, 18);color: rgb(255, 255, 255);font: 14px 'Arial'; }");
			ui.comboBox->clear();

			if (!bSetCurrentItem4Recent)
			{
				// being called at the first time.
				saved_srsdefination = srsdefination;
			}

			srsdefination = MohackerWin::stripPrependIndentation(srsdefination);

			if (srsdefination.toStdString() == NOTGEOREFERENCED)
			{
				//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				ui.comboBox->setEditable(false);
				ui.comboBox->addItem((AI3D::GUI::MohackerWin::prependIndentation() + srsdefination));
			}
			else
			{
#ifdef USE_AI3D_PROJ
				//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
				QStringList listCoords_default;
				QStringList listCoords_Recent;
				QStringList listCoords_More;

				if (AI3D::CORE::BlockObject::isChineseVersion())
				{
					listCoords_default << "默认";
					listCoords_Recent << "最近";
					listCoords_More << "更多";
					///listCoords_More << "空间参考系统数据库";
					listCoords_More << (MohackerWin::prependIndentation() + "空间参考系统数据库");
				}
				else
				{
					listCoords_default << "Default";
					//参照cc,此处纯显示不加入库中
					listCoords_Recent << "Recent";
					listCoords_More << "More";
					///listCoords_More << "Spatial reference system database";
					listCoords_More << (MohackerWin::prependIndentation() + "Spatial reference system database");
				}

				AI3D::PROJ::CoordinateReferenceSystem enucrs;
				if (!srsdefination.isEmpty())
				{
						QString strenu = srsdefination.toUpper();
						QString strenu_ = srsdefination.left(3);
						if (!strenu_.compare("ENU", Qt::CaseInsensitive))
						{
							enucrs.CreateFromENUDefinition(srsdefination);
							listCoords_default << (MohackerWin::prependIndentation() + QString::fromStdString(enucrs.GetDescription()));
						}
					
				}

				auto lists = AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->GetRecentCrs();
				
				int count = 0;
///				std::cout << lists.size() << std::endl;;
				for (auto iter : lists)
				{
///					listCoords_Recent << iter.description();
///					listCoords_Recent << (MohackerWin::prependIndentation() + iter.description());
					listCoords_Recent << (MohackerWin::prependIndentation() + QString::fromStdString(iter.GetDescription() + "(" + iter.GetAuthID() + ")"));
					/// note: should fiter previous srsdefinition which has already been assigned for default category?
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

				if (bSetCurrentItem4Recent && listCoords_Recent.size() > 1)
				{
					ui.comboBox->blockSignals(true);
					ui.comboBox->setCurrentIndex(listCoords_default.size() + 1);
					ui.comboBox->blockSignals(false);
					///previous_srs = listCoords_Recent.at(1);
					previous_srs = MohackerWin::stripPrependIndentation(listCoords_Recent.at(1));
				}
				else
				{
					//默认84坐标系
					ui.comboBox->blockSignals(true);
					ui.comboBox->setCurrentIndex(1);//chy  @attention 需要改为设置为当前的选项
					ui.comboBox->blockSignals(false);
					///previous_srs = ui.comboBox->itemData(1).toString();
					previous_srs = MohackerWin::stripPrependIndentation(ui.comboBox->itemData(1,Qt::DisplayRole).toString());
				}

#else

				QStringList listCoords_default;
				listCoords_default << "Default";
				listCoords_default << srsdefination;
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
		}

		void ExportXmlDia::Slot_SrsItemChanged(QString srsname)
		{
			if (MohackerWin::stripPrependIndentation(saved_srsdefination).toStdString() == NOTGEOREFERENCED)
			{
				// just ignore this signal and return based on previous logic.
				return;
			}

			srsname = MohackerWin::stripPrependIndentation(srsname);

			if (srsname == "Spatial reference system database" || srsname == "空间参考系统数据库")
			{
				AI3D::PROJ::CoordinateReferenceSystem crs;

				QString _srsname;

				int idx = ui.comboBox->currentIndex();
				if (idx >= 0 && idx <= ui.comboBox->count())
					_srsname = ui.comboBox->itemData(idx, Qt::DisplayRole).toString();

				_srsname = previous_srs;
				bool bFoundLocalENU = false;
				bool bFoundValidENUCrs = false;

				int startAuthIdPos = _srsname.lastIndexOf("(");
				int endAuthIdPos = _srsname.lastIndexOf(")");
				
				if (_srsname.contains(MohackerWin::localSRS(), Qt::CaseInsensitive))
				{
					//std::cout << "current.text1:" << _srsname.toStdString() << std::endl;
					//bFoundLocalENU = true;
					QString authId = "Local:0";
					crs.createFromString(authId);
				}
				else if (startAuthIdPos >= 0 && endAuthIdPos >= 0 && startAuthIdPos < endAuthIdPos)
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

///				AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this);
				//QgsProjectionSelectionTreeWidget* qgsWidget = new QgsProjectionSelectionTreeWidget();
				AI3D::PROJ::ProjectionSelectionTreeWidget* qgsWidget = new AI3D::PROJ::ProjectionSelectionTreeWidget(this, AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterHorizontal | AI3D::PROJ::CoordinateReferenceSystemProxyModel::FilterCompound, (bFoundValidENUCrs ? crs.description() : ""),
					(bFoundValidENUCrs ? crs.authid() : ""));

				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsSelected, this, &ExportXmlDia::Slot_SrsSelected);
				connect(qgsWidget, &AI3D::PROJ::ProjectionSelectionTreeWidget::crsRestore, this, &ExportXmlDia::Slot_SrsRestore);

				if (bFoundLocalENU)
				{
					/// 
				}
				else
				{
					qgsWidget->setCrs(crs);
				}

				//	qgsWidget->setFixedSize(578, 650);
				qgsWidget->setFixedSize(1130, 810);
				qgsWidget->show();

				if (bFoundLocalENU)
					qgsWidget->selectCrsByName(QString("Local East-North-Up (ENU)"));

				return;
			}

			previous_srs = srsname;
		}

		void ExportXmlDia::Slot_SrsSelected(QString& srs)
		{
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			ui.comboBox->blockSignals(true);
			InitSrss(saved_srsdefination,true);
			ui.comboBox->blockSignals(false);
		}

		void ExportXmlDia::Slot_SrsRestore()
		{
			//std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
			if (!previous_srs.isEmpty())
			{
				ui.comboBox->blockSignals(true);
				ui.comboBox->setCurrentText((MohackerWin::prependIndentation() + previous_srs));
				ui.comboBox->blockSignals(false);
			}
		}

		void ExportXmlDia::openFileDialog()
		{
#if 1
			//打开选择文件对话框存在以下问题：在msvc2015 32bit编译，会出现错误
			QString oldStr = QFileInfo(oldFileName).absolutePath();
			/*QFileDialog fd(nullptr, tr("Choose block export file name"), oldStr, tr("XML file(*.xml)"));
			fd.setAcceptMode(QFileDialog::AcceptOpen);
			fd.setFileMode(QFileDialog::ExistingFile);
			fd.setViewMode(QFileDialog::Detail);

			if (QDialog::Accepted != fd.exec()) {
				return;
			}*/

			//posFile_path = fd.selectedFiles().first();

			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), \
				/*AI3D::CORE::File::GetParentDir*/(ui.le_path->text().toStdString()).c_str(), \
				tr("XML file(*.xml)"));
			if (fileName != "" && fileName.endsWith(".xml"))
			{
				QFileInfo finfo(fileName);
				QString strAbsoluteFilePath = finfo.absolutePath();
				QString strCompleteBaseName = finfo.completeBaseName();

				qDebug() << fileName << strAbsoluteFilePath << strCompleteBaseName;
			///	ui.le_path->setText(fileName);

				SetOutFileFullName(fileName);
			}
			else
			{
				qDebug() << "other file choose for exportxmldia:" << fileName;
				return;
			}
#else
			QString dstDirPath = QFileDialog::getExistingDirectory(
				this, "Export Directory", currFilePath.isEmpty() ? initFilePath : currFilePath);

			if (dstDirPath.isEmpty())
			{
				return;
			}

			currFileName = initFileName;
			currFilePath = dstDirPath;

			// adjust fileName based on currFilePath and currFileName;
			AutoGeneratedFullFilePath();
#endif
		}

		void ExportXmlDia::setOldFileName(QString strFile)
		{
			oldFileName = strFile;
		}

		QString ExportXmlDia::getFilePath()
		{
			return posFile_path;
		}

		QString ExportXmlDia::GetSrsName()
		{
			///QString str = ui.comboBox->currentText();
			QString str = MohackerWin::stripPrependIndentation(ui.comboBox->currentText());
			return str;
		}

		bool ExportXmlDia::isSelectTiePoint()
		{
			if (ui.checkBox->checkState() == Qt::Checked)
			{
				return true;
			}else
			{
				return false;
			}
		}

		void ExportXmlDia::SetOutFileFullName(QString filename)
		{
			ui.le_path->setText(filename);
		}

		srs_s ExportXmlDia::GetSelectSrs_s()
		{
			QString depictstr = ui.comboBox->currentText();
			depictstr = MohackerWin::stripPrependIndentation(depictstr);
			srs_s current_srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(depictstr.toStdString());

			///QString text = MohackerWin::stripPrependIndentation(ui.comboBox->currentText());
			///text.toUpper();
			///if (text.contains("ENU"))

			if(depictstr.contains("ENU",Qt::CaseInsensitive))
			{
				current_srs.ID = EmportEnuID;
			}

			return current_srs;
		}

		

		void ExportXmlDia::ShowProgress()
		{
#if 1
			ui.tabWidget->setCurrentIndex(1);
			ui.splitter->setVisible(true);
			ui.progressBar->setValue(0);
			ui.label_complete->setVisible(false);
			ui.label_OpenFile->setVisible(false);
			ui.Btn_Close->setVisible(false);
#endif
		}

		void ExportXmlDia::FinhshWriteXml()
		{
#if 1
			ui.tabWidget->setCurrentIndex(1);
			ui.splitter->setVisible(false);
			ui.label_complete->setVisible(true);
			ui.label_OpenFile->setVisible(true);
			ui.Btn_Close->setVisible(true);
#endif
		}

		void ExportXmlDia::Slot_LinkActivated_Label_OpenFolder()
		{

			QString str = ui.le_path->text();

			std::string path = qstr2str(str);

			path = AI3D::CORE::File::GetParentDir(path);
			LOGI(path);
			//QString path = str.left(str.lastIndexOf("/"));
			//QStringList pathList = path.split("/");

			str = str2qstr(path);

			str.replace("/", "\\");

			QProcess::startDetached("explorer",QStringList() << str);
			/*str.prepend("/select,");*/
			

//#ifdef WIN32
//			ShellExecuteA(0, "open", "explorer.exe", LPCSTR(str.toStdString().c_str()), NULL, true);
//#endif // #ifdef WIN32

			
		}

		void ExportXmlDia::SetTabWidgetIndex(int index)
		{
			ui.tabWidget->setCurrentIndex(index);
		}

		void ExportXmlDia::SetCanSelectTiePoint(bool flag)
		{
			ui.checkBox->setVisible(flag);
			
		}

		void ExportXmlDia::SetInitFileName(QString filePath,QString fileName)
		{
			//qDebug() << "setInitFileName:" << filePath << fileName;
			if (fileName.isEmpty() || filePath.isEmpty())
				return;

			//qDebug() << "setInitFileName2:" << filePath << fileName;
			initFileName = fileName;
			initFilePath = filePath;

			currFileName = fileName;
			currFilePath = filePath;

			//qDebug() << "setInitFileName3:" << filePath << fileName;
			// auto adjust based on rule.
			AutoGeneratedFullFilePath();
		}

		void ExportXmlDia::SetFileParentDir(QString filePath)
		{
			if (filePath.isEmpty())
				return;

			currFileName = initFileName;
			currFilePath = filePath;

			// auto adjust based on rule.
			AutoGeneratedFullFilePath();
		}

		void ExportXmlDia::Slot_FilePathEdited()
		{
			// split the text from edit control into fileName and path,check whether the path is valid later.
			bFilePathHasEdited = true;
		}

		void ExportXmlDia::AutoGeneratedFullFilePath()
		{
			disconnect(ui.le_path, &QLineEdit::editingFinished, this, &ExportXmlDia::Slot_FilePathEdited);
			
			fullFilePath = currFilePath + "/" + currFileName + ".xml";
			QString fullFilePathWithoutExtension = currFilePath + "/" + currFileName;

			std::string strfile = qstr2str(fullFilePath);
			strfile = AI3D::CORE::File::EnsureUnifySlash(strfile);

			std::string strfileWithoutExtension = qstr2str(fullFilePathWithoutExtension);
			strfileWithoutExtension = AI3D::CORE::File::EnsureUnifySlash(strfileWithoutExtension);

			fullFilePath = str2qstr(strfile);
			fullFilePathWithoutExtension = str2qstr(strfileWithoutExtension);

			qDebug() << "auto gen:" << fullFilePath << fullFilePathWithoutExtension;
			bool bFoundNotExistingOne = false;

			// if fullFilePath exists,may append "(2)" more times after initBaseFilename(without extension) until finding non-existing one.
			if (QFileInfo(fullFilePath).exists())
			{			
				do
				{
					fullFilePathWithoutExtension += "(2)";
					
					QString testBaseFilename = fullFilePathWithoutExtension + ".xml";
					if (!QFileInfo(testBaseFilename).exists())
					{
						bFoundNotExistingOne = true;
						fullFilePath = testBaseFilename;

						qDebug() << "auto gen:" << fullFilePath << fullFilePathWithoutExtension;
						break;
					}
				} while (true);
			}
			else
			{
				bFoundNotExistingOne = true;
			}

			qDebug() << "auto gen:" << fullFilePath << fullFilePathWithoutExtension;
			ui.le_path->setText(fullFilePath);

			connect(ui.le_path, &QLineEdit::editingFinished, this, &ExportXmlDia::Slot_FilePathEdited, Qt::DirectConnection);
		}
	}
}
