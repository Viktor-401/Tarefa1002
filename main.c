#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>   
#include <ctype.h>
#include "pico/stdlib.h"
#include <math.h>
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.c"

// Declaração dos pinos
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BUTTON_A 5
#define MATRIZ_PIN 7
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON 22

// Função debounce, para retirar ruidos ao pressionar o botão
bool debounce();
// Callback da interrupção do botão
void button_press(uint gpio, uint32_t events); 
void draw_display_border();
void update_display();
void setup_pwm(int pin);

// Estado do led verde
bool green_led = false;
bool activate_leds = true;
int square_x = 0;
int square_y = 0;
int border_size = 1;
// Última vez que o botão foi pressionado, usado na função de debounce
absolute_time_t last_press = {0};

ssd1306_t display;

#define PERIOD 2000   // Período do PWM (valor máximo do contador)
#define DIVIDER_PWM 16 // Divisor fracional do clock para o PWM
int red_brightness = 0;
int blue_brightness = 0;

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
    gpio_init(JOYSTICK_BUTTON);
    gpio_init(GREEN_LED_PIN);
    gpio_init(BLUE_LED_PIN);
    gpio_init(RED_LED_PIN);

    // ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    //PWM
    setup_pwm(RED_LED_PIN);
    setup_pwm(BLUE_LED_PIN);

    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);

    gpio_pull_up(BUTTON_A);
    gpio_pull_up(JOYSTICK_BUTTON);

    // Habilitação das interrupções acionadas pelos botões, ativam em borda de descida
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_press);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_press);

    // Loop principal
    while (true)
    {
        adc_select_input(0);
        square_y = 32-(trunc((adc_read()-2030)/64));
        blue_brightness = trunc(abs(adc_read()-2030));
        blue_brightness = blue_brightness<40?0:blue_brightness;
        printf("Y: %d\n", adc_read());
        printf("Blue(0-2000): %d\n", blue_brightness);

        adc_select_input(1);
        square_x = 64+(trunc((adc_read()-2030)/32));
        red_brightness = trunc(abs(adc_read()-2030));
        red_brightness = red_brightness<150?0:red_brightness;
        printf("X: %d\n", adc_read());
        printf("red(0-2000): %d\n", red_brightness);

        update_display();
        if (activate_leds)
        {
            pwm_set_gpio_level(RED_LED_PIN, red_brightness);
            pwm_set_gpio_level(BLUE_LED_PIN, blue_brightness);
            gpio_put(GREEN_LED_PIN, green_led);
        }
        else
        {
            pwm_set_gpio_level(RED_LED_PIN, 0);
            pwm_set_gpio_level(BLUE_LED_PIN, 0);
            gpio_put(GREEN_LED_PIN, 0);
        }
        
        sleep_ms(10);
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
    if (debounce())
    {
        switch (gpio)
        {
            case BUTTON_A:
                activate_leds = !activate_leds;
                break;
            case JOYSTICK_BUTTON:
                if (activate_leds)
                {                
                    green_led = !green_led;
                    gpio_put(GREEN_LED_PIN, green_led);
                }
                border_size += 2;
                if (border_size > 7)
                {
                    border_size = 1;
                }
                break;
        }
    }
}

// Função para atualizar o display OLED, mostrando o estado atual dos leds e o caracter atual
void update_display()
{
    // Limpa o display
    ssd1306_fill(&display, false);

    
    ssd1306_rect(&display, square_y, square_x, 8, 8, true, true);
    draw_display_border();

    // Envia os dados para o display
    ssd1306_send_data(&display);
}

void draw_display_border()    
{
    ssd1306_rect(&display, 0, 0, 128, border_size, true, true); // horizontal top
    ssd1306_rect(&display, 64-border_size, 0, 128, border_size, true, true); // horizontal bottom
    ssd1306_rect(&display, 0, 0, border_size, 64, true, true); // vertical left
    ssd1306_rect(&display, 0, 128-border_size, border_size, 64, true, true); // vertical right
}

void setup_pwm(int pin)
{
    uint slice;
    gpio_set_function(pin, GPIO_FUNC_PWM); // Configura o pino do pin para função PWM
    slice = pwm_gpio_to_slice_num(pin);    // Obtém o slice do PWM associado ao pino do pin
    pwm_set_clkdiv(slice, DIVIDER_PWM);    // Define o divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);           // Configura o valor máximo do contador (período do PWM)
    pwm_set_gpio_level(pin, 0);            // Define o nível inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);          // Habilita o PWM no slice correspondente
}