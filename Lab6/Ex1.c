// Hiểu cách hoạt động của bộ đếm (Counter) và cờ tràn (Update Interrupt Flag - UIF).
// •	Yêu cầu: Viết hàm delay_ms sử dụng TIM2. Với các bước thực hiện như sau: 
// 1.	Cấu hình thanh ghi Prescaler (PSC) và Auto-Reload Register (ARR) sao cho Timer đếm mỗi 1s/ 1 lần.
// 2.	Trong hàm delay, sử dụng vòng lặp while để đợi cờ UIF trong thanh ghi SR lên mức 1.
// 3.	Nhấp nháy LED PA5 với chu kỳ 1 giây (sử dụng hàm delay vừa viết).
// 4.	Nhớ xóa cờ UIF bằng phần mềm sau mỗi lần đếm tràn.
// •	Thực hành: Từ đó, cấu hình tần số hoạt động của TIM2 với tần số dao động của hệ thống là 16 MHz. Thiết kế 1 bộ Upcounter Timer và bộ Downcounter Timer để cho đèn LED PA5 nháy 1s,2s

#include "stm32f4xx_hal.h"

void init_led();
void delay_ms_up(uint32_t ms);
void delay_ms_down(uint32_t ms);

int main(){
	init_led();
	while(1){
		GPIOA->ODR |= 1 << 5;
		delay_ms_up(1000);
		GPIOA->ODR &= ~(1 << 5);
		delay_ms_down(2000);
	}
}

void init_led(){
	RCC->AHB1ENR |= 1 << 0;
	GPIOA->MODER |= 1 << 10;
}

void delay_ms_up(uint32_t ms){
	RCC->APB1ENR |= 1 << 0;

	TIM2->PSC = 16000 - 1;
	TIM2->ARR = ms - 1;
	TIM2->CR1 &= ~(1 << 4);
	TIM2->CNT = 0;
	TIM2->CR1 |= 1 << 0;

	while(!(TIM2->SR & 1 << 0));
	TIM2->SR &= ~(1 << 0);

	TIM2->CR1 &= ~(1 << 0);
}

void delay_ms_down(uint32_t ms){
	RCC->APB1ENR |= 1 << 0;

	TIM2->PSC = 16000 - 1;
	TIM2->ARR = ms - 1;
	TIM2->CR1 |= 1 << 4;
	TIM2->CNT = ms - 1;
	TIM2->CR1 |= 1 << 0;

	while(!(TIM2->SR & 1 << 0));
	TIM2->SR &= ~(1 << 0);

	TIM2->CR1 &= ~(1 << 0);
}







#include "stm32f4xx_hal.h"

void init_led();
void delay_ms_up(uint32_t ms);
void delay_ms_down(uint32_t ms);

int main(){

	init_led(); // Khởi tạo chân PA5 làm output điều khiển LED

	while(1){

		GPIOA->ODR |= 1 << 5; // Ghi bit 1 vào PA5 -> LED sáng

		delay_ms_up(1000); // Delay 1000ms bằng timer đếm tiến

		GPIOA->ODR &= ~(1 << 5); // Xóa bit PA5 -> LED tắt

		delay_ms_down(2000); // Delay 2000ms bằng timer đếm lùi
	}
}

void init_led(){

	RCC->AHB1ENR |= 1 << 0; // Bật clock cho GPIOA

	GPIOA->MODER |= 1 << 10; // Set PA5 ở mode output (01)
}

void delay_ms_up(uint32_t ms){

	RCC->APB1ENR |= 1 << 0; // Bật clock cho Timer2

	TIM2->PSC = 16000 - 1; // Chia clock 16MHz xuống 1kHz -> mỗi tick = 1ms

	TIM2->ARR = ms - 1; // Timer sẽ đếm tới ms-1 rồi tràn

	TIM2->CR1 &= ~(1 << 4); // DIR = 0 -> chế độ đếm tiến (0 -> ARR)

	TIM2->CNT = 0; // Giá trị bắt đầu đếm = 0

	TIM2->CR1 |= 1 << 0; // CEN = 1 -> bật timer

	while(!(TIM2->SR & 1 << 0)); // Chờ tới khi UIF = 1 (timer tràn)

	TIM2->SR &= ~(1 << 0); // Xóa cờ UIF

	TIM2->CR1 &= ~(1 << 0); // Tắt timer
}

void delay_ms_down(uint32_t ms){

	RCC->APB1ENR |= 1 << 0; // Bật clock cho Timer2

	TIM2->PSC = 16000 - 1; // Chia clock xuống 1kHz -> mỗi tick = 1ms

	TIM2->ARR = ms - 1; // Giá trị giới hạn đếm

	TIM2->CR1 |= 1 << 4; // DIR = 1 -> chế độ đếm lùi (ARR -> 0)

	TIM2->CNT = ms - 1; // Nạp giá trị bắt đầu đếm

	TIM2->CR1 |= 1 << 0; // Bật timer

	while(!(TIM2->SR & 1 << 0)); // Chờ cờ UIF bật khi đếm xong

	TIM2->SR &= ~(1 << 0); // Xóa cờ UIF

	TIM2->CR1 &= ~(1 << 0); // Tắt timer
}