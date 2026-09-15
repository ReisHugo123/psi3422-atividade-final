/*
 * hcsr04.c - HC-SR04 pelos registradores do TPM1.
 *
 * Trigger: TPM1_CH0 (PTB0) em PWM, pulso de ~21 us a cada 70 ms.
 * Echo:    TPM1_CH1 (PTB1) em input capture nas duas bordas, com interrupcao.
 *          A ISR guarda o instante da subida e o da descida; a largura do pulso
 *          e proporcional a distancia.
 *
 * Base de tempo: 48 MHz / 128 = 375 kHz, ou seja 1 tick = 8/3 us.
 * Distancia: d_mm = us * 10 / 58 (o som gasta ~58 us por cm, ida e volta).
 *
 * Detalhe que custou caro na atividade original: o flag de captura CHF nao limpa
 * por polling nesta placa - o CnV congela por protecao de sobrescrita. So limpa
 * de dentro da ISR, escrevendo no TPM1->STATUS.
 */
#include "hcsr04.h"
#include <MKL25Z4.h>
#include <zephyr/kernel.h>
#include <zephyr/irq.h>

#define HCSR04_TPM        TPM1
#define HCSR04_TRIG_CH    0u
#define HCSR04_ECHO_CH    1u
#define HCSR04_TRIG_PIN   0u       /* PTB0 = TPM1_CH0 (ALT3) */
#define HCSR04_ECHO_PIN   1u       /* PTB1 = TPM1_CH1 (ALT3) */
#define HCSR04_ECHO_SF    (1u << HCSR04_ECHO_CH)
#define TPM1_IRQ          18u

#define TPM_SRC_FLL       1u
#define PS_128            7u
#define PERIODO_TICKS     26250u   /* 70 ms */
#define TRIG_TICKS        8u       /* ~21 us, o datasheet pede no minimo 10 */

/* Alcance do sensor: ~4 m ida e volta da ~23 ms. Largura maior que isso quer
 * dizer que as duas bordas capturadas nao eram do mesmo pulso (a ISR saiu de
 * fase); descarta e o proximo trigger ressincroniza. */
#define ECHO_MAX_TICKS    11250u   /* 30 ms */

#define ESPERA_MS         200      /* dois periodos de trigger, com folga */

static volatile uint32_t t_subida = 0u;
static volatile uint32_t largura  = 0u;   /* ultimo pulso, em ticks */
static volatile uint8_t  estado   = 0u;   /* 0 = espera subida, 1 = espera descida */

static struct k_sem sem_medida;

static void tpm1_isr(const void *arg)
{
	ARG_UNUSED(arg);

	if (HCSR04_TPM->STATUS & HCSR04_ECHO_SF) {
		uint32_t agora = HCSR04_TPM->CONTROLS[HCSR04_ECHO_CH].CnV;
		HCSR04_TPM->STATUS = HCSR04_ECHO_SF;

		if (estado == 0u) {
			t_subida = agora;
			estado   = 1u;
		} else {
			uint32_t w = (agora >= t_subida)
					? (agora - t_subida)
					: (agora + PERIODO_TICKS - t_subida);
			estado = 0u;

			if (w <= ECHO_MAX_TICKS) {
				largura = w;
				k_sem_give(&sem_medida);
			}
		}
	}
}

void hcsr04_init(void)
{
	k_sem_init(&sem_medida, 0, 1);

	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM->SOPT2  = (SIM->SOPT2 & ~SIM_SOPT2_TPMSRC_MASK) | SIM_SOPT2_TPMSRC(TPM_SRC_FLL);

	PORTB->PCR[HCSR04_TRIG_PIN] = PORT_PCR_MUX(3);
	PORTB->PCR[HCSR04_ECHO_PIN] = PORT_PCR_MUX(3);

	HCSR04_TPM->SC  = 0u;
	HCSR04_TPM->CNT = 0u;
	HCSR04_TPM->MOD = PERIODO_TICKS - 1u;

	/* CH0: PWM, alto enquanto CNT < CnV -> o trigger periodico */
	HCSR04_TPM->CONTROLS[HCSR04_TRIG_CH].CnSC = TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	HCSR04_TPM->CONTROLS[HCSR04_TRIG_CH].CnV  = TRIG_TICKS;

	/* CH1: captura nas duas bordas (ELSA|ELSB) com interrupcao (CHIE) */
	HCSR04_TPM->CONTROLS[HCSR04_ECHO_CH].CnSC =
		TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK;
	HCSR04_TPM->STATUS = HCSR04_ECHO_SF;

	IRQ_CONNECT(TPM1_IRQ, 1, tpm1_isr, NULL, 0);
	irq_enable(TPM1_IRQ);

	HCSR04_TPM->SC = TPM_SC_CMOD(1) | TPM_SC_PS(PS_128);
}

int32_t hcsr04_read_us(void)
{
	/* joga fora a medida anterior: quem chama quer a distancia de agora */
	k_sem_reset(&sem_medida);

	if (k_sem_take(&sem_medida, K_MSEC(ESPERA_MS)) != 0) {
		return HCSR04_TIMEOUT;
	}

	unsigned int chave = irq_lock();
	uint32_t ticks = largura;
	irq_unlock(chave);

	return (int32_t)((ticks * 8u) / 3u);
}

int32_t hcsr04_read_mm(void)
{
	int32_t us = hcsr04_read_us();

	if (us < 0) {
		return HCSR04_TIMEOUT;
	}
	return (int32_t)(((uint32_t)us * 10u) / 58u);
}
