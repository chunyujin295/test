/**
  * @file      Reconstration.h
  * @brief     AddSigGcp 增加单个GCP弹出界面类
  * @details
  * @author    李跃
  * @attention
  */
#ifndef _AI3D_GUI_ADDSIGGCP_H_
#define _AI3D_GUI_ADDSIGGCP_H_

#include <QDialog>
#include "ui_AddSigGcpWgt.h"
namespace AI3D
{
	namespace GUI
	{
		class AddSigGcp : public QDialog
		{
			Q_OBJECT

		public:
			AddSigGcp(QDialog* parent = Q_NULLPTR);
			~AddSigGcp();

			void Init(QString srsdefination, bool bSetCurrentItem4Recent = false);


			void Slot_Btn_Ok();
			void Slot_Btn_Cancle();

			QStringList getPosList(){ return vlist; };
			QString GetSrsName();
			void SetSrsName(QString str);
//			void Slot_SrsItemChanged();

		public slots:
			void Slot_SrsItemChanged(QString srsname);
			void Slot_SrsSelected(QString& srs);
			void Slot_SrsRestore();

		private:
			Ui::AddSigGcpWgt ui;
			QStringList vlist;
			QString previous_srs;

			QString saved_srsdefination;

		};
	}
}
#endif