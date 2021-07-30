//Wew redaction  Version 2.
//14.07.2021

#define DEBUG 0
#if(DEBUG)
#include <SoftwareSerial.h>
SoftwareSerial softSerial(A5, A4);    // Создаём объект softSerial указывая выводы RX, TX (можно указывать любые выводы Arduino UNO)
#endif


#define ON  1
#define OFF 0
// anodes 2,3,4 cathodes 5,6,7
#define h9 0
#define h6 1
#define h4 2
#define h2 3
#define sleep 4
#define power 10
#define runRes 9

byte data_C[13] =   { 
                    0, //  предвар стирка          0  led
                    0, //  стирка                  1  led
                    0, //  полоскание              2  led
                    0, //  отжим                   3  led
                    0, //  таймер отсрочки стиирки 4  led key
                    0, //  супер стирка            5  led key
                    0, //  быстрая стирка          6  led key
                    0, //  дополнит полоскание     7  led key
                    0, //  Старт / Стоп            8  led
                    0, //  Старт / Стоп            9  key
                    0, //  Вкл / Выкл             10  key
                    0, //  резерв                 11
                    0  //  резерв                 12
                  };

const byte row_col[9]  = {52, 53, 54, 62, 63, 64, 72, 73, 74}; // пины светодиодов anode-catode

#define buttNum 7
const byte buttonArr [7] = {8, 9, 10, 11, 15, 16, 14}; // pins of button and tow pins beep, switch power for addons

//storage variables
//boolean toggle  = 0;
//boolean toggle0 = 0;
boolean toggle_LED13 = 0;

#define selectorTemp     A7
#define selectorSpeed    A6
#define selectorProg     A3

#define SwPower          A0
#define Beep             12
#define keyBeepTime      50
#define progNum 16
const int selectorProg_values[2] [16] = {
  // положение регулятора считывание с аналог. входа
  {3, 90, 150, 220, 300, 370, 430, 500, 580, 630, 700, 780, 840, 920, 990, 1550},
  //  программы стирки //  программы стирки 
  {1, 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,  16} };
#define tempSpeedNum 8
const int selectorTempSpeed_values[3] [8] = {
  // положение регулятора считывание с аналог. входа
  {10, 210, 460, 600, 750, 850, 960, 1550}, 
  // температура стирки
  { 0, 30, 40, 50, 60, 70, 80,  90},   
  // обороты отжима     
  {70, 80, 90, 100, 0, 40, 50,  60} };
#define ID          0
#define Start       1
#define Prog        2
#define Temper      3
#define Spin        4
#define Status      5
#define FlagEnd     6
#define rez1        7
#define rez2        8
#define cTemp       9
#define sTemp      10
#define LevWat     11
#define NumProg    12
#define FlagDoor   13
#define CountProg  14
#define SpeedH     15
#define SpeedL     16
#define PauseH     17
#define PauseL     18
#define CRC        19

byte        Command_Massive[20] = { 0, // 0 ID
                                    0, // 1 Старт стирка -1 Стоп стрика -0
                                    0, // 2 Номер сценария стирки
                                    0, // 3 Температура воды стирки
                                    0, // 4 Обороты отжима белья
                                    0, // 5 Статус стирки
                                    0, // 6 Флаг окончания стирки
                                    0, // 7 резерв
                                    0, // 8 резерв
                                    0, // 9 Текущая температура воды в баке
                                    0, // 10 Заданная температура воды
                                    0, // 11 Уровень воды в баке
                                    0, // 12 Текущая исполняемая программа стирки из сценария стирки
                                    0, // 13 Флаг состояния люка
                                    0, // 14 Количество сценариев стирок на SD карте
                                    0, // 15 Текущая скорость вращения барабана H
                                    0, // 16 Текущая скорость вращения барабана L
                                    0, // 17 Текущее значение паузы H
                                    0, // 18 Текущее значение паузы L
                                    0  // 19 CRC
                                  };

uint8_t errConn = 0;    // счетчик ошибок приема данных от контроллера
bool flag_send = false; // флаг режима отпраки данных контроллеру
unsigned long delay_send = 0;// переменная паузы отправки данных

int delayStart = 0;     // счетчик отсрочки начала стирки
unsigned long longlastTime = 0;//переменная хранения счетчика для звук сигнала

#define ControlPanelID   2   //  номер контрол. панели управления в I2C  сети
#define MasterID         1   // номер контроллера СМ в сети
/*========================================================*/
void setup()
{
#if(DEBUG)
  softSerial.begin(115200);     // Инициируем передачу данных по программной шине UART на скорости 11520 (между Arduino и компьютером)
  softSerial.write("\n\n\nHello! I'm panel controll...\n");
#endif
  Serial.begin(9600);    // Инициируем передачу данных по аппаратной  шине UART на скорости 9600 (между Arduino и мастером)
  pinMode(selectorTemp,   INPUT );
  pinMode(selectorSpeed,  INPUT );
  pinMode(selectorProg,   INPUT );
  for (int i = 0; i < 9; i++)
  {
    pinMode(row_col[i] / 10, OUTPUT); // anodes
    pinMode(row_col[i] % 10, OUTPUT); //cathodes
  }
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(SwPower, OUTPUT); 
  digitalWrite(SwPower, HIGH);
  beep(200);
  for (int i = 0; i < buttNum; i++) pinMode(buttonArr[i], INPUT_PULLUP);

  cli();//Выкл всех прерываний
  //set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR2A register to 0
  TCCR0B = 0;// same for TCCR2B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 254;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 7812;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  TCCR1B |= (1 << WGM12);// turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);// Set CS12 and CS10 bits for 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);// enable timer compare interrupt
  sei();//Вкл всех прерываний
}

ISR(TIMER0_COMPA_vect) { //функция прерывания timer0
  cli();
  static int  count;
  for (int i = 0; i < 9; i++) //all Off
  {
    digitalWrite(row_col[i] / 10, LOW); //row
    digitalWrite(row_col[i] % 10, HIGH); //col
  }

  if (count > 9 || count < 0 ) count = 0;
  digitalWrite(row_col[count] / 10, HIGH);
  if (data_C[count] ) digitalWrite(row_col[count] % 10, LOW);
  count++;
  sei();
  delay_send++;
  ReadKey();
  beep(0);  
}

ISR(TIMER1_COMPA_vect) { //функция прерывания функция прерывания timer1 1Hz
  if (toggle_LED13) {          //ВКЛ pin 13 (LED)
    digitalWrite(13, HIGH);
    data_C[8] = toggle_LED13;    // led 9
    toggle_LED13 = 0;
    // toggle  = 1;
  }
  else {                  //ВЫКЛ pin 13 (LED)
    digitalWrite(13, LOW);
    data_C[8] = toggle_LED13;    //led 9
    toggle_LED13 = 1;
  }
  if (data_C[9] == 1 && data_C[10] == 1) data_C[8] = 1;
  if (data_C[10] == 0) {
    data_C[8] = 0;
    data_C[9] = 0;
  }

  if (delayStart > 0 && toggle_LED13) delayStart--;        // декремент задержки если она больше 0
  if (data_C[9] == 1 && delayStart > 0) data_C[4] = !data_C[4];  // мигаем , показываем что включена отсрочка запуска
  if (data_C[9] == 1 && delayStart < 1) data_C[4] = 0;  // гасим индикатор задерки и индикатор времени задержки
}
/*========================================================*/
void loop()
{

  updateData();
  sendData();
  checkErr();

}
/*========================================================*/
void ReadKey()// Функция чтения состояния кнопок
{
  static byte drebezg8, drebezg9, drebezg10, drebezg11, drebezg15, drebezg16;
  byte countButtonPres = 0;
  /*_______________________________________________________*/
  if (!digitalRead(15)) drebezg15++;   // Кнопка "старт"/"стоп"
  if (drebezg15  && digitalRead(15) )
  {
    drebezg15 = 0;
    data_C[9] = !data_C[9];             //start reset button   data_C[9]
    if (!data_C[9])
    {
      for (uint8_t i = 0; i < 9; i++)
      { // сброс всех настроек и состояний
        data_C[i] = 0;
        Command_Massive[i] = 0;
      }
    }
    countButtonPres++;
  }
  /*_______________________________________________________*/
  if (!digitalRead(16)) drebezg16++;         // Кнопка "питание"   data_C[10]
  if (drebezg16  && digitalRead(16) )
  {
    drebezg16 = 0;
    data_C[10] = !data_C[10];
    if (!data_C[10])
    {
      for (uint8_t i = 0; i < 9; i++)
      { // сброс всех настроек и состояний
        data_C[i] = 0;
        Command_Massive[i] = 0;
      }
    }
    countButtonPres++;
  }
  if (countButtonPres > 0) beep(keyBeepTime);
  /*_______________________________________________________*/
  if (data_C[runRes] == 1 || data_C[power] == 0) return;   // выход если не нажата кнопка
  /*_______________________________________________________*/
  if (!digitalRead(11)) drebezg11++;      // Кнопка таймера отсрочки начала стирки
  if (drebezg11  && digitalRead(11) )
  {
    drebezg11 = 0;
    data_C[4]++;
    if (Command_Massive[Start] == ON) data_C[4] = Command_Massive[Status];
    switch (data_C[4])
    {
      case 0:
        delayStart = 0;
        break;
      case 1:
        delayStart = 120;//7200;
        break;
      case 2:
        delayStart = 14400;
        break;
      case 3:
        delayStart = 21600;
        break;
      case 4:
        delayStart = 32400;
        break;
      case 5:
        delayStart = 32400; // )))
        break;
      default:
        data_C[4] = delayStart = 0;
    }
    countButtonPres++;
  }
  /*_______________________________________________________*/
  if (!digitalRead(9)) drebezg9++;    // Кнопка "быстрой" стирки
  if (drebezg9  && digitalRead(9) )
  {
    drebezg9 = 0;
    data_C[6] = !data_C[6];
    if (data_C[5]) data_C[6] = 0;
    countButtonPres++;
  }
  /*_______________________________________________________*/
  if (!digitalRead(10)) drebezg10++;  // Кнопка "супер" стирки
  if (drebezg10  && digitalRead(10) )
  {
    drebezg10 = 0;
    data_C[5] = !data_C[5];
    if (data_C[6]) data_C[5] = 0;
    countButtonPres++;
  }
  /*_______________________________________________________*/
  if (!digitalRead(8)) drebezg8++;  // Кнопка двойное потоскание
  if (drebezg8  && digitalRead(8) )
  {
    drebezg8 = 0;
    data_C[7] = !data_C[7];
    countButtonPres++;
  }
  /*_______________________________________________________*/

  if (countButtonPres > 0) beep(keyBeepTime); // Звук нажатия кнопки
}
/*========================================================*/
int get_selector_position(int selector)//Функция чтения положения селекторов температуры, скорости
{
  int value = analogRead(selector);
#if(DEBUG)
  softSerial.print("selector "); softSerial.println(selector);
  softSerial.print("U "); softSerial.println(value);
#endif
  if (selector == selectorProg)
  {
    for (int i = 0; i < progNum; i++)

      if (value < selectorProg_values[0][i])
        return i;
  }

  for (int i = 0; i < tempSpeedNum; i++)
  {
    if (value < selectorTempSpeed_values[0][i])
      return i;
  }

  return -1;
}
/*========================================================*/
void serialEvent()                           // функция прерывания получении символа в UART
{
  if (Serial.available() > (byte)sizeof(Command_Massive)) // Если пришло сообщение размером с нашу структуру
  {
    byte tmp[(byte)sizeof(Command_Massive)] = {0,};
    Serial.readBytes((byte*)&tmp, (byte)sizeof(Command_Massive));

    byte crc = 0;
    for (byte i = 0; i < 20 - 1; i++)  crc ^= tmp[i];
    crc ^= 20 + 2;
#if(DEBUG)
    softSerial.print("\ncrc "); softSerial.print(crc);
#endif
    if (tmp[(byte)sizeof(Command_Massive) - 1] != crc || tmp[0] != ControlPanelID)
    {
      errConn++;
#if(DEBUG)
      softSerial.print("  ERR "); softSerial.print(errConn);
#endif
      return;
    }
    memcpy((byte*)&Command_Massive, tmp, sizeof(tmp));
#if(DEBUG)
    softSerial.print("\n>> ");
    for (uint8_t i = 0; i < sizeof(Command_Massive); i++) 
    {softSerial.print(Command_Massive[i]); softSerial.print(" ");}
#endif
    flag_send = true;                       // Поднимаем флаг отправки данных
    delay_send = 0; // Сброс задержки перед отправкой данных
  }

}
/*========================================================*/
void sendData() // Функция отправки данных мастеру
{
  if (flag_send == false || delay_send < 50) return;    // Если флаг отправки не поднят или задержка отправки не прошла
  //выходим из функции отправки данных
  byte tmp_buf[sizeof(Command_Massive)] = {0,};
  tmp_buf[ID]     = MasterID;
  tmp_buf[Start]  = Command_Massive[Start];//data_C[runRes];
  tmp_buf[Prog]   = 10;//Command_Massive[Prog]
  tmp_buf[Temper] = 40;//Command_Massive[Temper]
  tmp_buf[Spin]   = 50;//Command_Massive[Spin]
  for (byte i = 0; i < 20 - 1; i++)       // Подсчет CRC  отправляемых данных
    tmp_buf[19] ^= tmp_buf[i];
  tmp_buf[19] ^= 20 + 2;
#if (DEBUG)
  softSerial.print("\n<< "); 
  for (uint8_t i = 0; i < sizeof(tmp_buf); i++) 
  {softSerial.print(tmp_buf[i]);softSerial.print(" ");}
#endif
  Serial.write((byte*)&tmp_buf, sizeof(tmp_buf));// Отвечаем контроллеру (мастеру)
  flag_send = false;  // опускаем флаг отправки данных мастеру
}
/*========================================================*/
void updateData() // Функция обработки данных состояния компонентов управл.
{
  if (data_C[runRes] && delayStart < 1) 
  {
    Command_Massive[Start] = ON;
  } else {
    Command_Massive[Start] = OFF;
  }
  if (Command_Massive[FlagEnd] == 1 && data_C[runRes] == 1 && delayStart < 1) 
  {
    data_C[runRes] = 0;
    delayStart   = 0;
    Command_Massive[FlagEnd] = 0;
    beep(500);
  } 

  if (data_C[runRes] == 0 && data_C[power] == 1)
  {
    Command_Massive[Prog]       = selectorProg_values      [1] [get_selector_position(selectorProg)];
    Command_Massive[Temper]     = selectorTempSpeed_values [1] [get_selector_position(selectorTemp)];
    Command_Massive[Spin]       = selectorTempSpeed_values [2] [get_selector_position(selectorSpeed)];
  }
  byte tmp = 0;
  if (Command_Massive[Start] == ON) {
    tmp = Command_Massive[Status];
  } else {
    tmp = data_C[4];
  }
// зажигаем индикаторы состояния стирки помещая в массив значение
  switch (tmp)
  {
    case 0:
      data_C[0] = 0; data_C[1] = 0; data_C[2] = 0; data_C[3] = 0;
      break;
    case 1:
      data_C[0] = 1; data_C[1] = 0; data_C[2] = 0; data_C[3] = 0;
      break;
    case 2:
      data_C[0] = 0; data_C[1] = 1; data_C[2] = 0; data_C[3] = 0;
      break;
    case 3:
      data_C[0] = 0; data_C[1] = 0; data_C[2] = 1; data_C[3] = 0;
      break;
    case 4:
      data_C[0] = 0; data_C[1] = 0; data_C[2] = 0; data_C[3] = 1;
      break;
    default:
      Command_Massive[Status] = 255;
  }

}

/*========================================================*/
void beep(unsigned int hold)// Фунуция звуковой индикации
{
  longlastTime++;
  static unsigned int holdTime;
  if (hold > 0)
  {
    digitalWrite(Beep, HIGH);
    holdTime = hold;
    longlastTime = 0;
  }
  if (longlastTime >= holdTime)
  {
    holdTime = 0;
    digitalWrite(Beep, LOW);
  }
}

/*========================================================*/
void checkErr()// Функция инициализации послед. порта 
{
  if (errConn < 1) return;
  errConn = 0;
  Serial.end();
  Serial.begin(9600);
}
/*========================================================*/
