#ifndef IMAGEPROPERTYFORM_H
#define IMAGEPROPERTYFORM_H

#include <QWidget>
#include <Eigen/Core>

namespace Ui {
class ImagePropertyForm;
}

class ImagePropertyForm : public QWidget
{
    Q_OBJECT

public:
    explicit ImagePropertyForm(QWidget *parent = 0);
    ~ImagePropertyForm();
	void setFileName(QString);
	void setPos(Eigen::Vector3d centerXYZ,Eigen::Matrix3d RotationMatrix_);
	void setGroup(double,double,double);
private:
    Ui::ImagePropertyForm *ui;
};

#endif 
