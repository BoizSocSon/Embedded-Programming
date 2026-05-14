// Bài tập: Tạo tín hiệu PWM trên chân PA5 (kết nối với LED xanh) của STM32F401RE để bật và tắt LED dần dần.
// Sử dụng TIM2 để tạo tín hiệu PWM trên PA5.
// Sử dụng TIM3 để tạo hàm độ trễ void delay_ms(int ms) nhằm tạo độ trễ ngắn giữa các mức tín hiệu PWM khác nhau
//       (tương ứng với các chu kỳ công việc khác nhau) để nhìn thấy rõ hơn hiệu ứng của
//      LED bật dần và tắt dần.


#include "stm32f401xe.h"

void GPIO_Init(void);
void TIM2_PWM_Init(void);
void delay_ms(uint32_t ms);

int main(void) {

    // Khởi tạo GPIO và Timer 2 cho PWM
    GPIO_Init();
    TIM2_PWM_Init();

    // Vòng lặp vô hạn để làm mất dần LED
    while (1) {
        // Tăng dần chu kỳ công việc
        for (int dutyCycle = 0; dutyCycle <= 1000; dutyCycle += 10) {
            TIM2->CCR1 = dutyCycle; // Đặt chu kỳ công việc (giá trị so sánh)
            delay_ms(10); // Độ trễ 10 ms để làm mất dần mịn
        }

        // Giảm dần chu kỳ công việc
        for (int dutyCycle = 1000; dutyCycle >= 0; dutyCycle -= 10) {
            TIM2->CCR1 = dutyCycle; // Đặt chu kỳ công việc (giá trị so sánh)
            delay_ms(10); // Độ trễ 10 ms để làm mất dần mịn
        }
    }
}

void GPIO_Init(void) {
    // Bật xung nhịp GPIOA (sử dụng thanh ghi RCC->AHB1ENR)
    RCC->AHB1ENR |= (1U << 0);

    // Đặt chế độ cho PA5 thành hàm xen kẽ (sử dụng thanh ghi GPIOA->MODER)
    GPIOA->MODER &= ~(3U << (5 * 2));
    GPIOA->MODER |= (2U << (5 * 2)); // 10: Chế độ hàm xen kẽ

    // Đặt hàm xen kẽ cho PA5 thành AF1 (TIM2_CH1) (sử dụng thanh ghi GPIOA->AFR[0])
    GPIOA->AFR[0] &= ~(0xFU << (5 * 4));
    GPIOA->AFR[0] |= (1U << (5 * 4)); // AF1
}

void TIM2_PWM_Init(void) {
    // Bật xung nhịp Timer 2 (sử dụng thanh ghi RCC->APB1ENR)
    RCC->APB1ENR |= (1U << 0);

    TIM2->PSC = 16 - 1; // Đặt chia tần số (sử dụng thanh ghi TIM2->PSC)

    // Đặt giá trị tự động nạp lại cho tần số PWM 1 kHz (chu kỳ 1ms) (sử dụng thanh ghi TIM2->ARR)
    TIM2->ARR = 1000 - 1;

    // Đặt thanh ghi so sánh đầu ra cho chu kỳ công việc ban đầu (0%) (sử dụng thanh ghi TIM2->CCR1)
    TIM2->CCR1 = 0;

    // Cấu hình chế độ PWM 1 trên Kênh 1 TIM2 (sử dụng thanh ghi TIM2->CCMR1)
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos); // 110: Chế độ PWM 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Bật tải trước

    // Bật đầu ra cho Kênh 1 TIM2 (sử dụng thanh ghi TIM2->CCER)
    TIM2->CCER |= (1U << 0); // CC1E: Bật đầu ra

    // Bật bộ đếm (sử dụng thanh ghi TIM2->CR1)
    TIM2->CR1 |= (1U << 0); // CEN: Bật bộ đếm
}

void delay_ms(uint32_t ms) {
    // Bật xung nhịp Timer 3 (sử dụng thanh ghi RCC->APB1ENR)
    RCC->APB1ENR |= (1U << 1); 

    // Đặt chia tần số để có tích tắc 1 ms (16 MHz / 16000 = 1 kHz) (sử dụng thanh ghi TIM3->PSC)
    TIM3->PSC = 16000 - 1; 

    // Đặt giá trị tự động nạp lại cho độ trễ miligiây mong muốn (sử dụng thanh ghi TIM3->ARR)
    TIM3->ARR = ms - 1; 

    // Bật sinh sự kiện cập nhật và bộ đếm (sử dụng thanh ghi TIM3->CR1)
    TIM3->EGR |= (1U << 0); // UG: Sinh sự kiện cập nhật
    TIM3->SR &= ~(1U << 0); // Xóa cờ UIF
    TIM3->CR1 |= (1U << 0); // CEN: Bật bộ đếm

    // Thăm dò cờ sự kiện cập nhật
    while (!(TIM3->SR & 0x1)); // Chờ cho đến khi cờ sự kiện cập nhật (UIF) được đặt
    
    // Xóa cờ sự kiện cập nhật (sử dụng thanh ghi TIM3->SR)
    TIM3->SR &= ~(1U << 0);
    TIM3->CR1 &= ~(1U << 0); // Tắt bộ đếm sau độ trễ
}