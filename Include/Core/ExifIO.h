
#ifndef _AI3D_EXIF_IO_H_
#define _AI3D_EXIF_IO_H_

#include <memory>
#include <string>
#include <Constants.h>
#include <exiv2/exiv2.hpp>
#include "Core/String.h"
#include "Core/Types.h"
#include "Core/File.h"

namespace AI3D
{
	namespace CORE
	{
		struct XmpData
		{
			XmpData()
			{
				clear();
			}
			void clear()
			{
				AbsoluteAltitude = -DBL_MAX;
				RelativeAltitude = -DBL_MAX;
				GpsLatitude = -DBL_MAX;
				GpsLongitude = -DBL_MAX;
				GimbalRollDegree = -DBL_MAX;
				GimbalYawDegree = -DBL_MAX;
				GimbalPitchDegree = -DBL_MAX;
				FlightRollDegree = -DBL_MAX;
				FlightYawDegree = -DBL_MAX;
				FlightPitchDegree = -DBL_MAX;
				FlightXSpeed = -DBL_MAX;
				FlightYSpeed = -DBL_MAX;
				FlightZSpeed = -DBL_MAX;
				RtkFlag = -DBL_MAX;
				DewarpFlag = -DBL_MAX;
				DewarpData = "";
				pre_calib_params.clear();
				isValid = false;
			};
			double AbsoluteAltitude;
			double RelativeAltitude;
			double GpsLatitude;
			double GpsLongitude;
			double GimbalRollDegree;
			double GimbalYawDegree;
			double GimbalPitchDegree;
			double FlightRollDegree;
			double FlightYawDegree;
			double FlightPitchDegree;
			double FlightXSpeed;
			double FlightYSpeed;
			double FlightZSpeed;
			double RtkFlag;
			double DewarpFlag;
			std::vector<double> pre_calib_params;
			std::string DewarpData;
			bool isValid = false;
		};
		struct ExifInfo 
		{
			ExifInfo() {
				clear();
			}
			void clear() {
				imagePath.clear();
				make.clear();
				model.clear();
				dateTime.clear();

				width = 0;
				height = 0;
				focalLengthIn35mm = 0.0;
				focalLength = 0;
				sensor_width = 0.0;
				longitude = -DBL_MAX;
				latitude = -DBL_MAX;
				altitude = -DBL_MAX;
				Orientation = 0;
				rotation.setConstant(NAN);
			};
			std::string imagePath = "";
			
			std::string make = "";
			std::string model = "";
			std::string dateTime = "";
			int width;
			int height;
			
			short Orientation;
			double focalLength = 0.0;
			double focalLengthIn35mm = 0.0;
			double sensor_width = 0.0;
			
			double longitude = -DBL_MAX;
			double latitude = -DBL_MAX;
			double altitude = -DBL_MAX;

			Eigen::Matrix3d rotation ;
		};

		
		class AI3D_API ExifIO
		{
		public:

			
			ExifIO();

			
			explicit ExifIO(const std::string& sFileName);

			void Write();
			
			int Open(const std::string& sFileName);

			
			bool Open_Beta(const std::string& sFileName);
			
			int GetWidth() const;

			
			int GetHeight() const;

			
			double GetFocal() const;
			void SetFocal(const double& focalLength);

			
			double GetFocalLengthIn35mm() const;
			void SetFocalLengthIn35mm(const double &focalLengthIn35mm);
			
			double GetFocalPlaneXResolution() const;

			
			double GetFocalPlaneYResolution() const;

			
			int GetFocalPlaneResolutionUnit() const;

			
			std::string GetBrand() const;
			void SetBrand(const std::string& brand);
			
			std::string GetModel() const;
			void SetModel(const std::string &model);
			
			std::string GetLensModel() const ;

			
			std::string GetImageUniqueID() const;

			
			bool DoesHaveExifInfo() const;

			
			std::string AllExifData() const;
			
			bool DoesHaveXMPData()const;
			XmpData AllXMPData()const;
			
			double GPSLatitude() const;
			void SetLatitude(double lat);
			
			double GPSLongitude() const;
			void SetLongitude(double lon);
			
			double GPSAltitude() const;
			void SetAltitude(double alt);

			
			
			short GetOrientation()const;
			void SetOrientation(const short orientation);

			Eigen::Matrix3d GetRotation();

			
			std::string GetDateTime()const;
			std::string GetDateTime();

			std::string toExifString(double d);
			std::string toExifString(double d, bool bRational, bool bLat);

			std::string GetExifValueString(Exiv2::ExifData& ed, std::string key);

			long GetExifValueLong(Exiv2::ExifData& ed, std::string key);

			float GetExifValueFloat(Exiv2::ExifData& ed, std::string key);

			Exiv2::Image::UniquePtr& GetImagePtr();

			ExifInfo& GetExifInfoMutual();
		private:
			
			ExifInfo exifinfo_;
			XmpData xmpdata_;
			Exiv2::Image::UniquePtr image_;
			
			bool bHaveExifInfo_ ;
			bool bHaveXmpData_;
		};
		
	} 
} 

#endif 