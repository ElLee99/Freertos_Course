/**
 * FreeRTOS Counting Semaphore Challenge
 * 
 * Challenge: use a mutex and counting semaphores to protect the shared buffer 
 * so that each number (0 throguh 4) is printed exactly 3 times to the Serial 
 * monitor (in any order). Do not use queues to do this!
 * 
 * Hint: you will need 2 counting semaphores in addition to the mutex, one for 
 * remembering number of filled slots in the buffer and another for 
 * remembering the number of empty slots in the buffer.
 * 
 * Date: January 24, 2021
 * Author: Shawn Hymel
 * License: 0BSD
 */


// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5};                  // Size of buffer array
static const int num_prod_tasks = 5;  // Number of producer tasks
static const int num_cons_tasks = 2;  // Number of consumer tasks
static const int num_writes = 3;      // Num times each producer writes to buf

// Globals
static int buf[BUF_SIZE]; //= {9,9,9,9,9};             // Shared buffer
static int head = 0;                  // Writing index to buffer
static int tail = 0;                  // Reading index to buffer
static SemaphoreHandle_t bin_sem;     // Waits for parameter to be read

static SemaphoreHandle_t mutex;
static SemaphoreHandle_t head_sem;
static SemaphoreHandle_t tail_sem;
//*****************************************************************************
// Tasks

// Producer: write a given number of times to shared buffer
void producer(void *parameters) {

  // Copy the parameters into a local variable
  int num = *(int *)parameters;

  // Release the binary semaphore
  xSemaphoreGive(bin_sem);

  // Fill shared buffer with task number
  for (int i = 0; i < num_writes; i++) {

    
    // Wait for empty slot in buffer to be available
    xSemaphoreTake(head_sem, portMAX_DELAY);
    
    // Lock critical section with a mutex
    xSemaphoreTake(mutex, portMAX_DELAY);
    
    // Critical section (accessing shared buffer)
    buf[head] = num;
    head = (head + 1) % BUF_SIZE;

    // Signal to consumer tasks that a slot in the buffer has been filled
    xSemaphoreGive(tail_sem);
    xSemaphoreGive(mutex);
  }

  // Delete self task
  vTaskDelete(NULL);
  Serial.println("Task deleted");
}

// Consumer: continuously read from shared buffer
void consumer(void *parameters) {

  int val;

  // Read from buffer
  while (1) {

    // Wait for at least one slot in buffer to be filled
    xSemaphoreTake(tail_sem, portMAX_DELAY);  

    // Lock critical section with a mutex
    xSemaphoreTake(mutex, portMAX_DELAY);
    
    // Critical section (accessing shared buffer and Serial)
    val = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    Serial.println(val);
    
    // Signal to producer thread that a slot in the buffer is free
    xSemaphoreGive(head_sem);
    xSemaphoreGive(mutex);
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  char task_name[12];
  
  // Configure Serial
  Serial.begin(9600);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Semaphore Alternate Solution---");

  // Create mutexes and semaphores before starting tasks
  bin_sem = xSemaphoreCreateBinary();
  head_sem = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE); 
  tail_sem = xSemaphoreCreateCounting(BUF_SIZE, 0);
  mutex = xSemaphoreCreateMutex();

  
  // Start producer tasks (wait for each to read argument)
  for (int i = 0; i < num_prod_tasks; i++) {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(producer,
                            task_name,
                            1024,
                            (void *)&i,
                            1,
                            NULL,
                            app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }

  // Start consumer tasks
  for (int i = 0; i < num_cons_tasks; i++) {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer,
                            task_name,
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  }

  // Notify that all tasks have been created
  Serial.println("All tasks created");
}

void loop() {
  
  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
