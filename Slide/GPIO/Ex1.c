// Exercise 1: Write a program to blink green led on NUCLEO-F401RE board (using register)

#include "stm32f4xx_hal.h"

int main(void)
{
    // 1. Enable clock to IO Port A (Using RCC AHB1ENR register)
    RCC->AHB1ENR |= 1 << 0;

    // 2. Configure I/O direction mode on PA5 to general output mode (Using GPIOx_MODER register)
    // Mỗi pin chiếm 2 bit trong MODER. PA5 ứng với bit 10 và 11.
    // Cấu hình 01 cho Output mode: Clear bit 11 và Set bit 10.
    GPIOA->MODER &= ~(3 << 10); // Xóa bit 11:10
    GPIOA->MODER |= (1 << 10);  // Set bit 10

    while(1)
    {
        // 3. Write 1 to pin PA5 (Using GPIOx_ODR register to turn led on)
        GPIOA->ODR |= (1 << 5);
        
        for (int i = 0; i < 3000000; i++);

        // 4. Write 0 to pin PA5 (Using GPIOx_ODR register to turn led off)
        GPIOA->ODR &= ~(1 << 5);
        
        for (int i = 0; i < 3000000; i++);
    }
}