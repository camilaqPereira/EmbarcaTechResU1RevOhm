/*
 * Por: Wilton Lacerda Silva
 *    Ohmímetro utilizando o ADC da BitDogLab
 * 
 * ALTERADO POR: Camila Boa Morte
 * 
 * Neste exemplo, utilizamos o ADC do RP2040 para medir a resistência de um resistor
 * desconhecido, utilizando um divisor de tensão com dois resistores.
 * O resistor conhecido é de 10k ohm e o desconhecido é o que queremos medir.
 *
 */

#include "Ohmimetro01.h"

void gpio_irq_handler(uint gpio, uint32_t events) {
  reset_usb_boot(0, 0);
}

int main() {
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string
  char str_e24_value[5]; // Buffer para armazenar a string
  char faixa1[10];
  char faixa2[10];
  char str_multiplicador[10]; // Buffer para armazenar a string
  
  bool cor = true;

  while (true) {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++) {
      soma += adc_read();
      sleep_ms(1);
    }

    float media = soma / 500.0f;

    // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

    /* ==================== Adicao da chamada da funcao find_e24_value ===============*/

    find_e24_value(R_x, str_e24_value, faixa1, faixa2, str_multiplicador); // Encontra o valor E24 mais próximo
    
    /* ================================================= */

    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
    
    ssd1306_draw_string(&ssd, faixa1, 8, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, faixa2, 8, 16);  // Desenha uma string
    ssd1306_draw_string(&ssd, str_multiplicador, 8, 26); // Desenha uma string

    ssd1306_draw_string(&ssd, "E24", 24, 41);    // Desenha uma string
    ssd1306_draw_string(&ssd, str_e24_value, 16, 52); // Desenha uma string
    
    ssd1306_vline(&ssd, 88, 3, 37, cor);           // Desenha uma linha vertical

    ssd1306_draw_string(&ssd, " ADC", 90, 6);          // Desenha uma string
    ssd1306_draw_string(&ssd, str_x, 90, 24);           // Desenha uma string
    
    ssd1306_line(&ssd, 72, 37, 72, 60, cor);           // Desenha uma linha vertical
    
    ssd1306_draw_string(&ssd, "Resis.", 74, 41);    // Desenha uma string
    ssd1306_draw_string(&ssd, str_y, 74, 52);          // Desenha uma string
    
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
  }
}




/**
 * @brief Encontra o valor de resistor E24 mais próximo da resistência fornecida e o formata como uma string.
 * 
 * Esta função calcula o valor padrão de resistor E24 mais próximo do valor de resistência de entrada (R_x)
 * e formata o resultado em uma representação de string. Além disso, determina as faixas de cores
 * para os dois primeiros dígitos significativos e o multiplicador do resistor.
 * 
 * @param R_x O valor da resistência (em ohms) para o qual o valor E24 mais próximo será encontrado.
 * @param str_e24_value Ponteiro para uma string onde o valor do resistor E24 formatado será armazenado.
 * @param faixa1 Ponteiro para uma string onde o código de cor para o primeiro dígito significativo será armazenado.
 * @param faixa2 Ponteiro para uma string onde o código de cor para o segundo dígito significativo será armazenado.
 * @param str_multiplicador Ponteiro para uma string onde o código de cor para o multiplicador será armazenado.
 */
void find_e24_value(float R_x, char* str_e24_value, char *faixa1, char *faixa2, char* str_multiplicador){

  int e24_lookup_len = sizeof(e24_lookup) / sizeof(e24_lookup[0]);

  for(int k = 0; k < e24_lookup_len; k++){  
       
    for(int multiplicador = 1; multiplicador <= 5; multiplicador++){
      int int_multiplicador = (int) round(pow(10, multiplicador));
      int valor = e24_lookup[k] * int_multiplicador; // Multiplica o valor por 10^multiplicador
      float rel_error = fabs((valor - R_x) / valor) * 100.0f; // Erro relativo em porcentagem
    
      if (rel_error < 5.0f) { // Se o erro relativo for menor que 5%
        
        sprintf(str_e24_value, "%d", valor); // Converte o float em string
        snprintf(str_multiplicador, 10, "%s", faixas[multiplicador-1]); // Converte o float em string

        int aux = (int) e24_lookup[k];
        int aux2 = (int) round((e24_lookup[k] - aux) * 10);

        
        snprintf(faixa1, 10, "%s", faixas[aux]); // Converte o inteiro em string
        snprintf(faixa2, 10, "%s", faixas[aux2]); // Converte o inteiro em string
        
        return;
      
      }
    }
  }

  sprintf(str_e24_value, "%d", 0); // Converte o float em string
  snprintf(faixa1, 10, "%s", "N/A"); // Converte o inteiro em string
  snprintf(faixa2, 10, "%s", "N/A"); // Converte o inteiro em string
  snprintf(str_multiplicador, 10, "%s", "N/A"); // Converte o inteiro em string

  
}