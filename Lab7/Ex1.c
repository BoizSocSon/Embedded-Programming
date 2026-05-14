#include "stm32f401xe.h"

void USART2_Init(){
	RCC->AHB1ENR |= 1 << 0;
	RCC->APB1ENR |= 1 << 17;

	GPIOA->MODER |= 1 << 5;
	GPIOA->AFR[0] |= 7 << 8;

	USART2->BRR = 0x0683;
	USART2->CR1 |= 1 << 3;
	USART2->CR1 |= 1 << 13;
}

void USART2_SendChar(char c){
	while(!(USART2->SR & 1 << 7));
	USART2->DR = c;
}

void USART2_SendString(const char *str){
	while(*str){
		USART2_SendChar(*str++);
	}
}

int main(){
	USART2_Init();
	USART2_SendString("Hello word\r\n");
	while(1);
}







#include "stm32f401xe.h"

void USART2_Init(){

	RCC->AHB1ENR |= 1 << 0; // Bật clock cho GPIOA
	RCC->APB1ENR |= 1 << 17; // Bật clock cho USART2

	GPIOA->MODER |= 1 << 5;
	// PA2 chuyển sang Alternate Function mode
	// PA2 dùng làm chân TX của USART2

	GPIOA->AFR[0] |= 7 << 8;
	// Chọn AF7 cho PA2
	// AF7 tương ứng USART2

	USART2->BRR = 0x0683;
	// Thiết lập baudrate
	// Với clock 16MHz -> baudrate khoảng 9600

	USART2->CR1 |= 1 << 3;
	// TE = 1
	// Cho phép truyền dữ liệu (Transmit Enable)

	USART2->CR1 |= 1 << 13;
	// UE = 1
	// Bật USART2 hoạt động
}

void USART2_SendChar(char c){

	while(!(USART2->SR & 1 << 7));
	// Chờ TXE = 1
	// Nghĩa là thanh ghi truyền trống
	// USART đã sẵn sàng nhận byte mới
	USART2->DR = c;
	// Ghi ký tự vào Data Register
	// USART sẽ tự động truyền dữ liệu ra chân TX
}

void USART2_SendString(const char *str){

	while(*str){
		// Duyệt từng ký tự cho tới khi gặp '\0'
		USART2_SendChar(*str++); // Gửi từng ký tự rồi tăng con trỏ sang ký tự tiếp theo
	}
}

int main(){
	USART2_Init(); // Khởi tạo USART2
	USART2_SendString("Hello word\r\n");
	// Gửi chuỗi ra UART
	// \r\n để xuống dòng trên terminal
	while(1);
	// Vòng lặp vô hạn giữ chương trình hoạt động
}