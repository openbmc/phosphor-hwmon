#pragma once

#include <cstdlib>

/** @struct Free
 *  @brief A malloc cleanup type for use with smart pointers.
 */
template <typename T>
struct Free
{
    void operator()(T* ptr) const
    {
        free(ptr);
    }
};

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
