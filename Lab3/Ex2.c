// Bài 2
// Bộ đếm Nhị phân (Binary Counter)
// Yêu cầu: Sử dụng 1 nút nhấn SW1 và 3 bóng LED nối vào các chân PB0, PB1, PB2.
// •	Ban đầu cả 3 LED đều tắt (Giá trị 000).
// •	Mỗi lần nhấn và thả SW1, trạng thái 3 LED sẽ hiển thị số đếm nhị phân tăng dần thêm 1 đơn vị.
// •	Ví dụ: 001 (LED 1 sáng) -> 010 (LED 2 sáng) -> 011 (LED 1, 2 sáng) -> ... -> 111 (Cả 3 sáng). Nhấn thêm lần nữa quay về 000.


#include "stm32f4xx.h"

int main() {
	RCC->AHB1ENR |= (1 << 1);

	GPIOB->MODER &= ~(3 << (2*0));
	GPIOB->MODER &= ~(3 << (2*1));
	GPIOB->MODER &= ~(3 << (2*2));
	GPIOB->MODER &= ~(3 << (2*9));

	GPIOB->MODER |= (1 << (2*0));
	GPIOB->MODER |= (1 << (2*1));
	GPIOB->MODER |= (1 << (2*2));

	int count = 0;

	while(1) {
		if (!(GPIOB->IDR & (1 << 9))) {
            count++;
            if (count > 7) count = 0;
            GPIOB->ODR &= ~(7 << 0);
            GPIOB->ODR |= (count << 0);
            while (!(GPIOB->IDR & (1 << 9)));
		}
	}
}
