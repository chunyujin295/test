/********************************************************************************
** Form generated from reading UI file 'BlockWgtCN.ui'
**
** Created by: Qt User Interface Compiler version 5.12.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BLOCKWGTCN2_H
#define UI_BLOCKWGTCN2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "Gui/MoPhotoWidget.h"

QT_BEGIN_NAMESPACE

class Ui_CBlockWgtCN2
{
public:
    QGridLayout *gridLayout_15;
    QWidget *widget_2;
    QFrame *frame;
    QGridLayout *gridLayout_8;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btn_addsig;
    QPushButton *btn_adddir;
    QPushButton *btn_push_removePgtable;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_Pho;
    QFrame *frame_2;
    QGridLayout *gridLayout_7;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btn_addpos;
    QPushButton *btn_delpos;
    QLabel *label_Pos;
    QFrame *frame_4;
    QGridLayout *gridLayout_9;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_10;
    QPushButton *btn_Siggcp;
    QPushButton *btn_addgcp;
    QPushButton *btn_delgcp;
    QFrame *frame_3;
    QGridLayout *gridLayout_6;
    QLabel *label_AT_2;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *btn_at;
    QPushButton *btn_paus;
    QPushButton *btn_rec;
    QSpacerItem *horizontalSpacer_7;
    QWidget *widget_3;
    QGridLayout *gridLayout_14;
    QGridLayout *gridLayout_10;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_AddData;
    QLabel *label_AT;
    QLabel *label_Reconstruction;
    QLabel *label_Production;
    QSpacerItem *horizontalSpacer_10;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout_3;
    MoPhotoTableWidget *tableView_photogroup;
    QVBoxLayout *verticalLayout_10;
    MoPhotoTableWidget *tableView_photo_pos;
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QGridLayout *gridLayout_12;
    QWidget *wgt_photogroup_info_;
    QVBoxLayout *verticalLayout_8;
    QWidget *wgt_form_photogroup;
    QGridLayout *gridLayout_5;
    QLineEdit *le_photogroup_sensorsize;
    QLabel *label_27;
    QLineEdit *le_photogroup_name;
    QLineEdit *le_k3_2;
    QLineEdit *le_photogroup_dir;
    QLabel *label_focalength;
    QLabel *k3Label_2;
    QLabel *label_18;
    QLabel *label_28;
    QLineEdit *le_p2_2;
    QLabel *label_16;
    QLabel *label_30;
    QLineEdit *le_k2_2;
    QLabel *label_14;
    QLineEdit *le_photogroup_imagesize;
    QLineEdit *le_k1_2;
    QLabel *label_26;
    QLineEdit *le_photogroup_camera;
    QLabel *label_4;
    QLineEdit *le_photogroup_num;
    QLabel *label_9;
    QLabel *label_32;
    QLineEdit *le_focalength;
    QLabel *label_31;
    QLabel *label_33;
    QLineEdit *le_p1_2;
    QSpacerItem *verticalSpacer_3;
    QWidget *wgt_photo_priview;
    QGridLayout *gridLayout_4;
    QLabel *label_Group;
    QVBoxLayout *verticalLayout_5;
    QSpacerItem *verticalSpacer_7;
    QGraphicsView *graphics_view_photo;
    QVBoxLayout *verticalLayout_4;
    QSpacerItem *verticalSpacer_5;
    QLabel *label_photo_open;
    QSpacerItem *verticalSpacer_6;
    QWidget *widget_normal;
    QFormLayout *formLayout;
    QLabel *label_2;
    QLineEdit *le_name;
    QLabel *label_3;
    QLineEdit *le_path;
    QLabel *label_20;
    QLineEdit *le_photo_ser_siz;
    QLabel *label_22;
    QLabel *label_23;
    QLineEdit *le_pos_lon;
    QLabel *label_24;
    QLineEdit *le_pos_lat;
    QLabel *label_25;
    QLineEdit *le_pos_height;
    QComboBox *comboBox;
    QSpacerItem *verticalSpacer_4;
    QWidget *tab_4;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_6;
    QProgressBar *progressBar_submit;
    QGridLayout *gridLayout_2;
    QLabel *label_cancleLog;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_7;
    QLabel *label_blockID;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label;
    QLabel *label_complete_time;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_Status;
    QLabel *label_StatusValue;
    QLabel *label_view_report;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_Progress;
    QLabel *label_ProgressValue;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_11;
    QLabel *label_create_time;
    QTableWidget *taskList;
    QPushButton *btn_newContruction;

    void setupUi(QWidget *CBlockWgtCN)
    {
        if (CBlockWgtCN->objectName().isEmpty())
            CBlockWgtCN->setObjectName(QString::fromUtf8("CBlockWgtCN"));
        CBlockWgtCN->setEnabled(true);
        CBlockWgtCN->resize(1307, 1462);
        CBlockWgtCN->setFocusPolicy(Qt::ClickFocus);
        CBlockWgtCN->setStyleSheet(QString::fromUtf8("font: 14px \"Arial\";\n"
"color: rgb(255, 255, 255);\n"
"background-color: #222222;\n"
"\n"
"\n"
"\n"
"\n"
""));
        gridLayout_15 = new QGridLayout(CBlockWgtCN);
        gridLayout_15->setObjectName(QString::fromUtf8("gridLayout_15"));
        gridLayout_15->setHorizontalSpacing(0);
        gridLayout_15->setVerticalSpacing(2);
        gridLayout_15->setContentsMargins(0, 2, 0, 0);
        widget_2 = new QWidget(CBlockWgtCN);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        widget_2->setMinimumSize(QSize(0, 65));
        widget_2->setMaximumSize(QSize(16777215, 65));
        widget_2->setStyleSheet(QString::fromUtf8("border: 2px solid #222222;\n"
"border-left:none;\n"
"background-color: #3C3D3F;\n"
""));
        frame = new QFrame(widget_2);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(5, 5, 115, 55));
        frame->setMinimumSize(QSize(115, 55));
        frame->setMaximumSize(QSize(113, 55));
        frame->setStyleSheet(QString::fromUtf8("border:1px solid;\n"
"border-color: #A5A5A5;\n"
"background-color: #222222;"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout_8 = new QGridLayout(frame);
        gridLayout_8->setSpacing(0);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        gridLayout_8->setContentsMargins(0, 1, 0, 5);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, 0, -1, -1);
        horizontalSpacer_2 = new QSpacerItem(13, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        btn_addsig = new QPushButton(frame);
        btn_addsig->setObjectName(QString::fromUtf8("btn_addsig"));
        btn_addsig->setEnabled(true);
        btn_addsig->setMinimumSize(QSize(35, 35));
        btn_addsig->setMaximumSize(QSize(35, 35));
        btn_addsig->setStyleSheet(QString::fromUtf8("\n"
"QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Import_photo_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_photo_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_photo_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_photo_no1x.png);\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
""));
        btn_addsig->setIconSize(QSize(36, 36));

        horizontalLayout_3->addWidget(btn_addsig);

        btn_adddir = new QPushButton(frame);
        btn_adddir->setObjectName(QString::fromUtf8("btn_adddir"));
        btn_adddir->setEnabled(true);
        btn_adddir->setMinimumSize(QSize(35, 35));
        btn_adddir->setMaximumSize(QSize(35, 35));
        btn_adddir->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Import_directory_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_directory_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_directory_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_directory_no1x.png);\n"
"}\n"
"\n"
"\n"
"\n"
"\n"
""));

        horizontalLayout_3->addWidget(btn_adddir);

        btn_push_removePgtable = new QPushButton(frame);
        btn_push_removePgtable->setObjectName(QString::fromUtf8("btn_push_removePgtable"));
        btn_push_removePgtable->setMinimumSize(QSize(35, 35));
        btn_push_removePgtable->setMaximumSize(QSize(35, 35));
        btn_push_removePgtable->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Remove_selected_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_selected_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_selected_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_selected_no1x.png);\n"
"}"));

        horizontalLayout_3->addWidget(btn_push_removePgtable);

        horizontalSpacer_3 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        gridLayout_8->addLayout(horizontalLayout_3, 1, 0, 1, 1);

        label_Pho = new QLabel(frame);
        label_Pho->setObjectName(QString::fromUtf8("label_Pho"));
        label_Pho->setMinimumSize(QSize(113, 17));
        label_Pho->setMaximumSize(QSize(113, 17));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setBold(false);
        font.setItalic(false);
        font.setWeight(50);
        label_Pho->setFont(font);
        label_Pho->setStyleSheet(QString::fromUtf8("font: 12px \"Arial\";\n"
"color: rgb(165, 165, 165);\n"
"background-color:#3C3D3F;\n"
"border:0px solid;\n"
"\n"
""));
        label_Pho->setAlignment(Qt::AlignCenter);

        gridLayout_8->addWidget(label_Pho, 0, 0, 1, 1);

        gridLayout_8->setRowStretch(0, 10);
        gridLayout_8->setRowStretch(1, 30);
        frame_2 = new QFrame(widget_2);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setGeometry(QRect(126, 5, 78, 55));
        frame_2->setMinimumSize(QSize(78, 55));
        frame_2->setMaximumSize(QSize(78, 55));
        frame_2->setStyleSheet(QString::fromUtf8("border:1px solid;\n"
"border-color: #A5A5A5;\n"
"background-color: #222222;"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        gridLayout_7 = new QGridLayout(frame_2);
        gridLayout_7->setSpacing(0);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        gridLayout_7->setContentsMargins(0, 1, 0, 5);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, -1, 0);
        btn_addpos = new QPushButton(frame_2);
        btn_addpos->setObjectName(QString::fromUtf8("btn_addpos"));
        btn_addpos->setMinimumSize(QSize(35, 35));
        btn_addpos->setMaximumSize(QSize(35, 35));
        btn_addpos->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Add_POS_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Add_POS_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Add_POS_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Add_POS_no1x.png);\n"
"}"));

        horizontalLayout_4->addWidget(btn_addpos);

        btn_delpos = new QPushButton(frame_2);
        btn_delpos->setObjectName(QString::fromUtf8("btn_delpos"));
        btn_delpos->setMinimumSize(QSize(35, 35));
        btn_delpos->setMaximumSize(QSize(35, 35));
        btn_delpos->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Remove_POS_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_POS_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_POS_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Remove_POS_no1x.png);\n"
"}"));

        horizontalLayout_4->addWidget(btn_delpos);

        horizontalLayout_4->setStretch(0, 1);
        horizontalLayout_4->setStretch(1, 1);

        gridLayout_7->addLayout(horizontalLayout_4, 1, 0, 1, 1);

        label_Pos = new QLabel(frame_2);
        label_Pos->setObjectName(QString::fromUtf8("label_Pos"));
        label_Pos->setMinimumSize(QSize(76, 17));
        label_Pos->setMaximumSize(QSize(113, 17));
        label_Pos->setStyleSheet(QString::fromUtf8("font: 12px \"Arial\";\n"
"color: rgb(165, 165, 165);\n"
"background-color:#3C3D3F;\n"
"border:0px solid;"));
        label_Pos->setAlignment(Qt::AlignCenter);

        gridLayout_7->addWidget(label_Pos, 0, 0, 1, 1);

        gridLayout_7->setRowStretch(0, 10);
        frame_4 = new QFrame(widget_2);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setGeometry(QRect(210, 5, 115, 55));
        frame_4->setMinimumSize(QSize(115, 55));
        frame_4->setMaximumSize(QSize(115, 55));
        frame_4->setStyleSheet(QString::fromUtf8("border:1px solid;\n"
"border-color: #A5A5A5;\n"
"background-color:#222222;"));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        gridLayout_9 = new QGridLayout(frame_4);
        gridLayout_9->setSpacing(0);
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        gridLayout_9->setContentsMargins(0, 1, 0, 0);
        label_5 = new QLabel(frame_4);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(113, 17));
        label_5->setMaximumSize(QSize(113, 17));
        label_5->setStyleSheet(QString::fromUtf8("font: 12px \"Arial\";\n"
"color: rgb(165, 165, 165);\n"
"background-color:#3C3D3F;\n"
"border:0px solid;"));
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout_9->addWidget(label_5, 0, 0, 1, 1);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setSpacing(0);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        btn_Siggcp = new QPushButton(frame_4);
        btn_Siggcp->setObjectName(QString::fromUtf8("btn_Siggcp"));
        btn_Siggcp->setMinimumSize(QSize(35, 35));
        btn_Siggcp->setMaximumSize(QSize(35, 35));
        btn_Siggcp->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Sig_GCP1x_nor.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Sig_GCP1x_hor.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Sig_GCP1x_pressed.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Sig_GCP1x_no.png);\n"
"}\n"
"\n"
""));

        horizontalLayout_10->addWidget(btn_Siggcp);

        btn_addgcp = new QPushButton(frame_4);
        btn_addgcp->setObjectName(QString::fromUtf8("btn_addgcp"));
        btn_addgcp->setMinimumSize(QSize(35, 35));
        btn_addgcp->setMaximumSize(QSize(35, 35));
        btn_addgcp->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Import_GCP_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_GCP_hon1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_GCP_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_GCP_no1x.png);\n"
"}\n"
"\n"
""));

        horizontalLayout_10->addWidget(btn_addgcp);

        btn_delgcp = new QPushButton(frame_4);
        btn_delgcp->setObjectName(QString::fromUtf8("btn_delgcp"));
        btn_delgcp->setMinimumSize(QSize(35, 35));
        btn_delgcp->setMaximumSize(QSize(35, 35));
        btn_delgcp->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/Import_RGCP_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_RGCP_hon1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_RGCP_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/Import_RGCP_no1x.png);\n"
"}\n"
"\n"
""));

        horizontalLayout_10->addWidget(btn_delgcp);

        horizontalLayout_10->setStretch(1, 1);
        horizontalLayout_10->setStretch(2, 1);

        gridLayout_9->addLayout(horizontalLayout_10, 1, 0, 1, 1);

        gridLayout_9->setRowStretch(0, 10);
        frame_3 = new QFrame(widget_2);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setGeometry(QRect(331, 5, 115, 55));
        frame_3->setMinimumSize(QSize(115, 55));
        frame_3->setMaximumSize(QSize(115, 55));
        frame_3->setStyleSheet(QString::fromUtf8("border:1px solid;\n"
"border-color: #A5A5A5;\n"
"background-color:#222222;"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        gridLayout_6 = new QGridLayout(frame_3);
        gridLayout_6->setSpacing(0);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setContentsMargins(0, 1, 0, 5);
        label_AT_2 = new QLabel(frame_3);
        label_AT_2->setObjectName(QString::fromUtf8("label_AT_2"));
        label_AT_2->setMinimumSize(QSize(113, 17));
        label_AT_2->setMaximumSize(QSize(113, 17));
        label_AT_2->setStyleSheet(QString::fromUtf8("font: 12px \"Arial\";\n"
"color: rgb(165, 165, 165);\n"
"background-color:#3C3D3F;\n"
"border:0px solid;"));
        label_AT_2->setAlignment(Qt::AlignCenter);

        gridLayout_6->addWidget(label_AT_2, 0, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(-1, 0, -1, -1);
        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_6);

        btn_at = new QPushButton(frame_3);
        btn_at->setObjectName(QString::fromUtf8("btn_at"));
        btn_at->setEnabled(true);
        btn_at->setMinimumSize(QSize(35, 35));
        btn_at->setMaximumSize(QSize(35, 35));
        btn_at->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/SubmitAT_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/SubmitAT_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/SubmitAT_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/SubmitAT_no1x.png);\n"
"}"));

        horizontalLayout_5->addWidget(btn_at);

        btn_paus = new QPushButton(frame_3);
        btn_paus->setObjectName(QString::fromUtf8("btn_paus"));
        btn_paus->setEnabled(true);
        btn_paus->setMinimumSize(QSize(35, 35));
        btn_paus->setMaximumSize(QSize(35, 35));
        btn_paus->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	image: url(:/new/button/skinbutton/CancelAT_nor1x.png);\n"
"	border:0px solid;\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"	image: url(:/new/button/skinbutton/CancelAT_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	image: url(:/new/button/skinbutton/CancelAT_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	image: url(:/new/button/skinbutton/CancelAT_no1x.png);\n"
"}"));

        horizontalLayout_5->addWidget(btn_paus);

        btn_rec = new QPushButton(frame_3);
        btn_rec->setObjectName(QString::fromUtf8("btn_rec"));
        btn_rec->setEnabled(true);
        btn_rec->setMinimumSize(QSize(35, 35));
        btn_rec->setMaximumSize(QSize(36, 36));
        btn_rec->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	border-image: url(:/new/button/skinbutton/ResubmitAT_nor1x.png);\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/ResubmitAT_honor1x.png);\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/ResubmitAT_press1x.png);\n"
"}\n"
"\n"
"QPushButton:disabled \n"
"{\n"
"\n"
"	border-image: url(:/new/button/skinbutton/ResubmitAT_no1x.png);\n"
"}"));

        horizontalLayout_5->addWidget(btn_rec);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_7);


        gridLayout_6->addLayout(horizontalLayout_5, 1, 0, 1, 1);

        gridLayout_6->setRowStretch(0, 10);
        gridLayout_6->setRowStretch(1, 30);

        gridLayout_15->addWidget(widget_2, 1, 0, 1, 1);

        widget_3 = new QWidget(CBlockWgtCN);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        widget_3->setMinimumSize(QSize(0, 25));
        widget_3->setMaximumSize(QSize(16777215, 25));
        widget_3->setStyleSheet(QString::fromUtf8("background-color: #3C3D3F;"));
        gridLayout_14 = new QGridLayout(widget_3);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        gridLayout_14->setContentsMargins(4, 0, 0, 0);
        gridLayout_10 = new QGridLayout();
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_AddData = new QLabel(widget_3);
        label_AddData->setObjectName(QString::fromUtf8("label_AddData"));
        label_AddData->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_AddData->sizePolicy().hasHeightForWidth());
        label_AddData->setSizePolicy(sizePolicy);
        label_AddData->setMinimumSize(QSize(120, 20));
        label_AddData->setMaximumSize(QSize(120, 20));
        label_AddData->setFont(font);
        label_AddData->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"	image: url(:/new/prefix1/skin/adddatapho.png);\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QLabel:disabled\n"
"{\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"	image: url(:/new/prefix1/skin/adddatapho_no.png);\n"
"}\n"
""));
        label_AddData->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_AddData);

        label_AT = new QLabel(widget_3);
        label_AT->setObjectName(QString::fromUtf8("label_AT"));
        label_AT->setEnabled(true);
        label_AT->setMinimumSize(QSize(120, 20));
        label_AT->setMaximumSize(QSize(120, 20));
        label_AT->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"	image: url(:/new/prefix1/skin/adddatapho.png);\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QLabel:disabled\n"
"{\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"	image: url(:/new/prefix1/skin/adddatapho_no.png);\n"
"}\n"
"\n"
""));
        label_AT->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_AT);

        label_Reconstruction = new QLabel(widget_3);
        label_Reconstruction->setObjectName(QString::fromUtf8("label_Reconstruction"));
        label_Reconstruction->setMinimumSize(QSize(120, 20));
        label_Reconstruction->setMaximumSize(QSize(120, 20));
        label_Reconstruction->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"	image: url(:/new/prefix1/skin/adddatapho.png);\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QLabel:disabled\n"
"{\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"	image: url(:/new/prefix1/skin/adddatapho_no.png);\n"
"}"));
        label_Reconstruction->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_Reconstruction);

        label_Production = new QLabel(widget_3);
        label_Production->setObjectName(QString::fromUtf8("label_Production"));
        label_Production->setMinimumSize(QSize(120, 20));
        label_Production->setMaximumSize(QSize(120, 20));
        label_Production->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"	image: url(:/new/prefix1/skin/adddatapho.png);\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QLabel:disabled\n"
"{\n"
"	font: 14px \"Arial\";\n"
"	color: rgb(255, 255, 255);\n"
"	image: url(:/new/prefix1/skin/adddatapho_no.png);\n"
"}"));
        label_Production->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_Production);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_10);


        gridLayout_10->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        gridLayout_10->setColumnStretch(0, 1);

        gridLayout_14->addLayout(gridLayout_10, 0, 0, 1, 1);


        gridLayout_15->addWidget(widget_3, 0, 0, 1, 1);

        tabWidget = new QTabWidget(CBlockWgtCN);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setEnabled(true);
        tabWidget->setFocusPolicy(Qt::ClickFocus);
        tabWidget->setStyleSheet(QString::fromUtf8("QTabWidget::pane { /* The tab widget frame */\n"
"      border-top: 1px solid #222222;\n"
"	 \n"
"  }\n"
"\n"
"  QTabWidget::tab-bar {\n"
"      left: 4px; /* move to the right by 5px */\n"
"	 \n"
"  }\n"
"\n"
"  /* Style the tab using the tab sub-control. Note that\n"
"      it reads QTabBar _not_ QTabWidget */\n"
"  QTabBar::tab {\n"
"      background: #000000;\n"
"      border: 0px solid #C4C4C3;\n"
"      border-bottom-color: #222222; /* same as the pane color */\n"
"      border-top-left-radius: 3px;\n"
"      border-top-right-radius: 3px;\n"
"      min-width: 150px;\n"
"	  min-heith: 50px;\n"
"      padding: 6px;\n"
"  }\n"
"\n"
"  QTabBar::tab:selected, QTabBar::tab:hover {\n"
"      background: #333333;\n"
"  }\n"
"\n"
"  QTabBar::tab:selected {\n"
"      border-color: #9B9B9B;\n"
"      border-bottom-color: #C2C7CB; /* same as pane color */\n"
"  }\n"
"\n"
"  QTabBar::tab:!selected {\n"
"      margin-top: 2px; /* make non-selected tabs look smaller */\n"
"  }"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        tab->setFocusPolicy(Qt::ClickFocus);
        tab->setAutoFillBackground(false);
        tab->setStyleSheet(QString::fromUtf8(""));
        gridLayout = new QGridLayout(tab);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, -1, -1);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(5);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        tableView_photogroup = new MoPhotoTableWidget(tab);
        tableView_photogroup->setObjectName(QString::fromUtf8("tableView_photogroup"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tableView_photogroup->sizePolicy().hasHeightForWidth());
        tableView_photogroup->setSizePolicy(sizePolicy1);
        tableView_photogroup->setMinimumSize(QSize(0, 0));
        tableView_photogroup->setMaximumSize(QSize(16777215, 11111111));
        tableView_photogroup->setStyleSheet(QString::fromUtf8("background-color: rgb(51, 51, 51);\n"
""));
        tableView_photogroup->setFrameShape(QFrame::NoFrame);
        tableView_photogroup->setLineWidth(1);
        tableView_photogroup->setMidLineWidth(1);
        tableView_photogroup->verticalHeader()->setVisible(false);

        verticalLayout_3->addWidget(tableView_photogroup);


        verticalLayout_2->addLayout(verticalLayout_3);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        tableView_photo_pos = new MoPhotoTableWidget(tab);
        tableView_photo_pos->setObjectName(QString::fromUtf8("tableView_photo_pos"));
        sizePolicy1.setHeightForWidth(tableView_photo_pos->sizePolicy().hasHeightForWidth());
        tableView_photo_pos->setSizePolicy(sizePolicy1);
        tableView_photo_pos->setMidLineWidth(1);
        tableView_photo_pos->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        tableView_photo_pos->setEditTriggers(QAbstractItemView::NoEditTriggers);

        verticalLayout_10->addWidget(tableView_photo_pos);


        verticalLayout_2->addLayout(verticalLayout_10);


        horizontalLayout->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, 10, 20);
        widget = new QWidget(tab);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setMinimumSize(QSize(500, 0));
        widget->setMaximumSize(QSize(500, 16777215));
        widget->setFocusPolicy(Qt::ClickFocus);
        widget->setStyleSheet(QString::fromUtf8("border-left:4px solid  #000000;\n"
"background-color: rgb(34, 34, 34);"));
        gridLayout_12 = new QGridLayout(widget);
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        gridLayout_12->setHorizontalSpacing(0);
        gridLayout_12->setContentsMargins(2, 0, 0, 0);
        wgt_photogroup_info_ = new QWidget(widget);
        wgt_photogroup_info_->setObjectName(QString::fromUtf8("wgt_photogroup_info_"));
        wgt_photogroup_info_->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_8 = new QVBoxLayout(wgt_photogroup_info_);
        verticalLayout_8->setSpacing(0);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        wgt_form_photogroup = new QWidget(wgt_photogroup_info_);
        wgt_form_photogroup->setObjectName(QString::fromUtf8("wgt_form_photogroup"));
        wgt_form_photogroup->setEnabled(true);
        wgt_form_photogroup->setStyleSheet(QString::fromUtf8("QWidget\n"
"{\n"
"background-color: #222222;\n"
"border:0px solid;\n"
"}\n"
"\n"
"QLineEdit\n"
"{\n"
"background-color:#131313;\n"
"border:1px solid;\n"
"border-color: #A5A5A5;\n"
"}\n"
""));
        gridLayout_5 = new QGridLayout(wgt_form_photogroup);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setHorizontalSpacing(15);
        gridLayout_5->setVerticalSpacing(30);
        gridLayout_5->setContentsMargins(5, 0, 10, 10);
        le_photogroup_sensorsize = new QLineEdit(wgt_form_photogroup);
        le_photogroup_sensorsize->setObjectName(QString::fromUtf8("le_photogroup_sensorsize"));
        le_photogroup_sensorsize->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_sensorsize->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_sensorsize, 7, 3, 1, 1);

        label_27 = new QLabel(wgt_form_photogroup);
        label_27->setObjectName(QString::fromUtf8("label_27"));

        gridLayout_5->addWidget(label_27, 5, 0, 1, 2);

        le_photogroup_name = new QLineEdit(wgt_form_photogroup);
        le_photogroup_name->setObjectName(QString::fromUtf8("le_photogroup_name"));
        le_photogroup_name->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_name->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_name, 1, 3, 1, 1);

        le_k3_2 = new QLineEdit(wgt_form_photogroup);
        le_k3_2->setObjectName(QString::fromUtf8("le_k3_2"));
        le_k3_2->setStyleSheet(QString::fromUtf8(""));
        le_k3_2->setReadOnly(false);

        gridLayout_5->addWidget(le_k3_2, 11, 2, 1, 2);

        le_photogroup_dir = new QLineEdit(wgt_form_photogroup);
        le_photogroup_dir->setObjectName(QString::fromUtf8("le_photogroup_dir"));
        le_photogroup_dir->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_dir->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_dir, 2, 3, 1, 1);

        label_focalength = new QLabel(wgt_form_photogroup);
        label_focalength->setObjectName(QString::fromUtf8("label_focalength"));

        gridLayout_5->addWidget(label_focalength, 8, 0, 1, 2);

        k3Label_2 = new QLabel(wgt_form_photogroup);
        k3Label_2->setObjectName(QString::fromUtf8("k3Label_2"));
        k3Label_2->setMinimumSize(QSize(60, 0));
        k3Label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(k3Label_2, 13, 1, 1, 1);

        label_18 = new QLabel(wgt_form_photogroup);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        gridLayout_5->addWidget(label_18, 3, 0, 1, 3);

        label_28 = new QLabel(wgt_form_photogroup);
        label_28->setObjectName(QString::fromUtf8("label_28"));

        gridLayout_5->addWidget(label_28, 7, 0, 1, 2);

        le_p2_2 = new QLineEdit(wgt_form_photogroup);
        le_p2_2->setObjectName(QString::fromUtf8("le_p2_2"));
        le_p2_2->setStyleSheet(QString::fromUtf8(""));
        le_p2_2->setReadOnly(false);

        gridLayout_5->addWidget(le_p2_2, 13, 2, 1, 2);

        label_16 = new QLabel(wgt_form_photogroup);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout_5->addWidget(label_16, 2, 0, 1, 1);

        label_30 = new QLabel(wgt_form_photogroup);
        label_30->setObjectName(QString::fromUtf8("label_30"));

        gridLayout_5->addWidget(label_30, 9, 0, 1, 1);

        le_k2_2 = new QLineEdit(wgt_form_photogroup);
        le_k2_2->setObjectName(QString::fromUtf8("le_k2_2"));
        le_k2_2->setStyleSheet(QString::fromUtf8(""));
        le_k2_2->setReadOnly(false);

        gridLayout_5->addWidget(le_k2_2, 10, 2, 1, 2);

        label_14 = new QLabel(wgt_form_photogroup);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_5->addWidget(label_14, 1, 0, 1, 1);

        le_photogroup_imagesize = new QLineEdit(wgt_form_photogroup);
        le_photogroup_imagesize->setObjectName(QString::fromUtf8("le_photogroup_imagesize"));
        le_photogroup_imagesize->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_imagesize->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_imagesize, 5, 3, 1, 1);

        le_k1_2 = new QLineEdit(wgt_form_photogroup);
        le_k1_2->setObjectName(QString::fromUtf8("le_k1_2"));
        le_k1_2->setStyleSheet(QString::fromUtf8(""));
        le_k1_2->setReadOnly(false);

        gridLayout_5->addWidget(le_k1_2, 9, 2, 1, 2);

        label_26 = new QLabel(wgt_form_photogroup);
        label_26->setObjectName(QString::fromUtf8("label_26"));

        gridLayout_5->addWidget(label_26, 6, 0, 1, 1);

        le_photogroup_camera = new QLineEdit(wgt_form_photogroup);
        le_photogroup_camera->setObjectName(QString::fromUtf8("le_photogroup_camera"));
        le_photogroup_camera->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_camera->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_camera, 6, 3, 1, 1);

        label_4 = new QLabel(wgt_form_photogroup);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(500, 28));
        label_4->setMaximumSize(QSize(500, 28));
        label_4->setStyleSheet(QString::fromUtf8("color:#A5A5A5;\n"
"background-color: rgb(51, 51, 51);"));
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_4, 0, 0, 1, 4);

        le_photogroup_num = new QLineEdit(wgt_form_photogroup);
        le_photogroup_num->setObjectName(QString::fromUtf8("le_photogroup_num"));
        le_photogroup_num->setStyleSheet(QString::fromUtf8(""));
        le_photogroup_num->setReadOnly(false);

        gridLayout_5->addWidget(le_photogroup_num, 3, 3, 1, 1);

        label_9 = new QLabel(wgt_form_photogroup);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setMinimumSize(QSize(60, 0));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_9, 9, 1, 1, 1);

        label_32 = new QLabel(wgt_form_photogroup);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setMinimumSize(QSize(60, 0));
        label_32->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_32, 11, 1, 1, 1);

        le_focalength = new QLineEdit(wgt_form_photogroup);
        le_focalength->setObjectName(QString::fromUtf8("le_focalength"));
        le_focalength->setStyleSheet(QString::fromUtf8(""));
        le_focalength->setReadOnly(false);

        gridLayout_5->addWidget(le_focalength, 8, 3, 1, 1);

        label_31 = new QLabel(wgt_form_photogroup);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        label_31->setMinimumSize(QSize(60, 0));
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_31, 10, 1, 1, 1);

        label_33 = new QLabel(wgt_form_photogroup);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        label_33->setMinimumSize(QSize(60, 0));
        label_33->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_33, 12, 1, 1, 1);

        le_p1_2 = new QLineEdit(wgt_form_photogroup);
        le_p1_2->setObjectName(QString::fromUtf8("le_p1_2"));
        le_p1_2->setStyleSheet(QString::fromUtf8(""));
        le_p1_2->setReadOnly(false);

        gridLayout_5->addWidget(le_p1_2, 12, 2, 1, 2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_5->addItem(verticalSpacer_3, 14, 1, 1, 1);


        verticalLayout_8->addWidget(wgt_form_photogroup);


        gridLayout_12->addWidget(wgt_photogroup_info_, 1, 0, 1, 1);

        wgt_photo_priview = new QWidget(widget);
        wgt_photo_priview->setObjectName(QString::fromUtf8("wgt_photo_priview"));
        wgt_photo_priview->setStyleSheet(QString::fromUtf8("border:0px solid;"));
        gridLayout_4 = new QGridLayout(wgt_photo_priview);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setVerticalSpacing(15);
        gridLayout_4->setContentsMargins(5, 0, 0, -1);
        label_Group = new QLabel(wgt_photo_priview);
        label_Group->setObjectName(QString::fromUtf8("label_Group"));
        label_Group->setMinimumSize(QSize(500, 28));
        label_Group->setMaximumSize(QSize(500, 28));
        label_Group->setStyleSheet(QString::fromUtf8("color:#A5A5A5;\n"
"background-color:#333333;\n"
""));
        label_Group->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_Group, 0, 0, 1, 4);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(20, -1, 15, 20);
        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_5->addItem(verticalSpacer_7);


        gridLayout_4->addLayout(verticalLayout_5, 1, 0, 1, 1);

        graphics_view_photo = new QGraphicsView(wgt_photo_priview);
        graphics_view_photo->setObjectName(QString::fromUtf8("graphics_view_photo"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(graphics_view_photo->sizePolicy().hasHeightForWidth());
        graphics_view_photo->setSizePolicy(sizePolicy2);
        graphics_view_photo->setMinimumSize(QSize(260, 150));
        graphics_view_photo->setMaximumSize(QSize(260, 150));

        gridLayout_4->addWidget(graphics_view_photo, 1, 1, 1, 2);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(20, -1, 15, 20);
        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_5);

        label_photo_open = new QLabel(wgt_photo_priview);
        label_photo_open->setObjectName(QString::fromUtf8("label_photo_open"));
        label_photo_open->setMinimumSize(QSize(70, 28));
        label_photo_open->setMaximumSize(QSize(70, 28));
        label_photo_open->setStyleSheet(QString::fromUtf8(""));
        label_photo_open->setAlignment(Qt::AlignCenter);
        label_photo_open->setOpenExternalLinks(false);

        verticalLayout_4->addWidget(label_photo_open);

        verticalLayout_4->setStretch(1, 1);

        gridLayout_4->addLayout(verticalLayout_4, 1, 3, 1, 1);

        verticalSpacer_6 = new QSpacerItem(20, 68, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_4->addItem(verticalSpacer_6, 3, 2, 1, 1);

        widget_normal = new QWidget(wgt_photo_priview);
        widget_normal->setObjectName(QString::fromUtf8("widget_normal"));
        widget_normal->setEnabled(true);
        sizePolicy.setHeightForWidth(widget_normal->sizePolicy().hasHeightForWidth());
        widget_normal->setSizePolicy(sizePolicy);
        widget_normal->setMouseTracking(true);
        widget_normal->setStyleSheet(QString::fromUtf8("QWidget\n"
"{\n"
"background-color: #222222;\n"
"}\n"
"\n"
"QWidget#widget_normal\n"
"{\n"
"	border-top:1px solid #404040;\n"
"}\n"
"\n"
"\n"
"\n"
"QLineEdit\n"
"{\n"
"background-color:#131313;\n"
"border:1px solid;\n"
"border-color: #A5A5A5;\n"
"}\n"
"\n"
"QComboBox\n"
"{\n"
"background-color:#131313;\n"
"border:1px solid;\n"
"border-color: #A5A5A5;\n"
"}"));
        formLayout = new QFormLayout(widget_normal);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setLabelAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        formLayout->setHorizontalSpacing(30);
        formLayout->setVerticalSpacing(30);
        formLayout->setContentsMargins(-1, 30, -1, 30);
        label_2 = new QLabel(widget_normal);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy);
        label_2->setMinimumSize(QSize(60, 0));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(0, QFormLayout::LabelRole, label_2);

        le_name = new QLineEdit(widget_normal);
        le_name->setObjectName(QString::fromUtf8("le_name"));
        le_name->setEnabled(true);
        le_name->setTabletTracking(true);
        le_name->setFocusPolicy(Qt::ClickFocus);
        le_name->setStyleSheet(QString::fromUtf8(""));
        le_name->setReadOnly(false);

        formLayout->setWidget(0, QFormLayout::FieldRole, le_name);

        label_3 = new QLabel(widget_normal);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(60, 0));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout->setWidget(1, QFormLayout::LabelRole, label_3);

        le_path = new QLineEdit(widget_normal);
        le_path->setObjectName(QString::fromUtf8("le_path"));
        le_path->setStyleSheet(QString::fromUtf8(""));
        le_path->setReadOnly(false);

        formLayout->setWidget(1, QFormLayout::FieldRole, le_path);

        label_20 = new QLabel(widget_normal);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_20);

        le_photo_ser_siz = new QLineEdit(widget_normal);
        le_photo_ser_siz->setObjectName(QString::fromUtf8("le_photo_ser_siz"));
        le_photo_ser_siz->setStyleSheet(QString::fromUtf8(""));
        le_photo_ser_siz->setReadOnly(false);

        formLayout->setWidget(2, QFormLayout::FieldRole, le_photo_ser_siz);

        label_22 = new QLabel(widget_normal);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy3);
        label_22->setMinimumSize(QSize(0, 30));
        label_22->setStyleSheet(QString::fromUtf8("background-color: #222222;\n"
"border:0px solid;"));
        label_22->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        formLayout->setWidget(3, QFormLayout::LabelRole, label_22);

        label_23 = new QLabel(widget_normal);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_23);

        le_pos_lon = new QLineEdit(widget_normal);
        le_pos_lon->setObjectName(QString::fromUtf8("le_pos_lon"));
        le_pos_lon->setStyleSheet(QString::fromUtf8(""));
        le_pos_lon->setReadOnly(false);

        formLayout->setWidget(4, QFormLayout::FieldRole, le_pos_lon);

        label_24 = new QLabel(widget_normal);
        label_24->setObjectName(QString::fromUtf8("label_24"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_24);

        le_pos_lat = new QLineEdit(widget_normal);
        le_pos_lat->setObjectName(QString::fromUtf8("le_pos_lat"));
        le_pos_lat->setStyleSheet(QString::fromUtf8(""));
        le_pos_lat->setReadOnly(false);

        formLayout->setWidget(5, QFormLayout::FieldRole, le_pos_lat);

        label_25 = new QLabel(widget_normal);
        label_25->setObjectName(QString::fromUtf8("label_25"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_25);

        le_pos_height = new QLineEdit(widget_normal);
        le_pos_height->setObjectName(QString::fromUtf8("le_pos_height"));
        le_pos_height->setStyleSheet(QString::fromUtf8(""));
        le_pos_height->setReadOnly(false);

        formLayout->setWidget(6, QFormLayout::FieldRole, le_pos_height);

        comboBox = new QComboBox(widget_normal);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setMinimumSize(QSize(0, 20));
        comboBox->setMaximumSize(QSize(16777215, 20));
        comboBox->setStyleSheet(QString::fromUtf8("font: 12px \"Arial\";"));

        formLayout->setWidget(3, QFormLayout::FieldRole, comboBox);

        le_name->raise();
        label_2->raise();
        label_3->raise();
        le_path->raise();
        label_20->raise();
        le_photo_ser_siz->raise();
        label_22->raise();
        label_23->raise();
        le_pos_lon->raise();
        label_24->raise();
        le_pos_lat->raise();
        label_25->raise();
        le_pos_height->raise();
        comboBox->raise();

        gridLayout_4->addWidget(widget_normal, 2, 0, 1, 4);


        gridLayout_12->addWidget(wgt_photo_priview, 0, 0, 1, 1);


        verticalLayout->addWidget(widget);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_4);

        verticalLayout->setStretch(0, 10);

        horizontalLayout->addLayout(verticalLayout);

        horizontalLayout->setStretch(0, 25);
        horizontalLayout->setStretch(1, 10);

        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        gridLayout_3 = new QGridLayout(tab_4);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(15);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        progressBar_submit = new QProgressBar(tab_4);
        progressBar_submit->setObjectName(QString::fromUtf8("progressBar_submit"));
        progressBar_submit->setStyleSheet(QString::fromUtf8(""));
        progressBar_submit->setValue(0);
        progressBar_submit->setTextVisible(false);

        verticalLayout_6->addWidget(progressBar_submit);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_cancleLog = new QLabel(tab_4);
        label_cancleLog->setObjectName(QString::fromUtf8("label_cancleLog"));

        gridLayout_2->addWidget(label_cancleLog, 3, 0, 1, 1);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_7 = new QLabel(tab_4);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_14->addWidget(label_7);

        label_blockID = new QLabel(tab_4);
        label_blockID->setObjectName(QString::fromUtf8("label_blockID"));

        horizontalLayout_14->addWidget(label_blockID);


        gridLayout_2->addLayout(horizontalLayout_14, 3, 2, 1, 1);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        label = new QLabel(tab_4);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_9->addWidget(label);

        label_complete_time = new QLabel(tab_4);
        label_complete_time->setObjectName(QString::fromUtf8("label_complete_time"));

        horizontalLayout_9->addWidget(label_complete_time);


        gridLayout_2->addLayout(horizontalLayout_9, 1, 2, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 5, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_Status = new QLabel(tab_4);
        label_Status->setObjectName(QString::fromUtf8("label_Status"));

        horizontalLayout_7->addWidget(label_Status);

        label_StatusValue = new QLabel(tab_4);
        label_StatusValue->setObjectName(QString::fromUtf8("label_StatusValue"));
        label_StatusValue->setMinimumSize(QSize(40, 0));
        label_StatusValue->setMidLineWidth(20);

        horizontalLayout_7->addWidget(label_StatusValue);


        gridLayout_2->addLayout(horizontalLayout_7, 1, 0, 2, 1);

        label_view_report = new QLabel(tab_4);
        label_view_report->setObjectName(QString::fromUtf8("label_view_report"));

        gridLayout_2->addWidget(label_view_report, 4, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 1, 1, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_Progress = new QLabel(tab_4);
        label_Progress->setObjectName(QString::fromUtf8("label_Progress"));
        label_Progress->setStyleSheet(QString::fromUtf8("color: rgb(122, 237, 171);"));

        horizontalLayout_6->addWidget(label_Progress);

        label_ProgressValue = new QLabel(tab_4);
        label_ProgressValue->setObjectName(QString::fromUtf8("label_ProgressValue"));
        label_ProgressValue->setStyleSheet(QString::fromUtf8("color: rgb(122, 237, 171);"));

        horizontalLayout_6->addWidget(label_ProgressValue);


        gridLayout_2->addLayout(horizontalLayout_6, 0, 0, 1, 1);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_11 = new QLabel(tab_4);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_8->addWidget(label_11);

        label_create_time = new QLabel(tab_4);
        label_create_time->setObjectName(QString::fromUtf8("label_create_time"));

        horizontalLayout_8->addWidget(label_create_time);


        gridLayout_2->addLayout(horizontalLayout_8, 0, 2, 1, 1);


        verticalLayout_6->addLayout(gridLayout_2);

        taskList = new QTableWidget(tab_4);
        taskList->setObjectName(QString::fromUtf8("taskList"));
        sizePolicy1.setHeightForWidth(taskList->sizePolicy().hasHeightForWidth());
        taskList->setSizePolicy(sizePolicy1);
        taskList->setMinimumSize(QSize(200, 100));
        taskList->setStyleSheet(QString::fromUtf8("QHeaderView                    \n"
"{\n"
"    background:#333333; \n"
"    color:#A5A5A5;\n"
"	font: 14px \"Arial\";        \n"
"}\n"
"\n"
"QHeaderView::section           \n"
"{\n"
"    \n"
"	font: 14px \"Arial\";\n"
"                  \n"
"    background:#333333;           \n"
"    border:none;                  \n"
"    text-align:left;               \n"
" \n"
"    min-height:64px;               \n"
"    max-height:64px;               \n"
" \n"
"    margin-left:0px;               \n"
"    padding-left:0px;              \n"
"}\n"
" \n"
"\n"
"QTableWidget                   \n"
"{\n"
"    background:#000000;           \n"
"    border:none;                 \n"
" \n"
"   font: 14px \"Arial\";\n"
"   color:#FFFFFF;                \n"
"}\n"
" \n"
"\n"
"QTableWidget::item               \n"
"{\n"
"    border-bottom:1px solid #383838 ; \n"
"}\n"
" \n"
"\n"
"\n"
"QScrollBar::handle:vertical       \n"
"{\n"
"    background: rgba(255,255,255,20%); \n"
"    border: 0px solid grey;            \n"
"    border-radius:3px;    "
                        "             \n"
"    width: 8px;                        \n"
"}\n"
"\n"
"QScrollBar::vertical                   \n"
"{\n"
"    border-width:1px;                       \n"
"    border-style: solid;                    \n"
"    border-color: rgba(255, 255, 255, 10%);\n"
"    width: 8px;                             \n"
"    margin:0px 0px 0px 0px;                \n"
"    border-radius:3px;                      \n"
"}\n"
"\n"
"\n"
"QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical \n"
"{\n"
"    background:rgba(255,255,255,10%);                           \n"
"}\n"
"\n"
"QScollBar::add-line:vertical, QScrollBar::sub-line:vertical  \n"
"{\n"
"    background:transparent;                                      \n"
"}\n"
""));

        verticalLayout_6->addWidget(taskList);

        verticalLayout_6->setStretch(0, 1);
        verticalLayout_6->setStretch(1, 3);
        verticalLayout_6->setStretch(2, 10);

        gridLayout_3->addLayout(verticalLayout_6, 0, 0, 1, 1);

        btn_newContruction = new QPushButton(tab_4);
        btn_newContruction->setObjectName(QString::fromUtf8("btn_newContruction"));
        btn_newContruction->setFlat(false);

        gridLayout_3->addWidget(btn_newContruction, 1, 0, 1, 1, Qt::AlignRight);

        tabWidget->addTab(tab_4, QString());

        gridLayout_15->addWidget(tabWidget, 2, 0, 1, 1);


        retranslateUi(CBlockWgtCN);

        tabWidget->setCurrentIndex(0);
        btn_newContruction->setDefault(false);


        QMetaObject::connectSlotsByName(CBlockWgtCN);
    } // setupUi

    void retranslateUi(QWidget *CBlockWgtCN)
    {
        CBlockWgtCN->setWindowTitle(QApplication::translate("CBlockWgtCN", "CBlockWgt", nullptr));
        btn_addsig->setText(QString());
        btn_adddir->setText(QString());
        btn_push_removePgtable->setText(QString());
        label_Pho->setText(QApplication::translate("CBlockWgtCN", "Photos", nullptr));
        btn_addpos->setText(QString());
        btn_delpos->setText(QString());
        label_Pos->setText(QApplication::translate("CBlockWgtCN", "POS", nullptr));
        label_5->setText(QApplication::translate("CBlockWgtCN", "GCP", nullptr));
        btn_Siggcp->setText(QString());
        btn_addgcp->setText(QString());
        btn_delgcp->setText(QString());
        label_AT_2->setText(QApplication::translate("CBlockWgtCN", "AT", nullptr));
        btn_at->setText(QString());
        btn_paus->setText(QString());
        btn_rec->setText(QString());
        label_AddData->setText(QApplication::translate("CBlockWgtCN", "1.Add data", nullptr));
        label_AT->setText(QApplication::translate("CBlockWgtCN", "2.AT", nullptr));
        label_Reconstruction->setText(QApplication::translate("CBlockWgtCN", "3.Reconstruction", nullptr));
        label_Production->setText(QApplication::translate("CBlockWgtCN", "4.Production", nullptr));
        label_27->setText(QApplication::translate("CBlockWgtCN", "\345\275\261\345\203\217\345\260\272\345\257\270", nullptr));
        label_focalength->setText(QApplication::translate("CBlockWgtCN", "\347\204\246\350\267\235\351\225\277\345\272\246(mm)", nullptr));
        k3Label_2->setText(QApplication::translate("CBlockWgtCN", "P2", nullptr));
        label_18->setText(QApplication::translate("CBlockWgtCN", "\345\275\261\345\203\217\346\225\260", nullptr));
        label_28->setText(QApplication::translate("CBlockWgtCN", "\346\204\237\345\272\224\345\231\250\345\260\272\345\257\270(mm)", nullptr));
        label_16->setText(QApplication::translate("CBlockWgtCN", "\347\233\256\345\275\225", nullptr));
        label_30->setText(QApplication::translate("CBlockWgtCN", "\347\225\270\345\217\230", nullptr));
        label_14->setText(QApplication::translate("CBlockWgtCN", "\345\220\215\347\247\260", nullptr));
        label_26->setText(QApplication::translate("CBlockWgtCN", "\347\233\270\346\234\272", nullptr));
        label_4->setText(QApplication::translate("CBlockWgtCN", "Photogroup", nullptr));
        label_9->setText(QApplication::translate("CBlockWgtCN", "K1", nullptr));
        label_32->setText(QApplication::translate("CBlockWgtCN", "K3", nullptr));
        label_31->setText(QApplication::translate("CBlockWgtCN", "K2", nullptr));
        label_33->setText(QApplication::translate("CBlockWgtCN", "P1", nullptr));
        label_Group->setText(QApplication::translate("CBlockWgtCN", "Photo detail", nullptr));
        label_photo_open->setText(QApplication::translate("CBlockWgtCN", "<html><head / ><body><p><a href = \" \"><span style=\" text-decoration: underline; color:#ffffff;\">Open</span></a></p></body></html>", nullptr));
        label_2->setText(QApplication::translate("CBlockWgtCN", "\345\220\215\347\247\260", nullptr));
        label_3->setText(QApplication::translate("CBlockWgtCN", "\347\233\256\345\275\225", nullptr));
        label_20->setText(QApplication::translate("CBlockWgtCN", "\345\260\272\345\257\270 (M)", nullptr));
        label_22->setText(QApplication::translate("CBlockWgtCN", "SRS", nullptr));
        label_23->setText(QApplication::translate("CBlockWgtCN", "\347\273\217\345\272\246", nullptr));
        label_24->setText(QApplication::translate("CBlockWgtCN", "\347\272\254\345\272\246", nullptr));
        label_25->setText(QApplication::translate("CBlockWgtCN", "\351\253\230\345\272\246", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("CBlockWgtCN", "Photos", nullptr));
        label_cancleLog->setText(QString());
        label_7->setText(QApplication::translate("CBlockWgtCN", "Block ID:", nullptr));
        label_blockID->setText(QApplication::translate("CBlockWgtCN", "-------/-------", nullptr));
        label->setText(QApplication::translate("CBlockWgtCN", "Complete time:", nullptr));
        label_complete_time->setText(QApplication::translate("CBlockWgtCN", "---/----", nullptr));
        label_Status->setText(QApplication::translate("CBlockWgtCN", "AT Stage:", nullptr));
        label_StatusValue->setText(QString());
        label_view_report->setText(QApplication::translate("CBlockWgtCN", "<html><head/><body><p><a href=\" \"><span style=\" color:#222222;\">View AT Repor</span></a><a href=\" \"><span style=\" text-decoration: underline; color:#222222;\">t</span></a></p></body></html>", nullptr));
        label_Progress->setText(QApplication::translate("CBlockWgtCN", "Progress:", nullptr));
        label_ProgressValue->setText(QString());
        label_11->setText(QApplication::translate("CBlockWgtCN", "Submit time:", nullptr));
        label_create_time->setText(QApplication::translate("CBlockWgtCN", "-----/-----", nullptr));
        btn_newContruction->setText(QApplication::translate("CBlockWgtCN", "New reconstruction", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("CBlockWgtCN", "AT", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CBlockWgtCN2: public Ui_CBlockWgtCN2 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BLOCKWGTCN_H
