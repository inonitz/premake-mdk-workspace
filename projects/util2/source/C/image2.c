#include "util2/C/image2.h"
#include "util2/C/macro.h"
#include "util2/C/static_assert.h"
#include "util2/C/aligned_malloc.h"
#include "util2/C/stb_image.h"
#include <string.h>
#if defined __AVX2__
#	include <immintrin.h>
#elif defined __SSE4_1__
#	include <smmintrin.h>
#endif


UTIL2_STATIC_ASSERT(
    sizeof(util2Image) == 16,
    util2ImageType_SizeEqualityCheck,
    ""
)


bool_t convert_u32rgba_to_f32rgba(
    u8*  inData,
    f32* outData,
    u16  dimx,
    u16  dimy
);

void util2_unload_image(util2Image* img) {
	if (img->m_normfloat)
		util2_aligned_free(img->m_data);
	else
		stbi_image_free(img->m_data);


	img->m_x         = DEFAULT16;
	img->m_y         = DEFAULT16;
	img->m_channels  = DEFAULT8;
	img->m_normfloat = DEFAULT8;
	img->m_data      = NULLPTR_TYPE(u8);
	return;
}

bool_t util2_load_image(
    const char* fileName,
    u8          convert_to_float,
    util2Image* result
) {
	bool_t success = BOOL_FALSE;
	i32    x, y, channels;
	u8*    img_buf   = __ccast(u8*, stbi_load(fileName, &x, &y, &channels, 4));
	u8*    float_buf = __ccast(u8*, NULLPTR);


	if (convert_to_float) {
		float_buf = __rcast(u8*, util2_aligned_malloc(x * y * 4 * sizeof(f32), CACHE_LINE_BYTES));
		success   = convert_u32rgba_to_f32rgba(img_buf, __ccast(f32*, float_buf), x, y);

		stbi_image_free(img_buf);
	}
	result->m_x         = __scast(u32, x);
	result->m_y         = __scast(u32, y);
	result->m_channels  = 4;
	result->m_normfloat = boolean(convert_to_float);
	result->m_data      = convert_to_float ? (u8*) float_buf : img_buf;


	return success && (img_buf != NULLPTR);
}

bool_t convert_u32rgba_to_f32rgba(
    u8*  inData,
    f32* outData,
    u16  dimx,
    u16  dimy
) {
	if (inData == NULLPTR || outData == NULLPTR)
		return BOOL_FALSE;


	u64 currPixel = 0;
	u64 bufLength = 4 * __scast(u64, dimx) * dimy;


#ifdef __AVX2__
	if (!isaligned(outData, 32)) {
		printf("convert_u32rgba_to_f32rgba() -> __AVX2__ Path: outData isn't aligned on (atleast) a 32 Byte memory boundary\n");
		return BOOL_FALSE;
	}


	__m128i      A;
	__m256i      B;
	__m256       C;
	__m256       D;
	const __m256 constf32 = _mm256_broadcastss_ps(_mm_set_ss(0.00392156862745098f)); /* load constant 1/255 to each element */
	for (currPixel = 0; currPixel < bufLength; currPixel += 8) {
		A = _mm_lddqu_si128((const __m128i*) &inData[currPixel]); /* Load 16 bytes 				       */
		B = _mm256_cvtepu8_epi32(A);                              /* Convert first 8 bytes to u32's       */
		C = _mm256_cvtepi32_ps(B);                                /* Convert u32's to f32's 		       */
		D = _mm256_mul_ps(C, constf32);                           /* Multiply f32's by (1/255). 		   */
		_mm256_store_ps(&outData[currPixel], D);                  /* Store result in appropriate location */
	}


#elif defined __SSE4_1__
	if (!isaligned(outData, 16)) {
		printf("convert_u32rgba_to_f32rgba() -> __SSE4_1__ Path: outData isn't aligned on (atleast) a 16 Byte memory boundary\n");
		return BOOL_FALSE;
	}


	alignsz(16) const f32 const128[4] = {0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f};
	const __m128          cf32        = _mm_load_ps(const128); /* SSE */
	__m128i               A, B;
	__m128                C, D;
	for (currPixel = 0; currPixel < bufLength; currPixel += 4) {
		A = _mm_lddqu_si128((const __m128i*) &inData[currPixel]); /* SSE3 */
		B = _mm_cvtepu8_epi32(A);                                 /* SSE4.1 */
		C = _mm_cvtepi32_ps(B);                                   /* SSE2   */
		D = _mm_mul_ps(C, cf32);                                  /* SSE    */
		_mm_store_ps(&outData[currPixel], D);
	}


#else
	/* fallback */
	alignsz(16) f32 tmpf[4];
	alignsz(16) u32 tmpu[4];
	u32             tmp = 0;
	for (currPixel = 0; currPixel < bufLength; currPixel += 4) {
		memcpy(&tmp, &inData[currPixel], 4);
		tmpu[0] = tmp & 0x000000FF; /* First  Byte */
		tmpu[1] = tmp & 0x0000FF00; /* Second Byte */
		tmpu[2] = tmp & 0x00FF0000; /* Third  Byte */
		tmpu[3] = tmp & 0xFF000000; /* Fourth Byte */

		tmpf[0] = __scast(f32, tmpu[0]); /* First  Byte */
		tmpf[1] = __scast(f32, tmpu[1]); /* Second Byte */
		tmpf[2] = __scast(f32, tmpu[2]); /* Third  Byte */
		tmpf[3] = __scast(f32, tmpu[3]); /* Fourth Byte */
		memcpy(&outData[currPixel], tmpf, sizeof(f32) * 4);
	}
#endif


	return BOOL_TRUE;
}