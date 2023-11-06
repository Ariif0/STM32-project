#include "main.h"


I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

static void MX_I2C1_Init(void);

// Definisi alamat sensor INA219
#define INA219_ADDRESS (0x40 << 1)

// Daftar Register INA219
#define INA219_REG_CONFIG         (0x00)
#define INA219_REG_SHUNTVOLTAGE   (0x01)
#define INA219_REG_BUSVOLTAGE     (0x02)
#define INA219_REG_POWER          (0x03)
#define INA219_REG_CURRENT        (0x04)
#define INA219_REG_CALIBRATION    (0x05)
#define INA219_CONFIG_RESET       (0x8000)

//kalibrasi 32v_2A
#define INA219_CONFIG_BVOLTAGERANGE_32V         (0x2000) // Rentang 0-32V
#define INA219_CONFIG_GAIN_8_320MV              (0x1800) // Penguatan 8, Rentang 320mV
#define INA219_CONFIG_BADCRES_12BIT             (0x0180) // Resolusi ADC 12-bit untuk tegangan bus
#define INA219_CONFIG_SADCRES_12BIT_1S_532US    (0x0018) // 1 x 12-bit sampel shunt, Resolusi ADC untuk arus shunt
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS (0x0007)


uint16_t Read16(I2C_HandleTypeDef *i2c, uint8_t Register) {
    uint8_t data[2];
    HAL_I2C_Mem_Read(i2c, INA219_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
    return ((data[0] << 8) | data[1]);
}


void Write16(I2C_HandleTypeDef *i2c, uint8_t Register, uint16_t Value) {
    uint8_t data[2];
    data[0] = (uint8_t)(Value >> 8);
    data[1] = (uint8_t)(Value & 0xFF);
    HAL_I2C_Mem_Write(i2c, INA219_ADDRESS, Register, I2C_MEMADD_SIZE_8BIT, data, 2, 1000);
}

// Fungsi untuk mereset INA219
void INA219_Reset(I2C_HandleTypeDef *i2c) {
    Write16(i2c, INA219_REG_CONFIG, INA219_CONFIG_RESET);
    HAL_Delay(1);
}

// Fungsi untuk mengatur nilai kalibrasi INA219
void INA219_setCalibration(I2C_HandleTypeDef *i2c, uint16_t CalibrationData) {
    Write16(i2c, INA219_REG_CALIBRATION, CalibrationData);
}

// Fungsi untuk mengatur konfigurasi INA219
void INA219_setConfig(I2C_HandleTypeDef *i2c, uint16_t Config) {
    Write16(i2c, INA219_REG_CONFIG, Config);
}

// Fungsi untuk menginisialisasi INA219
uint8_t INA219_Init(I2C_HandleTypeDef *i2c) {
    INA219_Reset(i2c);

    // Mengkonfigurasi register kalibrasi dari IC INA219 dengan nilai yang sesuai dengan shunt resistor
    // Nilai ini dapat dihitung dengan rumus: Cal = 0.04096 / (Current_LSB * RSHUNT)
    // Current_LSB adalah resolusi arus yang diinginkan,  0.1 mA = 0.0001 A
    // RSHUNT adalah nilai shunt resistor, 0.1 ohm
    INA219_setCalibration(i2c, 4096);

    INA219_setConfig(i2c, INA219_CONFIG_BVOLTAGERANGE_32V |
            INA219_CONFIG_GAIN_8_320MV |
            INA219_CONFIG_BADCRES_12BIT |
            INA219_CONFIG_SADCRES_12BIT_1S_532US |
            INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS);

}

float INA219_BusVoltage_raw(I2C_HandleTypeDef *i2c) {
    uint16_t result = Read16(i2c, INA219_REG_BUSVOLTAGE);
    return ((result >> 3) * 4);
}


float INA219_getCurrent_raw(I2C_HandleTypeDef *i2c) {
	uint16_t ina219_currentDivider_mA = 10;
    int16_t result = Read16(i2c, INA219_REG_CURRENT);
    result /= ina219_currentDivider_mA;
    return (result);
}


float busVoltage = 0;
float shuntCurrent = 0;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    INA219_Init(&hi2c1);
    unsigned long readSensor = 0;

    while (1) {
        if (HAL_GetTick() >= readSensor) {
            readSensor = HAL_GetTick() + 500;
            busVoltage = INA219_BusVoltage_raw(&hi2c1) * 0.001;
            shuntCurrent = INA219_getCurrent_raw(&hi2c1);

        }
    }
}




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
