#include "shader2.hpp"
#include "util/file.hpp"
#include "util/marker2.hpp"
#include <glbinding/gl/gl.h>
#include <cmath>
#include <cstdio>
#include <cstring>


using namespace util;


static inline std::array<char, 2048> genericErrorLogBuffer;


constexpr const char* shaderTypeToString(u32 type)
{
    constexpr std::array<gl::GLenum,  3> types = {  gl::GL_VERTEX_SHADER, gl::GL_FRAGMENT_SHADER, gl::GL_COMPUTE_SHADER  };
    constexpr std::array<const char*, 4> strs  = { "GL_VERTEX_SHADER", "GL_FRAGMENT_SHADER", "GL_COMPUTE_SHADER", "GL_SHADER_UNKOWN" };
    size_t i = 0;
    while(i < 3 && types[i] != __scast(gl::GLenum, type)) ++i;
    return strs[i];
}


void writeComputeGroupSizeToShader(char* source, u32 sizeX, u32 sizeY, u32 sizeZ)
{
    constexpr std::array<char[19], 3> substrings = {
            "local_size_x = ",
            "local_size_y = ",
            "local_size_z = "
    };
    

    const std::array<u32, 3> numberToStringSize = {
        __scast(u32, ceil( log10(sizeX + (sizeX == 1u)) )  ),
        __scast(u32, ceil( log10(sizeY + (sizeY == 1u)) )  ),
        __scast(u32, ceil( log10(sizeZ + (sizeZ == 1u)) )  ) 
    };
    std::array<char[5], 3> numberToString = {
        "    ",
        "    ",
        "    "
    };
    std::array<char*, 3> positions = { nullptr, nullptr, nullptr };
    
    
    sprintf(numberToString[0], "%u", sizeX);
    sprintf(numberToString[1], "%u", sizeY);
    sprintf(numberToString[2], "%u", sizeZ);
    positions[0] = strstr(source, substrings[0]) + 15;
    positions[1] = strstr(source, substrings[1]) + 15;
    positions[2] = strstr(source, substrings[2]) + 15;
    memcpy(positions[0], "   ", 3);
    memcpy(positions[1], "   ", 3);
    memcpy(positions[2], "   ", 3);
    memcpy(positions[0], numberToString[0], numberToStringSize[0]);
    memcpy(positions[1], numberToString[1], numberToStringSize[1]);
    memcpy(positions[2], numberToString[2], numberToStringSize[2]);
    // debug_messagefmt("--------------------------------\nModified Shader File:\n%s\n--------------------------------\n", source);
    // markfmt("ntsSize: { %u %u %u }\nnts: { %s %s %s }\n", 
    //     numberToStringSize[0],
    //     numberToStringSize[1],
    //     numberToStringSize[2],
    //     numberToString[0],
    //     numberToString[1],
    //     numberToString[2]
    // );
    return;
}




bool Program::loadShader(ShaderData& init, BufferData const& loadedShader)
{
    i32 successStatus = __scast(i32, gl::GL_TRUE);
    i32 length = __scast(i32, loadedShader.size);

    
    init.id = gl::glCreateShader(__scast(gl::GLenum, init.type));
    gl::glShaderSource(init.id, 1, &loadedShader.data, &length);
    gl::glCompileShader(init.id);
    gl::glGetShaderiv(init.id, gl::GL_COMPILE_STATUS, &successStatus);
    if(!successStatus) {
        gl::glGetShaderInfoLog(init.id, genericErrorLogBuffer.size(), &length, genericErrorLogBuffer.data());
        markfmt("Shader [%s][Path='%s'] Compilation Failed -> Error Log:\n%s\n", 
            shaderTypeToString(init.type),
            init.filepath,
            genericErrorLogBuffer.data()
        );

        gl::glDeleteShader(init.id);
        init.id = DEFAULT32;
    }

    markfmt("returned %u for %s", successStatus, shaderTypeToString(init.type));
    return boolean(successStatus);
}


void Program::refreshShaderSource(u32 shaderID, const char* filepath)
{
    BufferData buf = {nullptr, 0};


    if(filepath != nullptr) {
        m_shaders[shaderID].filepath = filepath;
    }
    filepath = m_shaders[shaderID].filepath; /* use default filepath if filepath == nullptr */


    loadFile(filepath, &buf.size, __scast(char*, nullptr));
    m_sources[shaderID].resize(buf.size);
    loadFile(filepath, &buf.size, m_sources[shaderID].data()); /* will crash if false, so no need to check return bool */
    return;
}


void Program::refreshShaderSource(u32 shaderID, BufferData const& buffer)
{
    m_sources[shaderID].resize(buffer.size);
    memcpy(m_sources[shaderID].data(), buffer.data, buffer.size);
    return;
}


void Program::resizeLocalWorkGroup(u32 shaderID, u32 workGroupSizeX, u32 workGroupSizeY, u32 workGroupSizeZ)
{
    ifcrash_debug(m_shaders[shaderID].type != gl::GL_COMPUTE_SHADER);
    writeComputeGroupSizeToShader(m_sources[shaderID].data(), workGroupSizeX, workGroupSizeY, workGroupSizeZ);
    return;
}


bool Program::compile()
{
    size_t     i             = 0;
    i32        successStatus = __scast(i32, gl::GL_TRUE);
    BufferData populate      = {nullptr, 0};


    /* Shader Compile stage Begin. */
    for(; i < m_shaders.size() && successStatus; ++i) {
        ifcrash_debug(m_sources[i].size() > UINT32_MAX);
        populate = {
            m_sources[i].data(),
            __scast(u32, m_sources[i].size())
        };
        // debug_messagefmt("Loading shader [%s/%u ] with Buffer %p [%llu bytes]\nSource:\n%s\n", shaderTypeToString(shaders[i].type), shaders.size(), populate.data, populate.size, sources[i].data());
        successStatus = successStatus && loadShader(m_shaders[i], populate); 
    }
    /* Shader Compile stage End. */
    
    /* Error Checking For Shader Stage */
    if(!successStatus) {
        markfmt("Failed to load Shader Files/Buffers. Failed on shaderID = %llu\n", i);
        for(size_t s = 0; s < i; ++s) { /* Delete previously compiled shaders */
            gl::glDeleteShader(m_shaders[s].id);
            m_shaders[s].id = DEFAULT32;
        }
        return __scast(bool, gl::GL_FALSE);
    }



    /* Shader Program Creation Stage Begin. */
    if(m_id != DEFAULT32) {
        gl::glDeleteProgram(m_id);
    }
    m_id = gl::glCreateProgram();
    for(size_t i = 0; i < m_shaders.size(); ++i) { gl::glAttachShader(m_id, m_shaders[i].id); }
    gl::glLinkProgram(m_id);
    /* Shader Program Creation Stage End. */

    
    /* Error Checking For Program Stage */
    gl::glGetProgramiv(m_id, gl::GL_LINK_STATUS, &successStatus);
    if(!successStatus) 
    {
        gl::glGetProgramInfoLog(m_id, sizeof(genericErrorLogBuffer), NULL, genericErrorLogBuffer.data());
        markfmt("Failed to link Shader Program id %u\nError Log: \n%s\n", 
            m_id,
            genericErrorLogBuffer.data()
        );
        gl::glDeleteProgram(m_id);
        m_id = DEFAULT32;
    }

    /* Delete All Shaders if necessary */
    for(auto& shader : m_shaders) {
        gl::glDeleteShader(shader.id);
        shader.id = DEFAULT32;
    }
    return successStatus;
}


void Program::bind()   const { gl::glUseProgram(m_id); }
void Program::unbind() const { gl::glUseProgram(0);    }


void Program::destroy()
{
    gl::glDeleteProgram(m_id);
    m_id = DEFAULT32;

    m_shaders.clear();
    m_sources.clear();
    return;
}


void Program::UniformBlock(std::string_view const& name, u32 blockIndex)
{
    u32 tmp = gl::glGetProgramResourceIndex(m_id, gl::GL_UNIFORM_BLOCK, name.data());
    ifcrash_debug(tmp == gl::GL_INVALID_INDEX);
    gl::glUniformBlockBinding(m_id, tmp, blockIndex);
    return;
}

void Program::StorageBlock(std::string_view const& name, u32 blockIndex)
{
    u32 tmp = gl::glGetProgramResourceIndex(m_id, gl::GL_SHADER_STORAGE_BLOCK, name.data());
    ifcrash_debug(tmp == gl::GL_INVALID_INDEX);
    gl::glShaderStorageBlockBinding(m_id, tmp, blockIndex);
    return;
}

#define CREATE_UNIFORM_FUNCTION_IMPL(TypeSpecifier, arg0, ...) \
[[maybe_unused]] void Program::uniform##TypeSpecifier( \
	std::string_view const& name, \
	arg0 \
	) { \
		gl::glUniform##TypeSpecifier( gl::glGetUniformLocation(m_id, name.data()), __VA_ARGS__); \
	} \

CREATE_UNIFORM_FUNCTION_IMPL(1f,  f32 v, v);
CREATE_UNIFORM_FUNCTION_IMPL(1i,  i32 v, v);
CREATE_UNIFORM_FUNCTION_IMPL(1ui, u32 v, v);

CREATE_UNIFORM_FUNCTION_IMPL(2f,  array2f const& v, v[0], v[1]);
CREATE_UNIFORM_FUNCTION_IMPL(2i,  array2i const& v, v[0], v[1]);
CREATE_UNIFORM_FUNCTION_IMPL(2ui, array2u const& v, v[0], v[1]);

CREATE_UNIFORM_FUNCTION_IMPL(3f,  array3f const& v, v[0], v[1], v[2]);
CREATE_UNIFORM_FUNCTION_IMPL(3ui, array3u const& v, v[0], v[1], v[2]);
CREATE_UNIFORM_FUNCTION_IMPL(3i,  array3i const& v, v[0], v[1], v[2]);

CREATE_UNIFORM_FUNCTION_IMPL(4f,  array4f const& v, v[0], v[1], v[2], v[3]);
CREATE_UNIFORM_FUNCTION_IMPL(4i,  array4i const& v, v[0], v[1], v[2], v[3]);
CREATE_UNIFORM_FUNCTION_IMPL(4ui, array4u const& v, v[0], v[1], v[2], v[3]);

CREATE_UNIFORM_FUNCTION_IMPL(1fv, f32 const * v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2fv, f32 const * v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(3fv, f32 const * v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(4fv, f32 const * v, 1, v);

CREATE_UNIFORM_FUNCTION_IMPL(1iv, i32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2iv, i32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(3iv, i32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(4iv, i32 const* v, 1, v);

CREATE_UNIFORM_FUNCTION_IMPL(1uiv, u32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(2uiv, u32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(3uiv, u32 const* v, 1, v);
CREATE_UNIFORM_FUNCTION_IMPL(4uiv, u32 const* v, 1, v);

CREATE_UNIFORM_FUNCTION_IMPL(Matrix2fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4fv,   std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x3fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x2fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x4fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x2fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x4fv, std::vector<f32> const& v, 1, false, v.data());
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x3fv, std::vector<f32> const& v, 1, false, v.data());

CREATE_UNIFORM_FUNCTION_IMPL(Matrix2fv,   f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3fv,   f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4fv,   f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x3fv, f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x2fv, f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix2x4fv, f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x2fv, f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix3x4fv, f32 const* v, 1, false, v);
CREATE_UNIFORM_FUNCTION_IMPL(Matrix4x3fv, f32 const* v, 1, false, v);
#undef CREATE_UNIFORM_FUNCTION_IMPL