/***************************************************************************
                    qgsrecentcoordinatereferencesystemsmodel.cpp
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
#include "Gui/RecentCoordinateReferenceSystemsModel.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/QProj.h"

#include <QFont>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif
namespace AI3D
{
    namespace PROJ
    {
        RecentCoordinateReferenceSystemsModel::RecentCoordinateReferenceSystemsModel(QObject* parent, int subclassColumnCount)
            : QAbstractItemModel(parent)
            , mColumnCount(subclassColumnCount)
        {
#ifdef ENABLE_MODELTEST
            new ModelTest(this, this);
#endif

            mCrs = QProj::coordinateReferenceSystemRegistry()->recentCrs();
            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::recentCrsPushed, this, &RecentCoordinateReferenceSystemsModel::recentCrsPushed);
            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::recentCrsRemoved, this, &RecentCoordinateReferenceSystemsModel::recentCrsRemoved);
            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::recentCrsCleared, this, &RecentCoordinateReferenceSystemsModel::recentCrsCleared);
        }

        Qt::ItemFlags RecentCoordinateReferenceSystemsModel::flags(const QModelIndex& index) const
        {
            if (!index.isValid())
            {
                return Qt::ItemFlags();
            }

            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }

        QVariant RecentCoordinateReferenceSystemsModel::data(const QModelIndex& index, int role) const
        {
            const CoordinateReferenceSystem crs = RecentCoordinateReferenceSystemsModel::crs(index);
            if (!crs.isValid())
                return QVariant();

            if (index.column() == 0)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                    return crs.userFriendlyIdentifier();

                    case static_cast<int>(CustomRole::Crs) :
                        return crs;

                        case static_cast<int>(CustomRole::AuthId) :
                            return crs.authid();

                        default:
                            break;
                }
            }

            return QVariant();
        }

        int RecentCoordinateReferenceSystemsModel::rowCount(const QModelIndex& parent) const
        {
            if (parent.isValid())
                return 0;

            return mCrs.size();
        }

        int RecentCoordinateReferenceSystemsModel::columnCount(const QModelIndex&) const
        {
            return mColumnCount;
        }

        QModelIndex RecentCoordinateReferenceSystemsModel::index(int row, int column, const QModelIndex& parent) const
        {
            if (row < 0 || row >= mCrs.size() || column < 0 || column >= columnCount(parent) || parent.isValid())
                return QModelIndex();

            return createIndex(row, column);
        }

        QModelIndex RecentCoordinateReferenceSystemsModel::parent(const QModelIndex&) const
        {
            return QModelIndex();
        }

        CoordinateReferenceSystem RecentCoordinateReferenceSystemsModel::crs(const QModelIndex& index) const
        {
            if (!index.isValid())
                return CoordinateReferenceSystem();

            return mCrs.value(index.row());
        }

        void RecentCoordinateReferenceSystemsModel::recentCrsPushed(const CoordinateReferenceSystem& crs)
        {
            const int currentRow = mCrs.indexOf(crs);
            if (currentRow > 0)
            {
                // move operation
                beginMoveRows(QModelIndex(), currentRow, currentRow, QModelIndex(), 0);
                mCrs.removeAt(currentRow);
                mCrs.insert(0, crs);
                endMoveRows();
            }
            else if (currentRow < 0)
            {
                // add operation
                beginInsertRows(QModelIndex(), 0, 0);
                mCrs.insert(0, crs);
                endInsertRows();
            }
        }

        void RecentCoordinateReferenceSystemsModel::recentCrsRemoved(const CoordinateReferenceSystem& crs)
        {
            const int currentRow = mCrs.indexOf(crs);
            if (currentRow >= 0)
            {
                beginRemoveRows(QModelIndex(), currentRow, currentRow);
                mCrs.removeAt(currentRow);
                endRemoveRows();
            }
        }

        void RecentCoordinateReferenceSystemsModel::recentCrsCleared()
        {
            beginResetModel();
            mCrs.clear();
            endResetModel();
        }



        //
        // RecentCoordinateReferenceSystemsProxyModel
        //

        RecentCoordinateReferenceSystemsProxyModel::RecentCoordinateReferenceSystemsProxyModel(QObject* parent, int subclassColumnCount)
            : QSortFilterProxyModel(parent)
            , mModel(new RecentCoordinateReferenceSystemsModel(this, subclassColumnCount))
        {
            setSourceModel(mModel);
            setDynamicSortFilter(true);
        }

        RecentCoordinateReferenceSystemsModel* RecentCoordinateReferenceSystemsProxyModel::recentCoordinateReferenceSystemsModel()
        {
            return mModel;
        }

        const RecentCoordinateReferenceSystemsModel* RecentCoordinateReferenceSystemsProxyModel::recentCoordinateReferenceSystemsModel() const
        {
            return mModel;
        }

        void RecentCoordinateReferenceSystemsProxyModel::setFilters(CoordinateReferenceSystemProxyModel::Filters filters)
        {
            if (mFilters == filters)
                return;

            mFilters = filters;
            invalidateFilter();
        }

        void RecentCoordinateReferenceSystemsProxyModel::setFilterDeprecated(bool filter)
        {
            if (mFilterDeprecated == filter)
                return;

            mFilterDeprecated = filter;
            invalidateFilter();
        }

        void RecentCoordinateReferenceSystemsProxyModel::setFilterString(const QString& filter)
        {
            mFilterString = filter;
            invalidateFilter();
        }

        bool RecentCoordinateReferenceSystemsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
        {
            if (!mFilters)
                return true;

            const QModelIndex sourceIndex = mModel->index(sourceRow, 0, sourceParent);

            const CoordinateReferenceSystem crs = mModel->crs(sourceIndex);
            if (mFilterDeprecated && crs.isDeprecated())
                return false;

            const ProjCore::CrsType type = crs.type();
            switch (type)
            {
            case ProjCore::CrsType::Unknown:
            case ProjCore::CrsType::Other:
                break;

            case ProjCore::CrsType::Geodetic:
            case ProjCore::CrsType::Geocentric:
            case ProjCore::CrsType::Geographic2d:
            case ProjCore::CrsType::Geographic3d:
            case ProjCore::CrsType::Projected:
            case ProjCore::CrsType::Temporal:
            case ProjCore::CrsType::Engineering:
            case ProjCore::CrsType::Bound:
            case ProjCore::CrsType::DerivedProjected:
                if (!mFilters.testFlag(CoordinateReferenceSystemProxyModel::Filter::FilterHorizontal))
                    return false;
                break;

            case ProjCore::CrsType::Vertical:
                if (!mFilters.testFlag(CoordinateReferenceSystemProxyModel::Filter::FilterVertical))
                    return false;
                break;

            case ProjCore::CrsType::Compound:
                if (!mFilters.testFlag(CoordinateReferenceSystemProxyModel::Filter::FilterCompound))
                    return false;
                break;
            }

            if (!mFilterString.trimmed().isEmpty())
            {
                if (!(crs.description().contains(mFilterString, Qt::CaseInsensitive)
                    || crs.authid().contains(mFilterString, Qt::CaseInsensitive)))
                    return false;
            }

            return true;
        }

        CoordinateReferenceSystem RecentCoordinateReferenceSystemsProxyModel::crs(const QModelIndex& index) const
        {
            const QModelIndex sourceIndex = mapToSource(index);
            return mModel->crs(sourceIndex);
        }

    }
}