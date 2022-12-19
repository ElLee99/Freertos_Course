//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#include <stdlib.h>

//Settings
static const uint8_t buf_len = 255;

//Task handles
static TaskHandle_t read_Serial_Handle = NULL;
static TaskHandle_t print_Serial_Handle = NULL;

//Global variables
static char *memory = NULL;


//Read Serial: reads the serial input and stores it in dynamic memory
void read_Serial(void *parameter){
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
      buf[idx] = c;
      idx ++;
    }
    
      if (c == '\n') {
        buf[idx - 1] = '\0';
        memory = (char*)pvPortMalloc(idx * sizeof(char));
        memcpy(memory, buf, idx);
        memset(buf, 0, buf_len);
        idx = 0;
        c= '\0';
        vTaskResume(print_Serial_Handle);
      }       
  }
}

//Print Serial: Prints the string we saved in the read serial task
void print_Serial(void *parameter){
    
  while(1){
    Serial.println(memory);
    vTaskDelay(500/portTICK_PERIOD_MS);
    vPortFree(memory);
    memory = NULL;
    vTaskSuspend(print_Serial_Handle);
  }
}
  
void setup() {

  Serial.begin(300);
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println();
  Serial.println("Enter a string");
  vTaskDelay(1000/portTICK_PERIOD_MS);

  
  xTaskCreatePinnedToCore(read_Serial,
                          "Read Serial",
                          1024,
                          NULL,
                          1,
                          &read_Serial_Handle,
                          app_cpu);

  xTaskCreatePinnedToCore(print_Serial,
                          "Print Serial",
                          1024,
                          NULL,
                          1,
                          &print_Serial_Handle,
                          app_cpu);

                          
                          
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
