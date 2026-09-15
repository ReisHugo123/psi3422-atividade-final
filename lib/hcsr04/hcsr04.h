/*
 * hcsr04.h - sensor de ultrassom HC-SR04 na FRDM-KL25Z.
 *
 * Vem de uma atividade da disciplina de Arquitetura de Sistemas Embarcados
 * (trigger por PWM e echo por input capture no TPM1). A diferenca aqui e a
 * espera: em vez de busy-wait no contador, a interrupcao libera um semaforo e a
 * thread dorme ate a medida ficar pronta, para o sensor nao roubar CPU da
 * navegacao.
 *
 * Ligacoes: VCC -> 3V3, GND -> GND, TRIG -> PTB0 (A0), ECHO -> PTB1 (A1).
 * O sensor e alimentado com 3,3 V de proposito: assim o ECHO sai em 3,3 V e vai
 * direto no pino, sem divisor de tensao.
 */
#ifndef HCSR04_H_
#define HCSR04_H_

#include <stdint.h>

/* Sem echo: nada refletiu dentro do alcance. */
#define HCSR04_TIMEOUT   (-1)

/* TPM1: CH0 gera o trigger (pulso de ~21 us a cada 70 ms) e CH1 mede o echo. */
void hcsr04_init(void);

/* Largura do proximo pulso de echo em us, ou HCSR04_TIMEOUT. Bloqueia a thread. */
int32_t hcsr04_read_us(void);

/* Distancia em milimetros, ou HCSR04_TIMEOUT. */
int32_t hcsr04_read_mm(void);

#endif /* HCSR04_H_ */
