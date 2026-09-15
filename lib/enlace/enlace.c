/*
 * enlace.c - empacota e desempacota a mensagem de 8 bytes.
 *
 * Os dois lados usam o MESMO endereco, canal, taxa e tamanho de payload, senao
 * os radios nao se enxergam e nada avisa. Por isso as tres constantes vivem aqui
 * e nao em cada main.
 */
#include "enlace.h"

#include <nrf24.h>
#include <zephyr/kernel.h>

static const uint8_t ENDERECO[NRF24_ADDR_LEN] = { 'K', 'L', '2', '5', 'C' };
#define CANAL   76

void enl_init(void)
{
	nrf24_init();
	nrf24_config(ENDERECO, CANAL, ENL_PAYLOAD);
	nrf24_modo_rx();
}

static void empacota(const enl_msg_t *m, uint8_t *p)
{
	p[0] = m->cmd;
	p[1] = m->estado;
	p[2] = (uint8_t)(m->dist_mm);
	p[3] = (uint8_t)(m->dist_mm >> 8);
	p[4] = (uint8_t)(m->dist_mm >> 16);
	p[5] = (uint8_t)(m->dist_mm >> 24);
	p[6] = m->motivo;
	p[7] = m->seq;
}

static void desempacota(const uint8_t *p, enl_msg_t *m)
{
	m->cmd     = p[0];
	m->estado  = p[1];
	m->dist_mm = (uint32_t)p[2] | ((uint32_t)p[3] << 8)
		   | ((uint32_t)p[4] << 16) | ((uint32_t)p[5] << 24);
	m->motivo  = p[6];
	m->seq     = p[7];
}

int enl_recebe(enl_msg_t *m)
{
	uint8_t p[ENL_PAYLOAD];

	if (!nrf24_recebe(p, ENL_PAYLOAD)) {
		return 0;
	}
	desempacota(p, m);

	return 1;
}

int enl_envia(const enl_msg_t *m)
{
	uint8_t p[ENL_PAYLOAD];
	int ok;

	empacota(m, p);

	nrf24_modo_tx();
	ok = nrf24_envia(p, ENL_PAYLOAD);
	/* volta para escuta na mesma funcao: esquecer isso deixa o lado surdo, e o
	 * sintoma e "o primeiro comando funciona e o segundo nao" */
	nrf24_modo_rx();

	return ok;
}
