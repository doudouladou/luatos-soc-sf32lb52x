#include "luat_mcu.h"
#include "bf0_pin_const.h"
#include "bf0_hal_pinmux.h"
long luat_mcu_ticks(void) {
    return rt_tick_get();
}

uint64_t luat_mcu_tick64(void) {
    return 0;
}

uint64_t luat_mcu_tick64_ms(void) {
    return 0;
}

int luat_mcu_us_period(void) {
    return 0;
}

uint32_t luat_mcu_hz(void) {
    return 0;
}
void luat_mcu_set_clk_source(uint8_t source_main, uint8_t source_32k, uint32_t delay) {

}

int luat_mcu_set_clk(size_t mhz) {
    return 0;
}

int luat_mcu_get_clk(void) {
    return 0;
}

const char* luat_mcu_unique_id(size_t* t) {
    return NULL;
}

// uart alt tx=1, rx=0
void luat_mcu_iomux_ctrl(uint8_t type, uint8_t sn, int pad_index, uint8_t alt, uint8_t is_input)
{
    if (type == LUAT_MCU_PERIPHERAL_UART)
    {
        HAL_PIN_Set(PAD_PA00 + pad_index, alt == 1 ? (USART1_RXD + (sn - 1) * 4 + 1) : (USART1_RXD + (sn - 1) * 4), PIN_PULLUP, 1);
    }
}