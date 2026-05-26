#include "Gui/CommonDelDia.h"


namespace AI3D
{
	namespace GUI
	{
		CommonDelDia::CommonDelDia(QDialog* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
		//this->setWindowTitle("SubmitAT");
		//this->setWindowFlags(Qt::Dialog);
		connect(ui.pushButton, &QPushButton::clicked, this, [this]() {

			this->accept();

			});

		connect(ui.pushButton_2, &QPushButton::clicked, this, [this]() {

			this->close();

			});

		connect(ui.close_btn, &QPushButton::clicked, this, [this]() {

			this->close();

			});
	}

	CommonDelDia::~CommonDelDia()
	{

	}

	void CommonDelDia::SetInfor(QString str)
	{

		ui.label->setText(str);
	}


}

}
