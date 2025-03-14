# luatos-soc-sf32lb52x

LuatOS 运行在 sf32lb52x 上

将编译出的bootloader.bin、app.bin和fatb.bin放在tool工具目录下，app.bin需要手动更名为main.bin，用户脚本放在工具目录的disk目录下

运行main.py即可自动打包lua文件并下载

python3 main.py -p COM14