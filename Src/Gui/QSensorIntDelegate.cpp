#include "Gui/QSensorIntDelegate.h"
//#include "Gui/QComBoxSelf.h"
#include <QComboBox>
namespace AI3D
{
	namespace GUI
	{
		QSensorIntDelegate::QSensorIntDelegate(QObject* parent)
		{


		}

		QSensorIntDelegate::~QSensorIntDelegate()
		{
		}

		QWidget* QSensorIntDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			if (index.column() == 1)
			{
				QComboBox* Box = new QComboBox(parent);
				Box->setFixedHeight(option.rect.height());
				Box->setStyleSheet("{color:#FFFFFF;};{font: 14px 'Arial';};{background-color: #323232;}");
				Box->addItem(tr("Control point"));
				Box->addItem(tr("Check point"));
				//Box->addItems(index.model()->data(index, Qt::UserRole).toStringList()); //向数据源申请数据，动态改变combobox的显示内容
				return Box;
			}
			else
			{
				return QStyledItemDelegate::createEditor(parent, option, index);
			}
			//QComboBox* spin = new QComboBox(parent);
			//spin->setStyleSheet("{color:#FFFFFF;};{font: 14px 'Arial';};{background-color: #323232;}");
			////spin->setFrame(false);
			//spin->addItem(tr("Control point"));
			//spin->addItem(tr("Check point"));
			//return spin;

		}

		void QSensorIntDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
		{
			QString value = index.model()->data(index, Qt::EditRole).toString();
			QComboBox* spin = static_cast<QComboBox*>(editor);
			spin->setCurrentText(value);
		}

		void QSensorIntDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
		{
			int nColoum = index.column();
			if (nColoum == 1) {

				QComboBox* comboBox = static_cast<QComboBox*>(editor);
				QString value = comboBox->currentText();
				model->setData(index, value, Qt::EditRole);
				emit CurrentIndexChangeSignal(index.row(), index.column(), value);
				/*if (comboBox != 0) {
					QString curText = model->data(index, Qt::UserRole + 1).toString();
					if (curText != comboBox->currentText())
					{
						model->setData(index, comboBox->currentText(), Qt::UserRole + 1);
						emit CurrentIndexChangeSignal(index.row(), index.column(), comboBox->currentText());
					}

				}*/


			}
			else
			{
				QStyledItemDelegate::setModelData(editor, model, index);
			}
			//QComboBox*spin = static_cast<QComboBox*> (editor);
			////spin->interpretText();
			//QString strValue = spin->currentText();
			//model->setData(index, strValue, Qt::EditRole);

		}

		void QSensorIntDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			editor->setGeometry(option.rect);
			QComboBox* comboBox = static_cast<QComboBox*>(editor);
			comboBox->showPopup();
		}
	}
}