# PSI3422 — Entrega final: carrinho comandado por rádio

Carrinho 2WD com **FRDM-KL25Z**, ponte H **L298N**, dois **encoders**, ultrassom
**HC-SR04** e rádio **nRF24L01+**. Zephyr RTOS + PlatformIO.

⚠ **A placa da atividade 3 não ficou funcional.** Foram duas montagens, as duas
com curto de solda — na segunda, a ilha do PTE3 (J9-11) ficou em continuidade ao
mesmo tempo com GND e com 3,3 V, curto-circuitando a alimentação. O carrinho foi
**apresentado com fiação por jumper**, nos mesmos pinos que a placa rotearia, então
o firmware é idêntico nos dois casos.

O comportamento pedido no enunciado:

- controle remoto pelo computador, por rádio, com comandos de **RUN**, **STOP**,
  **mostrar a distância percorrida** e **apagar a distância**;
- no estado RUN o carrinho atravessa um labirinto, avança, recua, faz curvas, e
  **estima e guarda a distância percorrida**, sendo que dar meia-volta não reduz
  essa distância;
- no estado STOP ele fica parado esperando instrução.

## Esta entrega é integração, não invenção

Cada peça já tinha sido construída e provada numa atividade anterior. Aqui elas
passam a conviver no mesmo carrinho e no mesmo firmware.

| Peça | Vem de | Estado |
|---|---|---|
| `lib/motores` | atividade 1, carrinho | provado |
| `lib/hcsr04` | atividade 1, carrinho | provado |
| `lib/spi`, `lib/nrf24` | atividade 2, wireless | provado na bancada, passo 5 |
| `lib/encoder`, `lib/odometria` | atividade 4, encoders | provado |
| `lib/enlace` | **novo**, 70 linhas | o protocolo de comando |
| placa | atividade 3 | **não funcional**, substituída por jumper |

## Dois programas, um projeto

O `#define LADO` no topo de `src/main.c` escolhe qual firmware sai do build.

| `LADO` | Onde vai | O que faz |
|---|---|---|
| `LADO_CARRINHO` | placa do carrinho | escuta o rádio, obedece, responde, navega |
| `LADO_CONTROLE` | segunda FRDM, ligada no PC | lê o teclado do terminal, manda o comando, imprime a resposta |

São necessárias **duas FRDM-KL25Z**, que é o mesmo arranjo da atividade 2. A
placa do controle não precisa de nada além do rádio.

No carrinho as mesmas letras também valem pelo **terminal USB**. Isso não é
conveniência, é método: dá para provar o labirinto e o odômetro antes de o rádio
entrar na conta, e quando algo falhar já se sabe de que lado olhar.

## Comandos

| Tecla | O que faz |
|---|---|
| `R` | entra no estado RUN |
| `S` | entra no estado STOP |
| `D` | mostra a distância percorrida no computador |
| `Z` | apaga a distância guardada no carrinho |

Toda resposta traz o estado, a distância em milímetros e o código da última
manobra que terminou mal, se houve alguma.

## Ligações

Feitas por jumper, direto nas barras da FRDM. São os mesmos pinos que a placa da
atividade 3 rotearia.

| Módulo | Sinais |
|---|---|
| L298N | ENA=PTD2, IN1=PTD0, IN2=PTD5, ENB=PTD3, **IN3=PTE0, IN4=PTE1** |
| Encoders | ENC_ESQ=PTD7 (J2-19), ENC_DIR=PTD6 (J2-17) |
| HC-SR04 | TRIG=PTB0 (A0), ECHO=PTB1 (A1) |
| nRF24L01+ | SCK=PTC5, MOSI=PTC6, MISO=PTC7, **CSN=PTA4, CE=PTD4**, IRQ=PTA12 |

⚠ **O rádio mudou de pino em relação à atividade 2.** Lá ele morava em PTD0,
PTD2, PTD3 e PTD5, que são exatamente os pinos dos motores, e foi essa colisão
que fez a placa da atividade 3 existir. Indo para o SPI0 nativo de PORTC, o
remendo que a atividade 2 precisava fazer no `spi_init` **desaparece**, e a
biblioteca da disciplina passa a servir como foi entregue.

⚠ **O IRQ do rádio não é usado.** O firmware consulta o rádio por polling a cada
10 ms. Isso é de propósito: a interrupção de PORTA colidiria com a mesma questão
de vetor que os encoders resolvem em PORTD, e o ganho seria nenhum num protocolo
de pergunta e resposta.

## Compilar e gravar

```
pio run --target upload
pio device monitor
```

Com as duas placas ligadas no mesmo PC, aponte a porta:

```
pio run -t upload --upload-port E:
```

## Como o carrinho é organizado por dentro

Três threads, e a prioridade entre elas é o que faz o STOP funcionar. Prioridade
menor é mais urgente.

```
thread SONAR (prio 4)      thread RADIO (prio 5)          thread NAVEGACAO (prio 6)
mede a distancia sempre    escuta o radio e o terminal    roda o labirinto
corta o avanco na parede   aplica o comando e responde    soma a distancia andada
```

O STOP não pode esperar a manobra atual acabar, senão o carrinho segue andando
por até um segundo depois do comando. Mas frear de outra thread também não
resolve sozinho, porque a malha da odometria reescreve a velocidade dos motores
a cada 2 ms e desfaria o freio. Por isso existe `odo_aborta()`: ele levanta uma
bandeira que a malha consulta, e a manobra morre no passo seguinte devolvendo
`ODO_ABORTADO`. A parada entra em 2 ms.

## A lógica do labirinto

O carrinho tem **um** sensor, apontando para frente. Com isso a regra possível é:

```
livre        -> avanca continuamente (alvo de 2000 mm, cortado pelo sonar)
bloqueado    -> recua 60 mm
                tenta a direita  (gira +90)   livre? segue
                tenta a esquerda (gira -180)  livre? segue
                beco sem saida   (gira -90)   volta pelo caminho
```

O avanco **nao** e em passos fixos. O alvo de 2000 mm e longo de proposito: quem
encerra o trecho e a thread do sonar, que mede continuamente e chama
`odo_aborta()` ao ver parede a menos de 180 mm. Em passos fixos o carrinho ficava
cego entre duas leituras — com passo de 120 mm e limiar de 150 mm dava para comecar
um passo a 151 mm da parede e termina-lo a 31 mm dela.

Girar 90 graus de verdade é o que a atividade 4 entregou. Sem encoder esse giro
seria por tempo, e mudaria com o piso, o peso e a carga da bateria, o que num
labirinto significa bater na parede seguinte.

## A distância percorrida

Acumulada em **módulo**, sempre. O enunciado diz que dar meia-volta não reduz a
distância, e recuar também não reduz. Giro no próprio eixo não entra na conta,
porque o carrinho não sai do lugar.

A conta vem da média das duas rodas convertida pela calibração da atividade 4
(`ODO_PULSOS_POR_M`), então **a precisão daqui é a precisão da calibração de
lá**. Rodar a atividade 4 antes não é pré-requisito burocrático, é o que faz
este número valer alguma coisa.

O valor mora em RAM. Sobrevive a quantos ciclos RUN e STOP se queira, e zera
quando a placa é desligada ou quando chega o comando `Z`. Guardar em flash
exigiria partição de `settings` do Zephyr, e não foi feito.

## Limitações assumidas

- **Um sensor só, para frente.** Não vê parede lateral, então não dá para fazer
  seguidor de parede clássico. Parede muito em diagonal devolve silêncio e é
  lida como caminho livre.
- **O STOP corta a manobra pela metade.** É o comportamento certo, mas significa
  que a odometria daquele passo conta só o pedaço andado, o que é o correto.
- **Sem controle de velocidade.** A malha fecha em posição acumulada.
- **O rádio é meia-duplex e o protocolo é de pergunta e resposta.** Não existe
  telemetria contínua: o carrinho fala quando perguntado.
