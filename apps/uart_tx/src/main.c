#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <stdio.h>
#define MY_STACK_SIZE 500
#define MY_PRIORITY 5
#define LED1_NODE DT_NODELABEL(led_1)
#define LED2_NODE DT_NODELABEL(led_2)
/* change this to any other UART peripheral if desired */
//#define UART_DEVICE_NODE DT_NODELABEL(usart1)
#define UART_DEVICE_NODE DT_ALIAS(uart_tutorial)
#define USER_BUTTON DT_NODELABEL(user_button)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(USER_BUTTON, gpios);
static struct gpio_callback button_cb_data;
// Use alias if you want your code to be portable between boards
//#define UART_DEVICE_NODE DT_ALIAS(uart_tutorial)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
char message[10],null[] = "\0";
//const char *message = "This is a 40-byte long message. 1234567";
K_THREAD_STACK_DEFINE(led1_thread_stack_area, MY_STACK_SIZE);
struct k_thread led1_thread_data;
k_tid_t led1_threadid;
//int t1=0;
int count=0;
struct k_sem button_pressed_sem;
uint64_t button_press_states = 0x00;

void led1_control_thread(void *, void *, void *) {
    while (1) {
        //LOG_INF("led1_control_thread");
        int state = gpio_pin_get_dt(&button);
        button_press_states = button_press_states << 1;
        if (state == 1) {
            button_press_states |= 0x01;
        }
        
        uint8_t button_pressed_detected = button_press_states & 0x3ff;
        if (button_pressed_detected) {
            LOG_INF("phat hien nut nhan");
            if (count == 0) {
                k_sem_give(&button_pressed_sem);
            }
            count++;
        }
        k_msleep(20);
    }
}
K_THREAD_STACK_DEFINE(led1_thread_stack_area, MY_STACK_SIZE);
struct k_thread led1_thread_data;
k_tid_t led1_threadid;
int main(void)
{
    k_sem_init(&button_pressed_sem, 0, 1);
    led1_threadid = k_thread_create(&led1_thread_data, led1_thread_stack_area,
        K_THREAD_STACK_SIZEOF(led1_thread_stack_area),
        led1_control_thread,
        NULL, NULL, NULL,
        4, 0, K_NO_WAIT);
    // led1_threadid = k_thread_create(&led1_thread_data, led1_thread_stack_area,
    //     K_THREAD_STACK_SIZEOF(led1_thread_stack_area),
    //     led1_control_thread,
    //     NULL, NULL, NULL,
    //     MY_PRIORITY, 0, K_NO_WAIT);
    
        
    int ret;

    if (!gpio_is_ready_dt(&led1)) {
            return 0;
    }

    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
            return 0;
    }
    if (!gpio_is_ready_dt(&led2)) {
            return 0;
    }

    ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
            return 0;
    }
    if (!gpio_is_ready_dt(&button)) {
        return 0;
    } 
    k_thread_start(led1_threadid);
   
    while (1) {
        LOG_INF("dang o truoc ksemtake ham main");
        k_sem_take(&button_pressed_sem, K_FOREVER);
        LOG_INF("dang o sau ksemtake ham main");
        k_msleep(2000);///??????
        LOG_INF("dang o sau delay2s ham main");
        sprintf(message, "%d", count);
        //LOG_INF("elapsed: %s ,count = %d %d", message,count,strlen(message)); ?????

        for (int i = 0; i < strlen(message) + 1; i++) {
            uart_poll_out(uart_dev, *(message + i));
        }
        count = 0;
        //count=0;
        gpio_pin_toggle_dt(&led2);
        // k_sem_take(&button_pressed_sem, K_NO_WAIT);
    }
    return 0;
}