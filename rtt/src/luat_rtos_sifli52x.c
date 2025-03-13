#include "luat_base.h"
#include "rtthread.h"
#include "luat_timer.h"
#include "luat_rtos.h"

typedef struct
{
	void *timer;
	luat_rtos_timer_callback_t call_back;
	void *user_param;
	uint8_t is_repeat;
}luat_rtos_user_timer_t;

uint32_t luat_rtos_entry_critical(void)
{
	// luat_os_entry_cri();
    return 0;
}

/**
 * @brief 退出临界保护
 *
 * @param critical 进入临界保护时返回的参数
 */
void luat_rtos_exit_critical(uint32_t critical)
{
	// luat_os_exit_cri();
}

static void s_timer_callback(void* params)
{
	luat_rtos_user_timer_t *timer = (luat_rtos_user_timer_t *)params;
    // rt_kprintf("s_timer_callback 1:%d 2:%d 3:%d\n", timer, timer->is_repeat, timer->call_back);
	if (!timer)
		return;
	if (!timer->is_repeat)
	{
		rt_timer_stop(timer->timer);
	}
	if (timer->call_back)
	{
		timer->call_back(timer->user_param);
	}
}



void *luat_create_rtos_timer(void *cb, void *param, void *task_handle)
{
	luat_rtos_user_timer_t *timer = rt_malloc(sizeof(luat_rtos_user_timer_t));
	if (timer)
	{
        timer->timer = rt_timer_create(NULL, s_timer_callback, timer, 0,  RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
        if (!timer->timer)
		{
			rt_free(timer);
			return NULL;
		}
		timer->call_back = cb;
		timer->user_param = param;
		timer->is_repeat = 0;
	}
	return timer;
}

int luat_rtos_timer_start(luat_rtos_timer_t timer_handle, uint32_t timeout, uint8_t repeat, luat_rtos_timer_callback_t callback_fun, void *user_param)
{
    if (!timer_handle) return -1;
	luat_rtos_user_timer_t *htimer = (luat_rtos_user_timer_t *)timer_handle;

    rt_timer_stop(htimer->timer);
    htimer->is_repeat = repeat;
    htimer->call_back = callback_fun;
    htimer->user_param = user_param;

    rt_timer_control(htimer->timer, RT_TIMER_CTRL_SET_TIME, &timeout);
    rt_timer_start(htimer->timer);
    return 0;
}
int luat_start_rtos_timer(void *timer, uint32_t ms, uint8_t is_repeat)
{
	if (!timer) return -1;
	luat_rtos_user_timer_t *htimer = (luat_rtos_user_timer_t *)timer;
	rt_timer_stop(htimer->timer);
	rt_timer_control(htimer->timer, RT_TIMER_CTRL_SET_TIME, &ms);
	rt_timer_start(htimer->timer);
	return 0;
}
// luat_start_rtos_timer


void luat_stop_rtos_timer(void *timer)
{
    luat_rtos_user_timer_t *htimer = (luat_rtos_user_timer_t *)timer;
    rt_timer_stop(htimer->timer);
}

void luat_release_rtos_timer(void *timer)
{
	luat_rtos_user_timer_t *htimer = (luat_rtos_user_timer_t *)timer;
	rt_timer_delete(htimer->timer);
	rt_free(htimer);
}


// queue

int luat_rtos_queue_create(luat_rtos_queue_t *queue_handle, uint32_t item_count, uint32_t item_size)
{
	if (!queue_handle) return -1;
    *queue_handle = rt_mq_create(NULL, item_size, item_count, RT_IPC_FLAG_FIFO);
	return (*queue_handle)?0:-1;
}

int luat_rtos_queue_delete(luat_rtos_queue_t queue_handle)
{
	if (!queue_handle) return -1;
    rt_mq_delete((rt_mq_t)queue_handle);
	return 0;
}

int luat_rtos_queue_send(luat_rtos_queue_t queue_handle, void *item, uint32_t item_size, uint32_t timeout)
{
	if (!queue_handle || !item) return -1;

    rt_err_t err = RT_ERROR;

    err = rt_mq_send(queue_handle, item, item_size);
    // rt_kprintf("mq_send ret: %d\n", err);
    if(err != RT_EOK)
        return -1;
    return 0;
	// if (luat_rtos_get_ipsr())
	// {
	// 	BaseType_t pxHigherPriorityTaskWoken;
	// 	if (xQueueSendFromISR(queue_handle, item, &pxHigherPriorityTaskWoken) != pdPASS)
	// 		return -1;
	// 	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	// 	return 0;
	// }
	// else
	// {
	// 	if (xQueueSend(queue_handle, item, luat_rtos_ms2tick(timeout)) != pdPASS)
	// 		return -1;
	// }
	// return 0;
}

int luat_rtos_queue_recv(luat_rtos_queue_t queue_handle, void *item, uint32_t item_size, uint32_t timeout)
{
    
	if (!queue_handle || !item)
		return -1;
    // rt_kprintf("timeout %ld\n", timeout);
    rt_err_t err = RT_ERROR;
    err = rt_mq_recv(queue_handle, item, item_size, timeout);
    // rt_kprintf("mq_recv ret: %d\n", err);
    if (err != RT_EOK)
		return -1;

    return 0;


	// if (!queue_handle || !item)
	// 	return -1;
	// BaseType_t yield = pdFALSE;
    
	// if (luat_rtos_get_ipsr())
	// {
	// 	if (xQueueReceiveFromISR(queue_handle, item, &yield) != pdPASS)
	// 		return -1;
	// 	portYIELD_FROM_ISR(yield);
	// 	return 0;
	// }
	// else
	// {
	// 	if (xQueueReceive(queue_handle, item, luat_rtos_ms2tick(timeout)) != pdPASS)
	// 		return -1;
	// }
	// return 0;
}


/*msgbus*/
#ifndef LUA_QUEUE_COUNT
#define LUA_QUEUE_COUNT     256
#endif
static luat_rtos_queue_t lua_queue_handle;
void luat_msgbus_init(void) {
    luat_rtos_queue_create(&lua_queue_handle, LUA_QUEUE_COUNT,  sizeof(rtos_msg_t));
}
uint32_t luat_msgbus_put(rtos_msg_t* msg, size_t timeout) {
    return luat_rtos_queue_send(lua_queue_handle, msg, sizeof(rtos_msg_t), timeout);
}

uint32_t luat_msgbus_get(rtos_msg_t* msg, size_t timeout) {
    return luat_rtos_queue_recv(lua_queue_handle, msg, sizeof(rtos_msg_t), timeout);
}

uint32_t luat_msgbus_freesize(void) {
    uint32_t item_cnt;
    // luat_rtos_queue_get_cnt(lua_queue_handle, &item_cnt);
    return LUA_QUEUE_COUNT - item_cnt;
}

uint8_t luat_msgbus_is_empty(void) {
    uint32_t item_cnt;
    // luat_rtos_queue_get_cnt(lua_queue_handle, &item_cnt);
    return !item_cnt;
}