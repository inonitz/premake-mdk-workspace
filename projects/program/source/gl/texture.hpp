#pragma once
#include <util2/C/base_type.h>
#include <util2/vec2.hpp>
#include <vector>




#define TEX_IMAGE_READ_ONLY_ACCESS  0b00
#define TEX_IMAGE_WRITE_ONLY_ACCESS 0b01
#define TEX_IMAGE_READ_WRITE_ACCESS 0b11
#define TEX_BUF_RO_ACCESS (TEX_IMAGE_READ_ONLY_ACCESS)
#define TEX_BUF_WO_ACCESS (TEX_IMAGE_WRITE_ONLY_ACCESS)
#define TEX_BUF_RW_ACCESS (TEX_IMAGE_READ_WRITE_ACCESS) 


struct TextureBufferDescriptor
{
	struct textureParameter
	{
		u32 name;
		u32 val;
	};
	struct DataType 
	{
		u8  count;       /* how many elements per type */
		u16 layout;      /* the layout of the data (RGB, RGBA, BGR, ... ) */
		u16 gltype;      /* the type of each data element */
		u16 internalFmt; /* combines gltype & layout. ex. GL_FLOAT & GL_RGBA => GL_RGBA32F */
		/* 
			Technically, there is a bit of redundency since (ex.) gltype & layout encode internalFmt, layout encodes count, etc.
			Right now this is a pain in the ass to make utillity functions for these,
			I just want to make this work, then worry about more serious problems, until this becomes annoying enough to the point 
			where I must change it.
		*/
	};


	util2::math::vec2u dims;
	void* 		data = nullptr;
	DataType    format;
	std::vector<textureParameter> parameters;
};




typedef struct UpdatedTextureBuffer /* tex2D with a single level of detail */
{
public:
	void create(TextureBufferDescriptor const& info);
	void destroy();
	void bindToImage(u32 imageUnit, u8 imageAccess);
	void bindToUnit(u32 textureUnit);

	void unbindImage();
	void unbindUnit();
	void recreateImage(util2::math::vec2u newDims);


private:
	u32 m_id 		  = DEFAULT32;
	u32 m_imageUnit   = DEFAULT32;
	u32 m_bindingUnit = DEFAULT32;
	
	TextureBufferDescriptor m_info;
} TextureBuffer;