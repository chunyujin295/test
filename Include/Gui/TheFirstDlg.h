/**
  * @file      Reconstration.h
  * @brief     TheFirstDlg 打开主窗口弹出界面类
  * @details
  * @author    李跃
  * @attention
  */
#ifndef _AI3D_GUI_THEFIRSTDLG_H_
#define _AI3D_GUI_THEFIRSTDLG_H_

#include <QDialog>
#include "ui_TheFirstWgt.h"
namespace AI3D
{
	namespace GUI
	{
		class TheFirstDlg : public QDialog
		{
			Q_OBJECT

		public:
			TheFirstDlg(QDialog* parent = Q_NULLPTR);
			~TheFirstDlg();
		signals:
			void newProject();
			void openProject();
		private:
			Ui::TheFirstDiaWgt ui;

		};
	}
}
#endif