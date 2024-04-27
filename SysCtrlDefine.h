#ifndef __SYS_CTRL_DEFINE__
#define __SYS_CTRL_DEFINE__
//LED
#define LED_BRIGHTNESS          "/sys/devices/platform/leds/leds/led-err/brightness"
//PWM
#define PWM_DUTY_CYCLE          "/sys/class/pwm/pwmchip%d/pwm0/duty_cycle"
#define PWM_ENABLE              "/sys/class/pwm/pwmchip%d/pwm0/enable"
#define PWM_PERIOD              "/sys/class/pwm/pwmchip%d/pwm0/period"
#define PWM_EXPORT              "/sys/class/pwm/pwmchip%d/export"
//ADC
#define ADC_DEV_NAME1           "/sys/bus/platform/devices/2198000.adc/iio:device0/in_voltage8_raw"
#define ADC_DEV_NAME2           "/sys/bus/platform/devices/2198000.adc/iio:device0/in_voltage9_raw"
//adc init
#define ADC_DIRECTION           "/sys/class/gpio/gpio5/direction"
#define ADC_EXPORT              "/sys/class/gpio/export"

//GPIO value
#define SYS_GPIO_VALUE          "/sys/class/gpio/gpio5/value"


//plc
#define PLC_RESET_DIRECTION     "/sys/class/gpio/gpio131/direction"
#define PLC_RESET_GPIO          "/sys/class/gpio/gpio131/value"
#endif


