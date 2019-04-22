#ifndef __FRAMEWORK_CORE_L_APPLICATION_H__
#define __FRAMEWORK_CORE_L_APPLICATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <cairo.h>
#include <cairoint.h>
#include <framework/luahelper.h>

#define MT_APPLICATION	"__mt_application__"

struct lapplication_t {
	char * path;
	char * name;
	char * desc;
	char * author;
	char * website;
	char * version;
	cairo_surface_t * logo;
	cairo_surface_t * panel;
};

int luaopen_application(lua_State * L);

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_CORE_L_APPLICATION_H__ */
