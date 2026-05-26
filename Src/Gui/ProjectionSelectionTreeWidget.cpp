/***************************************************************************
 *   qgsprojectionselector.cpp                                             *
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "Gui/ProjectionSelectionTreeWidget.h"
#include "Gui/FilterLineEdit.h"
#include "Gui/MohackerWin.h"

//standard includes
#include <sqlite3.h>

#include "Core/Proj/QProj.h"
#include "Core/Logging.h"
#include "Core/Proj/CoordinateReferenceSystem.h"
#include "QSettings"

#include "Core/Proj/ProjOperation.h"

#include "Gui/CoordinateReferenceSystemModel.h"

#ifdef USE_AI3D_PROJ
#include "Core/Proj/QProj.h"
#include "Core/Proj/CoordinateReferenceSystemRegistry.h"
#include "Core/Proj/CrsSettings.h"
#endif

//qt includes
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QFileInfo>
#include <QHeaderView>
#include <QResizeEvent>
#include <QMessageBox>
#include <QRegularExpression>

#include "Gui/ControlPointsEditorWin.h"
#include "Core/BlockObject.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

namespace AI3D
{
    namespace PROJ
    {

        ProjectionSelectionTreeWidget::ProjectionSelectionTreeWidget(QWidget* parent, CoordinateReferenceSystemProxyModel::Filters filters,QString strLastEnuData, QString strLastEnuAuthId)
            : QDialog(parent)
        {
            setWindowFlags(windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint);

            setAttribute(Qt::WA_DeleteOnClose);

            setStyleSheet("QWidget { background-color:rgb(45,48,53);} ");

            setupUi(this);

            //std::cout << "inside pstw：" << __LINE__ << std::endl;
            ///std::cout << "got lastEnuData:" << strLastEnuData.toStdString() << std::endl;
            //std::cout << "inside pstw：" << __LINE__ << std::endl;

            ///mCrsModel = new CoordinateReferenceSystemProxyModel(this,strLastEnuData);
            mCrsModel = new CoordinateReferenceSystemProxyModel(this, strLastEnuData,strLastEnuAuthId);
            mCrsModel->setFilters(filters);

            mRecentCrsModel = new RecentCoordinateReferenceSystemTableModel(this);
            mRecentCrsModel->setFilters(filters);
            //std::cout << "inside pstw：" << __LINE__ << std::endl;

            lstCoordinateSystems->setModel(mCrsModel);
            int count = mCrsModel->coordinateReferenceSystemModel()->GetNumOfValidItem();
            leNumOfItems->setText(QString::fromStdString(std::to_string(count)));
            
            lstCoordinateSystems->setSelectionBehavior(QAbstractItemView::SelectRows);
            lstCoordinateSystems->expandAll();
            //std::cout << "inside pstw：" << __FUNCTION__ << " " << __LINE__ << std::endl;
            //std::cout << "inside pstw：" << __FUNCTION__ << std::endl;
//            std::cout << "inside pstw：" << std::endl;
            lstRecent->setModel(mRecentCrsModel);
            lstRecent->viewport()->setAttribute(Qt::WA_Hover);
            lstRecent->setSelectionBehavior(QAbstractItemView::SelectRows);
            lstRecent->setRootIsDecorated(false);

            RemoveRecentCrsDelegate* removeDelegate = new RemoveRecentCrsDelegate(lstRecent);
            lstRecent->setItemDelegateForColumn(2, removeDelegate);
            lstRecent->viewport()->installEventFilter(removeDelegate);

            if (mCrsModel->rowCount() == 1)
            {
                // if only one group, expand it by default
                lstCoordinateSystems->expand(mCrsModel->index(0, 0, QModelIndex()));
            }

            QFont f = teProjection->font();
            f.setPointSize(f.pointSize() - 2);
            teProjection->setFont(f);

            //chy 
            leSearch->setShowSearchIcon(true);

            connect(lstCoordinateSystems, &QTreeView::doubleClicked, this, &ProjectionSelectionTreeWidget::lstCoordinateSystemsDoubleClicked);
            connect(lstRecent, &QTreeView::doubleClicked, this, &ProjectionSelectionTreeWidget::lstRecentDoubleClicked);
            connect(lstRecent, &QTreeView::clicked, this, &ProjectionSelectionTreeWidget::lstRecentClicked);
            connect(lstCoordinateSystems->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ProjectionSelectionTreeWidget::lstCoordinateSystemsSelectionChanged);
            connect(lstRecent->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ProjectionSelectionTreeWidget::lstRecentSelectionChanged);


            connect(cbxHideDeprecated, &QCheckBox::toggled, this, [=](bool selected)
                {
                    mCrsModel->setFilterDeprecated(selected);
                    mRecentCrsModel->setFilterDeprecated(selected);
                });
            connect(leSearch, &FilterLineEdit::textChanged, this, [=](const QString& filter)
                {
                 
                    mCrsModel->setFilterString(filter);
                    mRecentCrsModel->setFilterString(filter);
                    lstCoordinateSystems->expandAll();
                    
                    if (leSearch->text().isEmpty())
                    {
                        int count = mCrsModel->coordinateReferenceSystemModel()->GetNumOfValidItem();
                        leNumOfItems->setText(QString::fromStdString(std::to_string(count)));
                    }
                    else
                    {
                        int count = mCrsModel->rowCount();

                        // std::cout << count << " coj " << std::endl;
                        ///leNumOfItems->setText(QString::fromStdString(std::to_string(count)));
                        calcNumOfAllLeafNodes();
                    }
                });

            //mAreaCanvas->setVisible( mShowMap );

          //  lstCoordinateSystems->header()->setSectionResizeMode(AuthidColumn, QHeaderView::Stretch);
			///      lstCoordinateSystems->header()->setDefaultSectionSize(200);
         ///   lstCoordinateSystems->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
            //lstCoordinateSystems->header()->setSectionResizeMode(AuthidColumn, QHeaderView::Fixed);
            //lstCoordinateSystems->header()->resizeSection(AuthidColumn, 80);
            //lstCoordinateSystems->setColumnWidth(1, 200);
            //lstCoordinateSystems->setColumnWidth(2, 130);
            
            ///lstCoordinateSystems->header()->setStretchLastSection(false);
            //lstCoordinateSystems->resizeColumnToContents(1);
          
            lstRecent->header()->setSectionResizeMode(AuthidColumn, QHeaderView::Stretch);

            // Clear Crs Column
            lstRecent->header()->setMinimumSectionSize(10);
            lstRecent->header()->setStretchLastSection(false);
            lstRecent->header()->resizeSection(ClearColumn, 20);

            // Clear recent crs context menu
            lstRecent->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(lstRecent, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pos)
                {
                    // If list is empty, do nothing
                    if (lstRecent->model()->rowCount() == 0)
                        return;
                    QMenu menu;
                    // Clear selected
                    const QModelIndex currentIndex = lstRecent->indexAt(pos);
                    if (currentIndex.isValid())
                    {
                        QAction* clearSelected = menu.addAction(QIcon(":/status/skin/cancle.png"), tr("Remove Selected CRS from Recently Used CRS"));
                        connect(clearSelected, &QAction::triggered, this, [this, currentIndex] { removeRecentCrsItem(currentIndex); });
                        menu.addSeparator();
                    }
                    // Clear all
                    QAction* clearAll = menu.addAction(QIcon(":/status/skin/cancle.png"), tr("Clear All Recently Used CRS"));
                    connect(clearAll, &QAction::triggered, this, &ProjectionSelectionTreeWidget::clearRecentCrs);
                    menu.exec(lstRecent->viewport()->mapToGlobal(pos));
                });

            // Install event fiter to catch delete key press on the recent crs list
            lstRecent->installEventFilter(this);

            mCheckBoxNoProjection->setHidden(true);
            mCheckBoxNoProjection->setEnabled(false);
            connect(mCheckBoxNoProjection, &QCheckBox::toggled, this, [=]
                {
                    if (!mBlockSignals)
                    {
                        ///emit crsSelected();
                        emit hasValidSelectionChanged(hasValidSelection());
                    }
                });

            connect(mCheckBoxNoProjection, &QCheckBox::toggled, this, [=](bool checked)
                {
                    if (mCheckBoxNoProjection->isEnabled())
                    {
                        mFrameProjections->setDisabled(checked);
                    }
                });

            connect(leLongitude, &QLineEdit::textChanged, this,
                &ProjectionSelectionTreeWidget::posTextChanged);
            connect(leLatitude, &QLineEdit::textChanged, this,
                &ProjectionSelectionTreeWidget::posTextChanged);

            QSettings settings;
            mSplitter->restoreState(settings.value(QStringLiteral("Windows/ProjectionSelector/splitterState")).toByteArray());

            lstCoordinateSystems->header()->setDefaultAlignment(Qt::AlignCenter);

//            lstCoordinateSystems->header()->setStyleSheet(
//                "QHeaderView::section { background:rgb(61,67,78);color:rgb(165,165,165);}"
//            );
        
            lstCoordinateSystems->setStyleSheet
            (
                "QTreeView::branch:has-children:!has-siblings:closed,"
                "QTreeView::branch:closed:has-children:has-siblings { border-image:none; image:url(:/new/prefix1/skin/tvright16.png)} "
                "QTreeView::branch:open:has-children:!has-siblings,"
                "QTreeView::branch:open:has-children:has-siblings { border-image:none; image:url(:/new/prefix1/skin/tvdown16.png)} "
                "QHeaderView::section { background:rgb(61,67,78);color:rgb(165,165,165);}"
            );

            ///adjustEnuItem();

            leSelection->setReadOnly(true);
            teDefinition->setReadOnly(true);
            leNumOfItems->setReadOnly(true);

            QDoubleValidator* doubleValidator = new QDoubleValidator(this);
            leLatitude->setValidator(doubleValidator);
            leLongitude->setValidator(doubleValidator);

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblTitle->setText("空间参考系统");
                lblItem->setText("个项目");
                lblSelection->setText("选择集:");
                lblType->setText("类型:");
                lblDefinition->setText("定义:");
                label_4->setText("空间参考系统");
                label_5->setText(QApplication::translate("QgsProjectionSelectorBase", "筛选器", nullptr));
                butOk->setText("确定");
                butCancel->setText("取消");
            }
        }

        ProjectionSelectionTreeWidget::~ProjectionSelectionTreeWidget()
        {
            QSettings settings;
            settings.setValue(QStringLiteral("Windows/ProjectionSelector/splitterState"), mSplitter->saveState());

            // Push current projection to front, only if set
        }

        void ProjectionSelectionTreeWidget::adjustEnuItem()
        {
            QItemSelectionModel* pSelectionModel = lstCoordinateSystems->selectionModel();

            if (pSelectionModel)
            {
                QAbstractItemModel* pModel = pSelectionModel->model();
                if (pModel && pModel->hasChildren())
                {
                    int rowCount = pModel->rowCount();
                    for (int i = 0; i < rowCount; i++)
                    {
                        QModelIndex modelFirstLevelCategory = pModel->index(i, 0);
                        int rowCount2 = pModel->rowCount(modelFirstLevelCategory);

                        if (pModel->rowCount(modelFirstLevelCategory) > 0)
                        {
                            int rowCount2 = pModel->rowCount(modelFirstLevelCategory);
                            for (int j = 0; j < rowCount2; j++)
                            {
                                QModelIndex modelSecondLevelCategory = pModel->index(j, 0, modelFirstLevelCategory);
                                if (pModel->rowCount(modelSecondLevelCategory) > 0)
                                {

                                }
                                else
                                {

                                }
                            }
                        }
                        else
                        {
                            pModel->setData(modelFirstLevelCategory, QIcon(":/new/prefix1/skinbutton/ycorner.png"), Qt::DecorationRole);
                        }
                    }
                }
            }
        }

        void ProjectionSelectionTreeWidget::resizeEvent(QResizeEvent* event)
        {
            lstCoordinateSystems->header()->resizeSection(NameColumn, event->size().width() - 180);

            lstCoordinateSystems->header()->resizeSection(AuthidColumn, 180);


            lstRecent->header()->resizeSection(NameColumn, event->size().width() - 260);
            lstRecent->header()->resizeSection(AuthidColumn, 240);
            lstRecent->header()->resizeSection(ClearColumn, 20);
        }

        bool ProjectionSelectionTreeWidget::eventFilter(QObject* obj, QEvent* ev)
        {
            if (obj != lstRecent)
                return false;

            if (ev->type() != QEvent::KeyPress)
                return false;

            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
            if (keyEvent->matches(QKeySequence::Delete))
            {
                const QModelIndex currentIndex = lstRecent->selectionModel()->selectedRows(0).value(0);
                if (currentIndex.isValid())
                    removeRecentCrsItem(currentIndex);
                return true;
            }

            return false;
        }

        void ProjectionSelectionTreeWidget::selectCrsByAuthId(const QString& authid)
        {
            //std::cout << "inside " << __FUNCTION__ << " " << __LINE__ << " authid:" << authid.toStdString() << std::endl;
            const QModelIndex  sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex(authid);
            if (!sourceIndex.isValid())
            {
            //    std::cout << "inside " << __FUNCTION__ << " " << __LINE__ << " authid:" << authid.toStdString() << std::endl;
                return;
            }

            const QModelIndex proxyIndex = mCrsModel->mapFromSource(sourceIndex);
            if (proxyIndex.isValid())
            {
            //    std::cout << "inside " << __FUNCTION__ << " " << __LINE__ << " authid:" << authid.toStdString() << std::endl;
                lstCoordinateSystems->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                lstCoordinateSystems->scrollTo(proxyIndex);
            }
            else
            {
                // deselect the selected item to avoid confusing the user
            //    std::cout << "inside " << __FUNCTION__ << " " << __LINE__ << " authid:" << authid.toStdString() << std::endl;
                lstCoordinateSystems->clearSelection();
                lstRecent->clearSelection();
                teProjection->clear();
            }
        }

        void ProjectionSelectionTreeWidget::selectCrsByName(const QString& name)
        {
            //std::cout << "inside " << __FUNCTION__ << " name:" << name.toStdString() << std::endl;
            const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->nameToIndex(name);
            if (!sourceIndex.isValid())
                return;

            const QModelIndex proxyIndex = mCrsModel->mapFromSource(sourceIndex);
            if (proxyIndex.isValid())
            {
                lstCoordinateSystems->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                lstCoordinateSystems->scrollTo(proxyIndex);
            }
            else
            {
                // deselect the selected item to avoid confusing the user
                lstCoordinateSystems->clearSelection();
                lstRecent->clearSelection();
//                teProjection->clear();
            }
        }

        void ProjectionSelectionTreeWidget::setCrs(const CoordinateReferenceSystem& crs)
        {
            if (!crs.isValid())
            {
                std::cout << "ptw setCrs invalid." << std::endl;
                mCheckBoxNoProjection->setChecked(true);
            }
            else
            {
                mBlockSignals = true;
                mCheckBoxNoProjection->setChecked(false);
                mBlockSignals = false;

                //std::cout << "ptw setCrs:" << crs.description().toStdString() << " " << crs.authid().toStdString() << std::endl;

                if (!crs.authid().isEmpty())
                    selectCrsByAuthId(crs.authid());
                else
                    loadUnknownCrs(crs);

                const bool changed = crs != ProjectionSelectionTreeWidget::crs();
                if (changed)
                {
                    ///emit crsSelected();
                    emit hasValidSelectionChanged(hasValidSelection());
                }
            }
        }

        //void ProjectionSelectionTreeWidget::setPreviewRect( const QgsRectangle &rect )
        //{
        //  mAreaCanvas->setCanvasRect( rect );
        //}
        //
        //QgsRectangle ProjectionSelectionTreeWidget::previewRect() const
        //{
        //  return mAreaCanvas->canvasRect();
        //}

        CoordinateReferenceSystemProxyModel::Filters ProjectionSelectionTreeWidget::filters() const
        {
            return mCrsModel->filters();
        }

        void ProjectionSelectionTreeWidget::setFilters(CoordinateReferenceSystemProxyModel::Filters filters)
        {
            mCrsModel->setFilters(filters);
            mRecentCrsModel->setFilters(filters);
            if (mCrsModel->rowCount() == 1)
            {
                // if only one group, expand it by default
                lstCoordinateSystems->expand(mCrsModel->index(0, 0, QModelIndex()));
            }
        }

        CoordinateReferenceSystem ProjectionSelectionTreeWidget::crs() const
        {
            if (mCheckBoxNoProjection->isEnabled() && mCheckBoxNoProjection->isChecked())
                return CoordinateReferenceSystem();

            const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);
            
            const QString authid = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
            const QString Name = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();

            //std::cout << "inside crs:" << Name.toStdString() << " /authid: " << authid.toStdString() << std::endl;

            if (Name.contains(AI3D::GUI::MohackerWin::localENUPrefix(), Qt::CaseInsensitive))
            {
            //    std::cout << "inside crs2:" << Name.toStdString() << " / " << authid.toStdString() << std::endl;
                CoordinateReferenceSystem crs;
                ///crs.CreateFromENUDefinition(Name);
                crs.CreateFromENUDefinition(authid);
                if (crs.isValid())
                {
              //      std::cout << "crs2 valid." << std::endl;
                }
                else
                {
              //      std::cout << "crs2 invalid." << std::endl;
                }
                return crs;
            }
            else if (!authid.isEmpty())
            {
                if (authid.contains("Local:0",Qt::CaseInsensitive))
                {
                    CoordinateReferenceSystem crs;
                    //QString authId = "Local:0";
                    crs.createFromString(authid);
                    return crs;
                }
                else if (authid.contains("enu", Qt::CaseInsensitive))
                {
                    CoordinateReferenceSystem crs;
                    crs.CreateFromENUDefinition(authid);
                    if (crs.isValid())
                    {
                //        std::cout << "crs32 valid." << std::endl;
                    }
                    else
                    {
                //        std::cout << "crs32 invalid." << std::endl;
                    }

                    return crs;
                }

                return CoordinateReferenceSystem::fromOgcWmsCrs(authid);
            }
            else
            {
                // custom CRS
                const QString wkt = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Wkt)).toString();
                const QString proj = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Proj)).toString();
                //std::cout << "inside crs3:" << Name.toStdString() << " / " << authid.toStdString() << std::endl;
                if (!wkt.isEmpty())
                    return CoordinateReferenceSystem::fromWkt(wkt);
                else if (!proj.isEmpty())
                    return CoordinateReferenceSystem::fromProj(proj);
                else
                    return CoordinateReferenceSystem();
            }
        }

        void ProjectionSelectionTreeWidget::setShowNoProjection(bool show)
        {
            mCheckBoxNoProjection->setVisible(show);
            mCheckBoxNoProjection->setEnabled(show);
            if (show)
            {
                mFrameProjections->setDisabled(mCheckBoxNoProjection->isChecked());
            }
        }

        void ProjectionSelectionTreeWidget::setShowBoundsMap(bool show)
        {
            mShowMap = show;
            //mAreaCanvas->setVisible( show );
        }

        bool ProjectionSelectionTreeWidget::showNoProjection() const
        {
            return !mCheckBoxNoProjection->isHidden();
        }

        void ProjectionSelectionTreeWidget::setNotSetText(const QString& text)
        {
            mCheckBoxNoProjection->setText(text);
        }

        bool ProjectionSelectionTreeWidget::showBoundsMap() const
        {
            return mShowMap;
        }

        bool ProjectionSelectionTreeWidget::hasValidSelection() const
        {
            if (mCheckBoxNoProjection->isChecked())
            {
                return true;
            }
            else
            {
                const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);
                const QString authid = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
                const QString wkt = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Wkt)).toString();
                const QString proj = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Proj)).toString();
                return !authid.isEmpty() || !wkt.isEmpty() || !proj.isEmpty();
            }
        }

        void ProjectionSelectionTreeWidget::setLastEnuData(QString& lastEnuData)
        {
            mCrsModel->coordinateReferenceSystemModel()->setLastEnuData(lastEnuData);
            ///selectCrsByName(QString("Local East-North-Up (ENU)"));
        }

        void ProjectionSelectionTreeWidget::posTextChanged(const QString& text)
        {
            QLineEdit* pSourceEdit = qobject_cast<QLineEdit *>(sender());

            if (strEnuBaseName.isEmpty())
                return;

            QString strLatitude = leLatitude->text();
            QString strLongitude = leLongitude->text();

            if (strLatitude.isEmpty() || strLongitude.isEmpty())
                return;

            // note: check the existence of the origin part in strEnuBaseName later;
            QString strCompoundName;

            QString strEnuBaseName_ = strEnuBaseName;
            QString strStartToBeStripped = "; origin:";

            if (strEnuBaseName_.contains(strStartToBeStripped))
            {
                int index = strEnuBaseName_.indexOf(strStartToBeStripped);
                if (index > 0)
                {
                    strEnuBaseName_ = strEnuBaseName_.left(index);
                }
            }

            //std::cout << "inside/e " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ 
            //    << strLatitude.toStdString() << " " << strLongitude.toStdString() << std::endl;

            QString strCompoundAuthId;

            QString strSimpleAuthName = "ENU";
            QString strSimpleAuthId;

            if (!strLongitude.isEmpty() && !strLatitude.isEmpty())
            {
            //    std::cout << "inside/a " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;

                strCompoundName = strEnuBaseName_ + "; origin:" + strLatitude + "N " + strLongitude + "E";
                strCompoundAuthId = QString("ENU:%1,%2").arg(strLatitude).arg(strLongitude);
                strSimpleAuthId = QString("%1,%2").arg(strLatitude).arg(strLongitude);
            }
            else if (!strLatitude.isEmpty())
            {
            //    std::cout << "inside/b " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                strCompoundName = strEnuBaseName_ + "; origin:" + strLatitude + "N";
                strCompoundAuthId = QString("ENU:%1,0").arg(strLatitude);
                strSimpleAuthId = QString("%1,0").arg(strLatitude);

            }
            else if (!strLongitude.isEmpty())
            {
            //    std::cout << "inside/c " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                strCompoundName = strEnuBaseName_ + "; origin:" + strLongitude + "E";
                strCompoundAuthId = QString("ENU:0,%1").arg(strLongitude);
                strSimpleAuthId = QString("0,%1").arg(strLongitude);
            }
            else
            {
            //    std::cout << "inside/d " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                strCompoundName = strEnuBaseName_;
                strSimpleAuthId = QString("0,0");
            }
            leSelection->setText(strCompoundName);
            //std::cout << "set data0:" << strCompoundName.toStdString() << std::endl;

            QItemSelectionModel* pSelectionModel = lstCoordinateSystems->selectionModel();
            if (pSelectionModel)
            {
                QAbstractItemModel* pModel = pSelectionModel->model();
                const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);
                //std::cout << "set data1:" << strCompoundName.toStdString() << std::endl;
                if (currentIndex.isValid())
                {
                //    std::cout << "set data2:" << strCompoundName.toStdString() << std::endl;
                    //pModel->setData(currentIndex, strCompoundName, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name));
                    //pModel->setData(currentIndex, strCompoundName);
                    //pModel->setData(currentIndex, strCompoundName, Qt::DisplayRole);

             ///       pModel->setData(currentIndex, strCompoundName, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId));
             
              ///      const QModelIndex sourceIndex = mCrsModel->mapToSource(selectedProxyIndex);

             ///       mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
             ///       const QString crsName = mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();

                    if (1)
                    {
                        //mCrsModel->getCoordinateReferenceSystemModel()->setDefinition(mCrsModel->getCoordinateReferenceSystemModel()->
                        //mCrsModel->getCoordinateReferenceSystemModel()->setDefinition(currentIndex, strCompoundName);
                        mCrsModel->coordinateReferenceSystemModel()->setLastEnuData(strCompoundName, strCompoundAuthId);
                       // mCrsModel->getCoordinateReferenceSystemModel()->setData(currentIndex, strCompoundName, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name));
                       // mCrsModel->getCoordinateReferenceSystemModel()->setData(currentIndex, strCompoundName, Qt::DisplayRole);
                       // mCrsModel->getCoordinateReferenceSystemModel()->setData(currentIndex, strCompoundAuthId, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId));
                        
                        //mCrsModel->getCoordinateReferenceSystemModel()->setData(selectedIndex, strCompoundName, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name));
                        //mCrsModel->getCoordinateReferenceSystemModel()->setData(selectedIndex, strCompoundName, Qt::DisplayRole);
                        //mCrsModel->getCoordinateReferenceSystemModel()->setData(selectedIndex, strCompoundAuthId, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId));

                        mCrsModel->getCoordinateReferenceSystemModel()->setDefinition(selectedIndex,strCompoundName, strSimpleAuthName, strSimpleAuthId);
              //          std::cout << " posText:" << strCompoundName.toStdString() << " authid:" << strCompoundAuthId.toStdString() << std::endl;
              //          std::cout << " posText2:" << strSimpleAuthName.toStdString() << " / " << strSimpleAuthId.toStdString() << std::endl;
                        mCrsModel->invalidate();
                    }
                    else
                    {
                        mCrsModel->coordinateReferenceSystemModel()->setLastEnuData(strCompoundName,strCompoundAuthId);
                        mCrsModel->coordinateReferenceSystemModel()->rebuild();
                        lstCoordinateSystems->expandAll();
                        //pSelectionModel->setCurrentIndex(lstCoordinateSystems->index)
                        const QModelIndex& parentIndex = mCrsModel->coordinateReferenceSystemModel()->index(0, 0);

                        selectCrsByName(QString("Local East-North-Up (ENU)"));
                        //lstCoordinateSystems->setCurrentIndex(mCrsModel->coordinateReferenceSystemModel()->index(0, 0,parentIndex));
                        if (pSourceEdit)
                            pSourceEdit->setFocus();
                    }
                }
            }           
        }

        void ProjectionSelectionTreeWidget::calcNumOfAllLeafNodes()
        {
            QItemSelectionModel* pSelectionModel = lstCoordinateSystems->selectionModel();
            int iTotalLeftNodes = 0;

            if (pSelectionModel)
            {
                QAbstractItemModel* pModel = pSelectionModel->model();
                if (pModel && pModel->hasChildren())
                {
                    int rowCount = pModel->rowCount();
                    for (int i = 0; i < rowCount; i++)
                    {
                        QModelIndex modelFirstLevelCategory = pModel->index(i, 0);
                        int rowCount2 = pModel->rowCount(modelFirstLevelCategory);

                        if (pModel->rowCount(modelFirstLevelCategory) > 0)
                        {
                            int rowCount2 = pModel->rowCount(modelFirstLevelCategory);
                            for (int j = 0; j < rowCount2; j++)
                            {
                                QModelIndex modelSecondLevelCategory = pModel->index(j, 0, modelFirstLevelCategory);
                                if (pModel->rowCount(modelSecondLevelCategory) > 0)
                                {
                                    iTotalLeftNodes += pModel->rowCount(modelSecondLevelCategory);
                                }
                                else
                                {
                                    iTotalLeftNodes++;
                                }
                            }                           
                        }
                        else
                        {
                            //iTotalLeftNodes++;
                        }
                    }
                }
            }

            leNumOfItems->setText(QString::number(iTotalLeftNodes));
        }

        void ProjectionSelectionTreeWidget::setOgcWmsCrsFilter(const QSet<QString>& crsFilter)
        {
            mCrsModel->setFilterAuthIds(crsFilter);
        }

        void ProjectionSelectionTreeWidget::loadUnknownCrs(const CoordinateReferenceSystem& crs)
        {
            const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->addCustomCrs(crs);
            lstCoordinateSystems->selectionModel()->select(mCrsModel->mapFromSource(sourceIndex), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            lstCoordinateSystems->scrollTo(mCrsModel->mapFromSource(sourceIndex));
        }

        void ProjectionSelectionTreeWidget::updateBoundsPreview()
        {
            const CoordinateReferenceSystem currentCrs = crs();
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (!currentCrs.isValid())
                return;
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            /* QgsRectangle rect = currentCrs.bounds();*/
            QString extentString = tr("");//Extent not known
          /*  mAreaCanvas->setPreviewRect(rect);
            if (!qgsDoubleNear(rect.area(), 0.0))
            {
                extentString = QStringLiteral("%1, %2, %3, %4")
                    .arg(rect.xMinimum(), 0, 'f', 2)
                    .arg(rect.yMinimum(), 0, 'f', 2)
                    .arg(rect.xMaximum(), 0, 'f', 2)
                    .arg(rect.yMaximum(), 0, 'f', 2);
            }*/
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            QStringList properties;
            QString str = currentCrs.authid();
            QString Type;
            QString strLatitude;
            QString strLongitude;
            
            //std::cout << "inside " << __FILE__ << " " << __LINE__ << " " << str.toStdString() << std::endl;
            if (/*leLatitude->isVisible() &&*/ str.contains("ENU"))
            {
                int iColonPos = str.lastIndexOf(":");
                int iCommaPos = str.lastIndexOf(",");
              //  std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << str.toStdString() << std::endl;
                if (iColonPos > 0 && iCommaPos > 0 && iColonPos < iCommaPos)
                {
                    strLatitude = str.mid(iColonPos + 1, iCommaPos - iColonPos - 1);
                    strLongitude = str.mid(iCommaPos + 1);
                    strLatitude = strLatitude.trimmed();
                    strLongitude = strLongitude.trimmed();
                //    std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << str.toStdString() << std::endl;
                //    std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << strLatitude.toStdString() << " "
                //        << strLongitude.toStdString() << std::endl;
                }
            }

            //std::cout << " type " << Type.toStdString() << std::endl;
            str.toUpper();
            if (currentCrs.isGeographic())
                properties << tr("Geographic (uses latitude and longitude for coordinates)");
            else
            {
                /*if (str.contains("LOCAL") || str =="EPSG:4978")
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        Type= tr("笛卡尔坐标系");
                    }
                    else
                    {
                        Type = tr("Cartesian systems");
                    }
                     
                }
                else if (str.contains("ENU"))
                {
                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        Type = tr("ENU坐标系");
                    }
                    else
                    {
                        Type = tr("ENU coordinate system");
                    }

                }
                else
                {

                }*/
               
                    properties << tr("Units: %1").arg(DistanceUnittoString(currentCrs.mapUnits()));
            }
            properties << (currentCrs.isDynamic() ? tr("Dynamic (relies on a datum which is not plate-fixed)") : tr("Static (relies on a datum which is plate-fixed)"));

            /*try
            {
                const QString celestialBody = currentCrs.celestialBodyName();
                if (!celestialBody.isEmpty())
                {
                    properties << tr("Celestial body: %1").arg(celestialBody);
            }
        }
            catch (QgsNotSupportedException&)
            {

            }

            try
            {
                const QgsDatumEnsemble ensemble = currentCrs.datumEnsemble();
                if (ensemble.isValid())
                {
                    QString id;
                    if (!ensemble.code().isEmpty())
                        id = QStringLiteral("<i>%1</i> (%2:%3)").arg(ensemble.name(), ensemble.authority(), ensemble.code());
                    else
                        id = QStringLiteral("<i>%</i>”").arg(ensemble.name());
                    if (ensemble.accuracy() > 0)
                    {
                        properties << tr("Based on %1, which has a limited accuracy of <b>at best %2 meters</b>.").arg(id).arg(ensemble.accuracy());
                    }
                    else
                    {
                        properties << tr("Based on %1, which has a limited accuracy.").arg(id);
                    }
                }
            }
            catch (QgsNotSupportedException&)
            {

            }*/

            const ProjOperation operation = currentCrs.operation();
            properties << tr("Method: %1").arg(operation.description());
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            const QString propertiesString = QStringLiteral("<dt><b>%1</b></dt><dd><ul><li>%2</li></ul></dd>").arg(tr("Properties"),
                properties.join(QLatin1String("</li><li>")));

            const QString extentHtml = QStringLiteral("<dt><b>%1</b></dt><dd>%2</dd>").arg(tr("Extent"), extentString);
            const QString wktString = QStringLiteral("<dt><b>%1</b></dt><dd><code>%2</code></dd>").arg(tr("WKT"), currentCrs.toWkt(ProjCore::CrsWktVariant::Preferred, true).replace('\n', QLatin1String("<br>")).replace(' ', QLatin1String("&nbsp;")));
            const QString proj4String = QStringLiteral("<dt><b>%1</b></dt><dd><code>%2</code></dd>").arg(tr("Proj4"), currentCrs.toProj());

#ifdef Q_OS_WIN
            const int smallerPointSize = std::max(font().pointSize() - 1, 8); // bit less on windows, due to poor rendering of small point sizes
#else
            const int smallerPointSize = std::max(font().pointSize() - 2, 6);
#endif

            const QModelIndex currentIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);


            QString selectedName;
            QString selectedType;

            //std::cout << "before getting name0:" << std::endl;
            if (currentIndex.isValid())
            {
              //  std::cout << "before getting name:" << std::endl;
                selectedName = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();

              //  std::cout << "after getting name:" << selectedName.toStdString() << std::endl;

                QString _selectedName = currentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
                QString _selectedName2 = currentIndex.data(Qt::DisplayRole).toString();

             //   std::cout << "after getting name2:" << _selectedName.toStdString() << std::endl;
             //   std::cout << "after getting name3:" << _selectedName2.toStdString() << std::endl;
            }

            //std::cout << "after getting name1:" << std::endl;

            const QModelIndex parentIndex = lstCoordinateSystems->selectionModel()->model()->parent(currentIndex);
            if (parentIndex.isValid())
            {
            //    std::cout << " parentIndex valid." << std::endl;
                const QModelIndex parentParentIndex = lstCoordinateSystems->selectionModel()->model()->parent(parentIndex);
                if (parentParentIndex.isValid())
                {
                    //selectedType = parentParentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();
                    selectedType = parentParentIndex.data(Qt::DisplayRole).toString();
            //        std::cout << " parentParentIndex valid." << std::endl;
                }
                else
                {
                    selectedType = parentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();
                  //  selectedType = parentIndex.data(static_cast<int>(CoordinateReferenceSystemModel::CustomRole::GroupId)).toString();
                    QString st;
                    selectedType = parentIndex.data(static_cast<int>(Qt::DisplayRole)).toString();

            //        std::cout << " parentParentIndex invalid." << selectedType.toStdString() << " // " << st.toStdString() << std::endl;
                    
         
                }
            }
            else
            {
                //std::cout << " parentIndex invalid." << std::endl;
            }


            lblTypeContent->setText(selectedType);


            leSelection->setText(selectedName);
            //teProjection->setText("hello");
            //teDefinition->setText("ho");
            //teProjection->setText(QStringLiteral("<div style=\"font-size: %1pt\"><h3>%2</h3><dl>").arg(smallerPointSize).arg(selectedName) + propertiesString + wktString + proj4String + extentHtml + QStringLiteral("</dl></div>"));
            teDefinition->setText(QStringLiteral("<div style=\"font-size: %1pt\"><h3>%2</h3><dl>").arg(smallerPointSize).arg(selectedName) + propertiesString + wktString + proj4String + extentHtml + QStringLiteral("</dl></div>"));

            if (!strLatitude.isEmpty())
            {
                //std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << strLatitude.toStdString() << " "
                //    << strLongitude.toStdString() << std::endl;

                leLatitude->setText(strLatitude);
            }
            else
            {
                //std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << strLatitude.toStdString() << " "
                //    << strLongitude.toStdString() << std::endl;

                leLatitude->setText("");
            }

            if (!strLongitude.isEmpty())
            {
                //std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << strLatitude.toStdString() << " "
                //    << strLongitude.toStdString() << std::endl;

                leLongitude->setText(strLongitude);
            }
            else
            {
                //std::cout << "inside/enu " << __FILE__ << " " << __LINE__ << " " << strLatitude.toStdString() << " "
                //    << strLongitude.toStdString() << std::endl;

                leLongitude->setText("");
            }
        }



        void ProjectionSelectionTreeWidget::resetEmptySelection()
        {
            leSelection->setText("");
            lblTypeContent->setText("");
            teDefinition->clear();
            teProjection->clear();
            lstRecent->clearSelection();

            leLongitude->setVisible(false);
            leLatitude->setVisible(false);
            lblUserDefinedEdit->setVisible(true);
            lblTypeContent->setVisible(true);
            teDefinition->setVisible(true);

            strEnuBaseName = "";

            if (AI3D::CORE::BlockObject::isChineseVersion())
            {
                lblType->setText("类型:");
                lblDefinition->setText("定义:");
            }
            else
            {
                lblType->setText("Type:");
                lblDefinition->setText("Definition:");
            }
        }

        // New coordinate system selected from the list
        void ProjectionSelectionTreeWidget::lstCoordinateSystemsSelectionChanged(const QItemSelection& selected, const QItemSelection&)
        {
            static int iCount = 1000;
            //std::cout << (++iCount) << " inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            if (selected.isEmpty())
            {
                //sDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
            //    std::cout << "inside / leLatitude invisble " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                resetEmptySelection();
                return;
            }

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            const QModelIndex selectedProxyIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);
            if (!selectedProxyIndex.isValid())
            {
            //    std::cout << " latitude invisible:" << __LINE__ << std::endl;
                resetEmptySelection();
                return;
            }

            lstCoordinateSystems->scrollTo(selectedProxyIndex);
            const QModelIndex sourceIndex = mCrsModel->mapToSource(selectedProxyIndex);
            selectedIndex = sourceIndex;

            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
            // If the item has children, it's not an end node in the tree, and
            // hence is just a grouping thingy, not an actual CRS.
            if (mCrsModel->coordinateReferenceSystemModel()->rowCount(sourceIndex) == 0)
            {
            //    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                // Found a real CRS
                if (!mBlockSignals)
                {
                    ///emit crsSelected();
                    emit hasValidSelectionChanged(true);
                }

                updateBoundsPreview();
                //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                const QString crsAuthId = mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
                const QString crsName = mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();

                //std::cout << "authId:" << crsAuthId.toStdString() << " name:" << crsName.toStdString() << std::endl;
                
                if (crsName.contains("enu", Qt::CaseInsensitive))
                {
 //                   std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    leLongitude->setVisible(true);
                    leLatitude->setVisible(true);
                    lblUserDefinedEdit->setVisible(false);
                    lblTypeContent->setVisible(false);
                    teDefinition->setVisible(false);

                    strEnuBaseName = crsName;

                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        lblType->setText("纬度:");
                        lblDefinition->setText("经度:");
                    }
                    else
                    {
                        lblType->setText("Latitude:");
                        lblDefinition->setText("Logitude:");
                    }
                }
                else
                {
                    strEnuBaseName = "";
                    //std::cout << "inside /latitude invisble " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    leLongitude->setVisible(false);
                    leLatitude->setVisible(false);
                    lblUserDefinedEdit->setVisible(true);
                    lblTypeContent->setVisible(true);
                    teDefinition->setVisible(true);

                    if (AI3D::CORE::BlockObject::isChineseVersion())
                    {
                        lblType->setText("类型:");
                        lblDefinition->setText("定义:");
                    }
                    else
                    {
                        lblType->setText("Type:");
                        lblDefinition->setText("Definition:");
                    }
                }


                if (!crsAuthId.isEmpty())
                {
                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    const QModelIndexList recentMatches = mRecentCrsModel->match(mRecentCrsModel->index(0, 0),
                        static_cast<int>(RecentCoordinateReferenceSystemsModel::CustomRole::AuthId),
                        crsAuthId);
                    if (!recentMatches.isEmpty())
                    {
                        //sDebugMsgLevel( QStringLiteral( "found srs %1 in recent" ).arg( crsAuthId ), 4 );
                    //    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        lstRecent->selectionModel()->select(recentMatches.at(0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                        lstRecent->scrollTo(recentMatches.at(0));
                    }
                    else
                    {
                        //sDebugMsgLevel( QStringLiteral( "srs %1 not recent" ).arg( crsAuthId ), 4 );
                    //    std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                        lstRecent->clearSelection();
                        lstCoordinateSystems->setFocus(Qt::OtherFocusReason);
                    }
                }
                else
                {
                    //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                    lstRecent->clearSelection();
                    lstCoordinateSystems->setFocus(Qt::OtherFocusReason);
                }
            }
            else
            {
                //std::cout << "inside/latitude invisible:" << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
                // Not a CRS
                resetEmptySelection();
                emit hasValidSelectionChanged(false);
            }
     //       std::cout << iCount << __TIME__ << " inside " << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << std::endl;
        }

        void ProjectionSelectionTreeWidget::lstCoordinateSystemsDoubleClicked(const QModelIndex& index)
        {
            if (!index.isValid())
            {
                //sDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
                return;
            }

            // If the item has children, it's not an end node in the tree, and
            // hence is just a grouping thingy, not an actual CRS.
            if (!mCrsModel->coordinateReferenceSystemModel()->hasChildren(mCrsModel->mapToSource(index)))
                emit projectionDoubleClicked();
        }

        void ProjectionSelectionTreeWidget::lstRecentSelectionChanged(const QItemSelection& selected, const QItemSelection&)
        {
            if (selected.isEmpty())
            {
                //sDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
                return;
            }

            const QModelIndex selectedIndex = lstRecent->selectionModel()->selectedRows(0).value(0);
            if (!selectedIndex.isValid())
                return;

            lstRecent->scrollTo(selectedIndex);

            const QString selectedAuthId = mRecentCrsModel->crs(selectedIndex).authid();
            const QModelIndex sourceIndex = mCrsModel->coordinateReferenceSystemModel()->authIdToIndex(selectedAuthId);
            if (sourceIndex.isValid())
            {
                const QModelIndex proxyIndex = mCrsModel->mapFromSource(sourceIndex);
                if (proxyIndex.isValid())
                {
                    lstCoordinateSystems->selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                    lstCoordinateSystems->scrollTo(proxyIndex);
                }
            }
        }

        void ProjectionSelectionTreeWidget::lstRecentDoubleClicked(const QModelIndex& index)
        {
            //sDebugMsgLevel( QStringLiteral( "Entered." ), 4 );
            if (!index.isValid())
            {
                //sDebugMsgLevel( QStringLiteral( "no current item" ), 4 );
                return;
            }

            emit projectionDoubleClicked();
        }

        void ProjectionSelectionTreeWidget::lstRecentClicked(const QModelIndex& index)
        {
            if (index.column() == ClearColumn)
            {
                removeRecentCrsItem(index);
            }
        }

        void ProjectionSelectionTreeWidget::butOkClicked()
        {
            //std::cout << "ok clicked." << std::endl;
            on_butOk_clicked();


        }

        void ProjectionSelectionTreeWidget::butCancelClicked()
        {
            //std::cout << "cancel clicked." << std::endl;
        }

        void ProjectionSelectionTreeWidget::on_butClose_clicked()
        {
			emit crsRestore();				  
            close();
            //CoordinateReferenceSystem crs = CoordinateReferenceSystem::fromSpecialEpsg("EPSG:4326");
            //CoordinateReferenceSystem crs = CoordinateReferenceSystem::createFromOgcWms("EPSG:4143");
            //static CoordinateReferenceSystem fromENUDefinition(const QString & definition);
            //CoordinateReferenceSystem crs(QString::fromUtf8("EPSG:4143"));
            //CoordinateReferenceSystem crs(QString::fromUtf8("EPSG:4326"));
            //CoordinateReferenceSystem crs = CoordinateReferenceSystem::fromENUDefinition(QString::fromUtf8("EPSG:4143"));
            ///CoordinateReferenceSystem crs(QString::fromUtf8("EPSG:4978"));
            ///setCrs(crs);
        }

        void ProjectionSelectionTreeWidget::on_butOk_clicked()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " clicked." << std::endl;
            const QModelIndex selectedProxyIndex = lstCoordinateSystems->selectionModel()->selectedRows(0).value(0);
            if (!selectedProxyIndex.isValid())
                return;

            if (leLongitude->isVisible())
            {
                if (leLatitude->text().isEmpty() || leLongitude->text().isEmpty())
                    return;
            }

            const QModelIndex sourceIndex = mCrsModel->mapToSource(selectedProxyIndex);
            bool bSelectedCRSValid = false;

            QString strCompoundName;
            if (leLongitude->isVisible() && !leSelection->text().isEmpty())
            {
                strCompoundName = leSelection->text();
            }

            // If the item has children, it's not an end node in the tree, and
            // hence is just a grouping thingy, not an actual CRS.
            if (mCrsModel->coordinateReferenceSystemModel()->rowCount(sourceIndex) == 0)
            {
                const QString crsAuthId = mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::AuthId)).toString();
                const QString crsName = mCrsModel->coordinateReferenceSystemModel()->data(sourceIndex, static_cast<int>(CoordinateReferenceSystemModel::CustomRole::Name)).toString();

                //std::cout << "authId:" << crsAuthId.toStdString() << " name:" << crsName.toStdString() << std::endl;

                if (!strCompoundName.isEmpty())
                {
                    double dLatitude = 0.00;
                    double dLongitude = 0.00;

                    QString recentStr = strCompoundName;

                    srs_s current_srs = AI3D::CORE::CoordinateDescriptor::GetSRSFromName(recentStr.toStdString());
                    if (current_srs.isValid())
                    {
                  //      std::cout << "pstw/valid enu:" << recentStr.toStdString() << " name:" << current_srs.name << " definition:" << current_srs.definition << " type:" << current_srs.type << std::endl;
                    }
                    else
                    {
                  //      std::cout << "pstw/invalid enu:" << recentStr.toStdString() << std::endl;
                    }

                    dLatitude = leLatitude->text().toDouble();
                    dLongitude = leLongitude->text().toDouble();

                    //QString newRecentStr = QString("ENU: %1E,%2N").arg(dLatitude).arg(dLongitude);
                    //QString newRecentStr = QString("ENU: %1,%2").arg(dLatitude).arg(dLongitude);
                    QString newRecentStr = QString("ENU:%1,%2").arg(dLatitude).arg(dLongitude);

                    // push recent: definition(followed by actual enu parameters including latitude and longitude.).
                    // crsSelected: ENU:xxxx (followed by fixed number)
                    //AI3D::GUI::ControlPointsEditorWin::pushRecentCRS(recentStr);
                    if (recentStr.contains("enu", Qt::CaseInsensitive))
                    {
                    //    CrsSettings::RemoveRecentCrsContains("enu");
                    }

                    AI3D::GUI::ControlPointsEditorWin::pushRecentCRS(newRecentStr);

                    bSelectedCRSValid = true;
                    emit crsSelected(recentStr);
                }
                else if (!crsAuthId.isEmpty())
                {
                    /*
                    AI3D::PROJ::CoordinateReferenceSystem newcrs;
                    newcrs.createFromString(crsName + "(" + crsAuthId + ")");
                    //AI3D::PROJ::coordinateReferenceSystemRegistry()->InsertRecent(new_crs);
                    //QProj::coordinateReferenceSystemRegistry()->InsertRecent(newcrs);
                    AI3D::PROJ::QProj::coordinateReferenceSystemRegistry()->InsertRecent(newcrs);
                    */
                    ///QString recentStr = crsName + "(" + crsAuthId + ")";
                    QString recentStr = crsAuthId;

                    if (recentStr.contains("enu", Qt::CaseInsensitive))
                    {
                    //    CrsSettings::RemoveRecentCrsContains("enu");
                    }

                    AI3D::GUI::ControlPointsEditorWin::pushRecentCRS(recentStr);

                   /* emit crsSelected();*/
                    bSelectedCRSValid = true;
                    emit crsSelected(recentStr);
                }
            }

            if (!bSelectedCRSValid)
            {
                emit crsRestore();
            }

          close();
        }

        void ProjectionSelectionTreeWidget::on_butCancel_clicked()
        {
            //std::cout << "inside " << __FILE__ << " " << __FUNCTION__ << " clicked." << std::endl;
            emit crsRestore();
            close();
        }

        void ProjectionSelectionTreeWidget::pushProjectionToFront()
        {
        }

        void ProjectionSelectionTreeWidget::clearRecentCrs()
        {
            // If the list is empty, there is nothing to do
            if (QProj::coordinateReferenceSystemRegistry()->recentCrs().isEmpty())
            {
                return;
            }

            // Ask for confirmation
            if (QMessageBox::question(this, tr("Clear Recent CRS"),
                tr("Are you sure you want to clear the list of recently used coordinate reference system?"),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            {
                return;
            }
            QProj::coordinateReferenceSystemRegistry()->clearRecent();
        }

        void ProjectionSelectionTreeWidget::removeRecentCrsItem(const QModelIndex& index)
        {
            const CoordinateReferenceSystem selectedRecentCrs = mRecentCrsModel->crs(index);
            QProj::coordinateReferenceSystemRegistry()->removeRecent(selectedRecentCrs);
        }


        ///@cond PRIVATE
        RecentCoordinateReferenceSystemTableModel::RecentCoordinateReferenceSystemTableModel(QObject* parent)
            : RecentCoordinateReferenceSystemsProxyModel(parent, 3)
        {
#ifdef ENABLE_MODELTEST
            new ModelTest(this, this);
#endif
        }

        QVariant RecentCoordinateReferenceSystemTableModel::headerData(int section, Qt::Orientation orientation, int role) const
        {
            if (orientation == Qt::Horizontal)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0:
                        ///return tr("Coordinate Reference System");
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            return tr("空间参考系统");
                        }
                        else
                        {
                            return tr("Spatial Reference System");
                        }
                        
                    case 1:
                        ///return tr("Authority ID");
                        if (AI3D::CORE::BlockObject::isChineseVersion())
                        {
                            return tr("描述");
                        }
                        else
                        {
                            return tr("Definition");
                        }
                        
                    case 2:
                        return QString();
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

        QVariant RecentCoordinateReferenceSystemTableModel::data(const QModelIndex& index, int role) const
        {
            if (!index.isValid())
                return QVariant();

            const int column = index.column();
            switch (column)
            {
            case 1:
            {
                const CoordinateReferenceSystem lCrs = crs(index);
                switch (role)
                {
                case Qt::DisplayRole:
                case Qt::ToolTipRole:
                    return lCrs.authid();

                default:
                    break;
                }
                break;
            }

            case 2:
            {
                switch (role)
                {
                case Qt::ToolTipRole:
                    return tr("Remove from recently used CRS");

                default:
                    break;
                }
                break;
            }

            default:
                break;
            }
            return RecentCoordinateReferenceSystemsProxyModel::data(index, role);
        }


        //
        // RemoveRecentCrsDelegate
        //

        RemoveRecentCrsDelegate::RemoveRecentCrsDelegate(QObject* parent)
            : QStyledItemDelegate(parent)
        {

        }

        bool RemoveRecentCrsDelegate::eventFilter(QObject* obj, QEvent* event)
        {
            if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove)
            {
                QHoverEvent* hoverEvent = static_cast<QHoverEvent*>(event);
                if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(obj->parent()))
                {
                    const QModelIndex indexUnderMouse = view->indexAt(hoverEvent->pos());
                    setHoveredIndex(indexUnderMouse);
                    view->viewport()->update();
                }
            } 
            else if (event->type() == QEvent::HoverLeave)
            {
                setHoveredIndex(QModelIndex());
                qobject_cast<QWidget*>(obj)->update();
            }

            return QStyledItemDelegate::eventFilter(obj, event);
        }

        void RemoveRecentCrsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            QStyledItemDelegate::paint(painter, option, index);

            if (index == mHoveredIndex)
            {
                QStyleOptionButton buttonOption;
                buttonOption.initFrom(option.widget);
                buttonOption.rect = option.rect;

                option.widget->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);
            }

            const QIcon icon = QIcon(":/status/skin/cancle.png");
            const QRect iconRect(option.rect.left() + (option.rect.width() - 16) / 2,
                option.rect.top() + (option.rect.height() - 16) / 2,
                16, 16);

            icon.paint(painter, iconRect);
        }

        void RemoveRecentCrsDelegate::setHoveredIndex(const QModelIndex& index)
        {
            mHoveredIndex = index;
        }

        ///@endcond PRIVATE
    }
}