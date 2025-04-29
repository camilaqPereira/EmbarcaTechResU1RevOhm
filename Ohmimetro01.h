#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 9890;   // Resistor de 10k ohm (empirico)
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.28;     // Tensão de referência do ADC (empirico)
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6

/* Valores da serie E24*/
float e24_lookup[] = {1.0f, 1.1f, 1.2f, 1.3f, 1.5f, 1.6f, 1.8f, 2.0f, 2.2f, 2.4f,
    2.7f, 3.0f, 3.3f, 3.6f, 3.9f, 4.3f, 4.7f, 5.1f, 5.6f, 6.8f,
    7.5f, 8.2f, 9.1f};

    /* Tabela de consulta para as cores dos resistores*/
char faixas[10][10] = {"Preto", "Marrom", "Vermelho", "Laranja", "Amarelo",
    "Verde", "Azul", "Roxo", "Cinza", "Branco"};
    
void find_e24_value(float R_x, char *str_e24_value, char *faixa1, char *faixa2, char *str_multiplicador);
  