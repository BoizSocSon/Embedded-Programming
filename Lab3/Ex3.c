// Cắm mạch trên breadboard để kết nối vi điều khiển STM32F401RE với 1 nút bấm và 1 LED. Thiết kế chương trình có chức năng sau:
// 1.	Mỗi lần người dùng nhấn và thả nút USER, bộ đếm sẽ tăng thêm 1.
// 2.	Giá trị bộ đếm được giới hạn trong khoảng 1 đến 5. Sau giá trị 5, lần nhấn tiếp theo sẽ quay lại 1.
// 3.	Sau mỗi lần nhấn hợp lệ, LED sẽ nhấp nháy số lần tương ứng với giá trị bộ đếm.
// Lần nhấn	Giá trị đếm	LED nhấp nháy
// 1	1	Nhấp nháy 1 lần
// 2	2	Nhấp nháy 2 lần
// 3	3	Nhấp nháy 3 lần
// 4	4	Nhấp nháy 4 lần
// 5	5	Nhấp nháy 5 lần

// Lưu ý: Chương trình phải nhận sự kiện nhấn nút hợp lệ (không bị lặp nhiều lần khi giữ nút). Sinh viên làm đến tối thiểu là 5 lần nhấn

#include "stm32f4xx.h"

uint8_t count = 0;

void delay(uint32_t count_val) {
    for (uint32_t i = 0; i < count_val * 400; i++) {
        __NOP();
    }
}

void blink_led(uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        GPIOA->BSRR = (1 << 11);
        delay(2000);
        GPIOA->BSRR = (1 << 27);
        delay(2000);
    }
}

int main(void) {
    RCC->AHB1ENR |= (1 << 0);
    RCC->AHB1ENR |= (1 << 1);

    GPIOA->MODER &= ~(3 << 22);
    GPIOA->MODER |=  (1 << 22);

    GPIOB->MODER &= ~(3 << 18);

    while (1) {
        if (!(GPIOB->IDR & (1 << 9))) {
            delay(20);

            if (!(GPIOB->IDR & (1 << 9))) {

                count++;

                if (count > 5) {
                    count = 1;
                }

                while (!(GPIOB->IDR & (1 << 9)));

                delay(20);

                blink_led(count);
            }
        }
    }
}












#include "stm32f4xx.h"

// Biến lưu giá trị bộ đếm (từ 1 đến 5)
uint8_t count = 0;

// Hàm delay đơn giản bằng vòng lặp
void delay(uint32_t count_val) {
    for (uint32_t i = 0; i < count_val * 400; i++) {
        __NOP();
    }
}

// Hàm thực hiện nhấp nháy LED n lần
void blink_led(uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        GPIOA->BSRR = (1 << 11); // Bật LED
        delay(2000);              // Chờ
        GPIOA->BSRR = (1 << 27); // Tắt LED (11 + 16 = 27)
        delay(2000);              // Chờ giữa các lần nháy
    }
}

int main(void) {
    /* 1. Cấu hình Clock */
    RCC->AHB1ENR |= (1 << 0); // Port A cho LED
    RCC->AHB1ENR |= (1 << 1); // Port B cho Nút nhấn

    /* 2. Cấu hình PA11 làm Output (LED) */
    GPIOA->MODER &= ~(3 << 22);
    GPIOA->MODER |=  (1 << 22);

    /* 3. Cấu hình PB9 làm Input (Nút nhấn) */
    GPIOB->MODER &= ~(3 << 18);

    while (1) {
        /* 4. Kiểm tra sự kiện nhấn nút (PB9 xuống mức 0) */
        if (!(GPIOB->IDR & (1 << 9))) {
            delay(20); // Chống dội (Debounce)
            if (!(GPIOB->IDR & (1 << 9))) {

                // Tăng bộ đếm và giới hạn từ 1 đến 5
                count++;
                if (count > 5) {
                    count = 1;
                }

                /* 5. Chờ người dùng thả nút mới thực hiện nháy LED */
                // Điều này giúp tránh việc LED nháy liên tục khi đang giữ nút
                while (!(GPIOB->IDR & (1 << 9)));
                delay(20); // Chống dội khi thả nút

                /* 6. Nháy LED tương ứng với giá trị bộ đếm */
                blink_led(count);
            }
        }
    }
}
