// Exercise 2 : Write a program to turn on green led on NUCLEO-F401RE board when pressing user
// button and vice versa (using register)

#include "stm32f4xx_hal.h"

#define button_pressed 0

// Khai báo prototype các hàm
void init_led(void);
void init_button(void);

int main(void)
{
    init_led();    // Initialize led
    init_button(); // Initialize button

    // Reading status of button and turn on/off led based on status of button
    uint32_t read_status;
    
    while (1)
    {
        // 5. Đọc trạng thái chân PC13 bằng thanh ghi IDR
        // (Dịch phải 13 bit và mask với 0x1 để lấy giá trị chân PC13)
        read_status = (GPIOC->IDR >> 13) & 0x1; 

        if (read_status == button_pressed)
        {
            // Turn on green led (PA5) bằng thanh ghi ODR
            GPIOA->ODR |= (1 << 5);
        }
        else
        {
            // Turn off green led (PA5)
            GPIOA->ODR &= ~(1 << 5);
        }
    }
}

void init_led(void)
{
    // 1. Enable clock to IO Port A (Using RCC AHB1ENR register)
    RCC->AHB1ENR |= (1 << 0);

    // 2. Configure I/O direction mode on PA5 to general output mode (Using GPIOx_MODER register)
    // Clear bit 10-11 và set bit 10 thành 1 (01: Output mode)
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);
}

void init_button(void)
{
    // 3. Enable clock to IO Port C (Using RCC AHB1ENR register)
    RCC->AHB1ENR |= (1 << 2);

    // 4. Configure I/O direction mode on PC13 to input mode (Using GPIOx_MODER register)
    // Clear bit 26-27 (00: Input mode)
    GPIOC->MODER &= ~(3 << 26);
}