#include "texture.hpp"
#include <glbinding/gl/gl.h>


void TextureBuffer::create(const TextureBufferDescriptor &inf)
{
	m_info = inf;
	gl::glCreateTextures(gl::GL_TEXTURE_2D, 1, &m_id);
	for(auto& param_pair : m_info.parameters) {
		gl::glTextureParameteri(m_id, __scast(gl::GLenum, param_pair.name), param_pair.val);
	}
	recreateImage(inf.dims);
	return;
}


void TextureBuffer::destroy()
{
	if(m_id != DEFAULT32) { 
		if(m_imageUnit   != DEFAULT32) unbindImage();
		if(m_bindingUnit != DEFAULT32) unbindUnit();
		gl::glDeleteTextures(1, &m_id); 
	}
	return;
}


void TextureBuffer::bindToImage(u32 imgUnit, u8 accessRights)
{
	m_imageUnit = imgUnit;
	
	u32 level = 0;
	accessRights &= 0b11;
	gl::glBindImageTexture(
		imgUnit, 
		m_id, 
		level, 
		0,
		level, 
		gl::GL_READ_ONLY + accessRights, 
		__scast(gl::GLenum, m_info.format.internalFmt)
	);
	return;
}


void TextureBuffer::bindToUnit(u32 textureUnit)
{
	m_bindingUnit = textureUnit;
	gl::glBindTextureUnit(m_bindingUnit, m_id);
	return;
}


void TextureBuffer::unbindImage()
{
	gl::glBindImageTexture(m_imageUnit, 0, 0, 0, 0, gl::GL_READ_ONLY, gl::GL_RGBA32F);
	m_imageUnit = DEFAULT32;
	return;
}


void TextureBuffer::unbindUnit()
{
	gl::glBindTextureUnit(m_bindingUnit, 0);
	m_bindingUnit = DEFAULT32;
	return;
}


// void TextureBuffer::recreateImage(math::vec2u newDims)
// {
// 	info.dims = newDims;
// 	gl::glBindTexture(GL_TEXTURE_2D, id);
// 	gl::glTexImage2D(
// 		GL_TEXTURE_2D, 
// 		0, 
// 		info.format.internalFmt, 
// 		info.dims.x, 
// 		info.dims.y, 
// 		0, 
// 		info.format.layout, 
// 		info.format.gl::gltype, 
// 		nullptr
// 	);
// 	gl::glBindTexture(GL_TEXTURE_2D, 0);
// 	return;
// }


void TextureBuffer::recreateImage(util2::math::vec2u newDims)
{
	// unbindImage();
	// unbindUnit();
	m_info.dims = newDims;
	gl::glBindTexture(gl::GL_TEXTURE_2D, m_id);
	gl::glTexImage2D(
		gl::GL_TEXTURE_2D,
		0, 
		__scast(gl::GLenum, m_info.format.internalFmt),
		m_info.dims.x,
		m_info.dims.y,
		0,
		__scast(gl::GLenum, m_info.format.layout),
		__scast(gl::GLenum, m_info.format.gltype),
		m_info.data
	);
	gl::glBindTexture(gl::GL_TEXTURE_2D, 0);
	return;
}