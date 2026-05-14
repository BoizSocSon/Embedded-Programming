// BÀI 2: ĐỊNH THỜI TIMER - CHẾ ĐỘ NGẮT (TRUNG BÌNH)
// •	Mục tiêu: Sử dụng ngắt để thực hiện tác vụ định kỳ mà không làm treo CPU.
// •	Yêu cầu: Sử dụng TIM3 để tạo ngắt mỗi 500ms.
// 1.	Tương tự như trong bài 1, viết 1 hàm delay_ms để sử dụng
// 2.	Cấu hình ngắt trong thanh ghi DIER và bật ngắt TIM3 trong NVIC.
// 3.	Trong hàm phục vụ ngắt TIM3_IRQHandler, thực hiện đảo trạng thái (toggle) LED PA5 với tần số 1s/1 lần  .
// 4.	Trong hàm main, thực hiện một vòng lặp nháy LED khác ở chân PB7 bằng với tốc độ nháy nhanh hay chậm tùy các bạn, khác trong hàm TIM3_IRQHandler. Quan sát xem LED PA5 có bị ảnh hưởng khi CPU đang thực hiện lệnh delay không.

#include "stm32f4xx_hal.h"

// Khai báo các hàm
void init_gpio(void);
void init_timer3_interrupt(void);
void delay_ms(uint32_t ms);

int main(void) {
    init_gpio();
    init_timer3_interrupt();

    while (1) {
        // Nháy LED PB7 trong vòng lặp main (tốc độ nhanh: 100ms)
        GPIOB->ODR ^= (1 << 7);
        delay_ms(100); 
        
        // Quan sát: LED PA5 vẫn sẽ nháy đều 1s/lần nhờ ngắt TIM3, 
        // không bị ảnh hưởng bởi hàm delay_ms(100) ở đây.
    }
}

void init_gpio(void) {
    // Bật clock cho GPIOA (bit 0) và GPIOB (bit 1)
  	RCC->AHB1ENR |= (1 << 0);
	RCC->AHB1ENR |= (1 << 1);

    // Cấu hình PA5 là Output (bit 10-11: 01)
	GPIOA->MODER &= ~(3 << (2 * 5));
	GPIOA->MODER |= (1 << (2 * 5));

    // Cấu hình PB7 là Output (bit 14-15: 01)
	GPIOB->MODER &= ~(3 << (2 * 7));
	GPIOB->MODER |= (1 << (2 * 7));

}

void init_timer3_interrupt(void) {
    // 1. Bật clock cho TIM3 (bit 1 của APB1ENR)
    RCC->APB1ENR |= (1 << 1);

    // 2. Cấu hình thời gian đếm 500ms (với clock hệ thống 16MHz)
    // Tần số sau bộ chia = 16MHz / 16000 = 1000Hz (1ms/tick)
    TIM3->PSC = 16000 - 1;
    // Đếm đến 500 nhịp = 500ms
    TIM3->ARR = 500 - 1;

    // 3. Cấu hình ngắt trong thanh ghi DIER (Update Interrupt Enable - bit 0)
    TIM3->DIER |= (1 << 0);

    // 4. Bật ngắt TIM3 trong NVIC
	NVIC_SetPriority(TIM3_IRQn, 1);
	NVIC_EnableIRQ(TIM3_IRQn);

    // 5. Bật Timer (CEN bit)
    TIM3->CR1 |= (1 << 0);
}

// 3. Hàm phục vụ ngắt TIM3_IRQHandler
void TIM3_IRQHandler(void) {
    // Kiểm tra cờ ngắt Update (UIF bit 0 trong SR)
    if (TIM3->SR & (1 << 0)) {
        // Xóa cờ ngắt bằng phần mềm (ghi 0 hoặc theo cơ chế rc_w1 tùy dòng, thường là ghi 0)
        TIM3->SR &= ~(1 << 0);

        // Đảo trạng thái LED PA5
        // Vì ngắt xảy ra mỗi 500ms, nên đảo trạng thái liên tục sẽ tạo chu kỳ 1s
        GPIOA->ODR ^= (1 << 5);
    }
}

// 1. Hàm delay_ms sử dụng TIM2
void delay_ms(uint32_t ms) {

    RCC->APB1ENR |= (1 << 0);

    TIM2->PSC = 16000 - 1;
    TIM2->ARR = ms - 1;

    TIM2->CR1 |= (1 << 0);

    while (!(TIM2->SR & (1 << 0)));

    TIM2->SR &= ~(1 << 0);

    TIM2->CR1 &= ~(1 << 0);
}