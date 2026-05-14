// BÀI 3: HỆ THỐNG ĐÈN GIAO THÔNG THÔNG MINH ( Các bạn nên dùng một biến toàn cục để đếm thời gian)( Nâng cao)
// Cắm 3 bóng LED rời trên breadboard: ĐỎ, VÀNG, XANH LÁ lần lượt nối vào các chân PA5, PA6, PA7 hoặc chân nào tùy em.Sử dụng Nút nhấn USER (chân PC13) đóng vai trò là "Nút bấm xin qua đường" dành cho người đi bộ. 
// Thiết kế chương trình điều khiển hệ thống đèn giao thông theo chu kỳ:
// 	Đèn xanh sáng trong 5 giây 
// 	Đèn vàng sáng trong 2 giây 
// 	Đèn đỏ sáng trong 5 giây 
// 	Sau đó lặp lại chu trình.
// Khi người dùng nhấn nút yêu cầu qua đường:
// 	Nếu đèn đang XANH: Bắt buộc hệ thống ngắt ngang thời gian xanh hiện tại, lập tức chuyển ngay sang trạng thái VÀNG (chạy nốt 2 giây) rồi sang ĐỎ để an toàn cho người đi bộ.
// 	Nếu đèn đang ĐỎ hoặc VÀNG: Ghi nhớ yêu cầu (lưu vào 1 biến cờ), sau khi đèn chuyển sang ĐỎ ở chu kỳ tiếp theo, tự động cộng thêm +3 giây vào thời gian sáng của đèn ĐỎ để người đi bộ có thêm thời gian qua đường.

#include "stm32f4xx.h"

#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7

typedef enum {
	GREEN_STATE,
	YELLOW_STATE,
	RED_STATE
} TrafficState;

volatile uint32_t second_counter = 0;
volatile uint32_t state_duration = 5;
volatile uint8_t pedestrian_request = 0;
volatile TrafficState current_state = GREEN_STATE;

void GPIO_Init_Custom(void);
void TIM2_Init(void);
void EXTI_Init(void);

void Set_Green(void);
void Set_Yellow(void);
void Set_Red(void);

void Change_State(TrafficState new_state);

int main (void) {
	GPIO_Init_Custom();
	TIM2_Init();
	EXTI_Init();
	Change_State(GREEN_STATE);

	while(1) {

	}
}

void GPIO_Init_Custom(void) {
	RCC->AHB1ENR |= (1 << 0);
	RCC->AHB1ENR |= (1 << 2);

	GPIOA->MODER &= ~(3 << (2 * 5));
	GPIOA->MODER &= ~(3 << (2 * 6));
	GPIOA->MODER &= ~(3 << (2 * 7));

	GPIOA->MODER |= (1 << (2 * 5));
	GPIOA->MODER |= (1 << (2 * 6));
	GPIOA->MODER |= (1 << (2 * 7));

	GPIOC->MODER &= ~(3 << (2 * 13));

}

void TIM2_Init(void) {
	RCC->APB1ENR |= (1 << 0);
	TIM2->PSC = 16000 - 1;
	TIM2->ARR = 1000 - 1;
	TIM2->DIER |= (1 << 0);
	TIM2->CR1 |= (1 << 0);
	NVIC_EnableIRQ(TIM2_IRQn);
}

void EXTI_Init(void) {
	RCC->APB2ENR |= (1 << 14);

	SYSCFG->EXTICR[3] &= ~(15 << 4);
	SYSCFG->EXTICR[3] |= (2 << 4);

	EXTI->FTSR |= (1 << 13);
	EXTI->RTSR &= ~(1 << 13);

	EXTI->IMR |= (1 << 13);

	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Set_Green(void) {
	GPIOA->ODR &= ~(1 << RED_LED);
	GPIOA->ODR &= ~(1 << YELLOW_LED);

	GPIOA->ODR |= (1 << GREEN_LED);
}

void Set_Yellow(void) {
	GPIOA->ODR &= ~(1 << RED_LED);
	GPIOA->ODR &= ~(1 << GREEN_LED);

	GPIOA->ODR |= (1 << YELLOW_LED);
}

void Set_Red(void) {
	GPIOA->ODR &= ~(1 << GREEN_LED);
	GPIOA->ODR &= ~(1 << YELLOW_LED);

	GPIOA->ODR |= (1 << RED_LED);
}

void Change_State(TrafficState new_state) {
	current_state = new_state;
	second_counter = 0;

	switch(current_state) {
		case GREEN_STATE:
			Set_Green();
			state_duration = 5;
			break;
		case YELLOW_STATE:
			Set_Yellow();
			state_duration = 2;
			break;
		case RED_STATE:
			Set_Red();
			if (pedestrian_request) {
				state_duration = 8;
				pedestrian_request = 0;
			} else {
				state_duration = 5;
			}
			break;
	}
}

void TIM2_IRQHandler(void) {
	if (TIM2->SR & (1 << 0)) {
		TIM2->SR &= ~(1 << 0);

		second_counter++;
		switch(current_state) {
			case GREEN_STATE:
				if (second_counter >= state_duration)
					Change_State(YELLOW_STATE);
				break;

			case YELLOW_STATE:
				if (second_counter >= state_duration)
					Change_State(RED_STATE);
				break;
			case RED_STATE:
				if (second_counter >= state_duration)
					Change_State(GREEN_STATE);
				break;
		}
	}
}

void EXTI15_10_IRQHandler(void) {
	if (EXTI->PR & (1 << 13)) {
		EXTI->PR = (1 << 13);

		if(current_state == GREEN_STATE) {
			Change_State(YELLOW_STATE);
		} else {
			pedestrian_request = 1;
		}
	}
}




// #include "stm32f4xx.h"

// /* ĐỊNH NGHĨA TRẠNG THÁI */
// typedef enum {
//     STATE_XANH = 0,
//     STATE_VANG,
//     STATE_DO
// } TrafficState;

// /* BIẾN TOÀN CỤC */
// volatile TrafficState current_state = STATE_XANH;
// volatile uint32_t seconds = 0;
// volatile uint8_t pedestrian_pending = 0; // Cờ ghi nhớ yêu cầu qua đường

// /* HÀM CẬP NHẬT LED */
// void update_leds(void) {
//     // PA5: ĐỎ, PA6: VÀNG, PA7: XANH LÁ
//     // Xóa sạch các chân PA5, PA6, PA7 trước khi bật chân mới
//     GPIOA->ODR &= ~((1 << 5) | (1 << 6) | (1 << 7));

//     switch (current_state) {
//         case STATE_XANH: GPIOA->ODR |= (1 << 7); break;
//         case STATE_VANG: GPIOA->ODR |= (1 << 6); break;
//         case STATE_DO:   GPIOA->ODR |= (1 << 5); break;
//     }
// }

// /* CẤU HÌNH PHẦN CỨNG */
// void System_Config(void) {
//     // 1. Bật Clock cho PORT A (LED) và PORT C (Nút nhấn)
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

//     // 2. Cấu hình GPIO PA5, PA6, PA7 là Output
//     GPIOA->MODER &= ~((3 << 10) | (3 << 12) | (3 << 14)); // Clear
//     GPIOA->MODER |=  ((1 << 10) | (1 << 12) | (1 << 14)); // Set Output

//     // 3. Cấu hình PC13 là Input (Nút nhấn User trên mạch Nucleo là Pull-up mặc định)
//     GPIOC->MODER &= ~(3 << 26);

//     // 4. Cấu hình Ngắt ngoài EXTI cho PC13
//     RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;           // Bật clock SYSCFG
//     SYSCFG->EXTICR[3] &= ~(0xF << 4);                // Clear EXTI13
//     SYSCFG->EXTICR[3] |=  (0x2 << 4);                // Map EXTI13 tới PORT C
//     EXTI->IMR |= (1 << 13);                          // Unmask ngắt 13
//     EXTI->FTSR |= (1 << 13);                         // Ngắt cạnh xuống (nhấn nút)
//     NVIC_EnableIRQ(EXTI15_10_IRQn);                  // Bật ngắt trên NVIC

//     // 5. Cấu hình Timer 3 tạo ngắt mỗi 1 giây
//     RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
//     TIM3->PSC = 16000 - 1;                           // Giả định Clock nội 16MHz -> 1kHz
//     TIM3->ARR = 1000 - 1;                            // 1000ms = 1s
//     TIM3->DIER |= TIM_DIER_UIE;                      // Cho phép ngắt Update
//     NVIC_EnableIRQ(TIM3_IRQn);
//     TIM3->CR1 |= TIM_CR1_CEN;                        // Bắt đầu đếm
// }

// /* TRÌNH PHỤC VỤ NGẮT NÚT NHẤN (PC13) */
// void EXTI15_10_IRQHandler(void) {
//     if (EXTI->PR & (1 << 13)) { // Kiểm tra đúng cờ ngắt line 13
//         if (current_state == STATE_XANH) {
//             // NẾU ĐANG XANH: Chuyển ngay sang VÀNG, reset giây để chạy đủ 2s vàng
//             current_state = STATE_VANG;
//             seconds = 0;
//             pedestrian_pending = 1; // Ghi nhớ để tí nữa đèn Đỏ cộng thêm thời gian
//             update_leds();
//         } else {
//             // NẾU ĐANG VÀNG HOẶC ĐỎ: Chỉ ghi nhớ cờ
//             pedestrian_pending = 1;
//         }
//         EXTI->PR = (1 << 13);   // Xóa cờ ngắt
//     }
// }

// /* TRÌNH PHỤC VỤ NGẮT TIMER (MỖI 1 GIÂY) */
// void TIM3_IRQHandler(void) {
//     if (TIM3->SR & TIM_SR_UIF) {
//         seconds++;

//         switch (current_state) {
//             case STATE_XANH:
//                 if (seconds >= 5) {
//                     current_state = STATE_VANG;
//                     seconds = 0;
//                 }
//                 break;

//             case STATE_VANG:
//                 if (seconds >= 2) {
//                     current_state = STATE_DO;
//                     seconds = 0;
//                 }
//                 break;

//             case STATE_DO:
//                 // Kiểm tra nếu có yêu cầu từ người đi bộ thì thời gian là 5+3=8s
//                 uint32_t target_time = (pedestrian_pending) ? 8 : 5;

//                 if (seconds >= target_time) {
//                     current_state = STATE_XANH;
//                     seconds = 0;
//                     pedestrian_pending = 0; // Reset cờ sau khi hết đèn đỏ
//                 }
//                 break;
//         }
//         update_leds();
//         TIM3->SR &= ~TIM_SR_UIF; // Xóa cờ ngắt timer
//     }
// }

// /* HÀM MAIN */
// int main(void) {
//     System_Config();
//     update_leds(); // Trạng thái ban đầu: Xanh sáng

//     while (1) {
//         // CPU rảnh rỗi, toàn bộ logic nằm trong Interrupt
//         __WFI(); // Chế độ nghỉ chờ ngắt (Wait For Interrupt)
//     }
// }

// #include "stm32f4xx.h"

// #define GREEN 0
// #define YELLOW 1
// #define RED 2

// void init_leds();
// void init_buttons();
// void delay_ms(uint32_t ms);
// void init_external_interrupt();
// void init_TIM3_interrupt();

// volatile uint16_t count = 0;
// volatile uint16_t force_yellow = 0;
// volatile uint16_t ped_request = 0;
// volatile uint16_t time_limit

// #include "stm32f4xx.h"

// /* =========================
//    DEFINE LED PIN
// ========================= */

// #define RED_LED     5   // PA5
// #define YELLOW_LED  6   // PA6
// #define GREEN_LED   7   // PA7

// /* =========================
//    TRAFFIC STATES
// ========================= */

// typedef enum
// {
//     GREEN_STATE,
//     YELLOW_STATE,
//     RED_STATE

// }TrafficState;

// /* =========================
//    GLOBAL VARIABLES
// ========================= */

// volatile uint32_t second_counter = 0;

// volatile uint8_t pedestrian_request = 0;

// volatile TrafficState current_state = GREEN_STATE;

// volatile uint32_t state_duration = 5;

// /* =========================
//    FUNCTION PROTOTYPES
// ========================= */

// void GPIO_Init_Custom(void);
// void TIM2_Init(void);
// void EXTI_Init(void);

// void Set_Green(void);
// void Set_Yellow(void);
// void Set_Red(void);

// void Change_State(TrafficState new_state);

// /* =========================================================
//    MAIN
// ========================================================= */

// int main(void)
// {
//     GPIO_Init_Custom();

//     TIM2_Init();

//     EXTI_Init();

//     Change_State(GREEN_STATE);

//     while(1)
//     {

//     }
// }

// /* =========================================================
//    GPIO INIT
// ========================================================= */

// void GPIO_Init_Custom(void)
// {
//     /* Enable clock GPIOA and GPIOC */
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
//     RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

//     /* PA5 PA6 PA7 OUTPUT */

//     GPIOA->MODER &= ~(3 << (5 * 2));
//     GPIOA->MODER &= ~(3 << (6 * 2));
//     GPIOA->MODER &= ~(3 << (7 * 2));

//     GPIOA->MODER |=  (1 << (5 * 2));
//     GPIOA->MODER |=  (1 << (6 * 2));
//     GPIOA->MODER |=  (1 << (7 * 2));

//     /* PC13 INPUT */

//     GPIOC->MODER &= ~(3 << (13 * 2));
// }

// /* =========================================================
//    TIMER2 INIT
//    Interrupt every 1 second
// ========================================================= */

// void TIM2_Init(void)
// {
//     /* Enable TIM2 clock */

//     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

//     /*
//        Timer clock = 16MHz

//        Prescaler = 16000 - 1
//        => counter clock = 1kHz

//        ARR = 1000 - 1
//        => interrupt every 1 second
//     */

//     TIM2->PSC = 16000 - 1;

//     TIM2->ARR = 1000 - 1;

//     /* Enable update interrupt */

//     TIM2->DIER |= TIM_DIER_UIE;

//     /* Enable timer */

//     TIM2->CR1 |= TIM_CR1_CEN;

//     /* Enable NVIC */

//     NVIC_EnableIRQ(TIM2_IRQn);
// }

// /* =========================================================
//    EXTI INIT FOR PC13
// ========================================================= */

// void EXTI_Init(void)
// {
//     /* Enable SYSCFG clock */

//     RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

//     /* Connect EXTI13 to PC13 */

//     SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

//     /* Falling edge trigger */

//     EXTI->FTSR |= EXTI_FTSR_TR13;

//     /* Unmask EXTI13 */

//     EXTI->IMR |= EXTI_IMR_IM13;

//     /* Enable interrupt */

//     NVIC_EnableIRQ(EXTI15_10_IRQn);
// }

// /* =========================================================
//    LED CONTROL
// ========================================================= */

// void Set_Green(void)
// {
//     GPIOA->ODR &= ~(1 << RED_LED);
//     GPIOA->ODR &= ~(1 << YELLOW_LED);

//     GPIOA->ODR |=  (1 << GREEN_LED);
// }

// void Set_Yellow(void)
// {
//     GPIOA->ODR &= ~(1 << RED_LED);
//     GPIOA->ODR &= ~(1 << GREEN_LED);

//     GPIOA->ODR |=  (1 << YELLOW_LED);
// }

// void Set_Red(void)
// {
//     GPIOA->ODR &= ~(1 << YELLOW_LED);
//     GPIOA->ODR &= ~(1 << GREEN_LED);

//     GPIOA->ODR |=  (1 << RED_LED);
// }

// /* =========================================================
//    CHANGE STATE
// ========================================================= */

// void Change_State(TrafficState new_state)
// {
//     current_state = new_state;

//     second_counter = 0;

//     switch(current_state)
//     {
//         case GREEN_STATE:

//             Set_Green();

//             state_duration = 5;

//             break;

//         case YELLOW_STATE:

//             Set_Yellow();

//             state_duration = 2;

//             break;

//         case RED_STATE:

//             Set_Red();

//             /*
//                Nếu có yêu cầu qua đường
//                cộng thêm 3 giây
//             */

//             if(pedestrian_request)
//             {
//                 state_duration = 8;

//                 pedestrian_request = 0;
//             }
//             else
//             {
//                 state_duration = 5;
//             }

//             break;
//     }
// }

// /* =========================================================
//    TIMER2 INTERRUPT
// ========================================================= */

// void TIM2_IRQHandler(void)
// {
//     /* Check update flag */

//     if(TIM2->SR & TIM_SR_UIF)
//     {
//         /* Clear flag */

//         TIM2->SR &= ~TIM_SR_UIF;

//         second_counter++;

//         switch(current_state)
//         {
//             case GREEN_STATE:

//                 if(second_counter >= state_duration)
//                 {
//                     Change_State(YELLOW_STATE);
//                 }

//                 break;

//             case YELLOW_STATE:

//                 if(second_counter >= state_duration)
//                 {
//                     Change_State(RED_STATE);
//                 }

//                 break;

//             case RED_STATE:

//                 if(second_counter >= state_duration)
//                 {
//                     Change_State(GREEN_STATE);
//                 }

//                 break;
//         }
//     }
// }

// /* =========================================================
//    EXTI INTERRUPT
// ========================================================= */

// void EXTI15_10_IRQHandler(void)
// {
//     if(EXTI->PR & EXTI_PR_PR13)
//     {
//         /* Clear pending bit */

//         EXTI->PR |= EXTI_PR_PR13;

//         /*
//            Nếu đang GREEN
//            chuyển ngay sang YELLOW
//         */

//         if(current_state == GREEN_STATE)
//         {
//             Change_State(YELLOW_STATE);
//         }
//         else
//         {
//             /*
//                Nếu đang RED hoặc YELLOW
//                lưu yêu cầu qua đường
//             */

//             pedestrian_request = 1;
//         }
//     }
// }