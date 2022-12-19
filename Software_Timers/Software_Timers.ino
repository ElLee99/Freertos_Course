//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


//Settings
static const uint8_t buf_len = 255;

//Pins
static const int  LED_BUILTIN = 2;
static const int led_pin = LED_BUILTIN;

//Globals 
static TimerHandle_t dim_timer = NULL;


//*****************************************************************************
// Callbacks

void dim_Callback (TimerHandle_t xTimer){
  digitalWrite(led_pin, LOW);
}


//*****************************************************************************
// Tasks
void serial_Port(void *parameter){
  char c;

  
  while(1){
    if (Serial.available() > 0) {
        c = Serial.read();
        Serial.print(c);
        digitalWrite(led_pin, HIGH);
        
        xTimerStart(dim_timer, portMAX_DELAY); 
    }
  }
}



void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(300);
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println("WELCOMEEEEEEE");
  vTaskDelay(1000/portTICK_PERIOD_MS);


  xTaskCreatePinnedToCore(serial_Port,
                          "Serial Port",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  dim_timer = xTimerCreate(
                            "Dim Timer",                    // Name of timer
                            5000 / portTICK_PERIOD_MS,      // Period of timer (in ticks)
                            pdFALSE,                        // Auto-reload
                            (void *)0,                      // Timer ID
                            dim_Callback);                  //Callback function

  vTaskDelete(NULL);
}

void loop() {

}
