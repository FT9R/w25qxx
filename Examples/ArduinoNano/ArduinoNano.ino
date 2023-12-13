#include "w25qxx.h"

#define USER_LED_PIN 13
#define PAGE 1408

volatile uint32_t uwTick;
w25qxx_HandleTypeDef w25qxx_Handle;
const uint8_t bufferWrite[] = "Hello World!";
uint8_t bufferRead[sizeof(bufferWrite)] = { 0 };

void setup() {
  IO_Init();
  Timer2_Init();
  sei();
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1e5, MSBFIRST, SPI_MODE0));

  /* w25qxx demo begin */
  Serial.println("Interface link");
  w25qxx_Link(&w25qxx_Handle, w25qxx_SPI1_Receive, w25qxx_SPI1_Transmit, w25qxx_CS0_Set);

  Serial.println("Device initialization");
  w25qxx_Init(&w25qxx_Handle);

  Serial.println("Forcing status registers to default state");
  w25qxx_Handle.statusRegister = 0x00;
  w25qxx_WriteStatus(&w25qxx_Handle, 1, true);
  w25qxx_WriteStatus(&w25qxx_Handle, 2, true);
  w25qxx_WriteStatus(&w25qxx_Handle, 3, true);

  Serial.println("First approach to read");
  w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_ADDRESS(PAGE), true, false);
  switch (w25qxx_Handle.error) {
    case W25QXX_ERROR_NONE:
      if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0) {
        Serial.println("Data already exist at target page boundaries");
      }
      break;

    case W25QXX_ERROR_CHECKSUM:
      Serial.println("Target page probably contains corrupted data or erased");
      Serial.println("Checksum error reset");
      w25qxx_ResetError(&w25qxx_Handle);

      Serial.println("Whole chip erase");
      w25qxx_Erase(&w25qxx_Handle, W25QXX_CHIP_ERASE, 0, W25QXX_WAIT_BUSY);

      Serial.println("Page programming");
      w25qxx_Write(&w25qxx_Handle, bufferWrite, sizeof(bufferWrite), W25QXX_PAGE_ADDRESS(PAGE), true,
                   W25QXX_WAIT_BUSY);

      Serial.println("Second approach to read");
      w25qxx_Read(&w25qxx_Handle, bufferRead, sizeof(bufferRead), W25QXX_PAGE_ADDRESS(PAGE), true, false);
      if (memcmp(bufferRead, bufferWrite, sizeof(bufferRead)) == 0) {
        Serial.println("Writing process success");
      } else {
        Serial.println("Writing process failure");
        Error_Handler();
      }
      break;

    default:
      Error_Handler();
      break;
  }
}

void loop() {
}

void Timer2_Init() {
  TCCR2A = 1 << WGM21;  // CTC
  TCCR2B = 1 << CS22;   // clkT2S/64 (from prescaler)
  OCR2A = 250 - 1;
  TIMSK2 = 1 << OCIE2A;  // Timer/Counter2 Output Compare Match A Interrupt Enable
}

void IO_Init() {
  /* User LED */
  pinMode(USER_LED_PIN, OUTPUT);
  digitalWrite(USER_LED_PIN, LOW);

  /* SPI NSS */
  pinMode(CS0_PIN, OUTPUT);
  digitalWrite(CS0_PIN, HIGH);
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
  Serial.print("An error occured: ");
  switch (w25qxx_Handle.error) {
    case W25QXX_ERROR_NONE:
      break;

    case W25QXX_ERROR_STATUS_MISMATCH:
      Serial.println("status match");
      break;

    case W25QXX_ERROR_INITIALIZATION:
      Serial.println("initialization");
      break;

    case W25QXX_ERROR_ARGUMENT:
      Serial.println("argument");
      break;

    case W25QXX_ERROR_ADDRESS:
      Serial.println("address");
      break;

    case W25QXX_ERROR_SPI:
      Serial.println("spi");
      break;

    case W25QXX_ERROR_TIMEOUT:
      Serial.println("timeout");
      break;

    case W25QXX_ERROR_CHECKSUM:
      Serial.println("checksum");
      break;

    case W25QXX_ERROR_INSTRUCTION:
      Serial.println("instruction");
      break;
  }

  Serial.print("Last detected status: ");
  switch (w25qxx_Handle.status) {
    case W25QXX_STATUS_NOLINK:
      Serial.println("no link");
      break;

    case W25QXX_STATUS_RESET:
      Serial.println("reset");
      break;

    case W25QXX_STATUS_READY:
      Serial.println("ready");
      break;

    case W25QXX_STATUS_BUSY:
      Serial.println("busy");
      break;

    case W25QXX_STATUS_BUSY_INIT:
      Serial.println("busy init");
      break;

    case W25QXX_STATUS_BUSY_WRITE:
      Serial.println("busy write");
      break;

    case W25QXX_STATUS_BUSY_READ:
      Serial.println("busy read");
      break;

    case W25QXX_STATUS_BUSY_ERASE:
      Serial.println("busy erase");
      break;

    case W25QXX_STATUS_UNDEFINED:
      Serial.println("undefined");
      break;
  }

  while (1) {}
  /* USER CODE END Error_Handler_Debug */
}