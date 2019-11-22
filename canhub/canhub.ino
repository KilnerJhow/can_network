//Versão escrita para arduino nano.

int tx1=1;
int tx2=1;
int tx3=1;
int tx4=1;
int tx5=1;
int tx6=1;
int tx7=1;
int tx8=1;
int level=1;


//Configurção dos pinos e da porta serial.
void setup() {
  
  pinMode(2,OUTPUT);//RxCAN1
  pinMode(3,INPUT);//TxCAN1
  
  pinMode(4,OUTPUT);//RxCAN2
  pinMode(5,INPUT);//TxCAN2
  
  pinMode(6,OUTPUT);//RxCAN3
  pinMode(7,INPUT);//TxCAN3
  
  pinMode(8,OUTPUT);//RxCAN4
  pinMode(9,INPUT);//TxCAN4
  
  pinMode(10,OUTPUT);//RxCAN5
  pinMode(11,INPUT);//TxCAN5
  
  pinMode(14,OUTPUT);//RxCAN6
  pinMode(15,INPUT);//TxCAN6
  
  pinMode(16,OUTPUT);//RxCAN7
  pinMode(17,INPUT);//TxCAN7

  pinMode(18,OUTPUT);//RxCAN8
  pinMode(19,INPUT);//TxCAN8

  pinMode(13,OUTPUT);//Led

  Serial.begin(9600);
}

//Todos os pinos são lidos e caso algum seja acionado, o valor high é repassado a todas as saídas do hub.
void loop() {
  
  tx1 = digitalRead(3);
  tx2 = digitalRead(5);
  // tx3 = digitalRead(7);
  // tx4 = digitalRead(9);
  // tx5 = digitalRead(11);
  // tx6 = digitalRead(15);
  // tx7 = digitalRead(17);
  // tx8 = digitalRead(19);
  
  level = ( tx1 & tx2 /*& tx3 & tx4 & tx5 & tx6 & tx7 & tx8*/ );

  digitalWrite(2,level);
  digitalWrite(4,level);
  // digitalWrite(6,level);
  // digitalWrite(8,level);
  // digitalWrite(10,level);
  // digitalWrite(14,level);
  // digitalWrite(16,level);
  // digitalWrite(18,level);
  
  Serial.println(level);
  // Serial.print(" ");


}
