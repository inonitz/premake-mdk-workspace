#ifndef __UTIL2_FLAT_ARRAY_HASH_TABLE__
#define __UTIL2_FLAT_ARRAY_HASH_TABLE__
#include "util2/hash.hpp"
#include "util2/pool.hpp"
#include "util2/type_trait.hpp"
#include "util2/C/aligned_malloc.h"
#include <utility>
#include <vector>
#include <array>
#include <functional>
#include <string>


namespace util2 {


/* 
	Hasher MUST overload the Following Functions: 
		default Constructor
		size_t operator(const_ref<Key>& k) [get hash Value       ] 
		refresh() 						   [for changing the seed] 
*/
template<
	typename Key, 
	typename Value, 
	class 	Hasher = util2::Hash
>
struct UTIL2_API flat_hash
{
public:
	using StringFromKV = std::function<
		std::string
			(const_ref<Key>, const_ref<Value>)
		>;


#define FLAT_HASH_INSERT_SUCCESS 0
#define FLAT_HASH_INSERT_FAIL_TABLE_FULL 1
#define FLAT_HASH_INSERT_FAIL_BUCKET_FULL 2
#define FLAT_HASH_DELETE_SUCCESS 0
#define FLAT_HASH_DELETE_FAIL_TABLE_EMPTY 1
#define FLAT_HASH_DELETE_FAIL_BUCKET_EMPTY 2
#define FLAT_HASH_DELETE_FAIL_KEY_NOT_FOUND 3


private:
	using ValueManager = Pool<sizeof(Value), true>;
	using ValKeyPair   = std::pair<Value*, Key>;
	static constexpr f32 cm_growth_factor = 1.5f;
    static constexpr f32 cm_load_factor   = 0.70f;


	size_t current_load() { return m_tableSize / m_buckets.size(); }


	constexpr size_t maxKeysPerBucket(size_t nodes, size_t buckets) {
		f32 out = __scast(f32, nodes) / buckets;
		return __scast(size_t, (out > 0.0f && out < 1.0f) + out); 
	}

	constexpr size_t minimumNodesFromBuckets(size_t buckets) {
		return __scast(size_t, __scast(f32, buckets) * (1 / cm_load_factor) );
	}



	template<bool KeyValPairIterator = false> struct UTIL2_API Iterator
	{
	private:
		std::vector<ValKeyPair> const& m_pairs; /* [index, key] pairs */
		u32 push;

		using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
		using value_type 		= typename std::conditional<KeyValPairIterator, ValKeyPair, Value>::type;
        using pointer           = value_type*;
        using reference         = value_type&;


	public:
		Iterator(std::vector<ValKeyPair> const& to_iterate_on, u32 offset) : m_pairs(to_iterate_on), push{offset} {}


		reference operator*() const {
			if constexpr (KeyValPairIterator) { return m_pairs[push]; }
			return *m_pairs[push].first;
		}
		pointer operator->() const {
			if constexpr (KeyValPairIterator) { return &m_pairs[push]; }
			return m_pairs[push].first;
		}
        Iterator& operator++() { ++push; return *this; }  
        friend bool operator==(const Iterator& a, const Iterator& b) { return a.operator->() == b.operator->(); }
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.operator->() != b.operator->(); }

	};


	using ValIterator    = Iterator<0>;
	using KeyValIterator = Iterator<1>;




	struct alignpk(64) KeyGroup {
		struct pack {
			u8 bits; /* first 5 bits: occupation of each node. */
			u8 pad[3];
		} ctrl_t;
		struct KeyNode {
			Key key;
			u32 index;
		} Node[5];


		KeyGroup() : ctrl_t{0b00000, {0}}, Node{ 
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 },
			{ 0, DEFAULT32 }
		} {}

		__force_inline u8 first_available() const { return __builtin_ffs(~ctrl_t.bits) -1;  } /* get index to first available block.     */
		__force_inline u8 first_occupied()  const { return __builtin_ffs( ctrl_t.bits) -1;  } /* make sure NOT to call when bits == 0 :) */
		__force_inline u8 length() 			const { return __builtin_popcount(ctrl_t.bits); }
		bool full()  const { return ctrl_t.bits == 0b11111; }
		bool empty() const { return ctrl_t.bits == 0b00000; }
		void push(const_ref<Key> key, u32 idx)
		{
			u8 ins = first_available();
			Node[ins]	 = { key, idx };
			ctrl_t.bits |= (1 << ins); /* set bit to 1, meaning occupied. */
			return;
		}
	};


	u8 bucket_lookup(u32 bidx, const_ref<Key> key) {
		u8 i = 5;
		
		i *= (m_buckets[bidx].ctrl_t.bits == 0);
		while( (key == m_buckets[bidx].Node[i].key) != true && i < 5) ++i;
		return i;
	}


	Value* bucket_lookup_ptr(u32 bidx, const_ref<Key> key)
	{
		u8  nidx = bucket_lookup(bidx, key);
		u32 vidx = 0;
		u64 ptr  = 0;
		bool found = (nidx != 5);

		/* instead of the if statement, we'll multiply each index value by the condition, that way we can avoid it. */
		bidx *= found; /* found=false: will give bucket 0 */
		nidx *= found; /* found=false: will give node   0 */
		vidx = found * m_buckets[bidx].Node[nidx].index; /* found=false: will give m_values[0] */
		ptr = found * __rcast(u64, &m_values[vidx]);     /* convert pointer to underlying value, multiply by boolean value for true pointer val */
		return __rcast(Value*, ptr); 					 /* found=false: will return nullptr   */
	}


	u32 bucket_del(u32 bidx, const_ref<Key> key) { /* assume the KeyGroup will ALWAYS contain atleast a single element before deletion. */
		u32 i = 0, valueIdx = 0;
		while( (key == m_buckets[bidx].Node[i].key) != true && i < 5) ++i;


		if(unlikely(i == 5)) /* i.e not found. */
			return DEFAULT32;
		else {
			m_buckets[bidx].ctrl_t.bits &= ~(1 << i); /* clear bit[i] to 0, meaning free. */
			valueIdx = m_buckets[bidx].Node[i].index;

			m_buckets[bidx].Node[i] = { 0, DEFAULT32 };
			return valueIdx;
		}
	}


	u32 bucket_del_nocleanup(u32 bidx, const_ref<Key> key)
	{
		u32 i = 0;
		while( (key == m_buckets[bidx].Node[i].key) != true && i < 5) ++i;

		if(unlikely(i == 5)) return DEFAULT32;

		m_buckets[bidx].ctrl_t.bits &= ~(1 << i);
		m_buckets[bidx].Node[i].key = 0;
		return m_buckets[bidx].Node[i].index;
	}



	/* [NOTE]: 
		Theres always the problem of, will the re-hash actually satisfy the requirement. 
		In most cases - it will. In VERY UNLIKELY cases, it won't.
		Thats why I'm letting the user take care of this >:)
	*/
	bool rehash_table(size_t newSize)
	{
		std::vector<KeyGroup> newTable(newSize);
		u8 insertSuccess = true, ctrl = 0, node = 0;


		u32 bidx = 0;
		hash.refresh();
		for(auto& bucket : m_buckets)
		{
			ctrl = bucket.ctrl_t.bits;
			while(ctrl && insertSuccess) {
				node = __builtin_ffs(ctrl) -1;
				bidx = hash(bucket.Node[node].key) % newSize; /* recompute hash for new size. */

				newTable[bidx].push( /* insert k, v pair to its new location in the new table. */
					bucket.Node[node].key,
					bucket.Node[node].index
				);
				
				// printf("calculated node %u at bucket %u with hash=%llX [key=%llu]\n", node, bidx, hash(bucket.Node[node].key), bucket.Node[node].key);
				insertSuccess = !newTable[bidx].full();
				ctrl = ctrl & ~(1u << node);
			}


			if(unlikely(!insertSuccess)) {
				// debug_messagefmt("bucket %u is full. Can't re-hash table (bigger size needed) (bitfield=%u)\n", bidx, newTable[bidx].ctrl_t.bits);
				break;
			}
			++bidx;
		}



		if(insertSuccess) {
			m_buckets.swap(newTable);
			++m_rehashed;
		}
		return insertSuccess;
	}



private:
	Hasher 				  hash;
	/* Value allocator & Value buffer (we only Own the pointer, we do not Manage it) */
	ValueManager 		  m_vmng;
	Value*                m_values;

	std::vector<ValKeyPair> m_iterator;

	std::vector<KeyGroup> m_buckets;
	u32 				  m_tableSize;
    u32                   m_collisions;
	u8                    m_rehashed;
	u8 					  pad[7];


public:
	void create(u32 buckets, u32 suggestedMaxNodeAmount = 0)
	{
		m_buckets.resize(buckets);
		/* if (suggestedMaxNodeAmount == 0) 
			-> suggestedMaxNodeAmount = minimumNodesFromBuckets(buckets) 
		*/
		suggestedMaxNodeAmount += (suggestedMaxNodeAmount == 0) * minimumNodesFromBuckets(buckets);
		// markfmt("created table with %u elements, %u buckets\n", suggestedMaxNodeAmount, buckets);
		m_values = util2_aligned_malloc(sizeof(Value) * suggestedMaxNodeAmount, sizeof(Value));		
		m_vmng.create(m_values, suggestedMaxNodeAmount);
		
		m_iterator.resize(suggestedMaxNodeAmount);

		m_tableSize  = 0;
		m_rehashed   = 0;
		m_collisions = 0;

		static const KeyGroup tmp{};
		std::fill(m_buckets.begin(), m_buckets.end(), tmp);
		hash.refresh();
		return;
	}


	void destroy()
	{
		m_tableSize  = 0;
		m_rehashed   = 0;
		m_collisions = 0;
		m_buckets.clear();
		m_iterator.clear();
		m_vmng.destroy();
		util2_aligned_free(m_values);
		return;
	}

	
	Value* lookup(const_ref<Key> k) {
		u32 bidx = hash(k) % m_buckets.size();
		return bucket_lookup_ptr(bidx, k);
	}


	u8 insert(const_ref<Key> k, const_ref<Value> v)
	{
		if(unlikely(m_vmng.size() == m_tableSize)) /* Hashtable needs resize */
			return FLAT_HASH_INSERT_FAIL_TABLE_FULL;

		u32    idx  = hash(k) % m_buckets.size(), vidx = 0;
		Value* find = bucket_lookup_ptr(idx, k);
		if(unlikely(find == nullptr && m_buckets[idx].full())) 
			return FLAT_HASH_INSERT_FAIL_BUCKET_FULL; /* Hashtable needs resize */


		if(likely(find == nullptr))
		{
			vidx = m_vmng.allocate_index(); /* get new value from alloc */


			/* Update Key, Value Pair Vector for iterator */
			ValKeyPair tmp = { &m_values[vidx], vidx };
			auto it = std::upper_bound( /* Find insertion point in the vector */
				m_iterator.begin(), 
				m_iterator.end(), 
				tmp, 
				[](ValKeyPair const& a, ValKeyPair const& b) { return a.second < b.second; } 
			);
			m_iterator.insert(it, tmp);


			m_buckets[idx].push(k, vidx); 	/* push into available position in bucket */
			++m_tableSize; 					/* table increased in size.        		  */
			m_collisions += m_buckets[idx].length() > 1; /* another collision occured if AFTER INSERTION there's atleast 2 elements. */
			find = &m_values[vidx];
		}
		*find = v; /* set value to what we were given. */


		// if(current_load() > cm_load_factor) { /* hash table re-hashing will be the users' job */
		// 	grow_buckets_rehash(__scast(size_t, __scast(f32, m_tableSize) * cm_growth_factor ));
		// }
		return FLAT_HASH_INSERT_SUCCESS;
	}


	u8 del(const_ref<Key> k)
	{
		if(unlikely(m_tableSize == 0)) return FLAT_HASH_DELETE_FAIL_TABLE_EMPTY;
		
		u32 idx = hash(k) % m_buckets.size();
		if(unlikely(m_buckets[idx].empty())) 
			return FLAT_HASH_DELETE_FAIL_BUCKET_EMPTY;
		
		u32 vidx = bucket_del(idx, k);
		if(unlikely(vidx == DEFAULT32))
			return FLAT_HASH_DELETE_FAIL_KEY_NOT_FOUND;


		m_vmng.free_index(vidx);
		m_iterator.erase(
			std::find( /* Not so fast really, but good enough for now. */
				m_iterator.begin(), 
				m_iterator.end(), 
				ValKeyPair{ &m_values[vidx], k }
		));
		--m_tableSize;
		return FLAT_HASH_DELETE_SUCCESS;
	}


	bool rehash(size_t suggestedNewSize = 0)
	{
		size_t tmp = __scast(size_t, __scast(f32, m_buckets.size()) * cm_growth_factor );
		suggestedNewSize += (suggestedNewSize == 0) * tmp;
		return rehash_table(suggestedNewSize);
	}


	bool   need_rehash() const { return current_load() > cm_load_factor; }
	bool   full()     const { return m_tableSize == m_vmng.size(); }
	bool   empty()    const { return m_tableSize == 0; 			   }
	size_t buckets()  const { return m_buckets.size(); 			   }
	size_t size()     const { return m_tableSize;      			   }
	size_t max_size() const { return m_vmng.size();    			   }
	Value* data()     const { return m_values;    	   			   }


	KeyGroup* bucketBegin() const { return m_buckets.begin();      }
	KeyGroup* bucketEnd()   const { return m_buckets.end();        }
	ValIterator begin() const { return ValIterator(m_iterator, 0					); }
	ValIterator end()   const { return ValIterator(m_iterator, m_iterator.size() - 1); }


	decltype(m_iterator) const& asMapIterator() const { return m_iterator; }


	void printIterator() const
	{
		printf("Iterator of %u Elements: { ", m_tableSize);
		for(size_t i = 0; i < m_iterator.size(); ++i) {
			printf("[%p, %llu], ", (void*)m_iterator[i].first, m_iterator[i].second);
		}
		printf("\b\b }\n\n\n\n");
	}


	void to_file(const_ref<StringFromKV> print, FILE* file=stdout)
	{
		fprintf(file, "Hashtable:\n    %llu Buckets\n    %u/%llu Nodes [%u Allocated (%llu Bytes)]\n    %5u Collisions\n    %u     Re-hashes\n    Contents:\n",
			m_buckets.size(),
			m_tableSize,
			m_vmng.size(),
			m_tableSize,
			m_tableSize * sizeof(KeyGroup),
			m_collisions,
			m_rehashed
		);
		fprintf(file, "    [Bucket ID] [  Nodes   ]: [key, value] => [... , ...]\n");


		size_t ctrl = 0, node = 0;
		std::string tmpstr;


		for(size_t i = 0; i < m_buckets.size(); ++i)
		{
			ctrl = m_buckets[i].ctrl_t.bits;
			fprintf(file, "    [%9llu] [%u Elements]: ", i, __builtin_popcountll(ctrl));

			while(ctrl)
			{
				node   = __builtin_ffsll(ctrl) -1;
				tmpstr = print(
					m_buckets[i].Node[node].key, 
					m_values[m_buckets[i].Node[node].index]
				);
				fprintf(file, " [%s] --> ", tmpstr.c_str());
				ctrl = ctrl & ~(1u << node);
			}
			fprintf(file, " || \n");
		}
		return;
	}


	/*
		fnType = 0: Insert
		fnType = 1: Lookup
		fnType = 2: Delete
		fnType = 3: Rehash
		NOTE: 
		statusCode for lookup is value_ptr == nullptr,
		otherwise its the statusCode returned from the function.
	*/
	static const char* statusToString(u8 fnType, u8 statusCode) 
	{
		static constexpr std::array<const char*, 11> statusCodes = {
			"INSERT_SUCCESS           ",
			"FLAT_HASH_INSERT_FAIL_TABLE_FULL   ",
			"FLAT_HASH_INSERT_FAIL_BUCKET_FULL  ",
			"LOOKUP_SUCCESS           ", /* Returned ptr != nullptr */
			"LOOKUP_FAIL_NOT_FOUND    ",
			"FLAT_HASH_DELETE_SUCCESS           ",
			"FLAT_HASH_DELETE_FAIL_TABLE_EMPTY  ",
			"FLAT_HASH_DELETE_FAIL_BUCKET_EMPTY ",
			"FLAT_HASH_DELETE_FAIL_KEY_NOT_FOUND",
			"REHASH_FAIL             ",
			"REHASH_SUCCESS           "
		};
		static constexpr u8 offsets[4] = { 0, 3, 5, 9 };
		return statusCodes[offsets[fnType] + statusCode];
	}


	#undef FLAT_HASH_INSERT_FAIL_BUCKET_FULL
	#undef FLAT_HASH_INSERT_FAIL_TABLE_FULL
	#undef INSERT_SUCCESS
	#undef FLAT_HASH_DELETE_FAIL_KEY_NOT_FOUND
	#undef FLAT_HASH_DELETE_FAIL_BUCKET_EMPTY
	#undef FLAT_HASH_DELETE_FAIL_TABLE_EMPTY
	#undef FLAT_HASH_DELETE_SUCCESS
};


} /* namespace util2 */


#endif /* __UTIL2_FLAT_ARRAY_HASH_TABLE__ */