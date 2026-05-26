
#include <QHBoxLayout>

#include "qgsprojectionselectionwidget.h"
#include <iostream>
#include "qgsprojectionselectiondialog.h"

#include "qgssettings.h"

#include "qgscoordinatetransform.h"
#include "Core/CoordinateSystem.h"
void QgsProjectionSelectionWidget::setDefaultSelectItem(QString strItemName)
{
    QAbstractItemModel* model = m_pCombox->view()->model();
    QModelIndexList Items = model->match(model->index(0, 0), Qt::DisplayRole, QVariant::fromValue(strItemName), 2, Qt::MatchRecursive);
    //这种做法解决combox中存在TreeView视图时刷洗设置二级以下节点为当前默认显示项Index无法显示正确信息的问题。
    //如果需要将 子树下的第二 第三级等的子树设置为默认选项，需要这样子设置
    if (Items.size() > 0)
    {
        for (QModelIndex m_oRightIndex : Items)
        {
            m_pCombox->setRootModelIndex(m_oRightIndex.parent());
            m_pCombox->setModelColumn(m_oRightIndex.column());
            m_pCombox->setCurrentIndex(m_oRightIndex.row());
            m_pCombox->setRootModelIndex(QModelIndex());
            m_pCombox->view()->setCurrentIndex(m_oRightIndex);
        }
    }
}
void QgsProjectionSelectionWidget::refreshModelData(TreeViewDataInfoList DataList)
{
    if (m_pCombox->m_pTreeModel == nullptr || m_pCombox->m_pTreeView == nullptr)
        return;
    m_pCombox->m_pTreeModel->updataModelTree(DataList);  //根据新的数据更新combox下的下拉树
    m_pCombox->m_pTreeView->expandAll(); //全部展开
    //setDefaultSelectItem();  //设置默认选中项
}

QgsProjectionSelectionWidget::QgsProjectionSelectionWidget(QWidget* parent)
    : QWidget(parent)
{
    //setWindowFlags(Qt::CustomizeWindowHint);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    /*layout->setSpacing(6);*/
    setLayout(layout);
   
    
    m_pCombox = new QTreeListComboBox(this);
    
    layout->addWidget(m_pCombox);
   
   //添加设置的crs
   // addProjectCrsOption();

    QgsSettings settings;
    mDefaultCrs = QgsCoordinateReferenceSystem(settings.value(QStringLiteral("/projections/defaultProjectCrs"), QLatin1String("EPSG:4326"), QgsSettings::App).toString());
    if (mDefaultCrs.authid() != mProjectCrs.authid())
    {
        addDefaultCrsOption();
    }
    
    addRecentCrs();
    TreeViewDataInfo* morecrs = new TreeViewDataInfo(crsoption_s::CRS_MoreCrs);
    morecrs->m_TreeItmeName = "More";
    datalist.append(morecrs);
    TreeViewDataInfo* crsdatabasenode = new TreeViewDataInfo(crsoption_s::CRS_MoreCrs,morecrs);
    crsdatabasenode->m_TreeItmeName = QLatin1String("Spatial Reference System Database");// mDefaultCrs.userFriendlyIdentifier();
    datalist.append(crsdatabasenode);
    
     //
    refreshModelData(datalist);
    //setDefaultSelectItem(crsOptionText(mProjectCrs));
    connect(m_pCombox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QgsProjectionSelectionWidget::comboIndexChanged);
   
}

QgsCoordinateReferenceSystem QgsProjectionSelectionWidget::crs() const
{
    QModelIndex index1 = m_pCombox->m_pTreeView->currentIndex();
    TreeViewItem* item1 = m_pCombox->m_pTreeModel->itemFromIndex(index1);
    
  switch (item1->DataInfo()->crs_opt_/*static_cast< CrsOption >( mCrsComboBox->currentData().toInt() ) */)
  {
  case crsoption_s::CRS_CurrentCrs:
  {
      
      return mCrs;
  }
  case crsoption_s::CRS_DefaultCrs:
  {
     
      return mDefaultCrs;
  }
  case crsoption_s::CRS_RecentCrs:
  {
      
      long srsid = item1->DataInfo()->m_TreeItmeName.toInt();// mCrsComboBox->currentData(Qt::UserRole + 1).toLongLong();
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId(srsid);
      return crs;
  }
   
  }
  return mCrs;
}



void QgsProjectionSelectionWidget::setOptionVisible(const QgsCoordinateReferenceSystem option, const bool visible)
{
   
           addCurrentCrsOption();
    
}


void QgsProjectionSelectionWidget::setOptionVisible( const QgsProjectionSelectionWidget::CrsOption option, const bool visible )
{
  int optionIndex = mCrsComboBox->findData( option );

  if ( visible && optionIndex < 0 )
  {
    //add missing CRS option
    switch ( option )
    {
     /* case QgsProjectionSelectionWidget::LayerCrs:
      {
        setLayerCrs( mLayerCrs );
        return;
      }*/
      case QgsProjectionSelectionWidget::ProjectCrs:
      {
        addProjectCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::DefaultCrs:
      {
        addDefaultCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::CurrentCrs:
      {
        addCurrentCrsOption();
        return;
      }
      case QgsProjectionSelectionWidget::RecentCrs:
        //recently used CRS option cannot be readded
        return;
      case QgsProjectionSelectionWidget::CrsNotSet:
      {
        addNotSetOption();

        if ( optionVisible( CurrentCrs ) && !mCrs.isValid() )
        {
          // hide invalid option if not set option is shown
          setOptionVisible( CurrentCrs, false );
        }

        return;
      }
    }
  }
  else if ( !visible && optionIndex >= 0 )
  {
    //remove CRS option
    mCrsComboBox->removeItem( optionIndex );

    if ( option == CrsNotSet )
    {
      setOptionVisible( CurrentCrs, true );
    }
  }
}

void QgsProjectionSelectionWidget::setNotSetText( const QString &text )
{
  mNotSetText = text;
  int optionIndex = mCrsComboBox->findData( CrsNotSet );
  if ( optionIndex >= 0 )
  {
    mCrsComboBox->setItemText( optionIndex, mNotSetText );
  }
}

void QgsProjectionSelectionWidget::setMessage( const QString &text )
{
  mMessage = text;
}

bool QgsProjectionSelectionWidget::optionVisible( QgsProjectionSelectionWidget::CrsOption option ) const
{

  int optionIndex = mCrsComboBox->findData( option );
  return optionIndex >= 0;
}

void QgsProjectionSelectionWidget::selectCrs()
{
  //find out crs id of current proj4 string
  QgsProjectionSelectionDialog dlg( this );
  if ( !mMessage.isEmpty() )
    dlg.setMessage( mMessage );
  dlg.setCrs( mCrs );

  if ( !mNotSetText.isEmpty() )
    dlg.setNotSetText( mNotSetText );

  //if ( optionVisible( QgsProjectionSelectionWidget::CrsOption::CrsNotSet ) )
  //{
  //  dlg.setShowNoProjection( true );
  //}

  if ( dlg.exec() )
  {
      m_pCombox->blockSignals( true );
   
   
    QgsCoordinateReferenceSystem crs = dlg.crs();// "WGS 84 - World Geodetic System 1984 (EPSG:4326) + EGM96 geoid height (EPSG:5773)";// 
    QString strItemName =  crsOptionText(crs);// .userFriendlyIdentifier();
    std::cout << strItemName.toStdString()<< " || "<< crs.description().toStdString()<< "+++++ "<< __FUNCTION__ << " "<< __LINE__ << crs.authid().toStdString() << std::endl;
    
    {
        std::cout << "****" << crs.authid().toStdString() << std::endl;
        //QgsCoordinateReferenceSystem crsnew = QgsCoordinateReferenceSystem("epsg:4326+5773");
        QgsCoordinateTransform transform;
        transform.setSourceCrs(crs);
        QgsCoordinateReferenceSystem defcrs = QgsCoordinateReferenceSystem("epsg:4326");
        transform.setDestinationCrs(defcrs);
        double x = 116.283774;
        double y = 39.808981;
        double z = -66.339502;
        
        AI3D::CORE::CoordinateTransformer::TransformByEpsgCode(1, &x, &y, &z, crs.authid().toStdString(), "epsg:4326");
        //transform.transformCoords(1, &x, &y, &z);
        std::cout << x << " " << y << " " << z << std::endl;
    }
    
    /*setDefaultSelectItem(strItemName);*/
    QModelIndexList Items = m_pCombox->m_pTreeModel->match(m_pCombox->m_pTreeModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(strItemName), 2, Qt::MatchRecursive);
    if (Items.size() > 0)
    {
        for (QModelIndex m_oRightIndex : Items)
        {
            QModelIndex index = m_oRightIndex.parent();
            int col = m_oRightIndex.column();
            int row = m_oRightIndex.row();
            std::cout << strItemName.toStdString() << " " << col<< " " << row << std::endl;
            m_pCombox->m_pTreeView->setCurrentIndex(m_oRightIndex);
           
        }
    }
    else//构建info
    {
        
        QString strItemName = crsOptionText(mCrs);// .userFriendlyIdentifier();
        std::cout << strItemName.toStdString() << std::endl;
        setCrs(crs);
    }
    m_pCombox->blockSignals(false);
    QgsCoordinateReferenceSystem::pushRecentCoordinateReferenceSystem(mCrs);
    //if (mCrs != crs)
    //{
    //    mCrs = crs;
    //   // emit crsChanged(crs);
    //}
   /* emit crsChanged( crs );*/
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }
}




void QgsProjectionSelectionWidget::dragEnterEvent( QDragEnterEvent *event )
{
  //if ( !( event->possibleActions() & Qt::CopyAction ) )
  //{
  //  event->ignore();
  //  return;
  //}

  //if ( mapLayerFromMimeData( event->mimeData() ) )
  //{
  //  // dragged an acceptable layer, phew
  //  event->setDropAction( Qt::CopyAction );
  //  event->accept();
  //  mCrsComboBox->setHighlighted( true );
  //  update();
  //}
  //else
  //{
  //  event->ignore();
  //}
}

void QgsProjectionSelectionWidget::dragLeaveEvent( QDragLeaveEvent *event )
{
  if ( mCrsComboBox->isHighlighted() )
  {
    event->accept();
    mCrsComboBox->setHighlighted( false );
    update();
  }
  else
  {
    event->ignore();
  }
}

void QgsProjectionSelectionWidget::dropEvent( QDropEvent *event )
{
  //if ( !( event->possibleActions() & Qt::CopyAction ) )
  //{
  //  event->ignore();
  //  return;
  //}

  //if ( QgsMapLayer *layer = mapLayerFromMimeData( event->mimeData() ) )
  //{
  //  // dropped a map layer
  //  setFocus( Qt::MouseFocusReason );
  //  event->setDropAction( Qt::CopyAction );
  //  event->accept();

  //  if ( layer->crs().isValid() )
  //    setCrs( layer->crs() );
  //}
  //else
  //{
  //  event->ignore();
  //}
  //mCrsComboBox->setHighlighted( false );
  //update();
}

void QgsProjectionSelectionWidget::addNotSetOption()
{
 /* mCrsComboBox->insertItem( 0, mNotSetText, QgsProjectionSelectionWidget::CrsNotSet );
  if ( !mCrs.isValid() )
    whileBlocking( mCrsComboBox )->setCurrentIndex( 0 );*/
}

void QgsProjectionSelectionWidget::comboIndexChanged( int idx )
{
   int id = m_pCombox->currentIndex();
   
    
    QModelIndex index1 = m_pCombox->m_pTreeView->currentIndex();
    TreeViewItem* item1 = m_pCombox->m_pTreeModel->itemFromIndex(index1);
    setDefaultSelectItem(item1->DataInfo()->m_TreeItmeName);
  switch (item1->DataInfo()->crs_opt_/*static_cast< CrsOption >( mCrsComboBox->itemData( idx ).toInt() */ )
  {
    
  case crsoption_s::CRS_CurrentCrs/*QgsProjectionSelectionWidget::CurrentCrs*/:
      emit crsChanged( mCrs );
      break;
  case crsoption_s::CRS_DefaultCrs:/*QgsProjectionSelectionWidget::DefaultCrs*/
      emit crsChanged( mDefaultCrs );
      break;
  case crsoption_s::CRS_RecentCrs/*QgsProjectionSelectionWidget::RecentCrs*/:
    {
        long srsid = item1->DataInfo()->m_TreeItmeName.toInt(); ;// long srsid = mCrsComboBox->itemData(idx, Qt::UserRole + 1).toLongLong();
      QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromSrsId( srsid );
      emit crsChanged( crs );
      break;
    }
  case crsoption_s::CRS_MoreCrs/*QgsProjectionSelectionWidget::RecentCrs*/:
  {
      selectCrs();
     
      break;
  }
  
  }
  
}

bool QgsProjectionSelectionWidget::optionVisible(const QgsCoordinateReferenceSystem& crs) const
{
    QString strItemName = crsOptionText(crs);
    QModelIndexList Items = m_pCombox->m_pTreeModel->match(m_pCombox->m_pTreeModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(strItemName), 2, Qt::MatchRecursive);
    return Items.size() > 0;
}

void QgsProjectionSelectionWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
  {
     
      if (!optionVisible(crs))
      {
          if (mCrs != crs)
          {
              mCrs = crs;
              QString strItemName = crsOptionText(mCrs);// .userFriendlyIdentifier();
              std::cout << strItemName.toStdString() << std::endl;
              //emit crsChanged( crs );
          }

          setOptionVisible(crs,true);
          //加入到recent里边
          /*addRecentCrs();*/
          setDefaultSelectItem(crsOptionText(crs));
      }
 /*   if ( !optionVisible( QgsProjectionSelectionWidget::CurrentCrs ) )
      setOptionVisible( QgsProjectionSelectionWidget::CurrentCrs, true );
    mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                               crsOptionText( crs ) );
    mCrsComboBox->blockSignals( true );
    mCrsComboBox->setCurrentIndex( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ) );
    mCrsComboBox->blockSignals( false );*/
  }
 /* else
  {
    int crsNotSetIndex = mCrsComboBox->findData( QgsProjectionSelectionWidget::CrsNotSet );
    if ( crsNotSetIndex >= 0 )
    {
      mCrsComboBox->blockSignals( true );
      mCrsComboBox->setCurrentIndex( crsNotSetIndex );
      mCrsComboBox->blockSignals( false );
    }
    else
    {
      mCrsComboBox->setItemText( mCrsComboBox->findData( QgsProjectionSelectionWidget::CurrentCrs ),
                                 crsOptionText( crs ) );
    }
  }*/
 
  updateTooltip();
}


void QgsProjectionSelectionWidget::addProjectCrsOption()
{
  if ( mProjectCrs.isValid() )
  {

    //mCrsComboBox->addItem( tr( "Project CRS: %1" ).arg( mProjectCrs.userFriendlyIdentifier() ), QgsProjectionSelectionWidget::ProjectCrs );
  }
}

void QgsProjectionSelectionWidget::addDefaultCrsOption()
{
   
    TreeViewDataInfo* defaultcrs = new TreeViewDataInfo(crsoption_s::CRS_DefaultCrs);
    defaultcrs->m_TreeItmeName = "Default Crs";
    
    datalist.append(defaultcrs);
    TreeViewDataInfo* crsdatabasenode = new TreeViewDataInfo( crsoption_s::CRS_DefaultCrs, defaultcrs);
    crsdatabasenode->m_TreeItmeName = mDefaultCrs.userFriendlyIdentifier();
    datalist.append(crsdatabasenode);
   
  //mCrsComboBox->addItem( tr( "Default CRS: %1" ).arg( mDefaultCrs.userFriendlyIdentifier() ), QgsProjectionSelectionWidget::DefaultCrs );
}

void QgsProjectionSelectionWidget::addCurrentCrsOption()
{
    //招到recent节点，
    int rowCount = m_pCombox->m_pTreeModel->rowCount();
    int colCount = m_pCombox->m_pTreeModel->columnCount();
    for (int i = 0; i < rowCount; i++)
    {
            QModelIndex modelIndex = m_pCombox->m_pTreeModel->index(i, 0);
           
            TreeViewItem* item1 = m_pCombox->m_pTreeModel->itemFromIndex(modelIndex);

            //std::cout << item1->DataInfo()->m_TreeItmeName.toStdString() << std::endl;
            TreeViewDataInfo* parent = item1->DataInfo();
            if (item1->DataInfo()->crs_opt_ == crsoption_s::CRS_RecentCrs)
            {
                TreeViewDataInfo* currentcrs = new TreeViewDataInfo(crsoption_s::CRS_RecentCrs,item1->DataInfo());
                if (!crsOptionText(mCrs).isEmpty())
                {
                    currentcrs->m_TreeItmeName = crsOptionText(mCrs);
                    datalist.append(currentcrs);
                    refreshModelData(datalist);
                    //setDefaultSelectItem(crsOptionText(mCrs));
                    return;
                }
            }
            // if(item1->data(0).toString().toStdString())
        
    }
    

}

QString QgsProjectionSelectionWidget::crsOptionText( const QgsCoordinateReferenceSystem &crs )
{
  if ( crs.isValid() )
    return crs.userFriendlyIdentifier();
  else
    return tr( "invalid projection" );
}



void QgsProjectionSelectionWidget::addRecentCrs()
{
  const QList< QgsCoordinateReferenceSystem> recentProjections = QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems();
  TreeViewDataInfo* recentcrs = new TreeViewDataInfo(crsoption_s::CRS_RecentCrs);
  recentcrs->m_TreeItmeName = "Recent Crs";
  datalist.append(recentcrs);
  
  for ( const QgsCoordinateReferenceSystem &crs : recentProjections )
  {
    long srsid = crs.srsid();

    //check if already shown
    if ( crsIsShown( srsid ) )
    {
      continue;
    }

    if ( crs.isValid() )
    {
        TreeViewDataInfo* crsdatabasenode = new TreeViewDataInfo(crsoption_s::CRS_RecentCrs, recentcrs);
        crsdatabasenode->m_TreeItmeName = crs.userFriendlyIdentifier();
        datalist.append(crsdatabasenode);
     /* mCrsComboBox->addItem( crs.userFriendlyIdentifier(), QgsProjectionSelectionWidget::RecentCrs );
      mCrsComboBox->setItemData( mCrsComboBox->count() - 1, QVariant( ( long long )srsid ), Qt::UserRole + 1 );*/
    }
  }
}

bool QgsProjectionSelectionWidget::crsIsShown( const long srsid ) const
{
  return srsid == mLayerCrs.srsid() || srsid == mDefaultCrs.srsid() || srsid == mProjectCrs.srsid();
}

int QgsProjectionSelectionWidget::firstRecentCrsIndex() const
{
  for ( int i = 0; i < mCrsComboBox->count(); ++i )
  {
    if ( static_cast< CrsOption >( mCrsComboBox->itemData( i ).toInt() ) == RecentCrs )
    {
      return i;
    }
  }
  return -1;
}

void QgsProjectionSelectionWidget::updateTooltip()
{
  QgsCoordinateReferenceSystem c = crs();
  if ( c.isValid() )
    setToolTip( c.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
  else
    setToolTip( QString() );
}

//QgsMapLayer *QgsProjectionSelectionWidget::mapLayerFromMimeData( const QMimeData *data ) const
//{
//  const QgsMimeDataUtils::UriList uriList = QgsMimeDataUtils::decodeUriList( data );
//  for ( const QgsMimeDataUtils::Uri &u : uriList )
//  {
//    // is this uri from the current project?
//    if ( QgsMapLayer *layer = u.mapLayer() )
//    {
//      return layer;
//    }
//  }
//  return nullptr;
//}
