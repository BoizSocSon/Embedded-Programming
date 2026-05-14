

#include "stm32f4xx.h"
#include <stdio.h>

// Hàm cấu hình UART2 (TX: PA2, RX: PA3)
void UART2_Init(void) {
    // 1. Bật clock cho GPIOA và UART2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // 2. Cấu hình chân PA2 (TX) và PA3 (RX) là Alternate Function
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2)));

    // Chọn AF7 cho USART2 trên PA2, PA3
    GPIOA->AFR[0] |= (7U << (2 * 4)) | (7U << (3 * 4));

    // 3. Cấu hình Baudrate (Giả sử PCLK1 = 42MHz, Baud = 9600)
    // UARTDIV = 42,000,000 / (16 * 9600) = 273.4375
    // Mantissa = 273 (0x111), Fraction = 0.4375 * 16 = 7
    USART2->BRR = (273U << 4) | 7U;

    // 4. Cho phép truyền, nhận và bật UART
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

// Hàm gửi 1 ký tự
void UART2_Write(char ch) {
    while (!(USART2->SR & USART_SR_TXE)); // Chờ đệm truyền trống
    USART2->DR = ch;
}

// Hàm nhận 1 ký tự
char UART2_Read(void) {
    while (!(USART2->SR & USART_SR_RXNE)); // Chờ dữ liệu đến
    return (char)USART2->DR;
}

// Hàm gửi chuỗi văn bản
void UART2_SendString(char *str) {
    while (*str) UART2_Write(*str++);
}

// Cấu hình 3 LED trên PA5, PA6, PA7
void GPIO_LED_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER |=  ((1U << (5 * 2)) | (1U << (6 * 2)) | (1U << (7 * 2)));
}

// Hiển thị số nhị phân lên LED
void Display_Binary(uint8_t num) {
    // Xóa trạng thái cũ của 3 chân PA5, PA6, PA7
    GPIOA->ODR &= ~(7U << 5); 
    // Ghi giá trị mới (dịch số num lên tương ứng vị trí chân PA5)
    GPIOA->ODR |= ((num & 0x07) << 5);
}

int main(void) {
    UART2_Init();
    GPIO_LED_Init();

    UART2_SendString("STM32 F401 Ready! Nhap so tu 0-7:\r\n");

    while (1) {
        char receivedChar = UART2_Read();
        
        // Chuyển ký tự ASCII sang số nguyên (ví dụ '5' -> 5)
        if (receivedChar >= '0' && receivedChar <= '7') {
            uint8_t number = receivedChar - '0';

            // 1. Gửi lại số vừa nhận để kiểm tra
            UART2_SendString("Da nhan: ");
            UART2_Write(receivedChar);
            UART2_SendString(" -> Dang hien thi nhi phan...\r\n");

            // 2. Chuyển đổi và hiển thị lên 3 LED
            Display_Binary(number);
        } else {
            UART2_SendString("Loi: Vui long chi nhap tu 0 den 7!\r\n");
        }
    }
}