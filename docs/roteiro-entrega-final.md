# Roteiro de bancada da entrega final

Vem depois da atividade 4. A calibração dela (`ODO_PULSOS_POR_M` e
`ODO_PULSOS_90`) tem de estar medida e copiada para
`lib/odometria/odometria.h` **deste** projeto, senão o odômetro mente e o giro
de 90 graus do labirinto não fecha.

Tempo estimado: **1h30**, se a atividade 4 já estiver pronta.

## Ordem, e por que ela é essa

Cada etapa acrescenta **uma** coisa nova. Quando algo falhar, o que mudou desde
a última etapa que funcionou é curto o suficiente para caber na cabeça.

| Etapa | O que entra de novo | Como se prova |
|---|---|---|
| 1 | copiar a calibração | o banner do boot imprime os dois números |
| 2 | o labirinto, sem rádio | comando pelo terminal USB |
| 3 | o odômetro | `D` no terminal, contra a trena |
| 4 | o rádio | as duas placas conversando |
| 5 | tudo junto | a demonstração |

---

## Etapa 1: copiar a calibração (5 min)

Abra `lib/odometria/odometria.h` do projeto da atividade 4, copie os quatro
números e cole no arquivo de mesmo nome deste projeto:

```c
#define ODO_PULSOS_POR_M      196
#define ODO_PULSOS_90          22
#define ODO_ESCORREGO_RETO      0
#define ODO_ESCORREGO_GIRO      0
```

Grave com `LADO_CARRINHO` e confira no terminal que o banner imprime os valores
que você mediu, e não os de fábrica.

## Etapa 2: o labirinto, ainda sem rádio (30 min)

O carrinho aceita as mesmas letras pelo terminal USB. Deixe-o preso ao notebook
por enquanto.

1. Ponha o carrinho de frente para uma parede a mais de 30 cm.
2. Digite `R`. Ele deve andar 200 mm, ler o sonar, andar de novo.
3. Aproxime a mão ou um livro a menos de 25 cm da frente dele. Ele deve recuar
   80 mm, girar 90 graus para a direita e continuar.
4. Digite `S` no meio de uma manobra. Ele tem de **parar na hora**, e não no fim
   do passo. O terminal imprime `abortada a pedido (STOP)`.

| Sintoma | Causa provável |
|---|---|
| não anda, LED vermelho | conferir o banner: `IN3=PTE0 IN4=PTE1` na placa |
| anda e não para na parede | sonar sem eco, conferir TRIG em PTB0 e ECHO em PTB1 |
| gira demais ou de menos | a calibração da etapa 1 não foi copiada |
| `ODO_TRAVOU` logo no `R` | encoder sem pulso, voltar ao MODO 1 da atividade 4 |

## Etapa 3: o odômetro (15 min)

1. `Z` para zerar.
2. `R`, deixe andar uma reta conhecida, `S`.
3. `D` e compare com a trena. O erro tem de ser o mesmo da atividade 4.
4. Faça o carrinho dar uma meia-volta e andar de volta. `D` de novo: o número
   tem de **continuar subindo**, nunca diminuir. É isso que o enunciado pede.

## Etapa 4: o rádio (25 min)

Agora sim a segunda placa.

1. Grave a segunda FRDM com `#define LADO   LADO_CONTROLE`.
2. Ligue as duas no PC e abra **dois** monitores, um em cada porta COM.
3. No terminal do controle, digite `D`. Devem acontecer três coisas: o controle
   imprime `-> D`, o carrinho imprime `[DIST]`, e o controle imprime a resposta
   com a distância.

| Sintoma | Causa provável |
|---|---|
| `SEM ACK` | o carrinho não recebeu. Conferir alimentação do rádio em 3,3 V, e que as duas placas usam o mesmo canal e endereço (estão em `lib/enlace/enlace.c`) |
| entregue mas sem resposta | o carrinho recebeu e não conseguiu responder. Quase sempre é o `nrf24_modo_rx` faltando de um lado |
| só o primeiro comando funciona | um dos lados ficou em TX. O `enl_envia` já volta para RX sozinho, então suspeitar de alimentação |

⚠ **O rádio é 3,3 V.** O J9-10 é 5 V e fica ao lado do J9-08. Errar uma casa
custa o módulo.

## Etapa 5: a demonstração (15 min)

Monte o labirinto com caixas ou livros. O carrinho no chão, alimentado pelo
power bank, sem cabo.

Sequência que mostra os quatro comandos e os dois estados:

1. `Z` zera a distância.
2. `R` e o carrinho entra no labirinto.
3. `D` no meio do percurso, e o número aparece no computador.
4. `S` e ele para na hora.
5. `R` e ele volta a andar de onde parou.
6. `D` no fim, e o número é a distância total.

Grave o vídeo dessa sequência inteira, com o terminal do computador visível.

## O que anotar para o relatório

- os quatro números de calibração usados
- a distância que o carrinho reportou contra a medida com a trena
- quantas vezes o labirinto foi atravessado sem intervenção
- qualquer manobra que tenha abortado, e com que código
