#include "Gui/ProgressBarCom.h"
#include <QRadioButton>

#include <QStyleOption>
#include <QPainter>
#include <QBitmap>
#include <windows.h>

ProgressCom::ProgressCom(QWidget*parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	
	/*if (nullptr != parent)
	{
		int parentWidth = parent->width();
		int parentHeight = parent->height();
		int proWidth = this->width();
		int proHeight = this->height();
		QPoint movePoint(parentWidth / 2 - proWidth / 2, parentHeight / 2 - proHeight / 2);
		this->move(movePoint);
	}*/
}

ProgressCom::~ProgressCom()
{

}

void ProgressCom::paintEvent(QPaintEvent* p1)
{
	//绘制样式
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式

	QBitmap bmp(this->size());
	bmp.fill();
	QPainter painter(&bmp);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawRoundedRect(bmp.rect(), 6, 6);
	setMask(bmp);

}

void ProgressCom::setLabelText(QString str)
{
	ui.label->setText(str);

}



void ProgressCom::setValue_1(int num)
{
	//int nWidth = (280 * num) / 100;
	//ui.label_2->setGeometry(10, 14, nWidth, 10);
	//QString tempStr = QString("%1%").arg(num);
	//ui.label_3->setText(tempStr);
	//ui.label_3->setGeometry(nWidth + 14, 10, 42, 16);
	//this->show();
}

void ProgressCom::setValue(int num)
{
	int nWidth = (280 * num) / 100;
	QString tempStr = QString("%1%").arg(num);
	ui.label_3->setText(tempStr);
	ui.label_3->setGeometry(nWidth + 20, 10, 42, 16);
	ui.progressBar->setGeometry(16, 14, nWidth, 10);
	ui.progressBar->setRange(0, num);
	ui.progressBar->setValue(num);
	this->update();
}


void ProgressCom::setTitleVisble(bool bIsVisble)
{
	ui.label->setVisible(bIsVisble);
}

void ProgressCom::setMaxValue(int num)
{
	//ui.progressBar->setMaximum(num);

}

void ProgressCom::setMinValue(int num)
{
	//ui.progressBar->setMinimum(num);
}






