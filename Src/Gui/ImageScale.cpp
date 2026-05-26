#include "Gui/ImageScale.h"

#include <QDir>
#include "Core/String.h"
#include "Core/Logging.h"

#include "Util/TaskProcess.h"

namespace AI3D
{
	namespace GUI
	{
		ImageScale::ImageScale(QObject* parent)
			:stop_(false),
			QObject(parent)
		{

		}
		
		ImageScale::~ImageScale()
		{

		}

		void ImageScale::run()
		{
			
				int i = 0;
				int totalNum = fileName.size();


								
				for (auto item : fileName) {
					QString fileNewName = outFilePath + "/" + QFileInfo(item).fileName();
					if (!QFileInfo(fileNewName).exists()) {
						QImage image(item);
						if (image.isNull()) {
							LOGW("image is NULL i = %d", i);
						}
						QString hashName = outFilePath + "/" + "";

						std::string hashfile = AI3D::CORE::String::ToSHA256(qstr2str(fileNewName));

						std::string hash_path = hashfile.substr(0, 2);

						std::string hash_filename = hashfile.substr(2);
						std::string path = hash_path + "/" + hash_filename;
						
						QString thumbnailImagepath = QDir(QString(path.c_str())).absolutePath();
						QDir().mkpath(thumbnailImagepath);
						
						Sleep(1);
					}
					else
						Sleep(10);


					i++;
					emit finish(item, fileNewName, i, totalNum);
				}
			
		}

	}
}
