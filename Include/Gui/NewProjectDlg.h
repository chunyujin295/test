/**
  * @file      Reconstration.h
  * @brief     新建工程弹出界面
  * @details
  * @author    李跃
  * @attention
  */

#ifndef NEWPROJECTDLG_H
#define NEWPROJECTDLG_H

#include <QWidget>
#include <QDialog>
#include "ui_NewProjectWgt.h"

namespace AI3D
{
    namespace GUI
    {


        class NewProjectDlg : public QDialog
        {
            Q_OBJECT

        public:
            explicit NewProjectDlg(QString& pathName);
            ~NewProjectDlg();

            inline const QString& GetProjectName() { return _projectName; }
            inline const QString& GetProjectPath() { return _projectPath; }
        private:
            void Init(const QString& pathName);
            void CreateConnections();
        private:
            Ui::NewProjectDlg* ui;

        private:
            QString _projectName;
            QString _projectPath;
        };
    }
}
#endif // NEWPROJECTDLG_H
