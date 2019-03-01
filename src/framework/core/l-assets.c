/*
 * framework/core/l-assets.c
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

#include <framework/core/l-assets.h>

static const char assets_lua[] = X(
local Xfs = Xfs
local Font = Font
local Image = Image
local Ninepatch = Ninepatch
local DisplayImage = DisplayImage
local DisplayNinepatch = DisplayNinepatch

local M = Class()

function M:init()
	self.images = {}
	self.fonts = {}
	self.themes = {}
end

function M:loadImage(name)
	if type(name) == "string" then
		if not self.images[name] and Xfs.isfile(name) then
			self.images[name] = Image.new(name)
		end
		return self.images[name]
	end
	return nil
end

function M:loadFont(family, size)
	if type(family) == "string" then
		if not self.fonts[family] then
			self.fonts[family] = {}
		end
		local size = size or 1
		if not self.fonts[family][size] and Xfs.isfile(family) then
			self.fonts[family][size] = Font.new(family, size)
		end
		return self.fonts[family][size]
	end
	return nil
end

function M:loadTheme(name)
	local default = "assets/themes/default"
	local name = type(name) == "string" and name or default
	if not self.themes[name] then
		if Xfs.isdir(name) then
			self.themes[name] = require(name)
		end
		if not self.themes[name] then
			name = "assets/themes/" .. name
			if Xfs.isdir(name) then
				self.themes[name] = require(name)
			end
			if not self.themes[name] then
				if not self.themes[default] then
					if Xfs.isdir(default) then
						self.themes[default] = require(default)
					end
				end
				return self.themes[default]
			end
		end
	end
	return self.themes[name]
end

function M:loadDisplay(name)
	if type(name) == "string" and Xfs.isfile(name) then
		if string.lower(string.sub(name, -6)) == ".9.png" then
			return DisplayNinepatch.new(Ninepatch.new(name))
		elseif string.lower(string.sub(name, -4)) == ".png" then
			return DisplayImage.new(self:loadImage(name))
		end
	else
		return name
	end
end

function M:clear()
	self.images = {}
	self.fonts = {}
	self.themes = {}
end

return M
);

int luaopen_assets(lua_State * L)
{
	if(luaL_loadbuffer(L, assets_lua, sizeof(assets_lua) - 1, "Assets.lua") == LUA_OK)
		lua_call(L, 0, 1);
	return 1;
}
