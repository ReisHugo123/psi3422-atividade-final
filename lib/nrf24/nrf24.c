#include <zephyr/kernel.h>
#include <MKL25Z4.h>
#include <spi.h>
#include <nrf24.h>

#define PIN_CSN   4u   /* PTA4,  J1-10 */
#define PIN_CE    4u   /* PTD4,  J1-06 */
#define PIN_IRQ   12u  /* PTA12, J1-08 */

/* SPPR e SPR como campo do registrador, nao como divisor: 2 e 2 dao
 * 24 MHz / (3 * 8) = 1 MHz. As constantes PRESCALE_n e DIVISOR_n do spi.h guardam
 * o divisor e estouram a mascara do campo. */
#define SPI_SPPR  2
#define SPI_SPR   2

/* Tcwh: 50 ns de CSN alto entre comandos. */
static void pausa(void)
{
	for (int i = 0; i < 8; i++) {
		__asm volatile("nop");
	}
}

static inline void csn_baixo(void) { PTA->PCOR = 1u << PIN_CSN; }
static inline void csn_alto(void)  { PTA->PSOR = 1u << PIN_CSN; pausa(); }

static void escreve(uint8_t cmd, const uint8_t *buf, uint8_t n)
{
	csn_baixo();
	nrf24_spi(cmd);
	for (uint8_t i = 0; i < n; i++) {
		nrf24_spi(buf[i]);
	}
	csn_alto();
}

static void le(uint8_t cmd, uint8_t *buf, uint8_t n)
{
	csn_baixo();
	nrf24_spi(cmd);
	for (uint8_t i = 0; i < n; i++) {
		buf[i] = nrf24_spi(NRF24_CMD_NOP);
	}
	csn_alto();
}

void nrf24_init(void)
{
	/* PORTC inteiro, que e o que o spi_init da disciplina oferece e o que a placa
	 * roteou. Nada de remendo aqui: SCK em PTC5, MOSI em PTC6, MISO em PTC7. */
	spi_init(SPI_0, ALT_0, SPI_SPPR, SPI_SPR, CS_MAN);

	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK
	            | SIM_SCGC5_PORTD_MASK;

	/* Pull-up no MISO: o modulo aciona forte e ignora, mas com a linha solta a
	 * leitura vira 0xFF em vez de flutuar. Separa "ninguem aciona o MISO" de "MISO
	 * preso em zero", que sem pull leriam igual. */
	PORTC->PCR[7] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	/* Com chip select manual o pino de SS tem de ser GPIO comum, e isso pede
	 * MODFEN = 0. Com MODFEN = 1 ele vira entrada de mode fault, cujo disparo
	 * limpa MSTR e SPE. */
	SPI0->C2 = 0;

	/* PTA4 sai do reset como NMI, que e ativo em nivel BAIXO. Muxar para GPIO
	 * desliga o NMI nesse pino, e e por isso que o CSN pode morar aqui: ele e
	 * saida nossa e fica alto em repouso. O que NAO podia era o IRQ do radio,
	 * que e ativo em baixo e dispararia NMI a cada pacote recebido. */
	PORTA->PCR[PIN_CSN] = PORT_PCR_MUX(1);
	PORTD->PCR[PIN_CE] = PORT_PCR_MUX(1);
	/* Pull-up no IRQ para que, desconectado, leia nivel inativo em vez de flutuar. */
	PORTA->PCR[PIN_IRQ] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	PTA->PSOR = 1u << PIN_CSN;
	PTD->PCOR = 1u << PIN_CE;
	PTA->PDDR |= (1u << PIN_CSN);
	PTD->PDDR |= (1u << PIN_CE);
	PTA->PDDR &= ~(1u << PIN_IRQ);
}

void nrf24_ce(int nivel)
{
	if (nivel) {
		PTD->PSOR = 1u << PIN_CE;
	} else {
		PTD->PCOR = 1u << PIN_CE;
	}
}

int nrf24_irq_ativo(void)
{
	return (PTA->PDIR & (1u << PIN_IRQ)) ? 0 : 1;
}

/* Sempre em par: o STATUS volta por MISO no mesmo byte que o comando sai por
 * MOSI, e um send sem o read deixa o SPRF com byte velho, deslocando as leituras
 * seguintes. */
uint8_t nrf24_spi(uint8_t out)
{
	spi_send(SPI_0, out);
	return spi_read(SPI_0);
}

uint8_t nrf24_status(void)
{
	return nrf24_cmd(NRF24_CMD_NOP);
}

uint8_t nrf24_cmd(uint8_t cmd)
{
	uint8_t st;

	csn_baixo();
	st = nrf24_spi(cmd);
	csn_alto();

	return st;
}

uint8_t nrf24_read_reg(uint8_t reg)
{
	uint8_t v;

	le(NRF24_CMD_R_REGISTER | (reg & 0x1Fu), &v, 1);
	return v;
}

void nrf24_write_reg(uint8_t reg, uint8_t valor)
{
	escreve(NRF24_CMD_W_REGISTER | (reg & 0x1Fu), &valor, 1);
}

void nrf24_read_regs(uint8_t reg, uint8_t *buf, uint8_t n)
{
	le(NRF24_CMD_R_REGISTER | (reg & 0x1Fu), buf, n);
}

void nrf24_write_regs(uint8_t reg, const uint8_t *buf, uint8_t n)
{
	escreve(NRF24_CMD_W_REGISTER | (reg & 0x1Fu), buf, n);
}

void nrf24_config(const uint8_t *endereco, uint8_t canal, uint8_t payload_len)
{
	nrf24_ce(0);

	/* Registrador de configuracao so aceita escrita em power down ou standby, e o
	 * radio entra em power down (PWR_UP = 0) apos o reset. Por isso o PWR_UP fica
	 * para o fim. */
	nrf24_write_reg(NRF24_REG_CONFIG, NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO);
	nrf24_write_reg(NRF24_REG_EN_AA, 0x01);      /* auto ACK so no pipe 0 */
	nrf24_write_reg(NRF24_REG_EN_RXADDR, 0x01);  /* so o pipe 0 habilitado */
	nrf24_write_reg(NRF24_REG_SETUP_AW, 0x03);   /* endereco de 5 bytes */
	/* ARD 500 us e ARC 15 retransmissoes. 500 us cobre qualquer tamanho de ACK em
	 * 1 Mbps; com os 250 us de reset o ACK maior que 5 bytes nao caberia. */
	nrf24_write_reg(NRF24_REG_SETUP_RETR, 0x1F);
	nrf24_write_reg(NRF24_REG_RF_CH, canal);
	/* 1 Mbps e 0 dBm. 1 Mbps em vez de 2 porque tem 3 dB mais de sensibilidade no
	 * receptor, e aqui alcance vale mais que taxa. */
	nrf24_write_reg(NRF24_REG_RF_SETUP, 0x06);

	/* No PTX o RX_ADDR_P0 tem de ser igual ao TX_ADDR, senao o ACK que volta nao e
	 * aceito e toda transmissao parece falhar. */
	nrf24_write_regs(NRF24_REG_TX_ADDR, endereco, NRF24_ADDR_LEN);
	nrf24_write_regs(NRF24_REG_RX_ADDR_P0, endereco, NRF24_ADDR_LEN);
	nrf24_write_reg(NRF24_REG_RX_PW_P0, payload_len);

	nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_IRQS);
	nrf24_cmd(NRF24_CMD_FLUSH_TX);
	nrf24_cmd(NRF24_CMD_FLUSH_RX);

	nrf24_write_reg(NRF24_REG_CONFIG,
			NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO | NRF24_CONFIG_PWR_UP);
	k_busy_wait(2000);  /* Tpd2stby: 1,5 ms saindo de power down */
}

void nrf24_modo_tx(void)
{
	nrf24_ce(0);
	nrf24_write_reg(NRF24_REG_CONFIG,
			nrf24_read_reg(NRF24_REG_CONFIG) & ~NRF24_CONFIG_PRIM_RX);
}

void nrf24_modo_rx(void)
{
	nrf24_write_reg(NRF24_REG_CONFIG,
			nrf24_read_reg(NRF24_REG_CONFIG) | NRF24_CONFIG_PRIM_RX);
	nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_IRQS);
	nrf24_cmd(NRF24_CMD_FLUSH_RX);

	/* No RX o CE fica alto o tempo todo: e ele que mantem o radio escutando. */
	nrf24_ce(1);
	k_busy_wait(130);  /* Tstby2a */
}

int nrf24_envia(const uint8_t *buf, uint8_t n)
{
	uint8_t st = 0;

	nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT);
	escreve(NRF24_CMD_W_TX_PAYLOAD, buf, n);

	/* Pulso de CE de no minimo 10 us: transmite um pacote e volta para standby-I. */
	nrf24_ce(1);
	k_busy_wait(15);
	nrf24_ce(0);

	/* Pior caso com ARD 500 us e ARC 15 da ~9 ms. O teto de 20 ms existe so para a
	 * funcao nao travar se o radio parar de responder. */
	for (int i = 0; i < 2000; i++) {
		st = nrf24_status();
		if (st & (NRF24_STATUS_TX_DS | NRF24_STATUS_MAX_RT)) {
			break;
		}
		k_busy_wait(10);
	}

	if (st & NRF24_STATUS_TX_DS) {
		nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_TX_DS);
		return 1;
	}

	/* MAX_RT tem de ser limpo, senao nenhuma transmissao seguinte acontece. E o
	 * payload nao sai do FIFO sozinho quando a entrega falha. */
	nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_MAX_RT);
	nrf24_cmd(NRF24_CMD_FLUSH_TX);
	return 0;
}

int nrf24_recebe(uint8_t *buf, uint8_t n)
{
	if (!(nrf24_status() & NRF24_STATUS_RX_DR)) {
		return 0;
	}

	le(NRF24_CMD_R_RX_PAYLOAD, buf, n);
	nrf24_write_reg(NRF24_REG_STATUS, NRF24_STATUS_RX_DR);
	return 1;
}
