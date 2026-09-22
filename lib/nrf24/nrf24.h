/*
 * nRF24L01+ na FRDM-KL25Z. SPI0 pela biblioteca spi.c da disciplina.
 *
 *   VCC  3V3    J9-08    maximo 3,6 V - o vizinho J9-10 e 5 V e queima o modulo
 *   GND  GND    J9-12
 *   SCK  PTC5   J1-09        MOSI PTC6  J1-11        MISO PTC7  J1-01
 *   CE   PTD4   J1-06        CSN  PTA4  J1-10        IRQ  PTA12 J1-08 (nao usado)
 *
 * PINOS DA PLACA DA ATIVIDADE 3, e nao os da atividade 2. La o radio estava em
 * PTD0, PTD2, PTD3 e PTD5, que sao os pinos dos motores, e e essa colisao que fez
 * a placa existir. Indo para o SPI0 nativo de PORTC o remendo da atividade 2
 * some, e a biblioteca de SPI da disciplina passa a servir como foi entregue.
 *
 * CE e CSN sao GPIO: o CSN tem de ficar baixo pelo comando inteiro (comando mais
 * ate 32 bytes) e o chip select automatico do KL25Z sobe entre bytes.
 */
#ifndef NRF24_H_
#define NRF24_H_

#include <stdint.h>

/* Comandos - tabela 20 do datasheet. */
#define NRF24_CMD_R_REGISTER    0x00u  /* OR com o endereco */
#define NRF24_CMD_W_REGISTER    0x20u  /* OR com o endereco */
#define NRF24_CMD_R_RX_PAYLOAD  0x61u
#define NRF24_CMD_W_TX_PAYLOAD  0xA0u
#define NRF24_CMD_FLUSH_TX      0xE1u
#define NRF24_CMD_FLUSH_RX      0xE2u
#define NRF24_CMD_NOP           0xFFu

/* Registradores - tabela 28. */
#define NRF24_REG_CONFIG        0x00u
#define NRF24_REG_EN_AA         0x01u
#define NRF24_REG_EN_RXADDR     0x02u
#define NRF24_REG_SETUP_AW      0x03u
#define NRF24_REG_SETUP_RETR    0x04u
#define NRF24_REG_RF_CH         0x05u
#define NRF24_REG_RF_SETUP      0x06u
#define NRF24_REG_STATUS        0x07u
#define NRF24_REG_OBSERVE_TX    0x08u
#define NRF24_REG_RX_ADDR_P0    0x0Au
#define NRF24_REG_RX_ADDR_P1    0x0Bu
#define NRF24_REG_TX_ADDR       0x10u
#define NRF24_REG_RX_PW_P0      0x11u
#define NRF24_REG_RX_PW_P1      0x12u
#define NRF24_REG_FIFO_STATUS   0x17u

#define NRF24_CONFIG_EN_CRC     0x08u
#define NRF24_CONFIG_CRCO       0x04u  /* CRC de 2 bytes */
#define NRF24_CONFIG_PWR_UP     0x02u
#define NRF24_CONFIG_PRIM_RX    0x01u

#define NRF24_STATUS_RX_DR      0x40u
#define NRF24_STATUS_TX_DS      0x20u
#define NRF24_STATUS_MAX_RT     0x10u
#define NRF24_STATUS_IRQS       0x70u  /* os tres, para limpar de uma vez */

#define NRF24_FIFO_RX_EMPTY     0x01u
#define NRF24_FIFO_RX_FULL      0x02u

#define NRF24_ADDR_LEN          5u

void nrf24_init(void);
void nrf24_ce(int nivel);
int nrf24_irq_ativo(void);

uint8_t nrf24_spi(uint8_t out);
uint8_t nrf24_status(void);
uint8_t nrf24_cmd(uint8_t cmd);
uint8_t nrf24_read_reg(uint8_t reg);
void nrf24_write_reg(uint8_t reg, uint8_t valor);

/* Registrador de endereco: buf[0] e o LSByte e sai primeiro no fio. */
void nrf24_read_regs(uint8_t reg, uint8_t *buf, uint8_t n);
void nrf24_write_regs(uint8_t reg, const uint8_t *buf, uint8_t n);

/*
 * Configuracao comum aos dois lados: os dois radios precisam do mesmo endereco,
 * canal, taxa, tamanho de payload e CRC, senao nao se enxergam. Deixa o radio
 * ligado em standby-I.
 */
void nrf24_config(const uint8_t *endereco, uint8_t canal, uint8_t payload_len);

void nrf24_modo_tx(void);
void nrf24_modo_rx(void);

/* 1 quando o ACK chegou, ou seja o pacote foi entregue de verdade. */
int nrf24_envia(const uint8_t *buf, uint8_t n);

/* 1 quando havia pacote na FIFO. Nao bloqueia. */
int nrf24_recebe(uint8_t *buf, uint8_t n);

#endif /* NRF24_H_ */
