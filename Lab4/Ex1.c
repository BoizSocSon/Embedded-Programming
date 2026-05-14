// Sử dụng LED màu xanh lá (chân PA5) và nút bấm USER (chân PC13) tích hợp sẵn trên board NUCLEO-F401RE. Cắm thêm 1 nút bấm ngoài trên breadboard kết nối vào chân PC8.
// Viết chương trình điều khiển LED bằng ngắt ngoài (EXTI) theo các yêu cầu sau:
// 1.	Hoạt động bình thường: Trong vòng lặp chính while(1) của hàm main(), điều khiển LED PA5 nhấp nháy (toggle) liên tục với chu kỳ nhất định.
// 2.	Cấu hình 2 chân PC13 và PC8 tạo ra ngắt ngoài mỗi khi phát hiện sườn xuống.
// 3.	Khi nhấn PC13 (Ngắt EXTI13): Sử dụng hàm phục vụ ngắt EXTI15_10_IRQHandler. Trong hàm này, điều khiển LED PA5 sáng liên tục trong một khoảng thời gian (ví dụ 3 giây), sau đó thoát ngắt để chương trình chính tiếp tục việc nhấp nháy.
// 4.	Khi nhấn PC8 (Ngắt EXTI8): Sử dụng hàm phục vụ ngắt EXTI9_5_IRQHandler. Trong hàm này, điều khiển LED PA5 tắt hoàn toàn trong một khoảng thời gian (ví dụ 3 giây), sau đó thoát ngắt để chương trình chính tiếp tục việc nhấp nháy.
// 5.	Có thể thử bấm cùng 1 lúc PC8 và PC13 để xem ngắt nào sẽ được ưu tiên hơn, quan sát kĩ hiện tượng để hiểu hơn về mức độ ưu tiên

#include "stm32f4xx.h"

// Hàm delay đơn giản (không chính xác tuyệt đối nhưng đủ dùng để quan sát)
void delay_ms(uint32_t ms) {
    uint32_t i;
    for (i = 0; i < ms * 1000; i++); // Ước lượng cho clock mặc định 16MHz
}

void GPIO_Config(void) {
    // 1. Bật Clock cho GPIOA và GPIOC
    // Bit 0 tương ứng với GPIOA, Bit 2 tương ứng với GPIOC
    RCC->AHB1ENR |= (1 << 0) | (1 << 2);

    // 2. Cấu hình PA5 là Output (LED)
    GPIOA->MODER &= ~(3 << (5 * 2)); // Clear bits
    GPIOA->MODER |=  (1 << (5 * 2)); // Set Mode Output

    // 3. Cấu hình PC13 và PC8 là Input (Nút bấm)
    GPIOC->MODER &= ~((3 << (13 * 2)) | (3 << (8 * 2)));

    // Cấu hình Pull-up cho PC13 và PC8 (Vì nút bấm thường nối GND khi nhấn)
    GPIOC->PUPDR &= ~((3 << (13 * 2)) | (3 << (8 * 2)));
    GPIOC->PUPDR |=  ((1 << (13 * 2)) | (1 << (8 * 2)));
}

void EXTI_Config(void) {
    // 1. Bật Clock cho SYSCFG (Sử dụng bit 14 của thanh ghi APB2ENR)
    RCC->APB2ENR |= (1 << 14);

    // 2. Kết nối chân PC13 vào EXTI13
    // SYSCFG_EXTICR4 (mảng index [3]) quản lý EXTI12 đến EXTI15.
    // EXTI13 quản lý bởi bit 4 đến bit 7 của thanh ghi này.
    SYSCFG->EXTICR[3] &= ~(15 << 4);    // Xóa 4 bit tại vị trí 4, 5, 6, 7
    SYSCFG->EXTICR[3] |=  (2 << 4);     // Ghi giá trị 2 (Port C) vào EXTI13

    // 3. Kết nối chân PC8 vào EXTI8
    // SYSCFG_EXTICR3 (mảng index [2]) quản lý EXTI8 đến EXTI11.
    // EXTI8 quản lý bởi bit 0 đến bit 3 của thanh ghi này.
    SYSCFG->EXTICR[2] &= ~(15 << 0);    // Xóa 4 bit tại vị trí 0, 1, 2, 3
    SYSCFG->EXTICR[2] |=  (2 << 0);     // Ghi giá trị 2 (Port C) vào EXTI8

    // 3. Cấu hình ngắt sườn xuống (Falling Edge) cho EXTI8 và EXTI13
    EXTI->FTSR |= (1 << 8) | (1 << 13);
    EXTI->RTSR &= ~((1 << 8) | (1 << 13)); // Đảm bảo không ngắt sườn lên

    // 4. Cho phép ngắt (Unmask) EXTI8 và EXTI13
    EXTI->IMR |= (1 << 8) | (1 << 13);

    // 5. Cấu hình NVIC (Priority và Enable ngắt)
    NVIC_SetPriority(EXTI9_5_IRQn, 1);    // Ưu tiên cao hơn (số nhỏ hơn)
    NVIC_SetPriority(EXTI15_10_IRQn, 2);  // Ưu tiên thấp hơn (số lớn hơn)
	NVIC_SetPriorityGrouping(2);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// Xử lý ngắt cho chân PC13
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & (1 << 13)) { // Kiểm tra cờ ngắt chân 13
        GPIOA->BSRR = (1 << 5); // LED PA5 Sáng
        delay_ms(3000);          // Giữ trong 3 giây
        EXTI->PR = (1 << 13);   // Xóa cờ ngắt (Viết 1 để xóa)
    }
}

// Xử lý ngắt cho chân PC8
void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & (1 << 8)) {  // Kiểm tra cờ ngắt chân 8
        GPIOA->BSRR = (1 << (5 + 16)); // LED PA5 Tắt (Reset bit)
        delay_ms(3000);                 // Giữ trong 3 giây
        EXTI->PR = (1 << 8);           // Xóa cờ ngắt
    }
}

int main(void) {
    GPIO_Config();
    EXTI_Config();

    while (1) {
        // Hoạt động bình thường: Nhấp nháy LED PA5
        GPIOA->ODR ^= (1 << 5);
        delay_ms(200);
    }
}

