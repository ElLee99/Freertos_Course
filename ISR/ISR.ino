//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t TIMER_DIVIDER = 80;
static const uint64_t TIMER_MAX_COUNT = 100000;
static const uint8_t CONSOLE_BUF_LEN = 255;
static const uint8_t DOUBLE_BUF_LEN = 10;
static const char COMPARISON [] = "avg";


//Pins
static const int adc_pin = A0;
static const int  LED_BUILTIN = 2;
static const int led_pin = LED_BUILTIN;


//Globals
static hw_timer_t *timer_0 = NULL;
static TaskHandle_t avg_task = NULL;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static volatile uint16_t buf_up[DOUBLE_BUF_LEN];
static volatile uint16_t buf_down[DOUBLE_BUF_LEN];
static volatile uint16_t *write_buffer = buf_up;
static volatile uint16_t *read_buffer = buf_down;
static float glo_avg;


//*****************************************************************************
// Functions that can be called from anywhere (in this file)

//Swap buffer interruption
// Swap the write_to and read_from pointers in the double buffer
// Only ISR calls this at the moment, so no need to make it thread-safe
void IRAM_ATTR swap_buffer(){
  volatile uint16_t *temporary_ptr = write_buffer;
  write_buffer = read_buffer;
  read_buffer = temporary_ptr;
}


//*****************************************************************************
// Interrupt Service Routines (ISRs)

// Timer 0 Interruption
// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer_0(){

  static uint8_t idx = 0;
  BaseType_t higher_priority_task_woken = pdFALSE;

  if(idx < DOUBLE_BUF_LEN){
    write_buffer [idx] = analogRead(adc_pin);
    idx++;
  }

  else{
    idx = 0;
    swap_buffer();

     // A task notification works like a binary semaphore but is faster
    vTaskNotifyGiveFromISR(avg_task, &higher_priority_task_woken);
  }


  //If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
  //should be performed to ensure the interrupt returns directly to the highest
  //priority task.
  if (higher_priority_task_woken){
    portYIELD_FROM_ISR();
  }
  
}




//*****************************************************************************
// Tasks

//"Serial Console" Task
void serial_Console(void * parameters){
  char c;
  char buf[CONSOLE_BUF_LEN];
  uint8_t idx = 0;
  char word_comp[3];

  // Clear whole buffer
  memset(buf, 0, CONSOLE_BUF_LEN);
    
  while(1){

  // Read characters from serial
    if (Serial.available() > 0) {
      c = Serial.read();
      Serial.print(c);
      buf[idx] = c;
      idx ++;
    }

    if (c == '\n') {
        memcpy(word_comp, buf, 3);
        if (memcmp(word_comp, COMPARISON, 3) == 0){
        Serial.print("Average = ");
        Serial.println(glo_avg);
        }

    memset(buf, 0, CONSOLE_BUF_LEN);
    idx = 0;
    }
    
     // Don't hog the CPU. Yield to other tasks for a while
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}


//"Calculate Average" task
void calc_Average (void* parameter){
  float avg;

  while(1){
    
    // Wait for notification from ISR (similar to binary semaphore)
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    avg = 0;
    for(int i = 0; i < DOUBLE_BUF_LEN; i++){
      avg += (float)read_buffer[i];
      //vTaskDelay(105 / portTICK_PERIOD_MS); // Uncomment to test overrun flag
    }
    avg /= DOUBLE_BUF_LEN;


    // Updating the shared float may or may not take multiple isntructions, so
    // we protect it with a mutex or critical section. The ESP-IDF critical
    // section is the easiest for this application.
    portENTER_CRITICAL(&spinlock);
    glo_avg = avg;
    portEXIT_CRITICAL(&spinlock);


    
  }
}

//*****************************************************************************

void setup() {
  Serial.begin(9600);
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println("WELCOMEEEEEEE");
  vTaskDelay(1000/portTICK_PERIOD_MS);
  
  timer_0 = timerBegin(0, TIMER_DIVIDER, true);           // Create and start timer (num, divider, countUp)
  timerAttachInterrupt(timer_0 , &onTimer_0, true);       // Provide ISR to timer (timer, function, edge)
  timerAlarmWrite(timer_0, TIMER_MAX_COUNT, true);        // At what count should ISR trigger (timer, count, autoreload)
  timerAlarmEnable(timer_0);                              // Allow ISR to trigger


  xTaskCreatePinnedToCore(serial_Console,     //Function to be called
                          "Serial Console",   //Name of the task
                          1024,               //Stack size (bytes in ESP32, word in FreeRTOS)
                          NULL,               //Parameter to pass to function (pointer)
                          1,                  //Task priority
                          NULL,               //Task Handle
                          app_cpu);


  xTaskCreatePinnedToCore(calc_Average,
                         "Calculate Average",
                         1024,
                         NULL,
                         2,
                         &avg_task,
                         app_cpu);
  

  vTaskDelete(NULL);
}

void loop() {

}
