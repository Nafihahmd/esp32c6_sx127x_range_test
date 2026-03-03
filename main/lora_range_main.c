
/*********************8*****
#define LED_PIN GPIO_NUM_15
#define LORA_CS    GPIO_NUM_17
#define LORA_RESET GPIO_NUM_21
#define LORA_DIO0  GPIO_NUM_2

#define SPI_MOSI GPIO_NUM_18
#define SPI_MISO GPIO_NUM_20
#define SPI_SCK  GPIO_NUM_19

static void led_blink(int count)
{
    for(int i=0;i<count;i++) {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
************************************/
/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "lora.h"

static void led_blink(int count)
{
    for(int i=0;i<count;i++) {
        gpio_set_level(CONFIG_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(CONFIG_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

#if CONFIG_SENDER
void task_tx(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t buf[255]; // Maximum Payload size of SX1276/77/78/79 is 255
	while(1) {
		TickType_t nowTick = xTaskGetTickCount();
		int send_len = sprintf((char *)buf,"Hello World!!");
		lora_send_packet(buf, send_len);
		ESP_LOGI(pcTaskGetName(NULL), "%d byte packet sent...", send_len);
		int lost = lora_packet_lost();
		if (lost != 0) {
			ESP_LOGW(pcTaskGetName(NULL), "%d packets lost", lost);
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_SENDER

#if CONFIG_RECEIVER
void task_rx(void *pvParameters)
{
	ESP_LOGI(pcTaskGetName(NULL), "Start");
	uint8_t buf[255]; // Maximum Payload size of SX1276/77/78/79 is 255
    int rssi, snr;
	while(1) {
		lora_receive(); // put into receive mode
		if (lora_received()) {
			int rxLen = lora_receive_packet(buf, sizeof(buf));
            rssi = lora_packet_rssi();
            snr = lora_packet_snr();
			ESP_LOGI(pcTaskGetName(NULL), "received(%d):[%.*s]", rxLen, rxLen, buf);
            ESP_LOGI(pcTaskGetName(NULL), "Rssi: %d, snr: %d", rssi, snr);
            led_blink(2);
		}
		vTaskDelay(1); // Avoid WatchDog alerts
	} // end while

	// never reach here
	vTaskDelete( NULL );
}
#endif // CONFIG_RECEIVER

void app_main()
{
	// Initialize LoRa
	if (lora_init() == 0) {
		ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
		while(1) {
			vTaskDelay(1);
		}
	}

#if CONFIG_RECEIVER
    // Initialize LED GPIO
    
    // LED setup
    gpio_reset_pin(CONFIG_LED_PIN);
    gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);
#endif

#if CONFIG_433MHZ
	ESP_LOGI(pcTaskGetName(NULL), "Frequency is 433MHz");
	lora_set_frequency(433e6); // 433MHz
#elif CONFIG_866MHZ
	ESP_LOGI(pcTaskGetName(NULL), "Frequency is 866MHz");
	lora_set_frequency(866e6); // 866MHz
#elif CONFIG_915MHZ
	ESP_LOGI(pcTaskGetName(NULL), "Frequency is 915MHz");
	lora_set_frequency(915e6); // 915MHz
#elif CONFIG_OTHER
	ESP_LOGI(pcTaskGetName(NULL), "Frequency is %dMHz", CONFIG_OTHER_FREQUENCY);
	long frequency = CONFIG_OTHER_FREQUENCY * 1000000;
	lora_set_frequency(frequency);
#endif

	lora_enable_crc();

	int cr = 1;
	int bw = 7;
	int sf = 7;
#if CONFIG_ADVANCED
	cr = CONFIG_CODING_RATE;
	bw = CONFIG_BANDWIDTH;
	sf = CONFIG_SF_RATE;
#endif

	lora_set_coding_rate(cr);
	//lora_set_coding_rate(CONFIG_CODING_RATE);
	//cr = lora_get_coding_rate();
	ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", cr);

	lora_set_bandwidth(bw);
	//lora_set_bandwidth(CONFIG_BANDWIDTH);
	//int bw = lora_get_bandwidth();
	ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", bw);

	lora_set_spreading_factor(sf);
	//lora_set_spreading_factor(CONFIG_SF_RATE);
	//int sf = lora_get_spreading_factor();
	ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", sf);

#if CONFIG_SENDER
	xTaskCreate(&task_tx, "TX", 1024*3, NULL, 5, NULL);
#endif
#if CONFIG_RECEIVER
	xTaskCreate(&task_rx, "RX", 1024*3, NULL, 5, NULL);
#endif
}
