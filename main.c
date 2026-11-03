// GEOLISTRIK 10 DATA - STM32F103C8T6
// Wenner-Schlumberger Array
// STM32 + Relay Module + ADS1115 + SD Card
// Output: Excel ready CSV

#include "main.h"
#include <stdio.h>
#include <math.h>

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

float AB_spacing[10] = {10, 15, 20, 30, 40, 50, 60, 75, 90, 100}; // meter
float data_rho[10];
float data_V[10];
float data_I[10];

float Calculate_K(float ab2, float mn2) {
    // Wenner: K = 2 * pi * a
    // Schlumberger: K = pi * (AB/2)^2 / MN
    return M_PI * (ab2*ab2 - mn2*mn2) / (2*mn2);
}

void Inject_Current(uint8_t ab_index) {
    // Kontrol relay untuk jarak AB
    // PB0-PB3 untuk relay AB
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (ab_index & 0x01));
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (ab_index & 0x02));
    // Delay stabilisasi 500ms
    HAL_Delay(500);
}

float Read_Voltage(void) {
    // Baca potensial MN via ADS1115 / ADC STM32
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint16_t adc = HAL_ADC_GetValue(&hadc1);
    float volt = (adc * 3.3 / 4095.0) * 10.0; // gain 10x
    return volt;
}

float Read_Current(void) {
    // Baca arus via ACS712 / shunt resistor
    float v_shunt = Read_Voltage();
    float current = v_shunt / 0.1; // shunt 0.1 ohm
    return current;
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_Init();
  
  printf("GEOLISTRIK 10 DATA START - Central Java\n");
  printf("AB/2,MN/2,V,I,Rho\n");
  
  for(int i=0; i<10; i++) {
    float ab2 = AB_spacing[i];
    float mn2 = ab2 / 5.0; // MN = AB/5 untuk Schlumberger
    
    Inject_Current(i);
    
    // Stacking 5x untuk noise reduction
    float V_sum=0, I_sum=0;
    for(int s=0; s<5; s++) {
      V_sum += Read_Voltage();
      I_sum += Read_Current();
      HAL_Delay(100);
    }
    float V_avg = V_sum / 5.0;
    float I_avg = I_sum / 5.0;
    
    float K = Calculate_K(ab2, mn2);
    float rho = K * V_avg / I_avg; // Ohm.m
    
    data_V[i] = V_avg;
    data_I[i] = I_avg;
    data_rho[i] = rho;
    
    printf("%.1f,%.1f,%.4f,%.4f,%.2f\n", ab2, mn2, V_avg, I_avg, rho);
    
    // Interpretasi langsung
    if(rho < 20) printf("-> Aquifer detected at %.0fm\n", ab2);
    else if(rho > 200) printf("-> Bedrock at %.0fm\n", ab2);
    
    HAL_Delay(1000);
  }
  
  printf("\n10 DATA DONE - Export to Excel\n");
  
  while(1) {
    // Idle, tunggu download CSV
    HAL_Delay(1000);
  }
}
