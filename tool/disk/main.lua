-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "gpiodemo"
VERSION = "1.0.0"

log.info("main", PROJECT, VERSION)

-- sys库是标配
_G.sys = require("sys")

if wdt then
    -- 添加硬狗防止程序卡死，在支持的设备上启用这个功能
    wdt.init(9000) -- 初始化watchdog设置为9s
    sys.timerLoopStart(wdt.feed, 3000) -- 3s喂一次狗
end

-- sf32lb525开发板上控制蓝灯的pin
local pin = 26

-- GPIO初始化
local LEDA = gpio.setup(pin, 0, gpio.PULLUP)

sys.taskInit(function()
    local count = 0
    while 1 do
        sys.wait(1000) -- 等待1s
        LEDA(count % 2)
        count = count + 1
        log.info("main", "sf32 for luatos", os.date("%Y-%m-%d %H:%M:%S", os.time()))
    end
end)

-- API文档 https://wiki.luatos.com/api/gpio.html

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
