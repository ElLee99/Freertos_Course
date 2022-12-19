//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


//Pins
static const int  LED_BUILTIN = 2;
static const int led_pin = LED_BUILTIN;

//Our task: blink a LED
void toggleLed1(void *parameter){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(500/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(625/portTICK_PERIOD_MS);
  }
}


void toggleLed2(void *parameter){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(250/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(250/portTICK_PERIOD_MS);
  }
}

void setup() {

  //Configure pin
  pinMode(led_pin, OUTPUT);

  //Task to tun forever
  xTaskCreatePinnedToCore(
                           toggleLed1,   //Function to be called
                           "Toggle Led", //Name of task
                           1024,        //Stack size(bytes in ESP32, word in FreeRTOS)
                           NULL,        //Parameter to pass to function (pointer)
                           1,           //Task priority (0 to configMAX_PRIORITIES -1)
                           NULL,        //Task Handle
                           app_cpu);    
    
  xTaskCreatePinnedToCore(
                           toggleLed2,   //Function to be called
                           "Toggle Led", //Name of task
                           1024,        //Stack size(bytes in ESP32, word in FreeRTOS)
                           NULL,        //Parameter to pass to function (pointer)
                           2,           //Task priority (0 to configMAX_PRIORITIES -1)
                           NULL,        //Task Handle
                           app_cpu); 

  
}

void loop() {

}
