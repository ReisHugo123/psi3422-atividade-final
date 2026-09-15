/*
 * enlace.h - protocolo de comando entre o computador e o carrinho, por radio.
 *
 * Mensagem de 8 bytes fixos, pedido e resposta no mesmo formato. O radio e half
 * duplex, entao quem fala espera calado: o controle manda e escuta, o carrinho
 * escuta e so fala para responder. Ver docs/protocolo.md.
 */
#ifndef ENLACE_H_
#define ENLACE_H_

#include <stdint.h>

#define ENL_PAYLOAD   8

/* Do controle para o carrinho. */
#define ENL_RUN       'R'
#define ENL_STOP      'S'
#define ENL_DIST      'D'
#define ENL_ZERA      'Z'

/* Do carrinho para o controle. */
#define ENL_RESP      'd'

/* Estados que a resposta carrega. */
#define ENL_PARADO    0
#define ENL_ANDANDO   1

typedef struct {
	uint8_t  cmd;
	uint8_t  estado;
	uint32_t dist_mm;
	uint8_t  motivo;    /* 0 normal, senao o odo_res_t que abortou a manobra */
	uint8_t  seq;
} enl_msg_t;

/* Liga o radio e deixa em escuta. */
void enl_init(void);

/* 1 quando havia mensagem. Nao bloqueia. */
int enl_recebe(enl_msg_t *m);

/* Troca para transmissao, manda, volta para escuta. 1 quando o ACK voltou, ou
 * seja quando o outro lado recebeu de verdade. */
int enl_envia(const enl_msg_t *m);

#endif /* ENLACE_H_ */
