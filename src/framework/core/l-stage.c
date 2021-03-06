/*
 * framework/core/l-stage.c
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

#include <framework/core/l-stage.h>

static const char stage_lua[] = X(
local M = Class(DisplayObject)

function M:init()
	self.__exiting = false
	self.__timerlist = {}
	self.__display = Display.new()
	self.super:init(self.__display:getSize())
end

function M:exit()
	self.__exiting = true
	return self
end

function M:hasTimer(timer)
	local tl = self.__timerlist

	if not tl or #tl == 0 then
		return false
	end

	for i, v in ipairs(tl) do
		if v == timer then
			return true
		end
	end

	return false
end

function M:addTimer(timer)
	if self:hasTimer(timer) then
		return false
	end

	timer:start()
	table.insert(self.__timerlist, timer)
	return true
end

function M:removeTimer(timer)
	local tl = self.__timerlist

	if not tl or #tl == 0 then
		return false
	end

	for i, v in ipairs(tl) do
		if v == timer then
			v:pause()
			table.remove(tl, i)
			return true
		end
	end

	return false
end

function M:schedTimer(dt)
	for i, v in ipairs(self.__timerlist) do
		if v.__running then
			v.__runtime = v.__runtime + dt

			if v.__runtime >= v.__delay then
				v.__runcount = v.__runcount + 1
				v.__listener(v)
				v.__runtime = 0

				if v.__iteration ~= 0 and v.__runcount >= v.__iteration then
					self:removeTimer(v)
				end
			end
		end
	end
end

function M:getSize()
	return self.__display:getSize()
end

function M:getPhysicalSize()
	return self.__display:getPhysicalSize()
end

function M:getBpp()
	return self.__display:getBpp()
end

function M:setBacklight(brightness)
	return self.__display:setBacklight(brightness)
end

function M:getBacklight()
	return self.__display:getBacklight()
end

function M:showobj(value)
	self.__display:showobj(value)
	return self
end

function M:showfps(value)
	self.__display:showfps(value)
	return self
end

function M:snapshot()
	return self.__display:snapshot()
end

function M:loop()
	local Event = Event
	local __display = self.__display
	local stopwatch = Stopwatch.new()

	self:addTimer(Timer.new(1 / 60, 0, function(t)
		self:dispatch(Event.new("enter-frame"))
		self:render(__display)
		collectgarbage("step")
	end))

	self:addEventListener("key-down", function(d, e)
		if e.key == 10 then self:exit() end
	end)

	while not self.__exiting do
		local e = Event.pump()
		if e ~= nil then
			self:dispatch(e)
		end

		local elapsed = stopwatch:elapsed()
		if elapsed > 0 then
			stopwatch:reset()
			self:schedTimer(elapsed)
		end
	end
end

return M
);

int luaopen_stage(lua_State * L)
{
	if(luaL_loadbuffer(L, stage_lua, sizeof(stage_lua) - 1, "Stage.lua") == LUA_OK)
		lua_call(L, 0, 1);
	return 1;
}
