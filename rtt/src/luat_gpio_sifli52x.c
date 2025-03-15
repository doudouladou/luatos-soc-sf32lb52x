#include "luat_gpio.h"
#include "rtdevice.h"
#include "luat_log.h"
#define LUAT_LOG_TAG "gpio"
#define MAX_GPIO 30

int luat_gpio_irq_default(int pin, void *args);

static void luat_gpio_irq_callback(void *args)
{

    int pin = (int) args;
    if (pin < 0 || pin >= MAX_GPIO)
        return;
    luat_gpio_irq_default(pin, (void *)rt_pin_read(pin));
}

int luat_gpio_setup(luat_gpio_t *gpio)
{
    int pin = gpio->pin;
    if (pin < 0 || pin >= MAX_GPIO)
        return 0;
    switch (gpio->mode)
    {
    case Luat_GPIO_OUTPUT:
        rt_pin_mode(pin, PIN_MODE_OUTPUT);
        break;
    case Luat_GPIO_INPUT:
    case Luat_GPIO_IRQ:
    default:
    {
        switch (gpio->pull)
        {
        case Luat_GPIO_PULLUP:
            rt_pin_mode(pin, PIN_MODE_INPUT_PULLUP);
            break;
        case Luat_GPIO_PULLDOWN:
            rt_pin_mode(pin, PIN_MODE_INPUT_PULLDOWN);
            break;
        default:
            rt_pin_mode(pin, PIN_MODE_INPUT);
            break;
        }
        break;
    }
    }
    if (gpio->mode == Luat_GPIO_IRQ)
    {
        int mode = PIN_IRQ_MODE_RISING;
        switch (gpio->irq)
        {
        case Luat_GPIO_RISING:
            mode = PIN_IRQ_MODE_RISING;
            break;
        case Luat_GPIO_FALLING:
            mode = PIN_IRQ_MODE_FALLING;
            break;
        case Luat_GPIO_HIGH_IRQ:
            mode = PIN_IRQ_MODE_HIGH_LEVEL;
            break;
        case Luat_GPIO_LOW_IRQ:
            mode = PIN_IRQ_MODE_LOW_LEVEL;
            break;
        case Luat_GPIO_BOTH:
            mode = PIN_IRQ_MODE_RISING_FALLING;
            break;
        default:
            break;
        }
        rt_pin_attach_irq(pin, mode, luat_gpio_irq_callback, (void *)(rt_uint32_t) pin);
        rt_pin_irq_enable(pin, PIN_IRQ_ENABLE);
        return 0;
    }
    else
    {
        rt_pin_irq_enable(pin, PIN_IRQ_DISABLE);
        rt_pin_detach_irq(pin);
    }
    return 0;
}


int luat_gpio_open(luat_gpio_cfg_t *gpio)
{
    int pin = gpio->pin;
    if (pin < 0 || pin >= MAX_GPIO)
        return 0;
    switch (gpio->mode)
    {
    case Luat_GPIO_OUTPUT:
        rt_pin_mode(pin, PIN_MODE_OUTPUT);
        break;
    case Luat_GPIO_INPUT:
    case Luat_GPIO_IRQ:
    default:
    {
        switch (gpio->pull)
        {
        case Luat_GPIO_PULLUP:
            rt_pin_mode(pin, PIN_MODE_INPUT_PULLUP);
            break;
        case Luat_GPIO_PULLDOWN:
            rt_pin_mode(pin, PIN_MODE_INPUT_PULLDOWN);
            break;
        default:
            rt_pin_mode(pin, PIN_MODE_INPUT);
            break;
        }
        break;
    }
    }
    if (gpio->mode == Luat_GPIO_IRQ)
    {
        int mode = PIN_IRQ_MODE_RISING;
        switch (gpio->irq_type)
        {
        case Luat_GPIO_RISING:
            mode = PIN_IRQ_MODE_RISING;
            break;
        case Luat_GPIO_FALLING:
            mode = PIN_IRQ_MODE_FALLING;
            break;
        case Luat_GPIO_HIGH_IRQ:
            mode = PIN_IRQ_MODE_HIGH_LEVEL;
            break;
        case Luat_GPIO_LOW_IRQ:
            mode = PIN_IRQ_MODE_LOW_LEVEL;
            break;
        case Luat_GPIO_BOTH:
            mode = PIN_IRQ_MODE_RISING_FALLING;
            break;
        default:
            break;
        }
        rt_pin_attach_irq(pin, mode, luat_gpio_irq_callback, (void *)(rt_uint32_t) pin);
        rt_pin_irq_enable(pin, PIN_IRQ_ENABLE);
        
        return 0;
    }
    else
    {
        rt_pin_irq_enable(pin, PIN_IRQ_DISABLE);
        rt_pin_detach_irq(pin);
    }
    return 0;
}
/**
 * @brief GPIO输出电平
 * @param Pin Pin序号
 * @param Level 1高电平，0低电平
 */
int luat_gpio_set(int pin, int level)
{
    rt_pin_write(pin, level);
    return 0;
}

/**
 * @brief 读取GPIO输入电平
 * @param Pin Pin序号
 * @return 1高电平， 0低电平，其他无效
 */
int luat_gpio_get(int pin)
{
    return rt_pin_read(pin);
}
/**
 * @brief 关闭GPIO
 * @param Pin Pin序号
 */
void luat_gpio_close(int pin)
{
    rt_pin_irq_enable(pin, PIN_IRQ_DISABLE);
    rt_pin_detach_irq(pin);
    rt_pin_mode(pin, PIN_MODE_INPUT);
    return;
}


