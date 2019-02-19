/*
 * driver/fb-f1c500s.c
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
#include <dma/dma.h>
#include <clk/clk.h>
#include <reset/reset.h>
#include <gpio/gpio.h>
#include <led/led.h>
#include <interrupt/interrupt.h>
#include <framebuffer/framebuffer.h>
#include <f1c500s-gpio.h>
#include <f1c500s/reg-tcon.h>
#include <f1c500s/reg-debe.h>
#include <f1c500s/reg-defe.h>

#define F1C500S_GPIO_BASE	(0x01c20800)
#define F1C500S_GPIOD_DATA	((3 * 0x24) + 0x10)

struct fb_f1c500s_pdata_t
{
	virtual_addr_t virtdefe;
	virtual_addr_t virtdebe;
	virtual_addr_t virttcon;
	virtual_addr_t virtgpio;

	char * clkdefe;
	char * clkdebe;
	char * clktcon;
	int rstdefe;
	int rstdebe;
	int rsttcon;
	int width;
	int height;
	int pwidth;
	int pheight;
	int bits_per_pixel;
	int bytes_per_pixel;
	int index;
	void * vram[2];

	struct {
		int pixel_clock_hz;
		int h_front_porch;
		int h_back_porch;
		int h_sync_len;
		int v_front_porch;
		int v_back_porch;
		int v_sync_len;
		int h_sync_active;
		int v_sync_active;
		int den_active;
		int clk_active;
	} timing;

	struct led_t * backlight;
	int brightness;
};

static inline void r61520_write(struct fb_f1c500s_pdata_t * pdat, u32_t isdat, u32_t val)
{
	u32_t tmp;

	tmp = (val & 0x00ff) << 1;
	tmp |= (val & 0xff00) << 2;
	tmp |= isdat ? 0x80000 : 0;
	tmp |= 0x100000;
	write32(pdat->virtgpio + F1C500S_GPIOD_DATA, tmp);
	tmp |= 0x40000;
	write32(pdat->virtgpio + F1C500S_GPIOD_DATA, tmp);
}

static void r61520_write_cmd(struct fb_f1c500s_pdata_t * pdat, u32_t val)
{
	r61520_write(pdat, 0, val);
}

static void r61520_write_dat(struct fb_f1c500s_pdata_t * pdat, u32_t val)
{
	r61520_write(pdat, 1, val);
}

static inline void r61520_init(struct fb_f1c500s_pdata_t * pdat)
{
	r61520_write_cmd(pdat, 0xb0);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xb1);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xb3);
	r61520_write_dat(pdat, 0x02);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xb4);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xc0);
	r61520_write_dat(pdat, 0x07);
	r61520_write_dat(pdat, 0x4f);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x01);
	r61520_write_dat(pdat, 0x33);

	r61520_write_cmd(pdat, 0xc1);
	r61520_write_dat(pdat, 0x01);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x1a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x08);

	r61520_write_cmd(pdat, 0xc3);
	r61520_write_dat(pdat, 0x01);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x1a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x08);

	r61520_write_cmd(pdat, 0xc4);
	r61520_write_dat(pdat, 0x11);
	r61520_write_dat(pdat, 0x01);
	r61520_write_dat(pdat, 0x43);
	r61520_write_dat(pdat, 0x01);

	r61520_write_cmd(pdat, 0xc8);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x8a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x09);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x23);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x60);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xc9);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x8a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x09);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x23);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x09);
	r61520_write_dat(pdat, 0x88);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x23);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xca);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x8a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x09);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x23);
	r61520_write_dat(pdat, 0x10);
	r61520_write_dat(pdat, 0x05);
	r61520_write_dat(pdat, 0x09);
	r61520_write_dat(pdat, 0x88);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x08);
	r61520_write_dat(pdat, 0x0a);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x23);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0xd0);
	r61520_write_dat(pdat, 0x07);
	r61520_write_dat(pdat, 0xc6);
	r61520_write_dat(pdat, 0xdc);

	r61520_write_cmd(pdat, 0xd1);
	r61520_write_dat(pdat, 0x54);
	r61520_write_dat(pdat, 0x0d);
	r61520_write_dat(pdat, 0x02);

	r61520_write_cmd(pdat, 0xd2);
	r61520_write_dat(pdat, 0x63);
	r61520_write_dat(pdat, 0x24);

	r61520_write_cmd(pdat, 0xd4);
	r61520_write_dat(pdat, 0x63);
	r61520_write_dat(pdat, 0x24);

	r61520_write_cmd(pdat, 0xd8);
	r61520_write_dat(pdat, 0x07);
	r61520_write_dat(pdat, 0x07);

	r61520_write_cmd(pdat, 0xe0);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0x13);

	r61520_write_cmd(pdat, 0x20);

	r61520_write_cmd(pdat, 0x35);
	r61520_write_dat(pdat, 0x00);

	r61520_write_cmd(pdat, 0x44);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x30);

	r61520_write_cmd(pdat, 0x36);
	r61520_write_dat(pdat, 0xe0);

	r61520_write_cmd(pdat, 0x3a);
	r61520_write_dat(pdat, 0x55);

	r61520_write_cmd(pdat, 0x2a);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x01);
	r61520_write_dat(pdat, 0x3f);

	r61520_write_cmd(pdat, 0x2b);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0x00);
	r61520_write_dat(pdat, 0xef);

	r61520_write_cmd(pdat, 0x11);
	r61520_write_cmd(pdat, 0x29);
	r61520_write_cmd(pdat, 0x2c);
}

static inline void f1c500s_debe_set_mode(struct fb_f1c500s_pdata_t * pdat)
{
	struct f1c500s_debe_reg_t * debe = (struct f1c500s_debe_reg_t *)(pdat->virtdebe);
	u32_t val;

	val = read32((virtual_addr_t)&debe->mode);
	val |= (1 << 0);
	write32((virtual_addr_t)&debe->mode, val);

	write32((virtual_addr_t)&debe->disp_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
	write32((virtual_addr_t)&debe->layer0_size, (((pdat->height) - 1) << 16) | (((pdat->width) - 1) << 0));
	write32((virtual_addr_t)&debe->layer0_stride, ((pdat->width) << 5));
	write32((virtual_addr_t)&debe->layer0_addr_low32b, (u32_t)(pdat->vram[pdat->index]) << 3);
	write32((virtual_addr_t)&debe->layer0_addr_high4b, (u32_t)(pdat->vram[pdat->index]) >> 29);
	write32((virtual_addr_t)&debe->layer0_attr1_ctrl, 0x09 << 8);

	val = read32((virtual_addr_t)&debe->mode);
	val |= (1 << 8);
	write32((virtual_addr_t)&debe->mode, val);

	val = read32((virtual_addr_t)&debe->reg_ctrl);
	val |= (1 << 0);
	write32((virtual_addr_t)&debe->reg_ctrl, val);

	val = read32((virtual_addr_t)&debe->mode);
	val |= (1 << 1);
	write32((virtual_addr_t)&debe->mode, val);
}

static inline void f1c500s_debe_set_address(struct fb_f1c500s_pdata_t * pdat, void * vram)
{
	struct f1c500s_debe_reg_t * debe = (struct f1c500s_debe_reg_t *)(pdat->virtdebe);

	write32((virtual_addr_t)&debe->layer0_addr_low32b, (u32_t)vram << 3);
	write32((virtual_addr_t)&debe->layer0_addr_high4b, (u32_t)vram >> 29);
}

static inline void f1c500s_tcon_enable(struct fb_f1c500s_pdata_t * pdat)
{
	struct f1c500s_tcon_reg_t * tcon = (struct f1c500s_tcon_reg_t *)pdat->virttcon;
	u32_t val;

	val = read32((virtual_addr_t)&tcon->ctrl);
	val |= (1 << 31);
	write32((virtual_addr_t)&tcon->ctrl, val);

	val = read32((virtual_addr_t)&tcon->tcon0_cpu_intf);
	val |= (1 << 28);
	write32((virtual_addr_t)&tcon->tcon0_cpu_intf, val);
}

static inline void f1c500s_tcon_disable(struct fb_f1c500s_pdata_t * pdat)
{
	struct f1c500s_tcon_reg_t * tcon = (struct f1c500s_tcon_reg_t *)pdat->virttcon;
	u32_t val;

	write32((virtual_addr_t)&tcon->ctrl, 0);
	write32((virtual_addr_t)&tcon->int0, 0);

	val = read32((virtual_addr_t)&tcon->tcon0_dclk);
	val &= ~(0xf << 28);
	write32((virtual_addr_t)&tcon->tcon0_dclk, val);

	write32((virtual_addr_t)&tcon->tcon0_io_tristate, 0xffffffff);
	write32((virtual_addr_t)&tcon->tcon1_io_tristate, 0xffffffff);
}

static inline void f1c500s_tcon_set_mode(struct fb_f1c500s_pdata_t * pdat)
{
	struct f1c500s_tcon_reg_t * tcon = (struct f1c500s_tcon_reg_t *)pdat->virttcon;
	int bp, total;
	u32_t val;

	val = read32((virtual_addr_t)&tcon->ctrl);
	val &= ~(0x1 << 0);
	write32((virtual_addr_t)&tcon->ctrl, val);

	val = (pdat->timing.v_front_porch + pdat->timing.v_back_porch + pdat->timing.v_sync_len);
	write32((virtual_addr_t)&tcon->tcon0_ctrl, (1 << 31) | ((val & 0x1f) << 4) | (0x01 << 24));
	val = clk_get_rate(pdat->clktcon) / pdat->timing.pixel_clock_hz;
	write32((virtual_addr_t)&tcon->tcon0_dclk, (0xf << 28) | (val << 0));
	write32((virtual_addr_t)&tcon->tcon0_timing_active, ((pdat->width - 1) << 16) | ((pdat->height - 1) << 0));

	bp = pdat->timing.h_sync_len + pdat->timing.h_back_porch;
	total = pdat->width * 3 + pdat->timing.h_front_porch + bp;
	write32((virtual_addr_t)&tcon->tcon0_timing_h, ((total - 1) << 16) | ((bp - 1) << 0));
	bp = pdat->timing.v_sync_len + pdat->timing.v_back_porch;
	total = pdat->height + pdat->timing.v_front_porch + bp;
	write32((virtual_addr_t)&tcon->tcon0_timing_v, ((total * 2) << 16) | ((bp - 1) << 0));
	write32((virtual_addr_t)&tcon->tcon0_timing_sync, ((pdat->timing.h_sync_len - 1) << 16) | ((pdat->timing.v_sync_len - 1) << 0));

	write32((virtual_addr_t)&tcon->tcon0_hv_intf, 0);
	write32((virtual_addr_t)&tcon->tcon0_cpu_intf, (0x4 << 29) | (1 << 26));

	write32((virtual_addr_t)&tcon->tcon0_io_polarity, (1 << 28));
	write32((virtual_addr_t)&tcon->tcon0_io_tristate, 0);
}

static inline void fb_f1c500s_cfg_gpios(int base, int n, int cfg, enum gpio_pull_t pull, enum gpio_drv_t drv)
{
	for(; n > 0; n--, base++)
	{
		gpio_set_cfg(base, cfg);
		gpio_set_pull(base, pull);
		gpio_set_drv(base, drv);
	}
}

static inline void fb_f1c500s_init(struct fb_f1c500s_pdata_t * pdat)
{
	fb_f1c500s_cfg_gpios(F1C500S_GPIOD1, 8, 0x1, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	fb_f1c500s_cfg_gpios(F1C500S_GPIOD10, 12, 0x1, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	r61520_init(pdat);

	fb_f1c500s_cfg_gpios(F1C500S_GPIOD1, 8, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	fb_f1c500s_cfg_gpios(F1C500S_GPIOD10, 12, 0x2, GPIO_PULL_NONE, GPIO_DRV_STRONG);
	f1c500s_tcon_disable(pdat);
	f1c500s_debe_set_mode(pdat);
	f1c500s_tcon_set_mode(pdat);
	f1c500s_tcon_enable(pdat);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;
	return led_get_brightness(pdat->backlight);
}

static struct render_t * fb_create(struct framebuffer_t * fb)
{
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;
	struct render_t * render;
	void * pixels;
	size_t pixlen;

	pixlen = pdat->width * pdat->height * pdat->bytes_per_pixel;
	pixels = memalign(4, pixlen);
	if(!pixels)
		return NULL;

	render = malloc(sizeof(struct render_t));
	if(!render)
	{
		free(pixels);
		return NULL;
	}

	render->width = pdat->width;
	render->height = pdat->height;
	render->pitch = (pdat->width * pdat->bytes_per_pixel + 0x3) & ~0x3;
	render->format = PIXEL_FORMAT_ARGB32;
	render->pixels = pixels;
	render->pixlen = pixlen;
	render->priv = NULL;

	return render;
}

static void fb_destroy(struct framebuffer_t * fb, struct render_t * render)
{
	if(render)
	{
		free(render->pixels);
		free(render);
	}
}

static void fb_present(struct framebuffer_t * fb, struct render_t * render, struct dirty_rect_t * rect, int nrect)
{
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;

	if(render && render->pixels)
	{
		pdat->index = (pdat->index + 1) & 0x1;
		memcpy(pdat->vram[pdat->index], render->pixels, render->pixlen);
		dma_cache_sync(pdat->vram[pdat->index], render->pixlen, DMA_TO_DEVICE);
		f1c500s_debe_set_address(pdat, pdat->vram[pdat->index]);
	}
}

static struct device_t * fb_f1c500s_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_f1c500s_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	char * clkdefe = dt_read_string(n, "clock-name-defe", NULL);
	char * clkdebe = dt_read_string(n, "clock-name-debe", NULL);
	char * clktcon = dt_read_string(n, "clock-name-tcon", NULL);
	int i;

	if(!search_clk(clkdefe) || !search_clk(clkdebe) || !search_clk(clktcon))
		return NULL;

	pdat = malloc(sizeof(struct fb_f1c500s_pdata_t));
	if(!pdat)
		return NULL;

	fb = malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		free(pdat);
		return NULL;
	}

	pdat->virtdefe = phys_to_virt(F1C500S_DEFE_BASE);
	pdat->virtdebe = phys_to_virt(F1C500S_DEBE_BASE);
	pdat->virttcon = phys_to_virt(F1C500S_TCON_BASE);
	pdat->virtgpio = phys_to_virt(F1C500S_GPIO_BASE);
	pdat->clkdefe = strdup(clkdefe);
	pdat->clkdebe = strdup(clkdebe);
	pdat->clktcon = strdup(clktcon);
	pdat->rstdefe = dt_read_int(n, "reset-defe", -1);
	pdat->rstdebe = dt_read_int(n, "reset-debe", -1);
	pdat->rsttcon = dt_read_int(n, "reset-tcon", -1);
	pdat->width = dt_read_int(n, "width", 320);
	pdat->height = dt_read_int(n, "height", 240);
	pdat->pwidth = dt_read_int(n, "physical-width", 216);
	pdat->pheight = dt_read_int(n, "physical-height", 135);
	pdat->bits_per_pixel = dt_read_int(n, "bits-per-pixel", 18);
	pdat->bytes_per_pixel = dt_read_int(n, "bytes-per-pixel", 4);
	pdat->index = 0;
	pdat->vram[0] = dma_alloc_noncoherent(pdat->width * pdat->height * pdat->bytes_per_pixel);
	pdat->vram[1] = dma_alloc_noncoherent(pdat->width * pdat->height * pdat->bytes_per_pixel);

	pdat->timing.pixel_clock_hz = dt_read_long(n, "clock-frequency", 8000000);
	pdat->timing.h_front_porch = dt_read_int(n, "hfront-porch", 40);
	pdat->timing.h_back_porch = dt_read_int(n, "hback-porch", 87);
	pdat->timing.h_sync_len = dt_read_int(n, "hsync-len", 1);
	pdat->timing.v_front_porch = dt_read_int(n, "vfront-porch", 13);
	pdat->timing.v_back_porch = dt_read_int(n, "vback-porch", 31);
	pdat->timing.v_sync_len = dt_read_int(n, "vsync-len", 1);
	pdat->timing.h_sync_active = dt_read_bool(n, "hsync-active", 0);
	pdat->timing.v_sync_active = dt_read_bool(n, "vsync-active", 0);
	pdat->timing.den_active = dt_read_bool(n, "den-active", 0);
	pdat->timing.clk_active = dt_read_bool(n, "clk-active", 0);
	pdat->backlight = search_led(dt_read_string(n, "backlight", NULL));

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = pdat->width;
	fb->height = pdat->height;
	fb->pwidth = pdat->pwidth;
	fb->pheight = pdat->pheight;
	fb->bpp = pdat->bytes_per_pixel * 8;
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->priv = pdat;

	clk_enable(pdat->clkdefe);
	clk_enable(pdat->clkdebe);
	clk_enable(pdat->clktcon);
	if(pdat->rstdefe >= 0)
		reset_deassert(pdat->rstdefe);
	if(pdat->rstdebe >= 0)
		reset_deassert(pdat->rstdebe);
	if(pdat->rsttcon >= 0)
		reset_deassert(pdat->rsttcon);
	for(i = 0x0800; i < 0x1000; i += 4)
		write32(pdat->virtdebe + i, 0);
	fb_f1c500s_init(pdat);

	if(!register_framebuffer(&dev, fb))
	{
		clk_disable(pdat->clkdefe);
		clk_disable(pdat->clkdebe);
		clk_disable(pdat->clktcon);
		free(pdat->clkdefe);
		free(pdat->clkdebe);
		free(pdat->clktcon);
		dma_free_noncoherent(pdat->vram[0]);
		dma_free_noncoherent(pdat->vram[1]);

		free_device_name(fb->name);
		free(fb->priv);
		free(fb);
		return NULL;
	}
	dev->driver = drv;

	return dev;
}

static void fb_f1c500s_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;

	if(fb && unregister_framebuffer(fb))
	{
		clk_disable(pdat->clkdefe);
		clk_disable(pdat->clkdebe);
		clk_disable(pdat->clktcon);
		free(pdat->clkdefe);
		free(pdat->clkdebe);
		free(pdat->clktcon);
		dma_free_noncoherent(pdat->vram[0]);
		dma_free_noncoherent(pdat->vram[1]);

		free_device_name(fb->name);
		free(fb->priv);
		free(fb);
	}
}

static void fb_f1c500s_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_f1c500s_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_f1c500s_pdata_t * pdat = (struct fb_f1c500s_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_f1c500s = {
	.name		= "fb-f1c500s",
	.probe		= fb_f1c500s_probe,
	.remove		= fb_f1c500s_remove,
	.suspend	= fb_f1c500s_suspend,
	.resume		= fb_f1c500s_resume,
};

static __init void fb_f1c500s_driver_init(void)
{
	register_driver(&fb_f1c500s);
}

static __exit void fb_f1c500s_driver_exit(void)
{
	unregister_driver(&fb_f1c500s);
}

driver_initcall(fb_f1c500s_driver_init);
driver_exitcall(fb_f1c500s_driver_exit);
