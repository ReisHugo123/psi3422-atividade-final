/*
 * odometria.h - movimento medido, em cima de lib/motores e lib/encoder.
 *
 * Na atividade 1 o angulo era tempo, e tempo depende de piso, peso e carga da
 * bateria. Aqui o comando e em pulsos: a manobra acaba quando as rodas
 * giraram o quanto se pediu.
 *
 * Os dois numeros que traduzem pulso em mundo real se MEDEM, nao se calculam.
 * Procedimento e tabelas em docs/calibracao.md.
 */
#ifndef ODOMETRIA_H_
#define ODOMETRIA_H_

#include <stdint.h>
#include "encoder.h"

/* ==================== cartao de calibracao ==================== */

/* Geometria, medida no carrinho. O alvo do encoder nao e disco vazado: sao
 * 3 fitas isolantes pretas na roda esquerda e 4 na direita, lidas por
 * refletancia pelo HW-201. Numeros diferentes nos dois lados nao quebram nada,
 * porque o criterio de parada usa a MEDIA das duas contagens e nas duas
 * manobras as rodas percorrem a mesma distancia. Exige ODO_KP em 0. */
#define ODO_ABERTURAS_DISCO     4
#define ODO_BORDAS_POR_ABERT    2
#define ODO_PULSOS_POR_VOLTA   (ODO_ABERTURAS_DISCO * ODO_BORDAS_POR_ABERT)
#define ODO_DIAM_RODA_MM       65
#define ODO_ENTRE_RODAS_MM     170

/* MEDIDOS na atividade 4, laboratorio, piso do lab, alimentacao por power bank.
 * POR_M: 38 pulsos comandados deram 1212 mm, entao 31 pulsos por metro, ou
 * 32,3 mm por pulso. Confirmado no MODO 4, que repetiu 1000 mm e voltou para a
 * marca de partida.
 * PULSOS_90: com 3 o giro saiu em 65 graus, com 4 ficou perto de 90. Com 14
 * bordas por volta o angulo comandado so anda de ~22 em ~22 graus, entao 4 e o
 * degrau mais proximo de 90 que este encoder permite pedir.
 * Trocar de piso ou de alimentacao invalida os dois. */
#define ODO_PULSOS_POR_M       31
#define ODO_PULSOS_90           4

/* Escorregada depois do freio. Comeca em 0 porque o firmware mede a propria
 * escorregada e imprime; chute aqui esconde o efeito. */
#define ODO_ESCORREGO_RETO      0
#define ODO_ESCORREGO_GIRO      0

/* ==================== velocidades ==================== */
#define ODO_VEL_RETO           100
#define ODO_VEL_GIRO           100
#define ODO_VEL_MIN             40

/* Correcao de simetria, em % de velocidade por pulso de erro. Nasce em 0:
 * corrigir e sempre TIRAR velocidade, e com power bank de 5 V o motor ja esta
 * em ~3,2 V no duty maximo, entao baixar mais para a roda e a manobra aborta
 * com ODO_TRAVOU. Com bateria de 7,4 a 9 V, 2 a 4 funciona. Com power bank, a
 * correcao certa e TRIM_ESQ/TRIM_DIR em motores.c. */
#define ODO_KP                    0

/* Protecoes. O que protege de verdade e ODO_PARADO_MS: roda que para de contar
 * (bateu, atolou, encoder solto) aborta em menos de 1 s. */
#define ODO_TIMEOUT_MS        15000
#define ODO_PARADO_MS           800
#define ODO_ASSENTA_MS          300
#define ODO_PASSO_MS              2

/* ==================== resultado ==================== */

typedef enum {
	ODO_OK = 0,
	ODO_TIMEOUT,
	ODO_TRAVOU,
	ODO_PARAMETRO,
	ODO_ABORTADO,     /* outra thread pediu parada, tipicamente o comando STOP */
} odo_res_t;

typedef struct {
	odo_res_t res;
	uint32_t  alvo_esq, alvo_dir;
	uint32_t  freio_esq, freio_dir;   /* contagem no instante do freio */
	uint32_t  esq, dir;               /* contagem depois de assentar */
	uint32_t  ms;
} odo_relato_t;

/* ==================== interface ==================== */

uint32_t odo_pulsos_de_mm(uint32_t mm);
uint32_t odo_mm_de_pulsos(uint32_t pulsos);

void odo_init(void);
void odo_para(void);

/* mm > 0 anda para frente, mm < 0 de re. */
void odo_anda_mm(int32_t mm, int vel, odo_relato_t *r);

/* Contagem crua: e o modo de CALIBRAR distancia e angulo. */
void odo_anda_pulsos(uint32_t pulsos, int vel, int para_tras, odo_relato_t *r);
void odo_gira_pulsos(uint32_t pulsos, int vel, int para_direita, odo_relato_t *r);

/* Giro no proprio eixo. graus > 0 para a direita. */
void odo_gira_graus(int graus, int vel, odo_relato_t *r);

/* Arco de raio definido, medido no centro do carrinho. O raio tem de ser maior
 * que meio entre-rodas, senao a roda de dentro teria de girar para tras. */
void odo_curva_graus(int graus, uint32_t raio_mm, int vel_externa, odo_relato_t *r);

void odo_imprime(const char *o_que, const odo_relato_t *r);

/* Parada de emergencia pedida de FORA da thread que esta manobrando. E o que o
 * comando STOP precisa: sem isso a malha reescreve a velocidade dos motores a
 * cada 2 ms e desfaz qualquer freio dado por outra thread. A manobra em curso
 * morre no proximo passo da malha e devolve ODO_ABORTADO. */
void odo_aborta(void);
void odo_limpa_aborto(void);

#endif /* ODOMETRIA_H_ */
