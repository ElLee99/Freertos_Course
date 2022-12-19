//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <stdlib.h>

//Settings
static const uint8_t buf_len = 20;

//Pins
static const int  LED_BUILTIN = 2;
static const int led_pin = LED_BUILTIN;


//Task handles
//static TaskHandle_t task_1 = NULL;
//static TaskHandle_t task_2 = NULL;

//Global variables
int blinking_time = 500;


//Task 1: ask for a time in the serial terminal that will be the blinking time
void startTask1(void *parameter){
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  // Clear whole buffer
  memset(buf, 0, buf_len);

  // Loop forever
  while (1) {

    // Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();

      // Update delay variable and reset buffer if we get a newline character
      if (c == '\n') {
        blinking_time = atoi(buf);
        Serial.print("Updated LED delay to: ");
        Serial.println(blinking_time);
        memset(buf, 0, buf_len);
        idx = 0;
      } else {
        
        // Only append if index is not over message limit
        if (idx < buf_len - 1) {
          buf[idx] = c;
          idx++;
        }
      }
    }
  }
}


//Task 2:Blinks the time for the time specified
void startTask2(void *parameter){
  while(1){
    digitalWrite(led_pin, HIGH);
    vTaskDelay(blinking_time/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(blinking_time/portTICK_PERIOD_MS);
  }
}
void setup() {
  pinMode(led_pin, OUTPUT);

  
  Serial.begin(300);
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println();
  Serial.println("Enter the blinking time of the LED");
   vTaskDelay(1000/portTICK_PERIOD_MS);
   
  //Start the task 1
  xTaskCreatePinnedToCore(startTask1,
                          "Task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  xTaskCreatePinnedToCore(startTask2,
                          "Task 2",
                          1024,
                          NULL,
                          2,
                          NULL,
                          app_cpu);


  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
