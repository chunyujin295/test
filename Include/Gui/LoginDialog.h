#pragma once
#include <QDialog>
#include "ui_LoginDialog.h"
#include <QObject>
#include "Util/HttpClient.h"
#include "Util/DeviceInfo.h"
#include <QSettings>
#include <QDesktopServices>
#include "Util/User.h"
#include "Util/constant.h"
#include <QMessageBox>

namespace AI3D
{
	namespace GUI
	{
		class LoginDialog : public QDialog
		{
			Q_OBJECT

		public:
			LoginDialog(QDialog* parent = Q_NULLPTR);
			~LoginDialog();
			void checkAuth();
			void checkLogin();

		signals:
			void closeAll();

		private slots:
			void login();
			void regist();
			void forgetPsw();
			void exitAll();
			
		private:
			Ui::LoginDialogClass ui;

		};
	}
}

