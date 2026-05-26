#ifndef _QCOMBOXSELF_H
#define _QCOMBOXSELF_H


#include <QWidget>
#include "ui_QComBoxSelf.h"
//
namespace AI3D
{
	namespace GUI
	{
		class QComBoxSelf : public QWidget
		{
			Q_OBJECT

		public:
			QComBoxSelf(QWidget* parent = Q_NULLPTR);
			~QComBoxSelf();
			void setValue(float num);
			QString getValue();
		private:
			Ui::QComBoxSelf ui;
		};
	}
}
#endif // !1