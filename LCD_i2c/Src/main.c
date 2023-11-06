#include "main.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

// Definisi beberapa konstanta untuk kontrol LCD
#define RS_HIGH 0x01
#define RS_LOW 0x00
#define EN_HIGH 0x04
#define EN_LOW 0x00
#define BACKLIGHT 0x08

// Deklarasi fungsi-fungsi yang akan digunakan
void lcd_send_cmd(char cmd); // Deklarasi fungsi untuk mengirim perintah ke LCD
void lcd_send_string(char *str); // Deklarasi fungsi untuk mengirim string ke LCD
void lcd_send_data(char data); // Deklarasi fungsi untuk mengirim data karakter ke LCD
void lcd_init(void); // Deklarasi fungsi untuk menginisialisasi LCD

// Fungsi untuk mengirim perintah ke LCD
void lcd_send_cmd(char cmd)
{
    char data_u, data_l;
    uint8_t data_t[4];

    // Pisahkan byte atas dan bawah dari perintah
    data_u = cmd & 0xF0;
    data_l = (cmd << 4) & 0xF0;

    // Atur data untuk mengaktifkan dan menonaktifkan EN (Enable) serta RS (Register Select)
    data_t[0] = data_u | EN_HIGH | RS_LOW; // Aktifkan EN (Enable), nonaktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[1] = data_u | EN_LOW | RS_LOW; // Nonaktifkan EN (Enable), nonaktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[2] = data_l | EN_HIGH | RS_LOW; // Aktifkan EN (Enable), nonaktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[3] = data_l | EN_LOW | RS_LOW; // Nonaktifkan EN (Enable), nonaktifkan RS (Register Select)
    HAL_Delay(1);

    // Kirim data ke LCD melalui I2C
    HAL_I2C_Master_Transmit(&hi2c1, 0x4E, (uint8_t *)data_t, 4, 100);
    HAL_Delay(1);
}

// Fungsi untuk mengirim data ke LCD
void lcd_send_data(char data)
{
    char data_u, data_l;
    uint8_t data_t[4];

    // Pisahkan byte atas dan bawah dari data
    data_u = data & 0xF0;
    data_l = (data << 4) & 0xF0;

    // Atur data untuk mengaktifkan dan menonaktifkan EN (Enable) serta RS (Register Select)
    data_t[0] = data_u | EN_HIGH | RS_HIGH; // Aktifkan EN (Enable), aktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[1] = data_u | EN_LOW | RS_HIGH; // Nonaktifkan EN (Enable), aktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[2] = data_l | EN_HIGH | RS_HIGH; // Aktifkan EN (Enable), aktifkan RS (Register Select)
    HAL_Delay(1);
    data_t[3] = data_l | EN_LOW | RS_HIGH | BACKLIGHT; // Nonaktifkan EN (Enable), aktifkan RS (Register Select) dengan pencahayaan latar belakang
    HAL_Delay(1);

    // Kirim data ke LCD melalui I2C
    HAL_I2C_Master_Transmit(&hi2c1, 0x4E, (uint8_t *)data_t, 4, 100);
    HAL_Delay(1);
}

// Fungsi untuk menginisialisasi LCD
void lcd_init(void)
{
    HAL_Delay(100);
    lcd_send_cmd(0x02); // Mengirim perintah untuk mengatur kursor ke posisi awal
    HAL_Delay(1);
    lcd_send_cmd(0x28); // Mengatur mode 4-bit data, 2 baris, font 5x7
    HAL_Delay(1);
    lcd_send_cmd(0x0C); // Menyembunyikan kursor dan menonaktifkan kedipan kursor
    HAL_Delay(1);
    lcd_send_cmd(0x80); // Pindahkan kursor ke baris pertama dan kolom pertama
    HAL_Delay(1);
    lcd_send_cmd(0x01); // Bersihkan layar
    HAL_Delay(1);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}

uint16_t i = 0;
float fNilai;
char cNilai[16];

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    lcd_init();
    lcd_send_string("Hello world!");
    HAL_Delay(2000);

    while (1)
    {
        fNilai = fNilai + 0.25;
        sprintf(cNilai, "%0.3f", fNilai);
        lcd_send_cmd(0x01);
        lcd_send_string(cNilai);
        HAL_Delay(1000);
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
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
