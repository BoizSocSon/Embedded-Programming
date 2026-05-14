#include "stm32f401xe.h"
#include "stdio.h"

/*
 * Bài: Đo khoảng cách bằng cảm biến siêu âm HC-SR04 và gửi kết quả qua UART
 * Mô tả ngắn:
 *  - Gởi xung TRIG (PA1) 10µs để bắt đầu đo
 *  - Đọc xung ECHO (PA0) bằng Input Capture (TIM2_CH1) để lấy thời gian xung
 *  - Tính khoảng cách: distance(cm) = capture_time(µs) * 0.017
 *  - Gởi kết quả qua USART2 (PA2 TX)
 *
 * Các chân dùng:
 *  - PA1: TRIG (output)
 *  - PA0: ECHO (input capture TIM2_CH1)
 *  - PA2: USART2 TX
 */

void init_GPIO();
void TIM2_InputCapture_Init();
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void init_USART2();
void USART2_SendString(const char *str);
void USART2_Send(float value);

// Các biến dùng trong ISR và vòng lặp chính
volatile uint8_t complete_flag = 0;   // 1 khi đã đo xong
volatile uint32_t capture_time = 0;   // thời gian xung ECHO (đơn vị: microsecond tương đối với cấu hình prescaler)
volatile uint8_t edge_flag = 0;       // 0: chờ cạnh lên, 1: đã bắt cạnh lên, chờ cạnh xuống


int main(){
	init_GPIO();                 // Cấu hình các chân PA0/PA1/PA2
	TIM2_InputCapture_Init();    // Cấu hình TIM2 cho Input Capture trên PA0
	init_USART2();               // Cấu hình USART2 để gửi dữ liệu
	USART2_SendString("\r\n--- SYSTEM START ---\r\n");

	while(1){
		// Bắt đầu một lần đo: reset cờ hoàn thành
		complete_flag = 0;

		// Gửi xung TRIG 10µs lên PA1 để cảm biến phát sóng siêu âm
		GPIOA->ODR |= 1 << 1;    // PA1 = 1
		delay_us(10);            // 10 microsecond
		GPIOA->ODR &= ~(1 << 1); // PA1 = 0

		// Chờ ISR cập nhật complete_flag hoặc timeout để tránh treo
		uint32_t timeout = 50000;
		while(complete_flag == 0 && timeout > 0){
			timeout--;
		}

		// Nếu ISR đã đo xong, tính khoảng cách và gửi qua UART
		if(complete_flag == 1){
			float distance = 0.017 * capture_time; // cm
			USART2_Send(distance);
		}
		else{
			// Không nhận được ECHO hoặc vượt quá khoảng cách
			USART2_SendString("Out of range!!!\r\n");
		}

		// Chờ 2 giây trước khi đo lại
		delay_ms(2000);
	}
}

void init_USART2(){
	// Bật clock cho USART2 và cấu hình transmit
	RCC->APB1ENR |= 1 << 17;    // USART2EN = 1

	// Set baud rate (ví dụ: 115200)
	USART2->BRR = 0x0683;

	// Bật transmitter và UART
	USART2->CR1 |= 1 << 3;      // TE = 1
	USART2->CR1 |= 1 << 13;     // UE = 1
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
void USART2_Send(float value){
	char buffer[25];
	int intPart = (int) value;
	int fracPart = (int)((value - intPart)*100);
	sprintf(buffer, "Distance: %d.%d cm\r\n", intPart, fracPart);
	USART2_SendString(buffer);
}

void init_GPIO(){
	// Bật clock cho GPIOA
	RCC->AHB1ENR |= 1 << 0;

	// PA0: ECHO - Alternate Function (TIM2_CH1)
	GPIOA->MODER |= 1 << 1;    // MODER0 = 10
	GPIOA->AFR[0] |= 1 << 0;   // AF1

	// PA1: TRIG - General purpose output
	GPIOA->MODER |= 1 << 2;    // MODER1 = 01

	// PA2: USART2 TX - Alternate Function AF7
	GPIOA->MODER |= 1 << 5;    // MODER2 = 10
	GPIOA->AFR[0] |= 7 << 8;   // AF7 for USART2
}

void TIM2_InputCapture_Init(){
	// Bật clock cho TIM2
	RCC->APB1ENR |= 1 << 0;    // TIM2EN = 1

	// Prescaler và ARR lớn để đo được thời gian ECHO
	TIM2->PSC = 16 - 1;        // chia clock để có tick phù hợp
	TIM2->ARR = 65535 - 1;     // max count
	TIM2->CNT = 0;

	// Cấu hình channel 1 làm Input Capture (CC1S = 01)
	TIM2->CCMR1 |= 1 << 0;
	// Bắt cạnh lên mặc định
	TIM2->CCER &= ~(1 << 1);

	// Kích hoạt ngắt capture/compare và NVIC
	TIM2->DIER |= 1 << 1;      // CC1IE
	NVIC_EnableIRQ(TIM2_IRQn);

	// Kích hoạt capture cho channel 1 và start timer
	TIM2->CCER |= 1 << 0;      // CC1E
	TIM2->CR1 |= 1 << 0;       // CEN
}

void TIM2_IRQHandler(){
	// Nếu có ngắt Capture/Compare channel 1
	if(TIM2->SR & (1 << 1)){
		TIM2->SR &= ~(1 << 1); // xóa cờ

		if(edge_flag == 0){
			// Bắt cạnh lên: reset counter để bắt đầu đo
			TIM2->CNT = 0;
			// Chuyển sang bắt cạnh xuống
			TIM2->CCER &= ~(1 << 0);
			TIM2->CCER |= 1 << 1;  // bật bắt falling
			TIM2->CCER |= 1 << 0;  // enable
			edge_flag = 1;
		}
		else{
			// Bắt cạnh xuống: lưu giá trị CCR1 = thời gian xung
			capture_time = TIM2->CCR1;
			complete_flag = 1;    // báo main() đã có kết quả
			edge_flag = 0;
			// Reset lại để lần sau bắt cạnh lên
			TIM2->CCER &= ~(1 << 0);
			TIM2->CCER &= ~(1 << 1);
			TIM2->CCER |= 1 << 0;
		}
	}
}

void delay_us(uint32_t us){
	// Dùng TIM3 tạo delay chính xác (microsecond)
	RCC->APB1ENR |= 1 << 1;    // TIM3EN
	TIM3->PSC = 16 - 1;
	TIM3->ARR = us - 1;
	TIM3->CNT = 0;
	TIM3->CR1 |= 1 << 0;       // start
	TIM3->SR &= ~(1 << 0);
	while(!(TIM3->SR & 1 << 0));
	TIM3->SR &= ~(1 << 0);
	TIM3->CR1 &= ~(1 << 0);    // stop
}

void delay_ms(uint32_t ms){
	// Dùng TIM4 tạo delay millisecond
	RCC->APB1ENR |= 1 << 2;    // TIM4EN
	TIM4->PSC = 16000 - 1;
	TIM4->ARR = ms - 1;
	TIM4->CNT = 0;
	TIM4->CR1 |= 1 << 0;       // start
	TIM4->SR &= ~(1 << 0);
	while(!(TIM4->SR & 1 << 0));
	TIM4->SR &= ~(1 << 0);
	TIM4->CR1 &= ~(1 << 0);    // stop
}


