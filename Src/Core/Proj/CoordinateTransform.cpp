


#include "Core/Proj/CoordinateTransform.h"
#include "Core/Proj/CoordinateTransform_p.h"
#include "Core/Proj/QProj.h"
#include "Core/Proj/Exception.h"

#include "Core/Proj/ReadWriteLocker.h"
#include "Core/Proj/ProjOperation.h"



#include <QDomNode>
#include <QDomElement>
#include <QApplication>
#include <QPolygonF>
#include <QStringList>
#include <QVector>

#include <proj.h>


#include <sqlite3.h>
#include <qlogging.h>
#include <vector>
#include <algorithm>
namespace AI3D
{
    namespace PROJ
    {
        
        

        QReadWriteLock CoordinateTransform::sCacheLock;
        QMultiHash< QPair< QString, QString >, CoordinateTransform > CoordinateTransform::sTransforms; 
        bool CoordinateTransform::sDisableCache = false;



        CoordinateTransform::CoordinateTransform()
        {
            d = new CoordinateTransformPrivate();
        }
        void CoordinateTransform::invalidateCache(bool disableCache)
        {
            const ReadWriteLocker locker(sCacheLock, ReadWriteLocker::Write);
            if (sDisableCache)
                return;

            if (disableCache)
            {
                sDisableCache = true;
            }

            sTransforms.clear();
        }

        void CoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread(void* pj_context)
        {
            
            
            
            
            if (sDisableCache)
                return;

            const ReadWriteLocker locker(sCacheLock, ReadWriteLocker::Write);
            
            if (sDisableCache)
                return;

            for (auto it = sTransforms.begin(); it != sTransforms.end(); )
            {
                auto& v = it.value();
                if (v.d->removeObjectsBelongingToCurrentThread(pj_context))
                    it = sTransforms.erase(it);
                else
                    ++it;
            }
        }
    }
}