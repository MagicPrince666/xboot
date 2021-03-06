/*
 * driver/fb-sandbox-sdl.c
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
#include <framebuffer/framebuffer.h>
#include <sandbox.h>

struct fb_sandbox_sdl_pdata_t
{
	int width;
	int height;
	int pwidth;
	int pheight;
	int bpp;
	void * priv;
};

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;
	sandbox_fb_sdl_set_backlight(pdat->priv, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;
	return sandbox_fb_sdl_get_backlight(pdat->priv);
}

static struct render_t * fb_create(struct framebuffer_t * fb)
{
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;
	struct sandbox_fb_surface_t * surface;
	struct render_t * render;

	surface = malloc(sizeof(struct sandbox_fb_surface_t));
	if(!surface)
		return NULL;

	if(!sandbox_fb_sdl_surface_create(pdat->priv, surface))
	{
		free(surface);
		return NULL;
	}

	render = malloc(sizeof(struct render_t));
	if(!render)
	{
		sandbox_fb_sdl_surface_destroy(pdat->priv, surface);
		free(surface);
		return NULL;
	}

	render->width = surface->width;
	render->height = surface->height;
	render->pitch = surface->pitch;
	render->format = PIXEL_FORMAT_ARGB32;
	render->pixels = surface->pixels;
	render->pixlen = surface->height * surface->pitch;
	render->priv = surface;

	return render;
}

static void fb_destroy(struct framebuffer_t * fb, struct render_t * render)
{
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;

	if(render)
	{
		sandbox_fb_sdl_surface_destroy(pdat->priv, render->priv);
		free(render->priv);
		free(render);
	}
}

static void fb_present(struct framebuffer_t * fb, struct render_t * render, struct dirty_rect_t * rect, int nrect)
{
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;
	sandbox_fb_sdl_surface_present(pdat->priv, render->priv, (struct sandbox_fb_dirty_rect_t *)rect, nrect);
}

static struct device_t * fb_sandbox_sdl_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_sandbox_sdl_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	int width = dt_read_int(n, "width", 640);
	int height = dt_read_int(n, "height", 480);
	char title[64];
	void * ctx;
	int bpp;

	sprintf(title, "Xboot Runtime Environment - V%s", xboot_version_string());
	ctx = sandbox_fb_sdl_open(title, width, height);
	if(!ctx)
		return NULL;

	bpp = sandbox_fb_sdl_get_bpp(ctx);
	if(bpp != 32)
	{
		sandbox_fb_sdl_close(ctx);
		return NULL;
	}

	pdat = malloc(sizeof(struct fb_sandbox_sdl_pdata_t));
	if(!pdat)
		return NULL;

	fb = malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		free(pdat);
		return NULL;
	}

	pdat->priv = ctx;
	pdat->width = sandbox_fb_sdl_get_width(pdat->priv);
	pdat->height = sandbox_fb_sdl_get_height(pdat->priv);
	pdat->pwidth = dt_read_int(n, "physical-width", sandbox_fb_get_pwidth(pdat->priv));
	pdat->pheight = dt_read_int(n, "physical-height", sandbox_fb_get_pheight(pdat->priv));
	pdat->bpp = bpp;

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = pdat->width;
	fb->height = pdat->height;
	fb->pwidth = pdat->pwidth;
	fb->pheight = pdat->pheight;
	fb->bpp = pdat->bpp;
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->priv = pdat;

	if(!register_framebuffer(&dev, fb))
	{
		sandbox_fb_sdl_close(pdat->priv);

		free_device_name(fb->name);
		free(fb->priv);
		free(fb);
		return NULL;
	}
	dev->driver = drv;

	return dev;
}

static void fb_sandbox_sdl_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_sandbox_sdl_pdata_t * pdat = (struct fb_sandbox_sdl_pdata_t *)fb->priv;

	if(fb && unregister_framebuffer(fb))
	{
		sandbox_fb_sdl_close(pdat->priv);

		free_device_name(fb->name);
		free(fb->priv);
		free(fb);
	}
}

static void fb_sandbox_sdl_suspend(struct device_t * dev)
{
}

static void fb_sandbox_sdl_resume(struct device_t * dev)
{
}

static struct driver_t fb_sandbox_sdl = {
	.name		= "fb-sandbox-sdl",
	.probe		= fb_sandbox_sdl_probe,
	.remove		= fb_sandbox_sdl_remove,
	.suspend	= fb_sandbox_sdl_suspend,
	.resume		= fb_sandbox_sdl_resume,
};

static __init void fb_sandbox_sdl_driver_init(void)
{
	register_driver(&fb_sandbox_sdl);
}

static __exit void fb_sandbox_sdl_driver_exit(void)
{
	unregister_driver(&fb_sandbox_sdl);
}

driver_initcall(fb_sandbox_sdl_driver_init);
driver_exitcall(fb_sandbox_sdl_driver_exit);
