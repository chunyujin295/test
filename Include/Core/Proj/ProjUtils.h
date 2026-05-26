
#ifndef PROJUTILS_H
#define PROJUTILS_H

#include <QtGlobal>
#include <QDate>
#include "DatumTransform.h"
#include <memory>
#include <QStringList>

#if !defined(USE_THREAD_LOCAL) || defined(Q_OS_WIN)
#include <QThreadStorage>
#endif

#ifndef SIP_RUN
struct PJconsts;
typedef struct PJconsts PJ;
#endif
#include "Constants.h"

#if PROJ_VERSION_MAJOR>=8
struct pj_ctx;
typedef struct pj_ctx PJ_CONTEXT;
#else
struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;
#endif

namespace AI3D
{
    namespace PROJ
    {

        
        class AI3D_API ProjUtils
        {
        public:

            
            static int projVersionMajor();

            
            static int projVersionMinor();

            
            static QString epsgRegistryVersion();

            
            static QDate epsgRegistryDate();

            
            static QString esriDatabaseVersion();

            
            static QDate esriDatabaseDate();

            
            static QString ignfDatabaseVersion();

            
            static QDate ignfDatabaseDate();

            
            static QStringList searchPaths();

#ifndef SIP_RUN

            
            enum IdentifyFlag
            {
                FlagMatchBoundCrsToUnderlyingSourceCrs = 1 << 0, 
            };
            Q_DECLARE_FLAGS(IdentifyFlags, IdentifyFlag)

                
                struct ProjPJDeleter
            {

                
                void AI3D_API operator()(PJ* object) const;

            };

            
            using proj_pj_unique_ptr = std::unique_ptr< PJ, ProjPJDeleter >;

            
            static bool usesAngularUnit(const QString& projDef);

            

            
            static bool axisOrderIsSwapped(const PJ* crs);

            
            static bool isDynamic(const PJ* crs);

            
            static proj_pj_unique_ptr crsToHorizontalCrs(const PJ* crs);

            
            static proj_pj_unique_ptr unboundCrs(const PJ* crs);

            
            static proj_pj_unique_ptr crsToDatumEnsemble(const PJ* crs);

            
            static bool identifyCrs(const PJ* crs, QString& authName, QString& authCode, IdentifyFlags flags = IdentifyFlags());

            
            static bool coordinateOperationIsAvailable(const QString& projDef);

            
            static QList< DatumTransform::GridDetails > gridsUsed(const QString& proj);

#if 0 

            
            static QStringList nonAvailableGrids(const QString& projDef);
#endif
#endif
        };

#ifndef SIP_RUN



        
        class AI3D_API ProjContext
        {
        public:

            ProjContext();
            ~ProjContext();

            
            static PJ_CONTEXT* get();

        private:
            PJ_CONTEXT* mContext = nullptr;

            
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
            static thread_local QgsProjContext sProjContext;
#else
            static QThreadStorage< ProjContext* > sProjContext;
#endif
        };


        Q_DECLARE_OPERATORS_FOR_FLAGS(ProjUtils::IdentifyFlags)
#endif
    }
}
#endif 
