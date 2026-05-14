/*
 * CHƯƠNG TRÌNH: ĐIỀU KHIỂN LED BẰNG CẢM BIẾN SIÊU ÂM HC-SR04
 * 
 * MỤC TIÊU:
 *   - Đo khoảng cách bằng cảm biến siêu âm HC-SR04
 *   - Sử dụng Input Capture (TIM1 Channel 1) để đo độ rộng xung ECHO
 *   - Sử dụng PWM (TIM2 Channel 1) để điều khiển độ sáng LED PA5
 *   - Khoảng cách càng gần, LED càng sáng (và ngược lại)
 * 
 * CÁC CHÂN SỬ DỤNG:
 *   - PA1: TRIG (Trigger của cảm biến HC-SR04) - Output
 *   - PA8: ECHO (Echo từ cảm biến HC-SR04) - TIM1_CH1 (Input Capture)
 *   - PA5: LED (Điều khiển độ sáng qua PWM) - TIM2_CH1 (PWM)
 * 
 * CÔNG THỨC TÍNH KHOẢNG CÁCH:
 *   Distance(cm) = Time(µs) × 0.017
 *   (Âm thanh di chuyển với tốc độ ~340 m/s, nên 0.017 cm/µs)
 */

#include "stm32f401xe.h"

// Các biến volatile để giao tiếp giữa main và interrupt handler
volatile uint8_t complete_flag = 0;     // Cờ báo hoàn thành đo khoảng cách
volatile uint8_t edge_flag = 0;         // Cờ báo đã bắt được cạnh lên (rising edge)
volatile uint32_t capture_time = 0;     // Lưu thời gian xung ECHO (µs)

void init_GPIO();
void init_pwm();
void init_input_capture();
void delay_us(uint32_t us);

/*
 * HÀM MAIN: Vòng lặp chính
 * Lưu lượng:
 *   1. Gửi xung TRIG 10µs lên PA1
 *   2. Chờ cảm biến phản hồi qua ECHO (PA8)
 *   3. Input Capture đo độ rộng xung ECHO
 *   4. Tính khoảng cách từ thời gian xung
 *   5. Điều khiển PWM của LED dựa trên khoảng cách
 *   6. Chờ 60ms rồi lặp lại
 */
int main(){
	init_GPIO();
	init_pwm();
	init_input_capture();

	while(1){
		// Reset các cờ để chuẩn bị cho lần đo khoảng cách tiếp theo
		complete_flag = 0;
		edge_flag = 0;

		// Gửi xung TRIG lên cảm biến HC-SR04
		// TRIG cần xung cao 10µs để kích hoạt cảm biến
		GPIOA->ODR |= 1 << 1;       // PA1 = 1 (TRIG cao)
		delay_us(10);               // Chờ 10µs
		GPIOA->ODR &= ~(1 << 1);    // PA1 = 0 (TRIG thấp)

		// Chờ lấy kết quả đo từ Input Capture
		// Timeout = 500000 để tránh treo nếu cảm biến không hoạt động
		uint32_t timeout = 500000;

		while(complete_flag == 0 && timeout > 0){
			timeout--;  // Giảm timeout mỗi vòng
		}

		// Nếu capture thành công
		if(complete_flag == 1){
			// Tính khoảng cách dựa trên thời gian xung ECHO
			// Công thức: Distance = capture_time(µs) × 0.017 cm/µs
			float distance = 0.017 * capture_time;  // cm

			// Kiểm tra khoảng cách hợp lệ (0-100cm)
			if(distance > 0 && distance <= 100){
				// Điều chỉnh độ sáng LED: CCR1 = 10 × distance
				// distance = 0cm → CCR1 = 0 (tắt)
				// distance = 100cm → CCR1 = 1000 (sáng toàn bộ)
				TIM2->CCR1 = 10 * distance;
			}
			else{
				// Khoảng cách ngoài phạm vi → tắt LED
				TIM2->CCR1 = 0;
			}
		}
		else{
			// Đo lỗi (timeout) → tắt LED
			TIM2->CCR1 = 0;
		}

		// Chờ 60ms trước khi lần đo tiếp theo
		// (Tránh cảm biến nhận được xung phản xạ từ lần trước)
		delay_us(60000);

	}
}

/*
 * HÀM KHỞI TẠO GPIO
 * Cấu hình các chân GPIO để sử dụng với Timers
 */
void init_GPIO(){
	// Bật clock cho Port A
	RCC->AHB1ENR |= 1 << 0;     // GPIOAEN = 1

	// PA8: ECHO (Input Capture cho TIM1_CH1)
	// Mode = 10 (Alternate Function)
	GPIOA->MODER |= 1 << 17;
	// AFR[1] là thanh ghi AF cho PA8-PA15
	// AF1 cho TIM1 Channel 1
	GPIOA->AFR[1] |= 1 << 0;    // PA8 -> AF1

	// PA5: LED (PWM cho TIM2_CH1)
	// Mode = 10 (Alternate Function)
	GPIOA->MODER |= 1 << 11;
	// AFR[0] là thanh ghi AF cho PA0-PA7
	// AF1 cho TIM2 Channel 1
	GPIOA->AFR[0] |= 1 << 20;   // PA5 -> AF1

	// PA1: TRIG (Output GPIO để điều khiển)
	// Mode = 01 (General Purpose Output)
	GPIOA->MODER |= 1 << 2;
}

/*
 * HÀM KHỞI TẠO PWM TRÊN TIM2 CHANNEL 1 (PA5 - LED)
 * Tần số PWM: 1kHz, Duty cycle: 0-100%
 * 
 * Công thức tần số:
 *   f = 84MHz / ((PSC+1) × (ARR+1))
 *   f = 84MHz / (16 × 1000) = 525 Hz ≈ 1kHz
 */
void init_pwm(){
	// Bật clock cho TIM2 (APB1 bus)
	RCC->APB1ENR |= 1 << 0;     // TIM2EN = 1

	// Cấu hình Prescaler: chia tần số từ 84MHz xuống
	TIM2->PSC = 16 - 1;         // PSC = 15, chia 84MHz cho 16 -> 5.25MHz

	// Cấu hình Auto Reload Register (ARR): xác định chu kỳ xung
	TIM2->ARR = 1000;           // ARR = 1000, 5.25MHz / 1000 ≈ 5.25kHz (gần 1kHz)

	// Giá trị ban đầu duty cycle
	TIM2->CCR1 = 0;             // LED tắt ban đầu

	// Cấu hình PWM Mode 1 cho Channel 1
	// CCMR1 bit 6:4 = OC1M = 110 (PWM Mode 1)
	// CCMR1 bit 3 = OC1PE = 1 (Output Compare Preload Enable)
	TIM2->CCMR1 |= (1 << 6 | 1 << 5);  // Set bit 6 và bit 5 = PWM Mode

	// Kích hoạt Channel 1 output
	TIM2->CCER |= 1 << 0;       // CC1E = 1 (Channel 1 Output Enable)

	// Khởi động Timer 2
	TIM2->CR1 |= 1 << 0;        // CEN = 1 (Counter Enable)
}

/*
 * HÀM KHỞI TẠO INPUT CAPTURE TRÊN TIM1 CHANNEL 1 (PA8 - ECHO)
 * Đo độ rộng của xung ECHO từ cảm biến HC-SR04
 * 
 * Nguyên lý:
 *   - Bắt cạnh lên (rising edge): reset counter, chuyển sang bắt cạnh xuống
 *   - Bắt cạnh xuống (falling edge): lưu giá trị counter vào CCR1
 *   - Thời gian xung = giá trị CCR1
 * 
 * Công thức:
 *   f_clock = 84MHz / (PSC+1) = 84MHz / 16 = 5.25MHz (chu kỳ = 190ns)
 *   Time(µs) = capture_time × 190ns ÷ 1000 ≈ capture_time × 0.19µs
 */
void init_input_capture(){
	// Bật clock cho TIM1 (APB2 bus - Advanced Timer)
	RCC->APB2ENR |= 1 << 0;     // TIM1EN = 1

	// Cấu hình Prescaler
	TIM1->PSC = 16 - 1;         // PSC = 15, chia 84MHz cho 16 -> 5.25MHz

	// Cấu hình Auto Reload Register
	TIM1->ARR = 65000;          // ARR = 65000 (cho phép đo xung tối đa ~12ms)

	// Reset bộ đếm
	TIM1->CNT = 0;              // CNT = 0

	// Cấu hình Channel 1 ở chế độ Input Capture
	// CCMR1 bit 1:0 = CC1S = 01 (Mapped on TI1 input - capture on CH1)
	TIM1->CCMR1 |= 1 << 0;      // CC1S = 01 (Input Capture)

	// Mặc định bắt cạnh lên
	// CCER bit 1 = CC1NP = 0 (không bắt cạnh xuống ban đầu)
	TIM1->CCER &= ~(1 << 1);    // CC1NP = 0

	// Kích hoạt ngắt Capture/Compare Channel 1
	// DIER bit 1 = CC1IE = 1 (Channel 1 Capture/Compare Interrupt Enable)
	TIM1->DIER |= 1 << 1;       // CC1IE = 1

	// Kích hoạt ngắt TIM1_CC trong NVIC
	NVIC_EnableIRQ(TIM1_CC_IRQn);

	// Kích hoạt Channel 1 Capture
	// CCER bit 0 = CC1E = 1 (Channel 1 Capture/Compare Enable)
	TIM1->CCER |= 1 << 0;       // CC1E = 1

	// Khởi động Timer 1
	TIM1->CR1 |= 1 << 0;        // CEN = 1 (Counter Enable)
}

/*
 * HÀM INTERRUPT HANDLER CHO TIM1 CHANNEL 1
 * Xử lý việc bắt cạnh lên và cạnh xuống của xung ECHO
 * 
 * Lưu lượng:
 *   1. Bắt cạnh lên: reset counter, chuyển sang bắt cạnh xuống, set edge_flag = 1
 *   2. Bắt cạnh xuống: lưu giá trị counter vào capture_time, set complete_flag = 1
 */
void TIM1_CC_IRQHandler(){
	// Kiểm tra cờ ngắt CC1 (Capture/Compare Channel 1)
	if(TIM1->SR & 1 << 1){      // CC1IF = 1 (Channel 1 Interrupt Flag)
		// Xóa cờ ngắt
		TIM1->SR &= ~(1 << 1);     // CC1IF = 0

		if(edge_flag == 0){
			// LẦN ĐẦU: Bắt được cạnh LÊN (Rising Edge) của xung ECHO
			// Đây là khi cảm biến bắt đầu phát xung quay lại

			// Reset bộ đếm để bắt đầu đo từ 0
			TIM1->CNT = 0;

			// Cấu hình để bắt cạnh XUỐNG (Falling Edge) trong lần tiếp theo
			// Vô hiệu hóa Capture trước
			TIM1->CCER &= ~(1 << 0);   // CC1E = 0

			// CC1NP = 1: Bắt cạnh xuống
			TIM1->CCER |= 1 << 1;      // CC1NP = 1

			// Kích hoạt lại Capture
			TIM1->CCER |= 1 << 0;      // CC1E = 1

			// Đánh dấu đã bắt cạnh lên, chờ bắt cạnh xuống
			edge_flag = 1;
		}
		else{
			// LẦN THỨ 2: Bắt được cạnh XUỐNG (Falling Edge) của xung ECHO
			// Đây là khi xung ECHO kết thúc

			// Lưu thời gian xung (độ rộng xung) từ CCR1
			capture_time = TIM1->CCR1;

			// Trở lại cấu hình bắt cạnh LÊN cho lần đo tiếp theo
			TIM1->CCER &= ~(1 << 0);   // CC1E = 0
			TIM1->CCER &= ~(1 << 1);   // CC1NP = 0 (bắt cạnh lên)
			TIM1->CCER |= 1 << 0;      // CC1E = 1 (kích hoạt lại)

			// Báo hiệu main() rằng đã đo xong
			complete_flag = 1;

			// Reset flag để chuẩn bị cho lần đo tiếp theo
			edge_flag = 0;
		}
	}
}

/*
 * HÀM DELAY MICROSECOND (Chính xác)
 * Sử dụng TIM3 để tạo delay chính xác theo microsecond
 * 
 * Cách hoạt động:
 *   - PSC = 15: chia 84MHz cho 16 -> 5.25MHz (chu kỳ = 190ns ≈ 0.19µs)
 *   - ARR = us - 1: khi counter đạt us, sẽ báo hiệu UIF (Update Interrupt Flag)
 *   - Chờ UIF = 1 rồi dừng Timer
 */
void delay_us(uint32_t us){
	// Bật clock cho TIM3
	RCC->APB1ENR |= 1 << 1;     // TIM3EN = 1

	// Cấu hình Prescaler
	TIM3->PSC = 16 - 1;         // PSC = 15, chia 84MHz cho 16 -> 5.25MHz

	// Cấu hình Auto Reload Register
	// Khi counter đạt ARR, sẽ xảy ra update event
	TIM3->ARR = us - 1;         // ARR = us - 1

	// Reset bộ đếm
	TIM3->CNT = 0;              // CNT = 0

	// Khởi động Timer 3
	TIM3->CR1 |= 1 << 0;        // CEN = 1 (Counter Enable)

	// Xóa cờ Update Interrupt Flag trước khi chờ
	TIM3->SR &= ~(1 << 0);      // UIF = 0

	// Chờ cho đến khi cờ UIF = 1 (timer hoàn thành)
	while(!(TIM3->SR & 1 << 0)); // Chờ UIF = 1

	// Xóa cờ UIF
	TIM3->SR &= ~(1 << 0);      // UIF = 0

	// Dừng Timer 3
	TIM3->CR1 &= ~(1 << 0);     // CEN = 0 (Counter Disable)
}