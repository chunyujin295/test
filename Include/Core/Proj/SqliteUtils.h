



#ifndef SQLITEUTILS_H
#define SQLITEUTILS_H



#include <memory>
#include <QString>
#include "Constants.h"
struct sqlite3;
struct sqlite3_stmt;


#ifndef SIP_RUN

namespace AI3D
{
    namespace PROJ
    {
        
        struct AI3D_API Sqlite3Closer
        {

            
            void operator()(sqlite3* database) const;
        };

        
        struct AI3D_API  Sqlite3StatementFinalizer
        {

            
            void operator()(sqlite3_stmt* statement) const;
        };

        
        class AI3D_API sqlite3_statement_unique_ptr : public std::unique_ptr< sqlite3_stmt, Sqlite3StatementFinalizer>
        {
        public:

            
            int step();

            
            QString columnName(int column) const;

            
            QString columnAsText(int column) const;

            
            QByteArray columnAsBlob(int column) const;

            
            qlonglong columnAsInt64(int column) const;

            
            double columnAsDouble(int column) const;

            
            int columnCount() const;
        };


        
        class AI3D_API sqlite3_database_unique_ptr : public std::unique_ptr< sqlite3, Sqlite3Closer>
        {
        public:

            
            int open(const QString& path);

            
            int open_v2(const QString& path, int flags, const char* zVfs);

            
            QString errorMessage() const;

            
            sqlite3_statement_unique_ptr prepare(const QString& sql, int& resultCode) const;

            
            int exec(const QString& sql, QString& errorMessage) const;

        };

        
        QString AI3D_API sqlite3_mprintf(const char* format, ...);

#endif

        
        class AI3D_API SqliteUtils
        {
        public:

            
            static QString quotedString(const QString& value);

            
            static QString quotedIdentifier(const QString& identifier);

            
            static QString quotedValue(const QVariant& value);

            
            static QStringList systemTables();

            
            static QSet<QString> uniqueFields(sqlite3* connection, const QString& tableName, QString& errorMessage) ;

            
            static long long nextSequenceValue(sqlite3* connection, const QString& tableName, QString errorMessage) ;

        };
    }
}
#endif 
