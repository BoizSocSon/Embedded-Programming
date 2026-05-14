#include "stm32f4xx_hal.h"

int main(void)
{
	RCC->AHB1ENR |= 1 << 0;   // 1. Enable clock to IO Port A (Using RCC AHB1ENR register)
	GPIOA->MODER &= ~(3 << (2 * 5));
	GPIOA->MODER |= (1 << (2 * 5));       // 2. Configure I/O direction mode on PA5 to general output mode (Using GPIOx_MODER register)
	while(1)
	{
							 //3. Write 1 to pin PA5 (Using GPIOx_ODR register or GPIOx_BSRR register to turn led on)
		for (int i = 0; i < 300000; i++);
		GPIOA->BSRR = (1 << 5);
							 // 4. Write 0 to pin PA5 (Using GPIOx_ODR register or GPIOx_BSRR register to turn led off)
		for (int i = 0; i < 300000; i++);
		GPIOA->BSRR = (1 << (5 + 16));
	}

}


#include "stm32f4xx_hal.h"

int main(void)
{
	RCC->AHB1ENR |= 1 << 0;   // 1. Enable clock to IO Port A (Using RCC AHB1ENR register)
	GPIOA->MODER &= ~(3 << (2 * 5));
	GPIOA->MODER |= (1 << (2 * 5));       // 2. Configure I/O direction mode on PA5 to general output mode (Using GPIOx_MODER register)
	while(1)
	{
							 //3. Write 1 to pin PA5 (Using GPIOx_ODR register or GPIOx_BSRR register to turn led on)
		for (int i = 0; i < 3000000; i++);
		GPIOA->ODR |= (1 << 5);
							 // 4. Write 0 to pin PA5 (Using GPIOx_ODR register or GPIOx_BSRR register to turn led off)
		for (int i = 0; i < 3000000; i++);
		GPIOA->ODR &= ~(1 << 5);
	}

}
