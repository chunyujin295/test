#include "Gui/ExportRecColmapDia.h"
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
		ExportRecColmapDia::ExportRecColmapDia(QDialog* parent)
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
				ui.label_3->setText(tr("重建导出Colmap"));
				ui.label_8->setText(tr("输入Tile文件夹"));
				ui.label_8->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

				ui.label_7->setText(tr("导出文件夹"));

				ui.Btn_OK->setText(tr("确定"));
				ui.Btn_Cancel->setText(tr("取消"));
			}

			this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
			connect(ui.btn_tool_filechoose_2, &QPushButton::clicked, this, &ExportRecColmapDia::openInFileDialog);
			connect(ui.btn_tool_filechoose, &QPushButton::clicked, this, &ExportRecColmapDia::openOutFileDialog);
			//connect(ui.Btn_OK, &QPushButton::clicked, this, &ExportXmlDia::sig_VecString);
			connect(ui.Btn_Cancel, &QPushButton::clicked, this, &ExportRecColmapDia::close);
			connect(ui.Btn_OK, &QPushButton::clicked, this, [=]() {

				///if (QMessageBox::No == Message_Box::question(this, "delete", "Are you sure to delete the current block!", Message_Box_Type::Question_Yes_No))
				///{
				///	return;
				///}
				emit WriteRecColmapFile();
				});
			connect(ui.label_OpenFile, &QLabel::linkActivated, this, &ExportRecColmapDia::Slot_LinkActivated_Label_OpenFolder);
			connect(ui.Btn_Close, &QPushButton::clicked, this, &ExportRecColmapDia::close);
			connect(ui.le_path, &QLineEdit::editingFinished, this, &ExportRecColmapDia::Slot_FilePathEdited, Qt::DirectConnection);


			ui.le_path->setEnabled(false);

			/// note: which srs definition should be passed on? has been already called by other function in main window class.
			//InitSrss();
			///InitSrss("");

			SetTabWidgetIndex(0);
			//EmportEnuID = 0;
			bFilePathHasEdited = false;
		}

		ExportRecColmapDia::~ExportRecColmapDia()
		{

		}

		void ExportRecColmapDia::openInFileDialog()
		{
			QString srcDirPath = QFileDialog::getExistingDirectory(
				this, "输出路径", currFilePath.isEmpty() ? initFilePath : currFilePath);

			if (srcDirPath.isEmpty())
			{
				return;
			}

			currFileName = initFileName;
			currFilePath = srcDirPath;

			// adjust fileName based on currFilePath and currFileName;
			SetInFileFullName(srcDirPath);
		}


		void ExportRecColmapDia::openOutFileDialog()
		{
			QString dstDirPath = QFileDialog::getExistingDirectory(
				this, "输出路径", currFilePath.isEmpty() ? initFilePath : currFilePath);

			if (dstDirPath.isEmpty())
			{
				return;
			}

			currFileName = initFileName;
			currFilePath = dstDirPath;

			// adjust fileName based on currFilePath and currFileName;
			SetOutFileFullName(dstDirPath);
		}

		void ExportRecColmapDia::setOldFileName(QString strFile)
		{
			oldFileName = strFile;
		}

		/*QString ExportRecColmapDia::getFilePath()
		{
			return posFile_path;
		}*/


		void ExportRecColmapDia::SetOutFileFullName(QString filename)
		{
			ui.le_path->setText(filename);
		}

		void ExportRecColmapDia::SetInFileFullName(QString filename)
		{
			ui.le_path_2->setText(filename);
		}
		

		void ExportRecColmapDia::ShowProgress()
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

		void ExportRecColmapDia::FinhshWriteXml(int result)
		{
#if 1
			ui.tabWidget->setCurrentIndex(1);
			ui.splitter->setVisible(false);
			QString resultText = "";
			if (result == 0) {
				resultText = "转换完成";
			}
			else {
				resultText = "文件或路径不存在，或已损坏";
			}
			ui.label_complete->setText(resultText);
			ui.label_complete->setVisible(true);
			ui.label_OpenFile->setVisible(true);
			ui.Btn_Close->setVisible(true);
#endif
		}

		void ExportRecColmapDia::Slot_LinkActivated_Label_OpenFolder()
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

		void ExportRecColmapDia::SetTabWidgetIndex(int index)
		{
			ui.tabWidget->setCurrentIndex(index);
		}


		void ExportRecColmapDia::SetInitFileName(QString filePath,QString fileName)
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

		void ExportRecColmapDia::SetFileParentDir(QString filePath)
		{
			if (filePath.isEmpty())
				return;

			currFileName = initFileName;
			currFilePath = filePath;

			// auto adjust based on rule.
			AutoGeneratedFullFilePath();
		}

		void ExportRecColmapDia::Slot_FilePathEdited()
		{
			// split the text from edit control into fileName and path,check whether the path is valid later.
			bFilePathHasEdited = true;
		}

		void ExportRecColmapDia::AutoGeneratedFullFilePath()
		{
			disconnect(ui.le_path, &QLineEdit::editingFinished, this, &ExportRecColmapDia::Slot_FilePathEdited);
			
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

			connect(ui.le_path, &QLineEdit::editingFinished, this, &ExportRecColmapDia::Slot_FilePathEdited, Qt::DirectConnection);
		}
	}
}
