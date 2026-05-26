#include "Gui/TheFirstDlg.h"
#include "Core/BlockObject.h"
#include "Util/TaskProcess.h"

namespace AI3D
{
	namespace GUI
	{
		TheFirstDlg::TheFirstDlg(QDialog* parent)
			: QDialog(parent)
		{
			ui.setupUi(this);
			this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
			connect(ui.pushButton, &QPushButton::clicked, this, [this]() {emit newProject(); });
			connect(ui.pushButton_2, &QPushButton::clicked, this, [this]() {emit openProject();	});
			connect(ui.pushButton_3, &QPushButton::clicked, this, [this]() {this->close(); });

			if (AI3D::CORE::BlockObject::isChineseVersion())
			{
				ui.pushButton->setText(str2qstr(std::string("新建工程")));
				ui.pushButton_2->setText(str2qstr(std::string("打开工程")));
				ui.pushButton_3->setText(str2qstr(std::string("关闭")));
			}
		}

		TheFirstDlg::~TheFirstDlg()
		{

		}
	}
}
