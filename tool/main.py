import os, sys, logging, subprocess, shutil, argparse
import traceback

project_root = None
build_root = None



def remove():
    if os.path.exists(project_root + "/ftab.bin"):
        os.remove(project_root + "/ftab.bin")
    if os.path.exists(project_root + "/main.bin"):
        os.remove(project_root + "/main.bin")
    if os.path.exists(project_root + "/bootloader.bin"):
        os.remove(project_root + "/bootloader.bin")
    shutil.copyfile(build_root + "/main.bin", project_root + "/main.bin")
    shutil.copyfile(build_root + "/ftab/ftab.bin", project_root + "/ftab.bin")
    shutil.copyfile(build_root + "/bootloader/bootloader.bin", project_root + "/bootloader.bin")

def file_exists():
    global project_root
    file_root = os.path.abspath('../')
    if os.path.exists(project_root + "/ftab.bin"):
        print('/ftab File exists')
    else:
        print('/ftab File not exists')
        return False

    if os.path.exists(project_root + "/main.bin"):
        print('/main File exists')
    else:
        print('/main File not exists')
        return False

    if os.path.exists(project_root + "/bootloader.bin"):
        print('/bootloader File exists')
    else:
        print('/bootloader File not exists')
        return False
    return True


def build_exists():
    global build_root
    file_root = os.path.abspath('../')
    if os.path.exists(build_root + "/main.bin"):
        print('/main File exists')
    else:
        return False

    if os.path.exists(build_root + "/ftab/ftab.bin"):
        print('/ftab File exists')
    else:
        return False

    if os.path.exists(build_root + "/bootloader/bootloader.bin"):
        print('/bootloader File exists')
    else:
        return False
    return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", '--port', help="none")
    args = parser.parse_args()
    if args.port:
        port = args.port
    else:
        print("请输入端口")
        sys.exit(0)

    global project_root
    project_root = os.path.abspath(".")

    global build_root
    build_root = os.path.join(os.path.abspath("../") , "rtt","project", "build_em-lb525_hcpu")

    print(build_root)
    
    if build_exists() == True:
        remove()

    if not file_exists():
        print("待下载的文件不存在")
        sys.exit(0)

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