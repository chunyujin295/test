#include "Gui/QComBoxSelf.h"

namespace AI3D
{
	namespace GUI
	{

		QComBoxSelf::QComBoxSelf(QWidget* parent)
			: QWidget(parent)
		{
			ui.setupUi(this);
			setWindowFlags(Qt::WindowTitleHint);
		}

		QComBoxSelf::~QComBoxSelf()
		{
		}

		void QComBoxSelf::setValue(float num)
		{
			//ui.lineEdit->setText(QString::number(num));
		}

		QString QComBoxSelf::getValue()
		{

			return ui.comboBox->currentText();
		}

	}
}