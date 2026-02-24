// ============================================================================
// Projeto: Alarme de Proximidade Analógico com Setpoint Digital
// Microcontrolador: Arduino Nano (ATmega328P)
// ============================================================================

// --- Mapeamento de Pinos ---
const int pinoTrigger = 7;   // Saída: Envia o pulso de início para o HC-SR04
const int pinoLed = 3;       // Saída PWM: Controla o brilho do LED
const int pinoBotao = 4;     // Entrada: Botão de Reset
const int pinoSinal = A2;    // Entrada Analógica: Recebe a tensão do LM358

// --- Variáveis de Sistema ---
int leituraADC = 0;          // Armazena o valor bruto lido do filtro (0 a 1023)
int brilhoPWM = 0;           // Valor da intensidade do LED (0 a 255)
bool alarmeSilenciado = false; // Estado lógico do botão de reset

// --- Calibração (O "Setpoint Digital") ---
// Física do circuito: Objeto MAIS PERTO = Pulso MAIS CURTO = Tensão MENOR.
// Ajuste este valor olhando o Monitor Serial para definir a distância de disparo.
int setpoint = 600;          

void setup() {
  // Configuração das portas de saída
  pinMode(pinoTrigger, OUTPUT);
  pinMode(pinoLed, OUTPUT);
  
  // Configuração do botão com o resistor interno do Arduino (Pull-up)
  // Isso dispensa o uso de resistor físico no botão.
  pinMode(pinoBotao, INPUT_PULLUP);
  
  // Inicialização da comunicação serial para verificação de resultados
  Serial.begin(9600);
  Serial.println("--- Sistema Hibrido de Deteccao Iniciado ---");
}

void loop() {
  // ==========================================
  // 1. INTERFACE DO USUÁRIO (Botão de Reset)
  // ==========================================
  // Com o INPUT_PULLUP, quando o botão é pressionado, a porta lê LOW (0)
  if (digitalRead(pinoBotao) == LOW) {
    alarmeSilenciado = true;   // Muda o estado do alarme
    analogWrite(pinoLed, 0);   // Apaga o LED imediatamente
    Serial.println("--> Alarme Silenciado pelo Botao!");
    delay(300);                // Pequeno atraso para evitar "bouncing" (efeito mola do botão)
  }

  // ==========================================
  // 2. AQUISIÇÃO E CONDICIONAMENTO
  // ==========================================
  // Envia o pulso de Trigger de 10us para acordar o sensor HC-SR04
  digitalWrite(pinoTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrigger, LOW);

  // Aguarda o filtro RC (1.6Hz) estabilizar a rampa de tensão
  delay(50); 

  // Lê a tensão média (componente DC) entregue pelo amplificador LM358
  leituraADC = analogRead(pinoSinal);

  // ==========================================
  // 3. LÓGICA DE COMPARAÇÃO (O LM339 de Software)
  // ==========================================
  // Se a leitura for MENOR que o setpoint, significa que o objeto cruzou o limite
  if (leituraADC < setpoint) {
    
    // Só liga o LED se o botão de reset não tiver sido acionado recentemente
    if (alarmeSilenciado == false) {
      
      // Controle Proporcional (PWM):
      // Quanto mais o valor cai abaixo do setpoint (mais perto), mais o LED brilha.
      // O map() converte a escala do ADC para a escala do PWM (0 a 255).
      brilhoPWM = map(leituraADC, setpoint, 0, 50, 255);
      
      // Limita os valores para não causar erros no sinal PWM
      brilhoPWM = constrain(brilhoPWM, 0, 255);
      
      analogWrite(pinoLed, brilhoPWM);
    }
    
  } else {
    // Objeto afastado (Fora da zona de alarme)
    analogWrite(pinoLed, 0);       // Apaga o LED
    alarmeSilenciado = false;      // Rearma o alarme automaticamente se o objeto sair de perto
    brilhoPWM = 0;
  }

  // ==========================================
  // 4. MONITORAMENTO E RESULTADOS (Para o Relatório)
  // ==========================================
  // Calcula a tensão em Volts apenas para exibir na tela
  float tensaoCalculada = leituraADC * (5.0 / 1023.0);
  
  Serial.print("Leitura ADC: ");
  Serial.print(leituraADC);
  Serial.print(" | Tensao: ");
  Serial.print(tensaoCalculada);
  Serial.print("V | PWM LED: ");
  Serial.println(brilhoPWM);

  // Pausa antes do próximo ciclo de medição
  delay(50);
}   