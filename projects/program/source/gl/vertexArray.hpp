#pragma once
#include <util2/C/base_type.h>
#include <util2/C/macro.h>
#include <vector>




struct VertexDescriptor
{
	struct DataType
	{
		u16 gltype;
		u8  count;
	};
	std::vector<DataType> attributes;


	u8  count   (u8 attributeIndex) const { return attributes[attributeIndex].count; }
	u32 size    (u8 attributeIndex) const;
	u32 typeSize(u8 attributeIndex) const;
	u32 offset  (u8 attributeIndex) const;


	u8     attributeCount() const { return attributes.size(); }
	size_t totalSize() 		const;

	static VertexDescriptor defaultVertex() {
		return VertexDescriptor{{  { 0x1401, 1 }  }}; /* 0x1401 = GL_UNSIGNED_BYTE */
	}
};


struct BufferDescriptor
{
	void* 			 data;
	u32   			 count;
	VertexDescriptor vinfo;
};


struct ElementBufferRenderData
{
	u32 count   = DEFAULT32;
	u16 gl_type = DEFAULT16;
	u16 gl_size = DEFAULT16;
};


struct Buffer
{
public:
	Buffer() = default;
	
	void create(BufferDescriptor const& info, 		u32 usage);
	void update(BufferDescriptor const& updateInfo, u32 byteOffset); /* [NOTE]: vinfo is unused, as there is no purpose for it. */
	void destroy();


protected:
	u32 m_id;
	BufferDescriptor m_info;

	friend struct VertexArray;
	friend struct ShaderStorageBuffer;
};


struct ShaderStorageBuffer
{
public:
	void create(BufferDescriptor const& info, u32 usage);
	void update(u32 byteOffset, BufferDescriptor const& info);
	void destroy();


	void setBindingIndex(u32 binding);
	void clearBindingIndex();
	void bind();
	void unbind();
private:
	Buffer m_base;
	u32    m_bindingPoint;
};




struct VertexArray
{
public:
	VertexArray() = default;


	void create(Buffer& Vertices, Buffer& Indices);
	void create(Buffer& Vertices) { createCommon(Vertices); }
	void destroy();
	void bind()   const;
	void unbind() const;

	auto const& getRenderData() const { return m_renderData; }

private:
	void createCommon(Buffer& Vertices);

protected:
	u32 m_vao, m_vbo, m_ebo;
	ElementBufferRenderData m_renderData;
};