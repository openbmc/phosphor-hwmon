#pragma once

#include <cstdlib>

namespace phosphor
{
namespace utility
{
/** @struct Free
 *  @brief A malloc cleanup type for use with smart pointers.
 */
template <typename T> struct Free
{
    void operator()(T* ptr) const
    {
        free(ptr);
    }
};
} // namespace utility
} // namespace phosphor
// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
