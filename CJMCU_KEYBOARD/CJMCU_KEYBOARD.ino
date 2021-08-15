#include <SoftwareSerial.h>
#include <Keyboard.h>
#include<Mouse.h>


//SoftwareSerial mySerial(PIN_SPI_MOSI, PIN_SPI_MISO); // RX, TX
SoftwareSerial mySerial_bluetool(PIN_SPI_MOSI, PIN_SPI_MISO); // RX, TX

String str_receive;


/*
  一.硬件
  1.CJMCU-Beetle arduino Leonardo USB ATMEGA32U4 Mini Size Development Board
  2.透传蓝牙

  引脚连接
  CJMCU <==> 透传蓝牙
  5v         5v
  GND        GND
  MOSI       TX
  MISO       RX

  二.软件
  1.Arduino 1.8.13 打开本代码
  2.开发板选择 arduino leonardo
  3.CJMCU-Beetle插入PC usb
  4.选择端口，烧录本程序

  三.代码运行原理:
  1.透传蓝牙收到字符串，通过模拟键盘传给PC,作用：唤醒PC,输密码，PC休眠

  四.用法：
  1.CJMCU插入pc的USB口。
  2.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒。
    当PC进入休眠状态，CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备
  3.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU，
    例如模拟按键盘，点击鼠标，以达到唤醒PC的目的.
 
  五.整体电流:
  PC休眠时会多消耗 42-50ma电流为本设备供电
*/




//sec秒内不接收串口数据，并清缓存
void clear_uart(int ms_time)
{
  //唤醒完成后就可以正常接收串口数据了
  uint32_t starttime = 0;
  char ch;
  //5秒内有输入则输出
  starttime = millis();
  //临时接收缓存，防止无限等待
  while (true)
  {
    if  (millis()  - starttime > ms_time)
      break;
    while (mySerial_bluetool.available())
    {
      ch = (char) mySerial_bluetool.read();
      Serial.print(ch);
    }
    yield();
    delay(20);
  }
}




void setup() {
  Serial.begin(115200);
  mySerial_bluetool.begin(9600);

  Keyboard.begin();//开始键盘通讯
  Mouse.begin();

  Serial.println("start...");

}


void loop() {

  if (mySerial_bluetool.available() > 0)
  {
    str_receive = "";
    char ch;
    while (mySerial_bluetool.available() > 0)
    {
      ch = mySerial_bluetool.read();
      str_receive = str_receive + ch;
      delay(10);
    }
  }

  if (str_receive.length() > 0)
  {
    Serial.println("cmd=" + str_receive);

    if (str_receive == "KEY_RETURN")
    {
      Keyboard.press(KEY_RETURN);
      Keyboard.release(KEY_RETURN);
    }
    else if (str_receive == "MOUSE_CLICK")
    {
      Mouse.click();
    }
    else
    {
      Keyboard.print(str_receive);
      //key_press(mqtt_receive);
    }

    str_receive = "";
  }
  delay(500);

}
