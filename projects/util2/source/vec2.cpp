#include "util2/vec2.hpp"
#include "util2/C/compiler_warning.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <immintrin.h>


namespace util2::math {


template<typename T> T radians(T v) {
	return v * __scast(T, 0.017453292519943295); /* (value / 180) * PI */
}
template<typename T> T degrees(T v) {
	return v * __scast(T, 57.29577951308232); /* (value / PI) * 180 */
}
template<typename T> bool approximately_zero(T v) {
	return -std::nextafter<T>(0, 1) < v && v < std::nextafter<T>(0, 1);
}


template float radians<float>(float v);
template float degrees<float>(float v);
template bool  approximately_zero<float>(float v);


thread_local static Vector<float, 2 > temporaryBufferVec2f{};
thread_local static Vector<float, 3 > temporaryBufferVec3f{};
thread_local static Vector<float, 4 > temporaryBufferVec4f{};
thread_local static Vector<float, 4 > temporaryBufferMat2f{};
thread_local static Vector<float, 9 > temporaryBufferMat3f{};
thread_local static Vector<float, 16> temporaryBufferMat4f{};
thread_local static Vector<u32,   2 > temporaryBufferVec2u{};
thread_local static Vector<u32,   3 > temporaryBufferVec3u{};
thread_local static Vector<u32,   4 > temporaryBufferVec4u{};
thread_local static Vector<i32,   2 > temporaryBufferVec2i{};
thread_local static Vector<i32,   3 > temporaryBufferVec3i{};
thread_local static Vector<i32,   4 > temporaryBufferVec4i{};


#define SET_MULTIPLE_VALUES 4llu
#define normalizeIndex(i) (SET_MULTIPLE_VALUES * i)
#define ni(i) normalizeIndex(i)


#define INTRMD_FUNC_GENERATOR(op_symbol, name, arg1a0, arg1a1, arg1a2, arg1a3, arg1a4, ...) \
template<typename T, u8 len> void name( \
	Vector<T, len> const& a, \
    __VA_ARGS__            , \
	Vector<T, len>& 	  dst \
) { \
	static Vector<T, SET_MULTIPLE_VALUES> tmp{}; \
	size_t i  = 0; \
	\
	for(; i < (len / SET_MULTIPLE_VALUES); ++i) { \
		tmp[0] = a[ni(i)    ] op_symbol arg1a0; \
		tmp[1] = a[ni(i) + 1] op_symbol arg1a1; \
		tmp[2] = a[ni(i) + 2] op_symbol arg1a2; \
		tmp[3] = a[ni(i) + 3] op_symbol arg1a3; \
		std::memcpy(&dst.begin()[i], tmp.begin(), tmp.bytes()); \
	} \
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { dst[i] = a[i] op_symbol arg1a4; } \
	return; \
} \


#define GENERATE_VECTOR_OPERATOR_FUNCTIONS(op_symbol, name) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b[ni(i)], b[ni(i) + 1], b[ni(i) + 2], b[ni(i) + 3], b[i], Vector<T, len> const& b) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b, b, b, b, b, T b) \

GENERATE_VECTOR_OPERATOR_FUNCTIONS(+, add)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(-, sub)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(*, mul)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(/, div)

#undef DEFINE_VECTOR_STRUCTURE_ARGS
#undef GENERATE_VECTOR_OPERATOR_FUNCTIONS
#undef INTRMD_FUNC_GENERATOR


template<typename T> void cross_prod(
	Vector<T, 3> const& a, 
	Vector<T, 3> const& b, 
	Vector<T, 3>&       dst
) {
	dst[0] = a[1] * b[2] - b[1] * a[2];
	dst[1] = a[2] * b[0] - b[2] * a[0];
	dst[2] = a[0] * b[1] - b[0] * a[1];
	return;
}


template<typename T, u8 len> T dot_prod(
	Vector<T, len> const& a, 
	Vector<T, len> const& b
) {
	Vector<T, SET_MULTIPLE_VALUES> tmp;
	T aggregate{0x00};
	size_t i  = 0;

	for(; i < (len / SET_MULTIPLE_VALUES); ++i) {
		tmp[0] = a[ni(i)    ] * b[ni(i)    ];
		tmp[1] = a[ni(i) + 1] * b[ni(i) + 1];
		tmp[2] = a[ni(i) + 2] * b[ni(i) + 2];
		tmp[3] = a[ni(i) + 3] * b[ni(i) + 3];
		tmp[i    ] = tmp[i	  ] + tmp[i + 1];
		tmp[i + 1] = tmp[i + 2] + tmp[i + 3];
		tmp[i] 	   = tmp[i] 	+ tmp[i + 1];
		
		aggregate += tmp[i];
	}
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { aggregate += a[i] * b[i]; }
	return aggregate;
}

#undef normalizeIndex
#undef ni
#undef SET_MULTIPLE_VALUES


template<typename T, u8 len> void norm(Vector<T, len>& a) {
	T tmp = dot_prod(a, a); tmp = sqrt(tmp) / tmp;
	mul<T, len>(a, tmp, a);
	return;
}


template<typename T, u8 len> T length_sqr(Vector<T, len> const& a) { return dot_prod(a, a);       }
template<typename T, u8 len> T vec_length(Vector<T, len> const& a) { return sqrt(dot_prod(a, a)); }


#define DEFINE_IMPL_VECTOR_STRUCTURE(type_t, fmt, strfmt, ...) \
vec##fmt::vec##fmt(const type_t* validAddr) { \
    std::memcpy(mem.begin(), validAddr, mem.bytes()); \
    return; \
} \
vec##fmt& vec##fmt::operator=(const vec##fmt& cpy) { \
    std::memcpy(begin(), cpy.begin(), bytes()); \
    return *this; \
} \
vec##fmt::__Mem& vec##fmt::operator+(cref b)  { add(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt::__Mem& vec##fmt::operator-(cref b)  { sub(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt::__Mem& vec##fmt::operator*(cref b)  { mul(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt::__Mem& vec##fmt::operator/(cref b)  { div(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt::__Mem& vec##fmt::operator/(type_t a) { div(mem, a,     temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt::__Mem& vec##fmt::operator*(type_t a) { mul(mem, a,     temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator+(cref b) const { add(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator-(cref b) const { sub(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator*(cref b) const { mul(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator/(cref b) const { div(mem, b.mem, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator*(type_t const a) const { mul(mem, a, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
const vec##fmt::__Mem& vec##fmt::operator/(type_t const a) const { div(mem, a, temporaryBufferVec##fmt); return temporaryBufferVec##fmt;  } \
vec##fmt& vec##fmt::operator+=(cref b) { add(mem, b.mem, mem); return *this;  } \
vec##fmt& vec##fmt::operator-=(cref b) { sub(mem, b.mem, mem); return *this;  } \
vec##fmt& vec##fmt::operator*=(cref b) { mul(mem, b.mem, mem); return *this;  } \
vec##fmt& vec##fmt::operator*=(type_t a) { mul(mem, a,     mem); return *this;  } \
vec##fmt& vec##fmt::operator/=(type_t b) { div(mem, b,     mem); return *this;  } \
\
type_t           vec##fmt::length()     const { temporaryBufferVec##fmt = mem; return vec_length(temporaryBufferVec##fmt); } \
type_t           vec##fmt::lensqr()     const { temporaryBufferVec##fmt = mem; return length_sqr(temporaryBufferVec##fmt); } \
vec##fmt::__Mem& vec##fmt::normalized() const { temporaryBufferVec##fmt = mem; norm(temporaryBufferVec##fmt); return temporaryBufferVec##fmt; } \
void             vec##fmt::normalize() { norm(mem); return; } \
\
\
type_t&       vec##fmt::operator[](uint8_t idx)       { return mem[idx]; } \
const type_t& vec##fmt::operator[](uint8_t idx) const { return mem[idx]; } \
char* vec##fmt::to_string() const \
{ \
	size_t size_s = std::snprintf(nullptr, 0, strfmt, __VA_ARGS__) + 1; \
	char* strBuffer = __scast(char*, malloc(size_s)); \
	std::snprintf(strBuffer, size_s, strfmt, __VA_ARGS__); \
	return strBuffer; \
} \
size_t vec##fmt::to_strbuf(char* buf, size_t bufsize) const \
{ \
	size_t size_s = std::snprintf(nullptr, 0, strfmt, __VA_ARGS__) + 1; \
	if(bufsize == 0 || bufsize < size_s) { \
		return size_s; \
	} \
	std::snprintf(buf, size_s, strfmt, __VA_ARGS__); \
	return 0; \
} \
\
\
vec##fmt::__Mem& operator*(type_t a, vec##fmt::cref b) { mul(b.mem, a,                       temporaryBufferVec##fmt); return temporaryBufferVec##fmt; } \
vec##fmt::__Mem& operator-(          vec##fmt::cref b) { mul(b.mem, static_cast<type_t>(-1), temporaryBufferVec##fmt); return temporaryBufferVec##fmt; } \
type_t dot(vec##fmt::cref a, vec##fmt::cref b) { return dot_prod(a.mem, b.mem); } \


#define DEFINE_IMPL_CROSSPROD_FUNC(type_t, fmt) \
vec##fmt::__Mem& cross(vec##fmt const& a, vec##fmt const& b) { \
    cross_prod(a.mem, b.mem, temporaryBufferVec##fmt); \
    return temporaryBufferVec##fmt; \
} \

#define DEFINE_IMPL_ROTATE_FUNC(type_t, fmt) \
vec##fmt::__Mem& rotate(vec##fmt const& v, vec##fmt const& unit, f32 theta) { \
    auto cos = cosf(theta), sin = sinf(theta); \
    auto omc = dot(unit, v) * (1.0f - cos); \
    vec##fmt __cross, __vcos, __kkv; \
\
    mul(v.mem, cos, __vcos.mem); /* __vcos = v * cos */ \
    mul(unit.mem, omc, __kkv.mem);  /* __kkv = dot(k, v)  * (1 - cos) */ \
    cross_prod(unit.mem, v.mem, __cross.mem); /* __cross = (k X v) * sin */ \
    __cross *= sin; \
\
    add(__vcos.mem, __kkv.mem, temporaryBufferVec##fmt);   /* tmp = __vcos + __kkv */ \
    __vcos.mem = temporaryBufferVec3f;                /* __vcos = tmp */ \
    add(__vcos.mem, __cross.mem, temporaryBufferVec##fmt); /* tmp = __vcos + __cross */ \
    return temporaryBufferVec##fmt; \
} \


DEFINE_IMPL_VECTOR_STRUCTURE(f32, 2f, "vec2f %p: ( %8.4f, %8.4f )",				  (void*)begin(), x, y)
DEFINE_IMPL_VECTOR_STRUCTURE(u32, 2u, "vec2u %p: ( %5u, %5u )"                  , (void*)begin(), x, y)
DEFINE_IMPL_VECTOR_STRUCTURE(i32, 2i, "vec2i %p: ( %5d, %5d )"                  , (void*)begin(), x, y)
DEFINE_IMPL_VECTOR_STRUCTURE(f32, 4f, "vec4f %p: ( %8.4f, %8.4f, %8.4f, %8.4f )", (void*)begin(), x, y, z, w)
DEFINE_IMPL_VECTOR_STRUCTURE(u32, 4u, "vec4u %p: ( %5u, %5u, %5u, %5u )"        , (void*)begin(), x, y, z, w)
DEFINE_IMPL_VECTOR_STRUCTURE(i32, 4i, "vec4i %p: ( %5d, %5d, %5d, %5d )"        , (void*)begin(), x, y, z, w)
DEFINE_IMPL_VECTOR_STRUCTURE(f32, 3f, "vec3f %p: ( %8.4f, %8.4f, %8.4f )"       , (void*)begin(), x, y, z)
DEFINE_IMPL_VECTOR_STRUCTURE(u32, 3u, "vec3u %p: ( %5u, %5u, %5u )"             , (void*)begin(), x, y, z)
DEFINE_IMPL_VECTOR_STRUCTURE(i32, 3i, "vec3i %p: ( %5d, %5d, %5d )"             , (void*)begin(), x, y, z)
DEFINE_IMPL_CROSSPROD_FUNC(f32, 3f)
DEFINE_IMPL_CROSSPROD_FUNC(u32, 3u)
DEFINE_IMPL_CROSSPROD_FUNC(i32, 3i)
DEFINE_IMPL_ROTATE_FUNC(f32, 3f)


mat2f::mat2f(const float* validAddr) {
	std::memcpy(mem.begin(), validAddr, mem.bytes());
	return;
}
mat2f::__Mem& mat2f::operator+(mat2f const& b) { add(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
mat2f::__Mem& mat2f::operator-(mat2f const& b) { sub(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
mat2f::__Mem& mat2f::operator*(mat2f const& b) { mul(mem, b.mem, temporaryBufferMat2f); return temporaryBufferMat2f; }
mat2f::__Mem& mat2f::operator*(float        b) { mul(mem, b,     temporaryBufferMat2f); return temporaryBufferMat2f; }
mat2f::__Mem& mat2f::operator/(float        b) { div(mem, b,     temporaryBufferMat2f); return temporaryBufferMat2f; }
      vec2f  mat2f::column    (uint8_t idx)	      { ifcrash(idx >= 2); return { mem[idx], mem[idx + 2] }; }
const vec2f  mat2f::column    (uint8_t idx) const { ifcrash(idx >= 2); return { mem[idx], mem[idx + 2] }; }
      vec2f& mat2f::operator[](uint8_t idx)	      { ifcrash(idx >= 2); return row[idx]; }
const vec2f& mat2f::operator[](uint8_t idx)	const { ifcrash(idx >= 2); return row[idx]; }
float&       mat2f::operator()(uint8_t i, uint8_t j)       { return mem[i * 2 + j]; }
const float& mat2f::operator()(uint8_t i, uint8_t j) const { return mem[i * 2 + j]; }
mat2f::__Mem& operator*(float a, mat2f const& b) { mul(b.mem, a, temporaryBufferMat2f); return temporaryBufferMat2f; }
char* mat2f::to_string() const
{
    char* strBuffer = __scast(char*, malloc(128));
    static const char* fstr = "mat2f 0x%p:\n( %-8.4f, %-8.4f )\n( %-8.4f, %-8.4f )\n";
    std::snprintf(strBuffer, 128, fstr,
        (void*)begin(),
        row[0].x, row[0].y,
        row[1].x, row[1].y
    );
    return strBuffer;
}


mat4f::mat4f(const float* validAddr) {
    std::memcpy(mem.begin(), validAddr, mem.bytes());
    return;
}
mat4f& mat4f::operator=(const mat4f& cpy) {
    std::memcpy(begin(), cpy.begin(), bytes());
    return *this;
}
      vec4f  mat4f::column(uint8_t idx)           { ifcrash(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
const vec4f  mat4f::column(uint8_t idx)     const { ifcrash(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
      vec4f& mat4f::operator[](uint8_t idx)       { ifcrash(idx >= 4); return row[idx]; }
const vec4f& mat4f::operator[](uint8_t idx) const { ifcrash(idx >= 4); return row[idx]; }
float&       mat4f::operator()(uint8_t i, uint8_t j)       { return mem[i * 4 + j]; }
const float& mat4f::operator()(uint8_t i, uint8_t j) const { return mem[i * 4 + j]; }
mat4f::__Mem& mat4f::operator+(mat4f const& b) { add(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
mat4f::__Mem& mat4f::operator-(mat4f const& b) { sub(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
mat4f::__Mem& mat4f::operator*(mat4f const& b) { mul(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
mat4f::__Mem& mat4f::operator*(float        b) { mul(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }
mat4f::__Mem& mat4f::operator/(float        b) { div(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }
char* mat4f::to_string() const
{
    char* strBuffer = __scast(char*, malloc(512));
    static const char* fstr = "mat4f 0x%p:\n( %-8.4f, %-8.4f, %-8.4f, %-8.4f )\n( %-8.4f, %-8.4f, %-8.4f, %-8.4f )\n( %-8.4f, %-8.4f, %-8.4f, %-8.4f )\n( %-8.4f, %-8.4f, %-8.4f, %-8.4f )\n";
    std::snprintf(strBuffer, 512, fstr,
        (void*)begin(),
        row[0].x, row[0].y, row[0].z, row[0].w,
        row[1].x, row[1].y, row[1].z, row[1].w,
        row[2].x, row[2].y, row[2].z, row[2].w,
        row[3].x, row[3].y, row[3].z, row[3].w
    );
    return strBuffer;
}


mat4f::__Mem& operator*(float a, mat4f const& b)
{
	mul(b.mem, a, temporaryBufferMat4f);
	return temporaryBufferMat4f;
}



void Multiply(vec4f const& a, mat4f const& b, vec4f& out) {
	out.x = dot(a, b.column(0));
	out.y = dot(a, b.column(1));
	out.z = dot(a, b.column(2));
	out.w = dot(a, b.column(3));
}
void Multiply(mat4f const& a, mat4f const& b, mat4f& out) {
	mat4f fetch_btransposed = {
		b.column(0),
		b.column(1),
		b.column(2),
		b.column(3)
	};
	vec4f tmp0{};
	for(size_t i = 0; i < 4; ++i) {
		tmp0.x = dot(a[i], fetch_btransposed[0]);
		tmp0.y = dot(a[i], fetch_btransposed[1]);
		tmp0.z = dot(a[i], fetch_btransposed[2]);
		tmp0.w = dot(a[i], fetch_btransposed[3]);
		out[i] = tmp0;
	}
	return;
}
void Multiply(vec2f const& a, mat2f const& b, vec2f& out)
{
	/* 
		I HOPE this will get optimized into:
			promote __m64[a] -> __m128[tmp]
			shuffle __m128[tmp]
			multiply 	   __m128[tmp], __m128[b]   -> __m128[tmp]
			horizontal_add __m128[tmp], __m128[tmp] -> __m128[tmp]
			demote __m128[tmp].xy -> __m64[out]
	*/
	vec4f tmp = { a.x, a.x, a.y, a.y };
	tmp *= b.homogenised;
	out = {
		tmp.x + tmp.y,
		tmp.z + tmp.w
	};
	return;
}
void Multiply(mat2f const& a, mat2f const& b, mat2f& out)
{
	vec4f tmp0, tmp1, btrans;
	btrans = { b.x0, b.x1, b.y0, b.y1 };
	tmp0 = { a.m00, a.m01, a.m00, a.m01 };
	tmp1 = { a.m10, a.m11, a.m10, a.m11 };
	tmp0 *= btrans;
	tmp1 *= btrans;
	out.row[0] = { tmp0.x + tmp0.y, tmp0.z + tmp0.w };
	out.row[1] = { tmp1.x + tmp1.y, tmp1.z + tmp1.w };
	return;
}


void identity(mat4f& out) {
	out.mem.zero();
	out.m00 = 1.0f;
	out.m11 = 1.0f;
	out.m22 = 1.0f;
	out.m33 = 1.0f;
	return;
}
void identity(mat2f& out) {
	out = { 1.0f, 0.0f, 1.0f, 0.0f };
	return;
}


void translate(vec3f const& translate, mat4f& out) {
	identity(out);
	out[3] = translate.homogenised; 
	out[3].w = 1.0f;
	transpose(out);
	return;
}

void scale(vec3f const& scale, mat4f& out) {
	out.mem.zero();
	out.m00 = scale.x;
	out.m11 = scale.y;
	out.m22 = scale.z;
	out.m33 = 1.0f;
	return;
}

void scale(vec2f const& scale, mat2f& out)
{
	out.mem.zero();
	out.m00 = scale.x;
	out.m11 = scale.y;
	return;
}


void rotate(f32 angle, mat2f& out)
{
	angle = radians(angle);
	out.m00 = cosf(angle);
	out.m01 = sinf(angle);
	out.m11 = out.m00;
	out.m10 = out.m01;
	out.m01 *= -1.0f;
	return;
}

void rotate(math::vec3f const& u, f32 theta, mat4f& out)
{
	f32 cos = cosf(theta), sin = sinf(theta);
	f32 omc = 1 - cos;
	math::vec3f uomc = u * omc;
	math::vec3f usin = u * sin;
	math::vec3f diag = math::vec3f{u * uomc} + math::vec3f{cos}; 
	math::vec3f uomcshuff = u;


	identity(out);
	out.m00 = diag[0];
	out.m11 = diag[1];
	out.m22 = diag[2];
	diag = { uomc.y, uomc.z, uomc.z }; /* diag  = (uomc.y, uomc.z, uomc.z ) */
	uomcshuff[2] = uomcshuff.y;
	uomcshuff[1] = uomcshuff.x; /* shuff = (u.x, u.x, u.y ) */
	uomcshuff *= diag;

	out.m01 = uomcshuff[0] - usin.z; /* m01 = u.x * u.y * (1 - cos) - u.z * sin */
	out.m02 = uomcshuff[1] + usin.y; /* m02 = u.x * u.z * (1 - cos) + u.y * sin */
	out.m10 = uomcshuff[0] + usin.z; /* m10 = u.x * u.y * (1 - cos) + u.z * sin */
	out.m12 = uomcshuff[2] - usin.x; /* m12 = u.y * u.z * (1 - cos) - u.x * sin */
	out.m20 = uomcshuff[1] - usin.y; /* m20 = u.x * u.y * (1 - cos) - u.y * sin */
	out.m21 = uomcshuff[2] + usin.x; /* m21 = u.y * u.z * (1 - cos) + u.x * sin */
	return;
}


void transposed(mat4f const& in, mat4f& out) {
	for(size_t i = 0; i < 4; ++i) {
		out(i, 0) = in(0, i);
		out(i, 1) = in(1, i);
		out(i, 2) = in(2, i);
		out(i, 3) = in(3, i);
	}
}

void transpose(mat4f& inout) {
	mat4f tmp = inout;
	for(size_t i = 0; i < 4; ++i) {
		inout(i, 0) = tmp(0, i);
		inout(i, 1) = tmp(1, i);
		inout(i, 2) = tmp(2, i);
		inout(i, 3) = tmp(3, i);
	}
	return;
}

void transposed(mat2f const& in, mat2f& out) {
	out = { 
		in.x0, in.x1, 
		in.y0, in.y1 
	};
	return;
}

void transpose(mat2f& inout) {
	/* I'm sure this won't get compiled to a shuffle_ps but whatever... */
	vec2f tmp = { inout.x1, inout.y0 };
	inout.m01 = tmp.x;
	inout.m10 = tmp.y;
	return;
}


void perspective(float aspectRatio, float fovy, float near, float far, mat4f& out) {
	float t, r, nmf;
	aspectRatio = 1.0f / aspectRatio;

	t    = 1.0f / tan(fovy * 0.5f);
	r    = t * aspectRatio;
	nmf  = 1.0f / (near - far);
	fovy = 2.0f * far * near;

	out.mem.zero();
	out.m00 = r;                  /* mem[ 0] */ 
	out.m11 = t;                  /* mem[ 5] */ 
	out.m22 = (far + near) * nmf; /* mem[10] */ 
	out.m23 = -1.0f;              /* mem[11] */ 
	out.m32 = fovy * nmf;         /* mem[14] */ 
	return;
}


void inv_perspective(const mat4f &in, mat4f &out)
{
	out.mem.zero();
	math::vec4f inXY = { in(0, 0), in(1, 1), in(3, 2), in(2, 3) };
	math::vec4f one{1.0f};

	inXY = one / inXY;
	out.m00 = inXY.m0;
	out.m11 = inXY.m1;
	out.m23 = inXY.m2;
	out.m32 = inXY.m3;
	out.m33 = -in(2, 2) * inXY.m2 * inXY.m3; 
	return;
}


void orthographic(
	vec2f  leftRight, 
	vec2f  topBottom, 
	vec2f  nearFar,
	mat4f& out
) {
	/* 
		[NOTE]:
		Would prob be more efficient to create a tmp mat4, 
		set it and copy the whole cache line directly to memory,
		instead of writing each float individually to memory 
		(
			The latter is the way its done now in code, 
			although who knows how the compiler will handle it :/
		)
	*/
	f32 rml = 1.0f / (leftRight[1] - leftRight[0]);
	f32 tmb = 1.0f / (topBottom[0] - topBottom[1]);
	f32 nmf = 1.0f / (nearFar[0]   - nearFar[1]  );
	out.mem.zero();

	out.m00 = 2.0f * rml;
	out.m11 = 2.0f * tmb;
	out.m22 = 2.0f * nmf;
	out.m33 = 1.0f;
	out.m03 = -1.0f * (leftRight[1] + leftRight[0]) * rml;
	out.m13 = -1.0f * (topBottom[0] + topBottom[1]) * tmb;
	out.m23 =         (nearFar[0]   + nearFar[1]  ) * nmf;
	return;
}


void orthographic(
	f32 left,   f32 right, 
	f32 bottom, f32 top,
	f32 near,   f32 far,
	mat4f& out
) {
	orthographic({ left, right }, { top, bottom }, { near, far }, out);
	return;
}


void lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
) {
	vec3f forward, right, newup, trans;

	/* (eye - at).div(|(eye-at)|) because 'forward' is towards the screen (us), 'eye' is AT the screen and 'at' is what we want to look at */
	forward = eyePos - at; 
	forward.normalize();

	/* camera_view_dir x up_basis_vector = right_basis_vector */
	right = cross(up, forward);
	right.normalize();
	
	/* right_basis_vector(x_axis) x forward_basis_vector(z_axis) = up_basis_vector(y_axis) */
	newup = cross(forward, right);

	/* Set View Matrix Basis Vectors (R^-1)*/
	out.mem.zero();
	out[0] = right.homogenised;
	out[1] = newup.homogenised;
	out[2] = forward.homogenised;
	transpose(out);

	/* Set The Translation Vector to (-eyePos, 1.0f) (T^-1) */
	trans  = -1.0f * eyePos;
	out[3] = {
		dot(right,   trans),
		dot(newup,   trans),
		dot(forward, trans),
		1.0f
	};
	return;
}


void inverse(mat4f const& m, mat4f& out) 
{
	float 
		A2323, A1323, A1223, A0323,
		A0223, A0123, A2313, A1313, 
		A1213, A2312, A1312, A1212, 
		A0313, A0213, A0312, A0212, 
		A0113, A0112, det;
	float 
		y1A2323, x1A2323, x1A1323, x1A1223, 
		z1A1323, z1A0323, y1A0323, y1A0223, 
		w1A1223, w1A0223, w1A0123, z1A0123;


	A2323 = m.z2 * m.w3 - m.w2 * m.z3;
	A1323 = m.y2 * m.w3 - m.w2 * m.y3;
	A1223 = m.y2 * m.z3 - m.z2 * m.y3;
	A0323 = m.x2 * m.w3 - m.w2 * m.x3;
	A0223 = m.x2 * m.z3 - m.z2 * m.x3;
	A0123 = m.x2 * m.y3 - m.y2 * m.x3;
	A2313 = m.z1 * m.w3 - m.w1 * m.z3;
	A1313 = m.y1 * m.w3 - m.w1 * m.y3;
	A1213 = m.y1 * m.z3 - m.z1 * m.y3;
	A2312 = m.z1 * m.w2 - m.w1 * m.z2;
	A1312 = m.y1 * m.w2 - m.w1 * m.y2;
	A1212 = m.y1 * m.z2 - m.z1 * m.y2;
	A0313 = m.x1 * m.w3 - m.w1 * m.x3;
	A0213 = m.x1 * m.z3 - m.z1 * m.x3;
	A0312 = m.x1 * m.w2 - m.w1 * m.x2;
	A0212 = m.x1 * m.z2 - m.z1 * m.x2;
	A0113 = m.x1 * m.y3 - m.y1 * m.x3;
	A0112 = m.x1 * m.y2 - m.y1 * m.x2;
	y1A2323 = m.y1 * A2323;
	x1A2323 = m.x1 * A2323;
	x1A1323 = m.x1 * A1323;
	x1A1223 = m.x1 * A1223;
	z1A1323 = -1.0f * m.z1 * A1323;
	z1A0323 = -1.0f * m.z1 * A0323;
	y1A0323 = -1.0f * m.y1 * A0323;
	y1A0223 = -1.0f * m.y1 * A0223;
	w1A1223 = m.w1 * A1223;
	w1A0223 = m.w1 * A0223;
	w1A0123 = m.w1 * A0123;
	z1A0123 = m.z1 * A0123;

	det =
	  m.x0 * ( y1A2323 + z1A1323 + w1A1223 )
	- m.y0 * ( x1A2323 + z1A0323 + w1A0223 )
	+ m.z0 * ( x1A1323 + y1A0323 + w1A0123 )
	- m.w0 * ( x1A1223 + y1A0223 + z1A0123 );
	det = 1 / det;

	out.x0 = det *   ( y1A2323 + z1A1323 + w1A1223 );
	out.y0 = det * - ( m.y0 * A2323 - m.z0 * A1323 + m.w0 * A1223 );
	out.z0 = det *   ( m.y0 * A2313 - m.z0 * A1313 + m.w0 * A1213 );
	out.w0 = det * - ( m.y0 * A2312 - m.z0 * A1312 + m.w0 * A1212 );
	out.x1 = det * - ( x1A2323 + z1A0323 + w1A0223 );
	out.y1 = det *   ( m.x0 * A2323 - m.z0 * A0323 + m.w0 * A0223 );
	out.z1 = det * - ( m.x0 * A2313 - m.z0 * A0313 + m.w0 * A0213 );
	out.w1 = det *   ( m.x0 * A2312 - m.z0 * A0312 + m.w0 * A0212 );
	out.x2 = det *   ( x1A1323 + y1A0323 + w1A0123 );
	out.y2 = det * - ( m.x0 * A1323 - m.y0 * A0323 + m.w0 * A0123 );
	out.z2 = det *   ( m.x0 * A1313 - m.y0 * A0313 + m.w0 * A0113 );
	out.w2 = det * - ( m.x0 * A1312 - m.y0 * A0312 + m.w0 * A0112 );
	out.x3 = det * - ( x1A1223 + y1A0223 + z1A0123 );
	out.y3 = det *   ( m.x0 * A1223 - m.y0 * A0223 + m.z0 * A0123 );
	out.z3 = det * - ( m.x0 * A1213 - m.y0 * A0213 + m.z0 * A0113 );
	out.w3 = det *   ( m.x0 * A1212 - m.y0 * A0212 + m.z0 * A0112 );
	return;
}




#if __SSE3__ == 1
#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))
// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)
// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
/* 

	for row major matrix
	we use __m128 to represent 2x2 matrix as A = | A0  A1 |
	                                             | A2  A3 |
*/
__force_inline __m128 Mat2Mul(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix multiply A*B */
{
	return _mm_add_ps(
			_mm_mul_ps(                     vec1, VecSwizzle(vec2, 0,3,0,3)),
		    _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1))
		);
}
__force_inline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix adjugate multiply (A#)*B */
{
	return _mm_sub_ps(
			_mm_mul_ps(VecSwizzle(vec1, 3,3,0,0), vec2),
		    _mm_mul_ps(VecSwizzle(vec1, 1,1,2,2), VecSwizzle(vec2, 2,3,0,1))
		);
}
__force_inline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2) /* 2x2 row major Matrix multiply adjugate A*(B#) */
{
	return _mm_sub_ps(
			_mm_mul_ps(                     vec1, VecSwizzle(vec2, 3,0,3,0)),
		    _mm_mul_ps(VecSwizzle(vec1, 1,0,3,2), VecSwizzle(vec2, 2,1,2,1))
		);
}

void inverseSimd(
	mat4f const& in,
	mat4f& 		 out
) {
	__m128 A, B, C, D;
	__m128 detA, detB, detC, detD;
	__m128 detSub, detM, rDetM, tr;
	__m128 D_C, A_B, X_, Y_, Z_, W_;
	A = VecShuffle_0101(in.row[0].xmm, in.row[1].xmm);
	B = VecShuffle_2323(in.row[0].xmm, in.row[1].xmm);
	C = VecShuffle_0101(in.row[2].xmm, in.row[3].xmm);
	D = VecShuffle_2323(in.row[2].xmm, in.row[3].xmm);

	detSub = _mm_sub_ps(
		_mm_mul_ps(VecShuffle(in.row[0].xmm, in.row[2].xmm, 0,2,0,2), VecShuffle(in.row[1].xmm, in.row[3].xmm, 1,3,1,3)),
		_mm_mul_ps(VecShuffle(in.row[0].xmm, in.row[2].xmm, 1,3,1,3), VecShuffle(in.row[1].xmm, in.row[3].xmm, 0,2,0,2))
	);
	detA = VecSwizzle1(detSub, 0);
	detB = VecSwizzle1(detSub, 1);
	detC = VecSwizzle1(detSub, 2);
	detD = VecSwizzle1(detSub, 3);

	D_C = Mat2AdjMul(D, C);
	A_B = Mat2AdjMul(A, B);
	X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
	W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

	detM = _mm_mul_ps(detA, detD);
	Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
	Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));
	detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

	tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0,2,1,3));
	tr = _mm_hadd_ps(tr, tr);
	tr = _mm_hadd_ps(tr, tr);
	detM = _mm_sub_ps(detM, tr);

	const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
	rDetM = _mm_div_ps(adjSignMask, detM);

	X_ = _mm_mul_ps(X_, rDetM);
	Y_ = _mm_mul_ps(Y_, rDetM);
	Z_ = _mm_mul_ps(Z_, rDetM);
	W_ = _mm_mul_ps(W_, rDetM);
	out.row[0].xmm = VecShuffle(X_, Y_, 3,1,3,1);
	out.row[1].xmm = VecShuffle(X_, Y_, 2,0,2,0);
	out.row[2].xmm = VecShuffle(Z_, W_, 3,1,3,1);
	out.row[3].xmm = VecShuffle(Z_, W_, 2,0,2,0);
	return;
}


#undef VecShuffle_2323
#undef VecShuffle_0101
#undef VecShuffle
#undef VecSwizzle_1133
#undef VecSwizzle_0022
#undef VecSwizzle1
#undef VecSwizzle
#undef VecSwizzleMask
#undef MakeShuffleMask
#else 
#	pragma message WARN("function void inverseSimd(mat4f const& in, mat4f& out) has no implementation -> Code not compiled with SSE3 Support")
#endif


void modelMatrix2d(
	math::vec2f const& translate,
	math::vec2f const& scaling,
	f32 			   rotationAngle,
	math::mat4f& 	   out
) {
	/* To multiply a vec2f by this matrix promote it to { v.x, v.y, 0.0f, 1.0f } to keep the translation component */
	math::mat2f s, r, rs;
	rotate(rotationAngle, r);
	scale(scaling, s);
	Multiply(r, s, rs);

	out = mat4f{rs};
	out.m03 = translate.x;
	out.m13 = translate.y;
	return;
}


} /* namespace util2::math */