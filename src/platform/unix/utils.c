/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2012, ruki All rights reserved.
 *
 * @author		ruki
 * @file		utils.c
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../utils.h"
#include "../../libc/libc.h"
#include <unistd.h>
#include <stdio.h>
#ifdef TB_CONFIG_OS_ANDROID
# 	include <android/log.h>     
#endif

/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */

// printf
tb_void_t tb_printf(tb_char_t const* fmt, ...)
{
	tb_long_t ret = 0;
	tb_char_t msg[8192] = {0};
	tb_va_format(msg, 8192, fmt, &ret);
	if (ret >= 0) msg[ret] = '\0';

#ifdef TB_CONFIG_OS_ANDROID
	__android_log_print(ANDROID_LOG_DEBUG, "tbox", "%s", msg);
#else
	printf("%s", msg);
	fflush(stdout);
#endif
}

// the host name
tb_bool_t tb_hostname(tb_char_t* name, tb_size_t size)
{
	return !gethostname(name, size)? tb_true : tb_false;
}

