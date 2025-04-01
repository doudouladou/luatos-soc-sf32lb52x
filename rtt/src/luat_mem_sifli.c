
#include "luat_base.h"
#define LUAT_LOG_TAG "main"
#include "luat_log.h"
#include "luat_mem.h"
#include "luat_fs.h"
#include "bget.h"
#include "luat_timer.h"
#include "luat_msgbus.h"
#include "rtthread.h"




#define VM_SIZE (100 * 1024)
static uint32_t *vm_heap;
void luat_heap_init(void){
    vm_heap = rt_malloc(VM_SIZE);
	bpool(vm_heap, VM_SIZE);
}

void* luat_heap_malloc(size_t len) {
    void* ptr = rt_malloc(len);
    if (ptr) {
        memset(ptr, 0, len);
    }
    return ptr;
}

void luat_heap_free(void* ptr) {
    rt_free(ptr);
}

void* luat_heap_realloc(void* ptr, size_t len) {
    return rt_realloc(ptr, len);
}

void* luat_heap_calloc(size_t count, size_t _size) {
    return luat_heap_malloc(count * _size);
}

void* luat_heap_zalloc(size_t _size) {
    return luat_heap_malloc(_size);
}

//------------------------------------------------
//------------------------------------------------
// ---------- 管理 LuaVM所使用的内存----------------
void* luat_heap_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (0) {
        LLOGD("realloc %p from %d to %d", ptr, osize, nsize);
        if (ptr) {
            if (nsize) {
                // 缩放内存块
                LLOGD("realloc %p from %d to %d", ptr, osize, nsize);
            }
            else {
                // 释放内存块
                LLOGD("free %p ", ptr);
                brel(ptr);
                return NULL;
            }
        }
        else {
            // 申请内存块
            ptr = bget(nsize);
            LLOGD("malloc %p type=%d size=%d", ptr, osize, nsize);
            return ptr;
        }
    }
    
    if (ptr == NULL && nsize == 0)
        return NULL;
    if (nsize)
    {
    	void* ptmp = bgetr(ptr, nsize);
    	if(ptmp == NULL && osize >= nsize)
    	{
    		return ptr;
    	}
        return ptmp;
    }
    brel(ptr);
    return NULL;
}

void luat_meminfo_luavm(size_t *total, size_t *used, size_t *max_used) {
    long curalloc, totfree, maxfree;
    unsigned long nget, nrel;
    bstats(&curalloc, &totfree, &maxfree, &nget, &nrel);
    *used = curalloc;
    *max_used = bstatsmaxget();
    *total = curalloc + totfree;
}
//-----------------------------------------------------------------------------


void luat_meminfo_sys(size_t* total, size_t* used, size_t* max_used) {
    rt_memory_info(total, used, max_used);
}