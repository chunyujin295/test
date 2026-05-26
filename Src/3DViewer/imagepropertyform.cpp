#include "3DViewer/imagepropertyform.h"
#include "ui_imagepropertyform.h"
#include "qfileinfo.h"
#include "qdir.h"

ImagePropertyForm::ImagePropertyForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImagePropertyForm)
{
    ui->setupUi(this);
	
}

ImagePropertyForm::~ImagePropertyForm()
{
    delete ui;
}

void ImagePropertyForm::setFileName(QString str)
{
	QFileInfo info(str);
	
	ui->lineEdit_name->setText(info.baseName());
	ui->lineEdit_dir->setText(info.absolutePath());
	ui->lineEdit_size->setText(QString::number(info.size()));
	
}

void ImagePropertyForm::setPos(Eigen::Vector3d centerXYZ, Eigen::Matrix3d RotationMatrix_)
{
	ui->lineEdit_posx->setText(QString::number(centerXYZ[0],'f',15));
	ui->lineEdit_posy->setText(QString::number(centerXYZ[1], 'f', 15));
	ui->lineEdit_posz->setText(QString::number(centerXYZ[2], 'f', 15));

	QString str =
	QString::number(RotationMatrix_(0, 0), 'f', 15) + ";" +
	QString::number(RotationMatrix_(0, 1), 'f', 15) + ";" +
	QString::number(RotationMatrix_(0, 2), 'f', 15) + ";" +

	QString::number(RotationMatrix_(1, 0), 'f', 15) + ";" +
	QString::number(RotationMatrix_(1, 1), 'f', 15) + ";" +
	QString::number(RotationMatrix_(1, 2), 'f', 15) + ";" +

	QString::number(RotationMatrix_(2, 0), 'f', 15) + ";" +
	QString::number(RotationMatrix_(2, 1), 'f', 15) + ";" +
	QString::number(RotationMatrix_(2, 2), 'f', 15) + ";";

	ui->lineEdit_rotation->setText(str);
	

}

void ImagePropertyForm::setGroup(double FocalLengthPixels, double PrincipalPoint_x, double PrincipalPoint_y)
{
	ui->lineEdit_principal_x->setText(QString::number(PrincipalPoint_x));
	ui->lineEdit_principal_y->setText(QString::number(PrincipalPoint_y));
	ui->lineEdit_focal_length->setText(QString::number(FocalLengthPixels));

}
