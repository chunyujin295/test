#include "Gui/NewprojectDlg.h"
#include <QFileDialog>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include "Gui/message_box.h"
#include "Core/TaskDef.h"
#include "Core/BlockObject.h"
#include "Util/TaskProcess.h"

namespace AI3D
{
    namespace GUI
    {
        NewProjectDlg::NewProjectDlg(QString& pathName) :
            ui(new Ui::NewProjectDlg)
        {
            ui->setupUi(this);

            Init(pathName);

        }

        NewProjectDlg::~NewProjectDlg()
        {
            delete ui;
        }

        void NewProjectDlg::Init(const QString& pathName)
        {
            // remove the help button
            setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
            //setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
            //set the default display path

            if(CheckUsingNoChinesePathVersion())
                ui->le_name->setAttribute(Qt::WA_InputMethodEnabled, false);

            if (pathName == "")
                ui->le_path->setText(/*TRIGLobal::projectPath*/"C:\\data\\Projects");
            else
                ui->le_path->setText(pathName);
            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                ui->le_path->setToolTip(str2qstr(std::string("请选择全路径地址!")));
            }
            else
            {
                ui->le_path->setToolTip("Please select a full path!");
            }
            if (!QDir(ui->le_path->text()).exists())
            {
                if (!QDir().mkpath(ui->le_path->text()))
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        Message_Box::warning(this, "错误","创建工程路径失败");
                    }
                    else
                    {
                        Message_Box::warning(this, "error", "mkpath projectPath false");
                    }
                }
            }

            CreateConnections();

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                ui->label_title->setText(str2qstr(std::string("新建工程")));
                ui->label->setText(str2qstr(std::string("工程名称")));
                ui->label_2->setText(str2qstr(std::string("工程路径")));
                ui->btn_push_ok->setText(str2qstr(std::string("确定")));
                ui->btn_push_cancel->setText(str2qstr(std::string("取消")));
            }
            this->resize(512, 400);
        }

        void NewProjectDlg::CreateConnections()
        {
            connect(ui->btn_push_ok, &QPushButton::clicked, [=]() {
                if (ui->le_name->text().trimmed().isEmpty())
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        ui->label_status->setText(str2qstr(std::string("工程名不能为空")));
                    }
                    else
                    {
                        ui->label_status->setText(str2qstr(std::string("Project name can't be empty")));
                    }
                    return;
                }

                QDir dir;
                if (!dir.exists(ui->le_path->text() + "/"))
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        ui->label_status->setText(str2qstr(std::string("路径无效，请重新输入")));
                    }
                    else
                    {
                        ui->label_status->setText(str2qstr(std::string("Path is invalid, please input again")));
                    }
                    return;
                }

                QDir fileDir;
                if (dir.exists(ui->le_path->text() + "/" + ui->le_name->text())) {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        ui->label_status->setText(str2qstr(std::string("目录已存在，请重新输入")));
                    }
                    else
                    {
                        ui->label_status->setText(str2qstr(std::string("Dir already exists, please input again")));
                    }
                    return;
                }

                QString path = ui->le_path->text() + "/" + ui->le_name->text();
                dir.mkpath(path);
                _projectName = ui->le_name->text();
                _projectPath = ui->le_path->text();
                this->accept();
                });
            connect(ui->btn_push_cancel, &QPushButton::clicked, [=]() {
                this->reject();
                });
            connect(ui->btn_tool_filechoose, &QToolButton::clicked, [=]() {
                QFileDialog fd;
                //        fd.setOption(QFileDialog::ShowDirsOnly, true);
                fd.setFileMode(QFileDialog::Directory);
                fd.setDirectory(ui->le_path->text());
                if (fd.exec() != QFileDialog::Accepted)
                    return;
                QString path = fd.selectedFiles().first();
                ui->le_path->setText(path);
                });

        }
    }
}

/*
Check current application's actual ability in depth with processing chinese projectname and chinese projectPath.

while input chinse path name:
   1)has created  two different project path,but the correct path is empty,the other one with illegal pathname has all the files needed inside).
   2)inside the other project path with illegal pathname,though mok project file exists,its filename still contains illegal characters.
   3)check the content from the mok file above,the field 'Project Name' is ok with correct chinese information(that should be gbk encoded).
   4)both  'Log' and 'previews' are empty now,not sure whether it is in normal state now.
   5)Block_1 is ready,and has necessary files without illegal content.
   6)Block_2:
      (1)job_xxxx directory/task_def_x.json:
          ATJson has correct chinese project path
          projectPath has correct chinese project path and chinese project name
      (2)Block_2.blk:
          ATJson has correct chinese project path(for source_data.json)
      (3)feedback.json
          ok.Msg will be tested with chinese information later.
      (4)time_job_xxx.json
          ok.no chinese information inside now.
   7)jobs/job_xxxxx.json
      always laying inside the pending directory of job queue,seems that no engine has got it successfully.
      all the fields with chinese information(in fact only parts of ProjectPath have chinese information) is ok,including 
            chinese project name(part of a project mok filename) and chinese project path.
      doesn't affect job queue if new at job submitted as if the job file does not exist.
   8)Right clicking the block item of current project tree and choosing 'Open folder...' goes to unexpected directory such as documents,not current project folder.

   steps to be done in order:
   1)only create one project directory
   2)the project directory created above must be correct without illegal characters.
   3)all the operations(such as read,write,tranverse and etc.) on the project directory should be worked well.
*/