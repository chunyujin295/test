#pragma once
#include <qwidget.h>
//#include <QWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QCloseEvent>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLCDNumber>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QCheckBox>


#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QListWidget>
#include <QResizeEvent>

#include <QSortFilterProxyModel>
#include <qdatetime.h>
#include <qtimer.h>
#include "Util/Settings.h"
#include "Gui/GlobalStruct.h"







//

//namespace AI3D

    //namespace GUI
    
        class MoPhotoDelegate;
        struct photogroup_list_item_st
        {
            std::string photogroupname_;
            int id_;
            int photocount_;
            double sensorsize_;
            double focalmm_;
            double focal35mm_;
           /* std::string  sensorsize_str_;
            std::string focalmm_str_;
            std::string focal35mm_str_;*/

        };


        ///typedef 
        class photopose_list_item_st
        {
        public:
            photopose_list_item_st() {}
           
            int image_id_;
            std::string photo_name_;
            std::string photo_dir_;
            pose_status_e status_;
            std::string  posvalus_str_="";

            static bool compareLessThan(const photopose_list_item_st& t1, const photopose_list_item_st& t2);
        };



        typedef enum column_photogroup_e
        {
            PGSERINO_COL = 0,//序号
            PGNAME_COL,
            PGPHOTOCOUNT_COL,
            PGSENSORSIZE_COL,
            PGFOCALLENGTH_COL,

            PG35MMFOCAL_COL,
            COUNT_PGCOL,
        }col_pg_e;

        typedef enum column_photoposlist_e
        {
            PHOTONAME_COL = 0,
            PHOTODIR_COL,
            PHOTOPOSESTATUS_COL,
            PHOTOPOS_COL,

            COUNT_POHOTOPOSCOL
        }col_photopos_e;
        //#endif
        //? 
        //1:滚动条
        //2:表头排序的逻辑，貌似单击photo 和rmx排序结果不太对
        class MoPhotoTableWidget : public QTableView
        {
            Q_OBJECT
        public:
            explicit MoPhotoTableWidget(QWidget* parent = nullptr);
            virtual ~MoPhotoTableWidget();

            void printInfo();
            bool bLeaved;
            int getMode();
            void SetMode(int _mode) { mode = _mode; };
            void clearData();
            void InitHeader();
            int getColCount();
            QStringList& getheaderLabels() { return headerLabels; }
            void SetSelectionChanged(bool bchanged = false);

            bool bchanged_ = false;
            QList<photogroup_list_item_st>& getPhotoGroupListData() { return photogroupListData; }

            void removeOneRow(int row);
            void selectOneRow(int row);

            void selectOneRowByGroupId(int groupId);
            void selectOneRowByImageId(uint32_t imageId);
            int ColCount();
            int RowCount();
            void appendRowData(photogroup_list_item_st& groupListItem);
            void updateRowData(int row, photogroup_list_item_st& groupListItem);
            int findRowByGroupId(uint group_id);
            uint64_t getGroupIdByRow(int row);
            uint32_t getImageIdByRow(int row);
            QStandardItem* getItem(int row ,int col);
            void appendRowData(photopose_list_item_st& poseItem);

            QList<photopose_list_item_st>& getPhotosposeListData() { return photoposListData; }
            void setPhotosposeListData(QList<photopose_list_item_st>& photoposListData);

            void setHeaderLabelsMode();

            virtual void mousePressEvent(QMouseEvent* event);
            virtual void mouseReleaseEvent(QMouseEvent* event);

        protected:
            void leaveEvent(QEvent* event);
            void updateRow(int row);
            void hideEvent(QHideEvent* event);


        public slots:
            //void cellEntered(int, int);

            void cellEntered2(const QModelIndex& index);
            void doubleClicked(const QModelIndex& index);
            void Slot_itemModified(int, int, const QString&) const;

        signals:
            void itemModified(int, int, const QString&) const;

        private:
            QColor previousHoverRowBackColor;
            int previousHoverRow;

            QColor origBackColor0; // alternative color0
            QColor origBackColor1; // alternative color1
            QColor origBackColor2; // selection color
            QColor origBackColor3; // selection and hover color
            QColor origBackColor4; // hover color

            void setRowColor(int, QColor);
            
            int selectedRow;
            int iHoverRow;
            MoPhotoDelegate* pItemDelegate;

            QStandardItemModel* pStandardItemModel = nullptr;
            int mode = 0;
            int colCount;
            QStringList headerLabels;

            QList<photogroup_list_item_st> photogroupListData;
            QList<photopose_list_item_st> photoposListData;
        };

        class MoPhotoDelegate : public QStyledItemDelegate
        {
            Q_OBJECT
        public:
            MoPhotoDelegate(QWidget* parent = nullptr,int mode=0);

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
            void setEditorData(QWidget* editor, const QModelIndex& index) const override;
            void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
            void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

            void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
            QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
           
        public slots:
            // todo：add leave event?
            void cellEntered(int row, int col);
            void cellEntered2(const QModelIndex& index);
            void doubleClicked(const QModelIndex& index);

        signals:
            void itemModified(int, int, const QString&) const;

        private:
            MoPhotoTableWidget* pTableWidget;
            int iHoverRow;
            int mode_;
           
        };

        class MoPhotoHeaderView : public QHeaderView
        {
            Q_OBJECT
        public:
            MoPhotoHeaderView(Qt::Orientation orientation, QWidget* parent = nullptr,int mode=0);
            ~MoPhotoHeaderView();

        protected:
            void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const;
            virtual QSize sizeHint() const;

        private:
            MoPhotoTableWidget* pTableWidget;
            int mode_ = 0;
        };


    


