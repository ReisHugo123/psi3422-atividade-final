/*
 * PSI3422 - Entrega final: carrinho comandado por radio, atravessando labirinto.
 *
 * O mesmo projeto gera os dois programas. O #define LADO escolhe qual:
 *
 *   LADO_CONTROLE   placa ligada no PC. Le o teclado do terminal, manda o
 *                   comando por radio e imprime a resposta que volta.
 *   LADO_CARRINHO   placa do carrinho. Escuta o radio, obedece, e responde.
 *
 * Comandos: R roda, S para, D mostra a distancia, Z zera a distancia.
 * No carrinho as mesmas letras valem pelo terminal USB, para dar para provar o
 * labirinto antes de o radio entrar.
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include <enlace.h>
#include <encoder.h>
#include <hcsr04.h>
#include <motores.h>
#include <odometria.h>

#define LADO_CONTROLE   1
#define LADO_CARRINHO   2

#define LADO   LADO_CARRINHO

/* ---- labirinto ---- */
#define DIST_PARE_MM     150   /* parede a menos que isso e bloqueio */
#define PASSO_MM         120   /* avanco entre duas leituras do sonar */
#define RECUO_MM          60   /* recua antes de girar, para nao raspar */
#define VEL_RETO         ODO_VEL_RETO
#define VEL_GIRO         ODO_VEL_GIRO

#define ESPERA_RESP_MS   400   /* quanto o controle espera pela resposta */

static const struct device *console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static const struct gpio_dt_spec led_verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_azul  = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_verm  = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

enum cor {
	APAGADO,
	VERDE,
	VERMELHO,
	AZUL,
	AMARELO,
};

static void leds_init(void)
{
	gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_azul,  GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_verm,  GPIO_OUTPUT_INACTIVE);
}

static void led_cor(enum cor c)
{
	gpio_pin_set_dt(&led_verde, (c == VERDE)    || (c == AMARELO));
	gpio_pin_set_dt(&led_verm,  (c == VERMELHO) || (c == AMARELO));
	gpio_pin_set_dt(&led_azul,   c == AZUL);
}

/* Le uma tecla sem bloquear. -1 quando nao ha nada. */
static int tecla(void)
{
	unsigned char c;

	return (uart_poll_in(console, &c) == 0) ? (int)c : -1;
}

static int letra_para_cmd(int c)
{
	switch (c) {
	case 'r': case 'R': return ENL_RUN;
	case 's': case 'S': return ENL_STOP;
	case 'd': case 'D': return ENL_DIST;
	case 'z': case 'Z': return ENL_ZERA;
	}
	return 0;
}

static void ajuda(void)
{
	printk("\ncomandos:  R roda   S para   D distancia   Z zera a distancia\n");
}

/* ===================== LADO DO CONTROLE ===================== */
#if LADO == LADO_CONTROLE

static void mostra(const enl_msg_t *m)
{
	printk("  carrinho: %s | distancia %u mm (%u,%03u m)",
	       m->estado == ENL_ANDANDO ? "RODANDO" : "PARADO",
	       m->dist_mm, m->dist_mm / 1000u, m->dist_mm % 1000u);
	if (m->motivo) {
		printk(" | ultima manobra terminou com codigo %u", m->motivo);
	}
	printk("\n");
}

int main(void)
{
	enl_msg_t env = { 0 }, resp;

	leds_init();
	led_cor(AZUL);
	enl_init();

	printk("\n=== PSI3422 - controle remoto do carrinho ===\n");
	printk("radio: SCK=PTC5 MOSI=PTC6 MISO=PTC7 CSN=PTA4 CE=PTD4\n");
	ajuda();

	while (1) {
		int c = tecla();
		int cmd = letra_para_cmd(c);

		if (cmd) {
			env.cmd = (uint8_t)cmd;
			env.seq++;
			printk("\n-> %c\n", cmd);

			if (!enl_envia(&env)) {
				led_cor(VERMELHO);
				printk("  SEM ACK: o carrinho nao recebeu. Ligado? Mesmo canal?\n");
			} else {
				led_cor(VERDE);
				uint32_t t0 = k_uptime_get_32();
				int veio = 0;

				while ((k_uptime_get_32() - t0) < ESPERA_RESP_MS) {
					if (enl_recebe(&resp) && resp.cmd == ENL_RESP) {
						mostra(&resp);
						veio = 1;
						break;
					}
					k_msleep(2);
				}
				if (!veio) {
					printk("  entregue, mas sem resposta dentro de %d ms\n",
					       ESPERA_RESP_MS);
				}
			}
		} else if (c >= 0) {
			ajuda();
		}

		k_msleep(5);
	}
	return 0;
}

/* ===================== LADO DO CARRINHO ===================== */
#else

static volatile int estado = ENL_PARADO;
static volatile uint8_t motivo;
static uint32_t odometro_mm;
K_MUTEX_DEFINE(mtx_odo);

static void soma_odometro(const odo_relato_t *r)
{
	/* Sempre em MODULO: o enunciado diz que dar meia-volta nao reduz a distancia
	 * percorrida, e recuar tambem nao. Giro no proprio eixo nao entra, porque o
	 * carrinho nao sai do lugar. */
	uint32_t media = (r->esq + r->dir + 1u) / 2u;

	k_mutex_lock(&mtx_odo, K_FOREVER);
	odometro_mm += odo_mm_de_pulsos(media);
	k_mutex_unlock(&mtx_odo);
}

static uint32_t le_odometro(void)
{
	uint32_t v;

	k_mutex_lock(&mtx_odo, K_FOREVER);
	v = odometro_mm;
	k_mutex_unlock(&mtx_odo);

	return v;
}

static void responde(void)
{
	enl_msg_t r = {
		.cmd     = ENL_RESP,
		.estado  = (uint8_t)estado,
		.dist_mm = le_odometro(),
		.motivo  = motivo,
	};

	/* o outro lado precisa de um instante para sair de TX e voltar a escutar */
	k_msleep(3);
	enl_envia(&r);
}

static void aplica(int cmd)
{
	switch (cmd) {
	case ENL_RUN:
		odo_limpa_aborto();
		estado = ENL_ANDANDO;
		motivo = 0;
		printk("[RUN]\n");
		break;
	case ENL_STOP:
		estado = ENL_PARADO;
		odo_aborta();          /* corta a manobra em curso, nao espera ela acabar */
		printk("[STOP]\n");
		break;
	case ENL_ZERA:
		k_mutex_lock(&mtx_odo, K_FOREVER);
		odometro_mm = 0;
		k_mutex_unlock(&mtx_odo);
		printk("[ZERA]\n");
		break;
	case ENL_DIST:
		printk("[DIST] %u mm\n", le_odometro());
		break;
	}
}

/* Thread do radio: prioridade maior que a da navegacao para o STOP entrar mesmo
 * com o carrinho no meio de uma manobra. */
static void thread_radio(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	while (1) {
		enl_msg_t m;

		if (enl_recebe(&m)) {
			aplica(m.cmd);
			responde();
		}

		int cmd = letra_para_cmd(tecla());

		if (cmd) {
			aplica(cmd);
		}

		k_msleep(10);
	}
}

static int livre(void)
{
	int32_t mm = hcsr04_read_mm();

	/* sem eco quer dizer que nada refletiu dentro do alcance, ou seja caminho
	 * livre. E o certo na maioria dos casos, e erra com parede em diagonal */
	return (mm == HCSR04_TIMEOUT) || (mm > DIST_PARE_MM);
}

/* 1 quando a manobra nao terminou bem e a navegacao tem de parar. */
static int falhou(const odo_relato_t *r, const char *o_que)
{
	if (r->res == ODO_OK) {
		return 0;
	}
	/* aborto a pedido nao e falha: e o STOP funcionando. So falha de verdade
	 * acende o vermelho e fica gravada no motivo que volta pelo radio. */
	if (r->res != ODO_ABORTADO) {
		motivo = (uint8_t)r->res;
	}
	estado = ENL_PARADO;
	odo_imprime(o_que, r);
	return 1;
}

/* Um passo do labirinto. Livre, anda. Bloqueado, recua e procura saida: tenta a
 * direita, depois a esquerda, e se as duas fecharem volta por onde veio. */
static void passo_labirinto(void)
{
	odo_relato_t r;

	if (livre()) {
		led_cor(VERDE);
		odo_anda_mm(PASSO_MM, VEL_RETO, &r);
		soma_odometro(&r);
		falhou(&r, "avanco");
		return;
	}

	led_cor(AMARELO);
	odo_anda_mm(-RECUO_MM, VEL_RETO, &r);
	soma_odometro(&r);
	if (falhou(&r, "recuo")) {
		return;
	}

	odo_gira_graus(90, VEL_GIRO, &r);
	if (falhou(&r, "vira a direita") || livre()) {
		return;
	}

	odo_gira_graus(-180, VEL_GIRO, &r);
	if (falhou(&r, "vira a esquerda") || livre()) {
		return;
	}

	/* beco sem saida: os tres lados fechados, volta pelo caminho */
	odo_gira_graus(-90, VEL_GIRO, &r);
	falhou(&r, "meia-volta");
}

static void thread_nav(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	while (1) {
		if (estado == ENL_ANDANDO) {
			passo_labirinto();
		} else {
			carrinho_para();
			led_cor(motivo ? VERMELHO : AZUL);
			k_msleep(50);
		}
	}
}

#define STACK_SZ   1536

K_THREAD_DEFINE(radio_tid, STACK_SZ, thread_radio, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(nav_tid,   STACK_SZ, thread_nav,   NULL, NULL, NULL, 6, 0, 0);

int main(void)
{
	leds_init();
	led_cor(AZUL);

	odo_init();          /* motores + encoder */
	hcsr04_init();
	enl_init();

	printk("\n=== PSI3422 - carrinho (entrega final) ===\n");
	printk("motores : ENA=PTD2 IN1=PTD0 IN2=PTD5 | ENB=PTD3 IN3=PTE0 IN4=PTE1\n");
	printk("encoders: ENC_ESQ=PTD6 (J2-17)  ENC_DIR=PTD7 (J2-19)\n");
	printk("sonar   : TRIG=PTB0 (A0)  ECHO=PTB1 (A1)\n");
	printk("radio   : SCK=PTC5 MOSI=PTC6 MISO=PTC7 CSN=PTA4 CE=PTD4\n");
	printk("calibracao: %d pulsos/m | %d pulsos por 90 graus\n",
	       ODO_PULSOS_POR_M, ODO_PULSOS_90);
	printk("labirinto: para a menos de %d mm, passo de %d mm\n",
	       DIST_PARE_MM, PASSO_MM);
	ajuda();
	printk("estado inicial: PARADO\n");

	return 0;
}

#endif
