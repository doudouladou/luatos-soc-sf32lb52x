# luatos-soc-sf32lb52x

LuatOS 运行在 sf32lb52x 上


clone LuatOS主库放在和luatos-soc-sf32lb52x同级目录下

LuatOS仓库地址：

[LuatOS: Powerful embedded Lua Engine for loT devices, withmany components and low memory requirements (16KRAM, 128K Flash)](https://gitee.com/openLuat/LuatOS)

本仓库移植参考如下地址：

[RT-Thread-Mirror/luatos-soc-rtt](https://gitee.com/RT-Thread-Mirror/luatos-soc-rtt)


将编译出的app.bin手动更名为main.bin，用户脚本放在工具目录的disk目录下

运行main.py即可自动打包lua文件并下载

python3 main.py -p COM14
