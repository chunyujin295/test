/***************************************************************************
                    qgsrecentcoordinatereferencesystemsmodel.h
                    -------------------
    begin                : January 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RECENTCOORDINATEREFERENCESYSTEMSMODEL_H
#define RECENTCOORDINATEREFERENCESYSTEMSMODEL_H

#include "CoordinateReferenceSystemModel.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QIcon>
#include "Constants.h"
namespace AI3D
{
    namespace PROJ
    {
        class CoordinateReferenceSystem;

        /**
         * \class QgsRecentCoordinateReferenceSystemsModel
         * \ingroup core
         * \brief A model for display of recently used coordinate reference systems.
         * \since QGIS 3.36
         */
        class /*AI3D_API*/ RecentCoordinateReferenceSystemsModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:

            // *INDENT-OFF*

            /**
             * Custom model roles.
             */
            enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST(RecentCoordinateReferenceSystemsModel, Roles) : int
            {
                Crs SIP_MONKEYPATCH_COMPAT_NAME(RoleCrs) = Qt::UserRole, //!< Coordinate reference system
                    AuthId SIP_MONKEYPATCH_COMPAT_NAME(RoleAuthId), //!< CRS authority ID
            };
            Q_ENUM(CustomRole)
                // *INDENT-ON*

                /**
                 * Constructor for QgsRecentCoordinateReferenceSystemsModel, with the specified \a parent object.
                 */
                RecentCoordinateReferenceSystemsModel(QObject* parent = nullptr, int subclassColumnCount = 1);

            Qt::ItemFlags flags(const QModelIndex& index) const override;
            QVariant data(const QModelIndex& index, int role) const override;
            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            int columnCount(const QModelIndex & = QModelIndex()) const override;
            QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
            QModelIndex parent(const QModelIndex& index) const override;

            /**
             * Returns the CRS for the corresponding \a index.
             *
             * Returns an invalid CRS if the index is not valid.
             */
            CoordinateReferenceSystem crs(const QModelIndex& index) const;

        private slots:

            void recentCrsPushed(const CoordinateReferenceSystem& crs);
            void recentCrsRemoved(const CoordinateReferenceSystem& crs);
            void recentCrsCleared();

        private:

            QList< CoordinateReferenceSystem > mCrs;
            int mColumnCount = 1;

        };


        /**
         * \brief A sort/filter proxy model for recent coordinate reference systems.
         *
         * \ingroup gui
         * \since QGIS 3.36
         */
        class /*AI3D_API*/ RecentCoordinateReferenceSystemsProxyModel : public QSortFilterProxyModel
        {
            Q_OBJECT

        public:

            /**
             * Constructor for QgsRecentCoordinateReferenceSystemsProxyModel, with the given \a parent object.
             */
            explicit RecentCoordinateReferenceSystemsProxyModel(QObject* parent = nullptr, int subclassColumnCount = 1);

            /**
             * Returns the underlying source model.
             */
            RecentCoordinateReferenceSystemsModel* recentCoordinateReferenceSystemsModel();

            /**
             * Returns the underlying source model.
             * \note Not available in Python bindings
             */
            const RecentCoordinateReferenceSystemsModel* recentCoordinateReferenceSystemsModel() const SIP_SKIP;

            /**
             * Set \a filters that affect how CRS are filtered.
             */
            void setFilters(CoordinateReferenceSystemProxyModel::Filters filters);

            /**
             * Sets whether deprecated CRS should be filtered from the results.
            */
            void setFilterDeprecated(bool filter);

            /**
             * Sets a \a filter string, such that only coordinate reference systems matching the
             * specified string will be shown.
            */
            void setFilterString(const QString& filter);

            /**
             * Returns any filters that affect how CRS are filtered.
             * \see setFilters()
             */
            CoordinateReferenceSystemProxyModel::Filters filters() const { return mFilters; }

            bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

            /**
             * Returns the CRS for the corresponding \a index.
             *
             * Returns an invalid CRS if the index is not valid.
             */
            CoordinateReferenceSystem crs(const QModelIndex& index) const;

        private:

            RecentCoordinateReferenceSystemsModel* mModel = nullptr;
            CoordinateReferenceSystemProxyModel::Filters mFilters = CoordinateReferenceSystemProxyModel::Filters();
            bool mFilterDeprecated = false;
            QString mFilterString;
        };

    }
}
#endif // QGSRECENTCOORDINATEREFERENCESYSTEMSMODEL_H
