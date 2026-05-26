/**
  * @file      ImageScale.h
  * @brief     …˙≥…ÕºœÒ‘§¿¿Õº¿‡
  * @details
  * @author    ly
  * @attention
  */

#ifndef IMAGE_SCALE_H
#define IMAGE_SCALE_H

#include <QObject>
#include <QRunnable>
#include <QFileInfo>
#include <QImage>
#include "windows.h"
namespace AI3D
{
	namespace GUI
	{

		class ImageScale :public QObject, public QRunnable
		{
			Q_OBJECT
		public:
			explicit ImageScale(QObject* parent = nullptr);
			~ImageScale();
			void start() { stop_ = false; };
			void stop() { stop_ = true; };
			void setFileName(QStringList _fileName) { fileName = _fileName; };
			void setOutFilePath(QString _outFilePath) { outFilePath = _outFilePath; };
			void run();
		signals:
			void finish(QString& oldFileName, QString& fileName, int& num, int& totalNum);
		private:
			QStringList fileName;
			QString outFilePath;
			bool stop_;
		};
	}
}
#endif // !1