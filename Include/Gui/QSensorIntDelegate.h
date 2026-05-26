#ifndef QSENSORINTDELEGATE_H
#define QSENSORINTDELEGATE_H


#include <QStyledItemDelegate>
#include <QItemDelegate>

#include <QRegExpValidator>
#include <QLineEdit>
#include <QRegExp> 
namespace AI3D
{
	namespace GUI
	{
		class QSensorIntDelegate : public QStyledItemDelegate
		{
			Q_OBJECT

		public:
			QSensorIntDelegate(QObject* parent = 0);
			~QSensorIntDelegate();


			QWidget* QSensorIntDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			void QSensorIntDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const;
			void QSensorIntDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
			void QSensorIntDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		signals:
			void CurrentIndexChangeSignal(int iRow, int iColumn, QString selectIndexText) const;
		};


		class QWIntLineDeleteFor :public QStyledItemDelegate
		{
			Q_OBJECT

		public:
			explicit QWIntLineDeleteFor(QObject* parent = nullptr) :QStyledItemDelegate(parent)
			{}

			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				QLineEdit* edit = new QLineEdit(parent);
				//edit->setStyleSheet("border-image:url(:/new/prefix1/skin/lineedit.png);");
				edit->setFrame(false);
				edit->setMaxLength(20);
				QRegExp rx("^(([0-9]+\\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\\.[0-9]+)|([0-9]*[1-9][0-9]*)){1,20}$");
				QRegExpValidator* pReg = new QRegExpValidator(rx, nullptr);
				edit->setValidator(pReg);
				return edit;
			}

			void setEditorData(QWidget* editor, const QModelIndex& index) const override
			{
				double value = index.model()->data(index, Qt::EditRole).toDouble();
				QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
				
				lineedit->setText(QString::number(value, 'f', 6));
			}

			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
			{
				//将代理组件的数据保存到数据模型中
				QLineEdit* lineedit = static_cast<QLineEdit*>(editor);
				double value = lineedit->text().toDouble();
				
				model->setData(index, value, Qt::EditRole);
			}

			void  updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{

				editor->setGeometry(option.rect);
			}

		};



	}
}
#endif // DEBUG
