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
 * @file		object.c
 * @ingroup 	object
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_IMPL_TAG 		"object"

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "object.h"

/* ///////////////////////////////////////////////////////////////////////
 * globals
 */

// the object xml reader
static tb_hash_t* 				g_object_xml_reader = tb_null;

// the object xml writer
static tb_hash_t* 				g_object_xml_writer = tb_null;

// the object bin reader
static tb_hash_t* 				g_object_bin_reader = tb_null;

// the object bin writer
static tb_hash_t* 				g_object_bin_writer = tb_null;

/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */
static tb_object_t* tb_object_read_xml(tb_gstream_t* gst)
{
	// init reader 
	tb_handle_t reader = tb_xml_reader_init(gst);
	tb_assert_and_check_return_val(reader, tb_null);

	// init object
	tb_object_t* object = tb_null;

	// walk
	tb_size_t event = TB_XML_READER_EVENT_NONE;
	while ((event = tb_xml_reader_next(reader)) && !object)
	{
		switch (event)
		{
		case TB_XML_READER_EVENT_ELEMENT_EMPTY: 
		case TB_XML_READER_EVENT_ELEMENT_BEG: 
			{
				// name
				tb_char_t const* name = tb_xml_reader_element(reader);
				tb_assert_and_check_goto(name, end);

				// func
				tb_object_xml_reader_func_t func = tb_object_get_xml_reader(name);
				tb_assert_and_check_goto(func, end);

				// read
				object = func(reader, event);
			}
			break;
		default:
			break;
		}
	}

end:

	// exit reader
	tb_xml_reader_exit(reader);

	// ok?
	return object;
}
static tb_bool_t tb_object_writ_xml(tb_object_t* object, tb_gstream_t* gst, tb_bool_t deflate)
{
	// func
	tb_object_xml_writer_func_t func = tb_object_get_xml_writer(object->type);
	tb_assert_and_check_return_val(func, tb_false);

	// writ xml header
	tb_gstream_printf(gst, "<?xml version=\"2.0\" encoding=\"utf-8\"?>");
	tb_object_writ_newline(gst, deflate);

	// writ
	tb_bool_t ok = func(object, gst, deflate, 0);

	// flush
	tb_gstream_bfwrit(gst, tb_null, 0);

	// ok
	return ok;
}
static tb_object_t* tb_object_read_bin(tb_gstream_t* gst)
{
	// init 
	tb_object_t* object = tb_null;

	// read bin header
	tb_byte_t data[32] = {0};
	if (!tb_gstream_bread(gst, data, 5)) return tb_null;

	// check 
	if (!tb_strnicmp(data, "tbo00", 5)) return tb_null;

	// walk
	tb_uint8_t flag = 0;
	while (flag = tb_gstream_bread_u8(gst))
	{
		// type & size
		tb_size_t type = flag >> 4;
		tb_size_t size = flag & 0x0f;
		if (type == 0xf) type = tb_gstream_bread_u8(gst);
		switch (size)
		{
		case 0xd:
			size = tb_gstream_bread_u8(gst);
			break;
		case 0xe:
			size = tb_gstream_bread_u16_be(gst);
			break;
		case 0xf:
			size = tb_gstream_bread_u32_be(gst);
			break;
		default:
			tb_assert_and_check_return_val(0, tb_null);
			break;
		}
		tb_trace_impl("type: %lu, size: %lu", type, size);
	
		// func
		tb_object_bin_reader_func_t func = tb_object_get_bin_reader(type);
		tb_assert_and_check_return_val(func, tb_null);

		tb_object_t* o = func(gst, type, size);
	}

	// ok?
	return object;
}
static tb_bool_t tb_object_writ_bin(tb_object_t* object, tb_gstream_t* gst, tb_bool_t deflate)
{
	// func
	tb_object_bin_writer_func_t func = tb_object_get_bin_writer(object->type);
	tb_assert_and_check_return_val(func, tb_false);

	// writ bin header
	if (!tb_gstream_bwrit(gst, "tbo00", 5)) return tb_false;

	// writ
	if (!func(object, gst)) return tb_false;

	// writ end
	if (!tb_gstream_bwrit_u8(gst, 0)) return tb_false;

	// flush
	tb_gstream_bfwrit(gst, tb_null, 0);

	// ok
	return tb_true;
}

/* ///////////////////////////////////////////////////////////////////////
 * interfaces
 */

tb_bool_t tb_object_init_reader()
{
	// init data
	if (!tb_data_init_reader()) return tb_false;

	// init date
	if (!tb_date_init_reader()) return tb_false;

	// init array
	if (!tb_array_init_reader()) return tb_false;

	// init number
	if (!tb_number_init_reader()) return tb_false;

	// init string
	if (!tb_string_init_reader()) return tb_false;

	// init boolean
	if (!tb_boolean_init_reader()) return tb_false;

	// init dictionary
	if (!tb_dictionary_init_reader()) return tb_false;

	// ok
	return tb_true;
}
tb_void_t tb_object_exit_reader()
{
	// exit the object xml reader
	if (g_object_xml_reader) tb_hash_exit(g_object_xml_reader);
	g_object_xml_reader = tb_null;
		
	// exit the object bin reader
	if (g_object_bin_reader) tb_hash_exit(g_object_bin_reader);
	g_object_bin_reader = tb_null;
}
tb_bool_t tb_object_init_writer()
{
	// init data
	if (!tb_data_init_writer()) return tb_false;

	// init date
	if (!tb_date_init_writer()) return tb_false;

	// init array
	if (!tb_array_init_writer()) return tb_false;

	// init number
	if (!tb_number_init_writer()) return tb_false;

	// init string
	if (!tb_string_init_writer()) return tb_false;

	// init boolean
	if (!tb_boolean_init_writer()) return tb_false;

	// init dictionary
	if (!tb_dictionary_init_writer()) return tb_false;

	// ok
	return tb_true;
}
tb_void_t tb_object_exit_writer()
{
	// exit the object xml writer
	if (g_object_xml_writer) tb_hash_exit(g_object_xml_writer);
	g_object_xml_writer = tb_null;

	// exit the object bin writer
	if (g_object_bin_writer) tb_hash_exit(g_object_bin_writer);
	g_object_bin_writer = tb_null;
}
tb_bool_t tb_object_init(tb_object_t* object, tb_size_t flag, tb_size_t type)
{
	// check
	tb_assert_and_check_return_val(object, tb_false);

	// init
	tb_memset(object, 0, sizeof(tb_object_t));
	object->flag = flag;
	object->type = type;
	object->refn = 1;

	// ok
	return tb_true;
}
tb_void_t tb_object_exit(tb_object_t* object)
{
	// check
	tb_assert_and_check_return(object);

	// readonly?
	tb_check_return(!(object->flag & TB_OBJECT_FLAG_READONLY));

	// refn must be 1
	tb_size_t refn = tb_object_ref(object);
	tb_assert_and_check_return(refn == 1);

	// exit
	tb_object_dec(object);
}
tb_void_t tb_object_cler(tb_object_t* object)
{
	// check
	tb_assert_and_check_return(object);

	// readonly?
	tb_check_return(!(object->flag & TB_OBJECT_FLAG_READONLY));

	// clear
	if (object->cler) object->cler(object);
}
tb_void_t tb_object_setp(tb_object_t* object, tb_cpointer_t priv)
{
	// check
	tb_assert_and_check_return(object);
	object->priv = priv;
}
tb_cpointer_t tb_object_getp(tb_object_t* object)
{
	// check
	tb_assert_and_check_return_val(object, tb_null);
	return object->priv;
}
tb_object_t* tb_object_copy(tb_object_t* object)
{
	// check
	tb_assert_and_check_return_val(object && object->copy, tb_null);

	// copy
	return object->copy(object);
}
tb_size_t tb_object_type(tb_object_t* object)
{
	tb_assert_and_check_return_val(object, TB_OBJECT_TYPE_NONE);
	return object->type;
}
tb_object_t* tb_object_data(tb_object_t* object, tb_size_t format)
{	
	// check
	tb_assert_and_check_return_val(object, tb_null);

	// done
	tb_object_t* 	odata = tb_null;
	tb_size_t 		maxn = 4096;
	tb_byte_t* 		data = tb_null;
	do
	{
		// make data
		data = data? tb_ralloc(data, maxn) : tb_malloc(maxn);
		tb_assert_and_check_break(data);

		// open stream
		tb_gstream_t* gst = tb_gstream_init_from_data(data, maxn);
		if (gst && tb_gstream_bopen(gst))
		{
			// writ object
			if (tb_object_writ(object, gst, format))
			{
				// size
				tb_size_t size = (tb_size_t)tb_gstream_offset(gst);

				// the data object
				if (size < maxn) 
				{
					odata = tb_data_init_from_data(data, size);
					data[size] = '\0';
				}
				else maxn <<= 1;
			}

			// exit stream
			tb_gstream_exit(gst);
		}

	} while (!odata);

	// exit data
	if (data) tb_free(data);
	data = tb_null;

	// ok?
	return odata;
}
tb_void_t tb_object_dump(tb_object_t* object)
{
	// check
	tb_assert_and_check_return(object);

	// data
	tb_object_t* odata = tb_object_data(object, TB_OBJECT_FORMAT_XML);
	if (odata)
	{
		// data & size 
		tb_byte_t const* 	data = tb_data_addr(odata);
		tb_size_t 			size = tb_data_size(odata);
		if (data && size)
		{
			tb_char_t const* 	p = tb_strstr(data, "?>");
			tb_char_t 			b[4096 + 1];
			if (p)
			{
				p += 2;
				while (*p && tb_isspace(*p)) p++;
				while (*p)
				{
					tb_char_t* 			q = b;
					tb_char_t const* 	d = b + 4096;
					for (; q < d && *p; p++, q++) *q = *p;
					*q = '\0';
					tb_printf("%s", b);
				}
				tb_printf("\n");
			}
		}

		// exit data
		tb_object_exit(odata);
	}
}
tb_size_t tb_object_ref(tb_object_t* object)
{
	tb_assert_and_check_return_val(object, 0);
	return object->refn;
}
tb_void_t tb_object_inc(tb_object_t* object)
{
	// check
	tb_assert_and_check_return(object);

	// readonly?
	tb_check_return(!(object->flag & TB_OBJECT_FLAG_READONLY));

	// refn++
	object->refn++;
}
tb_void_t tb_object_dec(tb_object_t* object)
{
	// check
	tb_assert_and_check_return(object);

	// readonly?
	tb_check_return(!(object->flag & TB_OBJECT_FLAG_READONLY));

	// refn--
	if (object->refn > 1) object->refn--;
	else if (object->exit) object->exit(object);
}
tb_object_t* tb_object_read(tb_gstream_t* gst)
{
	// check
	tb_assert_and_check_return_val(gst, tb_null);

	// probe format
	tb_byte_t* p = tb_null;
	if (!tb_gstream_bneed(gst, &p, 3)) return tb_null;
	tb_assert_and_check_return_val(p, tb_null);

	// read
	return !tb_strnicmp(p, "tbo", 3)? tb_object_read_bin(gst) : tb_object_read_xml(gst);
}
tb_object_t* tb_object_read_from_data(tb_byte_t const* data, tb_size_t size)
{
	// check
	tb_assert_and_check_return_val(data && size, tb_null);

	// init
	tb_object_t* object = tb_null;

	// make stream
	tb_gstream_t* gst = tb_gstream_init_from_data(data, size);
	if (gst && tb_gstream_bopen(gst))
	{
		// read object
		object = tb_object_read(gst);

		// exit stream
		tb_gstream_exit(gst);
	}

	// ok?
	return object;
}
tb_object_t* tb_object_read_from_url(tb_char_t const* url)
{
	// check
	tb_assert_and_check_return_val(url, tb_null);

	// init
	tb_object_t* object = tb_null;

	// make stream
	tb_gstream_t* gst = tb_gstream_init_from_url(url);
	if (gst && tb_gstream_bopen(gst))
	{
		// read object
		object = tb_object_read(gst);

		// exit stream
		tb_gstream_exit(gst);
	}

	// ok?
	return object;
}
tb_bool_t tb_object_writ(tb_object_t* object, tb_gstream_t* gst, tb_size_t format)
{
	// check
	tb_assert_and_check_return_val(object && gst, tb_false);

	// writ
	switch (format & 0x00ff)
	{
	case TB_OBJECT_FORMAT_XML:
		return tb_object_writ_xml(object, gst, format & TB_OBJECT_FORMAT_DEFLATE? tb_true : tb_false);
	case TB_OBJECT_FORMAT_BIN:
		return tb_object_writ_bin(object, gst, format & TB_OBJECT_FORMAT_DEFLATE? tb_true : tb_false);
	default:
		tb_assert(0);
		break;
	}

	return tb_false;
}
tb_bool_t tb_object_writ_to_url(tb_object_t* object, tb_char_t const* url, tb_size_t format)
{
	// check
	tb_assert_and_check_return_val(object && url, tb_false);

	// init
	tb_bool_t ok = tb_false;

	// make stream
	tb_gstream_t* gst = tb_gstream_init_from_url(url);
	if (gst)
	{
		if (tb_gstream_type(gst) == TB_GSTREAM_TYPE_FILE)
			tb_gstream_ctrl(gst, TB_FSTREAM_CMD_SET_FLAGS, TB_FILE_WO | TB_FILE_CREAT | TB_FILE_TRUNC);
		if (tb_gstream_bopen(gst))
		{
			if (tb_object_writ(object, gst, format)) ok = tb_true;
		}
		tb_gstream_exit(gst);
	}

	// ok?
	return ok;
}
tb_bool_t tb_object_set_xml_reader(tb_char_t const* type, tb_object_xml_reader_func_t func)
{
	// check
	tb_assert_and_check_return_val(type && func, tb_false);

	// init reader
	if (!g_object_xml_reader)
		g_object_xml_reader = tb_hash_init(TB_HASH_SIZE_MICRO, tb_item_func_str(tb_false, tb_null), tb_item_func_ptr());
	tb_assert_and_check_return_val(g_object_xml_reader, tb_false);

	// set
	tb_hash_set(g_object_xml_reader, type, func);

	// ok
	return tb_true;
}
tb_pointer_t tb_object_get_xml_reader(tb_char_t const* type)
{
	// check
	tb_assert_and_check_return_val(g_object_xml_reader, tb_null);

	// get
	return tb_hash_get(g_object_xml_reader, type);
}
tb_bool_t tb_object_set_xml_writer(tb_size_t type, tb_object_xml_writer_func_t func)
{
	// check
	tb_assert_and_check_return_val(type && func, tb_false);

	// init writer
	if (!g_object_xml_writer)
		g_object_xml_writer = tb_hash_init(TB_HASH_SIZE_MICRO, tb_item_func_uint32(), tb_item_func_ptr());
	tb_assert_and_check_return_val(g_object_xml_writer, tb_false);

	// set
	tb_hash_set(g_object_xml_writer, (tb_pointer_t)type, func);

	// ok
	return tb_true;
}
tb_pointer_t tb_object_get_xml_writer(tb_size_t type)
{
	// check
	tb_assert_and_check_return_val(g_object_xml_writer, tb_null);

	// get
	return tb_hash_get(g_object_xml_writer, (tb_pointer_t)type);
}
tb_bool_t tb_object_set_bin_reader(tb_size_t type, tb_object_bin_reader_func_t func)
{
	// check
	tb_assert_and_check_return_val(type && func, tb_false);

	// init reader
	if (!g_object_bin_reader)
		g_object_bin_reader = tb_hash_init(TB_HASH_SIZE_MICRO, tb_item_func_uint32(), tb_item_func_ptr());
	tb_assert_and_check_return_val(g_object_bin_reader, tb_false);

	// set
	tb_hash_set(g_object_bin_reader, (tb_pointer_t)type, func);

	// ok
	return tb_true;
}
tb_pointer_t tb_object_get_bin_reader(tb_size_t type)
{
	// check
	tb_assert_and_check_return_val(g_object_bin_reader, tb_null);

	// get
	return tb_hash_get(g_object_bin_reader, (tb_pointer_t)type);
}
tb_bool_t tb_object_set_bin_writer(tb_size_t type, tb_object_bin_writer_func_t func)
{
	// check
	tb_assert_and_check_return_val(type && func, tb_false);

	// init writer
	if (!g_object_bin_writer)
		g_object_bin_writer = tb_hash_init(TB_HASH_SIZE_MICRO, tb_item_func_uint32(), tb_item_func_ptr());
	tb_assert_and_check_return_val(g_object_bin_writer, tb_false);

	// set
	tb_hash_set(g_object_bin_writer, (tb_pointer_t)type, func);

	// ok
	return tb_true;
}
tb_pointer_t tb_object_get_bin_writer(tb_size_t type)
{
	// check
	tb_assert_and_check_return_val(g_object_bin_writer, tb_null);

	// get
	return tb_hash_get(g_object_bin_writer, (tb_pointer_t)type);
}
