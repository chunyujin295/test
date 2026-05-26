/********************************************************************************
** Form generated from reading UI file 'ProjectionSelectorBase.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROJECTIONSELECTORBASE_H
#define UI_PROJECTIONSELECTORBASE_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include "Gui/FilterLineEdit.h"
#include "Core/BlockObject.h"

namespace AI3D
{
    namespace PROJ
    {
QT_BEGIN_NAMESPACE

class Ui_ProjectionSelectorBase
{
public:
    QGridLayout *gridLayout_2;
    QCheckBox *mCheckBoxNoProjection;
    QFrame *mFrameProjections;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_5;
    FilterLineEdit *leSearch;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QSplitter *mSplitter;
    QTreeView *lstRecent;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *cbxHideDeprecated;
    QTreeView *lstCoordinateSystems;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_4;
    QTextEdit *teProjection;
    QVBoxLayout *verticalLayout;
    QHBoxLayout* pHBottomLayout;
    QPushButton* butOk;
    QPushButton* butCancel;
    QHBoxLayout* hlTitle;
    QLabel* lblTitle;
    QPushButton* butClose;
    QLineEdit* leNumOfItems;
    QLabel* lblItem;

    QHBoxLayout* hlSelection;
    QHBoxLayout* hlType;
    QHBoxLayout* hlDefinition;
    QLabel* lblSelection;
    QLineEdit* leSelection;
    QLabel* lblType;
    QLabel* lblTypeContent;
    QLabel* lblDefinition;
    QTextEdit* teDefinition;
    QLabel* lblUserDefinedEdit;
    QLineEdit* leLatitude;
    QLineEdit* leLongitude;

    void setupUi(QWidget *ProjectionSelectorBase)
    {
        if (ProjectionSelectorBase->objectName().isEmpty())
            ProjectionSelectorBase->setObjectName(QString::fromUtf8("ProjectionSelectorBase"));
        ProjectionSelectorBase->resize(578, 650);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ProjectionSelectorBase->sizePolicy().hasHeightForWidth());
        ProjectionSelectorBase->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        ProjectionSelectorBase->setWindowIcon(icon);
        gridLayout_2 = new QGridLayout(ProjectionSelectorBase);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
///        gridLayout_2->setContentsMargins(0, 0, 0, 0);

        hlTitle = new QHBoxLayout();
        lblTitle = new QLabel(ProjectionSelectorBase);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblTitle->setText("空间参考系统");
        }
        else
        {
            lblTitle->setText("Spatial Reference System");
        }

        lblTitle->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        butClose = new QPushButton(ProjectionSelectorBase);
        butClose->setObjectName(QString::fromUtf8("butClose"));
        //butClose->setText("Close");
        butClose->setFixedSize(26, 26);

        hlTitle->addWidget(lblTitle);
        hlTitle->addStretch(1);
        hlTitle->addWidget(butClose);

        gridLayout_2->addLayout(hlTitle, 0, 0, 1, 1);

        mCheckBoxNoProjection = new QCheckBox(ProjectionSelectorBase);
        mCheckBoxNoProjection->setObjectName(QString::fromUtf8("mCheckBoxNoProjection"));
        mCheckBoxNoProjection->setVisible(false);

        gridLayout_2->addWidget(mCheckBoxNoProjection, 1, 0, 1, 1);

        mFrameProjections = new QFrame(ProjectionSelectorBase);
        mFrameProjections->setObjectName(QString::fromUtf8("mFrameProjections"));
        mFrameProjections->setFrameShape(QFrame::NoFrame);
        mFrameProjections->setFrameShadow(QFrame::Plain);
        verticalLayout_2 = new QVBoxLayout(mFrameProjections);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_5 = new QLabel(mFrameProjections);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        horizontalLayout->addWidget(label_5);

        leSearch = new FilterLineEdit(mFrameProjections);
        leSearch->setObjectName(QString::fromUtf8("leSearch"));

        leSearch->setStyleSheet("QLineEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");
        //leSearch->setStyleSheet(" #leSearch { background-color:rgb(32,36,43);color:rgb(255,255,255);} ");

        horizontalLayout->addWidget(leSearch);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(mFrameProjections);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_3->setFont(font);

        horizontalLayout_2->addWidget(label_3);

        label_3->setVisible(false);

        verticalLayout_2->addLayout(horizontalLayout_2);

        mSplitter = new QSplitter(mFrameProjections);
        mSplitter->setObjectName(QString::fromUtf8("mSplitter"));
        mSplitter->setOrientation(Qt::Vertical);
        mSplitter->setChildrenCollapsible(false);
        lstRecent = new QTreeView(mSplitter);
        lstRecent->setObjectName(QString::fromUtf8("lstRecent"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lstRecent->sizePolicy().hasHeightForWidth());
        lstRecent->setSizePolicy(sizePolicy1);
        lstRecent->setMinimumSize(QSize(0, 105));
        lstRecent->setAlternatingRowColors(false);
        lstRecent->setUniformRowHeights(true);
        mSplitter->addWidget(lstRecent);
        layoutWidget = new QWidget(mSplitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, 0, -1, -1);
        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        ///label_4->setFont(font);

        label_4->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        horizontalLayout_3->addWidget(label_4);

        leNumOfItems = new QLineEdit(ProjectionSelectorBase);

        leNumOfItems->setFixedWidth(55);

        leNumOfItems->setText("12");
        leNumOfItems->setAlignment(Qt::AlignHCenter);

        leNumOfItems->setStyleSheet("QLineEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");

        lblItem = new QLabel(ProjectionSelectorBase);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblItem->setText("个项目");
        }
        else
        {
            lblItem->setText("items");
        }
        
        lblItem->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        horizontalLayout_3->addWidget(leNumOfItems);
        horizontalLayout_3->addWidget(lblItem);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        cbxHideDeprecated = new QCheckBox(layoutWidget);
        cbxHideDeprecated->setObjectName(QString::fromUtf8("cbxHideDeprecated"));
        QFont font1;
        font1.setPointSize(9);
        cbxHideDeprecated->setFont(font1);

        horizontalLayout_3->addWidget(cbxHideDeprecated);


        gridLayout->addLayout(horizontalLayout_3, 0, 0, 1, 1);

        lstCoordinateSystems = new QTreeView(layoutWidget);
        lstCoordinateSystems->setObjectName(QString::fromUtf8("lstCoordinateSystems"));
        ///lstCoordinateSystems->setAlternatingRowColors(true);
        lstCoordinateSystems->setAlternatingRowColors(false);
        lstCoordinateSystems->setUniformRowHeights(true);

        gridLayout->addWidget(lstCoordinateSystems, 1, 0, 1, 1);

        mSplitter->addWidget(layoutWidget);
        layoutWidget_2 = new QWidget(mSplitter);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));

        horizontalLayout_4 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        teProjection = new QTextEdit(layoutWidget_2);
        teProjection->setObjectName(QString::fromUtf8("teProjection"));
        sizePolicy.setHeightForWidth(teProjection->sizePolicy().hasHeightForWidth());
        teProjection->setSizePolicy(sizePolicy);
        teProjection->setMinimumSize(QSize(0, 0));
        teProjection->setBaseSize(QSize(0, 40));
        teProjection->setAutoFormatting(QTextEdit::AutoBulletList);
        teProjection->setReadOnly(true);

        horizontalLayout_4->addWidget(teProjection);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, 0, -1);

        horizontalLayout_4->addLayout(verticalLayout);

        mSplitter->addWidget(layoutWidget_2);

        verticalLayout_2->addWidget(mSplitter);


        gridLayout_2->addWidget(mFrameProjections, 2, 0, 1, 1);

        hlSelection = new QHBoxLayout();
        lblSelection = new QLabel(ProjectionSelectorBase);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblSelection->setText("选择集:");
        }
        else
        {
            lblSelection->setText("Selection:");
        }
        
        lblSelection->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        leSelection = new QLineEdit(ProjectionSelectorBase);
        hlSelection->addSpacing(0);
        hlSelection->addWidget(lblSelection);
        hlSelection->addWidget(leSelection, 1);

        leSelection->setStyleSheet("QLineEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");

        hlType = new QHBoxLayout();
        lblType = new QLabel(ProjectionSelectorBase);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            lblType->setText("类型:");
        }
        else
        {
            lblType->setText("Type:");
        }
        
        lblType->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        lblTypeContent = new QLabel(ProjectionSelectorBase);
        lblTypeContent->setText("Defined system");

        lblTypeContent->setStyleSheet("QLabel { background-color:rgb(32,36,43);color:rgb(255,255,255); }");
        lblTypeContent->setFixedWidth(833);

        leLatitude = new QLineEdit(ProjectionSelectorBase);
        leLatitude->setFixedWidth(833);
        leLatitude->setFixedHeight(36);
        leLatitude->setStyleSheet("QLineEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");

        hlType->addSpacing(40);
        hlType->addWidget(lblType);
        hlType->addWidget(leLatitude);
        hlType->addWidget(lblTypeContent);
        hlType->addStretch(1);

        hlDefinition = new QHBoxLayout();
        lblDefinition = new QLabel(ProjectionSelectorBase);
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {

            lblDefinition->setText("定义:");
        }
        else
        {
            lblDefinition->setText("Definition:");
        }
        
        lblDefinition->setStyleSheet("QLabel { color:rgb(255,255,255); }");

        teDefinition = new QTextEdit(ProjectionSelectorBase);
        teDefinition->setFixedWidth(833);
        teDefinition->setFixedHeight(70);
        teDefinition->setStyleSheet("QTextEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");

        leLongitude = new QLineEdit(ProjectionSelectorBase);
        leLongitude->setFixedWidth(833);
        leLongitude->setFixedHeight(36);
        leLongitude->setStyleSheet("QLineEdit { background-color:rgb(32,36,43);color:rgb(255,255,255); }");

        lblUserDefinedEdit = new QLabel(ProjectionSelectorBase);
        lblUserDefinedEdit->setText("Edit");
        lblUserDefinedEdit->setStyleSheet("background-color:transparent; color:#4E5562; text-decoration:underline;");

        hlDefinition->addSpacing(40);
        hlDefinition->addWidget(lblDefinition,0,Qt::AlignTop);
        //hlDefinition->addWidget(teDefinition, 1);
        hlDefinition->addWidget(teDefinition);
        hlDefinition->addWidget(leLongitude);
        hlDefinition->addSpacing(10);
        hlDefinition->addWidget(lblUserDefinedEdit);
        hlDefinition->addStretch(1);

        gridLayout_2->addLayout(hlSelection, 3, 0, 1, 1);
        gridLayout_2->addLayout(hlType, 4, 0, 1, 1);
        gridLayout_2->addLayout(hlDefinition, 5, 0, 1, 1);

        pHBottomLayout = new QHBoxLayout();
        butOk = new QPushButton(ProjectionSelectorBase);
        butCancel = new QPushButton(ProjectionSelectorBase);
        butOk->setObjectName(QString::fromUtf8("butOk"));
        butCancel->setObjectName(QString::fromUtf8("butCancel"));

        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            butOk->setText("确定");
            butCancel->setText("取消");
        }
        else
        {
            butOk->setText("OK");
            butCancel->setText("Cancel");
        }
        butOk->setFont(font);
        butOk->setStyleSheet(
            "QPushButton { background-color:#1547F8;color:#E6FFFFFF;border:none;border-radius:6px;font:16px \"Arial\"; }"
            "QPushButton:pressed { background-color:#3F455C; }"
            "QPushButton:disabled { background-color:#3F455C; } ");
      //  butOk->setStyleSheet("QPushButton { background-color:rgb(171,171,171);color:rgb(74,74,74); }");
       // butCancel->setStyleSheet("QPushButton { background-color:rgb(171,171,171);color:rgb(74,74,74); }");
        butCancel->setFont(font);
      
        butCancel->setStyleSheet("background-color:#3F455C;color:#FFFFFF;border:none;border-radius:6px;font:16px \"Arial\";");

        butOk->setFixedSize(120, 46);
        butCancel->setFixedSize(120, 46);


        pHBottomLayout->addStretch(1);
        pHBottomLayout->addWidget(butOk);
        pHBottomLayout->addWidget(butCancel);

        gridLayout_2->addLayout(pHBottomLayout, 6, 0, 1, 1);

        butClose->setIcon(QPixmap(":/new/prefix1/skin/srs_close26.png"));

        butClose->hide();
        leLongitude->hide();
        leLatitude->hide();

        QWidget::setTabOrder(leSearch, lstRecent);
        QWidget::setTabOrder(lstRecent, cbxHideDeprecated);
        QWidget::setTabOrder(cbxHideDeprecated, lstCoordinateSystems);

        teProjection->setVisible(false);
        cbxHideDeprecated->setVisible(false);
        lstRecent->setVisible(false);

        retranslateUi(ProjectionSelectorBase);

        QMetaObject::connectSlotsByName(ProjectionSelectorBase);
    } // setupUi

    void retranslateUi(QWidget *ProjectionSelectorBase)
    {
        ProjectionSelectorBase->setWindowTitle(QApplication::translate("ProjectionSelectorBase", "Spatial Reference System Database", nullptr));
#ifndef QT_NO_TOOLTIP
        mCheckBoxNoProjection->setToolTip(QApplication::translate("ProjectionSelectorBase", "Use this option to treat all coordinates as Cartesian coordinates in an unknown reference system.", nullptr));
#endif // QT_NO_TOOLTIP
        mCheckBoxNoProjection->setText(QApplication::translate("ProjectionSelectorBase", "No CRS (or unknown/non-Earth projection)", nullptr));
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            label_5->setText(QApplication::translate("ProjectionSelectorBase", "筛选器", nullptr));
        }
        else
        {
            label_5->setText(QApplication::translate("ProjectionSelectorBase", "Filter", nullptr));
        }

        label_3->setText(QApplication::translate("ProjectionSelectorBase", "Recently Used Coordinate Reference Systems", nullptr));
///        label_4->setText(QApplication::translate("ProjectionSelectorBase", "Predefined Coordinate Reference Systems", nullptr));
        if (AI3D::CORE::BlockObject::isChineseVersion())
        {
            label_4->setText(QApplication::translate("ProjectionSelectorBase", "空间参考系统", nullptr));
        }
        else
        {
            label_4->setText(QApplication::translate("ProjectionSelectorBase", "Spatial Reference Systems", nullptr));
        }
        
        cbxHideDeprecated->setText(QApplication::translate("ProjectionSelectorBase", "Hide deprecated CRSs", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProjectionSelectorBase: public Ui_ProjectionSelectorBase {};
} // namespace Ui

QT_END_NAMESPACE
	}
}
#endif // UI_PROJECTIONSELECTORBASE_H
