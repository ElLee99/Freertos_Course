//To use only one core
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif


//String to print 
const char msg[] = "Las pizzas de don crangrejo son lo mejor";

//Task handles
static TaskHandle_t task_1 = NULL;
static TaskHandle_t task_2 = NULL;

//***********************************************************//
//Tasks


//Task: print to Serial Terminal with lower priority
void startTask1(void *parameter){
  uint8_t msg_len = strlen(msg);
  while(1){
    Serial.println();
    for (uint8_t i = 0; i < msg_len; i++){
      Serial.print(msg[i]);
    }
    Serial.println();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }


//Task: print "*" to Serial Terminal with higher priority on core 1
void startTask2(void *parameter){
  while(1){
    Serial.print("*");
    vTaskDelay(100/portTICK_PERIOD_MS); 
  }
}
  

void setup() {

  //Configure the serial to a slow baudrate in order to watch the preemption
  Serial.begin(300);

  
  //Wait a moment to start
  vTaskDelay(1000/portTICK_PERIOD_MS); 
  Serial.println();
  Serial.println("---FreeRTOS Task Demo---");

  //Print the priority that it's using the task rn, in our case prints the data of the setup task
  Serial.print("Setup and loop task running on core");
  Serial.print(xPortGetCoreID());
  Serial.print("with priority");
  Serial.println(uxTaskPriorityGet(NULL));

  //Task to run forever
  xTaskCreatePinnedToCore(startTask1,
                          "Task 1",
                          1024,
                          NULL,
                          1, 
                          &task_1,
                          app_cpu);


  //Task to run with higher priority
  xTaskCreatePinnedToCore(startTask2,
                          "Task 2",
                          1024,
                          NULL,
                          2,
                          &task_2,
                          app_cpu);

}

void loop() {

  //Suspend the higher priority task for some intervals
  for(uint8_t i = 0; i<3; i++){
    vTaskSuspend(task_2);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    vTaskResume(task_2);
    vTaskDelay(2000/portTICK_PERIOD_MS);
  }
  
  //Delete the lower priority task
  if(task_1 !=NULL){
    vTaskDelete(task_1);
    task_1 = NULL;
  }
}
