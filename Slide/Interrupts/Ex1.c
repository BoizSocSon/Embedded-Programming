// Exercise 1: Write a program to toggle green led (on PA5) using user button (on PC13) on 
// NUCLEO – F401RE board

#include "stm32f4xx_hal.h"

void init_led();
void init_button();
void init_interrupt();

void main()
{
    init_led();
    init_button();
    init_interrupt();

    NVIC_EnableIRQ(EXTI15_10_IRQn); //Enable interrupt for EXTI Line from 10 to 15

    while(1);
}

void init_led()
{
    RCC->AHB1ENR |= 1;              //Enable clock to port A
    GPIOA->MODER |= 1 << 10;        //Configure PA5 as output
    GPIOA->PUPDR |= 1 << 10;        //Configure pull up register for PA5
}

void init_button()
{
    RCC->AHB1ENR |= 1 << 2;         //Enable clock to port C
    GPIOC->MODER &= ~0xC000000;     //Configure PC13 as input
}

void init_interrupt()
{
    RCC->APB2ENR |= 1 << 14;        //Enable System configuration controller clock (using RCC_APB2ENR register)
    SYSCFG->EXTICR[3] |= 1 << 5;    //Choose PC13 for EXTI13 line interrupt (using SYSCFG->EXTICR[3] register, equivalent to PC13)
    EXTI->IMR |= 1 << 13;           //Interrupt request from line 13 is not masked (using EXTI_IMR register)

    EXTI->FTSR |= 1 << 13;          //Falling edge trigger enable for line 13 (using EXTI_FTSR register)
}

void EXTI15_10_IRQHandler()
{
    uint32_t previous_state = (GPIOA->ODR >> 5) & 0x1; // Read previous of green led on PA5
    if (previous_state == 0)
    {
        GPIOA->ODR |= 1 << 5;       // Turn on led
    }
    else
    {
        GPIOA->ODR &= ~(1 << 5);    // Turn off led
    }
    EXTI->PR |= 1 << 13;            // Clear pending bit for EXTI13 (using EXTI_PR register)
}