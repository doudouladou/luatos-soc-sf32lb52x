-- 根据自己的服务器修改以下参数
local mqtt_host = "120.55.137.106"
local mqtt_port = 1884
local mqtt_isssl = false
local client_id = "abc"
local user_name = "user"
local password = "password"

local pub_topic = "/luatos/pub/" .. (mcu.unique_id():toHex())
local sub_topic = "/luatos/sub/" .. (mcu.unique_id():toHex())
local reconnectTimeId
local reconnectTime = 3000
local mqttc = nil

sys.taskInit(function()
    -- 等待联网
    local ret = sys.waitUntil("IP_READY")
    -- 下面的是mqtt的参数均可自行修改
    pub_topic = "/luatos/pub/" .. client_id
    sub_topic = "/luatos/sub/" .. client_id

    -- 打印一下上报(pub)和下发(sub)的topic名称
    -- 上报: 设备 ---> 服务器
    -- 下发: 设备 <--- 服务器
    -- 可使用mqtt.x等客户端进行调试
    log.info("mqtt", "pub", pub_topic)
    log.info("mqtt", "sub", sub_topic)

    -- 打印一下支持的加密套件, 通常来说, 固件已包含常见的99%的加密套件
    -- if crypto.cipher_suites then
    --     log.info("cipher", "suites", json.encode(crypto.cipher_suites()))
    -- end
    if mqtt == nil then
        while 1 do
            sys.wait(1000)
            log.info("bsp", "本bsp未适配mqtt库, 请查证")
        end
    end


    mqttc = mqtt.create(nil, mqtt_host, mqtt_port, mqtt_isssl)

    mqttc:auth(client_id, user_name, password)
    mqttc:on(function(mqtt_client, event, data, payload)
        -- 用户自定义代码
        log.info("mqtt", "event", event, mqtt_client, data, payload)
        if event == "conack" then
            if sys.timerIsActive(reconnectTimeId) then
                sys.timerStop(reconnectTimeId)
            end
            -- 联上了
            sys.publish("mqtt_conack")
            mqtt_client:subscribe(sub_topic) -- 单主题订阅
            -- mqtt_client:subscribe({[topic1]=1,[topic2]=1,[topic3]=1})--多主题订阅
        elseif event == "recv" then
            log.info("mqtt", "downlink", "topic", data, "payload", payload)
            sys.publish("mqtt_payload", data, payload)
        elseif event == "sent" then
            -- log.info("mqtt", "sent", "pkgid", data)
        elseif event == "disconnect" then
            -- 非自动重连时,按需重启mqttc
            if not reconnectTimeId or not sys.timerIsActive(reconnectTimeId) then
                reconnectTimeId = sys.timerStart(function()
                    mqtt_client:connect()
                end, reconnectTime)
            end
        end
    end)

    -- mqttc自动处理重连, 除非自行关闭
    mqttc:connect()
    sys.waitUntil("mqtt_conack")
    while true do
        -- 演示等待其他task发送过来的上报信息
        local ret, topic, data, qos = sys.waitUntil("mqtt_pub", 300000)
        if ret then
            -- 提供关闭本while循环的途径, 不需要可以注释掉
            if topic == "close" then
                break
            end
            mqttc:publish(topic, data, qos)
        end
    end
    mqttc:close()
    mqttc = nil
end)

-- 这里演示在另一个task里上报数据, 会定时上报数据,不需要就注释掉
sys.taskInit(function()
    sys.wait(3000)
    local data = "123,"
    local qos = 1 -- QOS0不带puback, QOS1是带puback的
    while true do
        sys.wait(3000)
        if mqttc and mqttc:ready() then
            local pkgid = mqttc:publish(pub_topic, data .. os.date(), qos)
        end
    end
end)

sys.taskInit(function()
    while true do
        sys.wait(3000)
        log.info("lua", rtos.meminfo())
        log.info("sys", rtos.meminfo("sys"))
    end
end)
