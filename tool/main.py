import os, sys, logging, subprocess, shutil, argparse
import traceback



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", '--port', help="none")
    args = parser.parse_args()
    if args.port:
        port = args.port
    else:
        print("请输入端口")
        sys.exit(0)
    project_root = os.path.abspath(".")
    print(project_root)
    if not os.path.exists(os.path.join(project_root, "disk","main.lua")):
        print("main.lua不存在")
        sys.exit(0)
    if not os.path.exists(os.path.join(project_root, "sftool.exe")):
        print("sftool.exe不存在")
        sys.exit(0)


    subprocess.check_call(["luatos-lua.exe", "main.lua"], cwd = project_root)

    subprocess.check_call(["sftool.exe", "-c", "SF32LB52", "-p", "%s" % port, "write_flash", "bootloader.bin@0x12010000", "main.bin@0x12020000", "ftab.bin@0x12000000", "script.bin@0x124E0000"], cwd = project_root)
    
if __name__ == '__main__':
    main()