#include "Gui/LoginDialog.h"

#pragma execution_character_set("utf-8")

namespace AI3D
{
	namespace GUI
	{
		LoginDialog::LoginDialog(QDialog* parent)
			: QDialog(parent)
		{
			ui.setupUi(this);
			this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
		}

		LoginDialog::~LoginDialog()
		{
            
		}

		void LoginDialog::exitAll() {
			// 询问用户是否真的要关闭对话框
			this->close();
			emit closeAll();
		}

		void LoginDialog::login() {
            QString username = ui.input_username->text();
            QString password = ui.input_psw->text();
            UserLoginInfo userInfo;
            userInfo.account = username;
            userInfo.password = password;
            HttpReply reply = userLogin(userInfo);
            if (reply.code == 0) {
                QString text = "登录成功\n";
                ui.label_errinfo->setText(text);
				checkAuth();
				this->close();
                }
            else {
                QString text = "登录失败:";
                QString errMsg = reply.data.value("errorMsg").toString();
                text = text + errMsg;
                ui.label_errinfo->setText(text);
            }
		}

		void LoginDialog::regist() {
            QString url = SERVER_HOST + "/account/regist";
            QDesktopServices::openUrl(QUrl(url));
		}

		void LoginDialog::forgetPsw() {
            QString url = SERVER_HOST + "/account/forgetpsw";
            QDesktopServices::openUrl(QUrl(url));
		}

		void LoginDialog::checkLogin() {
			if (!isLogin()) {
				//检测登录状态，未登录则弹登录窗口
				this->show();
			}
			else {
				//检查授权
				checkAuth();
			}
		}

        void LoginDialog::checkAuth() {
			//检测是否有授权
			QString localLicense = getLocalLicense();
			if (!checkLicenseAvaliable(localLicense)) {
				//本地授权无效
				HttpReply reply = getLicense();
				if (reply.code != 0) {
					//获取授权失败
					if (reply.code == 21002) {
						//账号无任何授权，询问是否试用
						qDebug() << "error 21002" << "\n";
						QMessageBox* msgBox = new QMessageBox();
						QString question = "当前账号无授权，是否激活使用版本？";
						msgBox->setText("授权检测");
						msgBox->setInformativeText(question);
						msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
						msgBox->button(QMessageBox::Yes)->setText("激活");
						msgBox->button(QMessageBox::No)->setText("退出");
						msgBox->setDefaultButton(QMessageBox::No); // 设置默认按钮
						// 显示消息框并获取用户的选择
						int ret = msgBox->exec();
						// 根据用户的选择执行相应的操作
						if (ret == QMessageBox::Yes) {
							// 用户点击了“是”
							HttpReply getRet = getDefaultLicense();
							if (getRet.code == 0) {
								//获取到默认license
								//msgBox->close();
								qDebug() << "get default license" << "\n";
							}
							else {
								QMessageBox errBox;
								errBox.warning(this, "授权检测", "当前账号无授权，请稍后再试");
								userLogout();
								this->show();
							}
						}
						else if (ret == QMessageBox::No) {
							// 用户点击了“否”
							//msgBox->close();
							qDebug() << "user dont use try" << "\n";
							//不试用，退出登录
							QMessageBox errBox;
							errBox.warning(this, "授权检测", "当前账号无授权，请获得权限后再用");
							userLogout();
							this->show();
						}
						else {
							msgBox->close();
							QMessageBox errBox;
							errBox.warning(this, "授权检测", "当前账号获取授权错误，请稍后再试");
							userLogout();
							this->show();
						}

					}
					else if (reply.code == 21003) {
						qDebug() << "error 21003" << "\n";
						//授权数达上限，询问是否购买，还是替换
						QMessageBox* msgBox = new QMessageBox();
						QString question = "当前账号授权数量达到上限，请选择购买更高级账号，或者替换现有机器的权限：";
						msgBox->setText("授权检测");
						msgBox->setInformativeText(question);
						msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
						msgBox->button(QMessageBox::Yes)->setText("升级账号");
						msgBox->button(QMessageBox::No)->setText("切换授权");
						msgBox->button(QMessageBox::Cancel)->setText("退出");
						msgBox->setDefaultButton(QMessageBox::No); // 设置默认按钮
						// 显示消息框并获取用户的选择
						int ret = msgBox->exec();
						// 根据用户的选择执行相应的操作
						if (ret == QMessageBox::Yes) {
							//购买新的授权
							QString url = SERVER_HOST + "/account/login";
							QDesktopServices::openUrl(QUrl(url));
							QMessageBox errBox;
							errBox.warning(this, "授权检测", "当前账号无授权，请更新权限后重新登录");
							userLogout();
							this->show();
						}
						else if (ret == QMessageBox::No) {
							//替换授权
							QString url = SERVER_HOST + "/account/user";
							QDesktopServices::openUrl(QUrl(url));
							QMessageBox errBox;
							errBox.warning(this, "授权检测", "当前账号无授权，请修改权限后重新登录");
							userLogout();
							this->show();
						}
						else {
							//用户不做改变
							msgBox->close();
							QMessageBox errBox;
							errBox.warning(this, "授权检测", "当前账号无授权，请稍后再试");
							userLogout();
							this->show();
						}
					}
					else {
						//未查到可用授权，提示用户，稍后再试，退出登录
						QMessageBox msgBox;
						msgBox.question(this, "授权检测", "当前账号获取授权错误，请稍后再试");
						userLogout();
						this->show();
					}

				}
			}
        }

	}
}