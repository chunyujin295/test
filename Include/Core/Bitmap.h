#ifndef _AI3D_CORE_BITMAP_H_
#define _AI3D_CORE_BITMAP_H_

#include <algorithm>
#include <cmath>
#include <ios>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <Constants.h>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <FreeImage.h>

#include "Core/string.h"

namespace AI3D
{

	namespace CORE
	{

		
		template <typename T>
		struct BitmapColor 
		{
			BitmapColor();
			BitmapColor(const T gray);
			BitmapColor(const T r, const T g, const T b);

			template <typename D>
			BitmapColor<D> Cast() const;

			bool operator==(const BitmapColor<T>& rhs) const;
			bool operator!=(const BitmapColor<T>& rhs) const;

			template <typename D>
			friend std::ostream& operator<<(std::ostream& output,
				const BitmapColor<D>& color);

			T r;
			T g;
			T b;
		};

		
		class AI3D_API Bitmap 
		{
		public:
			Bitmap();

			
			Bitmap(const Bitmap& other);
			
			Bitmap(Bitmap&& other);

			
			
			explicit Bitmap(FIBITMAP* data);

			
			Bitmap& operator=(const Bitmap& other);
			
			Bitmap& operator=(Bitmap&& other);

			
			bool Allocate(const int width, const int height, const bool as_rgb);

			
			void Deallocate();

			
			inline const FIBITMAP* GetData() const;
			inline FIBITMAP* GetDataMutual();

			
			inline int GetWidth() const;
			inline int GetHeight() const;
			inline int GetChannels() const;

			
			inline unsigned int GetBitsPerPixel() const;

			
			
			inline unsigned int GetScanWidth() const;

			
			inline bool IsRGB() const;
			inline bool IsGrey() const;

			
			size_t GetNumBytes() const;

			
			std::vector<uint8_t> ConvertToRawBits() const;
			std::vector<uint8_t> ConvertToRowMajorArray() const;
			std::vector<uint8_t> ConvertToColMajorArray() const;

			
			
			bool GetPixel(const int x, const int y, BitmapColor<uint8_t>* color) const;
			bool SetPixel(const int x, const int y, const BitmapColor<uint8_t>& color);

			
			const uint8_t* GetScanline(const int y) const;

			
			
			void Fill(const BitmapColor<uint8_t>& color);

			
			bool InterpolateNearestNeighbor(const double x, const double y,
				BitmapColor<uint8_t>* color) const;
			bool InterpolateBilinear(const double x, const double y,
				BitmapColor<float>* color) const;

			
			
			bool ExifFocalLength(double* focal_length, double* focal_length_35, double* sensor_width, std::string& make_str, std::string& model_str);
			bool ExifLatitude(double* latitude);
			bool ExifLongitude(double* longitude);
			bool ExifAltitude(double* altitude);

			
			bool Read(const std::string& path, const bool as_rgb = true);

			
			
			bool Write(const std::string& path,
				const FREE_IMAGE_FORMAT format = FIF_UNKNOWN,
				const int flags = 0) const;

			
			void Smooth(const float sigma_x, const float sigma_y);

			
			void Rescale(const int new_width, const int new_height,
				const FREE_IMAGE_FILTER filter = FILTER_BILINEAR);

			void Rescale(const int new_width, const int new_height, Bitmap& image);
			void Rescale(unsigned nMaxResolution);
			
			Bitmap Clone() const;
			Bitmap CloneAsGrey() const;
			Bitmap CloneAsRGB() const;

			
			void CloneMetadata(Bitmap* target) const;

			
			bool ReadExifTag(const FREE_IMAGE_MDMODEL model, const std::string& tag_name,
				std::string* result) const;

		private:
			typedef std::unique_ptr<FIBITMAP, decltype(&FreeImage_Unload)> FIBitmapPtr;

			void SetPtr(FIBITMAP* data);

			static bool IsPtrGrey(FIBITMAP* data);
			static bool IsPtrRGB(FIBITMAP* data);
			static bool IsPtrSupported(FIBITMAP* data);

			FIBitmapPtr data_;
			int width_;
			int height_;
			int channels_;
		};

		
		
		class AI3D_API JetColormap 
		{
		public:
			static float GetRed(const float gray);
			static float GetGreen(const float gray);
			static float GetBlue(const float gray);

		private:
			static float GetInterpolate(const float val, const float y0, const float x0,
				const float y1, const float x1);
			static float GetBase(const float val);
		};

		
		
		

		namespace internal 
		{

			template <typename T1, typename T2>
			T2 BitmapColorCast(const T1 value) 
			{
				return std::min(static_cast<T1>(std::numeric_limits<T2>::max()),
					std::max(static_cast<T1>(std::numeric_limits<T2>::min()),std::round(value)));
			}

		}  

		template <typename T>
		BitmapColor<T>::BitmapColor() : r(0), g(0), b(0) 
		{

		}

		template <typename T>
		BitmapColor<T>::BitmapColor(const T gray) : r(gray), g(gray), b(gray) 
		{

		}

		template <typename T>
		BitmapColor<T>::BitmapColor(const T r, const T g, const T b)
			: r(r), g(g), b(b) 
		{

		}

		template <typename T>
		template <typename D>
		BitmapColor<D> BitmapColor<T>::Cast() const 
		{
			BitmapColor<D> color;
			color.r = internal::BitmapColorCast<T, D>(r);
			color.g = internal::BitmapColorCast<T, D>(g);
			color.b = internal::BitmapColorCast<T, D>(b);
			return color;
		}

		template <typename T>
		bool BitmapColor<T>::operator==(const BitmapColor<T>& rhs) const 
		{
			return r == rhs.r && g == rhs.g && b == rhs.b;
		}

		template <typename T>
		bool BitmapColor<T>::operator!=(const BitmapColor<T>& rhs) const 
		{
			return r != rhs.r || g != rhs.g || b != rhs.b;
		}

		template <typename T>
		std::ostream& operator<<(std::ostream& output, const BitmapColor<T>& color) 
		{
			output << StringPrintf("RGB(%f, %f, %f)", static_cast<double>(color.r),
				static_cast<double>(color.g),
				static_cast<double>(color.b));
			return output;
		}
	}
} 

#endif  
