// Bài tập 2: Viết hàm độ trễ sử dụng bộ đếm 

#include "stm32f401xe.h"

void Led_Init(void);
void delay_ms(uint32_t ms);

int main(void) {
    // Initialize GPIO for PA5
    Led_Init();

    while (1) {
        delay_ms(1000);
        GPIOA->ODR ^= (1 << 5); // Chớp tắt PA5
    }
}

void Led_Init(void) {
    // Bật xung nhịp GPIOA (sử dụng thanh ghi RCC->AHB1ENR)
    RCC->AHB1ENR |= (1U << 0);

    // Đặt PA5 là chế độ đầu ra đa năng (sử dụng thanh ghi GPIOA->MODER)
    // Xóa bit 11:10 và đặt thành 01 (chế độ đầu ra đa năng)
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |= (1U << (5 * 2));
}

void delay_ms(uint32_t ms) {
    // Bật xung nhịp Timer 2 (sử dụng thanh ghi RCC->APB1ENR)
    RCC->APB1ENR |= (1U << 0);

    // Đặt chia tần số để có tích tắc 1 ms (16 MHz / 16000 = 1 kHz) (sử dụng thanh ghi TIM2->PSC)
    TIM2->PSC = 16000 - 1;

    // Đặt giá trị tự động nạp lại cho độ trễ miligiây mong muốn (sử dụng thanh ghi TIM2->ARR)
    TIM2->ARR = ms - 1;

    // Bật sinh sự kiện cập nhật và bộ đếm (sử dụng thanh ghi TIM2->CR1)
    TIM2->EGR |= (1U << 0);  // Sinh sự kiện cập nhật để nạp giá trị PSC ngay lập tức
    TIM2->SR &= ~(1U << 0);  // Xóa cờ Cập nhật Ngắt (UIF)
    TIM2->CR1 |= (1U << 0);  // CEN: Bật bộ đếm

    // Thăm dò cờ sự kiện cập nhật
    // Chờ cho đến khi cờ sự kiện cập nhật (UIF) được đặt
    while (!(TIM2->SR & 0x1));

    // Xóa cờ sự kiện cập nhật (sử dụng thanh ghi TIM2->SR)
    TIM2->SR &= ~(1U << 0);
    
    // Tắt bộ đếm sau khi hoàn thành độ trễ
    TIM2->CR1 &= ~(1U << 0); 
}