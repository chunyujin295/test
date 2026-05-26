#ifndef ATOOLTIPPER_H
#define ATOOLTIPPER_H

#include <QObject>
namespace AI3D
{
    namespace GUI
    {
        class AToolTipper : public QObject
        {
            Q_OBJECT
        public:
            explicit AToolTipper(QObject* parent = 0);

            virtual bool eventFilter(QObject*, QEvent*);

        protected:
            bool headerViewEventFilter(QObject*, QEvent*);
        };
    }
}

#endif // ATOOLTIPPER_H
