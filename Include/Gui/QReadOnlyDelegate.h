#ifndef QREADONLYDELEGATE_H
#define QREADONLYDELEGATE_H


#include <QStyledItemDelegate>
//
namespace AI3D
{
	namespace GUI
	{
		class QReadOnlyDelegate : public QStyledItemDelegate
		{
			Q_OBJECT

		public:
			QReadOnlyDelegate(QObject* parent = 0);
			~QReadOnlyDelegate();


			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			void setEditorData(QWidget* editor, const QModelIndex& index) const;
			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		signals:
			void CurrentIndexChangeSignal(int iRow, int iColumn, QString selectIndexText) const;
		};

		class QCommonDelegate : public QStyledItemDelegate
		{
			Q_OBJECT

		public:
			QCommonDelegate(QObject* parent = 0);
			~QCommonDelegate();


			QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			void setEditorData(QWidget* editor, const QModelIndex& index) const;
			void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
			void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		signals:
			void CurrentIndexChangeSignal(int iRow, int iColumn, QString selectIndexText) const;
		};
	}
}
#endif // DEBUG
