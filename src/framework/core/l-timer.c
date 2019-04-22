/*
 * framework/core/l-timer.c
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

#include <framework/core/l-timer.h>

static const char timer_lua[] = X(
local M = Class()

function M:init(delay, iteration, listener)
	self.__delay = delay or 1
	self.__iteration = iteration or 1
	self.__listener = listener
	self.__running = false
	self.__runtime = 0
	self.__runcount = 0
end

function M:start()
	self.__running = true
end

function M:pause()
	self.__running = false
end

function M:status()
	return self.__running
end

return M
);

int luaopen_timer(lua_State * L)
{
	if(luaL_loadbuffer(L, timer_lua, sizeof(timer_lua) - 1, "Timer.lua") == LUA_OK)
		lua_call(L, 0, 1);
	return 1;
}
