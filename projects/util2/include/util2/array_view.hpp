#ifndef __UTIL2_DEFINITION_ARRAY_VIEW__
#define __UTIL2_DEFINITION_ARRAY_VIEW__
#include "util2/C/util2_api.h"
#include "util2/C/base_type.h"
#include "util2/C/ifcrash2.h"

#include "util2/C/static_assert.h"

#include <cinttypes>
#include <cstring>

#include "util2/C/macro.h"

namespace util2 {


/*
    Honestly Owning-array_views should have a differnt interface than
    Non-owning array_view's, like in util2/pool.hpp
*/
template <
    typename T>
struct UTIL2_API iterator {
  public:
	iterator(T* const& ptr) : m_ptr(ptr) {
	}

	auto operator++() -> iterator {
		++m_ptr; /* NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) */
		return *this;
	}

	auto operator!=(iterator const& iter) -> bool {
		return m_ptr != iter.m_ptr;
	}

	auto operator*() -> T const& {
		return *m_ptr;
	}

  private:
	T* m_ptr;
};

template <
    typename T,
    bool hasCreationOwnerShip>
class UTIL2_API array_view {
  public:
	array_view() : m_buf{nullptr}, m_size{0} {
	}

	array_view(T* bufferaddress, u32 buffersize) : m_buf{bufferaddress},
	                                               m_size{buffersize} {
	}

	array_view(u32 creationsize) : m_buf{new T[creationsize]}, m_size{creationsize} {
		UTIL2_STATIC_ASSERT(hasCreationOwnerShip == true, 
            util2_array_view_has_ownership_over_created_pointer, 
            "util2::array_view<T>::array_view(u32 creation size) requires hasCreationOwnerShip=true"
        );
		return;
	}

	array_view(array_view&& otherview)                         = delete;
	array_view(array_view const& otherview)                    = delete; /* if you want to copy, do it explicitly with copy() */
	auto operator=(array_view&& otherview) -> array_view&      = delete;
	auto operator=(array_view const& otherview) -> array_view& = delete; /* if you want to copy, do it explicitly with copy() */

	~array_view() {
		if constexpr (hasCreationOwnerShip) {
			delete[] m_buf;
		}
		return;
	}

	void copyEqualSize(array_view const& otherview) {
		UTIL2_STATIC_ASSERT(ownershipOfData() == true, 
            util2_array_view_must_have_ownership_on_copy, 
            "util2::array_view<T> must have ownership on copy operation"
        );
        ifcrashstr(m_size == otherview.m_size, 
            "util2::array_view<T>::copyEqualSize() -> Array-View Lengths must be equal"
        );
		std::memcpy(m_buf, otherview.m_buf, bytes());
		return;
	}

	void copyMaybeResize(array_view const& otherview) {
		UTIL2_STATIC_ASSERT(ownershipOfData() == true, 
            util2_array_view_must_have_ownership_on_copy, 
            "util2::array_view<T> must have ownership on copy operation"
        );
		if (otherview.m_size > m_size) {
			delete[] m_buf;
			m_size = otherview.m_size;
            m_buf = new T[m_size];
		}
		std::memcpy(m_buf, otherview.m_buf, bytes());
		return;
	}

	/* NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-vararg) */
	auto operator[](u32 index) -> T& {
		ifcrashfmt(index >= m_size, 
            "util2::array_view<T>::operator[] -> Index{%" PRIu32 "} Out of range{%" PRIu32 "} Error\n", 
            index, 
            m_size
        );
		return m_buf[index];
	}

	auto operator()(u32 begin, u32 end) -> array_view<T, false> {
		ifcrashfmt(begin > end || end >= m_size, 
            "util2::array_view<T>::operator()(u32 begin, u32 end) -> "
            "{ Begin, End, BufferLength } -> { %" PRIu32 ", %" PRIu32 ", %" PRIu32 " }\n", 
            begin, 
            end, 
            m_size
        );
		return array_view<T, false>(m_buf + begin, m_buf + end);
	}
	/* NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-pro-type-vararg) */


	auto begin() -> iterator<T> {
		return iterator<T>(m_buf);
	}

	auto end() -> iterator<T> {
		return iterator<T>(m_buf + m_size);
	}

	auto cbegin() -> iterator<const T> {
		return iterator<const T>(m_buf);
	}

	auto cend() -> iterator<const T> {
		return iterator<const T>(m_buf + m_size);
	}

	[[nodiscard]] constexpr auto ownershipOfData() const -> bool {
		return hasCreationOwnerShip;
	}

	[[nodiscard]] auto size() -> u32 {
		return m_size;
	}

	[[nodiscard]] auto bytes() -> u64 {
		return m_size * sizeof(T);
	}


  private:
	T*  m_buf;
	u32 m_size;
	u32 m_reserved{DEFAULT32};
};


template<typename T> 
using UTIL2_API owningArrayView = array_view<T, true>;
template<typename T> 
using UTIL2_API nonOwningArrayView = array_view<T, false>;


} /* namespace util2 */


#endif /* __UTIL2_DEFINITION_ARRAY_VIEW__ */