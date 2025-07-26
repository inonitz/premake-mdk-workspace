#include "util2/pool.hpp"
#include "util2/aligned_malloc.hpp"

#include "util2/ifcrash.hpp"

#include <cinttypes>

#include <cstdio>
#include <cstring>



/* NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic) */
namespace util2::detail {


template<
    u32 objectSizeInBytes
> 
auto CommonPoolDef<objectSizeInBytes>::allocate() -> void*
{
    if(!boolean(m_freeBlk)) {
        // markstr("Allocation Error: Not Enough Blocks (0)\n");
        return nullptr;
    }

    byte* value = &m_buffer[ objectSizeInBytes * (m_available->index - 1) ];
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return value;
}


template<
    u32 objectSizeInBytes
> 
void CommonPoolDef<objectSizeInBytes>::free(void* ptr)
{
    u64 idx = index_from_pointer(ptr) / objectSizeInBytes;
    ifcrash(!isaligned(ptr, objectSize()) || !occupied(idx) || (m_freeBlk == m_elemCount));

    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;

    /* completely wipe the block of old data */
    std::memset(__scast(byte*, ptr), DEFAULT8, objectSize());
    return;
}


template<
    u32 objectSizeInBytes
> 
u64 CommonPoolDef<objectSizeInBytes>::allocate_index()
{
    if(!boolean(m_freeBlk)) {
        // markstr("Allocation Error: Not Enough Blocks (0)\n");
        return DEFAULT64;
    }

    u64 v = m_available->index - 1;
    m_available->index *= -1; /* now occupied */

    m_available = m_available->next;
    --m_freeBlk;
    return v;	
}


template<
    u32 objectSizeInBytes
> 
void CommonPoolDef<objectSizeInBytes>::free_index(u64 idx)
{
    ifcrash(!occupied(idx) || m_freeBlk == m_elemCount || idx >= m_elemCount);
    m_freelist[idx].index *= -1;
    m_freelist[idx].next = m_available;
    m_available = &m_freelist[idx];
    ++m_freeBlk;

    std::memset(&m_buffer[idx], DEFAULT8, objectSize()); /* completely wipe the block of old data */
    return;
}


template<
    u32 objectSizeInBytes
> 
void CommonPoolDef<objectSizeInBytes>::print() const
{
    static const char* strs[2] = { "Occupied", "Free    " };


    std::printf("Static Pool Allocator Located at 0x%" PRIxPTR " [%" PRIuLEAST64 "/ %" PRIuLEAST64 "] Occupied\n",
        m_buffer,
        m_elemCount - m_freeBlk,
        m_elemCount
    );

    for(u64 i = 0; i < m_elemCount; ++i)
    {
        std::printf("    [%" PRIuLEAST64 "] [%s] => Object [%" PRIuLEAST64 "]\n", 
            i, 
            strs[ boolean(m_freelist[i].index > 0) ], 
            __scast(u64, m_freelist[i].index)
        );
    }
    return;
}


template<
    u32 objectSizeInBytes
> 
void CommonPoolDef<objectSizeInBytes>::common_init(u64 amountOfElements)
{
    ifcrash(amountOfElements == 0);
    m_elemCount = amountOfElements; 
    m_freeBlk   = amountOfElements;
    for(u64 i = 0; i < amountOfElements - 1; ++i)
    {
        m_freelist[i].index = i + 1;
        m_freelist[i].next = &m_freelist[i + 1];
    }
    m_freelist[m_elemCount - 1] = { __scast(i64, m_elemCount), nullptr }; /* last element shouldn't point anywhere */
    m_available = &m_freelist[0];
    return;
}


/* NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) */
template struct CommonPoolDef<0x008>;
template struct CommonPoolDef<0x010>;
template struct CommonPoolDef<0x018>;
template struct CommonPoolDef<0x020>;
template struct CommonPoolDef<0x028>;
template struct CommonPoolDef<0x030>;
template struct CommonPoolDef<0x038>;
template struct CommonPoolDef<0x040>;
template struct CommonPoolDef<0x048>;
template struct CommonPoolDef<0x050>;
template struct CommonPoolDef<0x058>;
template struct CommonPoolDef<0x060>;
template struct CommonPoolDef<0x068>;
template struct CommonPoolDef<0x070>;
template struct CommonPoolDef<0x078>;
template struct CommonPoolDef<0x080>;
template struct CommonPoolDef<0x0c0>;
template struct CommonPoolDef<0x100>;
template struct CommonPoolDef<0x140>;
template struct CommonPoolDef<0x180>;
template struct CommonPoolDef<0x1c0>;
template struct CommonPoolDef<0x200>;
/* NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) */


} // namespace util2::detail




namespace util2 {


template<
    u32 objectSizeInBytes
> 
void Pool<objectSizeInBytes, false>::create(u64 amountOfElements)
{
    this->m_buffer = __scast(byte*,     
        util2::aligned_malloc<objectSizeInBytes>(objectSizeInBytes * amountOfElements)
    );
    this->m_freelist = __scast(NodeType*, 
        util2::aligned_malloc<sizeof(NodeType)>(sizeof(NodeType)  * amountOfElements)
    );
    this->common_init(amountOfElements);
    return;
}

template<
    u32 objectSizeInBytes
> 
void Pool<objectSizeInBytes, false>::destroy()
{
    util2::aligned_free(this->m_buffer);    
    util2::aligned_free(this->m_freelist);
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}


template<
    u32 objectSizeInBytes
> 
void Pool<objectSizeInBytes, true>::create(
    void*  aligned_allocated_memory,
    u64    amountOfElements
) {
    this->m_buffer   = __scast(byte*, aligned_allocated_memory);
    this->m_freelist = __scast(NodeType*, util2::aligned_malloc<sizeof(NodeType)>(sizeof(NodeType) * amountOfElements));
    this->common_init(amountOfElements);
    return;
}

template<
    u32 objectSizeInBytes
> 
void Pool<objectSizeInBytes, true>::destroy()
{
    util2::aligned_free(this->m_freelist);
    this->m_buffer    = nullptr;
    this->m_available = nullptr;
    this->m_elemCount = 0;
    this->m_freeBlk   = 0;
    return;
}


/* NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) */
template class Pool<0x008, true>;
template class Pool<0x010, true>;
template class Pool<0x018, true>;
template class Pool<0x020, true>;
template class Pool<0x028, true>;
template class Pool<0x030, true>;
template class Pool<0x038, true>;
template class Pool<0x040, true>;
template class Pool<0x048, true>;
template class Pool<0x050, true>;
template class Pool<0x058, true>;
template class Pool<0x060, true>;
template class Pool<0x068, true>;
template class Pool<0x070, true>;
template class Pool<0x078, true>;
template class Pool<0x080, true>;
template class Pool<0x0c0, true>;
template class Pool<0x100, true>;
template class Pool<0x140, true>;
template class Pool<0x180, true>;
template class Pool<0x1c0, true>;
template class Pool<0x200, true>;
template class Pool<0x008, false>;
template class Pool<0x010, false>;
template class Pool<0x018, false>;
template class Pool<0x020, false>;
template class Pool<0x028, false>;
template class Pool<0x030, false>;
template class Pool<0x038, false>;
template class Pool<0x040, false>;
template class Pool<0x048, false>;
template class Pool<0x050, false>;
template class Pool<0x058, false>;
template class Pool<0x060, false>;
template class Pool<0x068, false>;
template class Pool<0x070, false>;
template class Pool<0x078, false>;
template class Pool<0x080, false>;
template class Pool<0x0c0, false>;
template class Pool<0x100, false>;
template class Pool<0x140, false>;
template class Pool<0x180, false>;
template class Pool<0x1c0, false>;
template class Pool<0x200, false>;
/* NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) */


/* NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic) */
} /* namespace util2 */