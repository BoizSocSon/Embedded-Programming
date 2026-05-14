// BÀI 5: OUTPUT COMPARE - TẠO XUNG PWM
// •	Mục tiêu: Hiểu cách dùng bộ so sánh để điều khiển công suất thiết bị.
// •	Yêu cầu: Cấu hình TIM2_CH1 (chân PA5) ở chế độ PWM Mode 1.
// o	Thiết lập tần số xung PWM là 1kHz.
// o	Trong vòng lặp while(1), liên tục thay đổi giá trị thanh ghi so sánh CCR1 từ 0% đến 100% ARR và ngược lại.
// o	Kết quả: Tạo hiệu ứng (LED sáng dần rồi tối dần) trên PA5 hoặc 1 LED được nối ở bên ngoài với chân bất kì


#include "stm32f4xx.h"

void TIM2_PWM_Init(void) {
    // 1. Bật clock cho GPIOA và TIM2
    // F4 dùng AHB1 cho GPIO và APB1 cho TIM2
    RCC->AHB1ENR |= (1 << 0);   // GPIOAEN = bit 0
    RCC->APB1ENR |= (1 << 0);   // TIM2EN  = bit 0

    // 2. Cấu hình chân PA5 (LED LD2 trên Nucleo F401RE)
    // Mode: Alternate Function (10)
    GPIOA->MODER &= ~(3 << (5 * 2));
    GPIOA->MODER |=  (2 << (5 * 2));
    
    // Tốc độ cao (11)
    GPIOA->OSPEEDR |= (3 << (5 * 2));

    // Chọn AF1 cho PA5 (TIM2_CH1)
    // AFR[0] tương ứng AFRL, AFR[1] tương ứng AFRH
    GPIOA->AFR[0] &= ~(15 << (5 * 4));
    GPIOA->AFR[0] |=  (1 << (5 * 4)); // AF1 = TIM2

    // 3. Thiết lập thông số thời gian (Giả định Clock hệ thống 84MHz)
    // Tần số ra = 84MHz / ((PSC+1) * (ARR+1)) = 1kHz
    TIM2->PSC = 83;            // 84MHz / (83+1) = 1MHz
    TIM2->ARR = 999;           // 1MHz / (999+1) = 1kHz

    // 4. Cấu hình Channel 1 ở PWM Mode 1
    TIM2->CCMR1 &= ~(7 << 4);     // Xóa bit
    TIM2->CCMR1 |=  (6 << 4);           // OC1M = 110 (PWM Mode 1)
    TIM2->CCMR1 |= (1 << 3);     // Cho phép Preload trên CCR1

    // 5. Cho phép xuất xung ra chân PA5 (CC1E)
    TIM2->CCER |= (1 << 0);

    // 6. Khởi tạo giá trị ban đầu và cho phép Timer hoạt động
    TIM2->EGR |= (1 << 0);            // Cập nhật các thanh ghi shadow
    TIM2->CR1 |= (1 << 0);
}

void delay_ms(uint32_t ms) {
    // Delay đơn giản cho mục đích minh họa
    for (uint32_t i = 0; i < ms * 1000; i++) {
        __NOP();
    }
}

int main(void) {
    TIM2_PWM_Init();
    
    int duty = 0;
    int step = 10; // Bước nhảy

    while (1) {
        // Tăng độ sáng (Sáng dần)
        for (duty = 0; duty <= 1000; duty += step) {
            TIM2->CCR1 = duty;
            delay_ms(1); 
        }
        
        // Giảm độ sáng (Tối dần)
        for (duty = 1000; duty >= 0; duty -= step) {
            TIM2->CCR1 = duty;
            delay_ms(1); 
        }
    }
}