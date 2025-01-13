#ifndef __UTIL_STATIC_ASSERT__
#define __UTIL_STATIC_ASSERT__


#if !defined DEFAULT_STATIC_ASSERT_IMPL
#   define DEFAULT_STATIC_ASSERT_IMPL(Condition, UniqueIdentifier, Message) \
        typedef char STATIC_ASSERTION__##UniqueIdentifier[ !!(Condition) ? 1 : -1];
#endif


#if defined UTIL_STATIC_ASSERT
#   error "For Some reason there is an imposter running around using my name, you should probably find out who it is"
#endif

#ifdef __cplusplus
#   if __cplusplus < 201103L /* Pre C++11, we'll use the default macro */
#       define UTIL_STATIC_ASSERT(Condition, UniqueID, Message) \
                DEFAULT_STATIC_ASSERT_IMPL(Condition, UniqueID, Message)

#   else /* Post C++11, we can use static_assert */
#       define UTIL_STATIC_ASSERT(Condition, UniqueID, Message) \
                static_assert(Condition, Message);

#   endif
#elif defined __STDC_VERSION__
#   if __STDC_VERSION__ < 201112L /* pre C11, we'll use the default macro */
#       define UTIL_STATIC_ASSERT(Condition, UniqueID, Message) \
                _Static_assert(Condition, Message);

#   else /* post C11, we can use _Static_assert */
#       define UTIL_STATIC_ASSERT(Condition, UniqueID, Message) \
                DEFAULT_STATIC_ASSERT_IMPL(Condition, UniqueID, Message)

#   endif
#endif


#endif /* __UTIL_STATIC_ASSERT__ */