#include "Gui/QReadOnlyDelegate.h"
#include <QComboBox>
namespace AI3D
{
	namespace GUI
	{
		QReadOnlyDelegate::QReadOnlyDelegate(QObject* parent)
		{


		}

		QReadOnlyDelegate::~QReadOnlyDelegate()
		{
		}

		QWidget* QReadOnlyDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			Q_UNUSED(parent)
			Q_UNUSED(option)
			Q_UNUSED(index)
				return NULL;
		}

		void QReadOnlyDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
		{
			
		}

		void QReadOnlyDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
		{

		}

		void QReadOnlyDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			
		}
	}
}