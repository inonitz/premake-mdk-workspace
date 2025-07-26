#ifndef __UTIL2_POOL_ALLOCATOR__
#define __UTIL2_POOL_ALLOCATOR__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"

#include "util2/C/macro.h"



namespace util2::detail {


template<
	u32 objectSizeInBytes
> 
struct UTIL2_API CommonPoolDef
{
public:
	void* allocate();
	void  free(void* ptr);

	u64  allocate_index();
	void free_index(u64 idx);

	u64  availableBlocks() const { return m_freeBlk;         }
	u64  size()    	       const { return m_elemCount;       }
	u64  objectSize() 	   const { return objectSizeInBytes; }
	u64  bytes() 		   const { return size() * objectSize(); }	
	void print()  const;

protected:
	struct Node {
		i64   index;
		Node* next;
	};
	
	Node* m_freelist; /* Book-keeping -> Each node points to a T object in memory. */
	Node* m_available;
	byte* m_buffer;   /* The actual memory allocated. */
	u64   m_elemCount;
	u64   m_freeBlk;


	__force_inline auto occupied(u64 idx) const -> bool { 
		return m_freelist[idx].index < 0; /* NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) */
	}
	__force_inline auto index_from_pointer(void* ptr) const -> u64 { 
		return __scast(byte* const, ptr) - m_buffer;
	}
	void common_init(u64 amountOfElements);
};


} /* namespace util2::detail */




namespace util2 {


template<
	u32 objectSizeInBytes, 
	bool userManagedMemoryPointer = false
> class UTIL2_API Pool {};


template<
	u32 objectSizeInBytes
> 
class UTIL2_API Pool<objectSizeInBytes, false> : 
	public util2::detail::CommonPoolDef<objectSizeInBytes>
{
public:
	void create(u64 amountOfElements);
	void destroy();

private:
	using NodeType = typename 
		util2::detail::CommonPoolDef<objectSizeInBytes>::Node;
};


template<
	u32 objectSizeInBytes
> 
class UTIL2_API Pool<objectSizeInBytes, true> : 
	public util2::detail::CommonPoolDef<objectSizeInBytes>
{
public:
	void create(
		void* aligned_allocated_memory,
		u64   amountOfElements
	);
	void destroy();

private:
	using NodeType = typename 
		util2::detail::CommonPoolDef<objectSizeInBytes>::Node;
};


} /* namespace util2 */


#endif /* __UTIL2_POOL_ALLOCATOR__ */