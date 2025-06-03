#include <stdint.h>

// Memory-mapped addresses for GPIO, ADC, and PWM registers
#define SYSCTL_RCGCADC_R       (*((volatile uint32_t *)0x400FE638))
#define SYSCTL_RCGCGPIO_R      (*((volatile uint32_t *)0x400FE608))
#define GPIO_PORTE_DIR_R       (*((volatile uint32_t *)0x40024400))
#define GPIO_PORTE_AFSEL_R     (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_DEN_R       (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R     (*((volatile uint32_t *)0x40024528))
#define ADC0_ACTSS_R           (*((volatile uint32_t *)0x40038000))
#define ADC0_EMUX_R            (*((volatile uint32_t *)0x40038014))
#define ADC0_SSMUX3_R          (*((volatile uint32_t *)0x400380A0))
#define ADC0_SSCTL3_R          (*((volatile uint32_t *)0x400380A4))
#define ADC0_IM_R              (*((volatile uint32_t *)0x40038008))
#define ADC0_ISC_R             (*((volatile uint32_t *)0x4003800C))
#define ADC0_SSFIFO3_R         (*((volatile uint32_t *)0x400380A8))
#define SYSCTL_RCGCPWM_R       (*((volatile uint32_t *)0x400FE640))
#define SYSCTL_RCGCGPIO_PWM    (*((volatile uint32_t *)0x400FE608))
#define GPIO_PORTF_AFSEL_R     (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_PCTL_R      (*((volatile uint32_t *)0x4002552C))
#define PWM1_CTL_R             (*((volatile uint32_t *)0x40028000))
#define PWM1_GENA_R            (*((volatile uint32_t *)0x40028060))
#define PWM1_LOAD_R            (*((volatile uint32_t *)0x40028050))
#define PWM1_CMPA_R            (*((volatile uint32_t *)0x40028058))
#define PWM1_CTL_ENABLE        0x00000001
#define PWM1_GENA_ACTCMPAD_ZERO 0x000000C
#define PWM1_GENA_ACTLOAD_ONE  0x0000008

void ConfigureSystemClock(void);
void ConfigureADC(void);
void ConfigurePWM(void);
void ADC0_Init(void);
uint32_t ADC0_InSeq3(void);
void PWM1_Duty(uint16_t duty);

int main(void) {
    ConfigureSystemClock();
    ConfigureADC();
    ConfigurePWM();

    uint32_t adcResult;

    while (1) {
        adcResult = ADC0_InSeq3(); // Read ADC value

        // Map ADC value (0 - 4095) to PWM range (0 - 1000)
        uint16_t duty = (uint16_t)((adcResult * 1000) / 4095);
        PWM1_Duty(duty);
    }
}

void ConfigureSystemClock(void) {
    // Set the system clock to 80MHz
    SYSCTL_RCGCADC_R |= 0x0001; // Enable ADC0 clock
    SYSCTL_RCGCGPIO_R |= 0x10;  // Enable GPIO port E clock
}

void ConfigureADC(void) {
    GPIO_PORTE_DIR_R &= ~0x08;  // Make PE3 input
    GPIO_PORTE_AFSEL_R |= 0x08; // Enable alternate function on PE3
    GPIO_PORTE_DEN_R &= ~0x08;  // Disable digital function on PE3
    GPIO_PORTE_AMSEL_R |= 0x08; // Enable analog function on PE3

    ADC0_Init(); // Initialize ADC0
}

void ConfigurePWM(void) {
    SYSCTL_RCGCPWM_R |= 0x02; // Enable PWM1 clock
    SYSCTL_RCGCGPIO_PWM |= 0x20; // Enable GPIO port F clock

    GPIO_PORTF_AFSEL_R |= 0x02; // Enable alternate function on PF1
    GPIO_PORTF_PCTL_R |= 0x00000050; // Set PF1 as PWM output

    PWM1_CTL_R &= ~PWM1_CTL_ENABLE; // Disable PWM1
    PWM1_GENA_R = (PWM1_GENA_ACTCMPAD_ZERO | PWM1_GENA_ACTLOAD_ONE);
    PWM1_LOAD_R = 1000 - 1; // Set period to 1000 (PWM_FREQUENCY = 1kHz)
    PWM1_CMPA_R = 0; // Initialize PWM with 0% duty cycle
    PWM1_CTL_R |= PWM1_CTL_ENABLE; // Enable PWM1
}

void ADC0_Init(void) {
    ADC0_ACTSS_R &= ~0x0008; // Disable sample sequencer 3
    ADC0_EMUX_R &= ~0xF000;  // Set software trigger
    ADC0_SSMUX3_R = 0x000A;  // Set channel Ain10 (PE3)
    ADC0_SSCTL3_R = 0x0006;  // Set flag and end bits
    ADC0_ACTSS_R |= 0x0008;  // Enable sample sequencer 3
}

uint32_t ADC0_InSeq3(void) {
    uint32_t result;
    ADC0_ACTSS_R = 0x0008; // Initiate SS3
    result = ADC0_SSFIFO3_R & 0xFFF; // Read result
    ADC0_ISC_R = 0x0008; // Acknowledge completion
    return result;
}

void PWM1_Duty(uint16_t duty) {
    PWM1_CMPA_R = duty; // Set PWM duty cycle
}
