#include "stm32f401xe.h"


//Đang bị vấn đề không in ra dữ liệu được nhập
void init_GPIO();
void init_USART2();
void USART2_SendChar(char c);
void USART2_SendString(const char *str);
void USART2_Send();
void ledChange();

volatile uint8_t data = 0;
volatile uint8_t flag = 0;

const char *startString = "Enter your number (0-7): \r\n";
const char *error = "Incorrect. Please try again (0-7): \r\n";

int main(){
	init_GPIO();
	init_USART2();
	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);

	USART2_SendString(startString);
	while(1){
		if(flag == 1){
			flag = 0;
			USART2_Send();
			if(data <= 7){
				ledChange();
			}
		}
	}
}

void init_GPIO(){
	RCC->AHB1ENR |= 1 << 0;

	//PA0, PA1, PA4 are output
	GPIOA->MODER |= 1 << 0;
	GPIOA->MODER |= 1 << 2;
	GPIOA->MODER |= 1 << 8;

	//PA2 is TX, PA3 is RX
	GPIOA->MODER |= 1 << 5;
	GPIOA->MODER |= 1 << 7;

	GPIOA->AFR[0] |= 7 << 8;
	GPIOA->AFR[0] |= 7 << 12;
}

void init_USART2(){
	RCC->APB1ENR |= 1 << 17;

	USART2->CR1 |= 1 << 5;

	USART2->BRR = 0x0683;
	USART2->CR1 |= 1 << 3;
	USART2->CR1 |= 1 << 2;
	USART2->CR1 |= 1 << 13;
}
void USART2_IRQHandler(){
	if(USART2->SR & (1 << 5)){
		data = USART2->DR - '0';
		flag = 1;
	}
}

void ledChange(){
	GPIOA->ODR &= ~(1 << 0 | 1 << 1 | 1 << 4);
	if(data & 1) GPIOA->ODR |= 1 << 0;
	if(data & 2) GPIOA->ODR |= 1 << 1;
	if(data & 4) GPIOA->ODR |= 1 << 4;
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

void USART2_Send(){
	if(data < 0 || data > 7){
		USART2_SendString(error);
	}
	else{
		char binary[6] = "000\r\n";
		if(data & 1) binary[2] = '1';
		if(data & 2) binary[1] = '1';
		if(data & 4) binary [0] = '1';
		USART2_SendString("Number: ");
		USART2_SendChar(data + '0');
		USART2_SendString("\r\n");
		char *tempString = "Your number is: ";
		USART2_SendString(tempString);
		USART2_SendString(binary);
		USART2_SendString(startString);
	}
}










#include "stm32f401xe.h"

void init_GPIO();
void init_USART2();

void USART2_SendChar(char c);
void USART2_SendString(const char *str);
void USART2_Send();

void ledChange();

volatile uint8_t data = 0;
volatile uint8_t flag = 0;

const char *startString = "Enter your number (0-7): \r\n";
const char *error = "Incorrect. Please try again (0-7): \r\n";

int main(){
	init_GPIO();   // Khởi tạo GPIO
	init_USART2(); // Khởi tạo USART2
	NVIC_SetPriority(USART2_IRQn, 1); // Đặt mức ưu tiên cho ngắt USART2
	NVIC_EnableIRQ(USART2_IRQn); // Cho phép USART2 interrupt trong NVIC
	USART2_SendString(startString); // Gửi chuỗi yêu cầu nhập số

	while(1){
		// Chỉ xử lý khi đã nhận được dữ liệu
		if(flag == 1){
			flag = 0;
			USART2_Send(); // In dữ liệu vừa nhập ra terminal

			if(data <= 7){
				ledChange(); // Hiển thị dữ liệu bằng LED nhị phân
			}
		}
	}
}

void init_GPIO(){
	RCC->AHB1ENR |= 1 << 0; // Bật clock cho GPIOA

	// ===== PA0, PA1, PA4 OUTPUT =====
	GPIOA->MODER &= ~(3 << 0);
	GPIOA->MODER |=  (1 << 0);
	GPIOA->MODER &= ~(3 << 2);
	GPIOA->MODER |=  (1 << 2);
	GPIOA->MODER &= ~(3 << 8);
	GPIOA->MODER |=  (1 << 8);
	// Các chân LED ở mode output

	// ===== PA2 TX =====
	GPIOA->MODER &= ~(3 << 4);
	GPIOA->MODER |=  (2 << 4);
	// Alternate Function mode

	// ===== PA3 RX =====
	GPIOA->MODER &= ~(3 << 6);
	GPIOA->MODER |=  (2 << 6);
	// Alternate Function mode

	GPIOA->AFR[0] &= ~(15 << 8);
	GPIOA->AFR[0] |=  (7 << 8);
	// PA2 chọn AF7 = USART2_TX

	GPIOA->AFR[0] &= ~(15 << 12);
	GPIOA->AFR[0] |=  (7 << 12);
	// PA3 chọn AF7 = USART2_RX
}

void init_USART2(){

	RCC->APB1ENR |= 1 << 17;
	// Bật clock USART2

	USART2->CR1 |= 1 << 5;
	// RXNEIE = 1
	// Cho phép ngắt khi nhận xong dữ liệu

	USART2->BRR = 0x0683;
	// Baudrate 9600 với clock 16MHz

	USART2->CR1 |= 1 << 3;
	// TE = 1
	// Enable transmitter

	USART2->CR1 |= 1 << 2;
	// RE = 1
	// Enable receiver

	USART2->CR1 |= 1 << 13;
	// UE = 1
	// Bật USART2
}



void USART2_IRQHandler(){
	// Kiểm tra RXNE = 1
	// Có dữ liệu mới nhận được
	if(USART2->SR & (1 << 5)){
		data = USART2->DR - '0';
		// Đọc ký tự nhận được
		// Chuyển ASCII -> số nguyên

		flag = 1; // Báo cho main biết đã có dữ liệu mới
	}
}



void ledChange(){
	// Xóa trạng thái cũ của LED
	GPIOA->ODR &= ~(1 << 0 | 1 << 1 | 1 << 4);

	// Hiển thị bit 0
	if(data & 1)
		GPIOA->ODR |= 1 << 0;

	// Hiển thị bit 1
	if(data & 2)
		GPIOA->ODR |= 1 << 1;

	// Hiển thị bit 2
	if(data & 4)
		GPIOA->ODR |= 1 << 4;
}

void USART2_SendChar(char c){

	while(!(USART2->SR & 1 << 7));
	// Chờ TXE = 1
	// Thanh ghi truyền trống

	USART2->DR = c; // Gửi ký tự
}

void USART2_SendString(const char *str){
	while(*str){
		USART2_SendChar(*str++); // Gửi từng ký tự trong chuỗi
	}
}

void USART2_Send(){
	if(data > 7){
		USART2_SendString(error);
		// Nếu ngoài khoảng 0-7 thì báo lỗi
	} else {
		char binary[6] = "000\r\n";

		// Chuỗi nhị phân mặc định
		if(data & 1)
			binary[2] = '1';
		if(data & 2)
			binary[1] = '1';
		if(data & 4)
			binary[0] = '1';

		USART2_SendString("Number: ");
		USART2_SendChar(data + '0');
		USART2_SendString("\r\n");
		USART2_SendString("Your number is: ");
		USART2_SendString(binary);
		USART2_SendString(startString);
	}
}