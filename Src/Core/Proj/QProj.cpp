

#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/CoordinateTransform.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileOpenEvent>
#include <QMessageBox>
#include <QPalette>
#include <QProcess>
#include <QProcessEnvironment>
#include <QIcon>
#include <QPixmap>
#include <QThreadPool>
#include <QLocale>
#include <QStyle>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QTextStream>
#include <QScreen>
#include <QAuthenticator>
#include <QMutex>
#ifndef Q_OS_WIN
#include <netinet/in.h>
#include <pwd.h>
#else
#include <winsock.h>
#include <windows.h>
#include <lmcons.h>
#define SECURITY_WIN32
#include <security.h>
#ifdef _MSC_VER
#pragma comment( lib, "Secur32.lib" )
#endif
#endif
namespace AI3D
{
    namespace PROJ
    {
        QProj::endian_t QProj::endian()
        {
            return (htonl(1) == 1) ? XDR : NDR;
        }
        CoordinateReferenceSystemRegistry* QProj::coordinateReferenceSystemRegistry()
        {
            return members()->mCrsRegistry;
        }
        QProj::QProj()
        {
            mApplicationMembers = new ApplicationMembers();
        }
        QProj* QProj::instance()
        {
            static QProj* s_registry = new QProj();
            return s_registry;
        }
        QProj::~QProj()
        {
            delete mApplicationMembers;



            
            
            
            invalidateCaches();
        }
        

        void QProj::invalidateCaches()
        {
            
            
            
            CoordinateTransform::invalidateCache(true);
            CoordinateReferenceSystem::invalidateCache(true);
            
        }
        QProj::ApplicationMembers::ApplicationMembers()
        {
            
            

            {
                
                mCrsRegistry = new CoordinateReferenceSystemRegistry();
                
            }

        }

        QProj::ApplicationMembers::~ApplicationMembers()
        {

            delete mCrsRegistry;

        }


        QProj::ApplicationMembers* QProj::sApplicationMembers = nullptr;
        QProj::ApplicationMembers* QProj::members()
        {
            if (auto* lInstance = instance())
            {
                return lInstance->mApplicationMembers;
            }
            else
            {
                static QMutex sMemberMutex;
                QMutexLocker lock(&sMemberMutex);
                if (!sApplicationMembers)
                    sApplicationMembers = new ApplicationMembers();
                return sApplicationMembers;
            }
        }
    }
}