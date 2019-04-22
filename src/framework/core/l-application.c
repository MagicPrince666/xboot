/*
 * framework/core/l-application.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <xfs/xfs.h>
#include <framework/core/l-image.h>
#include <framework/core/l-application.h>

static cairo_status_t xfs_read_func(void * closure, unsigned char * data, unsigned int size)
{
	struct xfs_file_t * file = closure;
	size_t len = 0, n;

	while(size > 0)
	{
		n = xfs_read(file, data, size);
		if(n <= 0)
			break;
		size -= n;
		len += n;
		data += n;
	}
	if(len > 0)
		return CAIRO_STATUS_SUCCESS;
	return _cairo_error(CAIRO_STATUS_READ_ERROR);
}

static cairo_surface_t * cairo_image_surface_create_from_png_xfs(struct xfs_context_t * ctx, const char * filename)
{
	struct xfs_file_t * file;
	cairo_surface_t * cs;

	file = xfs_open_read(ctx, filename);
	if(!file)
		return _cairo_surface_create_in_error(_cairo_error(CAIRO_STATUS_FILE_NOT_FOUND));
	cs = cairo_image_surface_create_from_png_stream(xfs_read_func, file);
	xfs_close(file);
	return cs;
}

static int application_detect(struct lapplication_t * app, const char * path)
{
	struct xfs_context_t * ctx;
	struct xfs_file_t * file;
	struct json_value_t * v, * t;
	char * json, * p;
	size_t len;
	int i;

	ctx = xfs_alloc(path, 0);
	if(ctx && xfs_isfile(ctx, "main.lua"))
	{
		memset(app, 0, sizeof(struct lapplication_t));
		app->path = strdup(path);

		file = xfs_open_read(ctx, "manifest.json");
		if(file && (len = xfs_length(file)) > 0)
		{
			json = malloc(len + 1);
			if(json && (len = xfs_read(file, json, len)) > 0)
			{
				v = json_parse(json, len, NULL);
				if(v && (v->type == JSON_OBJECT))
				{
					for(i = 0; i < v->u.object.length; i++)
					{
						p = v->u.object.values[i].name;
						if(strcmp(p, "name") == 0)
						{
							t = v->u.object.values[i].value;
							if(t && (t->type == JSON_STRING))
								app->name = strdup(t->u.string.ptr);
						}
						else if(strcmp(p, "description") == 0)
						{
							t = v->u.object.values[i].value;
							if(t && (t->type == JSON_STRING))
								app->desc = strdup(t->u.string.ptr);
						}
						else if(strcmp(p, "author") == 0)
						{
							t = v->u.object.values[i].value;
							if(t && (t->type == JSON_STRING))
								app->author = strdup(t->u.string.ptr);
						}
						else if(strcmp(p, "website") == 0)
						{
							t = v->u.object.values[i].value;
							if(t && (t->type == JSON_STRING))
								app->website = strdup(t->u.string.ptr);
						}
						else if(strcmp(p, "version") == 0)
						{
							t = v->u.object.values[i].value;
							if(t && (t->type == JSON_STRING))
								app->version = strdup(t->u.string.ptr);
						}
					}
				}
				json_free(v);
			}
			free(json);
		}
		xfs_close(file);

		if(!app->name)
		{
			p = strdup(path);
			app->name = strdup(basename(p));
			free(p);
		}

		app->logo = cairo_image_surface_create_from_png_xfs(ctx, "logo.png");
		if(cairo_surface_status(app->logo) != CAIRO_STATUS_SUCCESS)
			app->logo = NULL;

		app->panel = cairo_image_surface_create_from_png_xfs(ctx, "panel.png");
		if(cairo_surface_status(app->panel) != CAIRO_STATUS_SUCCESS)
			app->panel = NULL;

		xfs_free(ctx);
		return 1;
	}
	xfs_free(ctx);
	return 0;
}

static int l_application_new(lua_State * L)
{
	const char * path = luaL_checkstring(L, 1);
	struct lapplication_t * app = lua_newuserdata(L, sizeof(struct lapplication_t));
	if(!application_detect(app, path))
		return 0;
	luaL_setmetatable(L, MT_APPLICATION);
	return 1;
}

static int l_application_list(lua_State * L)
{
	struct lapplication_t * app;
	struct vfs_stat_t st;
	struct vfs_dirent_t dir;
	struct slist_t * sl, * e;
	const char * path;
	int fd;

	sl = slist_alloc();
	path = "/application";
	if(vfs_stat(path, &st) >= 0 && S_ISDIR(st.st_mode))
	{
		if((fd = vfs_opendir(path)) >= 0)
		{
			while(vfs_readdir(fd, &dir) >= 0)
			{
				if(dir.d_name && (dir.d_name[0] == '.'))
					continue;
				slist_add(sl, NULL, "%s/%s", path, dir.d_name);
			}
			vfs_closedir(fd);
		}
	}
	path = "/private/application";
	if(vfs_stat(path, &st) >= 0 && S_ISDIR(st.st_mode))
	{
		if((fd = vfs_opendir(path)) >= 0)
		{
			while(vfs_readdir(fd, &dir) >= 0)
			{
				if(dir.d_name && (dir.d_name[0] == '.'))
					continue;
				slist_add(sl, NULL, "%s/%s", path, dir.d_name);
			}
			vfs_closedir(fd);
		}
	}
	slist_sort(sl);
	lua_newtable(L);
	slist_for_each_entry(e, sl)
	{
		app = lua_newuserdata(L, sizeof(struct lapplication_t));
		if(!application_detect(app, e->key))
			continue;
		luaL_setmetatable(L, MT_APPLICATION);
		lua_setfield(L, -2, e->key);
	}
	slist_free(sl);
	return 1;
}

static const luaL_Reg l_application[] = {
	{"new",		l_application_new},
	{"list",	l_application_list},
	{NULL,	NULL}
};

static int m_application_gc(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	if(app->path)
		free(app->path);
	if(app->name)
		free(app->name);
	if(app->desc)
		free(app->desc);
	if(app->author)
		free(app->author);
	if(app->website)
		free(app->website);
	if(app->logo)
		cairo_surface_destroy(app->logo);
	if(app->panel)
		cairo_surface_destroy(app->panel);
	return 0;
}

static int m_application_get_path(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->path);
	return 1;
}

static int m_application_get_name(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->name);
	return 1;
}

static int m_application_get_description(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->desc);
	return 1;
}

static int m_application_get_author(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->author);
	return 1;
}

static int m_application_get_website(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->website);
	return 1;
}

static int m_application_get_version(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	lua_pushstring(L, app->version);
	return 1;
}

static int m_application_get_logo(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	if(!app->logo)
		return 0;
	struct limage_t * img = lua_newuserdata(L, sizeof(struct limage_t));
	int w = cairo_image_surface_get_width(app->logo);
	int h = cairo_image_surface_get_height(app->logo);
	img->cs = cairo_surface_create_similar(app->logo, cairo_surface_get_content(app->logo), w, h);
	cairo_t * cr = cairo_create(img->cs);
	cairo_set_source_surface(cr, app->logo, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	luaL_setmetatable(L, MT_IMAGE);
	return 1;
}

static int m_application_get_panel(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	if(!app->panel)
		return 0;
	struct limage_t * img = lua_newuserdata(L, sizeof(struct limage_t));
	int w = cairo_image_surface_get_width(app->panel);
	int h = cairo_image_surface_get_height(app->panel);
	img->cs = cairo_surface_create_similar(app->panel, cairo_surface_get_content(app->panel), w, h);
	cairo_t * cr = cairo_create(img->cs);
	cairo_set_source_surface(cr, app->panel, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	luaL_setmetatable(L, MT_IMAGE);
	return 1;
}

static int m_application_execute(lua_State * L)
{
	struct lapplication_t * app = luaL_checkudata(L, 1, MT_APPLICATION);
	const char * fb = luaL_optstring(L, 2, NULL);
	const char * input = luaL_optstring(L, 3, NULL);
	lua_pushboolean(L, (vmexec(app->path, fb, input) < 0) ? 0 : 1);
	return 1;
}

static const luaL_Reg m_application[] = {
	{"__gc",			m_application_gc},
	{"getPath",			m_application_get_path},
	{"getName",			m_application_get_name},
	{"getDescription",	m_application_get_description},
	{"getAuthor",		m_application_get_author},
	{"getWebsite",		m_application_get_website},
	{"getVersion",		m_application_get_version},
	{"getLogo",			m_application_get_logo},
	{"getPanel",		m_application_get_panel},
	{"execute",			m_application_execute},
	{NULL,	NULL}
};

int luaopen_application(lua_State * L)
{
	luaL_newlib(L, l_application);
	luahelper_create_metatable(L, MT_APPLICATION, m_application);
	return 1;
}
