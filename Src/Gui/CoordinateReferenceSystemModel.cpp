/***************************************************************************
                             CoordinateReferenceSystemModel.h
                             -------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#include "Core/Proj/ProjCore.h"
#include "Gui/CoordinateReferenceSystemModel.h"
#include "Core/Proj/CoordinateReferenceSystemUtils.h"
#include "Core/Proj/QProj.h"
#include "Core/BlockObject.h"
#include "Util/TaskProcess.h"

#include <QFont>
namespace AI3D
{
    namespace PROJ
    {
        CoordinateReferenceSystemModel::CoordinateReferenceSystemModel(QObject* parent,QString strLastEnuData, QString strLastEnuAuthId)
            : QAbstractItemModel(parent)
            , mRootNode(std::make_unique< CoordinateReferenceSystemModelGroupNode >(QString(), QIcon(), QString()))
        {
            mCrsDbRecords = QProj::coordinateReferenceSystemRegistry()->crsDbRecords();

            this->lastEnuData = strLastEnuData;
            this->lastEnuAuthId = strLastEnuAuthId;
            
            //std::cout << "crs model constructor:" << strLastEnuData.toStdString() << " authid:" << strLastEnuAuthId.toStdString() << std::endl;

            rebuild();

            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::userCrsAdded, this, &CoordinateReferenceSystemModel::userCrsAdded);
            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::userCrsRemoved, this, &CoordinateReferenceSystemModel::userCrsRemoved);
            connect(QProj::coordinateReferenceSystemRegistry(), &CoordinateReferenceSystemRegistry::userCrsChanged, this, &CoordinateReferenceSystemModel::userCrsChanged);
        }
        int  CoordinateReferenceSystemModel::GetNumOfValidItem()
        {
            return mNumValidCrs;
        }
        Qt::ItemFlags CoordinateReferenceSystemModel::flags(const QModelIndex& index) const
        {
            if (!index.isValid())
            {
                return Qt::ItemFlags();
            }

            CoordinateReferenceSystemModelNode* n = index2node(index);
            if (!n)
                return Qt::ItemFlags();

            switch (n->nodeType())
            {
            case CoordinateReferenceSystemModelNode::NodeGroup:
                return index.column() == 0 ? Qt::ItemIsEnabled : Qt::ItemFlags();
            case CoordinateReferenceSystemModelNode::NodeCrs:
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
            BUILTIN_UNREACHABLE
        }

        void CoordinateReferenceSystemModel::setDefinition(const QModelIndex& index, QString& definition, QString& authName, QString& authId)
        {
            //std::cout << "inside crs model/setDefinition:" << definition.toStdString() << " / " << authName.toStdString() << " / " << authId.toStdString() << std::endl;
            if (!index.isValid())
                return;

            CoordinateReferenceSystemModelNode* n = index2node(index);
            if (!n)
                return;

            if (n->nodeType() == CoordinateReferenceSystemModelNode::NodeGroup)
                return;

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            try
            {
                CoordinateReferenceSystemModelCrsNode* crsNode = down_cast<CoordinateReferenceSystemModelCrsNode*>(n);
                if (!crsNode)
                {
                 //   std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    return;
                }
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

                //std::cout << "before setting data for enu:" << crsNode->getRecord().description.toStdString() << std::endl;

                crsNode->getRecord().description = definition;
                crsNode->getRecord().authName = authName;
                crsNode->getRecord().authId = authId;


                //std::cout << "after setting data for enu:" << crsNode->getRecord().description.toStdString() << std::endl;
            }
            catch (const std::exception&)
            {

            }
        }

        void CoordinateReferenceSystemModel::setDefinition(const QModelIndex& index,QString &str)
        {
            if (!index.isValid())
                return;

            CoordinateReferenceSystemModelNode* n = index2node(index);
            if (!n)
                return;

            if (n->nodeType() == CoordinateReferenceSystemModelNode::NodeGroup)
                return;

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            try
            {
                CoordinateReferenceSystemModelCrsNode* crsNode = down_cast<CoordinateReferenceSystemModelCrsNode*>(n);
                if (!crsNode)
                {
              //      std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    return;
                }
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

                //std::cout << "before setting data for enu:" << crsNode->getRecord().description.toStdString() << std::endl;

                crsNode->getRecord().description = str;
                crsNode->getRecord().authName = str;
                crsNode->getRecord().authId = str;

                //std::cout << "after setting data for enu:" << crsNode->getRecord().description.toStdString() << std::endl;
            }
            catch (const std::exception&)
            {

            }
        }

        QVariant CoordinateReferenceSystemModel::data(const QModelIndex& index, int role) const
        {
            if (!index.isValid())
                return QVariant();

            CoordinateReferenceSystemModelNode* n = index2node(index);
            if (!n)
                return QVariant();

            if (role == static_cast<int>(CustomRole::NodeType))
                return n->nodeType();

            switch (n->nodeType())
            {
                case CoordinateReferenceSystemModelNode::NodeGroup:
                {
                    CoordinateReferenceSystemModelGroupNode* groupNode = down_cast<CoordinateReferenceSystemModelGroupNode*>(n);
                    switch (role)
                    {
                    case Qt::DecorationRole:
                        switch (index.column())
                        {
                        case 0:
                            return groupNode->icon();
                        default:
                            break;
                        }
                        break;

                    case Qt::DisplayRole:
                    case Qt::ToolTipRole:
                        switch (index.column())
                        {
                        case 0:
                            //std::cout << "get groupNode name: id:" << groupNode->id().toStdString() << " name:" << groupNode->name().toStdString() << std::endl;
                            //groupNode->
                            return groupNode->name();

                        default:
                            //std::cout << "get2 groupNode name: id:" << groupNode->id().toStdString() << " name:" << groupNode->name().toStdString() << std::endl;
                            break;
                        }
                        break;

                    case Qt::FontRole:
                    {
                        QFont font;
                        font.setItalic(true);
                        if (groupNode->parent() == mRootNode.get())
                        {
                            font.setBold(true);
                        }
                        return font;
                    }

                    case static_cast<int>(CustomRole::GroupId) :
                        return groupNode->id();
                    }
                    return QVariant();

                }
                case CoordinateReferenceSystemModelNode::NodeCrs:
                {
                    CoordinateReferenceSystemModelCrsNode* crsNode = down_cast<CoordinateReferenceSystemModelCrsNode*>(n);
                    switch (role)
                    {
                    case Qt::DisplayRole:
                    case Qt::ToolTipRole:
                        switch (index.column())
                        {
                            case 0:
         //                       std::cout << "model get display role:" << crsNode->record().description.toStdString() << std::endl;
                                return crsNode->record().description;

                            case 1:
                            {
                                QString authidstr = crsNode->record().authName;
                                authidstr.toUpper();

                              /*  if (!crsNode->record().authId.isEmpty() && authidstr.contains("LOCAL"))
                                {
                                    return QStringLiteral("%1:%2").arg(crsNode->record().authName, "Local:0");
                                }*/
                                if (crsNode->record().authName == QLatin1String("CUSTOM") /* || !authidstr.contains("EPSG")*/)
                                    return QString();
                        
                                return QStringLiteral("%1:%2").arg(crsNode->record().authName, crsNode->record().authId);
                            }

                            default:
                                break;
                        }
                        break;

                    case static_cast<int>(CustomRole::Name) :
           //             std::cout << "model get name:" << crsNode->record().description.toStdString() << std::endl;
                        return crsNode->record().description;

                    case static_cast<int>(CustomRole::AuthId) :
                    {
                        //if (crsNode->record().authName.contains("enu", Qt::CaseInsensitive))
                        {
                            //std::cout << "model get authname:" << crsNode->record().authName.toStdString() << " authid:"
                            //    << crsNode->record().authId.toStdString() << std::endl;
                        }

                        QString authidstr = crsNode->record().authName;
                        authidstr.toUpper();

//                        if (!crsNode->record().authId.isEmpty() && (authidstr.contains("EPSG") || authidstr.contains("LOCAL") || authidstr.contains("ENU")))
                        if (!crsNode->record().authId.isEmpty() /* && (authidstr.contains("EPSG") || authidstr.contains("LOCAL"))*/)
                        {
                        //    std::cout << "model get authname2:" << crsNode->record().authName.toStdString() << " authid:"
                        //        << crsNode->record().authId.toStdString() << std::endl;

                            return QStringLiteral("%1:%2").arg(crsNode->record().authName, crsNode->record().authId);
                        }
                        else
                        {
                        //    std::cout << "model get authname3:" << crsNode->record().authName.toStdString() << " authid:"
                        //        << crsNode->record().authId.toStdString() << std::endl;

                            return QVariant();
                        }
                    }

                    case static_cast<int>(CustomRole::Deprecated) :
                        return crsNode->record().deprecated;

                    case static_cast<int>(CustomRole::Type) :
                        return QVariant::fromValue(crsNode->record().type);

                    case static_cast<int>(CustomRole::Wkt) :
                        return crsNode->wkt();

                    case static_cast<int>(CustomRole::Proj) :
                        return crsNode->proj();

                    case Qt::DecorationRole:
                    {
                        QString desc =  crsNode->record().description;
                        QString authidstr = crsNode->record().authName;

                        //std::cout << "return ycorner:" << desc.toStdString() << " desc/auto:" << authidstr.toStdString() << std::endl;
                        return QIcon(":/new/prefix1/skinbutton/ycorner.png");
                        break;
                    }

                    default:
                        break;

                    }
                }
            }
            return QVariant();
        }

        QVariant CoordinateReferenceSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
        {
            if (orientation == Qt::Horizontal)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0:
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            return str2qstr(std::string("空间参考系统"));
                        }
                        else
                        {
                            return tr("Spatial Reference System");
                        }

                        ///return tr( "Coordinate Reference System" );
                    case 1:
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            return str2qstr(std::string("描述"));
                        }
                        else
                        {
                            return tr("Definition");
                        }

                        ///return tr( "Authority ID" );
                    default:
                        break;
                    }
                    break;

                default:
                    break;
                }
            }
            return QVariant();
        }

        int CoordinateReferenceSystemModel::rowCount(const QModelIndex& parent) const
        {
            CoordinateReferenceSystemModelNode* n = index2node(parent);
            if (!n)
                return 0;

            return n->children().count();
        }

        int CoordinateReferenceSystemModel::columnCount(const QModelIndex&) const
        {
            return 2;
        }

        QModelIndex CoordinateReferenceSystemModel::index(int row, int column, const QModelIndex& parent) const
        {
            if (!hasIndex(row, column, parent))
                return QModelIndex();

            CoordinateReferenceSystemModelNode* n = index2node(parent);
            if (!n)
                return QModelIndex(); // have no children

            return createIndex(row, column, n->children().at(row));
        }

        QModelIndex CoordinateReferenceSystemModel::parent(const QModelIndex& child) const
        {
            if (!child.isValid())
                return QModelIndex();

            if (CoordinateReferenceSystemModelNode* n = index2node(child))
            {
                return indexOfParentTreeNode(n->parent()); // must not be null
            }
            else
            {
                Q_ASSERT(false); // no other node types!
                return QModelIndex();
            }
        }

        QModelIndex CoordinateReferenceSystemModel::authIdToIndex(const QString& authid) const
        {
            const QModelIndex startIndex = index(0, 0);
            const QModelIndexList hits = match(startIndex, static_cast<int>(CustomRole::AuthId), authid, 1, Qt::MatchRecursive);
            return hits.value(0);
        }

        QModelIndex CoordinateReferenceSystemModel::nameToIndex(const QString& name) const
        {
            const QModelIndex startIndex = index(0, 0);
            const QModelIndexList hits = match(startIndex, static_cast<int>(CustomRole::Name), name, 1, Qt::MatchRecursive|Qt::MatchContains);
            return hits.value(0);
        }

        void CoordinateReferenceSystemModel::setLastEnuData(QString& lastEnuData, QString& lastEnuAuthId)
        {
            this->lastEnuData = lastEnuData;
            this->lastEnuAuthId = lastEnuAuthId;
        }

        void CoordinateReferenceSystemModel::rebuild()
        {
            beginResetModel();

            mRootNode->deleteChildren();

            for (const CrsDbRecord& record : std::as_const(mCrsDbRecords))
            {
                addRecord(record);
            }

            mNumValidCrs += mCrsDbRecords.size();
            const QList<CoordinateReferenceSystemRegistry::UserCrsDetails> userCrsList = QProj::coordinateReferenceSystemRegistry()->userCrsList();
            mNumValidCrs += userCrsList.size();
            //添加user
            {
                CrsDbRecord userRecord;
                userRecord.authName = QStringLiteral("USER");

                userRecord.description = "Define new user defined system";
                addRecord(userRecord);
            }
            for (const CoordinateReferenceSystemRegistry::UserCrsDetails& details : userCrsList)
            {
                CrsDbRecord userRecord;
                userRecord.authName = QStringLiteral("USER");
                userRecord.authId = QString::number(details.id);
                userRecord.description = details.name;

                addRecord(userRecord);
            }
            //添加enu 
            {
                CrsDbRecord userRecord;
                userRecord.authName = QStringLiteral("ENU");
                ///userRecord.authId = "3927";

                if (!lastEnuData.isEmpty())
                {
                    //std::cout << "lastEnuData:" << " " << lastEnuData.toStdString() << std::endl;
                    userRecord.description = lastEnuData;
                    if (!lastEnuAuthId.isEmpty())
                    {
                        QString authIdWithoutAuthName = lastEnuAuthId;
                        if (lastEnuAuthId.contains("ENU:", Qt::CaseInsensitive))
                        {
                            authIdWithoutAuthName = authIdWithoutAuthName.mid(4).trimmed();                            
                            userRecord.authId = authIdWithoutAuthName;
                        }
                        else
                            userRecord.authId = lastEnuAuthId;
                    }
                }
                else
                {
                    //std::cout << "lastEnuData:" << " is null." << std::endl;
                    userRecord.description = "Local East-North-Up (ENU)";
                    userRecord.authId = "0,0";
                }

                addRecord(userRecord);
            }
            {
                //添加local
                CrsDbRecord userRecord;
                userRecord.authName = QStringLiteral("Local");
                userRecord.authId = "0";
                userRecord.type = ProjCore::CrsType::Geocentric;
                userRecord.description = "Local coordinate system";
                addRecord(userRecord);
            }

            endResetModel();
        }

        void CoordinateReferenceSystemModel::userCrsAdded(const QString& id)
        {
            const QList<CoordinateReferenceSystemRegistry::UserCrsDetails> userCrsList = QProj::coordinateReferenceSystemRegistry()->userCrsList();
            for (const CoordinateReferenceSystemRegistry::UserCrsDetails& details : userCrsList)
            {
                if (QStringLiteral("USER:%1").arg(details.id) == id)
                {
                    CrsDbRecord userRecord;
                    userRecord.authName = QStringLiteral("USER");
                    userRecord.authId = QString::number(details.id);
                    userRecord.description = details.name;

                    CoordinateReferenceSystemModelGroupNode* group = mRootNode->getChildGroupNode(QStringLiteral("USER"));
                    if (!group)
                    {
                        std::unique_ptr< CoordinateReferenceSystemModelGroupNode > newGroup = std::make_unique< CoordinateReferenceSystemModelGroupNode >(
                            tr("User-defined"),
                            QIcon(":/status/skin/cancle.png"), QStringLiteral("USER"));
                        beginInsertRows(QModelIndex(), mRootNode->children().length(), mRootNode->children().length());
                        mRootNode->addChildNode(newGroup.get());
                        endInsertRows();
                        group = newGroup.release();
                    }

                    const QModelIndex parentGroupIndex = node2index(group);

                    beginInsertRows(parentGroupIndex, group->children().size(), group->children().size());
                    CoordinateReferenceSystemModelCrsNode* crsNode = addRecord(userRecord);
                    crsNode->setProj(details.proj);
                    crsNode->setWkt(details.wkt);
                    endInsertRows();
                    break;
                }
            }
        }

        void CoordinateReferenceSystemModel::userCrsRemoved(long id)
        {
            CoordinateReferenceSystemModelGroupNode* group = mRootNode->getChildGroupNode(QStringLiteral("USER"));
            if (group)
            {
                for (int row = 0; row < group->children().size(); ++row)
                {
                    if (CoordinateReferenceSystemModelCrsNode* crsNode = dynamic_cast<CoordinateReferenceSystemModelCrsNode*>(group->children().at(row)))
                    {
                        if (crsNode->record().authId == QString::number(id))
                        {
                            const QModelIndex parentIndex = node2index(group);
                            beginRemoveRows(parentIndex, row, row);
                            delete group->takeChild(crsNode);
                            endRemoveRows();
                            return;
                        }
                    }
                }
            }
        }

        void CoordinateReferenceSystemModel::userCrsChanged(const QString& id)
        {
            CoordinateReferenceSystemModelGroupNode* group = mRootNode->getChildGroupNode(QStringLiteral("USER"));
            if (group)
            {
                for (int row = 0; row < group->children().size(); ++row)
                {
                    if (CoordinateReferenceSystemModelCrsNode* crsNode = dynamic_cast<CoordinateReferenceSystemModelCrsNode*>(group->children().at(row)))
                    {
                        if (QStringLiteral("USER:%1").arg(crsNode->record().authId) == id)
                        {
                            // treat a change as a remove + add operation
                            const QModelIndex parentIndex = node2index(group);
                            beginRemoveRows(parentIndex, row, row);
                            delete group->takeChild(crsNode);
                            endRemoveRows();

                            userCrsAdded(id);
                            return;
                        }
                    }
                }
            }
        }

        CoordinateReferenceSystemModelCrsNode* CoordinateReferenceSystemModel::addRecord(const CrsDbRecord& record)
        {
            CoordinateReferenceSystemModelGroupNode* parentNode = mRootNode.get();
            std::unique_ptr< CoordinateReferenceSystemModelCrsNode > crsNode = std::make_unique< CoordinateReferenceSystemModelCrsNode>(record);

            QString groupName;
            QString groupId;
            QIcon groupIcon;
            if (record.authName == QLatin1String("ENU"))
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    groupName = str2qstr(std::string("ENU坐标系"));
                }
                else
                {
                    groupName = tr("ENU");
                }

                groupId = QStringLiteral("ENU");
                groupIcon = QIcon(":/status/skin/userdefinecrs.png");
            }
            else if (record.authName == QLatin1String("USER"))
            {
                if (AI3D::CORE::BlockObject::isChineseVersion())
                {
                    groupName = str2qstr(std::string("用户自定义"));
                }
                else
                {
                    groupName = tr("User-defined");
                }

                groupId = QStringLiteral("USER");
                groupIcon = QIcon(":/status/skin/userdefinecrs.png");
            }
            else if (record.authName == QLatin1String("CUSTOM"))
            {
                // the group is guaranteed to exist at this point
                groupId = QStringLiteral("CUSTOM");
            }
            else
            {
                groupId = EnumValueToKey(record.type);
                switch (record.type)
                {
                case ProjCore::CrsType::Unknown:
                    break;
                case ProjCore::CrsType::Geodetic:
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        groupName = str2qstr(std::string("笛卡尔坐标系"));
                    }
                    else
                    {
                        groupName = tr("Geodetic");
                    }

                    //groupIcon = QIcon(":/crs/skin/cancle.png");
                    break;
                case ProjCore::CrsType::Geocentric:
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        groupName = str2qstr(std::string("笛卡尔坐标系"));
                    }
                    else
                    {
                        groupName = tr("Cartesian systems");
                    }

                  //  groupIcon = QIcon(":/crs/skin/geocentricrs.png");
                    break;
                case ProjCore::CrsType::Geographic2d:
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        groupName = str2qstr(std::string("大地坐标系"));  // (2D)
                    }
                    else
                    {
                        groupName = tr("Geographic");  // (2D)
                    }

                    //groupIcon = QIcon(":/crs/skin/cancle.png");
                    break;

                case ProjCore::CrsType::Geographic3d:
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        groupName = str2qstr(std::string("大地坐标系(3D)"));
                    }
                    else
                    {
                        groupName = tr("Geographic (3D)");
                    }

                   // groupIcon = QIcon(":/crs/skin/running.png");
                    break;

                case ProjCore::CrsType::Vertical:
                    groupName = tr("Vertical");
                    break;

                case ProjCore::CrsType::Projected:
                case ProjCore::CrsType::DerivedProjected:
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        groupName = str2qstr(std::string("投影坐标系"));
                    }
                    else
                    {
                        groupName = tr("Projected");
                    }

                //    groupIcon = QIcon(":/crs/skin/pending.png");
                    break;

                case ProjCore::CrsType::Compound:
                    groupName = tr("Compound");
                    break;

                case ProjCore::CrsType::Temporal:
                    groupName = tr("Temporal");
                    break;

                case ProjCore::CrsType::Engineering:
                    groupName = tr("Engineering");
                    break;

                case ProjCore::CrsType::Bound:
                    groupName = tr("Bound");
                    break;

                case ProjCore::CrsType::Other:
                    groupName = tr("Other");
                    break;
                }
            }

            if (CoordinateReferenceSystemModelGroupNode* group = parentNode->getChildGroupNode(groupId))
            {
                parentNode = group;
            }
            else
            {
                std::unique_ptr< CoordinateReferenceSystemModelGroupNode > newGroup = std::make_unique< CoordinateReferenceSystemModelGroupNode >(groupName, groupIcon, groupId);
                parentNode->addChildNode(newGroup.get());
                parentNode = newGroup.release();
            }

            if ((record.authName != QLatin1String("USER") && record.authName != QLatin1String("CUSTOM")) && (record.type == ProjCore::CrsType::Projected || record.type 
                == ProjCore::CrsType::DerivedProjected))
            {
                QString projectionName = CoordinateReferenceSystemUtils::translateProjection(record.projectionAcronym);
                if (projectionName.isEmpty())
                    projectionName = tr("Other");

                if (CoordinateReferenceSystemModelGroupNode* group = parentNode->getChildGroupNode(record.projectionAcronym))
                {
                    parentNode = group;
                }
                else
                {
                    std::unique_ptr< CoordinateReferenceSystemModelGroupNode > newGroup = std::make_unique< CoordinateReferenceSystemModelGroupNode >(projectionName, QIcon(), record.projectionAcronym);
                    parentNode->addChildNode(newGroup.get());
                    parentNode = newGroup.release();
                }
            }

            parentNode->addChildNode(crsNode.get());
            return crsNode.release();
        }

        QModelIndex CoordinateReferenceSystemModel::addCustomCrs(const CoordinateReferenceSystem& crs)
        {
            CrsDbRecord userRecord;
            userRecord.authName = QStringLiteral("CUSTOM");
            userRecord.description = crs.description().isEmpty() ? tr("Custom CRS") : crs.description();
            userRecord.type = crs.type();

            CoordinateReferenceSystemModelGroupNode* group = mRootNode->getChildGroupNode(QStringLiteral("CUSTOM"));
            if (!group)
            {
                std::unique_ptr< CoordinateReferenceSystemModelGroupNode > newGroup = std::make_unique< CoordinateReferenceSystemModelGroupNode >(
                    tr("Custom"),
                    QIcon(":/status/skin/complete.png"), QStringLiteral("CUSTOM"));
                beginInsertRows(QModelIndex(), mRootNode->children().length(), mRootNode->children().length());
                mRootNode->addChildNode(newGroup.get());
                endInsertRows();
                group = newGroup.release();
            }

            const QModelIndex parentGroupIndex = node2index(group);

            const int newRow = group->children().size();
            beginInsertRows(parentGroupIndex, newRow, newRow);
            CoordinateReferenceSystemModelCrsNode* node = addRecord(userRecord);
            node->setWkt(crs.toWkt(ProjCore::CrsWktVariant::Preferred));
            node->setProj(crs.toProj());
            endInsertRows();

            return index(newRow, 0, parentGroupIndex);
        }

        CoordinateReferenceSystemModelNode* CoordinateReferenceSystemModel::index2node(const QModelIndex& index) const
        {
            if (!index.isValid())
                return mRootNode.get();

            return reinterpret_cast<CoordinateReferenceSystemModelNode*>(index.internalPointer());
        }

        QModelIndex CoordinateReferenceSystemModel::node2index(CoordinateReferenceSystemModelNode* node) const
        {
            if (!node || !node->parent())
                return QModelIndex(); // this is the only root item -> invalid index

            QModelIndex parentIndex = node2index(node->parent());

            int row = node->parent()->children().indexOf(node);
            Q_ASSERT(row >= 0);
            return index(row, 0, parentIndex);
        }

        QModelIndex CoordinateReferenceSystemModel::indexOfParentTreeNode(CoordinateReferenceSystemModelNode* parentNode) const
        {
            Q_ASSERT(parentNode);

            CoordinateReferenceSystemModelNode* grandParentNode = parentNode->parent();
            if (!grandParentNode)
                return QModelIndex();  // root node -> invalid index

            int row = grandParentNode->children().indexOf(parentNode);
            Q_ASSERT(row >= 0);

            return createIndex(row, 0, parentNode);
        }

        ///@cond PRIVATE
        CoordinateReferenceSystemModelNode::~CoordinateReferenceSystemModelNode()
        {
            qDeleteAll(mChildren);
        }

        CoordinateReferenceSystemModelNode* CoordinateReferenceSystemModelNode::takeChild(CoordinateReferenceSystemModelNode* node)
        {
            return mChildren.takeAt(mChildren.indexOf(node));
        }

        void CoordinateReferenceSystemModelNode::addChildNode(CoordinateReferenceSystemModelNode* node)
        {
            if (!node)
                return;

            Q_ASSERT(!node->mParent);
            node->mParent = this;

            mChildren.append(node);
        }

        void CoordinateReferenceSystemModelNode::deleteChildren()
        {
            qDeleteAll(mChildren);
            mChildren.clear();
        }

        CoordinateReferenceSystemModelGroupNode* CoordinateReferenceSystemModelNode::getChildGroupNode(const QString& id)
        {
            for (CoordinateReferenceSystemModelNode* node : std::as_const(mChildren))
            {
                if (node->nodeType() == NodeGroup)
                {
                    CoordinateReferenceSystemModelGroupNode* groupNode = down_cast<CoordinateReferenceSystemModelGroupNode*>(node);
                    if (groupNode && groupNode->id() == id)
                        return groupNode;
                }
            }
            return nullptr;

        }

        CoordinateReferenceSystemModelGroupNode::CoordinateReferenceSystemModelGroupNode(const QString& name, const QIcon& icon, const QString& id)
            : mId(id)
            , mName(name)
            , mIcon(icon)
        {

        }

        CoordinateReferenceSystemModelCrsNode::CoordinateReferenceSystemModelCrsNode(const CrsDbRecord& record)
            : mRecord(record)
        {

        }
        ///@endcond


        //
        // CoordinateReferenceSystemProxyModel
        //

        CoordinateReferenceSystemProxyModel::CoordinateReferenceSystemProxyModel(QObject* parent,QString strLastEnuData, QString strLastEnuAuthId)
            : QSortFilterProxyModel(parent)
            , mModel(new CoordinateReferenceSystemModel(this,strLastEnuData,strLastEnuAuthId))
        {
            setSourceModel(mModel);
            setDynamicSortFilter(true);
            setSortLocaleAware(true);
            setFilterCaseSensitivity(Qt::CaseInsensitive);
            setRecursiveFilteringEnabled(true);
            sort(0);
            //std::cout << "inside crs proxymodel constructor:" << strLastEnuData.toStdString() << " authid:" << strLastEnuAuthId.toStdString() << std::endl;
        }
        
        CoordinateReferenceSystemModel* CoordinateReferenceSystemProxyModel::coordinateReferenceSystemModel()
        {
            return mModel;
        }

        CoordinateReferenceSystemModel* CoordinateReferenceSystemProxyModel::getCoordinateReferenceSystemModel()
        {
            return mModel;
        }

        const CoordinateReferenceSystemModel* CoordinateReferenceSystemProxyModel::coordinateReferenceSystemModel() const
        {
            return mModel;
        }

        void CoordinateReferenceSystemProxyModel::setFilters(CoordinateReferenceSystemProxyModel::Filters filters)
        {
            if (mFilters == filters)
                return;

            mFilters = filters;
            invalidateFilter();
        }

        void CoordinateReferenceSystemProxyModel::setFilterString(const QString& filter)
        {
            mFilterString = filter;
            invalidateFilter();
        }

        void CoordinateReferenceSystemProxyModel::setFilterAuthIds(const QSet<QString>& filter)
        {
            if (mFilterAuthIds == filter)
                return;

            mFilterAuthIds.clear();
            mFilterAuthIds.reserve(filter.size());
            for (const QString& id : filter)
            {
                mFilterAuthIds.insert(id.toUpper());
            }
            invalidateFilter();
        }

        void CoordinateReferenceSystemProxyModel::setFilterDeprecated(bool filter)
        {
            if (mFilterDeprecated == filter)
                return;

            mFilterDeprecated = filter;
            invalidateFilter();
        }

        bool CoordinateReferenceSystemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
        {
            if (mFilterString.trimmed().isEmpty() && !mFilters && !mFilterDeprecated && mFilterAuthIds.isEmpty())
                return true;

            const QModelIndex sourceIndex = mModel->index(sourceRow, 0, sourceParent);
            const CoordinateReferenceSystemModelNode::NodeType nodeType = static_cast<CoordinateReferenceSystemModelNode::NodeType>(sourceModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::NodeType)).toInt());
            switch (nodeType)
            {
            case CoordinateReferenceSystemModelNode::NodeGroup:
                return false;
            case CoordinateReferenceSystemModelNode::NodeCrs:
                break;
            }

            const bool deprecated = sourceModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Deprecated)).toBool();
            if (mFilterDeprecated && deprecated)
                return false;

            if (mFilters)
            {
                const ProjCore::CrsType type = sourceModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Type)).value< ProjCore::CrsType >();
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
                    if (!mFilters.testFlag(Filter::FilterHorizontal))
                        return false;
                    break;

                case ProjCore::CrsType::Vertical:
                    if (!mFilters.testFlag(Filter::FilterVertical))
                        return false;
                    break;

                case ProjCore::CrsType::Compound:
                    if (!mFilters.testFlag(Filter::FilterCompound))
                        return false;
                    break;
                }
            }

            const QString authid = sourceModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
            if (!mFilterAuthIds.isEmpty())
            {
                if (!mFilterAuthIds.contains(authid.toUpper()))
                    return false;
            }

            if (!mFilterString.trimmed().isEmpty())
            {
                const QString name = sourceModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();
                if (!(name.contains(mFilterString, Qt::CaseInsensitive)
                    || authid.contains(mFilterString, Qt::CaseInsensitive)))
                    return false;
            }
            return true;
        }

        bool CoordinateReferenceSystemProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
        {
            CoordinateReferenceSystemModelNode::NodeType leftType = static_cast<CoordinateReferenceSystemModelNode::NodeType>(sourceModel()->data(left, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::NodeType)).toInt());
            CoordinateReferenceSystemModelNode::NodeType rightType = static_cast<CoordinateReferenceSystemModelNode::NodeType>(sourceModel()->data(right, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::NodeType)).toInt());

            if (leftType != rightType)
            {
                if (leftType == CoordinateReferenceSystemModelNode::NodeGroup)
                    return true;
                else if (rightType == CoordinateReferenceSystemModelNode::NodeGroup)
                    return false;
            }

            const QString leftStr = sourceModel()->data(left).toString().toLower();
            const QString rightStr = sourceModel()->data(right).toString().toLower();

            if (leftType == CoordinateReferenceSystemModelNode::NodeGroup)
            {
                // both are groups -- ensure USER group comes last, and CUSTOM group comes first
                const QString leftGroupId = sourceModel()->data(left, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::GroupId)).toString();
                const QString rightGroupId = sourceModel()->data(left, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::GroupId)).toString();
                if (leftGroupId == QLatin1String("USER"))
                    return false;
                if (rightGroupId == QLatin1String("USER"))
                    return true;

                if (leftGroupId == QLatin1String("CUSTOM"))
                    return true;
                if (rightGroupId == QLatin1String("CUSTOM"))
                    return false;
            }

            // default sort is alphabetical order
            return QString::localeAwareCompare(leftStr, rightStr) < 0;
        }


    }
}