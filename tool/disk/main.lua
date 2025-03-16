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
local outPin = 26
-- PA3
local intPin = 3

-- GPIO初始化
local LED = gpio.setup(outPin, 0, gpio.PULLUP)
sys.taskInit(function()
    local count = 0
    while 1 do
        sys.wait(1000) -- 等待1s
        LED(count % 2)
        count = count + 1
        log.info("main", "sf32 for luatos", os.date("%Y-%m-%d %H:%M:%S", os.time()))
    end
end)


gpio.debounce(intPin, 100)
gpio.setup(intPin, function(val, io)
    log.info("GPIO3 int", gpio.get(intPin))
end, gpio.PULLDOWN, gpio.BOTH)


-- mcu.UART:uart 2：uart2, 20: PA20, 0:UART_RX, 0:none
-- mcu.UART:uart 2：uart2, 27: PA27, 1:UART_TX, 0:none
mcu.altfun(mcu.UART, 2, 20, 0, 0)
mcu.altfun(mcu.UART, 2, 27, 1, 0)
uart.setup(2, 115200)
uart.on(2, "receive", function(id, len)
    while true do
        local data = uart.read(id, 512)
        if data and #data > 0 then
            log.info("uart2", data)
        else
            break
        end
    end
end)
sys.taskInit(function()
    while true do
        sys.wait(1000)
        uart.write(2, "123\r\n")
    end
end)

-- API文档 https://wiki.luatos.com/api/gpio.html

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
