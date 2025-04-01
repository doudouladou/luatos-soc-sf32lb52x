#include "luat_base.h"
#include "rtthread.h"
#include "luat_timer.h"
#include "luat_rtos.h"
#include "c_common.h"

typedef struct
{
	void *timer;
	luat_rtos_timer_callback_t call_back;
	void *user_param;
	uint8_t is_repeat;
} luat_rtos_user_timer_t;

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

static void s_timer_callback(void *params)
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
		timer->timer = rt_timer_create(NULL, s_timer_callback, timer, 0, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
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
	if (!timer_handle)
		return -1;
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
	if (!timer)
		return -1;
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
	if (!queue_handle)
		return -1;
	*queue_handle = rt_mq_create(NULL, item_size, item_count, RT_IPC_FLAG_FIFO);
	return (*queue_handle) ? 0 : -1;
}

int luat_rtos_queue_delete(luat_rtos_queue_t queue_handle)
{
	if (!queue_handle)
		return -1;
	rt_mq_delete((rt_mq_t)queue_handle);
	return 0;
}

int luat_rtos_queue_send(luat_rtos_queue_t queue_handle, void *item, uint32_t item_size, uint32_t timeout)
{
	if (!queue_handle || !item)
		return -1;

	rt_err_t err = RT_ERROR;

	err = rt_mq_send(queue_handle, item, item_size);
	if (err != RT_EOK)
		return -1;
	return 0;
}

int luat_rtos_queue_recv(luat_rtos_queue_t queue_handle, void *item, uint32_t item_size, uint32_t timeout)
{

	if (!queue_handle || !item)
		return -1;
	rt_err_t err = RT_ERROR;
	err = rt_mq_recv(queue_handle, item, item_size, timeout);
	if (err != RT_EOK)
		return -1;
	return 0;
}

// mutex
void *luat_mutex_create(void) {
	return (void *)rt_mutex_create(NULL, RT_IPC_FLAG_PRIO);
};
LUAT_RET luat_mutex_lock(void *mutex) {
	rt_mutex_take((rt_mutex_t)mutex, RT_WAITING_FOREVER);
};
LUAT_RET luat_mutex_unlock(void *mutex) {
	rt_mutex_release((rt_mutex_t)mutex);
};
void luat_mutex_release(void *mutex) {
	rt_mutex_delete(mutex);
};


void luat_task_suspend_all(void)
{

}

void luat_task_resume_all(void)
{

}

typedef struct
{
	rt_thread_t handle;
	rt_mq_t queue;
	uint8_t is_run;
}task_handle_t;

void *create_event_task(TaskFun_t task_fun, void *param, uint32_t stack_bytes, uint8_t priority, uint16_t event_max_cnt, const char *task_name)
{

	task_handle_t *handle = luat_heap_malloc(sizeof(task_handle_t));


	if (event_max_cnt)
	{
		handle->queue = re_mq_create(NULL, sizeof(OS_EVENT), event_max_cnt, RT_IPC_FLAG_FIFO);
		if (!handle->queue)
		{
			luat_heap_free(handle);
			return NULL;
		}
	}
	else
	{
		handle->queue = NULL;
	}

	handle->handle = rt_thread_create(NULL, task_fun, param, stack_bytes, priority, RT_THREAD_TICK_DEFAULT);
	if (NULL == handle->handle)
	{
		if (handle->queue)
			rt_mq_delete(handle->queue);
		luat_heap_free(handle);
		return NULL;
	}

	handle->is_run = 1;
	return handle;
}


void delete_event_task(void *task_handle)
{
	task_handle_t *handle = (void *)task_handle;
	uint32_t cr;
	cr = luat_rtos_entry_critical();
	if (handle->queue)
	{
		rt_mq_delete(handle->queue);
	}
	handle->queue = NULL;
	handle->is_run = 0;
	luat_rtos_exit_critical(cr);
	// vTaskDelete(handle->handle);
}

int send_event_to_task(void *task_handle, OS_EVENT *event, uint32_t event_id, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t timeout_ms)
{
	if (!task_handle) return -1;
	task_handle_t *handle = (void *)task_handle;
	OS_EVENT Event;
	if (event)
	{
		Event = *event;
	}
	else
	{
		Event.ID = event_id;
		Event.Param1 = param1;
		Event.Param2 = param2;
		Event.Param3 = param3;
	}

	rt_err_t err = RT_ERROR;
	err = rt_mq_send(handle->queue, &Event, sizeof(OS_EVENT));
	// if (luat_rtos_get_ipsr())
	// {
	// 	result = xQueueSendToBackFromISR(handle->queue, &Event, &xHigherPriorityTaskWoken);
	// 	if (xHigherPriorityTaskWoken)
	// 	{
	// 		portYIELD_WITHIN_API();
	// 	}
	// }
	// else
	// {
	// 	return ((pdPASS == xQueueSendToBack(handle->queue, &Event, timeout_ms))?0:-1);
	// }
	return (RT_EOK == err)?0:-1;

	
}

int get_event_from_task(void *task_handle, uint32_t target_event_id, OS_EVENT *event,  CBFuncEx_t callback, uint32_t timeout_ms)
{
	uint64_t start_ms = GetSysTickMS();
	int32_t result = ERROR_NONE;
	uint32_t wait_ms = timeout_ms;
	task_handle_t *handle = (void *)task_handle;
	int ret = 0;
	// if (xTaskGetCurrentTaskHandle() != handle->handle)
	// {
	// 	return -1;
	// }
	if (luat_rtos_get_ipsr())
	{
		return -ERROR_PERMISSION_DENIED;
	}
	if (!task_handle) return -ERROR_PARAM_INVALID;

	if (!wait_ms)
	{
		wait_ms = RT_WAITING_FOREVER;
	}
GET_NEW_EVENT:
	if ((ret = rt_mq_recv(handle->queue, event, sizeof(OS_EVENT), wait_ms)) != RT_EOK)
	{
		return -ERROR_OPERATION_FAILED;
	}

	if ((target_event_id == CORE_EVENT_ID_ANY) || (event->ID == target_event_id))
	{
		goto GET_EVENT_DONE;
	}
	if (callback)
	{
		callback(event, task_handle);
	}

	if ((timeout_ms != RT_WAITING_FOREVER) && timeout_ms)
	{
		if (timeout_ms > (uint32_t)(GetSysTickMS() - start_ms + 3))
		{
			wait_ms = timeout_ms - (uint32_t)(GetSysTickMS() - start_ms);
		}
		else
		{
			return -ERROR_OPERATION_FAILED;
		}
	}
	else
	{
		wait_ms = RT_WAITING_FOREVER;
	}
	goto GET_NEW_EVENT;
GET_EVENT_DONE:
	return result;
}

// rtos 
int luat_rtos_task_create(luat_rtos_task_handle *task_handle, uint32_t stack_size, uint8_t priority, const char *task_name, luat_rtos_task_entry task_fun, void* user_data, uint16_t event_cout)
{
	if (!task_handle) return -1;
	*task_handle = create_event_task(task_fun, user_data, stack_size, priority, event_cout, task_name);
	return (*task_handle)?0:-1;
}
int luat_rtos_event_send(luat_rtos_task_handle task_handle, uint32_t id, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t timeout)
{
	if (!task_handle) return -1;
	return send_event_to_task(task_handle, NULL, id, param1, param2, param3, timeout);
}

int luat_rtos_event_recv(luat_rtos_task_handle task_handle, uint32_t wait_event_id, luat_event_t *out_event, luat_rtos_event_wait_callback_t *callback_fun, uint32_t timeout){
	if (!task_handle) return -1;
	return get_event_from_task(task_handle, wait_event_id, (void *)out_event, (CBFuncEx_t)callback_fun, timeout);
}

int luat_send_event_to_task(void *task_handle, uint32_t id, uint32_t param1, uint32_t param2, uint32_t param3)
{
	if (!task_handle) return -1;
	return send_event_to_task(task_handle, NULL, id, param1, param2, param3, LUAT_WAIT_FOREVER);
}

int luat_wait_event_from_task(void *task_handle, uint32_t wait_event_id, luat_event_t *out_event, void *call_back, uint32_t ms)
{
	if (!task_handle) return -1;
	return get_event_from_task(task_handle, wait_event_id, (void *)out_event, (CBFuncEx_t)call_back, ms);
}


/*msgbus*/
#ifndef LUA_QUEUE_COUNT
#define LUA_QUEUE_COUNT 256
#endif
static luat_rtos_queue_t lua_queue_handle;
void luat_msgbus_init(void)
{
	luat_rtos_queue_create(&lua_queue_handle, LUA_QUEUE_COUNT, sizeof(rtos_msg_t));
}
uint32_t luat_msgbus_put(rtos_msg_t *msg, size_t timeout)
{
	return luat_rtos_queue_send(lua_queue_handle, msg, sizeof(rtos_msg_t), timeout);
}

uint32_t luat_msgbus_get(rtos_msg_t *msg, size_t timeout)
{
	return luat_rtos_queue_recv(lua_queue_handle, msg, sizeof(rtos_msg_t), timeout);
}

uint32_t luat_msgbus_freesize(void)
{
	uint32_t item_cnt;
	// luat_rtos_queue_get_cnt(lua_queue_handle, &item_cnt);
	return LUA_QUEUE_COUNT - item_cnt;
}

uint8_t luat_msgbus_is_empty(void)
{
	uint32_t item_cnt;
	// luat_rtos_queue_get_cnt(lua_queue_handle, &item_cnt);
	return !item_cnt;
}

