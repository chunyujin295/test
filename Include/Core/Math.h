#ifndef _AI3D_CORE_MATH_H_
#define _AI3D_CORE_MATH_H_

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <list>
#include <stdexcept>
#include <vector>

#include "Core/Logging.h"
#include <Eigen/Core>
#include "opencv2/opencv.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define FD2R(d) ((d)*M_PI/180.f)
#define R2FD(d) ((d)*180.f/M_PI)
namespace 
{
	namespace 
	{
		
		
		std::vector<cv::Point2i> GetArrowPoints(const cv::Point& Start, const cv::Point& End, const double& Width, const double NoseWidth, const double& ArrowSizeL)
		{
			std::vector<cv::Point2i> ArrowPoints;
			double dx = End.x - Start.x;
			double dy = End.y - Start.y;

			double angle;
			if (dx == 0)
			{
				angle = M_PI / 2;
			}
			else
			{
				
				angle = std::atan(std::abs(dy / dx));
				if (dx < 0 && dy < 0)
				{
					
					angle = M_PI + angle;
				}
				else if (dx < 0 && dy>0)
				{
					
					angle = M_PI - angle;
				}
				else if (dx > 0 && dy < 0)
				{
					
					angle = M_PI * 2 - angle;
				}
			}
			const double d1 = Width;
			const double d2 = d1 + NoseWidth;
			const double SinAngle = std::sin(angle);
			const double CosAngle = std::cos(angle);

			cv::Point2i A = { int(Start.x + d1 * SinAngle), int(Start.y - d1 * CosAngle) };
			cv::Point2i B = { int(Start.x - d1 * SinAngle),int(Start.y + d1 * CosAngle) };

			cv::Point2i EndMid = { int(End.x - ArrowSizeL * CosAngle), int(End.y - ArrowSizeL * SinAngle) };
			cv::Point2i C = { int(EndMid.x + d1 * SinAngle), int(EndMid.y - d1 * CosAngle) };
			cv::Point2i D = { int(EndMid.x - d1 * SinAngle), int(EndMid.y + d1 * CosAngle) };
			cv::Point2i E = { int(EndMid.x + d2 * SinAngle), int(EndMid.y - d2 * CosAngle) };
			cv::Point2i F = { int(EndMid.x - d2 * SinAngle), int(EndMid.y + d2 * CosAngle) };

			ArrowPoints.push_back(A);
			ArrowPoints.push_back(C);
			ArrowPoints.push_back(E);
			ArrowPoints.push_back(End);
			ArrowPoints.push_back(F);
			ArrowPoints.push_back(D);
			ArrowPoints.push_back(B);
			return ArrowPoints;
		}

		
		bool DrawArrow(cv::Mat& img, const cv::Point& Start, const cv::Point& End, const cv::Scalar&RGB, double width = 5, double nosewidth = 5, double arrowsizel = 10)
		{
			
			int h = img.rows;
			int w = img.cols;
			
			cv::Mat mask1(h, w, CV_8UC1, cv::Scalar(0));

			cv::imshow("origin", img);
			
			auto ArrowPoints = GetArrowPoints(Start, End, width, nosewidth, arrowsizel);

			

			
			
			cv::polylines(mask1, ArrowPoints, true, cv::Scalar(0));

			cv::imshow("mask1", mask1);
			
			cv::findContours(mask1, ArrowPoints, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

			for (const auto& cnt : ArrowPoints)
			{
				cv::fillPoly(img, ArrowPoints, RGB);
			}
			
			
			
			

			
			return true;
		}

		
		template <typename T>
		int SignOfNumber(const T val);

		
		inline bool IsNaN(const float x);
		inline bool IsNaN(const double x);

		
		inline bool IsInf(const float x);
		inline bool IsInf(const double x);

		
		template <typename T>
		inline T Clip(const T& value, const T& low, const T& high);

		
		inline float DegToRad(const float deg);
		inline double DegToRad(const double deg);

		
		inline float RadToDeg(const float rad);
		inline double RadToDeg(const double rad);

		
		template <typename T>
		double Median(const std::vector<T>& elems);

		
		template <typename T>
		double Mean(const std::vector<T>& elems);

		
		template <typename T>
		double Variance(const std::vector<T>& elems);

		
		template <typename T>
		double StdDev(const std::vector<T>& elems);

		
		template <typename T>
		bool AnyLessThan(std::vector<T> elems, T threshold);

		
		template <typename T>
		bool AnyGreaterThan(std::vector<T> elems, T threshold);

		
		
		
		
		template <class Iterator>
		bool NextCombination(Iterator first, Iterator middle, Iterator last);

		
		template <typename T>
		T Sigmoid(const T x, const T alpha = 1);

		
		
		
		
		
		
		
		
		
		template <typename T>
		T ScaleSigmoid(T x, const T alpha = 1, const T x0 = 10);

		
		size_t NChooseK(const size_t n, const size_t k);

		
		
		template <typename T1, typename T2>
		T2 TruncateCast(const T1 value);

		
		template <typename T>
		T Percentile(const std::vector<T>& elems, const double p);

		
		
		

		namespace internal {

			template <class Iterator>
			bool NextCombination(Iterator first1, Iterator last1, Iterator first2,
				Iterator last2) {
				if ((first1 == last1) || (first2 == last2)) {
					return false;
				}
				Iterator m1 = last1;
				Iterator m2 = last2;
				--m2;
				while (--m1 != first1 && *m1 >= *m2) {
				}
				bool result = (m1 == first1) && *first1 >= *m2;
				if (!result) {
					while (first2 != m2 && *m1 >= *first2) {
						++first2;
					}
					first1 = m1;
					std::iter_swap(first1, first2);
					++first1;
					++first2;
				}
				if ((first1 != last1) && (first2 != last2)) {
					m1 = last1;
					m2 = first2;
					while ((m1 != first1) && (m2 != last2)) {
						std::iter_swap(--m1, m2);
						++m2;
					}
					std::reverse(first1, m1);
					std::reverse(first1, last1);
					std::reverse(m2, last2);
					std::reverse(first2, last2);
				}
				return !result;
			}

		}  

		template <typename T>
		int SignOfNumber(const T val) {
			return (T(0) < val) - (val < T(0));
		}



		
		inline bool DoubleNearSig(double a, double b, int significantDigits = 10)
		{
			const bool aIsNan = std::isnan(a);
			const bool bIsNan = std::isnan(b);
			if (aIsNan || bIsNan)
				return aIsNan && bIsNan;

			
			
			
			
			
			int aexp, bexp;
			const double ar = std::frexp(a, &aexp);
			const double br = std::frexp(b, &bexp);

			return aexp == bexp &&
				std::round(ar * std::pow(10.0, significantDigits)) == std::round(br * std::pow(10.0, significantDigits));
		}

		
		template<typename T>
		inline bool NumberNear(T a, T b, T epsilon = std::numeric_limits<T>::epsilon() * 4)
		{
			const bool aIsNan = std::isnan(a);
			const bool bIsNan = std::isnan(b);
			if (aIsNan || bIsNan)
				return aIsNan && bIsNan;

			const T diff = a - b;
			return diff >= -epsilon && diff <= epsilon;
		}

		
		inline bool NanCompatibleEquals(double a, double b)
		{
			const bool aIsNan = std::isnan(a);
			const bool bIsNan = std::isnan(b);
			if (aIsNan || bIsNan)
				return aIsNan && bIsNan;

			return a == b;
		}


		
		inline bool qgsDoubleNear(double a, double b, double epsilon = 4 * std::numeric_limits<double>::epsilon())
		{
			return NumberNear<double>(a, b, epsilon);
		}


		
		inline bool FloatNear(float a, float b, float epsilon = 4 * FLT_EPSILON)
		{
			return NumberNear<float>(a, b, epsilon);
		}


		inline double Round(double number, int places)
		{
			const double m = (number < 0.0) ? -1.0 : 1.0;
			const double scaleFactor = std::pow(10.0, places);
			return (std::round(number * m * scaleFactor) / scaleFactor) * m;
		}

		bool IsNaN(const float x) { return x != x; }
		bool IsNaN(const double x) { return x != x; }

		bool IsInf(const float x) { return !IsNaN(x) && IsNaN(x - x); }
		bool IsInf(const double x) { return !IsNaN(x) && IsNaN(x - x); }

		template <typename T>
		T Clip(const T& value, const T& low, const T& high) {
			return std::max(low, std::min(value, high));
		}

		float DegToRad(const float deg) {
			return deg * 0.0174532925199432954743716805978692718781530857086181640625f;
		}

		double DegToRad(const double deg) {
			return deg * 0.0174532925199432954743716805978692718781530857086181640625;
		}

		
		float RadToDeg(const float rad) {
			return rad * 57.29577951308232286464772187173366546630859375f;
		}

		double RadToDeg(const double rad) {
			return rad * 57.29577951308232286464772187173366546630859375;
		}

		template <typename T>
		double Median(const std::vector<T>& elems)
		{
			if (!CHECK_OPTION(!elems.empty()))
			{
				return -DBL_MAX;
			}

			const size_t mid_idx = elems.size() / 2;

			std::vector<T> ordered_elems = elems;
			std::nth_element(ordered_elems.begin(), ordered_elems.begin() + mid_idx,
				ordered_elems.end());

			if (elems.size() % 2 == 0) {
				const T mid_element1 = ordered_elems[mid_idx];
				const T mid_element2 = *std::max_element(ordered_elems.begin(),
					ordered_elems.begin() + mid_idx);
				return (mid_element1 + mid_element2) / 2.0;
			}
			else {
				return ordered_elems[mid_idx];
			}
		}

		template <typename T>
		T Percentile(const std::vector<T>& elems, const double p) {
			CHECK_OPTION(!elems.empty());
			CHECK_OPTION_GE(p, 0);
			CHECK_OPTION_LE(p, 100);

			const int idx = static_cast<int>(std::round(p / 100 * (elems.size() - 1)));
			const size_t percentile_idx =
				std::max(0, std::min(static_cast<int>(elems.size() - 1), idx));

			std::vector<T> ordered_elems = elems;
			std::nth_element(ordered_elems.begin(),
				ordered_elems.begin() + percentile_idx, ordered_elems.end());

			return ordered_elems.at(percentile_idx);
		}

		template <typename T>
		double Mean(const std::vector<T>& elems) {
			CHECK_LOG(!elems.empty());
			double sum = 0;
			for (const auto el : elems) {
				sum += static_cast<double>(el);
			}
			return sum / elems.size();
		}

		template <typename T>
		double Variance(const std::vector<T>& elems) {
			const double mean = Mean(elems);
			double var = 0;
			for (const auto el : elems) {
				const double diff = el - mean;
				var += diff * diff;
			}
			return var / (elems.size() - 1);
		}

		template <typename T>
		double StdDev(const std::vector<T>& elems) {
			return std::sqrt(Variance(elems));
		}

		template <typename T>
		bool AnyLessThan(std::vector<T> elems, T threshold) {
			for (const auto& el : elems) {
				if (el < threshold) {
					return true;
				}
			}
			return false;
		}

		template <typename T>
		bool AnyGreaterThan(std::vector<T> elems, T threshold) {
			for (const auto& el : elems) {
				if (el > threshold) {
					return true;
				}
			}
			return false;
		}

		template <class Iterator>
		bool NextCombination(Iterator first, Iterator middle, Iterator last) {
			return internal::NextCombination(first, middle, middle, last);
		}

		template <typename T>
		T Sigmoid(const T x, const T alpha) {
			return T(1) / (T(1) + exp(-x * alpha));
		}

		template <typename T>
		T ScaleSigmoid(T x, const T alpha, const T x0) {
			const T t0 = Sigmoid(-x0, alpha);
			const T t1 = Sigmoid(x0, alpha);
			x = (Sigmoid(2 * x0 * x - x0, alpha) - t0) / (t1 - t0);
			return x;
		}

		template <typename T1, typename T2>
		T2 TruncateCast(const T1 value) {
			return std::min(
				static_cast<T1>(std::numeric_limits<T2>::max()),
				std::max(static_cast<T1>(std::numeric_limits<T2>::min()), value));
		}
	}
} 

#endif  
