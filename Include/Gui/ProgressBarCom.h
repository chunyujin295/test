#pragma once

#include <QDialog>
#include "ui_ProgressCom.h"


class ProgressCom : public QWidget
{
	Q_OBJECT

public:
	ProgressCom(QWidget*parent = Q_NULLPTR);
	~ProgressCom();
	void paintEvent(QPaintEvent* p1);
	void setLabelText(QString str);
	void setValue(int num);
	void setValue_1(int num);
	void setMaxValue(int num);
	void setMinValue(int num);
	void setTitleVisble(bool bIsVisble);
private:
	Ui::ProgressCom ui;
	
};
