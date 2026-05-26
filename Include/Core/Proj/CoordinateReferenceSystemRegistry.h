

#ifndef COORDINATEREFERENCESYSTEMREGISTRY_H
#define COORDINATEREFERENCESYSTEMREGISTRY_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "CoordinateReferenceSystem.h"




namespace AI3D
{
    namespace PROJ
    {
        class ProjOperation;

        
        struct AI3D_API CrsDbRecord
        {
            QString description;
            QString projectionAcronym;
            QString srsId;
            QString authName;
            QString authId;
            ProjCore::CrsType type = ProjCore::CrsType::Unknown;
            bool deprecated = false;
        };


        
        class AI3D_API CoordinateReferenceSystemRegistry : public QObject
        {
            Q_OBJECT
        public:

            
            explicit CoordinateReferenceSystemRegistry(QObject* parent = nullptr);

            ~CoordinateReferenceSystemRegistry();

            
            class UserCrsDetails
            {
            public:

                
                long id = -1;

                
                QString name;

                
                QString proj;

                
                QString wkt;

                
                CoordinateReferenceSystem crs;
            };

            
            QList< CoordinateReferenceSystemRegistry::UserCrsDetails > userCrsList() const;

            
            long addUserCrs(const CoordinateReferenceSystem& crs, const QString& name, ProjCore::CrsDefinitionFormat nativeFormat = ProjCore::CrsDefinitionFormat::Wkt);

            
            bool updateUserCrs(long id, const CoordinateReferenceSystem& crs, const QString& name, ProjCore::CrsDefinitionFormat nativeFormat = ProjCore::CrsDefinitionFormat::Wkt);

            
            bool removeUserCrs(long id);

            
            QMap< QString, ProjOperation > projOperations() const;



            
            QSet< QString > authorities() const;

            
            QList< CrsDbRecord > crsDbRecords() const SIP_SKIP;

            
            QList< CoordinateReferenceSystem > recentCrs();
            QList<CoordinateReferenceSystem> GetRecentCrs();
            void PushIntoRecent(const CoordinateReferenceSystem& crs);
            void InsertRecent(const CoordinateReferenceSystem& crs);
            long AddUserCrsToDatabase( CoordinateReferenceSystem& crs, const QString& name, ProjCore::CrsDefinitionFormat nativeFormat);
            
            void pushRecent(const CoordinateReferenceSystem& crs);

            
            void removeRecent(const CoordinateReferenceSystem& crs);

            
            void clearRecent();

        signals:

            
            void userCrsChanged(const QString& id);

            
            void userCrsAdded(const QString& id);

            
            void userCrsRemoved(long id);

            
            void crsDefinitionsChanged();

            
            void recentCrsPushed(const CoordinateReferenceSystem& crs);

            
            void recentCrsRemoved(const CoordinateReferenceSystem& crs);

            
            void recentCrsCleared();

        private:

            bool insertProjection(const QString& projectionAcronym);

            
            mutable QMap< QString, ProjOperation > mProjOperations;
            mutable QSet< QString > mKnownAuthorities;
            mutable QList< CrsDbRecord > mCrsDbRecords;

        };
    }
}

#endif 
