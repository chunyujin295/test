#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QEvent>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QBitmap>
#include "Gui/message_box.h"


Message_Box::Message_Box(QWidget* parent, const QString& title, const QString& text) : QDialog(parent)
{
    ui.setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    //this->setGeometry(0, 0, 200, 200);
    m_pLabel = ui.label;
    m_pIconLabel = ui.iconLabel;
    m_pDefaultButton = ui.defaultButton;
    m_pYesButton = ui.yesButton;
    m_pNoButton = ui.noButton;
    m_pCancelButton = ui.cancelButton;
    m_pCloseButton = ui.closeButton;
    m_pIconLabel = ui.iconLabel;
    m_pUpdateLabel = ui.updateLabel;
    m_pTitleWidget = ui.titleWidget;
    m_pUpdateButton = ui.updateButton;

    m_pUpdateLabel->setVisible(false);
    m_pUpdateLabel->setText("New version is waiting to be updated.");

    m_pUpdateButton->setVisible(false);
    m_pUpdateButton->setText("Update");

    connect(m_pDefaultButton, SIGNAL(clicked()), this, SLOT(onDefaultButtonClicked()));
    connect(m_pYesButton, SIGNAL(clicked()), this, SLOT(onYesButtonClicked()));
    connect(m_pNoButton, SIGNAL(clicked()), this, SLOT(onNoButtonClicked()));
    connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
    connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));
    connect(m_pUpdateButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonClicked()));
}

Message_Box::~Message_Box()
{

}

void Message_Box::paintEvent(QPaintEvent* p1)
{
	//绘制样式
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//绘制样式

	QBitmap bmp(this->size());
	bmp.fill();
	QPainter painter(&bmp);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawRoundedRect(bmp.rect(), 10, 10);
	setMask(bmp);

}

void Message_Box::initAsAbout()
{
    this->setFixedSize(260, 240);
    m_pTitleWidget->setVisible(false);
    m_pYesButton->setVisible(false);
    m_pNoButton->setVisible(false);
    m_pCancelButton->setVisible(false);
    m_pCloseButton->setVisible(true);
    setIcon(":/new/prefix1/skin/logo48_52.png");
    m_pIconLabel->setGeometry(107, 30, 48, 52);
    m_pLabel->setGeometry(67, 102, 126, 20);
    m_pCloseButton->setGeometry(60, 182, 140, 28);
    //setDefaultButton(dynamic_cast<QPushButton *>(QMessageBox::NoButton));
}

void Message_Box::initAsAboutUpdate()
{
 //   this->setFixedSize(300, 270);
    this->setFixedSize(260, 256);
    m_pTitleWidget->setVisible(false);
    m_pYesButton->setVisible(false);
    m_pNoButton->setVisible(false);
    m_pCancelButton->setVisible(false);
    m_pCloseButton->setVisible(true);

//    m_pUpdateLabel->setVisible(true);
    m_pUpdateButton->setVisible(true);

    m_pUpdateLabel->setStyleSheet("color:red;");

    setIcon(":/new/prefix1/skin/logo48_52.png");
    m_pIconLabel->setGeometry(107, 30, 48, 52);

    m_pLabel->setGeometry(67, 102, 126, 20);
    m_pLabel->setAlignment(Qt::AlignCenter);

    //m_pUpdateLabel->setGeometry(24, 132, 252, 20);
    //m_pUpdateLabel->setAlignment(Qt::AlignCenter);   

    m_pUpdateButton->setGeometry(60, 154, 140, 28);
    //m_pUpdateButton->setStyleSheet("color:black;background:white;");
    m_pUpdateButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
        "	\n"
        "   color: #000000;\n"
        "	background-color: #569BFF;\n"
        "	border: 1px solid rgba(70,70,70,1);\n"
        "	box-shadow: 0px 1px 1px 0px rgba(0,0,0,0.5);\n"
        "	border-radius: 4px;\n"
        "}\n"
        "\n"
        "QPushButton:pressed{	\n"
        "	background-color: #538CCF;\n"
        "}\n"
        "\n"
        "QPushButton:hover{	\n"
        "	background-color: #569BFF;\n"
        "}\n"
        "\n"
        "QPushButton:default\n"
        "{\n"
        "	background-color: #569BFF;\n"
        "}"));


    m_pCloseButton->setGeometry(60, 197, 140, 28);
    //setDefaultButton(dynamic_cast<QPushButton *>(QMessageBox::NoButton));

    m_pCloseButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
        "	\n"
        "   color: #000000;\n"
        "	background-color: #ffffff;\n"
        "	border: 1px solid rgba(70,70,70,1);\n"
        "	box-shadow: 0px 1px 1px 0px rgba(0,0,0,0.5);\n"
        "	border-radius: 4px;\n"
        "}\n"
        "\n"
        "QPushButton:pressed{	\n"
        "	background-color: #538CCF;\n"
        "}\n"
        "\n"
        "QPushButton:hover{	\n"
        "	background-color: #569BFF;\n"
        "}\n"
        "\n"
        "QPushButton:default\n"
        "{\n"
        "	background-color: #ffffff;\n"
        "}"));



}

void Message_Box::initAsAboutUpdatePrompt()
{
    this->setFixedSize(682, 161);
    //m_pTitleWidget->setVisible(false);
    m_pYesButton->setVisible(false);
    m_pNoButton->setVisible(false);

    m_pCancelButton->setVisible(false);

    m_pCloseButton->setVisible(true);
    m_pCloseButton->setText("OK");

    //m_pUpdateLabel->setVisible(true);
    //m_pUpdateButton->setVisible(true);
 //   m_pUpdateLabel->setStyleSheet("color:red;");

    //setIcon(":/new/prefix1/skin/logo48_52.png");
    //m_pIconLabel->setGeometry(126, 30, 48, 52);
    m_pIconLabel->setVisible(false);

    //m_pLabel->setGeometry(117, 52, 486, 20);
    m_pLabel->setGeometry(100, 52, 520, 20);
    m_pLabel->setAlignment(Qt::AlignCenter);
    m_pLabel->setText("This software will update automatically after master and engine were restarted.");
    m_pLabel->setStyleSheet("color:black:background-color:white;border-radius:4px;font: 14px 'Arial';");

    //combox_New->setStyleSheet("font: 14px 'Arial';");
    //m_pUpdateLabel->setGeometry(24, 132, 252, 20);
    //m_pUpdateLabel->setAlignment(Qt::AlignCenter);

    //m_pUpdateButton->setGeometry(80, 172, 140, 28);
    ////m_pUpdateButton->setStyleSheet("color:black;background:white;");
    //m_pCloseButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
    //    "	\n"
    //    "   color: #000000;\n"
    //    "	background-color: #ffffff;\n"
    //    "	border: 1px solid rgba(70,70,70,1);\n"
    //    "	box-shadow: 0px 1px 1px 0px rgba(0,0,0,0.5);\n"
    //    "	border-radius: 4px;\n"
    //    "}\n"
    //    "\n"
    //    "QPushButton:pressed{	\n"
    //    "	background-color: #538CCF;\n"
    //    "}\n"
    //    "\n"
    //    "QPushButton:hover{	\n"
    //    "	background-color: #569BFF;\n"
    //    "}\n"
    //    "\n"
    //    "QPushButton:default\n"
    //    "{\n"
    //    "	background-color: #FFFFFF;\n"
    //    "}"));


    m_pCloseButton->setGeometry(271, 106, 140, 28);
    m_pCloseButton->setStyleSheet("font: 12px 'Arial';");

    m_pCloseButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
        "	\n"
        "   color: #000000;\n"
        "	background-color: #545454;\n"
        "	border: 1px solid rgba(70,70,70,1);\n"
        "	box-shadow: 0px 1px 1px 0px rgba(0,0,0,0.5);\n"
        "	border-radius: 4px;\n"
        "}\n"
        "\n"
        "QPushButton:pressed{	\n"
        "	background-color: #538CCF;\n"
        "}\n"
        "\n"
        "QPushButton:hover{	\n"
        "	background-color: #569BFF;\n"
        "}\n"
        "\n"
        "QPushButton:default\n" 
        "{\n"
        "	background-color: #545454;\n"
        "}"));

    //setDefaultButton(dynamic_cast<QPushButton *>(QMessageBox::NoButton));
}

void Message_Box::initAsQuestion(enum Message_Box_Type questionType)
{
    m_pCloseButton->setVisible(false);
    //QPixmap pixmap(":/new/prefix1/skin/questionlogo.png");
    //pixmap.scaled(ui.iconLabel->size());
    //ui.iconLabel->setPixmap(pixmap);
    setIcon(":/new/prefix1/skin/questionlogo.png");

    if (questionType == Question_Yes_No)
    {
        m_pCancelButton->setVisible(false);
        //设置按钮位置
        //m_pYesButton->setGeometry(200, 170, 75, 23);
        //m_pNoButton->setGeometry(290, 170, 75, 23);
    }
    else if (questionType == Question_Yes_Cancel)
    {
        m_pNoButton->setVisible(false);
        //设置按钮位置
    }
    else if (questionType == Question_Yes_No_Cancel)
    {
        m_pYesButton->setGeometry(121, 106, 140, 28);
        m_pNoButton->setGeometry(271, 106, 140, 28);
        m_pCancelButton->setGeometry(421, 106, 140, 28);
    }
}

void Message_Box::initAsInformation()
{
	m_pYesButton->setVisible(false);
	m_pNoButton->setVisible(false);
	m_pCancelButton->setVisible(false);
    setIcon(":/new/prefix1/skin/informationlogo.png");

}

void Message_Box::initAsError()
{
	m_pYesButton->setVisible(false);
	m_pNoButton->setVisible(false);
	m_pCancelButton->setVisible(false);
	setIcon(":/new/prefix1/skin/errorlogo.png");

}

void Message_Box::initAsCritical()
{
	m_pYesButton->setVisible(false);
	m_pNoButton->setVisible(false);
	m_pCancelButton->setVisible(false);
	setIcon(":/new/prefix1/skin/criticallogo.png");
}

void Message_Box::initAsWarning()
{
	m_pYesButton->setVisible(false);
	m_pNoButton->setVisible(false);
	m_pCancelButton->setVisible(false);
	setIcon(":/new/prefix1/skin/warninglogo.png");
    
}

void Message_Box::changeEvent(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::LanguageChange:
        translateUI();
        break;
    default:
        QDialog::changeEvent(event);
    }
}

void Message_Box::translateUI()
{
    QPushButton* pYesButton = m_pButtonBox->button(QDialogButtonBox::Yes);
    if (pYesButton != NULL)
        pYesButton->setText(tr("Yes"));

    QPushButton* pNoButton = m_pButtonBox->button(QDialogButtonBox::No);
    if (pNoButton != NULL)
        pNoButton->setText(tr("No"));

    QPushButton* pOkButton = m_pButtonBox->button(QDialogButtonBox::Ok);
    if (pOkButton != NULL)
        pOkButton->setText(tr("Ok"));

    QPushButton* pCancelButton = m_pButtonBox->button(QDialogButtonBox::Cancel);
    if (pCancelButton != NULL)
        pCancelButton->setText(tr("Cancel"));
}

QMessageBox::StandardButton Message_Box::standardButton(QAbstractButton* button) const
{
    return (QMessageBox::StandardButton)m_pButtonBox->standardButton(button);
}

QAbstractButton* Message_Box::clickedButton() const
{
    return m_pClickedButton;
}

int Message_Box::execReturnCode(QAbstractButton* button)
{
    int nResult = m_pButtonBox->standardButton(button);
    return nResult;
}

void Message_Box::onButtonClicked(QAbstractButton* button)
{
    m_pClickedButton = button;
    done(execReturnCode(button));
}

void Message_Box::onDefaultButtonClicked()
{
    done(QMessageBox::StandardButton::Cancel);
    //this->close();
}

void Message_Box::onYesButtonClicked()
{
    done(QMessageBox::StandardButton::Yes);
	//this->close();
}

void Message_Box::onNoButtonClicked()
{
    done(QMessageBox::StandardButton::No);
	//this->close();
}

void Message_Box::onCancelButtonClicked()
{
    done(QMessageBox::StandardButton::Cancel);
	//this->close();
}

extern bool needUpgradeLater();

// 执行版本升级请求
void Message_Box::onUpdateButtonClicked()
{
    done(QMessageBox::StandardButton::Cancel);

    // 设置版本正式更新请求标志,下次启动后会自动更新.?//?chy
    needUpgradeLater();

    //Message_Box::information(nullptr, tr("information"), tr("This software will update automatically after master and engine were restarted."));
    // 界面发送升级请求后提示需要重启生效的提示信息.
    Message_Box::aboutUpdatePrompt(nullptr, "", "");
    //this->close();
}

void Message_Box::onCloseButtonClicked()
{
    done(QMessageBox::StandardButton::Close);
	//this->close();
}

void Message_Box::setDefaultButton(QPushButton* button)
{
    //if (!m_pButtonBox->buttons().contains(button))
    //    return;
    m_pDefaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

void Message_Box::setDefaultButton(QMessageBox::StandardButton button)
{
    setDefaultButton(m_pButtonBox->button(QDialogButtonBox::StandardButton(button)));
}

void Message_Box::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void Message_Box::setText(const QString& text)
{
    m_pLabel->setText(text);
}

void Message_Box::setIcon(const QString& icon)
{
    if (nullptr != m_pIconLabel)
    {
        delete m_pIconLabel;
        m_pIconLabel = nullptr;
    }

    m_pIconLabel = new QLabel(this);
    m_pIconLabel->setPixmap(QPixmap(icon));
    m_pIconLabel->setGeometry(179, 49, 24, 24);
    m_pIconLabel->setVisible(true);
}

void Message_Box::addWidget(QWidget* pWidget)
{
    m_pLabel->hide();
    m_pGridLayout->addWidget(pWidget, 0, 1, 2, 1);
}




void Message_Box::about(QWidget* parent, const QString& title, const QString& text)
{
    Message_Box msgBox(parent, title, text);
    msgBox.initAsAbout();
    msgBox.setText(text);
	
    if (msgBox.exec() == -1)
    {
        return;
    }
}

// 主菜单的about弹出版本升级对话框.
void Message_Box::aboutUpdate(QWidget* parent, const QString& title, const QString& text)
{
    Message_Box msgBox(parent, title, text);
    msgBox.initAsAboutUpdate();
    msgBox.setText(text);

    if (msgBox.exec() == -1)
    {
        return;
    }
}

// 界面发送升级请求后提示需要重启生效的提示信息.
void Message_Box::aboutUpdatePrompt(QWidget* parent, const QString& title, const QString& text)
{
    Message_Box msgBox(parent, title, text);
    msgBox.initAsAboutUpdatePrompt();
    //msgBox.setText(text);

    if (msgBox.exec() == -1)
    {
        return;
    }
}

QMessageBox::StandardButton Message_Box::critical(QWidget* parent, const QString& title, const QString& text)
{
	Message_Box msgBox(parent, title, text);
	msgBox.initAsCritical();
	msgBox.setTitle(title);
	msgBox.setText(text);

	auto temp = msgBox.exec();

	if (temp == 0)
	{
		return QMessageBox::Cancel;
	}

	return QMessageBox::StandardButton(temp);
}

QMessageBox::StandardButton Message_Box::question(QWidget* parent, const QString& title, const QString& text, enum Message_Box_Type questionType)
{
	Message_Box msgBox(parent, title, text);
	msgBox.initAsQuestion(questionType);
    msgBox.setTitle(title);
    msgBox.setText(text);

    auto temp = msgBox.exec();

    if (temp == 0)
    {
        return QMessageBox::Cancel;
    }

    return QMessageBox::StandardButton(temp);
	//return msgBox.standardButton(msgBox.clickedButton());
   
}

QMessageBox::StandardButton Message_Box::information(QWidget* parent, const QString& title, const QString& text)
{
	Message_Box msgBox(parent, title, text);
    msgBox.initAsInformation();
	msgBox.setTitle(title);
	msgBox.setText(text);

	auto temp = msgBox.exec();

	if (temp == 0)
	{
		return QMessageBox::Cancel;
	}

	return QMessageBox::StandardButton(temp);
}

QMessageBox::StandardButton Message_Box::warning(QWidget* parent, const QString& title, const QString& text)
{
	Message_Box msgBox(parent, title, text);
	msgBox.initAsWarning();
	msgBox.setTitle(title);
	msgBox.setText(text);

	auto temp = msgBox.exec();

	if (temp == 0)
	{
		return QMessageBox::Cancel;
	}

	return QMessageBox::StandardButton(temp);
}

QMessageBox::StandardButton Message_Box::error(QWidget* parent, const QString& title, const QString& text)
{
	Message_Box msgBox(parent, title, text);
	msgBox.initAsError();
	msgBox.setTitle(title);
	msgBox.setText(text);

	auto temp = msgBox.exec();

	if (temp == 0)
	{
		return QMessageBox::Cancel;
	}

	return QMessageBox::StandardButton(temp);
}
