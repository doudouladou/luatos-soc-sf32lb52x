

#include "luat_base.h"
#define LUAT_LOG_TAG "main"
#include "luat_log.h"
#include "luat_mem.h"
#include "luat_fs.h"
#include "bget.h"
#include "luat_timer.h"
#include "luat_msgbus.h"
#include "dfs_file.h"
#include "drv_flash.h"
#include "ptab.h"

void luat_main_sifli(void) {
    luat_heap_init();
	luat_main();
}

void luat_nprint(char *s, size_t l) {
    rt_kprintf("%s\n", s);
}

void luat_log_log(int level, const char* tag, const char* _fmt, ...) {
        // if (luat_log_level_cur > level) return;
    char log_printf_buff[1024]  = {0};
    char *tmp = (char *)log_printf_buff;
    switch (level)
        {
        case LUAT_LOG_DEBUG:
            rt_kprintf("D/");
            break;
        case LUAT_LOG_INFO:
            rt_kprintf("I/");
            break;
        case LUAT_LOG_WARN:
            rt_kprintf("W/");
            break;
        case LUAT_LOG_ERROR:
            rt_kprintf("E/");
            break;
        default:
            rt_kprintf("D/");
            break;
        }
    rt_kprintf("%s", tag);
    rt_kprintf(" ");
    va_list args;
    va_start(args, _fmt);
    int len = vsnprintf_(tmp, 1024 - 2, _fmt, args);
    va_end(args);
    if (len > 0) {
        tmp[len] = '\n';
        rt_kprintf(tmp, len + 1);
    }
}
// 临时适配

const char* luat_os_bsp() {
	return "SiFli52X";
}

void luat_os_reboot(int code) {
    // TODO
}

void luat_ota_reboot() {
	// TODO
}

void luat_os_standy(int timeout) {
  (void)timeout;
  return; // nop
}

static const luaL_Reg loadedlibs[] = {
  {"_G", luaopen_base}, // _G
  {LUA_LOADLIBNAME, luaopen_package}, // require
  {LUA_COLIBNAME, luaopen_coroutine}, // coroutine协程库
  {LUA_TABLIBNAME, luaopen_table},    // table库,操作table类型的数据结构
  {LUA_IOLIBNAME, luaopen_io},        // io库,操作文件
  {LUA_OSLIBNAME, luaopen_os},        // os库,已精简
  {LUA_STRLIBNAME, luaopen_string},   // string库,字符串操作
  {LUA_MATHLIBNAME, luaopen_math},    // math 数值计算
//   {LUA_UTF8LIBNAME, luaopen_utf8},
  {LUA_DBLIBNAME, luaopen_debug},     // debug库,已精简
#if defined(LUA_COMPAT_BITLIB)
  {LUA_BITLIBNAME, luaopen_bit32},    // 不太可能启用
#endif
  {"rtos", luaopen_rtos},             // rtos底层库, 核心功能是队列和定时器
  {"log", luaopen_log},               // 日志库
  // {"timer", luaopen_timer},           // 延时库
//   {"pack", luaopen_pack},             // pack.pack/pack.unpack
//   {"json", luaopen_cjson},             // json
//   {"zbuff", luaopen_zbuff},            // 
//   {"crypto", luaopen_crypto},
#ifdef LUAT_USE_GPIO
    {"gpio",    luaopen_gpio},              // GPIO脚的操作
#endif
  {NULL, NULL}
};


void luat_openlibs(lua_State *L) {
    // 初始化队列服务
    luat_msgbus_init();
    //print_list_mem("done>luat_msgbus_init");
    // 加载系统库
    const luaL_Reg *lib;
    /* "require" functions from 'loadedlibs' and set results to global table */
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
        //extern void print_list_mem(const char* name);
        //print_list_mem(lib->name);
    }
}


uint32_t luat_rtos_get_ipsr(void){
    return 0;
}
