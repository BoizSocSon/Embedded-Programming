// Hiểu cách hoạt động của bộ đếm (Counter) và cờ tràn (Update Interrupt Flag - UIF).
// •	Yêu cầu: Viết hàm delay_ms sử dụng TIM2. Với các bước thực hiện như sau: 
// 1.	Cấu hình thanh ghi Prescaler (PSC) và Auto-Reload Register (ARR) sao cho Timer đếm mỗi 1s/ 1 lần.
// 2.	Trong hàm delay, sử dụng vòng lặp while để đợi cờ UIF trong thanh ghi SR lên mức 1.
// 3.	Nhấp nháy LED PA5 với chu kỳ 1 giây (sử dụng hàm delay vừa viết).
// 4.	Nhớ xóa cờ UIF bằng phần mềm sau mỗi lần đếm tràn.
// •	Thực hành: Từ đó, cấu hình tần số hoạt động của TIM2 với tần số dao động của hệ thống là 16 MHz. Thiết kế 1 bộ Upcounter Timer và bộ Downcounter Timer để cho đèn LED PA5 nháy 1s,2s

#include "stm32f4xx_hal.h"

void init_led_pa5(void);
void init_timer2(void);
void delay_ms_up(uint32_t ms);
void delay_ms_down(uint32_t ms);


int main(void) {
    init_led_pa5();
    init_timer2();

    while(1){
		GPIOA->ODR |= 1 << 5;
		delay_ms_up(1000);
		GPIOA->ODR &= ~(1 << 5);
		delay_ms_down(2000);
	}
}

void init_led_pa5(void) {
    // Bật clock Port A
    RCC->AHB1ENR |= (1 << 0);
    // Cấu hình PA5 là Output
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);
}

void init_timer2(void) {
    // 1. Cấu hình PSC và ARR
    // Bật clock cho TIM2
    RCC->APB1ENR |= (1 << 0);

    // Thiết lập PSC để có tần số đếm 1kHz (1ms mỗi tick)
    TIM2->PSC = 16000 - 1;

    // Thiết lập ARR là 1 để cờ UIF bật mỗi 1ms
    TIM2->ARR = 1 - 1;

    // Cho phép bộ đếm hoạt động (CEN bit)
    TIM2->CR1 |= (1 << 0);
}

void delay_ms_up(uint32_t ms){
	RCC->APB1ENR |= 1 << 0;

	TIM2->PSC = 16000 - 1;
	TIM2->ARR = ms - 1;
	TIM2->CR1 &= ~(1 << 4);
	TIM2->CNT = 0;
	TIM2->CR1 |= 1 << 0;

	while(!(TIM2->SR & 1 << 0));
	TIM2->SR &= ~(1 << 0);

	TIM2->CR1 &= ~(1 << 0);
}

void delay_ms_down(uint32_t ms){
	RCC->APB1ENR |= 1 << 0;

	TIM2->PSC = 16000 - 1;
	TIM2->ARR = ms - 1;
	TIM2->CR1 |= 1 << 4;
	TIM2->CNT = ms - 1;
	TIM2->CR1 |= 1 << 0;

	while(!(TIM2->SR & 1 << 0));
	TIM2->SR &= ~(1 << 0);

	TIM2->CR1 &= ~(1 << 0);
}
