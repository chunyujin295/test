



#include "Core/Proj/CoordinateTransformContext.h"
#include "Core/Proj/Coordinatetransformcontext_p.h"
#include "Core/Proj/ProjUtils.h"
#include "Core/Proj/ProjCore.h"
#include <QSettings>
QString crsToKey(const AI3D::PROJ::CoordinateReferenceSystem& crs)
{
    return crs.authid().isEmpty() ? crs.toWkt(AI3D::PROJ::ProjCore::CrsWktVariant::Preferred) : crs.authid();
}
template<>
bool qMapLessThanKey<QPair<AI3D::PROJ::CoordinateReferenceSystem, AI3D::PROJ::CoordinateReferenceSystem>>
(const QPair<AI3D::PROJ::CoordinateReferenceSystem, AI3D::PROJ::CoordinateReferenceSystem>& key1,
    const QPair<AI3D::PROJ::CoordinateReferenceSystem, AI3D::PROJ::CoordinateReferenceSystem>& key2)
{
    const QPair< QString, QString > key1String = qMakePair(crsToKey(key1.first), crsToKey(key1.second));
    const QPair< QString, QString > key2String = qMakePair(crsToKey(key2.first), crsToKey(key2.second));
    return key1String < key2String;
}

namespace AI3D
{
    namespace PROJ
    {

       


    


        CoordinateTransformContext::CoordinateTransformContext()
            : d(new CoordinateTransformContextPrivate())
        {}

        CoordinateTransformContext::~CoordinateTransformContext() = default;

        CoordinateTransformContext::CoordinateTransformContext(const CoordinateTransformContext& rhs)  
            : d(rhs.d)
        {}

        CoordinateTransformContext& CoordinateTransformContext::operator=(const CoordinateTransformContext& rhs)  
        {
            d = rhs.d;
            return *this;
        }

        bool CoordinateTransformContext::operator==(const CoordinateTransformContext& rhs) const
        {
            if (d == rhs.d)
                return true;

            d->mLock.lockForRead();
            rhs.d->mLock.lockForRead();
            const bool equal = d->mSourceDestDatumTransforms == rhs.d->mSourceDestDatumTransforms;
            d->mLock.unlock();
            rhs.d->mLock.unlock();
            return equal;
        }

        void CoordinateTransformContext::clear()
        {
            d.detach();
            
            d->mLock.lockForWrite();
            d->mSourceDestDatumTransforms.clear();
            d->mLock.unlock();
        }

        QMap<QPair<QString, QString>, DatumTransform::TransformPair> CoordinateTransformContext::sourceDestinationDatumTransforms() const
        {
            return QMap<QPair<QString, QString>, DatumTransform::TransformPair>();
        }

        QMap<QPair<QString, QString>, QString> CoordinateTransformContext::coordinateOperations() const
        {
            d->mLock.lockForRead();
            auto res = d->mSourceDestDatumTransforms;
            res.detach();
            d->mLock.unlock();
            QMap<QPair<QString, QString>, QString> results;
            for (auto it = res.constBegin(); it != res.constEnd(); ++it)
                results.insert(qMakePair(it.key().first.authid(), it.key().second.authid()), it.value().operation);

            return results;
        }

        bool CoordinateTransformContext::addSourceDestinationDatumTransform(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs, int sourceTransform, int destinationTransform)
        {
            if (!sourceCrs.isValid() || !destinationCrs.isValid())
                return false;
            Q_UNUSED(sourceTransform)
                Q_UNUSED(destinationTransform)
                return false;
        }

        bool CoordinateTransformContext::addCoordinateOperation(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs, const QString& coordinateOperationProjString, bool allowFallback)
        {
            if (!sourceCrs.isValid() || !destinationCrs.isValid())
                return false;
            d.detach();
            d->mLock.lockForWrite();
            CoordinateTransformContextPrivate::OperationDetails details;
            details.operation = coordinateOperationProjString;
            details.allowFallback = allowFallback;
            d->mSourceDestDatumTransforms.insert(qMakePair(sourceCrs, destinationCrs), details);
            d->mLock.unlock();
            return true;
        }

        void CoordinateTransformContext::removeSourceDestinationDatumTransform(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs)
        {
            removeCoordinateOperation(sourceCrs, destinationCrs);
        }

        void CoordinateTransformContext::removeCoordinateOperation(const CoordinateReferenceSystem& sourceCrs, const CoordinateReferenceSystem& destinationCrs)
        {
            d->mSourceDestDatumTransforms.remove(qMakePair(sourceCrs, destinationCrs));
        }

        bool CoordinateTransformContext::hasTransform(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const
        {
            const QString t = calculateCoordinateOperation(source, destination);
            return !t.isEmpty();
        }

        DatumTransform::TransformPair CoordinateTransformContext::calculateDatumTransforms(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const
        {
            Q_UNUSED(source)
                Q_UNUSED(destination)
                return DatumTransform::TransformPair(-1, -1);
        }

        QString CoordinateTransformContext::calculateCoordinateOperation(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const
        {
            if (!source.isValid() || !destination.isValid())
                return QString();

            d->mLock.lockForRead();

            auto it = d->mSourceDestDatumTransforms.constFind(qMakePair(source, destination));
            if (it == d->mSourceDestDatumTransforms.constEnd())
            {
                
                it = d->mSourceDestDatumTransforms.constFind(qMakePair(destination, source));
            }

            const QString result = it == d->mSourceDestDatumTransforms.constEnd() ? QString() : it.value().operation;
            d->mLock.unlock();
            return result;
        }

        bool CoordinateTransformContext::allowFallbackTransform(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const
        {
            if (!source.isValid() || !destination.isValid())
                return false;

            d->mLock.lockForRead();
            CoordinateTransformContextPrivate::OperationDetails res = d->mSourceDestDatumTransforms.value(qMakePair(source, destination), CoordinateTransformContextPrivate::OperationDetails());
            if (res.operation.isEmpty())
            {
                
                res = d->mSourceDestDatumTransforms.value(qMakePair(destination, source), CoordinateTransformContextPrivate::OperationDetails());
            }
            d->mLock.unlock();
            return res.allowFallback;
        }

        bool CoordinateTransformContext::mustReverseCoordinateOperation(const CoordinateReferenceSystem& source, const CoordinateReferenceSystem& destination) const
        {
            if (!source.isValid() || !destination.isValid())
                return false;

            d->mLock.lockForRead();
            CoordinateTransformContextPrivate::OperationDetails res = d->mSourceDestDatumTransforms.value(qMakePair(source, destination), CoordinateTransformContextPrivate::OperationDetails());
            if (!res.operation.isEmpty())
            {
                d->mLock.unlock();
                return false;
            }
            
            res = d->mSourceDestDatumTransforms.value(qMakePair(destination, source), CoordinateTransformContextPrivate::OperationDetails());
            if (!res.operation.isEmpty())
            {
                d->mLock.unlock();
                return true;
            }

            d->mLock.unlock();
            return false;
        }

        void CoordinateTransformContext::readSettings()
        {
            d.detach();
            d->mLock.lockForWrite();

            d->mSourceDestDatumTransforms.clear();

            QSettings settings;
            settings.beginGroup(QStringLiteral("/Projections"));
            const QStringList projectionKeys = settings.allKeys();

            
            QMap< QPair< CoordinateReferenceSystem, CoordinateReferenceSystem >, CoordinateTransformContextPrivate::OperationDetails > transforms;
            QStringList::const_iterator pkeyIt = projectionKeys.constBegin();
            for (; pkeyIt != projectionKeys.constEnd(); ++pkeyIt)
            {
                if (pkeyIt->contains(QLatin1String("coordinateOp")))
                {
                    const QStringList split = pkeyIt->split('/');
                    QString srcAuthId, destAuthId;
                    if (!split.isEmpty())
                    {
                        srcAuthId = split.at(0);
                    }
                    if (split.size() > 1)
                    {
                        destAuthId = split.at(1).split('_').at(0);
                    }

                    if (srcAuthId.isEmpty() || destAuthId.isEmpty())
                        continue;

                    const QString proj = settings.value(*pkeyIt).toString();
                    const bool allowFallback = settings.value(QStringLiteral("%1//%2_allowFallback").arg(srcAuthId, destAuthId)).toBool();
                    CoordinateTransformContextPrivate::OperationDetails deets;
                    deets.operation = proj;
                    deets.allowFallback = allowFallback;
                    transforms[qMakePair(CoordinateReferenceSystem(srcAuthId), CoordinateReferenceSystem(destAuthId))] = deets;
                }
            }

            
            auto transformIt = transforms.constBegin();
            for (; transformIt != transforms.constEnd(); ++transformIt)
            {
                d->mSourceDestDatumTransforms.insert(transformIt.key(), transformIt.value());
            }

            d->mLock.unlock();
            settings.endGroup();
        }

        void CoordinateTransformContext::writeSettings()
        {
            QSettings settings;
            settings.beginGroup(QStringLiteral("/Projections"));
            const QStringList groupKeys = settings.allKeys();
            QStringList::const_iterator groupKeyIt = groupKeys.constBegin();
            for (; groupKeyIt != groupKeys.constEnd(); ++groupKeyIt)
            {
                if (groupKeyIt->contains(QLatin1String("srcTransform")) || groupKeyIt->contains(QLatin1String("destTransform")) || groupKeyIt->contains(QLatin1String("coordinateOp")))
                {
                    settings.remove(*groupKeyIt);
                }
            }

            for (auto transformIt = d->mSourceDestDatumTransforms.constBegin(); transformIt != d->mSourceDestDatumTransforms.constEnd(); ++transformIt)
            {
                const QString srcAuthId = transformIt.key().first.authid();
                const QString destAuthId = transformIt.key().second.authid();

                if (srcAuthId.isEmpty() || destAuthId.isEmpty())
                    continue; 

                const QString proj = transformIt.value().operation;
                const bool allowFallback = transformIt.value().allowFallback;
                settings.setValue(srcAuthId + "//" + destAuthId + "_coordinateOp", proj);
                settings.setValue(srcAuthId + "//" + destAuthId + "_allowFallback", allowFallback);
            }

            settings.endGroup();
        }
    }
}