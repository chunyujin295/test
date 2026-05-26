
#pragma once
#include <QComboBox>
#include <QFrame>
#include <Qlist>
#include <QTreeView>
#include <QDialog>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

//namespace AI3D
//{
//    namespace GUI
//    {

        enum crsoption_s
        {
            CRS_DefaultCrs, //!< Global default QGIS CRS
            CRS_MoreCrs, //!< Current project CRS (if OTF reprojection enabled)
            CRS_CurrentCrs, //!< Current user selected CRS

            CRS_RecentCrs,
            CrsNotSet

        };






        class TreeDelegate;
        class TreeViewDataInfo;
        class QTreeComboBoxView : public QTreeView
        {
            Q_OBJECT
        public:
            QTreeComboBoxView(QWidget* parent = Q_NULLPTR);
            ~QTreeComboBoxView() {};
            //void SetTootNodeUnselectable();
        protected:
            void mousePressEvent(QMouseEvent* event) override;
            void mouseMoveEvent(QMouseEvent* event) override;
        signals:
            void treeMousePressed(bool inItem);
        private:
            TreeDelegate* pItemDelegate;
        };

        class TreeDelegate : public QStyledItemDelegate
        {
            Q_OBJECT

        public:
            TreeDelegate(QObject* parent = Q_NULLPTR);
            ~TreeDelegate();

        public:
            void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

        private:

        };

        typedef QList<TreeViewDataInfo*> TreeViewDataInfoList;
        class TreeViewDataInfo
        {
        public:
            TreeViewDataInfo(crsoption_s opt = CRS_DefaultCrs, TreeViewDataInfo* pParent = NULL);
            ~TreeViewDataInfo();
            int childCount() const;
            //存在子信息
            int childExists(const TreeViewDataInfo* pMap) const;
            TreeViewDataInfo* parentInfo() const;
        public:
            QString m_TreeItmeName;
            TreeViewDataInfoList m_ChildList;  //子节点的集合
            TreeViewDataInfo* m_pParentInfo;  // 父节点信息
            int m_childCount;

            crsoption_s crs_opt_;
        };

        //第二、数据层管理管理设计

        class TreeViewItem : public QObject
        {
        public:
            explicit TreeViewItem(TreeViewDataInfo* pMapInfo, TreeViewItem* pParent = NULL);
            ~TreeViewItem();
        public:
            //获取父项
            TreeViewItem* parent() const;
            // 获取子项
            TreeViewItem* child(int nIndex) const;
            // 增加子项
            void appendChild(TreeViewItem* pItem);
            // 获取子项个数
            int childCount() const;
            //获取子项索引
            int childIndex(const TreeViewItem* pItem) const;
            // 删除子项
            bool removeChild(int nIndex);
            // 获取列数据
            QVariant data(int nCol, int role = Qt::DisplayRole);
            // 设置列数据
            bool setData(int nCol, QVariant val, int role = Qt::EditRole);
            //行号
            int row() const;
            //树Itme项的标志
            Qt::ItemFlags flags(int nCol) const;

            //获取数据
            TreeViewDataInfo* DataInfo() const;
            //交换子项位置
            bool swapChild(int nFirst, int nSecond);

        private:
            TreeViewDataInfo* m_pDataInfo;
            TreeViewItem* m_pParent;            // 父项
            QList<TreeViewItem*> m_childItems;  // 子项集合

        };

        //第三、model数据的实现

        class ComboxTreeModel : public QAbstractItemModel
        {
            Q_OBJECT
        public:
            explicit ComboxTreeModel(QObject* parent = NULL);
            ~ComboxTreeModel();
            void updataModelTree(TreeViewDataInfoList MapList);  //更新treeView中的节点信息
            TreeViewItem* getRootItem() { return m_pRootItem; };
        public:
            TreeViewItem* itemFromIndex(QModelIndex index) const;  //从index 里获取具体的item信息
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const { return 1; };
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& child) const;
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
            //virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
            virtual Qt::ItemFlags flags(const QModelIndex& index) const;
            TreeViewItem* appendItem(TreeViewDataInfo* pDataInfo, TreeViewItem* pParentItem);
            QVariant bulidData(TreeViewItem* pItem, int nCol, int role = Qt::DisplayRole) const;
        private:
            TreeViewDataInfoList m_mapInfos;
            TreeViewItem* m_pRootItem;
            TreeViewDataInfo* m_pForRoot;
        };

        //第四、窗体类

        class TestDialog : public QDialog
        {
            Q_OBJECT
        public:
            TestDialog(QWidget* parent = nullptr);
            ~TestDialog();
            void refreshModelData(TreeViewDataInfoList DataList);
            void setDefaultSelectItem(QString strItemName = QString(""));//根据显示的item去设置默认的选项
        private:
            void initUI();
        private:
            ComboxTreeModel* m_pTreeModel;
            QTreeView* m_pTreeView;
            QComboBox* m_pCombox;
        };



        /**********QTreeView**********/


        class QTreeListComboBox : public QComboBox
        {
            Q_OBJECT
        public:
            QTreeListComboBox(QWidget* parent = Q_NULLPTR);
            ~QTreeListComboBox() {};
            //void SetTootNodeUnselectable();
        protected:
            void hidePopup() override;
        private:
            bool canHidePopup = true;//允许收缩
        public:
            QTreeComboBoxView* m_pTreeView = nullptr;
            ComboxTreeModel* m_pTreeModel;
        };


//    }
//}
