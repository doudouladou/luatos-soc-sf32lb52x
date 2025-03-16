#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "stdio.h"
#include "string.h"
#include "rtthread.h"

/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    int ret = -1;
    // ret = rt_flash_erase(LUAT_SCRIPT_START_ADDR, LUAT_SCRIPT_SIZE);
    if (ret != 0)
    {
        // return -1;
    }
    luat_main_sifli();
    /* Infinite loop */
    while (1)
    {
      rt_kprintf("Hello world!\n");
      rt_thread_delay(600000);
    }
    return 0;
}

