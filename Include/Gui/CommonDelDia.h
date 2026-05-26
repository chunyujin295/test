/**
  * @file      Reconstration.h
  * @brief     TheFirstDlg 通用关闭弹窗类
  * @details
  * @author    李跃
  * @attention
  */
#ifndef _AI3D_GUI_COMMONDELDIA_H_
#define _AI3D_GUI_COMMONDELDIA_H_

#include <QDialog>
#include "ui_CommonDelDia.h"
namespace AI3D
{
	namespace GUI
	{
		class CommonDelDia : public QDialog
		{
			Q_OBJECT

		public:
			CommonDelDia(QDialog* parent = Q_NULLPTR);
			~CommonDelDia();
			void SetInfor(QString str);
		private:
			Ui::CommonDelUI ui;

		};
	}
}
#endif