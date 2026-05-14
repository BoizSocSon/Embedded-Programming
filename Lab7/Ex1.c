#include "stm32f4xx.h"

void UART2_Config(void);
void UART2_SendChar(char c);
void UART2_SendString(char *str);

int main(void) {
    UART2_Config();
    
    while (1) {
        UART2_SendString("Hello, STM32!\r\n");
        // Delay đơn giản bằng vòng lặp
        for (int i = 0; i < 1000000; i++); 
    }
}

void UART2_Config(void) {
    // 1. Bật Clock cho GPIOA và UART2
    RCC->AHB1ENR |= (1 << 0);       // GPIOA EN
    RCC->APB1ENR |= (1 << 17);      // UART2 EN

    // 2. Cấu hình chân PA2 (TX) và PA3 (RX) là Alternate Function
    GPIOA->MODER &= ~((3 << 4) | (3 << 6)); // Clear bit
    GPIOA->MODER |=  ((2 << 4) | (2 << 6)); // Set mode AF (10)

    // 3. Chọn AF7 (UART2) cho PA2 và PA3 trong thanh ghi AFR
    // AFR[0] tương ứng AFRL, AFR[1] tương ứng AFRH
    GPIOA->AFR[0] |= (7 << 8);  // PA2 -> AF7
    GPIOA->AFR[0] |= (7 << 12); // PA3 -> AF7

    // 4. Cấu hình Baud rate = 9600
    // Giả sử PCLK1 = 16MHz (mặc định HSI), Over sampling = 16
    // USARTDIV = 16.000.000 / (16 * 9600) = 104.166
    // Mantissa = 104 (0x68), Fraction = 0.166 * 16 = 2.6 ~ 3 (0x3)
    USART2->BRR = (104 << 4) | 3;

    // 5. Cấu hình CR1: Bật UART, Bật TX, Bật RX, 8-bit data
    USART2->CR1 |= (1 << 13); // UE = 1 (UART Enable)
    USART2->CR1 |= (1 << 3);  // TE = 1 (Transmitter Enable)
    USART2->CR1 |= (1 << 2);  // RE = 1 (Receiver Enable)
    
    // CR2 mặc định là 1 Stop bit (00), không cần chỉnh
}

void UART2_SendChar(char c) {
    // Chờ cho đến khi thanh ghi truyền trống (TXE = 1)
    while (!(USART2->SR & (1 << 7)));
    USART2->DR = (c & 0xFF);
}

void UART2_SendString(char *str) {
    while (*str) {
        UART2_SendChar(*str++);
    }
}