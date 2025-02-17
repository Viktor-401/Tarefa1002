#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>   
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"
#include "ws2818b_interface.c" // Interface criada pelo autor, para facilitar a utilização da matriz
#include "numbers.c" // Matrizes correspontentes aos números de 0 a 9
#include <math.h>
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.c"

// Declaração dos pinos
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BUTTON_A 5
#define BUTTON_B 6
#define MATRIZ_PIN 7
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// Função debounce, para retirar ruidos ao pressionar o botão
bool debounce();
// Callback da interrupção do botão
void button_press(uint gpio, uint32_t events); 
// Função para atualizar o display OLED, mostrando o estado atual dos leds e o caracter atual
void update_display();

PIO pio = pio0;
// Index das states machines
uint sm_blink = 0;
uint sm_ws = 1;
// Número mostrado na matriz
int displayed_number = 0;
bool green_led = false;
bool blue_led = false;
char input;
// Última vez que o botão foi pressionado, usado na função de debounce
absolute_time_t last_press = {0};

ssd1306_t display;

int main()
{
    stdio_init_all();
    // Inicialização do I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_init(&display, 128, 64, false, 0x3C, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_send_data(&display);

    // Limpa completamente a tela
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Inicialização dos botões e leds
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_init(GREEN_LED_PIN);
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    // Habilitação das interrupções acionadas pelos botões, ativam em borda de descida
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_press);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &button_press);

    // Inicialização do programa pio da matriz de leds
    uint offset_ws = pio_add_program(pio0, &ws2818b_program);
    ws2818b_program_init(pio, sm_ws, offset_ws, MATRIZ_PIN, 800000.f);

    // A matriz de leds é inicializada com todos os leds apagados
    matriz_clear(pio, sm_ws);

    // Loop principal
    while (true)
    {
        sleep_ms(10);
        update_display();
        // Recebe o input do usuário pelo stdio
        input = getchar();
        if (isdigit(input)) // se for um número, atualiza a matriz de leds
        {
             switch (atoi(&input))
            {
            case 0:
                matriz_update(pio, sm_ws, zero);
                break;
            case 1:
                matriz_update(pio, sm_ws, one);
                break;
            case 2:
                matriz_update(pio, sm_ws, two);
                break;
            case 3:
                matriz_update(pio, sm_ws, three);
                break;
            case 4: 
                matriz_update(pio, sm_ws, four);
                break;
            case 5:
                matriz_update(pio, sm_ws, five);
                break;
            case 6:
                matriz_update(pio, sm_ws, six);
                break;
            case 7:
                matriz_update(pio, sm_ws, seven);
                break;
            case 8:
                matriz_update(pio, sm_ws, eight);
                break;
            case 9:
                matriz_update(pio, sm_ws, nine);
                break;
            }
        }
        else // se não for um número, apaga a matriz
        {
            matriz_clear(pio, sm_ws); 
        }
        update_display(); // atualiza o display independente do input
    }
}
//Compara o tempo atual com o tempo em que o botão foi pressionado pela última vez e determina se o sinal deve ser aceito
bool debounce()
{
    absolute_time_t time = get_absolute_time();

    if (absolute_time_diff_us(last_press, time) >= 200000) 
    {
        last_press = time;
        return true;
    }
    return false;
}

//Callback da interrupção do botão
void button_press(uint gpio, uint32_t events)
{
    //Determina qual botão foi pressionado, e então realiza a ação correspondente
    if (gpio == BUTTON_A && debounce())
    {
        // Troca o estado do led verde
        green_led = !green_led;
        gpio_put(GREEN_LED_PIN, green_led);
        if (green_led)
        {
            printf("Green led on\n");
        }
        else
        {
            printf("Green led off\n");
        }
    }
    else if (gpio == BUTTON_B && debounce())
    {
        // Troca o estado do led azul
        blue_led = !blue_led;  
        gpio_put(BLUE_LED_PIN, blue_led);
        if (blue_led)
        {
            printf("Blue led on\n");
        }
        else
        {
            printf("Blue led off\n");
        }
    }
    update_display(); // atualiza o display após a ação, para refletir o estado dos leds
}

// Função para atualizar o display OLED, mostrando o estado atual dos leds e o caracter atual
void update_display()
{
    // Limpa o display
    ssd1306_fill(&display, false);
    // Atualiza o display com o caracter atual e o estado dos leds
    ssd1306_draw_string(&display,"Caracter ", 0, 0);
    ssd1306_draw_char(&display,input, 72, 0);

    ssd1306_draw_string(&display,"Led Azul ", 0, 9);
    if (blue_led)
    {
        ssd1306_draw_string(&display,"ON", 72, 9);
    }
    else
    {
        ssd1306_draw_string(&display,"OFF", 72, 9);
    }

    ssd1306_draw_string(&display,"Led Verde ", 0, 18);
    if (green_led)
    {
        ssd1306_draw_string(&display,"ON", 72, 18);
    }
    else
    {
        ssd1306_draw_string(&display,"OFF", 72, 18);
    }
    // Envia os dados para o display
    ssd1306_send_data(&display);
}