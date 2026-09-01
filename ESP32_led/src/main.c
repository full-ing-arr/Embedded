#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>

#define LED_GPIO GPIO_NUM_3
#define BUTTON_GPIO GPIO_NUM_46

void app_main() {
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    printf("-------------------------");

    for (int i = 1; i < 100; i++) {
        printf("%d: %ld \n", i, pdMS_TO_TICKS(i));
    }

    printf("-------------------------");
    gpio_set_level(LED_GPIO, 1);

    int counter = 0;
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(2000));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        printf("Counter: %d\n", counter++);
    }
}
