#include "util2/time.hpp"
#include "util2/ifcrash.hpp"


static util2::Time::Timestamp g_generalPurposeStamp[UTIL2_TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM];


util2::Time::Timestamp& util2::Time::getGeneralPurposeStamp(u8 index)
{
    ifcrash(index > UTIL2_TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM); 
    return g_generalPurposeStamp[index];
}
