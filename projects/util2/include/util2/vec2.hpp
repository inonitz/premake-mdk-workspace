#ifndef __UTIL2_VECTOR_MATH_LIBRARY2__
#define __UTIL2_VECTOR_MATH_LIBRARY2__
#include "util2/C/util2_api.h"
#include "util2/C/macro.h"
#include "util2/string.hpp"
#include "util2/C/ifcrash2.h"
#include <nmmintrin.h>
#include <array>



/*
	* Refactor due, need to make design goals
		and work accordingly
*/
namespace util2::math { /* Matrices/Vectors are stored in ROW MAJOR ORDER (LINEARLY IN MEMORY) */


template<typename T> T radians(T v);
template<typename T> T degrees(T v);
template<typename T> bool approximately_zero(T v);
template<typename T> static T pi = __scast(T, 3.141592653589793238462643383279502884197169399375105820974944);


template<typename T, u8 length> class Vector
{
private:
    T __data[util2::round2<u8>(length)];
    
public:
	void zero() { 
        for(u32 i = 0; i < __carraysize(__data); ++i) {
            __data[i] = __scast(T, 0x00); 
        }
        return;
    }


	constexpr Vector() { zero(); }
	constexpr Vector(const_ref<T> defaultVal) 
	{
		for(u32 i = 0; i < __carraysize(__data); i += 2)  {
			__data[i    ] = defaultVal;
			__data[i + 1] = defaultVal;
		}
		return;
	}


	T& 		 operator[](uint8_t idx) 	   { ifcrash((u32)idx >= length); return __data[idx]; }
	const T& operator[](uint8_t idx) const { ifcrash((u32)idx >= length); return __data[idx]; }

	      T* begin()       { return &__data[0];      }
	      T* end()         { return &__data[length]; }
	const T* begin() const { return &__data[0];      }
	const T* end()   const { return &__data[length]; }
	constexpr size_t bytes()  const { return sizeof(T) * length;  }
	constexpr size_t len()    const { return length;		      }
};




#define DEFINE_VECTOR_STRUCTURE(len, type_t, fmt, declare_union_structs, declare_n_args_ctor, ...) \
struct UTIL2_API vec##fmt \
{ \
    using __Mem = Vector<type_t, len>; \
    using ArrayType = std::array<type_t, len>; \
    using cref = vec##fmt const&; \
    union { \
        __Mem mem; \
        \
        \
        declare_union_structs; \
    }; \
\
\
    vec##fmt() : mem() {} \
    explicit vec##fmt(const type_t* validAddr); \
    explicit vec##fmt(type_t value) : mem(value) {} \
    declare_n_args_ctor \
    vec##fmt(ArrayType const& arr) : vec##fmt(arr.data()) {} \
    vec##fmt(__Mem const& vec) : mem(vec) {} \
	vec##fmt(vec##fmt   const& cpy) : mem(cpy.mem) {} \
    vec##fmt& operator=(const vec##fmt& cpy); \
\
\
    __Mem& operator+(vec##fmt const& b); \
    __Mem& operator-(vec##fmt const& b); \
    __Mem& operator*(vec##fmt const& b); \
    __Mem& operator/(vec##fmt const& b); \
	__Mem& operator*(type_t a); \
    __Mem& operator/(type_t a); \
    const __Mem& operator+(vec##fmt const& b) const; \
	const __Mem& operator-(vec##fmt const& b) const; \
	const __Mem& operator*(vec##fmt const& b) const; \
	const __Mem& operator/(vec##fmt const& b) const; \
	const __Mem& operator*(type_t const a) const; \
	const __Mem& operator/(type_t const a) const; \
	vec##fmt& operator+=(vec##fmt const& b); \
    vec##fmt& operator-=(vec##fmt const& b); \
    vec##fmt& operator*=(vec##fmt const& b); \
    vec##fmt& operator*=(type_t a); \
    vec##fmt& operator/=(type_t a); \
\
	type_t length() const; \
    type_t lensqr() const; \
    __Mem& normalized() const; \
	void   normalize(); \
\
\
    type_t&       operator[](uint8_t idx)      ; \
    const type_t& operator[](uint8_t idx) const; \
		  type_t* begin() { return mem.begin(); } \
	      type_t* end()   { return mem.end();   } \
	const type_t* begin() const { return mem.begin(); } \
	const type_t* end()   const { return mem.end();   } \
    constexpr u64 bytes() const { return mem.bytes(); } \
	char*  to_string() const; \
	size_t to_strbuf(char* buf, size_t bufsize) const;\
}; \
\
\
UTIL2_API vec##fmt::__Mem& operator*(type_t a, vec##fmt const& b); \
UTIL2_API vec##fmt::__Mem& operator-(vec##fmt const& b); \
UTIL2_API type_t dot(vec##fmt::cref a, vec##fmt::cref b); \


#define DEFINE_CROSSPROD_FUNC(type_t, fmt) UTIL2_API vec##fmt::__Mem& cross(vec##fmt const& a, vec##fmt const& b);
#define DEFINE_ROTATE_FUNC(type_t, fmt)    UTIL2_API vec##fmt::__Mem& rotate(vec##fmt const& v, vec##fmt const& unit, f32 theta);




DISABLE_WARNING_PUSH
DISABLE_WARNING_PEDANTIC
DEFINE_VECTOR_STRUCTURE( \
	2,
	float, 
	2f, 
	__m64 mmx;
	struct { float x;   float y;     };
	struct { float u;   float v;     };
	struct { float i;   float j;     };
	struct { float yaw; float pitch; };
	struct { float m0;  float m1;    },
	vec2f(float a, float b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}
	explicit vec2f(f64 a, f64 b)
	{
		x = __scast(f32, a);
		y = __scast(f32, b);
		return;
	}
	explicit vec2f(i32 a, i32 b)
	{
		x = __scast(f32, a);
		y = __scast(f32, b);
		return;
	}
	explicit vec2f(u32 a, u32 b) {
		x = __scast(f32, a);
		y = __scast(f32, b);
		return;
	}
)
DEFINE_VECTOR_STRUCTURE( \
	2,
	i32, 
	2i,
	i64 qword;
	struct { i32 x;   i32 y;     };
	struct { i32 u;   i32 v;     };
	struct { i32 i;   i32 j;     };
	struct { i32 yaw; i32 pitch; };
	struct { i32 m0;  i32 m1;    },
	vec2i(i32 a, i32 b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}
	vec2i(u32 a, u32 b) 
	{ 
		x = __scast(i32, a); 
		y = __scast(i32, b); 
		return; 
	}
	explicit vec2i(vec2f const& v) 
	{ 
		x = __scast(i32, v.x); 
		y = __scast(i32, v.y); 
		return; 
	}
)
DEFINE_VECTOR_STRUCTURE( \
	2,
	u32, 
	2u,
	u64 qword;
	struct { u32 x;   u32 y;     };
	struct { u32 u;   u32 v;     };
	struct { u32 i;   u32 j;     };
	struct { u32 yaw; u32 pitch; };
	struct { u32 m0;  u32 m1;    },
	vec2u(u32 a, u32 b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}
	vec2u(i32 a, i32 b) 
	{ 
		x = __scast(u32, a); 
		y = __scast(u32, b); 
		return; 
	}
	explicit vec2u(vec2i const& v) 
	{
		x = __scast(u32, v.x);
		y = __scast(u32, v.y);
		return;
	}
)


DEFINE_VECTOR_STRUCTURE( \
	4,
	float,
	4f,
	__m128 xmm;
	struct {
		__m64 mmxl;
		__m64 mmxh;
	};
	struct { float x;   float y;     float z;    float w;     }; 
	struct { float r;   float g;     float b;    float a;     };
	struct { float i;   float j;     float k;    float l;     };
	struct { float yaw; float pitch; float roll; float theta; };
	struct { float m0;  float m1;    float m2;   float m3;    },
	vec4f(float a, float b, float c, float d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	vec4f(__m128 mm) : xmm(mm) {}
)
DEFINE_VECTOR_STRUCTURE( \
	4,
	u32,
	4u,
	__m128i xmm;
	struct { u32 x;   u32 y;     u32 z;    u32 w;     }; 
	struct { u32 r;   u32 g;     u32 b;    u32 a;     };
	struct { u32 i;   u32 j;     u32 k;	   u32 l;     };
	struct { u32 yaw; u32 pitch; u32 roll; u32 theta; };
	struct { u32 m0;  u32 m1;    u32 m2;   u32 m3;    },
	vec4u(u32 a, u32 b, u32 c, u32 d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	explicit vec4u(i32 a, i32 b, i32 c, i32 d)
	{
		x = __scast(u32, a);
		y = __scast(u32, b);
		z = __scast(u32, c);
		w = __scast(u32, d);
		return;
	}
	vec4u(__m128i mm) : xmm(mm) {}
)
DEFINE_VECTOR_STRUCTURE( \
	4,
	i32,
	4i,
	__m128i xmm;
	struct { i32 x;   i32 y;     i32 z;    i32 w;     }; 
	struct { i32 r;   i32 g;     i32 b;    i32 a;     };
	struct { i32 i;   i32 j;     i32 k;	   i32 l;     };
	struct { i32 yaw; i32 pitch; i32 roll; i32 theta; };
	struct { i32 m0;  i32 m1;    i32 m2;   i32 m3;    },
	vec4i(i32 a, i32 b, i32 c, i32 d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	}
	explicit vec4i(vec2i a, vec2i b) : vec4i(a.x, a.y, b.x, b.y) {}
	vec4i(__m128i mm) : xmm(mm) {}
)


DEFINE_VECTOR_STRUCTURE( \
	3,
	float,
	3f,
	__m128 xmm;
	struct { float x;   float y;     float z;    };
	struct { float u;   float v;     float w;    };
	struct { float r;   float g;     float b;    };
	struct { float i;   float j;     float k;    };
	struct { float yaw; float pitch; float roll; };
	struct { float m0;  float m1;    float m2;   };
	vec4f homogenised,
	vec3f(float a, float b, float c) 
	{ 
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	vec3f(__m128 mm) : xmm(mm) {}
)
DEFINE_CROSSPROD_FUNC(float, 3f)
DEFINE_ROTATE_FUNC(float, 3f)

DEFINE_VECTOR_STRUCTURE( \
	3,
	u32,
	3u,
	__m128i xmm;
	struct { u32 x;   u32 y;     u32 z;    };
	struct { u32 u;   u32 v;     u32 w;    };
	struct { u32 r;   u32 g;     u32 b;    };
	struct { u32 i;   u32 j;     u32 k;    };
	struct { u32 yaw; u32 pitch; u32 roll; };
	struct { u32 m0;  u32 m1;    u32 m2;   };
	vec4u homogenised,
	vec3u(u32 a, u32 b, u32 c) 
	{
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	vec3u(i32 a, i32 b, i32 c)
	{
		x = __scast(u32, a);
		y = __scast(u32, b);
		z = __scast(u32, c);
		return;
	}
	vec3u(__m128i mm) : xmm(mm) {}
)
DEFINE_CROSSPROD_FUNC(u32, 3u)

DEFINE_VECTOR_STRUCTURE( \
	3,
	i32,
	3i,
	__m128i xmm;
	struct { i32 x;   i32 y;     i32 z;    };
	struct { i32 u;   i32 v;     i32 w;    };
	struct { i32 r;   i32 g;     i32 b;    };
	struct { i32 i;   i32 j;     i32 k;    };
	struct { i32 yaw; i32 pitch; i32 roll; };
	struct { i32 m0;  i32 m1;    i32 m2;   };
	vec4i homogenised,
	vec3i(i32 a, i32 b, i32 c) 
	{ 
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	}
	explicit vec3i(vec3u const& uv)
	{
		x = __scast(i32, uv.x);
		y = __scast(i32, uv.y);
		z = __scast(i32, uv.z);
		return;
	}
	vec3i(__m128i mm) : xmm(mm) {}
)
DEFINE_CROSSPROD_FUNC(i32, 3i)



template<typename T> struct matrixView {
    using cref = matrixView const&;


	T* m_buf;
	i32 m_rows; 
	i32 m_columns;


    matrixView<T>() : m_buf(nullptr) {}
    explicit matrixView(T* validAddr, i32 rows, i32 columns) {
		m_buf 	  = validAddr;
		m_rows 	  = rows;
		m_columns = columns;
		return;
	}
	matrixView(matrixView const& cpy) : 
		matrixView{ cpy.m_buf, cpy.m_rows, cpy.m_columns} {}
    matrixView& operator=(const matrixView& cpy) {
		m_buf 	  = cpy.m_buf;
		m_rows    = cpy.m_rows;
		m_columns = cpy.m_columns;
		return *this;
	}
    T&       operator[](i32 idx)       { return m_buf[idx]; }
    const T& operator[](i32 idx) const { return m_buf[idx]; }
    T&       operator()(i32 i, i32 j)       { return m_buf[j + i * m_rows]; }
    const T& operator()(i32 i, i32 j) const { return m_buf[j + i * m_rows]; }
    T&       operator()(vec2i const& idx)       { return m_buf[idx.j + idx.i * m_rows]; }
    const T& operator()(vec2i const& idx) const { return m_buf[idx.j + idx.i * m_rows]; }
};




struct UTIL2_API mat2f 
{
    using __Arr = std::array<float, 4>;
	using __Mem = Vector<float, 4>;
	union {
		Vector<float, 4> mem;
		struct { vec2f row[2]; };
		struct { float x0,   y0,  x1,  y1; };
		struct { float m00, m01, m10, m11; };
		vec4f homogenised;
	};


	mat2f() : mem() 
	{ 
		/* Identity Matrix initialization is pretty useful. */
		mem[0] = 1.0f; 
		mem[3] = 1.0f;
		return;
	}
	mat2f(const float* validAddr);
	mat2f(float value) : mem(value) {}
	mat2f(
		float x0, float y0, 
		float x1, float y1
	) : homogenised{ x0, y0, x1, y1 } {}
	mat2f(
		vec2f const& a, 
		vec2f const& b
	) : row{ a, b } {}
	mat2f(__Arr const& arr) : mat2f(arr.data()) {}
	mat2f(__Mem const& vec) : mem(vec) {}
	mat2f(mat2f const& cpy) : mem(cpy.mem) {}
	mat2f& operator=(const mat2f& cpy) {
		homogenised = cpy.homogenised;
		return *this;
	}


	__Mem& operator+(mat2f const& b);
	__Mem& operator-(mat2f const& b);
	__Mem& operator*(mat2f const& b);
	__Mem& operator*(float        b);
	__Mem& operator/(float        b);
		  vec2f  column    (uint8_t idx)			   ;
	const vec2f  column    (uint8_t idx) 		  const;
	      vec2f& operator[](uint8_t idx)			   ;
	const vec2f& operator[](uint8_t idx)		  const;
	float&       operator()(uint8_t i, uint8_t j)      ;
	const float& operator()(uint8_t i, uint8_t j) const;

		  float*     begin()        { return mem.begin(); }
		  float*     end()          { return mem.end();   }
	const float*     begin()  const { return mem.begin(); }
	const float*     end()    const { return mem.end();   }
	constexpr size_t bytes()  const { return mem.bytes(); }
	constexpr size_t length() const { return mem.len();   }
	char* 		  to_string() const;
};
UTIL2_API mat2f::__Mem& operator*(float a, mat2f const& b);




struct UTIL2_API mat4f 
{
	using __Mem = Vector<float, 16>;
	using __Arr = std::array<float, 16>;
	union {
		__Mem mem;
		struct { vec4f row[4];  };
		struct { vec3f row3[4]; };
		struct {
			float   x0, y0, z0, w0,
					x1, y1, z1, w1, 
					x2, y2, z2, w2, 
					x3, y3, z3, w3;
		};
		struct {
			float   m00, m01, m02, m03,
					m10, m11, m12, m13, 
					m20, m21, m22, m23, 
					m30, m31, m32, m33;
		};
	};


	mat4f() : mem() 
	{ 
		/* Identity Matrix initialization is pretty useful. */
		mem[ 0] = 1.0f; 
		mem[ 5] = 1.0f; 
		mem[10] = 1.0f; 
		mem[15] = 1.0f; 
		return;
	}
	mat4f(float value) : mem(value) {}
	mat4f(
		float x0, float y0, float z0, float w0, 
		float x1, float y1, float z1, float w1, 
		float x2, float y2, float z2, float w2, 
		float x3, float y3, float z3, float w3
	) {
		row[0] = { x0, y0, z0, w0 }; 
		row[1] = { x1, y1, z1, w1 }; 
		row[2] = { x2, y2, z2, w2 };
		row[3] = { x3, y3, z3, w3 };
		return;
	}
	mat4f(
		vec4f const& a, 
		vec4f const& b,
		vec4f const& c, 
		vec4f const& d
	) {
		row[0] = a;
		row[1] = b;
		row[2] = c;
		row[3] = d;
		return;
	}
	mat4f(mat2f const& promote) : mem() {
		mem[ 0] = promote.m00;
		mem[ 1] = promote.m01;
		mem[ 4] = promote.m10;
		mem[ 5] = promote.m11; 
		mem[10] = 1.0f; 
		mem[15] = 1.0f; 
	}
	mat4f(__Arr const& arr) : mat4f(arr.data()) {}
	mat4f(__Mem const& vec) : mem(vec) {}
	mat4f(mat4f const& cpy) : mem(cpy.mem) {}
	mat4f(const float* validAddr);
	mat4f& operator=(const mat4f& cpy);


	__Mem& operator+(mat4f const& b);
	__Mem& operator-(mat4f const& b);
	__Mem& operator*(mat4f const& b);
	__Mem& operator*(float        b);
	__Mem& operator/(float        b);
	      vec4f  column(uint8_t idx)           ;
	const vec4f  column(uint8_t idx)     const ;
	      vec4f& operator[](uint8_t idx)       ; 
	const vec4f& operator[](uint8_t idx) const ; 
	float&       operator()(uint8_t i, uint8_t j)      ;
	const float& operator()(uint8_t i, uint8_t j) const;

		  float*  begin()        { return mem.begin();  }
		  float*  end()          { return mem.end();    }
	const float*  begin()  const { return mem.begin();  }
	const float*  end()    const { return mem.end();    }
	const float*  data()   const { return mem.begin();  }
	constexpr u64 bytes()  const { return mem.bytes();  }
	constexpr u64 length() const { return mem.len();    }
	char* to_string() const;
};
UTIL2_API mat4f::__Mem& operator*(float a, mat4f const& b);
DISABLE_WARNING_POP


UTIL2_API void Multiply(vec4f const& a, mat4f const& b, vec4f& out);
UTIL2_API void Multiply(mat4f const& a, mat4f const& b, mat4f& out);
UTIL2_API void Multiply(vec2f const& a, mat2f const& b, vec2f& out);
UTIL2_API void Multiply(mat2f const& a, mat2f const& b, mat2f& out);


/*
	Returns the following matrix in mat4f& out:
	[ 1, 0, 0, 0 ],
	[ 0, 1, 0, 0 ],
	[ 0, 0, 1, 0 ],
	[ 0, 0, 0, 1 ]
*/
UTIL2_API void identity  (mat4f& out);


/*
	Returns the following matrix in mat4f& out (where t = translate):
	[ 1, 0, 0, t.x ],
	[ 0, 1, 0, t.y ],
	[ 0, 0, 1, t.z ],
	[ 0, 0, 0, 1   ]
*/
UTIL2_API void translate (vec3f const& translate, mat4f& out);


/*
	Returns the following matrix in mat4f& out (where s = scale):
	[ s.x, 0,   0,   0 ],
	[  0, s.y,  0,   0 ],
	[  0,  0,  s.z,  0 ],
	[  0,  0,   0,   1 ]
*/
UTIL2_API void scale(vec3f const& scale, mat4f& out);


/*
	Returns the following matrix in mat2f& out (where s = scale):
	[ s.x,  0  ],
	[  0,  s.y ]
*/
UTIL2_API void scale(vec2f const& scale, mat2f& out);


/*
	Returns the following matrix in mat2f out (where t = radians(angle) ):
	[ cos(t), -sin(t) ],
	[ sin(t),  cos(t) ]
*/
UTIL2_API void rotate(f32 angle, mat2f& out);


/*
	Returns the following matrix in mat4f out ( where axis = normalized vector ):
	[TODO FINISH]
	[link]: https://upload.wikimedia.org/math/f/b/a/fbaee547c3c65ad3d48112502363378a.png
*/
UTIL2_API void rotate(math::vec3f const& axis, f32 angleRadians, mat4f& out);

/*
	Returns the following matrix in mat4f& out (where 
	in = [
		m00, m01, m02, m03,
		m04, m05, m06, m07,
		m08, m09, m10, m11,
		m12, m13, m14, m15
	]):
	[ m00, m04, m08, m12 ],
	[ m01, m05, m09, m13 ],
	[ m02, m06, m10, m14 ],
	[ m03, m07, m11, m15 ]
*/
UTIL2_API void transposed(mat4f const& in, mat4f& out);


/*
	Same as tranposed(); Uses temporary mat4f to directly modify inout.
*/
UTIL2_API void transpose (mat4f& inout);


/* 
	Full Explanation & Derivation:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
	Quick Derivation (If Link doesn't work, or you're lazy :| ):
	A. glFrustum(left, right, top, bottom, near, far):
		[ 2n/(r - l)     0,      (r + l)/(r - l),      0      ],
		[     0,     2n/(t - b), (t + b)/(t - b),      0      ],
		[     0,         0,      (f + n)/(n - f), 2fn/(n - f) ],
		[     0,         0,            -1,             0,     ]
	B. Take glFrustum(...) and plug
		t = tan(fovy/2) * near
		b = -1 * t
		r = t * windowAspectRatio
		l = -1 * r

	C. Returns the following matrix in mat4f& out (where 
		A = aspectRatio,
		t = radians(fovy),
		n = near, 
		f = far
	):
	[ cot(t / 2) / A,      0, 	       0,         0     ],
	[      0,         cot(t / 2),      0,         0     ],
	[      0,   		   0,     (n+f)/(n-f) 2fn/(n-f) ],
	[	   0,              0,         -1,         0     ]

*/
UTIL2_API void perspective(
	float  aspectRatio, 
	float  fovy, 
	float  near, 
	float  far, 
	mat4f& out
);


/* 
	[NOTE]: Formula Calculated using 4x4 inverse formula + regex + python str replace.
	Returns the following matrix in mat4f& out (where m%d%d = in[i][j]):
	[ 1/m00,   0,     0,          0,        ]
	[   0,   1/m11,   0,          0,        ]
	[   0,     0,     0,        1/m32,      ]
	[   0,     0,   1/m23, -m22/(m23 * m32) ]
*/
UTIL2_API void inv_perspective(mat4f const& in, mat4f& out);



/*
	Explanation & Derivation:
	https://learnwebgl.brown37.net/08_projections/projections_ortho.html

	Returns the following matrix in mat4f& out (where
		l, r = leftRight[0], leftRight[1]
		t, b = topBottom[0], topBottom[1],
		n, f = nearFar[0]  ,   nearFar[1]
	):
	[ 2/(r - l),      0, 	    0, 	   (r + l)/(l - r) ],
	[     0,      2/(t - b),    0, 	   (t + b)/(b - t) ],
	[     0,          0,	2/(n - f), (f + n)/(n - f) ],
	[     0,          0,        0, 	   	      1 	   ]
*/
UTIL2_API void orthographic(
	vec2f  leftRight, 
	vec2f  topBottom, 
	vec2f  nearFar,
	mat4f& out
);


/*
	Same as function above, just easier interface.
*/
UTIL2_API void orthographic(
	f32 left,   f32 right, 
	f32 bottom, f32 top,
	f32 near,   f32 far,
	mat4f& out
);



/* 
	Using a Right Handed Coordinate System (+z towards viewer, +x is right, +y is up)
	 +y
	 |
	 |
	 |
	   _____ +x
	 /
	/
	+z (camera looks to origin)
	Compared to Left Handed:
		  +y
		    |
		    |
		    |
	+x _____|
		   /
    	  /
    	-z
	[NOTE]: This is how the coordinate systems look when looking from -Z to the Origin (Use your fingers for demonstration).

	This Function is the World->Camera Transform Matrix.
	given:
	V_world = V_cam * X, where X = Camera's Transform Matrix, Or Camera_Space->World_Space
	Because we want V_cam =>

	V_world * X^-1 = V_cam.

	In Row Major Convention, X = R * T, where T = translation, R = rotation across each basis in 3d.
		==> X^-1 = (R * T)^-1 = T^-1 * R^-1
		<==> View_Matrix = T^-1 * R^-1 = translate(-eyePos) * BasisVectorMatrix (RotateXYZ, ...)
	This link will help - https://stackoverflow.com/questions/349050/calculating-a-lookat-matrix?noredirect=1&lq=1
	NOTE ABOUT LINK CONTENTS: The Matrix is left Handed, but the math is the same.
	
	Resulting View Matrix (World->Camera):
		where xAxis = right, yAxis = newup, zAxis = forward
		[         xaxis.x          yaxis.x          zaxis.x  0 ]
		[         xaxis.y          yaxis.y          zaxis.y  0 ]
		[         xaxis.z          yaxis.z          zaxis.z  0 ]
		[ dot(xaxis,-eye)  dot(yaxis,-eye)  dot(zaxis,-eye)  1 ]
*/
UTIL2_API void lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/* 
	Inverse of a homogenous transform matrix (V) =
		[ xaxis.x  xaxis.y  xaxis.z  dot(xaxis, eye) ]
		[ yaxis.x  yaxis.y  yaxis.z  dot(yaxis, eye) ]
		[ zaxis.x  zaxis.y  zaxis.z  dot(zaxis, eye) ]
		[    0 	      0 	   0       	    1    	 ]
*/
UTIL2_API void inv_lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/* 
	Source Code: https://github.com/willnode/N-Matrix-Programmer/blob/master/Info/Matrix_4x4.txt
*/
UTIL2_API void inverse(
	mat4f const& in,
	mat4f&       out
);


/*
	Source (God Bless you!): https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
*/
UTIL2_API void inverseSimd(
	mat4f const& in,
	mat4f& 		 out
);


/* 
	Returns the 2D Model Matrix 'TRS' in mat2f out (where t = radians(rotationAngle), s = scale, t = translate):
	[ s.x * cos(t), -s.y * sin(t), t.x, 0 ],
	[ s.x * sin(t),  s.y * cos(t), t.y, 0 ],
	[     0,             0, 	    1,  0 ],
	[     0,             0, 	    0,  1 ]
*/
UTIL2_API void modelMatrix2d(
	math::vec2f const& translate,
	math::vec2f const& scale,
	f32 			   rotationAngle,
	math::mat4f& 	   out
);


} // namespace util2::math



#endif