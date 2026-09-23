# -*- coding: utf-8 -*-
"""Gera o HTML do relatorio da Experiencia 1, no mesmo formato das atividades 1, 2 e 4."""
import io
import os

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SAIDA = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'relatorio-final.html')


def codigo(caminho):
    t = io.open(os.path.join(RAIZ, caminho), encoding='utf-8').read()
    return (t.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
             .rstrip())


CSS = """
/* as margens vem do printToPDF: margin aqui anularia todas elas */
@page { size: A4; }
html { -webkit-print-color-adjust: exact; }
body {
  font-family: Arial, Helvetica, sans-serif;
  font-size: 11pt; line-height: 1.55; color: #000; margin: 0;
  text-align: justify;
}
p { margin: 0 0 10pt 0; }
a { color: #1155cc; text-decoration: underline; word-break: break-all; }
.materia { font-size: 10.5pt; margin-bottom: 4pt; }
h1 { font-size: 17pt; font-weight: bold; line-height: 1.3;
     margin: 0 0 10pt 0; text-align: left; }
h2 { font-size: 11.5pt; font-weight: bold; margin: 18pt 0 8pt 0;
     text-align: left; break-after: avoid; }
h3 { font-size: 11pt; font-weight: bold; margin: 13pt 0 6pt 0;
     text-align: left; break-after: avoid; }
.cab { margin-bottom: 3pt; }
.cab-esp { margin-bottom: 10pt; }
table { border-collapse: collapse; width: 100%; margin: 4pt 0 2pt 0;
        font-size: 9.5pt; }
th, td { border: 1px solid #999; padding: 3.5pt 6pt; text-align: left;
         vertical-align: top; line-height: 1.35; }
th { background: #e8e8e8; font-weight: normal; }
tr { break-inside: avoid; }
thead { display: table-header-group; }
.cap { font-size: 9pt; font-style: italic; text-align: center;
       margin: 2pt 0 12pt 0; }
ul { margin: 0 0 10pt 0; padding-left: 18pt; }
li { margin-bottom: 9pt; }
li::marker { content: '\\2013\\00a0'; }
pre { font-family: 'Consolas', 'Courier New', monospace; font-size: 6.9pt;
      line-height: 1.32; white-space: pre-wrap; text-align: left;
      margin: 6pt 0 0 0; }
.num { font-variant-numeric: tabular-nums; }
"""

HTML = u"""<!doctype html>
<html lang="pt-BR"><head><meta charset="utf-8">
<title>Experiencia 1 Hugo e Wender</title>
<style>%(css)s</style></head><body>

<div class="materia">PSI3422 - Laboratório de Sistemas Eletrônicos</div>
<h1>Experiência 1 - Carrinho Comandado por Rádio Atravessando um Labirinto</h1>
<div class="cab">Nomes: Hugo dos Reis e Wender Souza</div>
<div class="cab-esp">Números USP: 12544308 e 13783054</div>
<div class="cab">Repositório do projeto</div>
<div class="cab-esp"><a href="https://github.com/ReisHugo123/psi3422-atividade-final">https://github.com/ReisHugo123/psi3422-atividade-final</a></div>
<div class="cab">Vídeos do carrinho em funcionamento</div>
<div class="cab"><a href="https://drive.google.com/file/d/1U5YnDd96yvcBYxHikRFAQ6FSdN4FyyPI/view?usp=sharing">https://drive.google.com/file/d/1U5YnDd96yvcBYxHikRFAQ6FSdN4FyyPI/view</a></div>
<div class="cab-esp"><a href="https://drive.google.com/file/d/11Au1dhRbMKE1U_guw_oZhxs3CtuJo48g/view?usp=sharing">https://drive.google.com/file/d/11Au1dhRbMKE1U_guw_oZhxs3CtuJo48g/view</a></div>

<h2>O que foi feito</h2>

<p>O carrinho 2WD montado ao longo da disciplina passou a ser comandado à distância por
rádio, a partir de um terminal no computador, e a atravessar sozinho um labirinto montado
no chão do laboratório. A apresentação foi feita em 22/09 e acompanhada pela professora,
com o carrinho no chão, alimentado por bateria e sem nenhum cabo ligado a ele.</p>

<p>O enunciado pedia quatro comandos e dois estados. Os comandos são <b>RUN</b>,
<b>STOP</b>, <b>mostrar a distância percorrida</b> e <b>apagar a distância</b>, todos
digitados no computador e transmitidos por rádio. No estado RUN o carrinho anda sozinho,
desvia das paredes e vai somando quanto andou; no estado STOP ele fica parado esperando
instrução. A distância acumulada é informada de volta pelo mesmo rádio e aparece no
terminal do computador.</p>

<p>Esta entrega é sobretudo <b>integração</b>. Motores e ultrassom vieram da atividade 1,
o rádio nRF24L01+ e a camada SPI vieram da atividade 2, e os encoders e a odometria
vieram da atividade 4. O que nasceu aqui foi a camada de enlace, que é o protocolo de
comando e resposta, com cerca de setenta linhas, e a máquina de estados do labirinto. O
ganho de juntar tudo não é somar funcionalidades, é que cada peça passa a depender da
qualidade da anterior: a distância que o carrinho reporta vale exatamente o que valer a
calibração feita na atividade 4, e o giro de 90 graus dentro do labirinto é o mesmo giro
que foi calibrado lá.</p>

<table>
<thead><tr><th>Módulo</th><th>Sinais na FRDM-KL25Z</th></tr></thead><tbody>
<tr><td>Ponte H L298N</td><td>ENA=PTD2, IN1=PTD0, IN2=PTD5, ENB=PTD3, IN3=PTE0, IN4=PTE1</td></tr>
<tr><td>Encoders</td><td>ENC_ESQ=PTD7 (J2-19), ENC_DIR=PTD6 (J2-17)</td></tr>
<tr><td>Ultrassom HC-SR04</td><td>TRIG=PTB0 (A0), ECHO=PTB1 (A1)</td></tr>
<tr><td>Rádio nRF24L01+</td><td>SCK=PTC5, MOSI=PTC6, MISO=PTC7, CSN=PTA4, CE=PTD4, IRQ=PTA12</td></tr>
<tr><td>Alimentação dos módulos</td><td>3,3 V e GND comum, saídos da própria FRDM</td></tr>
</tbody></table>
<div class="cap">Tabela 1 - Ligações usadas nas duas placas. O rádio saiu dos pinos da atividade 2 e foi para o SPI0 nativo de PORTC, porque lá ele ocupava PTD0, PTD2, PTD3 e PTD5, que são exatamente os pinos dos motores.</div>

<h2>O que foi implementado, parcialmente implementado e não implementado</h2>

<table>
<thead><tr><th>Item do enunciado</th><th>Situação</th><th>Como foi verificado</th></tr></thead><tbody>
<tr><td>Comando à distância por rádio, sem fio até o carrinho</td><td><b>Implementado</b></td><td>carrinho no chão alimentado por bateria, terminal no computador, na apresentação de 22/09</td></tr>
<tr><td>Comandos RUN, STOP, mostrar distância e apagar distância</td><td><b>Implementado</b></td><td>as quatro letras testadas em sequência, com a resposta do carrinho impressa no terminal</td></tr>
<tr><td>Estado RUN atravessando o labirinto com desvio de paredes</td><td><b>Implementado</b></td><td>travessia com o labirinto montado no laboratório, sem intervenção</td></tr>
<tr><td>Estado STOP parado esperando instrução</td><td><b>Implementado</b></td><td>STOP no meio de uma manobra, com parada imediata e não no fim do trecho</td></tr>
<tr><td>Estimar e guardar a distância percorrida</td><td><b>Implementado</b></td><td>comando de distância comparado com trena numa reta conhecida</td></tr>
<tr><td>Meia-volta não reduzir a distância acumulada</td><td><b>Implementado</b></td><td>acúmulo em módulo, garantido por construção e conferido no terminal</td></tr>
<tr><td>Usar a placa de interconexão da atividade 3</td><td><b>Não implementado</b></td><td>duas tentativas de solda, as duas com curto; o carrinho foi entregue em fiação por jumper</td></tr>
<tr><td>Guardar a distância em memória não volátil</td><td>Não implementado, por decisão</td><td>o enunciado não pede; exigiria partição de <i>settings</i> do Zephyr</td></tr>
<tr><td>Telemetria contínua do carrinho</td><td>Não implementado, por decisão</td><td>o protocolo é de pergunta e resposta, e o rádio é meia-duplex</td></tr>
</tbody></table>
<div class="cap">Tabela 2 - Situação de cada item. Tudo o que o enunciado pede em comportamento foi implementado; o que não ficou pronto foi a placa, e o carrinho funcionou sem ela.</div>

<h2>A placa da atividade 3, que não ficou pronta</h2>

<p>A placa de interconexão projetada na atividade 3 era para ser o suporte físico desta
entrega, concentrando numa só peça as ligações dos quatro módulos. Ela <b>não chegou a
funcionar</b>, e essa é a única parte do trabalho que ficou em aberto.</p>

<p>Foram feitas <b>duas montagens</b>. Nas duas o problema foi o mesmo: erro de solda
criando curto onde não devia haver. Na segunda placa o defeito foi localizado com o
multímetro em modo de continuidade, e é ilustrativo do tipo de erro: a ilha do <b>PTE3</b>,
que é o pino J9-11, ficou em continuidade ao mesmo tempo com o <b>GND</b> e com os
<b>3,3 V</b>. Isso significa que a linha de alimentação de 3,3 V estava curto-circuitada
contra o terra através daquela ilha. Com a placa nessa condição a FRDM-KL25Z nem sequer
enumera no computador, porque a proteção de corrente do regulador atua antes de a placa
subir.</p>

<p>Vale registrar por que esse erro é fácil de cometer nessa posição específica. O J9-11 é
da fileira interna e fica exatamente em frente ao J9-12, que é GND, a 2,54 mm de distância,
e o plano de terra da placa passa a 0,65 mm da ilha. Qualquer excesso de estanho naquele
ponto encosta nos dois. Não é uma falha de projeto, é uma ilha que exige mais cuidado de
execução do que as outras, e foi justamente ela que passou despercebida.</p>

<p>A consequência prática foi decidida em bancada: a placa foi retirada e o carrinho
<b>funcionou e foi apresentado com fiação por jumper</b>, ligando cada módulo direto nas
barras da FRDM, exatamente com os sinais da Tabela 1. Isso não muda nada do firmware,
porque os pinos são os mesmos que a placa rotearia. Antes de seguir por esse caminho foi
confirmado que a FRDM tinha sobrevivido ao curto: o drive DAPLINK monta, o identificador
único é lido corretamente, o KL25Z inicializa, a comunicação serial funciona nos dois
sentidos e a contagem de encoder fica em zero absoluto quando nada está conectado, sem
pulso fantasma.</p>

<h2>Arquitetura: dois programas, um projeto</h2>

<p>São necessárias duas FRDM-KL25Z, o mesmo arranjo da atividade 2. Um único projeto gera
os dois firmwares, e um <code>#define LADO</code> no topo do <code>main.c</code> escolhe
qual sai do build. Manter os dois lados no mesmo repositório evita a classe de erro mais
chata do rádio, que é o transmissor e o receptor divergirem em canal, endereço ou tamanho
de pacote e ninguém avisar: essas três constantes moram num arquivo só, o
<code>lib/enlace/enlace.c</code>, e por isso não há como um lado ser recompilado sem o
outro acompanhar.</p>

<p>No carrinho as mesmas letras também valem pelo terminal USB. Isso não foi conveniência,
foi método de depuração: dá para provar o labirinto e o odômetro com o carrinho preso ao
notebook, antes de o rádio entrar na conta, e quando algo falha já se sabe de que lado
olhar.</p>

<p>O carrinho roda três threads, e a prioridade entre elas é o que faz o STOP funcionar.
A thread do sonar tem a prioridade mais alta, a do rádio vem em seguida e a da navegação é
a mais baixa. O STOP não pode esperar a manobra atual terminar, senão o carrinho segue
andando por até um segundo depois do comando. Mas frear a partir de outra thread também
não resolveria sozinho, porque a malha da odometria reescreve a velocidade dos motores a
cada 2 ms e desfaria o freio. Por isso existe o <code>odo_aborta()</code>: ele levanta uma
bandeira que a malha consulta, e a manobra morre no passo seguinte devolvendo
<code>ODO_ABORTADO</code>. Na prática a parada entra em 2 ms.</p>

<h2>A travessia do labirinto</h2>

<p>O carrinho tem <b>um</b> sensor de distância, apontando para frente. Com isso a regra
possível é a mais simples: andar enquanto o caminho estiver livre e, ao encontrar parede,
recuar um pouco e procurar saída girando. A ordem de busca é direita, depois esquerda, e
se as duas estiverem bloqueadas ele trata como beco sem saída e volta pelo caminho por
onde veio.</p>

<p>Girar 90 graus de verdade é o que a atividade 4 entregou, e é o que torna essa regra
viável. Sem encoder o giro seria por tempo, e mudaria com o piso, com o peso em cima do
carrinho e com a carga da bateria, o que num labirinto significa bater na parede
seguinte.</p>

<h3>Por que o avanço é contínuo e não em passos</h3>

<p>A primeira versão avançava em passos fixos: andava um trecho, parava, lia o sonar e
decidia. Essa estrutura é confortável de programar, mas tem dois defeitos sérios. O
primeiro é de comportamento: o carrinho anda aos solavancos, parando a cada trecho, o que
não é como um carrinho atravessa um labirinto. O segundo é de segurança, e é o que
realmente obrigou a mudança: entre duas leituras o carrinho fica <b>cego</b>. Com passo de
120 mm e limiar de parada em 150 mm, era possível iniciar um passo a 151 mm de uma parede
e terminá-lo a 31 mm dela, porque nada olhava para frente no meio do caminho.</p>

<p>A solução foi inverter quem manda. O avanço passou a ser comandado com um alvo
deliberadamente longo, de 2000 mm, e <b>quem corta o avanço é o sonar, não o alvo</b>. Uma
thread dedicada mede a distância continuamente e, se vir parede a menos de 180 mm com o
carrinho em avanço, chama o mesmo <code>odo_aborta()</code> que o comando STOP usa. Se o
carrinho andar os 2000 mm inteiros sem ver nada, simplesmente começa outro trecho, e o
efeito visível é um movimento contínuo. Reaproveitar o mecanismo de aborto do STOP foi o
que tornou a mudança pequena: não foi preciso inventar caminho novo de parada, só um
segundo motivo para acionar o que já existia.</p>

<table>
<thead><tr><th>Constante</th><th>Valor</th><th>Papel</th></tr></thead><tbody>
<tr><td><code>AVANCO_MAX_MM</code></td><td>2000 mm</td><td>alvo longo de propósito; quem encerra o trecho é o sonar</td></tr>
<tr><td><code>DIST_PARE_MM</code></td><td>180 mm</td><td>abaixo disso a frente é considerada bloqueada</td></tr>
<tr><td><code>RECUO_MM</code></td><td>60 mm</td><td>recuo antes de girar, para a lateral não raspar na parede</td></tr>
<tr><td><code>SONAR_ESPERA_MS</code></td><td>120 ms</td><td>espera depois de cada giro, para a leitura refletir o rumo novo</td></tr>
</tbody></table>
<div class="cap">Tabela 3 - Parâmetros da navegação. O limiar de 180 mm com avanço contínuo não tem o ponto cego que a versão em passos tinha.</div>

<h2>A distância percorrida</h2>

<p>A distância é acumulada <b>em módulo</b>, sempre. O enunciado diz que dar meia-volta não
reduz a distância percorrida, e recuar também não reduz. Isso é garantido por construção e
não por uma verificação em tempo de execução: o acumulador só é somado, nunca subtraído, e
o encoder usado é de canal único, que conta borda e não sabe informar sentido. Não existe,
portanto, caminho de código pelo qual o número possa cair. Giro no próprio eixo não entra
na conta, porque nesse caso o carrinho não sai do lugar.</p>

<p>O valor vem da média dos pulsos das duas rodas, convertida pela calibração da atividade
4. Usar a média, e não uma roda só, é o que torna inofensivo o fato de as duas rodas terem
tido números diferentes de marcas na versão anterior do encoder: em trecho reto e em giro
no próprio eixo as duas rodas percorrem o mesmo comprimento, então a média é a estimativa
correta independentemente de qual roda tenha mais marcas.</p>

<p>O número mora em RAM. Sobrevive a quantos ciclos de RUN e STOP se queira e zera quando
a placa é desligada ou quando chega o comando de apagar.</p>

<h2>O alvo do encoder: da fita isolante ao disco impresso</h2>

<p>Na atividade 4 o alvo do encoder era <b>fita isolante preta colada no cubo da roda</b>,
alternando com o alumínio anodizado exposto. Funcionava, e foi com esse arranjo que a
atividade 4 foi entregue, mas tinha três defeitos que só apareceram com o carrinho rodando
de verdade.</p>

<p>O primeiro é que as marcas eram <b>poucas e desiguais</b>: sobraram três marcas numa
roda e quatro na outra, porque a fita era recortada e colada à mão no perímetro do cubo.
Poucas marcas significam resolução baixa, e resolução baixa aparece direto no ângulo do
giro, que é o parâmetro mais sensível do labirinto. O segundo é que a fita <b>descolava</b>
com o uso, especialmente com o carrinho andando de verdade e não apenas erguido na
bancada. O terceiro é que a largura de cada marca dependia do capricho do recorte, e marcas
de larguras diferentes tornam qualquer filtro de ruído mais difícil de ajustar.</p>

<p>Para esta entrega o alvo foi trocado por um <b>disco radial impresso em papel</b>, com
oito setores pretos alternados com oito claros, colado na face da roda. A escolha do padrão
radial, e não de uma tira enrolada na circunferência, resolveu o problema prático de
medida: como o padrão é angular, <b>ele vale em qualquer diâmetro</b>. O disco pode ser
recortado no tamanho que couber na roda e continua entregando oito marcas por volta, sem
precisar medir circunferência nenhuma. O toner da impressora é à base de carbono e bloqueia
o infravermelho, do mesmo jeito que a fita isolante bloqueava.</p>

<p>O resultado foi claramente melhor. As duas rodas passaram a ter o <b>mesmo</b> número de
marcas, oito, o que dobrou a resolução da roda que tinha quatro e mais que dobrou a da que
tinha três. Como o encoder conta nas duas bordas, isso dá dezesseis contagens por volta em
cada roda, e o teste de girar a roda uma volta completa com a mão passou a devolver
exatamente dezesseis em ambas. As marcas ficaram todas com a mesma largura angular, o alvo
parou de descolar, e o giro de 90 graus, que é o mais sensível, ficou visivelmente mais
repetitivo.</p>

<h2>Dificuldades e soluções encontradas</h2>

<h3>Os encoders estavam trocados entre si</h3>

<p>No teste em que se aciona um motor de cada vez e se observa qual contador sobe,
acionando o motor esquerdo quem contava era o contador da direita, e vice-versa. O encoder
da roda esquerda estava em PTD7 e o da direita em PTD6, e não o contrário. A correção foi
feita <b>no código</b>, trocando os dois <code>#define</code> de pino, e não na fiação: os
jumpers já estavam colados ao chassi com cola quente, e mexer neles custaria mais e
arriscaria desalinhar os sensores. O banner que o firmware imprime no boot passou a anunciar
a atribuição correta, para não induzir a conferir a fiação pelo lado errado numa próxima
sessão.</p>

<h3>Uma roda girava para trás</h3>

<p>Ao refazer a fiação, uma das rodas passou a girar no sentido contrário. O sentido de
cada motor é dado por um par de entradas da ponte H, então inverter o par inverte a roda.
A primeira tentativa inverteu as duas rodas juntas, o que mostrou que o par mexido
pertencia ao outro motor, e não ao que se imaginava. Ficou claro nesse episódio que os
canais dos motores estão espelhados em relação aos nomes, do mesmo jeito que os encoders
estavam: foi por isso que, na calibração da linha reta, o acerto de velocidade só teve
efeito quando aplicado ao lado oposto ao que a intuição indicava.</p>

<h3>O comparador do sensor gerava rajadas de bordas falsas</h3>

<p>O KL25Z não tem filtro digital de pino, então a supressão de ruído precisa ser feita em
software, ignorando bordas que chegam cedo demais para serem legítimas. O tempo de bloqueio
inicial, de 300 microssegundos, revelou-se curto: um contador de descarte instalado para
diagnóstico mostrou <b>452 e 524 bordas rejeitadas contra apenas 65 aceitas</b>, o que
indica que o comparador estava oscilando na transição entre claro e escuro e não que havia
ruído elétrico externo. O bloqueio foi aumentado para 3 milissegundos. O intervalo entre
duas bordas legítimas, na velocidade de trabalho, é de cerca de 132 milissegundos, ou seja,
há 44 vezes de margem, e nenhuma borda real é perdida. Depois disso o teste de uma volta
completa à mão passou a dar exatamente dezesseis bordas nas duas rodas.</p>

<h3>Uma calibração errada que passava em todos os testes</h3>

<p>Esta foi a dificuldade mais instrutiva. Uma calibração anterior tinha ficado com um
valor de pulsos por metro que produzia duas medidas discordantes entre si em 36 por cento:
68 pulsos deram 1780 mm e 34 pulsos deram 1212 mm. O erro passou despercebido porque o
teste que estava sendo usado era mandar o carrinho ir e voltar e verificar se ele
retornava à marca de partida, e <b>esse teste não detecta escala errada</b>: o erro é
simétrico na ida e na volta, então o carrinho volta exatamente ao ponto de partida com
qualquer constante, certa ou errada. O único teste que detecta é comandar uma distância
absoluta, por exemplo 1000 mm, e medir com trena se ele andou 1000 mm. Depois de trocar o
critério de validação, a calibração foi refeita e passou a fechar. A lição ficou escrita no
próprio cabeçalho do arquivo de calibração, para não se repetir.</p>

<h3>A serial ficava surda depois do primeiro comando</h3>

<p>Durante a preparação, o terminal do controle aceitava o primeiro comando, imprimia a
resposta do carrinho, e a partir daí nenhuma tecla aparecia mais, embora a placa
continuasse imprimindo normalmente. A causa está na combinação de dois fatores. O driver
serial do KL25Z, ao ler um caractere, testa apenas o sinalizador de <i>dado recebido</i> e
<b>nunca limpa o sinalizador de estouro</b>, cujo tratamento fica numa função de verificação
de erro que a aplicação não chama. No UART0 o estouro, uma vez aceso, impede o sinalizador
de dado recebido de subir de novo, ou seja, a recepção morre de vez enquanto a transmissão
segue funcionando, e é por isso que a placa parecia viva. O gatilho estava no próprio laço
do controle, que passava até 400 ms esperando a resposta do rádio sem ler o teclado nenhuma
vez: uma tecla apertada nessa janela ficava retida e a seguinte estourava o contador. A
correção limpa o estouro antes de cada leitura e drena o registrador durante a espera,
guardando uma tecla de fôlego. Isso importa principalmente para o STOP, que é a parada de
emergência e é exatamente a tecla que se aperta com o carrinho já andando.</p>

<h3>O rádio emudecia até a placa ser reiniciada</h3>

<p>Com o carrinho parado por algum tempo, o controle passava a informar que não havia
confirmação de recebimento, embora o LED do carrinho indicasse que ele estava alimentado e
com o firmware rodando. Reiniciar o carrinho resolvia, e só isso resolvia.</p>

<p>A causa estava na leitura do rádio. O sinalizador consultado significa <i>chegou pacote
novo</i>, e não <i>há dado na fila</i>, mas a fila de recepção do nRF24L01+ guarda três
pacotes. Dois pacotes que cheguem entre duas leituras acendem o sinalizador uma vez só:
lendo um pacote e apagando o sinalizador, o segundo ficava retido. Com três retidos a fila
enche, e de fila cheia o rádio <b>deixa de confirmar recebimento</b>, que é exatamente o
sintoma observado. Os pacotes duplicados vêm do próprio mecanismo de retransmissão
automática, configurado para até quinze tentativas: cada confirmação perdida faz o
transmissor reenviar o mesmo pacote, que é guardado de novo.</p>

<p>E a falha travava de vez porque a única limpeza de fila estava na rotina que coloca o
rádio em modo de escuta, e essa rotina, no carrinho, só é executada quando ele
<b>responde</b> a alguma coisa — e responder exige ter recebido antes. Surdo, ele nunca
respondia; não respondendo, nunca limpava. Só o reinício saía dessa condição. A correção
foi trocar a pergunta: em vez de consultar o sinalizador de pacote novo, consultar
diretamente se a fila está vazia. Assim cada leitura retira um pacote e a fila se esvazia
sozinha, sem depender de sinalizador nenhum.</p>

<h3>O sensor do encoder tem distância mínima de trabalho</h3>

<p>O sensor refletivo do kit não funciona encostado no alvo. Abaixo de cerca de 2 cm a luz
do emissor chega direto ao receptor sem passar pelo alvo, o receptor satura e a saída para
de acompanhar o contraste. A tentativa inicial de aproximar o sensor da roda para ganhar
sinal piorou a leitura, e o ajuste correto foi afastá-lo. Vale registrar também que
<b>caneta preta permanente não serve como marca</b>: a tinta é transparente ao
infravermelho de 940 nm, e uma marca feita com ela é invisível para o sensor, embora seja
perfeitamente preta ao olho. O que bloqueia o infravermelho é o negro de fumo, presente na
fita isolante e no toner de impressora.</p>

<h2>O que faríamos para resolver o que ficou em aberto</h2>

<p>O único item em aberto é a placa da atividade 3. O defeito é conhecido, localizado e
reparável: é uma ponte de solda na ilha do PTE3, que é o J9-11. O que faríamos, na ordem,
é o seguinte.</p>

<ul>
<li>Remover o excesso de estanho da ilha com malha dessoldadora e fluxo, em vez de tentar
puxar com o ferro, que foi o que criou a ponte na primeira tentativa.</li>
<li>Adotar um <b>critério de liberação objetivo</b> antes de energizar: com o multímetro em
continuidade, 3,3 V contra GND <b>não pode apitar</b>. Esse teste leva cinco segundos e
teria evitado as duas montagens perdidas.</li>
<li>Fazer uma varredura completa de continuidade a partir de um terra conhecido até cada
ilha dos conectores J1, J2, J9 e J10, conferindo contra a lista de quais ilhas são
legitimamente terra. Qualquer apito fora dessa lista é ponte de solda.</li>
<li>Só então encaixar a FRDM e verificar se ela enumera, e subir os módulos <b>um a um</b>,
na mesma ordem que foi usada na fiação por jumper: primeiro a placa sozinha, depois os
encoders, depois os motores, depois o ultrassom e por último o rádio. Essa ordem existe
para que, quando algo falhe, o que mudou desde a última etapa que funcionou seja curto o
suficiente para caber na cabeça.</li>
</ul>

<p>Vale dizer que a placa não era pré-requisito para o comportamento pedido no enunciado,
e é por isso que a entrega pôde ser feita sem ela. A fiação por jumper usa exatamente os
mesmos pinos que a placa rotearia, então o firmware é o mesmo nos dois casos, sem uma
linha de diferença.</p>

<h2>Resultados medidos</h2>

<table>
<thead><tr><th>Grandeza</th><th>Valor usado</th><th>Origem</th></tr></thead><tbody>
<tr><td>Marcas por roda</td><td>8</td><td>disco radial impresso, igual nas duas rodas</td></tr>
<tr><td>Contagens por volta</td><td>16</td><td>contagem nas duas bordas; conferido girando a roda à mão</td></tr>
<tr><td>Pulsos por metro</td><td>65</td><td>medido com trena, comandando distância absoluta</td></tr>
<tr><td>Pulsos por 90 graus</td><td>8</td><td>medido em giro no próprio eixo</td></tr>
<tr><td>Diâmetro da roda</td><td>70 mm</td><td>medido</td></tr>
<tr><td>Distância entre rodas</td><td>157 mm</td><td>medido</td></tr>
<tr><td>Ajuste de velocidade da roda esquerda</td><td>94 por cento</td><td>ajustado em bancada até a trajetória sair reta</td></tr>
<tr><td>Bloqueio de ruído do encoder</td><td>3 ms</td><td>contra 132 ms entre bordas legítimas, 44 vezes de margem</td></tr>
</tbody></table>
<div class="cap">Tabela 4 - Constantes de calibração efetivamente usadas na apresentação, e de onde cada uma saiu.</div>

<p>Com esses valores, o comando de 1000 mm levou o carrinho a uma distância muito próxima
de um metro medido com trena, e o comando de 90 graus produziu um giro visualmente muito
próximo do ângulo reto, repetido várias vezes. Antes do acerto de velocidade da roda
esquerda o carrinho puxava para um lado ao longo de uma reta longa; depois do ajuste a
trajetória ficou reta a olho nu ao longo de todo o percurso disponível no laboratório. As
medidas foram feitas com trena e a olho, que é a instrumentação disponível, e por isso
estão relatadas de forma qualitativa onde não houve leitura registrada.</p>

<h2>Limitações assumidas</h2>

<ul>
<li><b>Um sensor só, apontando para frente.</b> O carrinho não enxerga parede lateral,
então não é possível implementar seguidor de parede clássico. Parede muito em diagonal
devolve silêncio ao ultrassom e é lida como caminho livre.</li>
<li><b>A regra de busca pode levar a caminho repetido.</b> Com um sensor só e sem mapa, o
carrinho pode dar meia-volta e refazer parte do trajeto. Isso é consequência da regra
escolhida, não um defeito de implementação, e não viola o enunciado, que pede travessia e
não caminho ótimo.</li>
<li><b>Sem controle de velocidade em malha fechada.</b> A malha fecha em posição
acumulada, não em velocidade instantânea.</li>
<li><b>O rádio é meia-duplex e o protocolo é de pergunta e resposta.</b> Não existe
telemetria contínua: o carrinho fala quando perguntado.</li>
<li><b>A distância vale o que valer a calibração.</b> A precisão do número reportado é a
precisão dos pulsos por metro medidos na atividade 4, e não algo melhor que isso.</li>
</ul>

<h2>Código - lib/enlace/enlace.c</h2>

<p>Listagem da camada de enlace, que é a única biblioteca nova desta entrega e concentra as
constantes que os dois lados precisam ter iguais. As bibliotecas de motores, encoder,
odometria, ultrassom, SPI e rádio, mais o <code>main.c</code> com os dois lados, estão no
repositório e acompanham a entrega.</p>

<pre>%(enlace)s</pre>

<h2>Uso de ferramentas de IA</h2>

<p>Conforme a disciplina permite mediante declaração, foi usado assistente de IA no apoio à
leitura do manual de referência do KL25Z, na conferência do mapa de pinos contra a tabela
oficial da placa, na redação inicial das bibliotecas, na análise dos números medidos em
bancada e no diagnóstico de duas falhas descritas neste relatório, a da serial e a da fila
do rádio. A montagem, a solda, a calibração, todas as medidas e a validação de cada etapa
foram feitas no laboratório pelos autores. Não foi consultada solução de ano anterior nem
usada biblioteca de terceiros além da camada SPI fornecida pela disciplina.</p>

</body></html>
"""

io.open(SAIDA, 'w', encoding='utf-8').write(
    HTML % {'css': CSS, 'enlace': codigo(r'lib\enlace\enlace.c')})
print('gerado:', SAIDA)
