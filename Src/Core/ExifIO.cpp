#include "Core/ExifIO.h"
#include "Core/String.h"
#include "Core/Bitmap.h"
#include <glog/logging.h>
#include <boost/regex.hpp>
#include <Core/ReturnCode.h>
#include "Core/AlgorithmBase.h"
#include <vector>

namespace AI3D
{
	namespace CORE
	{

		inline double parseDoubleOrNaN(const std::string& str)
		{
			return str == "" ? NAN : std::stod(str);
		}
		ExifIO::ExifIO() :bHaveExifInfo_(false), bHaveXmpData_(false), image_(nullptr)
		{};

		ExifIO::ExifIO(const std::string& sFileName) :bHaveExifInfo_(false) ,bHaveXmpData_(false)
		{
			if (Open(sFileName) != REGULAR_IMAGE)
			{
				LOGI("Invalid %s", sFileName.c_str());
			}
		};




		
		const std::string GPS_Longitude = "Exif.GPSInfo.GPSLongitude";
		const std::string GPS_Latitude = "Exif.GPSInfo.GPSLatitude";
		const std::string GPS_Altitude = "Exif.GPSInfo.GPSAltitude";
		
		std::string AltitudeToExiivGps(const std::string& altitude)
		{
			std::string temp = altitude;
			
			if(!String::StringContains(altitude, "."))
			
			{
				return altitude + "/1";
			}
			std::string fz, after, front;
			auto tList = String::StringSplit(altitude, ".");
			
			
			front = tList[0];
			
			after = tList.at(1) + "0000";
			after = after.substr(0, 4);
			
			fz = front + after;
			return fz + "/10000";
		}

		
		std::string  GetDDMMSS(const std::string& degree, int n, bool bEnd = false)
		{
			auto list = String::StringSplit(degree, ".");
			
			if (n == 1)
			{
				if (bEnd == false)
					return list.at(0);
				else
					return degree;
			}
			while (n--)
			{
				std::string temp = list.at(1);
				int len = temp.length();
				double d = 60.0 * std::atof(temp.c_str()) / (double)std::pow(10, len);
				std::ostringstream ss;
				ss.setf(std::ios::fixed);
				ss.precision(12);
				ss << d;
				return GetDDMMSS(ss.str(), n, bEnd);
			}
		}
		
		std::vector<std::string> DegreeToDDMMSS(const std::string& degree)
		{
			std::vector<std::string> qlist;
			std::string dd, mm, ss;
			if (!String::StringContains(degree, "."))
			
			{
				dd = degree;
			}
			else
			{
				dd = GetDDMMSS(degree, 1);
				mm = GetDDMMSS(degree, 2);
				ss = GetDDMMSS(degree, 3, true);
			}
			qlist.push_back(dd);
			qlist.push_back(mm);
			qlist.push_back(ss);
			
			return qlist;
		}

		
		std::string  DDMMSSToExivGps(std::vector<std::string>& strList)
		{

			std::string  dd, mm, ss;
			dd = strList.at(0) + "/1 ";
			mm = strList.at(1) + "/1 ";
			ss = strList.at(2);
			ss = AltitudeToExiivGps(ss);
			return dd + mm + ss;
		}
		bool AddExifGPSInfo(const std::string& keyStr, const std::string& value, Exiv2::ExifData& m_ed)
		{
			std::vector<std::string> tempList;
			std::string  tempValue;
			if (keyStr == "Exif.GPSInfo.GPSAltitude")
			{
				tempValue = AltitudeToExiivGps(value);
			}
			else
			{
				tempList = DegreeToDDMMSS(value);
				tempValue = DDMMSSToExivGps(tempList);
			}
			std::string _keyStr = keyStr;
			std::string _value = tempValue;
			Exiv2::ExifKey tmp = Exiv2::ExifKey(_keyStr);
			Exiv2::ExifData::iterator pos = m_ed.findKey(tmp);
			
			if (pos == m_ed.end())
			{
				Exiv2::URationalValue::UniquePtr rv(new Exiv2::URationalValue);
				
				rv->read(_value);
				Exiv2::ExifKey key = Exiv2::ExifKey(_keyStr);
				m_ed.add(key, rv.get());
			}
			else
			{
				
				Exiv2::Value::UniquePtr v = pos->getValue();
				
				Exiv2::URationalValue* prv = dynamic_cast<Exiv2::URationalValue*>(v.release());
				if (prv == 0) {
					throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Downcast failed");
					
				}
				Exiv2::URationalValue::UniquePtr rv(prv);
				rv->read(_value);
				pos->setValue(rv.get());
			}

			return true;
		}



		void ExifIO::Write()
		{
			image_->writeMetadata();
		}

		int ExifIO::Open(const std::string& sFileName)
		{
			
			
			
			
			
			
			try
			{
				std::vector<unsigned char> fileBytes;
				if (!File::ReadBinaryFileUtf8(sFileName, fileBytes) || fileBytes.empty())
				{
					LOGE("Failed to read image file: %s", sFileName.c_str());
					return ERROR_IMAGE;
				}
				image_ = Exiv2::ImageFactory::open(
					reinterpret_cast<const Exiv2::byte*>(fileBytes.data()),
					fileBytes.size());
				if (!image_) {
					LOGE("Failed to open image: %s", sFileName.c_str());
					return ERROR_IMAGE;
				}

				
				
				
				
				

					
				
				image_->readMetadata();
				
			
				Exiv2::ExifData& ed = image_->exifData();

				if (ed.empty())
				{
					LOGW(String::StringPrintf("%s has no exif file", sFileName.c_str()));
					return NOEXIF_IMAGE;
				}

				
				exifinfo_.imagePath = sFileName;
				
				exifinfo_.make = GetExifValueString(ed, "Exif.Image.Make");
				exifinfo_.model = GetExifValueString(ed, "Exif.Image.Model");
				exifinfo_.dateTime = GetExifValueString(ed, "Exif.Image.DateTime");

				
				exifinfo_.width = GetExifValueLong(ed, "Exif.Photo.PixelXDimension");
				exifinfo_.height = GetExifValueLong(ed, "Exif.Photo.PixelYDimension");

				if (exifinfo_.width == -1 || exifinfo_.height == -1)
				{
					exifinfo_.width = GetExifValueLong(ed, "Exif.Image.ImageWidth");
					exifinfo_.height = GetExifValueLong(ed, "Exif.Image.ImageLength");
				}
				
				
				

				
				
				exifinfo_.focalLength = GetExifValueFloat(ed, "Exif.Photo.FocalLength");
				
				exifinfo_.focalLengthIn35mm = GetExifValueFloat(ed, "Exif.Photo.FocalLengthIn35mmFilm");
			
				
				exifinfo_.Orientation = GetExifValueFloat(ed, "Exif.Image.Orientation");

				
				double lon = -DBL_MAX;
				double lat = -DBL_MAX;
				double alt = -DBL_MAX;
				bool bgps = false;

				std::vector<std::string> strs = { "Exif.GPSInfo.GPSLongitude" ,"Exif.GPSInfo.GPSLatitude" , "Exif.GPSInfo.GPSAltitude" };

				double fgps[3] = { -DBL_MAX,-DBL_MAX,-DBL_MAX };

				for (int i = 0; i < strs.size(); i++) {
					std::string strRet = GetExifValueString(ed, strs[i]);
					if (!strRet.empty()) {
						std::vector<std::string> strList = String::StringSplit(strRet, " ");
						double fval = 0;
						for (int j = 0; j < strList.size(); j++) {
							std::vector<std::string> stri = String::StringSplit(strList.at(j), "/");
							if (stri.size() == 2)
								fval += std::atof(stri[0].c_str()) / std::atof(stri[1].c_str()) / (pow(60, j));
						}
						fgps[i] = fval;
					}
				}

				exifinfo_.longitude = fgps[0];
				exifinfo_.latitude = fgps[1];
				exifinfo_.altitude = fgps[2];
				bHaveExifInfo_ = true;

				
				Exiv2::XmpParser::initialize();

				
				::atexit(Exiv2::XmpParser::terminate);
				
				Exiv2::XmpData xmpData;
				xmpData = image_->xmpData();
				
				
				
				
				
				
				
				
				
				
			
					std::string make = xmpData["Xmp.tiff.Make"].toString();
					String::StringToUpper(&make);
				
				if (!xmpData.empty()&& String::StringContains(make,"DJI"))
				{
				
					
					{
					
						
				
						xmpdata_.AbsoluteAltitude = parseDoubleOrNaN(xmpData["Xmp.drone-dji.AbsoluteAltitude"].toString());
						xmpdata_.RelativeAltitude = parseDoubleOrNaN(xmpData["Xmp.drone-dji.RelativeAltitude"].toString());
						xmpdata_.GimbalRollDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.GimbalRollDegree"].toString());
						xmpdata_.GimbalYawDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.GimbalYawDegree"].toString());
						xmpdata_.GimbalPitchDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.GimbalPitchDegree"].toString());
						xmpdata_.FlightRollDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightRollDegree"].toString());
						xmpdata_.FlightYawDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightYawDegree"].toString());
						xmpdata_.FlightPitchDegree = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightPitchDegree"].toString());
						xmpdata_.FlightXSpeed = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightXSpeed"].toString());
						xmpdata_.FlightYSpeed = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightYSpeed"].toString());
						xmpdata_.FlightZSpeed = parseDoubleOrNaN(xmpData["Xmp.drone-dji.FlightZSpeed"].toString());
						xmpdata_.GpsLatitude = parseDoubleOrNaN(xmpData["Xmp.drone-dji.GpsLatitude"].toString());
						xmpdata_.GpsLongitude = parseDoubleOrNaN(xmpData["Xmp.drone-dji.GpsLongitude"].toString());

						xmpdata_.RtkFlag = parseDoubleOrNaN(xmpData["Xmp.drone-dji.RtkFlag"].toString());
						xmpdata_.DewarpFlag = parseDoubleOrNaN(xmpData["Xmp.drone-dji.DewarpFlag"].toString());
						xmpdata_.DewarpData = parseDoubleOrNaN(xmpData["Xmp.drone-dji.DewarpData"].toString());
						xmpdata_.isValid = true;
						if (!xmpdata_.DewarpData.empty())
						{
							auto calib_params = String::StringSplit(xmpdata_.DewarpData, ";,");
							for (int i = 1; i < calib_params.size(); i++)
							{
								xmpdata_.pre_calib_params.push_back(std::stof(calib_params[i]));
							}
						}

						const double yaw = xmpdata_.GimbalYawDegree;
						const double pitch = xmpdata_.GimbalPitchDegree;
						const double roll = xmpdata_.GimbalRollDegree;
						if (!std::isnan(yaw) && !std::isnan(pitch) && !std::isnan(roll))
						{
							exifinfo_.rotation = AlgorithmBase::YPRToRotationInner(Eigen::Vector3d(
								yaw * M_PI / 180.0, pitch * M_PI / 180.0, roll * M_PI / 180.0));

						}

						bHaveXmpData_ = true;
					}
				
				
				
				}
				Exiv2::XmpParser::terminate();
				return REGULAR_IMAGE;
			}
			catch (Exiv2::Error& e)
			{
				LOGE(String::StringPrintf("Caught Exiv2 exception %s", e.what()));
				return ERROR_IMAGE;
			}
			catch (std::exception& e)
			{
				LOGE("Caught std::exception: %s", e.what());
				return ERROR_IMAGE;
			}
		}

		bool ExifIO::Open_Beta(const std::string& sFileName)
		{
			if (!File::IsFileExistent(sFileName))
			{
				LOGE(String::StringPrintf("Image filepath %s is not exist!", sFileName));
				return false;
			}

			Bitmap bitmap;
			bitmap.Read(sFileName);
			bitmap.ExifFocalLength(&exifinfo_.focalLength, &exifinfo_.focalLengthIn35mm, &exifinfo_.sensor_width, exifinfo_.make, exifinfo_.model);
			bitmap.ExifAltitude(&exifinfo_.altitude);
			bitmap.ExifLatitude(&exifinfo_.latitude);
			bitmap.ExifLongitude(&exifinfo_.longitude);

			exifinfo_.width = bitmap.GetHeight();
			exifinfo_.height = bitmap.GetWidth();

			
			
			
			
			
			
			
			
			
			
			
			
			
			
			return true;
		}

		int ExifIO::GetWidth() const
		{
			return exifinfo_.width;
		}

		int ExifIO::GetHeight() const
		{
			return exifinfo_.height;
		}

		double ExifIO::GetFocal() const
		{
			return exifinfo_.focalLength;
		}
		ExifInfo& ExifIO::GetExifInfoMutual()
		{
			return exifinfo_;
		}

		void ExifIO::SetFocal(const double& focalLength)
		{
			(image_->exifData())["Exif.Photo.FocalLength"] = toExifString(focalLength);
		}

		double ExifIO::GetFocalLengthIn35mm() const
		{
			return exifinfo_.focalLengthIn35mm;
		}

		void ExifIO::SetFocalLengthIn35mm(const double& focalLengthIn35mm)
		{
			(image_->exifData())["Exif.Photo.FocalLengthIn35mmFilm"] = int16_t(focalLengthIn35mm);
		}

		double ExifIO::GPSLatitude() const
		{
			return exifinfo_.latitude;
		}


		
		void ExifIO::SetLatitude(double lat)
		{
			AddExifGPSInfo("Exif.GPSInfo.GPSLatitude",std::to_string(lat), image_->exifData());

			
		}

		double ExifIO::GPSLongitude() const
		{
			return exifinfo_.longitude;
		}

		void ExifIO::SetLongitude(double lon)
		{
			AddExifGPSInfo("Exif.GPSInfo.GPSLongitude", std::to_string(lon), image_->exifData());
			
		}
		double ExifIO::GPSAltitude() const
		{
			return exifinfo_.altitude;
		}

		void ExifIO::SetAltitude(double alt)
		{
			AddExifGPSInfo("Exif.GPSInfo.GPSAltitude", std::to_string(alt), image_->exifData());
		
		}
		bool ExifIO::DoesHaveExifInfo() const
		{
			return bHaveExifInfo_;
		}

		std::string ExifIO::GetBrand() const
		{
			return exifinfo_.make;
		}

		void ExifIO::SetBrand(const std::string& brand)
		{
			
			
			
			
			
			
			
			
			
			(image_->exifData())["Exif.Image.Make"] = brand;
		}

		std::string ExifIO::GetModel() const
		{
			return exifinfo_.model;
		}

		void ExifIO::SetModel(const std::string& model)
		{
			
			
			
			
			
			
			
			
			
			(image_->exifData())["Exif.Image.Model"] = model;
		}

		Eigen::Matrix3d ExifIO::GetRotation()
		{
			return exifinfo_.rotation;
		}
		short ExifIO::GetOrientation()const
		{
			return exifinfo_.Orientation;
		}
		void ExifIO::SetOrientation(const short orientation)
		{
			(image_->exifData())["Exif.Image.Orientation"] = orientation;
		}

		
		std::string ExifIO::GetExifValueString(Exiv2::ExifData& ed, std::string key)
		{
			Exiv2::ExifKey tmp = Exiv2::ExifKey(key);
			Exiv2::ExifData::iterator pos = ed.findKey(tmp);
			if (pos == ed.end())
				return "";
			return pos->value().toString();
		}

		long ExifIO::GetExifValueLong(Exiv2::ExifData& ed, std::string key)
		{
			Exiv2::ExifKey tmp = Exiv2::ExifKey(key);
			Exiv2::ExifData::iterator pos = ed.findKey(tmp);
			if (pos == ed.end())
				return -1;
			
			return pos->value().toUint32();
		}

		float ExifIO::GetExifValueFloat(Exiv2::ExifData& ed, std::string key) {
			Exiv2::ExifKey tmp = Exiv2::ExifKey(key);
			Exiv2::ExifData::iterator pos = ed.findKey(tmp);
			if (pos == ed.end())
				return -1;
			return pos->value().toFloat();
		}

		Exiv2::Image::UniquePtr& ExifIO::GetImagePtr()
		{
			return image_;
		}
		std::string ExifIO::GetDateTime()const
		{
			return exifinfo_.dateTime;
		}
		std::string ExifIO::GetDateTime()
		{
			return exifinfo_.dateTime;
		}
		std::string ExifIO::toExifString(double d)
		{
			char result[200];
			d *= 100;
			sprintf(result, "%d/100", abs((int)d));
			return std::string(result);
		}

		std::string ExifIO::toExifString(double d, bool bRational, bool bLat)
		{  
			const char* gDeg = NULL; 
			const char* NS = d >= 0.0 ? "N" : "S";
			const char* EW = d >= 0.0 ? "E" : "W";
			const char* NSEW = bLat ? NS : EW;
			if (d < 0) d = -d;
			int deg = (int)d;
			d -= deg;
			d *= 60;
			int min = (int)d;
			d -= min;
			d *= 60;
			int sec = (int)d;
			double secd = d;
			char result[1024];
			if (bRational)
				sprintf(result, "%d/1 %d/1 %f/1", deg, min, secd);
			else
				sprintf(result, "%03d%s%02d'%02d\"%s", deg, gDeg, min, sec, NSEW);
			std::cout << std::string(result) << " ===" << result << std::endl;
			return std::string(result);
		}
		bool ExifIO::DoesHaveXMPData()const
		{
			return bHaveXmpData_;
		}
		XmpData ExifIO::AllXMPData()const
		{
			return xmpdata_;
		}
	}
}