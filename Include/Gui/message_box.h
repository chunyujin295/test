#pragma once
#ifndef MESSAGE_BOX
#define MESSAGE_BOX

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include "ui_message_box.h"
#include <QDialog>

class QLabel;
class QMessageBox;

enum Message_Box_Type
{
	NoIcon = 0,
	Information = 1,
	Warning = 2,
	Critical = 3,
	Question_Yes_No = 4,
    Question_Yes_Cancel = 5,
    Question_Yes_No_Cancel= 6
};

class Message_Box : public QDialog
{
    Q_OBJECT

public:
    //Message_Box(QDialog* parent = 0) {};
    Message_Box(QWidget* parent = 0, const QString& title ="", const QString& text = "");
    ~Message_Box();

    void paintEvent(QPaintEvent* p1);
    QAbstractButton* clickedButton() const;
    QMessageBox::StandardButton standardButton(QAbstractButton* button) const;
    // 设置默认按钮
    void setDefaultButton(QPushButton* button);
    void setDefaultButton(QMessageBox::StandardButton button);
    // 设置窗体标题
    void setTitle(const QString& title);
    // 设置提示信息
    void setText(const QString& text);
    // 设置窗体图标
    void setIcon(const QString& icon);
    // 添加控件-替换提示信息所在的QLabel
    void addWidget(QWidget* pWidget);

	static QMessageBox::StandardButton information(QWidget* parent, const QString& title, const QString& text);
	static QMessageBox::StandardButton question(QWidget* parent, const QString& title, const QString& text, enum Message_Box_Type questionType = Message_Box_Type::Question_Yes_No);
	static QMessageBox::StandardButton warning(QWidget* parent, const QString& title,const QString& text);
    static QMessageBox::StandardButton error(QWidget* parent, const QString& title, const QString& text);
    static void about(QWidget* parent, const QString& title, const QString& text);
    static void aboutUpdate(QWidget* parent, const QString& title, const QString& text);
    static void aboutUpdatePrompt(QWidget* parent, const QString& title, const QString& text);
    static QMessageBox::StandardButton critical(QWidget* parent, const QString& title, const QString& text);

protected:
    // 多语言翻译
    void changeEvent(QEvent* event);

private slots:
    void onButtonClicked(QAbstractButton* button);
    void onYesButtonClicked();
    void onNoButtonClicked();
    void onCancelButtonClicked();
    void onCloseButtonClicked();
    void onDefaultButtonClicked();
    void onUpdateButtonClicked();

private:
    void translateUI();
    int execReturnCode(QAbstractButton* button);
    void initAsAbout();
    void initAsAboutUpdate();
    void initAsAboutUpdatePrompt();
    void initAsQuestion(enum Message_Box_Type questionType);
    void initAsInformation();
    void initAsError();
    void initAsWarning();
    void initAsCritical();

private:
    QLabel* m_pIconLabel;
    QLabel* m_pLabel;
    QLabel* m_pUpdateLabel;
    QGridLayout* m_pGridLayout;
    QWidget* m_pTitleWidget;
    QDialogButtonBox* m_pButtonBox;
    QAbstractButton* m_pClickedButton;
    QAbstractButton* m_pDefaultButton;
    QAbstractButton* m_pYesButton;
    QAbstractButton* m_pNoButton;
    QAbstractButton* m_pCancelButton;
    QAbstractButton* m_pCloseButton;
    QAbstractButton* m_pUpdateButton;
private:
    Ui::Message_Box ui;
};
#endif