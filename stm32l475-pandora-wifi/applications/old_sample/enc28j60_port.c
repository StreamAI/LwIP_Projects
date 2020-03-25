#include "board.h"

#ifdef BSP_USING_ENC28J60

#include "drv_spi.h"
#include "enc28j60.h"
#include "drivers/pin.h"

// WIRELESS
#define PIN_NRF_IRQ   GET_PIN(D, 3)        // PD3 :  NRF_IRQ      --> WIRELESS
#define PIN_NRF_CE    GET_PIN(D, 4)        // PD4 :  NRF_CE       --> WIRELESS
#define PIN_NRF_CS    GET_PIN(D, 5)        // PD5 :  NRF_CS       --> WIRELESS

int enc28j60_init(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();
    rt_hw_spi_device_attach("spi2", "spi21", GPIOD, GPIO_PIN_5);

    /* attach enc28j60 to spi. spi21 cs - PD6 */
    enc28j60_attach("spi21");

    /* init interrupt pin */
    rt_pin_mode(PIN_NRF_IRQ, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(PIN_NRF_IRQ, PIN_IRQ_MODE_FALLING, (void(*)(void*))enc28j60_isr, RT_NULL);
    rt_pin_irq_enable(PIN_NRF_IRQ, PIN_IRQ_ENABLE);

    return 0;
}
INIT_COMPONENT_EXPORT(enc28j60_init);

#endif /* BSP_USING_ENC28J60 */
