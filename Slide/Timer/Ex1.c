// Bài tập 1: Chớp tắt LED xanh (PA5) trên bo mạch NUCLEO-F401RE mỗi 1 giây (sử dụng bộ đếm để tạo độ trễ 1 giây). 

#include "stm32f401xe.h"

void Timer2_Init(void);
void Led_Init(void);

int main(void) {
    // Khởi tạo GPIO cho PA5
    Led_Init();

    // Khởi tạo Timer 2
    Timer2_Init();

    // Vòng lặp vô hạn
    while (1) {
        // Chờ cờ sự kiện cập nhật (UIF) được đặt
        if (TIM2->SR & 0x1) {
            TIM2->SR &= ~0x1;  // Xóa cờ sự kiện cập nhật (sử dụng thanh ghi TIM2->SR)
            GPIOA->ODR ^= (1 << 5);  // Chớp tắt PA5
        }
    }
}

void Led_Init(void) {
    // Bật xung nhịp GPIOA
    RCC->AHB1ENR |= 1 << 0;  // Đặt PA5 là chế độ đầu ra đa năng
    GPIOA->MODER |= (1 << 10); // Đặt chế độ thành đầu ra
}

void Timer2_Init(void) {
    // Bật xung nhịp Timer 2 (sử dụng thanh ghi RCC->APB1ENR)
    RCC->APB1ENR |= 1 << 0;

    // Đặt chia tần số để có tích tắc 1 ms (16 MHz / 16000 = 1 kHz) (sử dụng thanh ghi TIM2->PSC)
    TIM2->PSC = 16000 - 1;

    // Đặt giá trị tự động nạp lại cho chu kỳ 1 giây (1000 ms) (sử dụng thanh ghi TIM2->ARR)
    TIM2->ARR = 1000 - 1;

    // Bật sinh sự kiện cập nhật và bộ đếm (sử dụng thanh ghi TIM2->CR1)
    TIM2->CR1 |= 1 << 0;
}