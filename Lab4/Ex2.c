// Cắm mạch trên breadboard để kết nối vi điều khiển STM32F401RE với 2 nút bấm (SW1, SW2) và 3 LED (LED 1, LED 2, LED 3). Thiết kế chương trình sử dụng 2 đường ngắt ngoài độc lập để điều khiển vị trí LED sáng (hiệu ứng dịch LED):
// 1.	Ban đầu khi khởi động, chỉ có LED 1 sáng, LED 2 và 3 tắt.
// 2.	Cấu hình SW1 và SW2 sinh ra ngắt ngoài khi nhấn (sườn xuống).
// 3.	Khi ngắt SW1 xảy ra: Vị trí LED sáng dịch chuyển sang trái 1 bước theo vòng tròn (LED 1 → LED 2 → LED 3 → vòng lại LED 1).
// 4.	Khi ngắt SW2 xảy ra: Vị trí LED sáng dịch chuyển sang phải 1 bước theo vòng tròn (LED 3 → LED 2 → LED 1 → vòng lại LED 3).
// 5.	Ràng buộc: Chỉ xử lý logic chuyển đổi trạng thái bật/tắt của các LED bên trong 2 hàm phục vụ ngắt. Vòng lặp while(1) trong chương trình chính chỉ dung để bật sáng LED1.


#include "main.h"

/* Khai báo các hàm */
void init_leds(void);
void init_buttons(void);
void init_interrupts(void);
void Update_LEDs(void);
void Error_Handler(void);

/* Biến lưu vị trí LED hiện tại (0: LED1, 1: LED2, 2: LED3) */
volatile int8_t led_pos = 0;

int main(void) {
    init_leds();
    init_buttons();
    init_interrupts();

    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);

    /* Trạng thái ban đầu: LED 1 sáng */
    Update_LEDs();

    while (1) {
        // Không cần xử lý gì, mọi thứ chạy bằng interrupt
    }
}

/* --- KHỞI TẠO LED (PB0, PB1, PB2) --- */
void init_leds(void)
{
    // Enable clock cho GPIOB
    RCC->AHB1ENR |= 1 << 1;

    // Set PB0, PB1, PB2 là output (MODER = 01)
    GPIOB->MODER &= ~(3 << (0 * 2));   // Clear PB0
    GPIOB->MODER |=  (1 << (0 * 2));   // Output mode PB0
    GPIOB->MODER &= ~(3 << (1 * 2));   // Clear PB1
    GPIOB->MODER |=  (1 << (1 * 2));   // Output mode PB1
    GPIOB->MODER &= ~(3 << (2 * 2));   // Clear PB2
    GPIOB->MODER |=  (1 << (2 * 2));   // Output mode PB2

    // Output push-pull
    GPIOB->OTYPER &= ~(1 << 0);
    GPIOB->OTYPER &= ~(1 << 1);
    GPIOB->OTYPER &= ~(1 << 2);

    // Không pull-up/pull-down
    GPIOB->PUPDR &= ~(3 << (0 * 2));
    GPIOB->PUPDR &= ~(3 << (1 * 2));
    GPIOB->PUPDR &= ~(3 << (2 * 2));
}

/* --- KHỞI TẠO BUTTON (PC2, PC3) --- */
void init_buttons(void)
{
    // Enable clock cho GPIOC
    RCC->AHB1ENR |= 1 << 2;

    // Set PC2, PC3 là input (00)
    GPIOC->MODER &= ~(3 << (2 * 2));
    GPIOC->MODER &= ~(3 << (3 * 2));

    // Pull-up
    GPIOC->PUPDR &= ~(3 << (2 * 2));
    GPIOC->PUPDR |=  (1 << (2 * 2)); // Pull-up PC2
    GPIOC->PUPDR &= ~(3 << (3 * 2));
    GPIOC->PUPDR |=  (1 << (3 * 2)); // Pull-up PC3
}

/* --- CẤU HÌNH NGẮT EXTI --- */
void init_interrupts(void)
{
    // 1. Enable clock cho SYSCFG (Bit 14 trong APB2ENR)
    RCC->APB2ENR |= (1 << 14);

    // 2. Cấu hình Port C cho EXTI2 và EXTI3
    // Sử dụng số 15 (tương đương 1111 nhị phân) để xóa 4 bit cấu hình
    // Sử dụng số 2 (tương đương 0010 nhị phân) để chọn Port C

    // Cấu hình PC2 cho EXTI2 (quản lý bởi bit 8 đến bit 11)
    SYSCFG->EXTICR[0] &= ~(15 << 8);    // Xóa 4 bit vị trí 8, 9, 10, 11
    SYSCFG->EXTICR[0] |=  (2 << 8);     // Ghi giá trị 2 (Port C) vào vị trí EXTI2

    // Cấu hình PC3 cho EXTI3 (quản lý bởi bit 12 đến bit 15)
    SYSCFG->EXTICR[0] &= ~(15 << 12);   // Xóa 4 bit vị trí 12, 13, 14, 15
    SYSCFG->EXTICR[0] |=  (2 << 12);    // Ghi giá trị 2 (Port C) vào vị trí EXTI3

    // 3. Cho phép (Unmask) EXTI2 và EXTI3
    EXTI->IMR |= (1 << 2);               // Set bit 2 cho EXTI Line 2
    EXTI->IMR |= (1 << 3);               // Set bit 3 cho EXTI Line 3

    // 4. Chọn trigger cạnh xuống (Falling Trigger)
    EXTI->FTSR |= (1 << 2);              // Kích hoạt cạnh xuống cho Line 2
    EXTI->FTSR |= (1 << 3);              // Kích hoạt cạnh xuống cho Line 3

    // 5. Đảm bảo tắt cạnh lên (Rising Trigger)
    EXTI->RTSR &= ~(1 << 2);
    EXTI->RTSR &= ~(1 << 3);
}

/* --- HÀM CẬP NHẬT LED --- */
void Update_LEDs(void)
{
    // Tắt toàn bộ 3 LED
    GPIOB->BSRR = (1 << (0 + 16)) | (1 << (1 + 16)) | (1 << (2 + 16));

    // Bật LED tương ứng
    if (led_pos == 0)
        GPIOB->BSRR = (1 << 0);
    else if (led_pos == 1)
        GPIOB->BSRR = (1 << 1);
    else if (led_pos == 2)
        GPIOB->BSRR = (1 << 2);
}

/* --- HÀM XỬ LÝ NGẮT PC2 --- */
void EXTI2_IRQHandler(void)
{
    // Kiểm tra EXTI2
    if (EXTI->PR & (1 << 2))
    {
        // Clear pending bit (ghi 1 để xóa)
        EXTI->PR |= (1 << 2);

        // Dịch trái: 0 -> 1 -> 2 -> 0
        led_pos++;
        if (led_pos > 2) led_pos = 0;
        Update_LEDs();
    }
}

/* --- HÀM XỬ LÝ NGẮT PC3 --- */
void EXTI3_IRQHandler(void)
{
    // Kiểm tra EXTI3
    if (EXTI->PR & (1 << 3))
    {
        // Clear pending bit (ghi 1 để xóa)
        EXTI->PR |= (1 << 3);

        // Dịch phải: 2 -> 1 -> 0 -> 2
        led_pos--;
        if (led_pos < 0) led_pos = 2;
        Update_LEDs();
    }
}

/* --- ĐỊNH NGHĨA ERROR_HANDLER --- */
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}




#include "stm32f4xx.h"

void init_leds(void);
void init_buttons(void);
void EXTI_Config(void);
void Update_LEDs(void);

volatile int8_t led_pos = 0;

int main() {
	init_leds();
	init_buttons();
	EXTI_Config();

	Update_LEDs();

	while (1) {

	}
}

void init_leds(void) {
	RCC->AHB1ENR |= 1 << 1;

	GPIOB->MODER &= ~(3 << (2 * 0));
	GPIOB->MODER &= ~(3 << (2 * 1));
	GPIOB->MODER &= ~(3 << (2 * 2));

	GPIOB->MODER |= (1 << (2 * 0));
	GPIOB->MODER |= (1 << (2 * 1));
	GPIOB->MODER |= (1 << (2 * 2));

    // Output push-pull
    GPIOB->OTYPER &= ~(1 << 0);
    GPIOB->OTYPER &= ~(1 << 1);
    GPIOB->OTYPER &= ~(1 << 2);

    // Không pull-up/pull-down
    GPIOB->PUPDR &= ~(3 << (0 * 2));
    GPIOB->PUPDR &= ~(3 << (1 * 2));
    GPIOB->PUPDR &= ~(3 << (2 * 2));
}

void init_buttons(void) {
	RCC->AHB1ENR |= 1 << 2;

	GPIOC->MODER &= ~(3 << (2 * 2));
	GPIOC->MODER &= ~(3 << (2 * 3));

	GPIOC->PUPDR &= ~(3 << (2 * 2));
	GPIOC->PUPDR &= ~(3 << (2 * 3));
	GPIOC->PUPDR |= (1 << (2 * 2));
	GPIOC->PUPDR |= (1 << (2 * 3));
}

void EXTI_Config(void) {
	RCC->APB2ENR |= (1 << 14);

	SYSCFG->EXTICR[0] &= ~(15 << 8);
	SYSCFG->EXTICR[0] |=  (2 << 8);

	SYSCFG->EXTICR[0] &= ~(15 << 12);
	SYSCFG->EXTICR[0] |=  (2 << 12);

	EXTI->IMR |= (1 << 2);
	EXTI->IMR |= (1 << 3);

	EXTI->FTSR |= (1 << 2);
	EXTI->FTSR |= (1 << 3);

	EXTI->RTSR &= ~(1 << 2);
	EXTI->RTSR &= ~(1 << 3);

	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);

}

void Update_LEDs(void) {
	GPIOB->BSRR = (1 << (16 + 0));
	GPIOB->BSRR = (1 << (16 + 1));
	GPIOB->BSRR = (1 << (16 + 2));

	if (led_pos == 0) {
		GPIOB->BSRR = (1 << 0);
	} else if (led_pos == 1) {
		GPIOB->BSRR = (1 << 1);
	} else if (led_pos == 2) {
		GPIOB->BSRR = (1 << 2);
	}

}

void EXTI2_IRQHandler(void) {
	if (EXTI->PR & (1 << 2)) {
		EXTI->PR |= (1 << 2);

		led_pos++;
		if (led_pos > 2) led_pos = 0;
		Update_LEDs();
	}
}

void EXTI3_IRQHandler(void) {
	if (EXTI->PR & (1 << 3)) {
		EXTI->PR |= (1 << 3);

		led_pos--;
		if (led_pos < 0) led_pos = 2;
		Update_LEDs();
	}
}

