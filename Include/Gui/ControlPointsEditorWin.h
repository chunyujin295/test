/**
  * @file      ControlPointsEditorWin.h
  * @brief     刺点对应界面类
  * @details
  * @author 
  * @attention
  */
#ifndef _AI3D_GUI_CONTROLPOINTSEDITORWIN_H_
#define _AI3D_GUI_CONTROLPOINTSEDITORWIN_H_

#include <QWidget>
#include <QStandardItemModel>
#include <QFutureWatcher>
#include <QItemDelegate>
#include <QButtonGroup>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <omp.h>
#include <QMainWindow>
#include <QRunnable>

#include "Core/BlockObject.h"
#include "Core/ATData.h"
#include "Core/Types.h"

#include "Gui/GraphicsView.h"
#include "Gui/GlobalStruct.h"
#include "Gui/MoWidget.h"


#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollBar>


#define COLOFFSET 3 //可放在global


const int RED_COLOR_R = 56;
const int RED_COLOR_G = 30;
const int RED_COLOR_B = 30;

const int YELLOW_COLOR_R = 56;
const int YELLOW_COLOR_G = 55;
const int YELLOW_COLOR_B = 30;

const int GREEN_COLOR_R = 46;
const int GREEN_COLOR_G = 74;
const int GREEN_COLOR_B = 91;


const int  SELECT_RED_COLOR_R = 56;
const int  SELECT_RED_COLOR_G = 30;
const int  SELECT_RED_COLOR_B = 30;

const int  SELECT_YELLOW_COLOR_R = 56;
const int  SELECT_YELLOW_COLOR_G = 55;
const int  SELECT_YELLOW_COLOR_B = 30;

const int  SELECT_GREEN_COLOR_R = 46;
const int  SELECT_GREEN_COLOR_G = 74;
const int  SELECT_GREEN_COLOR_B = 91;

const int DEFAULT_LIST_COLOR_R = 18;
const int DEFAULT_LIST_COLOR_G = 18;
const int DEFAULT_LIST_COLOR_B = 18;

//imageids_forgenpreview_ 缩略图相关
namespace AI3D
{
	namespace GUI
	{
        class Ui_ControlPointsWin2
        {
        public:
            QWidget* centralwidget;
            QGridLayout* gridLayout_7;
            QSplitter* splitter_3;
            QSplitter* splitter_2;
            QWidget* widget_showGcp;
            QGridLayout* gridLayout;
            QHBoxLayout* horizontalLayout_4;
            QLabel* label;
            QSpacerItem* horizontalSpacer_4;
            QLabel* label_2;
            QLabel* toolBtn_gcpstatis;
            QWidget* widget_SelectCoordi;
            QGridLayout* gridLayout_2;
            QGridLayout* gridLayout_3;
            QLabel* label_SRS;
            QComboBox* comboBox_srs;
            QWidget* layoutWidget;
            QVBoxLayout* verticalLayout;
       
            MoTableWidget* gcplistview;

            QWidget* layoutWidget1;
            QVBoxLayout* verticalLayout_2;
            QLabel* label_4;
          
            MoTableWidget* measurementsview;         
            QSplitter* splitter;

            QWidget* widget_showImage;
            QGridLayout* gridLayout_4;
            QHBoxLayout* horizontalLayout;
            QPushButton* btn_AllPho;
            QPushButton* btn_MatchPho;
            QPushButton* btn_MarkPho;
            QSpacerItem* horizontalSpacer;
            
            
            MoListWidget* previewlistview;
            
            QWidget* layoutWidget2;
            QVBoxLayout* verticalLayout_3;
            QWidget* epi_widget;
            QGridLayout* gridLayout_6;
            QHBoxLayout* horizontalLayout_5;
            QPushButton* btn_epipolarline;
            QSpacerItem* horizontalSpacer_5;
            QWidget* measuringview;
            QVBoxLayout* verticalLayout_21;
            //stylesheet 如何填充的
            void setupUi(QMainWindow* ControlPointsWin)
            {
                if (ControlPointsWin->objectName().isEmpty())
                    ControlPointsWin->setObjectName(QString::fromUtf8("ControlPointsWin"));
                ControlPointsWin->resize(1608, 714);
                ControlPointsWin->setStyleSheet(QString::fromUtf8("/* \346\234\252\344\270\213\346\213\211\346\227\266\357\274\214QComboBox\347\232\204\346\240\267\345\274\217 */\n"
                    "QComboBox {\n"
                    "    border: 0px solid gray;   /* \350\276\271\346\241\206 */\n"
                    "    border-radius: 3px;   /* \345\234\206\350\247\222 */\n"
                    "    color: #FFFFFF;\n"
                    "	font: 14px \"Arial\";\n"
                    "    background: #131313;\n"
                    "}\n"
                    "\n"
                    "/* \344\270\213\346\213\211\345\220\216\357\274\214\346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\346\240\267\345\274\217 */\n"
                    "QComboBox QAbstractItemView {\n"
                    "    outline: 0px solid gray;   /* \351\200\211\345\256\232\351\241\271\347\232\204\350\231\232\346\241\206 */\n"
                    "    border: 0px solid;   /* \346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\347\232\204\350\276\271\346\241\206 */\n"
                    "    color:#FFFFFF;\n"
                    "    background-color: #131313;  /* \346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\347\232\204\350\203\214\346\231\257\350\211\262 */\n"
                    "    selection-background-color:#333333;   /"
                    "* \346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\350\242\253\351\200\211\344\270\255\351\241\271\347\232\204\350\203\214\346\231\257\350\211\262 */\n"
                    "}\n"
                    "\n"
                    "/* \344\270\213\346\213\211\345\220\216\357\274\214\346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\346\257\217\351\241\271\347\232\204\346\240\267\345\274\217 */\n"
                    "QComboBox QAbstractItemView::item {\n"
                    "    height: 50px;   /* \351\241\271\347\232\204\351\253\230\345\272\246\357\274\210\350\256\276\347\275\256pComboBox->setView(new QListView());\345\220\216\357\274\214\350\257\245\351\241\271\346\211\215\350\265\267\344\275\234\347\224\250\357\274\211 */\n"
                    "}\n"
                    "\n"
                    "/* \344\270\213\346\213\211\345\220\216\357\274\214\346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\350\266\212\350\277\207\346\257\217\351\241\271\347\232\204\346\240\267\345\274\217 */\n"
                    "QComboBox QAbstractItemView::item:hover {\n"
                    "    color: #FFFFFF;\n"
                    "    background-color: rgb(22,22,22);   /* \346\225"
                    "\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\350\266\212\350\277\207\346\257\217\351\241\271\347\232\204\350\203\214\346\231\257\350\211\262 */\n"
                    "}\n"
                    "\n"
                    "/* \344\270\213\346\213\211\345\220\216\357\274\214\346\225\264\344\270\252\344\270\213\346\213\211\347\252\227\344\275\223\350\242\253\351\200\211\346\213\251\347\232\204\346\257\217\351\241\271\347\232\204\346\240\267\345\274\217 */\n"
                    "QComboBox QAbstractItemView::item:selected {\n"
                    "    color: #FFFFFF;\n"
                    "    background-color:rgb(22,22,22);\n"
                    "}\n"
                    "\n"
                    "/* QComboBox\344\270\255\347\232\204\345\236\202\347\233\264\346\273\232\345\212\250\346\235\241 */\n"
                    "QComboBox QAbstractScrollArea QScrollBar:vertical {\n"
                    "    width: 10px;\n"
                    "    background-color: #d0d2d4;   /* \347\251\272\347\231\275\345\214\272\345\237\237\347\232\204\350\203\214\346\231\257\350\211\262*/\n"
                    "}\n"
                    "\n"
                    "QComboBox QAbstractScrollArea QScrollBar::handle:vertical {\n"
                    "    border-radius: 5px;   /* \345\234\206\350\247\222 */\n"
                    "    background: rgb(160,160"
                    ",160);   /* \345\260\217\346\226\271\345\235\227\347\232\204\350\203\214\346\231\257\350\211\262\346\267\261\347\201\260lightblue */\n"
                    "}\n"
                    "\n"
                    "QComboBox QAbstractScrollArea QScrollBar::handle:vertical:hover {\n"
                    "    background: rgb(90, 91, 93);   /* \350\266\212\350\277\207\345\260\217\346\226\271\345\235\227\347\232\204\350\203\214\346\231\257\350\211\262yellow */\n"
                    "}\n"
                    "\n"
                    "QWidget#centralwidget{\n"
                    "	background-color: rgb(0, 0, 0);\n"
                    "}\n"
                    "\n"
                    "\n"
                    "QSplitter{\n"
                    "border:2px solid;\n"
                    "border-left-color:#000000;\n"
                    "border-right-color:#000000;\n"
                    "border-bottom;none;\n"
                    "border-top:none;\n"
                    "}\n"
                    "QListWidget{\n"
                    "	background-color: rgb(18, 18, 18);\n"
                    "	color: rgb(255, 255, 255);\n"
                    "	font: 14px \"Arial\";\n"
                    "border:none;\n"
                    "}\n"
                    "QWidget#measuringview{\n"
                    "	background-color: rgb(22, 22, 22);\n"
                    "	color: rgb(255, 255, 255);\n"
                    "	font: 14px \"Arial\";\n"
                    "	border:none;\n"
                    "}\n"
                    "\n"
                    "QTableWidg"
                    "et{\n"
                    "	background-color: rgb(22, 22, 22);\n"
                    "	color: rgb(255, 255, 255);\n"
                    "	font: 14px \"Arial\";\n"
                    "}\n"
                    "\n"
                    "QWidget#widget_showGcp{\n"
                    "	background-color: #333333;\n"
                    "\n"
                    "}\n"
                    "QWidget#widget_showImage{\n"
                    "background-color: #222222;\n"
                    "}\n"
                    "\n"
                    "\n"
                    "QWidget#widget_SelectCoordi{\n"
                    "background-color: #222222;\n"
                    "\n"
                    "}\n"
                    "QComboBox{\n"
                    "background-color: #131313;\n"
                    "color: rgb(255, 255, 255);\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    "\n"
                    "\n"
                    "QLabel#label{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    "QLabel#label_2{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    "QLabel#label_3{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    "QLabel#label_4{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "background-color:#333333;\n"
                    "}\n"
                    "QLabel#label_SRS{\n"
                    "\n"
                    "color: rgb(255, 255, 255);\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    "\n"
                    "QLabel#label_Group{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "background-color:#333333;\n"
                    ""
                    "}\n"
                    "\n"
                    "QWidget#widget_photo\n"
                    "{\n"
                    "background-color: #333333;\n"
                    "}\n"
                    "QLabel#label_photoinfo\n"
                    "{\n"
                    "color: rgb(165, 165, 165);\n"
                    "font: 14px \"Arial\";\n"
                    "background-color:#333333;\n"
                    "}\n"
                    "\n"
                    "QLabel#label_showEpi{\n"
                    "\n"
                    "color: rgb(255, 255, 255);\n"
                    "font: 14px \"Arial\";\n"
                    "background-color: #222222;\n"
                    "\n"
                    "}\n"
                    "\n"
                    "QPushButton{\n"
                    "color: #A5A5A5;\n"
                    "font: 14px \"Arial\";\n"
                    "\n"
                    "border:1px solid;\n"
                    "border-color:#5B5B5B;\n"
                    "background-color: #222222;\n"
                    "}\n"
                    "QPushButton:pressed{\n"
                    "color: #A5A5A5;\n"
                    "font: 14px \"Arial\";\n"
                    "background-color: #FFFFFF;\n"
                    "}\n"
                    "\n"
                    "\n"
                    "QPushButton#btn_epipolarline\n"
                    "{\n"
                    " text-align:left;\n"
                    "}\n"
                    "\n"
                    "QPushButton:hover\n"
                    "{\n"
                    "background-color: #333333;\n"
                    "}\n"
                    "\n"
                    "QWidget#epi_widget\n"
                    "{\n"
                    "background-color: #222222;\n"
                    "}\n"
                    "\n"
                    "QLabel#toolBtn_gcpstatis\n"
                    "{\n"
                    "color: #A5A5A5;\n"
                    "font: 14px \"Arial\";\n"
                    "}\n"
                    ""));
                centralwidget = new QWidget(ControlPointsWin);
                centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
                centralwidget->setStyleSheet(QString::fromUtf8(""));
                gridLayout_7 = new QGridLayout(centralwidget);
                gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
                splitter_3 = new QSplitter(centralwidget);
                splitter_3->setObjectName(QString::fromUtf8("splitter_3"));
                splitter_3->setOrientation(Qt::Horizontal);
                splitter_3->setHandleWidth(0);
                splitter_3->setChildrenCollapsible(true);
                splitter_2 = new QSplitter(splitter_3);
                splitter_2->setObjectName(QString::fromUtf8("splitter_2"));
                splitter_2->setStyleSheet(QString::fromUtf8(""));
                splitter_2->setOrientation(Qt::Vertical);
                splitter_2->setHandleWidth(0);
                widget_showGcp = new QWidget(splitter_2);
                widget_showGcp->setObjectName(QString::fromUtf8("widget_showGcp"));
                widget_showGcp->setMinimumSize(QSize(0, 28));
                widget_showGcp->setMaximumSize(QSize(16777215, 28));
                widget_showGcp->setStyleSheet(QString::fromUtf8(""));
                gridLayout = new QGridLayout(widget_showGcp);
                gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
                gridLayout->setHorizontalSpacing(0);
                gridLayout->setContentsMargins(0, 0, 0, 0);
                horizontalLayout_4 = new QHBoxLayout();
                horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
                label = new QLabel(widget_showGcp);
                label->setObjectName(QString::fromUtf8("label"));
                label->setMinimumSize(QSize(130, 28));
                label->setMaximumSize(QSize(130, 28));
                label->setStyleSheet(QString::fromUtf8(""));

                horizontalLayout_4->addWidget(label);

                horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

                horizontalLayout_4->addItem(horizontalSpacer_4);

                label_2 = new QLabel(widget_showGcp);
                label_2->setObjectName(QString::fromUtf8("label_2"));
                label_2->setMinimumSize(QSize(100, 28));
                label_2->setMaximumSize(QSize(100, 28));
                label_2->setStyleSheet(QString::fromUtf8(""));

                horizontalLayout_4->addWidget(label_2);

                toolBtn_gcpstatis = new QLabel(widget_showGcp);
                toolBtn_gcpstatis->setObjectName(QString::fromUtf8("toolBtn_gcpstatis"));
                toolBtn_gcpstatis->setMinimumSize(QSize(50, 28));
                toolBtn_gcpstatis->setMaximumSize(QSize(50, 28));
                toolBtn_gcpstatis->setStyleSheet(QString::fromUtf8(""));

                horizontalLayout_4->addWidget(toolBtn_gcpstatis);


                gridLayout->addLayout(horizontalLayout_4, 0, 0, 1, 1);

                splitter_2->addWidget(widget_showGcp);
                widget_SelectCoordi = new QWidget(splitter_2);
                widget_SelectCoordi->setObjectName(QString::fromUtf8("widget_SelectCoordi"));
                widget_SelectCoordi->setStyleSheet(QString::fromUtf8(""));
                gridLayout_2 = new QGridLayout(widget_SelectCoordi);
                gridLayout_2->setSpacing(0);
                gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
                gridLayout_2->setContentsMargins(0, 0, 2, 0);
                gridLayout_3 = new QGridLayout();
                gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
                gridLayout_3->setContentsMargins(-1, -1, 0, -1);
                label_SRS = new QLabel(widget_SelectCoordi);
                label_SRS->setObjectName(QString::fromUtf8("label_SRS"));
                label_SRS->setMinimumSize(QSize(30, 28));
                label_SRS->setMaximumSize(QSize(30, 28));
                label_SRS->setStyleSheet(QString::fromUtf8(""));

                gridLayout_3->addWidget(label_SRS, 0, 0, 1, 1);
                comboBox_srs = new QComboBox(widget_SelectCoordi);
                comboBox_srs->setObjectName(QString::fromUtf8("comboBox_srs"));
                QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                sizePolicy.setHorizontalStretch(0);
                sizePolicy.setVerticalStretch(0);
                sizePolicy.setHeightForWidth(comboBox_srs->sizePolicy().hasHeightForWidth());
                comboBox_srs->setSizePolicy(sizePolicy);
                comboBox_srs->setMinimumSize(QSize(450, 28));
                comboBox_srs->setMaximumSize(QSize(1000, 28));
                comboBox_srs->setFocusPolicy(Qt::ClickFocus);
                comboBox_srs->setStyleSheet(QString::fromUtf8(""));

                gridLayout_3->addWidget(comboBox_srs, 0, 1, 1, 1);


                gridLayout_2->addLayout(gridLayout_3, 0, 0, 1, 1);

                splitter_2->addWidget(widget_SelectCoordi);
                layoutWidget = new QWidget(splitter_2);
                layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
                verticalLayout = new QVBoxLayout(layoutWidget);
                verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
                verticalLayout->setContentsMargins(0, 0, 0, 0);
                
                gcplistview = new MoTableWidget(layoutWidget);
                gcplistview->setObjectName(QString::fromUtf8("gcplistview"));
                QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
                sizePolicy1.setHorizontalStretch(0);
                sizePolicy1.setVerticalStretch(0);
                sizePolicy1.setHeightForWidth(gcplistview->sizePolicy().hasHeightForWidth());
                gcplistview->setSizePolicy(sizePolicy1);
                gcplistview->setStyleSheet(QString::fromUtf8(""));

                gcplistview->horizontalScrollBar()->setStyleSheet("QScrollBar{height:10px;}");
                gcplistview->verticalScrollBar()->setStyleSheet("QScrollBar{width: 10px;}");

                verticalLayout->addWidget(gcplistview);
             
                verticalLayout->setStretch(0, 100);
                splitter_2->addWidget(layoutWidget);
                layoutWidget1 = new QWidget(splitter_2);
                layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
                verticalLayout_2 = new QVBoxLayout(layoutWidget1);
                verticalLayout_2->setSpacing(0);
                verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
                verticalLayout_2->setContentsMargins(0, 0, 0, 0);
                label_4 = new QLabel(layoutWidget1);
                label_4->setObjectName(QString::fromUtf8("label_4"));
                label_4->setMinimumSize(QSize(0, 28));
                label_4->setSizeIncrement(QSize(0, 28));
                label_4->setStyleSheet(QString::fromUtf8(""));

                verticalLayout_2->addWidget(label_4);

                measurementsview = new MoTableWidget(layoutWidget,1);

                measurementsview->setObjectName(QString::fromUtf8("measurementsview"));
                measurementsview->setStyleSheet(QString::fromUtf8(""));
                //measurementsview->horizontalHeader()->setDefaultSectionSize(80);

                measurementsview->horizontalScrollBar()->setStyleSheet("QScrollBar{height:10px;}");
                measurementsview->verticalScrollBar()->setStyleSheet("QScrollBar{width: 10px;}");

                verticalLayout_2->addWidget(measurementsview);

                

                splitter_2->addWidget(layoutWidget1);
                splitter_3->addWidget(splitter_2);
                splitter = new QSplitter(splitter_3);
                splitter->setObjectName(QString::fromUtf8("splitter"));
                ///splitter->setMinimumSize(QSize(1100, 500));
                splitter->setOrientation(Qt::Vertical);
                splitter->setHandleWidth(0);

               

                widget_showImage = new QWidget(splitter);
                widget_showImage->setObjectName(QString::fromUtf8("widget_showImage"));
                widget_showImage->setMaximumSize(QSize(16777215, 30));
                widget_showImage->setStyleSheet(QString::fromUtf8(""));

                gridLayout_4 = new QGridLayout(widget_showImage);
                gridLayout_4->setSpacing(0);
                gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
                gridLayout_4->setContentsMargins(0, 0, 0, 0);
                horizontalLayout = new QHBoxLayout();
                horizontalLayout->setSpacing(20);
                horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
                horizontalLayout->setContentsMargins(5, -1, -1, -1);
                btn_AllPho = new QPushButton(widget_showImage);
                btn_AllPho->setObjectName(QString::fromUtf8("btn_AllPho"));
                btn_AllPho->setMinimumSize(QSize(0, 28));
                btn_AllPho->setMaximumSize(QSize(16777215, 28));
                btn_AllPho->setStyleSheet(QString::fromUtf8(""));
                btn_AllPho->setCheckable(true);

                horizontalLayout->addWidget(btn_AllPho);

                btn_MatchPho = new QPushButton(widget_showImage);
                btn_MatchPho->setObjectName(QString::fromUtf8("btn_MatchPho"));
                btn_MatchPho->setMinimumSize(QSize(0, 28));
                btn_MatchPho->setMaximumSize(QSize(16777215, 28));
                btn_MatchPho->setStyleSheet(QString::fromUtf8(""));
                btn_MatchPho->setCheckable(true);

                horizontalLayout->addWidget(btn_MatchPho);

                btn_MarkPho = new QPushButton(widget_showImage);
                btn_MarkPho->setObjectName(QString::fromUtf8("btn_MarkPho"));
                btn_MarkPho->setMinimumSize(QSize(0, 28));
                btn_MarkPho->setMaximumSize(QSize(16777215, 28));
                btn_MarkPho->setStyleSheet(QString::fromUtf8(""));
                btn_MarkPho->setCheckable(true);
                //? 
                btn_MarkPho->setChecked(false);

                horizontalLayout->addWidget(btn_MarkPho);

                horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

                horizontalLayout->addItem(horizontalSpacer);


                gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

                splitter->addWidget(widget_showImage);
               
                previewlistview = new MoListWidget(splitter);

                previewlistview->setObjectName(QString::fromUtf8("previewlistview"));
                
                previewlistview->setStyleSheet(QString::fromUtf8("background-color: rgb(18, 18, 18);margin:0px;padding:0px;"
                    "QListView::item{margin:0px;padding:0px;}"
                ));
                previewlistview->verticalScrollBar()->setStyleSheet("width:10px;margin:0px;padding:0px;");
                previewlistview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                previewlistview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                previewlistview->setFixedHeight(254+15);

                //"QListView QScrollBar:vertical{width:10px;background-color:red;}"
                
                previewlistview->setSpacing(0);
                splitter->addWidget(previewlistview);
                layoutWidget2 = new QWidget(splitter);
                layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
                verticalLayout_3 = new QVBoxLayout(layoutWidget2);
                verticalLayout_3->setSpacing(0);
                verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
                verticalLayout_3->setContentsMargins(0, 0, 0, 0);
                epi_widget = new QWidget(layoutWidget2);
                epi_widget->setObjectName(QString::fromUtf8("epi_widget"));
                epi_widget->setMinimumSize(QSize(0, 32));
                epi_widget->setMaximumSize(QSize(16777215, 32));
                epi_widget->setStyleSheet(QString::fromUtf8(""));
                gridLayout_6 = new QGridLayout(epi_widget);
                gridLayout_6->setSpacing(0);
                gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
                gridLayout_6->setContentsMargins(3, 0, 0, 0);
                horizontalLayout_5 = new QHBoxLayout();
                horizontalLayout_5->setSpacing(0);
                horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
                btn_epipolarline = new QPushButton(epi_widget);
                btn_epipolarline->setObjectName(QString::fromUtf8("btn_epipolarline"));
                btn_epipolarline->setMinimumSize(QSize(120, 28));
                btn_epipolarline->setMaximumSize(QSize(120, 28));
                btn_epipolarline->setStyleSheet(QString::fromUtf8(""));

                horizontalLayout_5->addWidget(btn_epipolarline);

                horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

                horizontalLayout_5->addItem(horizontalSpacer_5);


                gridLayout_6->addLayout(horizontalLayout_5, 0, 0, 1, 1);


                verticalLayout_3->addWidget(epi_widget);

                measuringview = new QWidget(layoutWidget2);
                measuringview->setObjectName(QString::fromUtf8("measuringview"));
                measuringview->setMinimumSize(QSize(500, 300));
                measuringview->setStyleSheet(QString::fromUtf8(""));
                verticalLayout_21 = new QVBoxLayout(measuringview);
                verticalLayout_21->setSpacing(0);
                verticalLayout_21->setObjectName(QString::fromUtf8("verticalLayout_21"));
                verticalLayout_21->setContentsMargins(0, 0, 0, 0);

                verticalLayout_3->addWidget(measuringview);

                splitter->addWidget(layoutWidget2);
                splitter_3->addWidget(splitter);

                gridLayout_7->addWidget(splitter_3, 0, 0, 1, 1);

                ControlPointsWin->setCentralWidget(centralwidget);

                retranslateUi(ControlPointsWin);

                QMetaObject::connectSlotsByName(ControlPointsWin);
            } // setupUi
            //拷贝自 ui_.h
            void retranslateUi(QMainWindow* ControlPointsWin)
            {
                ControlPointsWin->setWindowTitle(QApplication::translate("ControlPointsWin", "MainWindow", nullptr));
                label->setText(QApplication::translate("ControlPointsWin", "Control point(s)list", nullptr));
                label_2->setText(QApplication::translate("ControlPointsWin", "GCP marked:", nullptr));
                toolBtn_gcpstatis->setText(QApplication::translate("ControlPointsWin", "0/0", nullptr));
               
                label_SRS->setText(QApplication::translate("ControlPointsWin", "SRS", nullptr));
                
                label_4->setText(QApplication::translate("ControlPointsWin", "Measurments", nullptr));
               
                btn_AllPho->setText(QApplication::translate("ControlPointsWin", "All photos", nullptr));
                btn_MatchPho->setText(QApplication::translate("ControlPointsWin", "Matched photo", nullptr));
                btn_MarkPho->setText(QApplication::translate("ControlPointsWin", "Marked GCP photo", nullptr));
                btn_epipolarline->setText(QApplication::translate("ControlPointsWin", "Show epipolar line", nullptr));
            } // retranslateUi

        };

        class ViewWidget;


		class ControlPointsEditorWin : public QMainWindow
		{
			Q_OBJECT


		private:

            //用于显示的类型
            typedef enum surveypoint_type_forshow_e
            {
                SURVEYSHOW_GCP,
                SURVEYSHOW_GCPCHECK,
                SURVEYSHOW_USERPOINT,
            }sv_type_e;

			typedef struct gcp_propertys_toshow_s_
			{
				point3D_t id_ = kInvalidPoint3DId;
				std::string name_ = "";
                sv_type_e type_ = SURVEYSHOW_GCP;
				Eigen::Vector3d xyz_ = { -DBL_MAX,-DBL_MAX, -DBL_MAX };
				Eigen::Vector3d esitmated_xyz_ = { -DBL_MAX,-DBL_MAX, -DBL_MAX };
				double error_3d_ = kInvalidError;
				double error_3d_xy_ = kInvalidError;
				double error_3d_z_ = kInvalidError;
				double rms_pix_ = kInvalidError;
				double rms_dis_ = kInvalidError;
                int photos_;
				int num_marked_photos_ = kInvalideNum;
				int color_ = -1;// 默认值@liyue,0:绿色,1:黄色，2：红色
				std::set<image_t> matched_imageids;//用以记录匹配影像的集合
			}gcp_toshow_s;

			typedef struct image_propertys_toshow_s_
			{
				image_t id_ = kInvalidImageId;
				std::string name_ = "";
				std::string preview_name_ = "";
				double x_ = -DBL_MAX; //实际刺的点
				double y_ = -DBL_MAX;
				double estimated_x_ = -DBL_MAX;//预测的坐标
				double estimated_y_ = -DBL_MAX;//预测的坐标
				double rms_pix_ = kInvalidError;
				double rms_dis_ = kInvalidError;
				int color_ = -1;// 默认值@liyue
                bool check_ = false;
                int type_ = 0;
                group_t groupId;
                int width;
                int height;
			}img_toshow_s;

		public:
			explicit ControlPointsEditorWin(AI3D::CORE::BlockObject* block, QWidget* parent = 0, AI3D::GUI::ViewWidget* viewWidget = nullptr);
			~ControlPointsEditorWin();
			void Init();
			void InitSrss(bool bSetCurrentItem4Recent = false);
			/*设置默认的坐标系统也即导入数据时选定的坐标系统来设置，
			1：列表中没有enu，但如果导入的数据有enu则追加进来
			2：如果是xml形式，有可能每个控制点有一个srs则取第一个，其余的需要转换，可以统一调用controlpoints类中的转换到basecoor；
			3：如果控制点有不同的坐标系也即2成立则需要转换并按转换后的显示*/
			void SetDefaultSrs(srs_s srs);
			void ParseDefaultSrs();
			void SetComBoxCurrentSrs();
			

			//
			srs_s GetSelectSrs(QString srsname);//解析界面上的srs，
			srs_s GetCurrentSrs();//从界面上
			void SetCurrentSrs(srs_s srs);

			void InitImageSet();
			void InitGcpData();
            void InitSurveyData();
			//void UpdateXYZAsSrsChanged();//因为srs切换了所以需要更新xyz显示
			void UpdateGcpListView(bool Is_UpdateCurrentGCP = false);
            void UpdateSurveyListView(bool Is_UpdateCurrentSurveyData = false);
            void PrepareSurveyListData(bool Is_UpdateCurrentGCP = false);
            void ShowSurveyListData();
            void  PrepareUserPointListData(bool Is_UpdateCurrentData);
			/*用于需要所有gcp更新时
			* 更新的内容包括：
			* 1：givenxyz；
			* 2：estimatedxyz；
			* 3：errors
			* 4：不同颜色：高亮显示的、不同误差不同颜色表示的
			*/
			void PrepareGcpListData(bool Is_UpdateCurrentGCP = false);
			void ShowGcpListData();//将准备的数据显示出来
			void ShowSelectedGcpView();
			void ChangeRowSeleColor(QModelIndex current, QModelIndex previous);

            static void pushRecentCRS(QString &str);

			//当某个控制点被选中时，除GCPLISTVIEW高亮显示外更新外其余均需要更新
			//当某个gcp发生改变时，gcpllist中的该gcp的error需要改变，
			//但这个地方是该gcp整体更新还是局部更新需要讨论	
			void UpdateGcpSelected();
			//void UpdateGcpListView(point3D_t gcp_id);
			//用于更新单个控制点信息更新时，看是否需要与PrepareGcpListData区分@liyue
			

			void ShowSelectedMeasurementView();//设置选中为高亮
			void UpdatePreviewListView();
			/*用于缩略图更新时，是否需要根据all\matched\marked的分开@liyue决定
			* 更新的内容包括，
			* 1：影像缩略图；
			* 2：不同颜色：高亮显示的、不同误差不同颜色表示的；
			*/
			void ShowPreviewHighLight();//设置选中为高亮
			void PreparePreviewListData();
			void ShowPreviewListData();

			//获取GCP的iD，用于对GcplistView单机双击操作时从界面解析出来当前选中的gcpid
			point3D_t GetSelectGcpId();
			void SetCurrentGcpId(point3D_t gcp_id);
			


			void SetCurrentImageId(image_t img_id);
			image_t GetCurrentImageId();

			/*以下几个函数原始状态:
			* 典型场景在导入gcp(无论是导入txt还是xml形式下)将控制点显示出来
			* 可以先开发完，@zhaok测试
			*/
			void InitGcpListView();
			void InitPreviewListView();//
			void InitMeasuringView();
			

			void InitGcpListViewHeader();
			void InitGcpListViewBottonStatus();

			void InitPreviewListViewHeader();
			

			void InitMeasuringViewHeader();
			

			
			void UpdateGcpListViewBottonStatus();
			void UpdateMeasuringViewBottonStatus();
			void UpdatePreviewListViewBottonStatus();


			//删除所有点之后
			void InitBlankGcpListView();
			//初始状态或者删除所有点之后
			void InitBlankPreviewListView();
			//初始状态或者删除所有点之后
			void InitBlankMeasuringView();
			//初始状态或者删除所有点之后
			void InitBlankMeasurementsView();
			

			void UpdateMeasuringView(bool isImage = false);//更新刺点操作界面
			void PrepareMeasuringData();//准备数据，除值之外还需要红框绿框等
			void ShowMeasuringData(/*QListWidgetItem* item*/bool isImage = false);
			//void UpdateMeasuringView(point3D_t gcp_id,image_t img_id);//需要考虑当img_id没有的时候主要是删除的时候			
			void UpdateMeasurementsView();
			void PrepareMeasurementsData();
			void ShowMeasurementsData();
			void RecoverMeasurementsView();//恢复原MeasurementsView


			void UpdateLabelRecoder();//更新label_3 eg::GCPmarked 10/30 
			
			//参数 xyz是否开启可编辑
			void MakeGcplistViewUneditable(bool bxyzedit_on = true);
			//每次更新前
			//void UpdateColorImages();

			void SetBlockdata(AI3D::CORE::BlockObject* block);
            int get_photos(const AI3D::CORE::Image& image/*, const QString& userTiePointName*/);

            void SetEditable(bool be);

            bool PostProcessAfterClickingGcpListView(const QModelIndex &index);


		private:
			AI3D::CORE::BlockObject* blockdata_;
			std::set<image_t> imageids_forgenpreview_;//记录用于生成缩略图的图像id'
			//  暂时未用到EIGEN_STL_UMAP(image_t, AI3D::CORE::Image) images_regist_;//记录转换后的影像外方位等信息;
			//EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) gcps_;
			/*AI3D::CORE::ControlPoints gcps_;*///包含好多操作
			 //此处类型建议独立设计，主要用于记录在界面上显示的内容包括数值以及颜色；@liyue
		//	EIGEN_STL_UMAP(point3D_t, AI3D::CORE::ControlPoint) gcps_show_;
			std::map<point3D_t, gcp_toshow_s> gcps_show_;
			std::map<point3D_t, std::set<image_t>> gcps_matched_;//每个GCP对应要显示的影像
			image_t measurement_highlight_ = kInvalidImageId;
			//std::map<int, std::vector<point3D_t> > color_gcps_;//int 0 ：green,1:黄点，2：红
			//用于记录当前控制点的measurements信息
			//measurements_show_;
			////用于记录匹配到的图片以及颜色显示高亮显示的记录
			//matchphotos_;
			std::map<int, std::pair<Eigen::Vector2d, Eigen::Vector2d>> epipolarlines_show_;//影像id以及对应的核线
			//已刺的值
			Eigen::Vector2d measured_xy_show_{ -DBL_MAX , -DBL_MAX };
			//预测值
			Eigen::Vector2d candidate_xy_show_{ -DBL_MAX , -DBL_MAX };
			std::pair<int, float> old_image_scale_ratio_;//
			//allphotos_;//用于记录所有影像，包括颜色显示
			/*用于记录需要不同颜色显示的照片；
			* 无论是all\match\marked不同颜色显示的用于只对当前gcp 的
			*/
			std::map<point3D_t, std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >>measurement_error_map_;
			//std::map < image_t, std::pair<Eigen::Vector2d, std::pair<double, double> > >measurement_error_map_;
			std::map<image_t, img_toshow_s > images_show_;

			//std::vector<point3D_t> marked_imageids_;
			int preview_show_mode_ = 2;//1 2 3 代表all photo  mathed photo marked gcp photo


			srs_s default_srs_;//默认的值
			srs_s current_srs_;


			QList<QListWidgetItem*> previewlistview_item_;//显示缩略图
			control_point_GUI::GraphicsView* measuringview_;//刺点界面；



			point3D_t currentgcp_id_ = kInvalidPoint3DId;//当前被选中的gcp;
			image_t currentimage_id_ = kInvalidImageId;//当前被选中的影像id；

			bool  bshow_epipolarline_;//是否显示核线；
            //bool  benable_epipolarline_;//是否显示核线；

			int  imagesorting_method_;//影像排序方式，建议枚举，默认按相机组排序，后续扩展

			QStringList gcplistview_header;
			QStringList gcplistview_header_WGS84;
			QStandardItemModel* gcpviewlist_model;
			//QSensorIntDelegate* comboxSelf;// ?
			
			bool deletetablemodel_ = false;
			bool isexectableview = false;
			bool btopredict_ = false;//是否开启实时刺点
			std::string block_path_ = "";

			QModelIndex oldgcpindex;
			QModelIndex currentgcpindex;
			bool update_ = false;
		private:

			void MatchPhotos(std::map<image_t, Eigen::Vector2d >& estimated_xy);
			void MatchPhotos(std::set<image_t >& images_ids);
			void CreateConnect();

			image_t GetFirstImageId();
			void SortMatchPhotos(int imagesorting_method);
			void InitPhotosButtons();
			QColor GetColor(int color);
			QColor GetSelectColor(int color);

			double scale_ = 1.0;
			double offsetx_ = 0.0;
			double offsety_ = 0.0;

			QPointF point_;
			bool isgcp_clicked_ = false;
			QItemSelectionModel* theSelection;
			QMenu* ui_menu_rightClick_selectRows;
			QAction* ui_action_deletegcp_;
			QAction* ui_action_addgcp_;
            QAction* ui_action_copygcp_;

			QMenu* ui_menu_rightClickMeasure_selectRows;
			QAction* ui_action_deletemeausurement_;
			
			int menuRow = -1;

            bool bChangingSrs;
            bool bMeasuringClicked = false;
            int iPreviousGcpListViewRow = -1;
            bool bChangedGcpListViewRow = false;

            QMap<group_t, QString> photoGroups;
            QMap<QString, QList<gcp_measurement_list_item_st> *> previewListMap;
            AI3D::GUI::ViewWidget* viewWidget;
            QString previous_srs;

		signals:
			void Sig_ModifiedTrue();
            void Sig_InsertGCPTab();

		public slots:
			//GcpListView相关操作
			//单击gcplistview某一行
			void Slot_GcpListItem_SingleClicked(QModelIndex, QModelIndex);
			void Slot_SrsItemChanged(QString srsname);//用于切换坐标系统
		
			void Slot_CategoryChanged(QString itemtext);//控制点改类型与name一样			
            void Slot_ItemDataChanged(QStandardItem* item);

			void Slot_GivenXYZChanged(int id, QString itemtext);
			void Slot_DeleteAllGcps();//一键删除；		
			//void Slot_AddGcps();//

			
			void Slot_DeleteGcp();//单个删除；暂时不做
            void Slot_CopyGcp();//单个删除；暂时不做

			//previewlistview相关操作
			void Slot_AllPhotos_Clicked();
			void Slot_MatchedPhotos_Clicked();
			void Slot_MarkedPhotos_Clicked();
			//对应某个缩略图被选中，单击双击都可
			
            void displayImage(const QModelIndex& index,QString& imageFile, int specialX, int specialY);

			//measuring 界面相关的操作主要就是shift + 单击，函数名？
			void Slot_Measuring_Clicked(int imageID, QPointF point);
            void Slot_SrsSelected(QString &srs);
            void Slot_SrsRestore();

            void tableviewClick(QModelIndex index);
            void measurement_tableviewClick(QModelIndex index);

			void Slot_SetCurrentScale(double value) { scale_ = value;  };
			void Slot_SetCurrentOffset(QPointF pointvalue) { point_ = pointvalue;  };

			//平移缩放等操作略

			//MeasurementsView界面相关操作
			//指某个Item单击或双击时
			
			//删除某个measurement
			void Slot_MeasurementsItem_Delete();

			

			void Slot_ShowEpi();
			

			void Slot_QTableView_CustomContextMenuRequested(const QPoint& pos);
			void Slot_QTableWidget_CustomContextMenuRequested(const QPoint& pos);

            void Slot_itemModified(int, int, const QString&);
            void Slot_RefreshMeasuringView();

            void Slot_add_user_tie_point(const AI3D::CORE::Image& image, const QString& userTiePointName);


            //人工点的加入
        private:
            void DeleteGCP();
            void DeleteUserpoint();
            bool IsCurrentSelectionUserType(const point3D_t& id);
			/** Measured pixel for current GCP / image; prefers image 2D map, else track observation. */
			Eigen::Vector2d GetDisplayedMeasurementPixel(image_t image_id);
		private:
			
            Ui_ControlPointsWin2* ui;
            //建立一个本列表中的id到具体点类型的id,first 本地点id
            std::map<point3D_t, std::pair<point3D_t, sv_type_e>> globalid_to_localid_map_;
            //
            std::map<std::pair<point3D_t, sv_type_e>, point3D_t> localid_to_globalid_map_;
		};

	

		
		
    }
}


#endif

