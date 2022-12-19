//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <stdlib.h>


//Settings
static const uint8_t buf_len = 255;
static const uint8_t Queue1_len = 5;
static const uint8_t Queue2_len = 5;


//Structs
typedef struct data_TaskB{
  char message[25]; 
  uint8_t blink_counter;
}data_TaskB;


//Globals 
static QueueHandle_t Queue1;
static QueueHandle_t Queue2;


//Pins
static const int  LED_BUILTIN = 2;
static const int led_pin = LED_BUILTIN;


//Tasks
////////////////////////////////////////////

//Task A
void task_A (void *parameters){
  char c;
  char buf[buf_len];
  char delay_comp[5];
  char delay_time [5];
  uint8_t idx = 0;
  uint16_t value_Delay; 


  //data_TaskB dat_received;
  data_TaskB *ptr_dat_received;// = &dat_received;
  
  
  // Clear whole buffer
  memset(buf, 0, buf_len);

  // Loop forever
  while (1) {
    
    if (xQueueReceive(Queue2, (void *)&ptr_dat_received, 0) == pdTRUE){
      Serial.print(ptr_dat_received -> message);
      Serial.print(" = ");
      Serial.println(ptr_dat_received -> blink_counter);
    }

    // Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();
      Serial.print(c);
      buf[idx] = c;
      idx ++;
    }
    
      if (c == '\n') {
        memcpy(delay_comp, buf, 5);
        
  
     if (memcmp(delay_comp, "delay", 5) == 0){
      Serial.println("Oki Doki");
      memcpy(delay_time, (void *)&buf+sizeof(char)*5, (sizeof(char)*4));
        value_Delay = atoi(delay_time);
        if (xQueueSend(Queue1, (void *)&value_Delay, 0) != pdTRUE){
          Serial.println("Queue full");
        }
      }
        buf[idx - 1] = '\0';
        idx = 0;
        c= '\0';
        memset(buf, 0, buf_len);
        memset(delay_comp, 0, 5);
      }
     }
  }


//Task B
void task_B(void *parameter){
  static uint16_t blinking_time = 500; 
  uint16_t new_Value_Delay;
  uint8_t counter = 0;
  data_TaskB dat;
  data_TaskB *pointer_TasKB = &dat;

  pinMode(LED_BUILTIN, OUTPUT);

  while(1){
    if (xQueueReceive(Queue1, (void *)&new_Value_Delay, 0) == pdTRUE) {
      blinking_time = new_Value_Delay;
    }

    digitalWrite(led_pin, HIGH);
    vTaskDelay(blinking_time/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(blinking_time/portTICK_PERIOD_MS);
    counter ++;
    
    if (counter == 100){
      memcpy(dat.message, "Blinked", sizeof("Blinked"));
      dat.blink_counter = dat.blink_counter + 1;
      if(xQueueSend(Queue2, (void *)&pointer_TasKB, 0) != pdTRUE){
        Serial.println("Queue full");
      }
    }
  }
}


void setup() {
  data_TaskB ptr_size;
  Serial.begin(300);
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println("Welcome to this Little Program");

  
  Queue1 = xQueueCreate(Queue1_len, sizeof(uint16_t));
  Queue2 = xQueueCreate(Queue2_len, sizeof(&ptr_size));

  
  xTaskCreatePinnedToCore(task_A,
                          "Task A",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  xTaskCreatePinnedToCore(task_B,
                          "Task B",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);


  
  vTaskDelete(NULL);
}

void loop() {


}
