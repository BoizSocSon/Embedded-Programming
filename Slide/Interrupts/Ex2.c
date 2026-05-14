// Exercise 2: Students make a circuit themselves on breadboard, consists of:
//  - 3 Button (using pull-up resistors)
//  - 3 Led
// - Resistors
// Write a program to test interrupt priority and mask interrupt on STM32F401RE microcontroller
//  + Button 1 has highest priority, button 2 has second priority and button 3 has third priority.
//  + Mask interrupt from button 3.


#include "stm32f4xx_hal.h"

// Khai báo prototype các hàm
void init_led(void);
void init_button(void);
void init_interrupt(void);

void main(void)
{
    init_led();
    init_button();
    init_interrupt();

    // Cấu hình ưu tiên ngắt (Priority)
    NVIC_SetPriority(EXTI0_IRQn, 0);       // Button 1 (PA0) - ưu tiên 1
    NVIC_SetPriority(EXTI1_IRQn, 1);       // Button 2 (PA1) - ưu tiên 2
    NVIC_SetPriority(EXTI9_5_IRQn, 2);     // Button 3 (PA7) - ưu tiên 3

    // Cho phép ngắt trong NVIC
    NVIC_EnableIRQ(EXTI0_IRQn);            // EXTI Line 0
    NVIC_EnableIRQ(EXTI1_IRQn);            // EXTI Line 1
    NVIC_EnableIRQ(EXTI9_5_IRQn);          // EXTI Line 7 (nằm trong cụm 9_5)

    while(1)
    {
        // Vòng lặp vô tận
    }
}

void init_led(void)
{
    RCC->AHB1ENR |= (1 << 0);              // Enable clock port A
    RCC->AHB1ENR |= (1 << 1);              // Enable clock port B

    // Configure PA12 (Red), PA11 (Green) as output
    GPIOA->MODER &= ~((3 << 24) | (3 << 22)); 
    GPIOA->MODER |=  ((1 << 24) | (1 << 22));

    // Configure PB12 (Yellow) as output
    GPIOB->MODER &= ~(3 << 24);
    GPIOB->MODER |=  (1 << 24);
}

void init_button(void)
{
    RCC->AHB1ENR |= (1 << 0);              // Enable clock port A (nếu chưa bật)
    
    // Configure PA0, PA1, PA7 as input (mặc định MODER là 00 - Input)
    GPIOA->MODER &= ~((3 << 0) | (3 << 2) | (3 << 14));
    
    // Kích hoạt Pull-up để nút nhấn hoạt động ổn định nếu nối xuống GND
    GPIOA->PUPDR |= ((1 << 0) | (1 << 2) | (1 << 14));
}

void init_interrupt(void)
{
    RCC->APB2ENR |= (1 << 14);             // Enable SYSCFG clock

    // EXTI0 và EXTI1 cho PA0, PA1 (SYSCFG_EXTICR1)
    SYSCFG->EXTICR[0] &= ~((0xF << 0) | (0xF << 4)); // Clear và chọn Port A cho Line 0, 1

    // EXTI7 cho PA7 (SYSCFG_EXTICR2)
    SYSCFG->EXTICR[1] &= ~(0xF << 12);     // Clear và chọn Port A cho Line 7

    // Cấu hình Masking: Cho phép ngắt từ Line 0, 1, 7
    EXTI->IMR |= (1 << 0) | (1 << 1) | (1 << 7); // Không che ngắt Line 0, 1, 7

    // Cấu hình Trigger cạnh xuống (Falling edge)
    EXTI->FTSR |= (1 << 0) | (1 << 1) | (1 << 7);
}

// --- Trình phục vụ ngắt (Handlers) ---

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1 << 0))               // Kiểm tra cờ ngắt Line 0
    {
        EXTI->PR |= (1 << 0);              // Clear pending bit (ghi 1 để xóa)
        
        GPIOA->ODR |= (1 << 12);           // Turn ON Red LED (PA12)
        for(int i = 0; i < 3000000; i++);  // Delay
        GPIOA->ODR &= ~(1 << 12);          // Turn OFF Red LED
    }
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1 << 1))               // Kiểm tra cờ ngắt Line 1
    {
        EXTI->PR |= (1 << 1);              // Clear pending bit
        
        GPIOA->ODR |= (1 << 11);           // Turn ON Green LED (PA11)
        for(int i = 0; i < 3000000; i++);
        GPIOA->ODR &= ~(1 << 11);          // Turn OFF Green LED
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1 << 7))               // Kiểm tra cờ ngắt Line 7
    {
        EXTI->PR |= (1 << 7);              // Clear pending bit
        
        GPIOB->ODR |= (1 << 12);           // Turn ON Yellow LED (PB12)
        for(int i = 0; i < 3000000; i++);
        GPIOB->ODR &= ~(1 << 12);          // Turn OFF Yellow LED
    }
}