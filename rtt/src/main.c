#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "rtthread.h"

#define TEST_PAN 0
#define TEST_EARSE_SCRIPT 0

/**
 * @brief  Main program
 * @param  None
 * @retval 0 if success, otherwise failure number
 */
int main(void)
{
#if TEST_EARSE_SCRIPT == 1
  int ret = -1;
  ret = rt_flash_erase(LUAT_SCRIPT_START_ADDR, LUAT_SCRIPT_SIZE);
  if (ret != 0)
  {
    return -1;
  }
#endif
#if TEST_PAN == 1
  bt_pan_luat_net_init();
#endif
  luat_main_sifli();
  /* Infinite loop */
  while (1)
  {
    rt_kprintf("Hello world!\n");
    rt_thread_delay(1000);
  }
  return 0;
}
