/*!The Treasure Box Library
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Copyright (C) 2009 - 2019, TBOOX Open Source Group.
 *
 * @author      ruki
 * @file        atomic.h
 *
 */
#ifndef TB_PLATFORM_WINDOWS_ATOMIC_H
#define TB_PLATFORM_WINDOWS_ATOMIC_H

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * macros
 */

#if !defined(tb_atomic_fetch_and_set) && !TB_CPU_BIT64
#   define tb_atomic_fetch_and_set(a, v)        tb_atomic_fetch_and_set_windows(a, v)
#endif

#if !defined(tb_atomic_compare_and_set)
#   define tb_atomic_compare_and_set(a, p, v)   tb_atomic_compare_and_set_windows(a, p, v)
#endif

#if !defined(tb_atomic_fetch_and_cmpset)
#   define tb_atomic_fetch_and_cmpset(a, p, v)  tb_atomic_fetch_and_cmpset_windows(a, p, v)
#endif

#if !defined(tb_atomic_fetch_and_add) && !TB_CPU_BIT64
#   define tb_atomic_fetch_and_add(a, v)        tb_atomic_fetch_and_add_windows(a, v)
#endif

#if defined(MemoryBarrier)
#   define tb_memory_barrier()                  MemoryBarrier()
#elif defined(_AMD64_)
#   define tb_memory_barrier()                  __faststorefence()
#elif defined(_IA64_)
#   define tb_memory_barrier()                  __mf()
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * inlines
 */
#if TB_CPU_BIT64
static __tb_inline__ tb_long_t tb_atomic_fetch_and_pset_windows(tb_atomic_t* a, tb_long_t p, tb_long_t v)
{
    // check
    tb_assert(a && p);
    tb_assert_static(sizeof(tb_atomic_t) == sizeof(LONGLONG));

    tb_long_t e = *p;
    *p = (tb_long_t)InterlockedCompareExchange64((LONG __tb_volatile__*)a, v, e);
    return *p == e;
}
#else
static __tb_inline__ tb_long_t tb_atomic_fetch_and_set_windows(tb_atomic_t* a, tb_long_t v)
{
    return (tb_long_t)InterlockedExchange((LONG __tb_volatile__*)a, v);
}
static __tb_inline__ tb_bool_t tb_atomic_compare_and_set_windows(tb_atomic_t* a, tb_long_t* p, tb_long_t v)
{
    tb_assert(a && p);
    tb_long_t e = *p;
    *p = (tb_long_t)InterlockedCompareExchange((LONG __tb_volatile__*)a, v, e);
    return *p == e;
}
static __tb_inline__ tb_long_t tb_atomic_fetch_and_cmpset_windows(tb_atomic_t* a, tb_long_t p, tb_long_t v)
{
    tb_assert(a);
    return (tb_long_t)InterlockedCompareExchange((LONG __tb_volatile__*)a, v, p);
}
static __tb_inline__ tb_long_t tb_atomic_fetch_and_add_windows(tb_atomic_t* a, tb_long_t v)
{
    return (tb_long_t)InterlockedExchangeAdd((LONG __tb_volatile__*)a, v);
}
#endif

#endif
