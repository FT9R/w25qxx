#include "w25qxx_Demo.h"

#define USER_LED_PIN 13

volatile uint32_t uwTick;

void setup() {
  IO_Init();
  Timer2_Init();
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1e5, MSBFIRST, SPI_MODE0));
  sei();
}

void loop() {
  if (w25qxx_Demo(w25qxx_Print))
    Error_Handler();
}

static void IO_Init(void) {
  /* User LED */
  pinMode(USER_LED_PIN, OUTPUT);
  digitalWrite(USER_LED_PIN, LOW);

  /* SPI NSS */
  pinMode(SPI1_CS0_PIN, OUTPUT);
  digitalWrite(SPI1_CS0_PIN, HIGH);
}

static void Timer2_Init(void) {
  SET_BIT(TCCR2A, 1 << WGM21);  // CTC
  SET_BIT(TCCR2B, 1 << CS22);   // clkT2S/64 (from prescaler)
  OCR2A = 250 - 1;
  SET_BIT(TIMSK2, 1 << OCIE2A);  // Timer/Counter2 Output Compare Match A Interrupt Enable
}

ISR(TIMER2_COMPA_vect) {
  ++uwTick;
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}