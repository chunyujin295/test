



#include "Core/Proj/Localec.h"

#include <locale>
#include <QByteArray>
namespace AI3D
{
    namespace PROJ
    {
        QMutex LocaleNumC::sLocaleLock;

        LocaleNumC::LocaleNumC()
        {
            sLocaleLock.lock();

            mOldlocale = setlocale(LC_NUMERIC, nullptr);
            if (mOldlocale)
                mOldlocale = qstrdup(mOldlocale);

            setlocale(LC_NUMERIC, "C");
        }

        LocaleNumC::~LocaleNumC()
        {
            setlocale(LC_NUMERIC, mOldlocale);
            delete[] mOldlocale;

            sLocaleLock.unlock();
        }
    }
}