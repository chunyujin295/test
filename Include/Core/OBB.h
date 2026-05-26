






#ifndef __CORE_OBB_H__
#define __CORE_OBB_H__
#include <Eigen/Dense>
#include <Eigen/Core>
#include "Core/Types.h"














		
		template <typename TYPE, int DIMS>
		class TOBB
		{
			

		public:
			typedef TYPE Type;
			typedef Eigen::Matrix<TYPE, DIMS, 1> POINT;
			typedef Eigen::Matrix<TYPE, DIMS, DIMS, Eigen::RowMajor> MATRIX;
			
		
			typedef unsigned ITYPE;
			typedef Eigen::Matrix<ITYPE, DIMS, 1> TRIANGLE;
			enum { numCorners = (DIMS == 1 ? 2 : (DIMS == 2 ? 4 : 8)) }; 
			enum { numScalar = (5 * DIMS) };

			MATRIX m_rot;	
			POINT m_pos;	
			POINT m_ext;	

			

			inline TOBB() {}
			inline TOBB(bool);
			
			inline TOBB(const MATRIX& rot, const POINT& ptMin, const POINT& ptMax);
			inline TOBB(const POINT* pts, size_t n);
			inline TOBB(const POINT* pts, size_t n, const TRIANGLE* tris, size_t s);

			
			inline void Set(const MATRIX& rot, const POINT& ptMin, const POINT& ptMax); 
			inline void Set(const POINT* pts, size_t n); 
			inline void Set(const POINT* pts, size_t n, const TRIANGLE* tris, size_t s); 
			inline void Set(const MATRIX& C, const POINT* pts, size_t n); 
			inline void SetRotation(const MATRIX& C); 
			inline void SetBounds(const POINT* pts, size_t n); 

			inline void BuildBegin(); 
			inline void BuildAdd(const POINT&); 
			inline void BuildEnd(); 

			inline bool IsValid() const;

			inline void Enlarge(TYPE);
			inline void EnlargePercent(TYPE);

			inline void Translate(const POINT&);
			inline void Transform(const MATRIX&);

			inline POINT GetCenter() const;
			inline void GetCenter(POINT&) const;

			inline POINT GetSize() const;
			inline void GetSize(POINT&) const;

			inline void GetCorners(POINT pts[numCorners]) const;
			

			inline TYPE GetVolume() const;

			bool Intersects(const POINT&) const;
			bool Intersects(const POINT&,  int compare_x, int compara_y) const;
			
			

			friend std::ostream& operator << (std::ostream& st, const TOBB& obb) {
				st << obb.m_rot; st << std::endl;
				st << obb.m_pos; st << std::endl;
				st << obb.m_ext; st << std::endl;
				return st;
			}
			friend std::istream& operator >> (std::istream& st, TOBB& obb) {
				st >> obb.m_rot;
				st >> obb.m_pos;
				st >> obb.m_ext;
				return st;
			}


		}; 
		
		template <typename TYPE, int DIMS>
		inline TOBB<TYPE, DIMS>::TOBB(bool ):
			m_rot(MATRIX::Identity()),
			m_pos(POINT::Zero()),
			m_ext(POINT::Zero())
		{
		}
		
		template <typename TYPE, int DIMS>
		inline TOBB<TYPE, DIMS>::TOBB(const MATRIX& rot, const POINT& ptMin, const POINT& ptMax)
		{
			Set(rot, ptMin, ptMax);
		}
		template <typename TYPE, int DIMS>
		inline TOBB<TYPE, DIMS>::TOBB(const POINT* pts, size_t n)
		{
			Set(pts, n);
		}
		template <typename TYPE, int DIMS>
		inline TOBB<TYPE, DIMS>::TOBB(const POINT* pts, size_t n, const TRIANGLE* tris, size_t s)
		{
			Set(pts, n, tris, s);
		} 
		

		
		
		


		

		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Set(const MATRIX& rot, const POINT& ptMin, const POINT& ptMax)
		{
			m_rot = rot;
			m_pos = (ptMax + ptMin) * TYPE(0.5);
			m_ext = (ptMax - ptMin) * TYPE(0.5);
		}

		
		

		
		
		
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Set(const POINT* pts, size_t n)
		{
			

			
			
			
			
			
			POINT mu(POINT::Zero());
			TYPE cxx = 0, cxy = 0, cxz = 0, cyy = 0, cyz = 0, czz = 0;
			for (size_t i = 0; i < n; ++i) {
				const POINT& p = pts[i];
				mu += p;
				cxx += p(0) * p(0);
				cxy += p(0) * p(1);
				cxz += p(0) * p(2);
				cyy += p(1) * p(1);
				cyz += p(1) * p(2);
				czz += p(2) * p(2);
			}
			const TYPE invN(TYPE(1) / TYPE(n));
			cxx = (cxx - mu(0) * mu(0) * invN) * invN;
			cxy = (cxy - mu(0) * mu(1) * invN) * invN;
			cxz = (cxz - mu(0) * mu(2) * invN) * invN;
			cyy = (cyy - mu(1) * mu(1) * invN) * invN;
			cyz = (cyz - mu(1) * mu(2) * invN) * invN;
			czz = (czz - mu(2) * mu(2) * invN) * invN;

			
			MATRIX C;
			C(0, 0) = cxx; C(0, 1) = cxy; C(0, 2) = cxz;
			C(1, 0) = cxy; C(1, 1) = cyy; C(1, 2) = cyz;
			C(2, 0) = cxz; C(2, 1) = cyz; C(2, 2) = czz;

			
			Set(C, pts, n);
		}
		
		
		
		
		
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Set(const POINT* pts, size_t n, const TRIANGLE* tris, size_t s)
		{
			ASSERT(n >= DIMS);

			
			
			POINT mu(POINT::Zero());
			TYPE Am = 0;
			TYPE cxx = 0, cxy = 0, cxz = 0, cyy = 0, cyz = 0, czz = 0;
			for (size_t i = 0; i < s; ++i) {
				ASSERT(tris[i](0) < n && tris[i](1) < n && tris[i](2) < n);
				const POINT& p = pts[tris[i](0)];
				const POINT& q = pts[tris[i](1)];
				const POINT& r = pts[tris[i](2)];
				const POINT mui = (p + q + r) / TYPE(3);
				const TYPE Ai = (q - p).cross(r - p).normalize() / TYPE(2);
				mu += mui * Ai;
				Am += Ai;

				
				const TYPE Ai12 = Ai / TYPE(12);
				cxx += (TYPE(9) * mui(0) * mui(0) + p(0) * p(0) + q(0) * q(0) + r(0) * r(0)) * Ai12;
				cxy += (TYPE(9) * mui(0) * mui(1) + p(0) * p(1) + q(0) * q(1) + r(0) * r(1)) * Ai12;
				cxz += (TYPE(9) * mui(0) * mui(2) + p(0) * p(2) + q(0) * q(2) + r(0) * r(2)) * Ai12;
				cyy += (TYPE(9) * mui(1) * mui(1) + p(1) * p(1) + q(1) * q(1) + r(1) * r(1)) * Ai12;
				cyz += (TYPE(9) * mui(1) * mui(2) + p(1) * p(2) + q(1) * q(2) + r(1) * r(2)) * Ai12;
				czz += (TYPE(9) * mui(2) * mui(2) + p(2) * p(2) + q(2) * q(2) + r(2) * r(2)) * Ai12;
			}

			
			
			mu /= Am;
			cxx /= Am; cxy /= Am; cxz /= Am; cyy /= Am; cyz /= Am; czz /= Am;

			
			cxx -= mu(0) * mu(0); cxy -= mu(0) * mu(1); cxz -= mu(0) * mu(2);
			cyy -= mu(1) * mu(1); cyz -= mu(1) * mu(2); czz -= mu(2) * mu(2);

			
			MATRIX C;
			C(0, 0) = cxx; C(0, 1) = cxy; C(0, 2) = cxz;
			C(1, 0) = cxy; C(1, 1) = cyy; C(1, 2) = cyz;
			C(2, 0) = cxz; C(1, 2) = cyz; C(2, 2) = czz;

			
			Set(C, pts, n);
		}
		
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Set(const MATRIX& C, const POINT* pts, size_t n)
		{
			
			SetRotation(C);
			
			SetBounds(pts, n);
		}
		
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::SetRotation(const MATRIX& C)
		{
			
			const Eigen::SelfAdjointEigenSolver<MATRIX> es(C);
			
			
			
			
			m_rot = es.eigenvectors().transpose();
			if (m_rot.determinant() < 0)
				m_rot = -m_rot;
		}
		
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::SetBounds(const POINT* pts, size_t n)
		{
			
			

			
			const TYPE tmax = std::numeric_limits<TYPE>::max();
			POINT minim(tmax, tmax, tmax), maxim(-tmax, -tmax, -tmax);
			for (size_t i = 0; i < n; ++i) {
				const POINT p_prime(m_rot * pts[i]);
				if (minim(0) > p_prime(0)) minim(0) = p_prime(0);
				if (minim(1) > p_prime(1)) minim(1) = p_prime(1);
				if (minim(2) > p_prime(2)) minim(2) = p_prime(2);
				if (maxim(0) < p_prime(0)) maxim(0) = p_prime(0);
				if (maxim(1) < p_prime(1)) maxim(1) = p_prime(1);
				if (maxim(2) < p_prime(2)) maxim(2) = p_prime(2);
			}

			
			
			
			const POINT center((maxim + minim) * TYPE(0.5));
			m_pos = m_rot.transpose() * center;
			m_ext = (maxim - minim) * TYPE(0.5);
		} 
		


		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::BuildBegin()
		{
			m_rot = MATRIX::Zero();
			m_pos = POINT::Zero();
			m_ext = POINT::Zero();
		}
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::BuildAdd(const POINT& p)
		{
			
			m_pos += p;
			
			m_rot(0, 0) += p(0) * p(0);
			m_rot(0, 1) += p(0) * p(1);
			m_rot(0, 2) += p(0) * p(2);
			m_rot(1, 0) += p(1) * p(1);
			m_rot(1, 1) += p(1) * p(2);
			m_rot(1, 2) += p(2) * p(2);
			
			++(*((size_t*)m_ext.data()));
		}
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::BuildEnd()
		{
			const TYPE invN(TYPE(1) / TYPE(*((size_t*)m_ext.data())));
			const TYPE cxx = (m_rot(0, 0) - m_pos(0) * m_pos(0) * invN) * invN;
			const TYPE cxy = (m_rot(0, 1) - m_pos(0) * m_pos(1) * invN) * invN;
			const TYPE cxz = (m_rot(0, 2) - m_pos(0) * m_pos(2) * invN) * invN;
			const TYPE cyy = (m_rot(1, 0) - m_pos(1) * m_pos(1) * invN) * invN;
			const TYPE cyz = (m_rot(1, 1) - m_pos(1) * m_pos(2) * invN) * invN;
			const TYPE czz = (m_rot(1, 2) - m_pos(2) * m_pos(2) * invN) * invN;

			
			MATRIX C;
			C(0, 0) = cxx; C(0, 1) = cxy; C(0, 2) = cxz;
			C(1, 0) = cxy; C(1, 1) = cyy; C(1, 2) = cyz;
			C(2, 0) = cxz; C(2, 1) = cyz; C(2, 2) = czz;
			SetRotation(C);
		} 
		


		
		template <typename TYPE, int DIMS>
		inline bool TOBB<TYPE, DIMS>::IsValid() const
		{
			return m_ext.minCoeff() > TYPE(0);
		} 
		


		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Enlarge(TYPE x)
		{
			m_ext.array() -= x;
		}
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::EnlargePercent(TYPE x)
		{
			m_ext *= x;
		} 
		


		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Translate(const POINT& d)
		{
			m_pos += d;
		}
		
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::Transform(const MATRIX& m)
		{
			m_rot = m * m_rot;
			m_pos = m * m_pos;
		}
		


		template <typename TYPE, int DIMS>
		inline typename TOBB<TYPE, DIMS>::POINT TOBB<TYPE, DIMS>::GetCenter() const
		{
			return m_pos;
		}
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::GetCenter(POINT& ptCenter) const
		{
			ptCenter = m_pos;
		} 
		


		template <typename TYPE, int DIMS>
		inline typename TOBB<TYPE, DIMS>::POINT TOBB<TYPE, DIMS>::GetSize() const
		{
			return m_ext * 2;
		}
		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::GetSize(POINT& ptSize) const
		{
			ptSize = m_ext * 2;
		} 
		


		template <typename TYPE, int DIMS>
		inline void TOBB<TYPE, DIMS>::GetCorners(POINT pts[numCorners]) const
		{
			if (DIMS == 2) {
				const POINT pEAxis[2] = {
					m_rot.row(0) * m_ext[0],
					m_rot.row(1) * m_ext[1]
				};
				pts[0] = m_pos - pEAxis[0] - pEAxis[1];
				pts[1] = m_pos + pEAxis[0] - pEAxis[1];
				pts[2] = m_pos + pEAxis[0] + pEAxis[1];
				pts[3] = m_pos - pEAxis[0] + pEAxis[1];
			}
			if (DIMS == 3) {
				const POINT pEAxis[3] = {
					m_rot.row(0) * m_ext[0],
					m_rot.row(1) * m_ext[1],
					m_rot.row(2) * m_ext[2]
				};
				pts[0] = m_pos - pEAxis[0] - pEAxis[1] - pEAxis[2];
				pts[1] = m_pos - pEAxis[0] - pEAxis[1] + pEAxis[2];
				pts[2] = m_pos + pEAxis[0] - pEAxis[1] - pEAxis[2];
				pts[3] = m_pos + pEAxis[0] - pEAxis[1] + pEAxis[2];
				pts[4] = m_pos + pEAxis[0] + pEAxis[1] - pEAxis[2];
				pts[5] = m_pos + pEAxis[0] + pEAxis[1] + pEAxis[2];
				pts[6] = m_pos - pEAxis[0] + pEAxis[1] - pEAxis[2];
				pts[7] = m_pos - pEAxis[0] + pEAxis[1] + pEAxis[2];
			}
		} 
		
		
		
		

		
		
		

		
		

		
		
		template <typename TYPE, int DIMS>
		inline TYPE TOBB<TYPE, DIMS>::GetVolume() const
		{
			return m_ext.prod() * numCorners;
		}
		
		template <typename TYPE, int DIMS>
		bool TOBB<TYPE, DIMS>::Intersects(const POINT& pt,int compare_x,int compara_y ) const
		{
			const POINT dist(m_rot * (pt - m_pos));
			if (DIMS == 2) {
				return std::abs(dist[0]) <= m_ext[0]
					&& std::abs(dist[1]) <= m_ext[1];
			}
			if (DIMS == 3)
			{
				
				return std::abs(dist[compare_x]) <= m_ext[compare_x]
					&& std::abs(dist[compara_y]) <= m_ext[compara_y]
					;
			}
		} 

		template <typename TYPE, int DIMS>
		bool TOBB<TYPE, DIMS>::Intersects(const POINT& pt) const
		{
			const POINT dist(m_rot * (pt - m_pos));
			if (DIMS == 2) {
				return std::abs(dist[0]) <= m_ext[0]
					&& std::abs(dist[1]) <= m_ext[1];
			}
			if (DIMS == 3) {
				return std::abs(dist[0]) <= m_ext[0]
					&& std::abs(dist[1]) <= m_ext[1]
					&& std::abs(dist[2]) <= m_ext[2];
			}
		} 





#endif 
