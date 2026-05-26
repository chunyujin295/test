

#include "Gui/QComboxTree.h"
#include <assert.h>
#include <xutility>
#include <QHBoxLayout>
#include <qtreeview.h>
#include <iostream>
#include <QPainter>
//（1）每一个节点的数据结构形式实现

//namespace AI3D
//{
//    namespace GUI
//    {

        TreeViewDataInfo::TreeViewDataInfo(crsoption_s opt, TreeViewDataInfo* pParent /*= NULL*/) :m_pParentInfo(pParent)
        {
            m_TreeItmeName = "";
            m_ChildList.clear();
            m_childCount = -1;
            if (pParent != NULL)
            {
                m_pParentInfo->m_ChildList.append(this);
            }
            crs_opt_ = opt;
        }

        void QTreeComboBoxView::mouseMoveEvent(QMouseEvent* event)
        {
            //if(event->buttons() == Qt::NoButton())
            auto curIndex = currentIndex();
            auto rect = this->visualRect(curIndex);
            auto buttonRect = QRect(rect.left() - 20, rect.top(), 20, rect.height());

            if (buttonRect.contains(event->pos()))
            {
                /* if (isExpanded(curIndex))
                     setExpanded(curIndex, false);
                 else */
                setExpanded(curIndex, true);
                emit treeMousePressed(true);
            }
            else
                emit treeMousePressed(false);
        }

        void QTreeComboBoxView::mousePressEvent(QMouseEvent* event)
        {

            /*auto curIndex = currentIndex();
            auto rect = this->visualRect(curIndex);
            auto buttonRect = QRect(rect.left() - 20, rect.top(), 20, rect.height());*/
            //auto flag = flags(curIndex);
            //if (buttonRect.contains(event->pos()))
            //{
            //   /* if (isExpanded(curIndex)) 
            //        setExpanded(curIndex, false);
            //    else */
            //        //setExpanded(curIndex, true);
            //    //emit treeMousePressed(true);
            //}
           /* else
                emit treeMousePressed(false);*/
        }
        //https://blog.csdn.net/naibozhuan3744/article/details/80914480
#define TREEVIEWSTYLE QString::fromUtf8( \
"QTreeView{\n"\
"  border:none;\n"\
"background: #505050;\n"\
"}\n"\
"QTreeView::item{\n"\
"    height: 25px;\n"\
"    border: none;\n"\
"    color: white;\n"\
"    background: transparent;\n"\
"}\n"\
"QTreeView::item:hover{\n"\
"  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0#e7effd, stop: 1 #cbdaf1);\n"\
"border:1px solid #bfcde4;\n"\
"}\n"\
"QTreeView::item : selected{\n"\
"color:green;\n"\
"    background: #1E90FF;\n"\
"}\n"\
"QTreeView::item:selected:active{\n"\
"color:red;\n"\
"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : color:red, stop:  color:red);\n"\
"}\n"\
"QTreeView::item:selected:!active{\n"\
"color:blue;\n"\
"background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : color:blue, stop: color:blue);\n"\
"}\n"\
)
//"QTreeView::branch{\n"\
//"    background: transparent;\n"\
//"}\n"\
//"QTreeView::branch:hover{\n"\
//"    background: transparent;\n"\
//"}\n"\
//"QTreeView::branch : selected{\n"\
//"    background: #1E90FF;\n"\
//"}\n"\
//"QTreeView::branch : closed : has - children{\n"\
//"    image: url(: / image / treeclose.png);\n"\
//"}\n"\
//"QTreeView::branch : open : has - children{\n"\
//"    image: url(: / image / treeopen.png);\n"\
//"}\n"\

//设置某些样式 下段代码来自于 https://blog.csdn.net/dpsying/article/details/80354976



        TreeDelegate::TreeDelegate(QObject* parent)
            : QStyledItemDelegate(parent)
        {

        }

        TreeDelegate::~TreeDelegate()
        {

        }

        void TreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            QStyleOptionViewItem itemOption(option);

            bool bSelected = false;
            if (itemOption.state & QStyle::State_HasFocus)
            {
                itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
            }
            if (itemOption.state & QStyle::State_MouseOver)
            {
                itemOption.state = itemOption.state ^ QStyle::State_MouseOver;
            }
            if (itemOption.state & QStyle::State_Selected)
            {
                bSelected = true;
                itemOption.state = itemOption.state ^ QStyle::State_Selected;
            }
            QStyledItemDelegate::paint(painter, itemOption, index);

            //画选中的行的框
            if (bSelected)
            {
                QRect rc = option.rect;
                painter->fillRect(option.rect, QColor(0, 0, 255, 125));

            }

        }


        QTreeComboBoxView::QTreeComboBoxView(QWidget* parent) :QTreeView(parent)
        {
            setRootIsDecorated(false);
            setStyleSheet("QTreeView::item:hover{background-color:rgb(150,150,155);}");
            pItemDelegate = new TreeDelegate(this);
            setItemDelegate(pItemDelegate);

        }
        TreeViewDataInfo::~TreeViewDataInfo()
        {
            if (!m_ChildList.isEmpty())
            {
                TreeViewDataInfoList::iterator b = m_ChildList.begin();
                TreeViewDataInfoList::iterator e = m_ChildList.end();
                for (; b != e; b++)
                {
                    TreeViewDataInfo* pInfo = *b;
                    if (NULL != pInfo)
                    {
                        delete pInfo;
                        pInfo = NULL;
                    }
                }
                m_ChildList.clear();
            }
        }

        int TreeViewDataInfo::childCount() const
        {
            return m_ChildList.size();
        }

        int TreeViewDataInfo::childExists(const TreeViewDataInfo* pDataInf) const
        {
            TreeViewDataInfoList::const_iterator pos = std::find(m_ChildList.begin(), m_ChildList.end(), pDataInf);
            if (pos != m_ChildList.end())
            {
                return pos - m_ChildList.begin();
            }

            return -1;
        }

        TreeViewDataInfo* TreeViewDataInfo::parentInfo() const
        {
            return m_pParentInfo;
        }

        //（2）数据层管理层实现

        TreeViewItem::TreeViewItem(TreeViewDataInfo* pMapInfo, TreeViewItem* pParent /*= NULL*/)
            :m_pDataInfo(pMapInfo), m_pParent(pParent)
        {

        }

        TreeViewItem::~TreeViewItem()
        {

        }

        TreeViewItem* TreeViewItem::parent() const
        {
            return m_pParent;
        }

        TreeViewItem* TreeViewItem::child(int nIndex) const
        {
            if (nIndex < 0 && nIndex >= childCount())
            {
                return NULL;
            }

            return m_childItems.at(nIndex);
        }

        void TreeViewItem::appendChild(TreeViewItem* pItem)
        {
            assert(NULL != pItem && pItem->m_pParent == this);
            if (NULL != pItem)
            {
                m_childItems.append(pItem);
            }
        }

        int TreeViewItem::childCount() const
        {
            return m_childItems.size();
        }

        int TreeViewItem::childIndex(const TreeViewItem* pItem) const
        {
            assert(NULL != pItem);
            if (NULL == pItem || m_childItems.isEmpty())
            {
                return -1;
            }

            QList<TreeViewItem*>::const_iterator pos = std::find(m_childItems.begin(), m_childItems.end(), pItem);
            if (pos != m_childItems.end())
            {
                return pos - m_childItems.begin();
            }
            else
            {
                return -1;
            }
        }

        bool TreeViewItem::removeChild(int nIndex)
        {
            if (nIndex < 0 || nIndex >= childCount())
            {
                return false;
            }

            TreeViewItem* pItem = child(nIndex);
            if (NULL == pItem)
            {
                return false;
            }

            if (NULL != pItem->m_pDataInfo)
            {
                if (NULL != m_pDataInfo)
                {
                    int nPos = m_pDataInfo->childExists(pItem->m_pDataInfo);
                    if (nPos >= 0 && nPos < m_pDataInfo->childCount())
                    {
                        m_pDataInfo->m_ChildList.removeAt(nPos);
                    }
                }

                delete pItem->m_pDataInfo;
                pItem->m_pDataInfo = NULL;
            }

            delete pItem;
            pItem = NULL;

            m_childItems.removeAt(nIndex);
            return true;
        }

        QVariant TreeViewItem::data(int nCol, int role /*= Qt::DisplayRole*/)
        {
            assert(NULL != m_pDataInfo);
            assert(0 <= nCol && nCol <= 3);

            if (NULL == m_pDataInfo)
            {
                return QVariant();
            }
            else if (Qt::TextAlignmentRole == role)
            {
                return int(Qt::AlignLeft);
            }
            else if (Qt::DisplayRole == role)
            {
                switch (nCol)
                {
                case 0: // get show name
                    return m_pDataInfo->m_TreeItmeName;
                    break;
                }
            }
            return QVariant();
        }

        bool TreeViewItem::setData(int nCol, QVariant val, int role /*= Qt::EditRole*/)
        {

            return true;
        }

        int TreeViewItem::row() const
        {
            if (NULL == m_pParent)
            {
                return 0;
            }
            else
            {
                return m_pParent->m_childItems.indexOf(const_cast<TreeViewItem*>(this));
            }
        }

        Qt::ItemFlags TreeViewItem::flags(int nCol) const
        {
            if (NULL == m_pDataInfo)
            {
                return ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);// :    Qt::ItemIsSelectable | Qt::ItemIsEnabled);/*Qt::NoItemFlags*/;
            }
            if (1 == nCol)
            {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            }
            else
            {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            }

        }

        TreeViewDataInfo* TreeViewItem::DataInfo() const
        {
            return m_pDataInfo;
        }

        bool TreeViewItem::swapChild(int nFirst, int nSecond)
        {
            assert(0 <= nFirst && nFirst < childCount());
            assert(0 <= nSecond && nSecond < childCount());

            if (nFirst < 0 || nFirst >= childCount()
                || nSecond < 0 || nSecond >= childCount())
            {
                return false;
            }

            m_childItems.swap(nFirst, nSecond);

            if (NULL != m_pDataInfo
                && m_pDataInfo->m_ChildList.size() > nFirst
                && m_pDataInfo->m_ChildList.size() > nSecond)
            {
                m_pDataInfo->m_ChildList.swap(nFirst, nSecond);
            }

            return true;
        }

        //（3）数据的model实现

        ComboxTreeModel::ComboxTreeModel(QObject* parent /*= NULL*/)
        {
            m_pForRoot = new TreeViewDataInfo();
            m_pRootItem = new TreeViewItem(m_pForRoot);
        }

        ComboxTreeModel::~ComboxTreeModel()
        {

        }
#include <QStandardItem>
        void ComboxTreeModel::updataModelTree(TreeViewDataInfoList MapList)
        {
            beginResetModel();
            m_mapInfos = MapList;
            if (m_mapInfos.isEmpty())
            {
                return;
            }
            if (NULL != m_pForRoot)
            {
                delete m_pForRoot;
                m_pForRoot = NULL;
            }
            if (NULL != m_pRootItem)
            {
                delete m_pRootItem;
                m_pRootItem = NULL;
            }
            bool bHasFind = false;
            m_pForRoot = new TreeViewDataInfo();
            m_pRootItem = new TreeViewItem(m_pForRoot);
            assert(NULL != m_pRootItem);
            //更新树里的数据，重新创建树的Itme项
            for (int i = 0; i < m_mapInfos.size(); i++)
            {
                TreeViewDataInfo* pMap = m_mapInfos.at(i);
                assert(NULL != pMap);
                if (NULL != pMap->m_pParentInfo)
                {
                    continue;
                }
                //std::cout << pMap->m_TreeItmeName.toStdString() <<" " <<pMap->childCount() << std::endl;
                appendItem(pMap, m_pRootItem);

            }
            endResetModel();
        }

        TreeViewItem* ComboxTreeModel::itemFromIndex(QModelIndex index) const
        {
            if (!index.isValid())
            {
                return m_pRootItem;
            }
            else
            {
                return static_cast<TreeViewItem*>(index.internalPointer());
            }
        }

        int ComboxTreeModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
        {
            if (parent.column() > 0)
            {
                return 0;
            }

            TreeViewItem* pParent = itemFromIndex(parent);

            assert(NULL != pParent);
            if (NULL != pParent)
            {
                return pParent->childCount();
            }
            else
            {
                return 0;
            }
        }

        QModelIndex ComboxTreeModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
        {
            assert(NULL != m_pRootItem);
            if (!hasIndex(row, column, parent))
            {
                return QModelIndex();
            }

            TreeViewItem* pParentItem = itemFromIndex(parent);

            assert(NULL != pParentItem);
            if (NULL != pParentItem)
            {
                TreeViewItem* pItem = pParentItem->child(row);
                QModelIndex oCreateIndex = createIndex(row, 0, pItem);
                return oCreateIndex;
            }

            return QModelIndex();
        }

        //bool ComboxTreeModel::setData(const QModelIndex& index, const QVariant& value, int role )
        //{/*使用数据value和角色role分别替换列表s1和rol中原有的值，使用replace便于下一个示例(拖放)的使用.*/
        //    TreeViewItem* pItem = static_cast<TreeViewItem*>(index.internalPointer());
        //    
        //    pItem->setData(index,value, role);
        //   /* m_mapInfos.replace(index.row() * columnCount() + index.column(), value);
        //    rol.replace(index.row() * columnCount() + index.column(), role);*/
        //    emit dataChanged(index, index);    	//数据改变后，需要发送此信号.
        //    return true;	
        //}			//返回true，表示数据设置成功.


        QModelIndex ComboxTreeModel::parent(const QModelIndex& child) const
        {
            TreeViewItem* pItem = itemFromIndex(child);
            assert(NULL != pItem);

            TreeViewItem* pParentItem = pItem->parent();
            if (NULL == pParentItem)
            {
                return QModelIndex();
            }
            else if (pParentItem == m_pRootItem)
            {
                return QModelIndex();
            }
            else
            {
                QModelIndex oCreateIndex = createIndex(pParentItem->row(), 0, pParentItem);
                return oCreateIndex;
            }
        }

        QVariant ComboxTreeModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
        {
            if (!index.isValid())
            {
                return QVariant();
            }

            TreeViewItem* pItem = static_cast<TreeViewItem*>(index.internalPointer());
            assert(NULL != pItem);



            if (NULL != pItem)
            {
                return bulidData(pItem, index.column(), role);
            }
            else
            {
                return QVariant();
            }
        }

        Qt::ItemFlags ComboxTreeModel::flags(const QModelIndex& index) const
        {
            TreeViewItem* pItem = itemFromIndex(index);
            assert(NULL != pItem);

            if (NULL != pItem)
            {
                if (pItem->DataInfo()->parentInfo() == NULL)
                {
                    return pItem->flags(0) & ~Qt::ItemIsSelectable;//使得不可选中 chy add
                }
                else
                {
                    return pItem->flags(index.column());
                }
            }
            else
            {
                return Qt::NoItemFlags;
            }
        }

        TreeViewItem* ComboxTreeModel::appendItem(TreeViewDataInfo* pDataInfo, TreeViewItem* pParentItem)
        {
            assert(NULL != pDataInfo && NULL != pParentItem);
            if (NULL == pDataInfo || NULL == pParentItem)
            {
                return NULL;
            }
            //增加Item项
            //std::cout << pDataInfo->m_TreeItmeName.toStdString()<<" " << pDataInfo->childCount() << std::endl;
            TreeViewItem* pItem = new TreeViewItem(pDataInfo, pParentItem);
            assert(NULL != pItem);
            if (NULL == pItem)
            {
                return NULL;
            }
            /*std::cout << pItem->childCount() << std::endl;*/
            pParentItem->appendChild(pItem);

            if (!pDataInfo->m_ChildList.isEmpty())
            {
                for (int i = 0; i < pDataInfo->m_ChildList.size(); i++)
                {
                    TreeViewDataInfo* pChildMap = pDataInfo->m_ChildList.at(i);
                    assert(NULL != pChildMap);

                    if (NULL != pChildMap)
                    {
                        //树里不断增加子树
                        appendItem(pChildMap, pItem);
                    }
                }
            }
            return pItem;
        }

        QVariant ComboxTreeModel::bulidData(TreeViewItem* pItem, int nCol, int role /*= Qt::DisplayRole*/) const
        {
            assert(NULL != pItem->DataInfo());
            QString strShowName("");
            if (NULL == pItem->DataInfo())
            {
                return QVariant();
            }
            //else if (role == Qt::BackgroundRole)
            //{
            //    if (NULL == pItem->DataInfo()->parentInfo())
            //    {
            //       /* if (pItem->data(Qt::UserRole).toBool())
            //            return QColor(Qt::red);
            //        else*/
            //            return QColor(240, 240, 240);
            //    }
            //   /* else
            //    {
            //        if (pItem->data(Qt::UserRole).toBool())
            //            return QColor(Qt::blue);
            //        else
            //            return QColor(Qt::black);
            //    }*/
            //}
            /*if (NULL == pItem->DataInfo()->parentInfo())
            {

                if ( role == Qt::BackgroundRole)
                {
                    if (pItem->data(Qt::UserRole).toBool())
                        return QColor(Qt::red);
                    else
                        return QColor(Qt::gray);
                }

            }*/
            else if (Qt::TextAlignmentRole == role)
            {
                return QVariant(Qt::AlignVCenter | Qt::AlignLeft);
            }
            else if (Qt::DisplayRole == role)
            {
                switch (nCol)
                {
                case 0:
                    strShowName = pItem->DataInfo()->m_TreeItmeName;
                    return strShowName;
                default:
                    break;
                }
            }
            return QVariant();
        }

        //（4）窗体类的实现

        TestDialog::TestDialog(QWidget* parent) :QDialog(parent) //半自动化释放内存
        {
            initUI();
        }

        TestDialog::~TestDialog()
        {

        }

        QTreeListComboBox::QTreeListComboBox(QWidget* parent)
            : QComboBox(parent)
        {


            m_pTreeView = new QTreeComboBoxView(this);
            m_pTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
            m_pTreeView->setHeaderHidden(true);
            m_pTreeModel = new ComboxTreeModel(this);
            this->setModel(m_pTreeModel);
            this->setView(m_pTreeView);

            connect(m_pTreeView, &QTreeComboBoxView::treeMousePressed, [&](bool inItem) {
                canHidePopup = !inItem;
                });

            connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, [&](int index) {
                hidePopup();
                });
        }

        void QTreeListComboBox::hidePopup()
        {
            if (canHidePopup) QComboBox::hidePopup();
        }
        void TestDialog::refreshModelData(TreeViewDataInfoList DataList)
        {
            if (m_pTreeModel == nullptr || m_pTreeView == nullptr)
                return;
            m_pTreeModel->updataModelTree(DataList);  //根据新的数据更新combox下的下拉树
            m_pTreeView->expandAll(); //全部展开
            setDefaultSelectItem();  //设置默认选中项
        }

        void TestDialog::setDefaultSelectItem(QString strItemName)
        {
            QAbstractItemModel* model = m_pCombox->view()->model();
            QModelIndexList Items = model->match(model->index(0, 0), Qt::DisplayRole, QVariant::fromValue(strItemName), 2, Qt::MatchRecursive);
            //这种做法解决combox中存在TreeView视图时刷洗设置二级以下节点为当前默认显示项Index无法显示正确信息的问题.
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

        void TestDialog::initUI()
        {
            QHBoxLayout* pLayOut = new QHBoxLayout;
            pLayOut->setContentsMargins(16, 10, 16, 10); //设置布局左右顶底距离的位置
            m_pCombox = new QComboBox(this);
            m_pTreeModel = new ComboxTreeModel();
            m_pTreeView = new QTreeView(m_pCombox);
            m_pTreeView->setHeaderHidden(true);
            //m_pTreeView->header()->hide();   //不显示头的行列
            //m_pTreeView->header()->hideSection(0);
            pLayOut->addWidget(m_pCombox);
            m_pCombox->setModel(m_pTreeModel); //将model设入树combox中
            m_pCombox->setView(m_pTreeView); //将treeView设入combox中
            this->setLayout(pLayOut);  //将布局加入到窗体中
        }
//    }
//}