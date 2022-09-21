#include <msp430.h>

#define TRUE    1
#define FALSE   0

#define FREQ   500


// Bits para controle do LCD
#define BIT_RS   BIT0
#define BIT_RW   BIT1
#define BIT_E    BIT2
#define BIT_BL   BIT3
#define BR100K  11          //(SMCLK) 1.048.576/11  ~= 100kHz

// Bits para controle da chave
#define ABERTA 1
#define FECHADA 0
#define DBC 1000

void io_config();
void adc_config(void);
void ta0_config(void);
void ta2_config(void);
void USCI_B0_config(void);
void atraso(long x);

void debounce(int valor);
int mon_s3(void);
int mon_s2(void);
int mon_s1(void);

void i2c_write(char dado);
void lcd_inic(void);
void lcd_aux(char dado);
char i2c_test(char adr);
void lcd_adr();

void lcd_write_nibble(char nib, int inst);
void lcd_write_char(char dado, int inst);
void lcd_clear();
void lcd_set_cursor(int pos);
void lcd_line_2();
void lcd_cursor(int state);
void lcd_write_str(unsigned char *texto);
int strlen(char *s);
void lcd_write_int(unsigned int num, int casas);

volatile int i, medx, medy, flag=0, contador=0, aux=0;
volatile unsigned int can0, can1;

int main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Inicializações e configurações
    io_config();
    adc_config();
    ta0_config();
    ta2_config();
    USCI_B0_config();

    lcd_adr();
    i2c_write(0);
    lcd_inic();

    lcd_clear();

    __enable_interrupt();

    while(TRUE){                                    // Loop externo que roda toda a sequência do programa
        lcd_clear();

        lcd_write_str("MAQUINA DE REFRI");            // Mensagem incial
        lcd_line_2();
        lcd_write_str("LAB SISMIC");

for(i=0;i<3000;i++){
        while( (TA2CCTL0&CCIFG) == 0);  //Usar CCIFG para
            TA2CCTL0 &= ~CCIFG;
        }

        contador=0;


        while(TRUE){


            lcd_clear();


            if(mon_s1() == TRUE){
                lcd_write_str("COPO PEQUENO");

        P8OUT |= BIT2;
for(i=0;i<2000;i++){
        while( (TA2CCTL0&CCIFG) == 0);  //Usar CCIFG para
            TA2CCTL0 &= ~CCIFG;
        }
                 __delay_cycles(100);

            P8OUT &= ~BIT2;


            }

             if(mon_s2() == TRUE){
                lcd_write_str("COPO MEDIO");

        P8OUT |= BIT2;
for(i=0;i<3000;i++){
        while( (TA2CCTL0&CCIFG) == 0);  //Usar CCIFG para
            TA2CCTL0 &= ~CCIFG;
        }
                 __delay_cycles(100);

            P8OUT &= ~BIT2;


            }

            if(mon_s3() == TRUE){
                lcd_write_str("COPO GRANDE");

        P8OUT |= BIT2;
for(i=0;i<4000;i++){
        while( (TA2CCTL0&CCIFG) == 0);  //Usar CCIFG para
            TA2CCTL0 &= ~CCIFG;
        }
                 __delay_cycles(100);

            P8OUT &= ~BIT2;


            }




        }

    }


    return 0;

}

/////////////////////////////////////////////////////////////////////////////////
////////////////////// Rotinas de tratamento de Interrupções ////////////////////
/////////////////////////////////////////////////////////////////////////////////


#pragma vector = TIMER0_A0_VECTOR                   // Interrupcao do timer
__interrupt void TA0_CCR0_ISR()                     // Quando o CCR0 chega ao maximo, acontece a interrupcao
{

}

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Configurações ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Configurar ADC
void adc_config(void){

    volatile unsigned char *pt;
    unsigned char i;

    ADC12CTL0 &= ~ADC12ENC;         // Desabilita as conversões antes de configuraro módulo

    ADC12CTL0 = ADC12SHT1_3 +       // Sample and hold time
                ADC12SHT0_3 +       // Sample and hold time
                ADC12ON     +       // Liga o ADC
                ADC12MSC;           // Modo sequencial!
    ADC12CTL1 = ADC12CSTARTADD_0 +  // Inicio da sequencia = canal 0.
                ADC12SHS_0 +        // Sample and holder "manual"
                ADC12SHP +          // Sampling signal = sampling timer
                ADC12DIV_0 +        // Divisor de clock.
                ADC12SSEL_0 +       // Oscilador do ADC
                ADC12CONSEQ_1;      // Sequence of channels
    ADC12CTL2 = ADC12TCOFF +        // Desliga o sensor de temperatura
                ADC12RES_2;         // Resolução = 12 bits

    ADC12IE     =   BIT1;           // Interrup apenas no final

    pt=&ADC12MCTL0;

    for (i=0; i<16;  i+=2){
        pt[i+0]=ADC12SREF_0 | ADC12INCH_14; //ADC12MCTL0 at  ADC12MCTL7
        pt[i+1]=ADC12SREF_0 | ADC12INCH_15; //ADC12MCTL8 at  ADC12MCTL15
    }
    pt[15] |= ADC12EOS;

    ADC12MCTL0  =   0;
    ADC12MCTL1  =   1 | ADC12EOS;   // End of Sequence

    ADC12CTL0  |=   ADC12ENC;       // Habilita pois a configuracao acabou
 }

// Configurar TA0
void ta0_config(void){
    TA0CTL   = TASSEL__ACLK | MC__UP;               // ACLK e modo 1
    TA0CCR0  = FREQ;                                // Ponto de disparo da interrupção
    TA0CCTL0 = CCIE;                                // Habilita interrupcao do timer
 }

// Configurar TA2
void ta2_config(void){
    TA2CTL = TASSEL_2|ID_0|MC_1|TACLR;
    TA2EX0 = TAIDEX_0;
    TA2CCTL2 = OUTMOD_6;
    TA2CCR0 = 2*1048;
    TA2CCR2 = TA2CCR0/2;
}

 // Configurar I/O
void io_config() {


    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;

    // Configura a chave SW6.2
    P6DIR &= ~BIT2;
    P6REN |= BIT2;   //tres botoes que acionam o mecanismo
    P6OUT |= BIT2;

    P6DIR &= ~BIT1;
    P6REN |= BIT1;
    P6OUT |= BIT1;

    P6DIR &= ~BIT0;
    P6REN |= BIT0;
    P6OUT |= BIT0;

    // Configura os LEDS
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    P8DIR |= BIT2;              //saida rele
    P8OUT &= ~BIT2;

    P3DIR |= BIT7;
    P3OUT &= ~BIT7;

    P4DIR |= BIT0;
    P4OUT &= ~BIT0;
    P4SEL &= ~BIT0;

    P4DIR |= BIT3;
    P4OUT &= ~BIT3;
    P4SEL &= ~BIT3;


    P2DIR |= BIT5;
    P2OUT &= ~BIT5;
    P2SEL |= BIT5;
}

// Configurar USCI_B0 como mestre
void USCI_B0_config(void){
    UCB0CTL1 = UCSWRST;         //Ressetar USCI_B1
    UCB0CTL0 =  UCMST    |      //Modo Mestre
                UCMODE_3 |      //I2C
                UCSYNC;         //Síncrono
    //UCB0BRW = BR10K;          //10 kbps
    UCB0BRW = BR100K;           //100 kbps
    UCB0CTL1 = UCSSEL_3;        //SMCLK e UCSWRST=0
    P3SEL |= BIT1 | BIT0;       //Funções alternativas
    P3REN |= BIT1 | BIT0;
    P3OUT |= BIT1 | BIT0;
}

// Atraso
void atraso(long x){
    volatile long cont=0;
    while(cont++ < x) ;
}

// Debouce para precisão da chave
void debounce(int valor){
    volatile int x;
        for (x=0; x<valor; x++);        //Apenas gasta tempo
    }

// Monitora a chave s3
int mon_s3(void){
    static int ps1=ABERTA;          //Guardar passado de S3
        if ((P6IN&BIT2) == 0){      //Qual estado atual de S3?
            if (ps1==ABERTA){       //Qual o passado de S3?
                debounce(DBC);
                ps1=FECHADA;
                return TRUE;
                }
            }
        else {
            if (ps1==FECHADA){      //Qual o passado de S3?
                debounce(DBC);
                ps1=ABERTA;
                return FALSE;
            }
        }
    return FALSE;
}

int mon_s1(void){
    static int ps2=ABERTA;          //Guardar passado de S3
        if ((P6IN&BIT0) == 0){      //Qual estado atual de S3?
            if (ps2==ABERTA){       //Qual o passado de S3?
                debounce(DBC);
                ps2=FECHADA;
                return TRUE;
                }
            }
        else {
            if (ps2==FECHADA){      //Qual o passado de S3?
                debounce(DBC);
                ps2=ABERTA;
                return FALSE;
            }
        }
    return FALSE;
}

int mon_s2(void){
    static int ps3=ABERTA;          //Guardar passado de S3
        if ((P6IN&BIT1) == 0){      //Qual estado atual de S3?
            if (ps3==ABERTA){       //Qual o passado de S3?
                debounce(DBC);
                ps3=FECHADA;
                return TRUE;
                }
            }
        else {
            if (ps3==FECHADA){      //Qual o passado de S3?
                debounce(DBC);
                ps3=ABERTA;
                return FALSE;
            }
        }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// LCD inicialização / Funções ////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    UCB0CTL1 |= UCTR    |                       //Mestre TX
                UCTXSTT;                        //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Sa da PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
                while(1);

    // Come ar inicializa  o
    lcd_aux(0);     //RS=RW=0, BL=1
    atraso(20000);
    lcd_aux(3);     //3
    atraso(10000);
    lcd_aux(3);     //3
    atraso(10000);
    lcd_aux(3);     //3
    atraso(10000);
    lcd_aux(2);     //2

    // Entrou em modo 4 bits
    lcd_aux(2);     lcd_aux(8);     //0x28
    lcd_aux(0);     lcd_aux(8);     //0x08
    lcd_aux(0);     lcd_aux(1);     //0x01
    lcd_aux(0);     lcd_aux(6);     //0x06
    lcd_aux(0);     lcd_aux(0xF);   //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    atraso(50);
}

// Auxiliar inicialização do LCD (RS=RW=0)
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //PCF7:4 = dado;
    atraso(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL | BIT_E;  //E=1
    atraso(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //E=0;
}

// Testar o endereço adr para escrita
char i2c_test(char adr){
    UCB0I2CSA = adr;                //Endere o a ser testado
    UCB0CTL1 |= UCTR;               //Mestre TX --> escravo RX
    UCB0CTL1 |= UCTXSTT;            //Gerar STASRT
    while ( (UCB0IFG & UCTXIFG) == 0);  //TXIFG indica que j  iniciou o START
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);      //Esperar STOP
    if ((UCB0IFG & UCNACKIFG) == 0) return TRUE;   //Chegou ACK
    else                            return FALSE;  //Chegou NACK
}

// Testa qual endereço será usado
void lcd_adr(void) {
    char adr;       //Endere o do PCF_8574

    if      (i2c_test(0x27) == TRUE) adr=0x27;
    else if (i2c_test(0x3F) == TRUE) adr=0x3F;

    UCB0I2CSA = adr;    //Endere o a ser usado
}

///////////////////////////////// Funções LCD ///////////////////////////////////

// Escrever dado na porta
void i2c_write(char dado){
    UCB0CTL1 |= UCTR    |                           //Mestre TX
                UCTXSTT;                            //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = dado;                               //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);       //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG){       //NACK?
        P1OUT |= BIT0;                              //NACK=Sinalizar problema
                                           //Travar execu  o
    }
    UCB0CTL1 |= UCTXSTP;                            //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);       //Esperar STOP
}

// Usa a função I2C write para mandar um nible de uma palavra para o lcd
void lcd_write_nibble(char nib, int inst) {
    char dado;
    dado = ( (nib & 0x0F) << 4);               // Ganrantir que os 4 bits mais significativos são 0
    dado |= 0x08;

    if (!inst)                                  // Verifica se é instrução (RS = 0) ou escrita (RS = 1)
        dado |= 0x01;

    i2c_write(dado);                            // Escereve dado
    i2c_write(dado | 0x04);                     // Ativa enable
    i2c_write(dado);                            // Escreve dado
}

// Usa a função que escreve um nibble no lcd para escrever um caractere
void lcd_write_char(char dado, int inst) {
    char n1, n2;
    n1 = (dado >> 4) & 0x0F;    // Separa o nibble maior
    n2 = dado & 0x0F;           // Separa o nible menor

    lcd_write_nibble(n1, inst);
    lcd_write_nibble(n2, inst);
}

// limpa o display
void lcd_clear() {
    atraso(40);
    lcd_write_char(0x01,1); // Passa a instrução de limpar o display
    atraso(40);
}

// Coloca o ponteiro na posição selecionada
void lcd_set_cursor(int pos) {
    switch (pos){
    case 1: lcd_write_char(0x80,1); break;
    case 2: lcd_write_char(0x81,1); break;
    case 3: lcd_write_char(0x82,1); break;
    case 4: lcd_write_char(0x83,1); break;
    case 5: lcd_write_char(0x84,1); break;
    case 6: lcd_write_char(0x85,1); break;
    case 7: lcd_write_char(0x86,1); break;
    case 8: lcd_write_char(0x87,1); break;
    case 9: lcd_write_char(0x88,1); break;
    case 10: lcd_write_char(0x89,1); break;
    case 11: lcd_write_char(0x8A,1); break;
    case 12: lcd_write_char(0x8B,1); break;
    case 13: lcd_write_char(0x8C,1); break;
    case 14: lcd_write_char(0x8D,1); break;
    case 15: lcd_write_char(0x8E,1); break;
    case 16: lcd_write_char(0x8F,1); break;

    case 17: lcd_write_char(0xC0,1); break;
    case 18: lcd_write_char(0xC1,1); break;
    case 19: lcd_write_char(0xC2,1); break;
    case 20: lcd_write_char(0xC3,1); break;
    case 21: lcd_write_char(0xC4,1); break;
    case 22: lcd_write_char(0xC5,1); break;
    case 23: lcd_write_char(0xC6,1); break;
    case 24: lcd_write_char(0xC7,1); break;
    case 25: lcd_write_char(0xC8,1); break;
    case 26: lcd_write_char(0xC9,1); break;
    case 27: lcd_write_char(0xCA,1); break;
    case 28: lcd_write_char(0xCB,1); break;
    case 29: lcd_write_char(0xCC,1); break;
    case 30: lcd_write_char(0xCD,1); break;
    case 31: lcd_write_char(0xCE,1); break;
    case 32: lcd_write_char(0xCF,1); break;

    default:
        lcd_write_char(0x80,1);
        break;
    }
}

// Coloca o ponteiro no início da linha 2
void lcd_line_2() {
    atraso(40);
    lcd_write_char(0xC0,1); // Usa a instrução de colocar o ponteiro em posição específico
    atraso(40);
}


void lcd_write_int(unsigned int num, int casas) {
    volatile int aux, rest, n;

    switch (casas){
    case 1:
        aux = num + 0x30;
        lcd_write_char(aux,0);
        break;
    case 2:
        aux = (num/10) + 0x30;
        rest = (num%10) + 0x30;
        lcd_write_char(aux,0);
        lcd_write_char(rest,0);
        break;
    case 3:
        aux = (num/100) + 0x30;
        rest = (num%100);
        lcd_write_char(aux,0);
        aux = (rest/10) + 0x30;
        rest = rest%10;
        lcd_write_char(aux,0);
        aux = rest + 0x30;
        lcd_write_char(aux,0);
        break;
    default:
        break;
    }
}

// Calcula o tamanho de uma string
int strlen(char *s)
{
    char *ptr=s;            // Salva o ponteiro pois serão feita operações com ele
    while (*s!='\0')        // Veridica se o ponteiro aponta para o fim do vetor
        s++;                // Incrementa o ponteiro
    return (int) (s-ptr);   // Retorna o fim menos o começo do vetor
}

// Escreve uma string no lcd recebendo seu ponteiro
void lcd_write_str(unsigned char *texto) {
    int qtd = strlen(texto);    // Pega o tamanho da string
    volatile int i;
        for (i = 0; i < qtd; i++){
            if (i==16)          // Verifica se chegou ao fim da primeira linha
                lcd_line_2();
            if (i==32){         // Verifica se chegou ao fim da segunda linha
                atraso(10000);
                lcd_clear();
            }
            if (i== 48)
                lcd_line_2();

            lcd_write_char(texto[i],0); // Escreve o caractere
        }
}

