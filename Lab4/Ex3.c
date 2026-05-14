// Cắm mạch trên breadboard để kết nối vi điều khiển STM32F401RE với 3 nút bấm (SW1, SW2, SW3) và 3 LED (LED 1, LED 2, LED 3). Viết chương trình minh họa cơ chế ngắt lồng nhau và ưu tiên chiếm quyền của bộ điều khiển NVIC theo các yêu cầu sau:
// 1.	Cấu hình SW1, SW2, SW3 sinh ra 3 ngắt ngoài (EXTI) trên 3 đường khác nhau.
// 2.	Thiết lập mức ưu tiên ngắt trong NVIC theo thứ tự: Mức ưu tiên của SW3 > Mức ưu tiên của SW2 > Mức ưu tiên của SW1 (Ngắt của SW3 có mức ưu tiên cao nhất).
// 3.	Hoạt động của các hàm phục vụ ngắt:
        // 	Khi ngắt SW1 xảy ra: Điều khiển LED 1 nhấp nháy chậm (chu kỳ ~1 giây) đủ 5 lần rồi tắt.
        // 	Khi ngắt SW2 xảy ra: Điều khiển LED 2 nhấp nháy nhanh (chu kỳ ~0.2 giây) đủ 10 lần rồi tắt.
        // 	Khi ngắt SW3 xảy ra: Bật sáng LED 3 giữ nguyên trong khoảng 3 giây, sau đó tắt LED 3.
// 4.	Yêu cầu kiểm chứng trên lớp: Sinh viên tiến hành kích hoạt ngắt SW1, trong lúc LED 1 đang nhấp nháy chậm thì nhấn tiếp SW2. Trong lúc LED 2 đang nhấp nháy nhanh thì nhấn tiếp SW3. Quan sát và giải thích hiện tượng các ngắt ưu tiên cao hơn liên tục cắt ngang các ngắt ưu tiên thấp hơn, và cách hệ thống quay lại hoàn thành các ngắt cũ sau khi ngắt ưu tiên cao kết thúc.


// ============ CODE THAY THẾ (CÓ CÁC LỖI) ============
#include "stm32f4xx.h"

// Khai báo các hàm xử lý
void delay_ms(uint32_t ms);              // Hàm delay sử dụng vòng lặp NOP
void init_leds(void);                    // Khởi tạo LED (PB0, PB1, PB2)
void init_buttons(void);                 // Khởi tạo nút bấm (PC0, PC1, PC2)
void EXTI_Config(void);                  // Cấu hình ngắt EXTI
void Update_LEDs(void);                  // Hàm cập nhật LED (không sử dụng)

// Biến global (không sử dụng trong code này)
volatile int8_t led_pos = 0;

// ============ HÀM MAIN ============
int main() {
	init_leds();           // Cấu hình GPIO cho LED
	init_buttons();        // Cấu hình GPIO cho nút bấm
	EXTI_Config();         // Cấu hình EXTI và NVIC

	// Vòng lặp chính rỗng - mọi xử lý được thực hiện bằng ngắt
	while (1) {

	}
}

// ============ HÀM DELAY ============
// Hàm delay sử dụng vòng lặp đơn giản (busy-wait)
// ms: thời gian delay tính bằng mili giây
// Cách tính: Với clock 16MHz, mỗi __NOP() mất ~1 chu kỳ clock
// 1ms ~ 16000 chu kỳ, nên count = ms * 1600 (ước lượng)
void delay_ms(uint32_t ms) {
	uint32_t count = ms * 1600;  // Ước lượng số vòng lặp cho ms mili giây
	while (count--) {
		__NOP();                 // NOP = No Operation (tốn 1 chu kỳ clock)
	}
}

// ============ KHỞI TẠO LED (PB0, PB1, PB2) ============
void init_leds(void) {
	// Bước 1: Bật clock cho GPIOB (Bit 1 của AHB1ENR)
	RCC->AHB1ENR |= 1 << 1;

	// Bước 2: Cấu hình MODER cho PB0, PB1, PB2 là Output (Mode = 01)
	// Mỗi pin chiếm 2 bit trong MODER:
	// - PB0 chiếm bit 0-1,  PB1 chiếm bit 2-3,  PB2 chiếm bit 4-5

	// Nhưng kết quả tính toán giống nhau: 2*0=0, 2*1=2, 2*2=4
	GPIOB->MODER &= ~(3 << (2 * 0));     // Clear bit 0-1 (PB0)
	GPIOB->MODER &= ~(3 << (2 * 1));     // Clear bit 2-3 (PB1)
	GPIOB->MODER &= ~(3 << (2 * 2));     // Clear bit 4-5 (PB2)

	// Ghi mode 01 (Output) cho 3 pin
	GPIOB->MODER |= (1 << (2 * 0));      // Set PB0 = Output
	GPIOB->MODER |= (1 << (2 * 1));      // Set PB1 = Output
	GPIOB->MODER |= (1 << (2 * 2));      // Set PB2 = Output

    // Bước 3: Cấu hình OTYPER (Output Type) = Push-Pull (0)
    GPIOB->OTYPER &= ~(1 << 0);         // PB0 = Push-Pull
    GPIOB->OTYPER &= ~(1 << 1);         // PB1 = Push-Pull
    GPIOB->OTYPER &= ~(1 << 2);         // PB2 = Push-Pull

    // Bước 4: Cấu hình PUPDR (Pull-up/Pull-down) = Không sử dụng (00)
    GPIOB->PUPDR &= ~(3 << (0 * 2));    // PB0 = No Pull
    GPIOB->PUPDR &= ~(3 << (1 * 2));    // PB1 = No Pull
    GPIOB->PUPDR &= ~(3 << (2 * 2));    // PB2 = No Pull
}

// ============ KHỞI TẠO NÚTBẤM (PC0, PC1, PC2) ============
void init_buttons(void) {
	// Bước 1: Bật clock cho GPIOC (Bit 2 của AHB1ENR)
	RCC->AHB1ENR |= 1 << 2;

	// Bước 2: Cấu hình MODER cho PC0, PC1, PC2 là Input (Mode = 00)
	// ❌ LỖI: Sử dụng (2*0), (2*1), (2*2) không nhất quán, nên viết lại
	GPIOC->MODER &= ~(3 << (2 * 0));     // Clear bit 0-1 (PC0)
	GPIOC->MODER &= ~(3 << (2 * 1));     // Clear bit 2-3 (PC1)
	GPIOC->MODER &= ~(3 << (2 * 2));     // Clear bit 4-5 (PC2)

	// Bước 3: Cấu hình PUPDR = Pull-up (01)
	// Pull-up để nút bấm ổn định khi không nhấn (logic 1)
	// Khi nhấn nút, tạo sườn xuống từ 1 -> 0 (Falling Edge)

	GPIOC->PUPDR &= ~(3 << (2 * 0));     // Clear PUPDR0
	GPIOC->PUPDR |= (1 << (2 * 0));      // Set PUPDR0 = Pull-up (PC0)

	GPIOC->PUPDR &= ~(3 << (2 * 1));     // Clear PUPDR1
	GPIOC->PUPDR |= (1 << (2 * 1));      // Set PUPDR1 = Pull-up (PC1)

	GPIOC->PUPDR &= ~(3 << (2 * 2));     // Clear PUPDR2
	GPIOC->PUPDR |= (1 << (2 * 2));      // Set PUPDR2 = Pull-up (PC2)
}

// ============ CẤU HÌNH EXTI VÀ NVIC ============
void EXTI_Config(void) {
	// Bước 1: Bật clock cho SYSCFG (Bit 14 của APB2ENR)
	// SYSCFG quản lý EXTI multiplexer (chọn Port cho mỗi EXTI line)
	RCC->APB2ENR |= (1 << 14);

	// Bước 2: Map Port C vào EXTI0, EXTI1, EXTI2
	// Thanh ghi EXTICR[0] quản lý EXTI0 đến EXTI3
	// Mỗi EXTI chiếm 4 bit, giá trị 2 = Port C

	// Cấu hình PC0 -> EXTI0 (bit 0-3)
	SYSCFG->EXTICR[0] &= ~(15 << 0);     // Clear bit 0-3
	SYSCFG->EXTICR[0] |= (2 << 0);       // Set = 2 (Port C)

	// Cấu hình PC1 -> EXTI1 (bit 4-7)
	SYSCFG->EXTICR[0] &= ~(15 << 4);     // Clear bit 4-7
	SYSCFG->EXTICR[0] |= (2 << 4);       // Set = 2 (Port C)

	// Cấu hình PC2 -> EXTI2 (bit 8-11)
	SYSCFG->EXTICR[0] &= ~(15 << 8);     // Clear bit 8-11
	SYSCFG->EXTICR[0] |= (2 << 8);       // Set = 2 (Port C)

	// Bước 3: Unmask EXTI lines (cho phép ngắt)
	// Thanh ghi IMR (Interrupt Mask Register)
	EXTI->IMR |= (1 << 0);               // Unmask EXTI0
	EXTI->IMR |= (1 << 1);               // Unmask EXTI1
	EXTI->IMR |= (1 << 2);               // Unmask EXTI2

	// Bước 4: Chọn loại trigger (cạnh xuống - Falling Edge)
	// FTSR = Falling Trigger Selection Register
	EXTI->FTSR |= (1 << 0);              // Enable falling edge cho EXTI0
	EXTI->FTSR |= (1 << 1);              // Enable falling edge cho EXTI1
	EXTI->FTSR |= (1 << 2);              // Enable falling edge cho EXTI2

	// Bước 5: Vô hiệu hóa cạnh lên (Rising Edge)
	// RTSR = Rising Trigger Selection Register
	EXTI->RTSR &= ~(1 << 0);             // Disable rising edge cho EXTI0
	EXTI->RTSR &= ~(1 << 1);             // Disable rising edge cho EXTI1
	EXTI->RTSR &= ~(1 << 2);             // Disable rising edge cho EXTI2

    // Bước 6: Cấu hình nhóm ưu tiên NVIC (Priority Grouping)
    // NVIC_SetPriorityGrouping(2) = 2 bit preemption, 2 bit subpriority
    // Cho phép ngắt ngang ngay lập tức ISR ưu tiên cao hơn
    // Nếu bỏ qua dòng này, mặc định là 0, không có preemption → ISR thấp phải chạy xong mới cho ISR cao chạy
	NVIC_SetPriorityGrouping(2);

	// Bước 7: Cấu hình mức ưu tiên cho từng ngắt
	// SW1 (EXTI0) = ưu tiên 3 (thấp nhất)
	// SW2 (EXTI1) = ưu tiên 2 (trung bình)
	// SW3 (EXTI2) = ưu tiên 1 (cao nhất)
	NVIC_SetPriority(EXTI0_IRQn, 3);     // SW1: Priority = 3 (Low)
	NVIC_SetPriority(EXTI1_IRQn, 2);     // SW2: Priority = 2 (Medium)
	NVIC_SetPriority(EXTI2_IRQn, 1);     // SW3: Priority = 1 (High)

	// Bước 8: Bật (Enable) ngắt trong NVIC
	NVIC_EnableIRQ(EXTI0_IRQn);          // Enable EXTI0 interrupt
	NVIC_EnableIRQ(EXTI1_IRQn);          // Enable EXTI1 interrupt
	NVIC_EnableIRQ(EXTI2_IRQn);          // Enable EXTI2 interrupt
}

// ============ HÀM XỬ LÝ NGẮT (ISR - Interrupt Service Routine) ============

// ---- EXTI0_IRQHandler: SW1 - Blink LED1 chậm (1s/chu kỳ) 5 lần ----
// Khi SW1 được nhấn, LED1 nhấp nháy với chu kỳ 1s (ON 500ms, OFF 500ms) tổng 5 lần
// BSRR register: bit 0-15 để BẬT (SET), bit 16-31 để TẮT (RESET)
void EXTI0_IRQHandler(void) {
	// Kiểm tra xem có phải ngắt do EXTI0 không
	if (EXTI->PR & (1 << 0)) {
		// Vòng lặp 5 lần blink
		for (int i = 0; i < 5; i++) {
			GPIOB->BSRR = (1 << 0);           // Bật LED1 (PB0)
			delay_ms(500);                     // Giữ sáng 500ms
			GPIOB->BSRR = (1 << (16 + 0));    // Tắt LED1 (PB0) bằng bit 16
			delay_ms(500);                     // Giữ tắt 500ms
		}

		// QUAN TRỌNG: Xóa cờ pending để thoát khỏi ngắt
		// Nếu không xóa, MCU sẽ bị kẹt trong ngắt
		EXTI->PR |= (1 << 0);
	}
}

// ---- EXTI1_IRQHandler: SW2 - Blink LED2 nhanh (0.2s/chu kỳ) 10 lần ----
// Khi SW2 được nhấn, LED2 nhấp nháy nhanh với chu kỳ 0.2s (ON 100ms, OFF 100ms) tổng 10 lần
void EXTI1_IRQHandler(void) {
	// Kiểm tra xem có phải ngắt do EXTI1 không
	if (EXTI->PR & (1 << 1)) {
		// Vòng lặp 10 lần blink
		for (int i = 0; i < 10; i++) {
			GPIOB->BSRR = (1 << 1);           // Bật LED2 (PB1)
			delay_ms(500);
			GPIOB->BSRR = (1 << (16 + 1));    // Tắt LED2 (PB1)
			delay_ms(500);
		}

		// Xóa cờ pending
		EXTI->PR |= (1 << 1);
	}
}

// ---- EXTI2_IRQHandler: SW3 - Bật LED3 giữ nguyên 3 giây ----
// Khi SW3 được nhấn, LED3 sáng liên tục trong 3 giây, rồi tắt
void EXTI2_IRQHandler(void) {
	// Kiểm tra xem có phải ngắt do EXTI2 không
	if (EXTI->PR & (1 << 2)) {
		GPIOB->BSRR = (1 << 2);              // Bật LED3 (PB2)
		delay_ms(3000);                      // Giữ sáng 3 giây (3000ms)
		GPIOB->BSRR = (1 << (16 + 2));       // Tắt LED3 (PB2)

		EXTI->PR |= (1 << 2);
	}
}














#include "stm32f4xx.h"

void delay_ms(uint32_t ms);
void init_leds(void);
void init_buttons(void);
void EXTI_Config(void);
void Update_LEDs(void);

volatile int8_t led_pos = 0;

int main() {
	init_leds();
	init_buttons();
	EXTI_Config();

	while (1) {

	}
}

void delay_ms(uint32_t ms) {
	uint32_t count = ms * 1600;
	while (count--) {
		__NOP();
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

	GPIOC->MODER &= ~(3 << (2 * 0));
	GPIOC->MODER &= ~(3 << (2 * 1));
	GPIOC->MODER &= ~(3 << (2 * 2));


	GPIOC->PUPDR &= ~(3 << (2 * 0));
	GPIOC->PUPDR &= ~(3 << (2 * 1));
	GPIOC->PUPDR &= ~(3 << (2 * 2));

	GPIOC->PUPDR |= (1 << (2 * 0));
	GPIOC->PUPDR |= (1 << (2 * 1));
	GPIOC->PUPDR |= (1 << (2 * 2));

}

void EXTI_Config(void) {
	RCC->APB2ENR |= (1 << 14);

	SYSCFG->EXTICR[0] &= ~(15 << 0);
	SYSCFG->EXTICR[0] |= (2 << 0);

	SYSCFG->EXTICR[0] &= ~(15 << 4);
	SYSCFG->EXTICR[0] |= (2 << 4);

	SYSCFG->EXTICR[0] &= ~(15 << 8);
	SYSCFG->EXTICR[0] |= (2 << 8);

	EXTI->IMR |= (1 << 0);
	EXTI->IMR |= (1 << 1);
	EXTI->IMR |= (1 << 2);

	EXTI->FTSR |= (1 << 0);
	EXTI->FTSR |= (1 << 1);
	EXTI->FTSR |= (1 << 2);

	EXTI->RTSR &= ~(1 << 0);
	EXTI->RTSR &= ~(1 << 1);
	EXTI->RTSR &= ~(1 << 2);

    NVIC_SetPriorityGrouping(2);

	NVIC_SetPriority(EXTI0_IRQn, 3);
	NVIC_SetPriority(EXTI1_IRQn, 2);
	NVIC_SetPriority(EXTI2_IRQn, 1);

	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
}

void EXTI0_IRQHandler(void) {
	if (EXTI->PR & (1 << 0)) {
		for (int i = 0; i < 5; i++) {
			GPIOB->BSRR = (1 << 0);
			delay_ms(500);
			GPIOB->BSRR = (1 << (16 + 0));
			delay_ms(500);
		}

		EXTI->PR |= (1 << 0);
	}
}

void EXTI1_IRQHandler(void) {
	if (EXTI->PR & (1 << 1)) {
		for (int i = 0; i < 10; i++) {
			GPIOB->BSRR = (1 << 1);
			delay_ms(500);
			GPIOB->BSRR = (1 << (16 + 1));
			delay_ms(500);
		}

		EXTI->PR |= (1 << 1);
	}
}

void EXTI2_IRQHandler(void) {
	if (EXTI->PR & (1 << 2)) {
		GPIOB->BSRR = (1 << 2);
		delay_ms(3000);
		GPIOB->BSRR = (1 << (16 + 2));
		EXTI->PR |= (1 << 2);
	}
}

