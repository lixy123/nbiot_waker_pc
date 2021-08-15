#include <HardwareSerial.h>
#include <SoftwareSerial.h>

//注：esp32-s2只有两个硬件串口，无法支持1,2两个硬件，
//办法是用软串口，但软串口有bug,esp32-s2只能发，不能收，且RX必须连接线
//https://www.arduino.cn/thread-104709-1-1.html
//https://github.com/akshaybaweja/SoftwareSerial 可以发送，收数据会重启!


HardwareSerial mySerial(1);
//HardwareSerial mySerial_bluetool(2);  //ESP32-S2不能用2号，收发会无效
//库在问题，可发出数据，不能收数据（收数据会异常重启。 
//                  RX必须连接上，否则也会异常重启
//                     RX TX
SoftwareSerial mySerial_bluetool(3,4,false, 256);

//air302 reset
int reset_pin = 5;
bool net_connect_succ = false;
bool mqtt_connect_succ = false;
int mqtt_connect_error_num = 0;

//这是一个国内的免费mqtt服务器，很可能哪天就停用了
String mqtt_server = "test.ranye-iot.net";

//这三个变量每个设备需要更换一套不同名称，
//否则会互相冲突或收不到信息！
String mqtt_clientid = "clientesp32_s2";
String mqtt_topic = "/clientesp32_s2";
String mqtt_topic_resp = "/clientesp32_s2/resp";

uint32_t boot_time = 0;   //每12小时 air302自动复位一次
uint32_t check_down_time = 0; //每小时AT命令检查一次，如果air302返回数据不正常，硬件reset


int check_down_time_cnt = 0;

String buff_split[5];


/*
  一.硬件
  1.ESP32
  2.NB-IOT AIR302
  引脚连接
  ESP32 <==> AIR302
  5v       5v
  GND      GND
  5       reset
  1       TX
  2       RX
  3.蓝牙透传设备
  ESP32 <==> 蓝牙透传(HC05,HC08均可，配置好透传)
  5v/或3.3V  5v
  GND        GND
  4          RX
  3          TX
  
  二.软件
  1.Arduino 1.8.13 打开本代码
  2.开发板选择 ESP32-S2
  3.ESP32-S2插入PC usb
  4.选择端口，烧录本程序

  三.代码运行原理:
  1.ESP32开机检测NB-IOT连接，AT命令连接到MQTT服务器
  2.当收到MQTT服务器的文字，将文字信息通过蓝牙透传传出,功能是唤醒休眠后的PC开机
  3.稳定性检查：
     A.当收到MQTT服务器中断信息，每60秒后自动重新连接MQTT。 尝试3次仍连接不上MQTT,复位NB-IOT硬件
     B.每小时检测AT命令是否能收到反应消息，如无，硬件复位NB-IOT硬件
     C.每小时检测一次百度网站连接 (注：代码暂时未加检测后处理)

  四.用法：
  1.ESP32连接好air302(带卡),蓝牙透传设备,插入pc的USB口。
  2.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒。
    将PC进入休眠状态，此CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备
  3.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU，
    例如模拟按键盘，点击鼠标，以达到唤醒PC的目的.
 
  五.注意：
  mqtt服务器最好用国内的，数据包稳定，不容易掉线。
  程序有自动重连检测并恢复重连，程序稳定性没经过高强度检验。
  目前是国内一家网上免费服务，随时可能关闭，建议找一个稳定好用的。

  六.整体电流:
  ESP32: 42-50ma
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
    while (mySerial.available())
    {
      ch = (char) mySerial.read();
      Serial.print(ch);
    }
    yield();
    delay(20);
  }
}


String send_at(String p_char, String break_str, int delay_sec) {

  String ret_str = "";
  String tmp_str = "";
  if (p_char.length() > 0)
  {
    Serial.println(String("cmd=") + p_char);
    mySerial.println(p_char);
  }

  //发完命令立即退出
  //if (break_str=="") return "";

  mySerial.setTimeout(1000);

  uint32_t start_time = millis() / 1000;
  while (millis() / 1000 - start_time < delay_sec)
  {
    if (mySerial.available() > 0)
    {
      //此句容易被阻塞
      tmp_str = mySerial.readStringUntil('\n');
      //tmp_str.replace("\r","");
      //tmp_str.trim()  ;
      Serial.println(">" + tmp_str);
      //如果字符中有特殊字符，用 ret_str=ret_str+tmp_str会出现古怪问题，最好用concat函数
      ret_str.concat(tmp_str);
      if (break_str.length() > 0 && tmp_str.indexOf(break_str) > -1)
        break;
    }
    delay(10);
  }
  return ret_str;
}


bool connect_mqtt()
{
  bool succ_flag = false;

  String ret;
  delay(5000);

  //清空可能的上一次MQTT连接或变量
  ret = send_at("AT+ECMTDISC=0", "+CME ERROR: 3", 5);
  Serial.println("ret=" + ret);
  delay(2000);


  int error_cnt = 0;
  while (true)
  {
    ret = send_at("AT+ECMTOPEN=0,\"" + mqtt_server + "\",1883", "+ECMTOPEN: 0,0", 30);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+ECMTOPEN: 0,0") > -1)
      break;
    delay(5000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }

  Serial.println(">>> 创建MQTT 连接 ok ...");
  delay(2000);
  error_cnt = 0;
  while (true)
  {
    ret = send_at("AT+ECMTCONN=0,\"" + mqtt_clientid + "\"", "+ECMTCONN: 0,0,0", 30);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+ECMTCONN: 0,0,0") > -1)
      break;
    delay(5000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> 设置设备ID ok ...");

  delay(5000);
  error_cnt = 0;
  while (true)
  {
    ret = send_at("AT+ECMTSUB=0,1,\"" + mqtt_topic + "\",2", "+ECMTSUB: 0,1,0,1", 30);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+ECMTSUB: 0,1,0,1") > -1)
    {
      succ_flag = true;
      break;
    }
    delay(5000);
    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> 订阅主题 ok ...");

  //mqtt 发送上线信息

  delay(2000);
  error_cnt = 0;
  while (true)
  {
    ret = send_at("AT+ECMTPUB=0,0,0,0,\"" + mqtt_topic_resp + "\",\"online\"", "+ECMTPUB: 0,0,0", 30);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+ECMTPUB: 0,0,0") > -1)
      break;
    delay(5000);
    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println("mqtt 发送上线信息 ok ...");

  return succ_flag;
}


//仅检查是否关机状态
bool check_waker_air302()
{
  String ret = "";
  delay(1000);
  int cnt = 0;
  bool check_ok = false;
  //通过AT命令检查是否关机，共检查3次
  while (true)
  {
    cnt++;
    ret = send_at("AT", "OK", 2);
    Serial.println("ret=" + ret);
    if (ret.indexOf("OK") > -1 )
    {
      check_ok = true;
      break;
    }
    if (cnt >= 3) break;
    delay(1000);
  }
  return check_ok;
}

//重启air302
void reset_air302()
{
  Serial.println("reset_air302...");
  digitalWrite(reset_pin, LOW);
  delay(1000);
  digitalWrite(reset_pin, HIGH);
  clear_uart(10000);
}

bool connect_nb()
{
  bool  ret_bool = false;

  int error_cnt = 0;
  String ret;


  error_cnt = 0;
  //网络信号质量查询，返回信号值
  while (true)
  {
    ret = send_at("AT+CPIN?", "+CPIN: READY", 5);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+CPIN: READY") > -1)
      break;
    delay(2000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> SIM 卡状态 ok ...");


  error_cnt = 0;
  //查询附着
  while (true)
  {
    ret = send_at("AT+CGATT=1", "OK", 5);
    Serial.println("ret=" + ret);
    if (ret.indexOf("OK") > -1)
      break;
    delay(2000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> 附着 OK ...");

  error_cnt = 0;
  //查询PDP 上下文激活
  while (true)
  {
    ret = send_at("AT+CGACT=1", "OK", 5);
    Serial.println("ret=" + ret);
    if (ret.indexOf("OK") > -1)
      break;
    delay(2000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> PDP 上下文激活 OK ...");

  error_cnt = 0;
  //查询网络信息
  while (true)
  {
    ret = send_at("AT+CGCONTRDP", "+CGCONTRDP: 0", 5);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+CGCONTRDP: 0") > -1)
      break;
    delay(2000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> 网络信息，运营商及网络制式 OK...");

  error_cnt = 0;
  //查询网络信息
  while (true)
  {
    //OK 结束， 判断标志为  +ECDNS:
    ret = send_at("AT+ECDNS=www.baidu.com", "OK", 5);
    Serial.println("ret=" + ret);
    if (ret.indexOf("+ECDNS: ") > -1)
      break;
    delay(2000);

    error_cnt++;
    if (error_cnt >= 5)
      return false;
  }
  Serial.println(">>> 百度dns测试...");

  ret_bool = true;
  return ret_bool;
}


void splitString(String message, String dot, String outmsg[], int len)
{
  int commaPosition, outindex = 0;
  for (int loop1 = 0; loop1 < len; loop1++)
    outmsg[loop1] = "";
  do {
    commaPosition = message.indexOf(dot);
    if (commaPosition != -1)
    {
      outmsg[outindex] = message.substring(0, commaPosition);
      outindex = outindex + 1;
      message = message.substring(commaPosition + 1, message.length());
    }
    if (outindex >= len) break;
  }
  while (commaPosition >= 0);

  if (outindex < len)
    outmsg[outindex] = message;
}





void setup() {
  Serial.begin(115200);
  // mySerial.begin(9600);
  //                               RX, TX
  mySerial.begin(9600, SERIAL_8N1, 1, 2);
  //mySerial_bluetool.begin(9600, SERIAL_8N1, 3, 4);
  mySerial_bluetool.begin(9600);
  Serial.println("...");
  //10秒时间是等待air302初始化
  delay(10000);

  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);

  //Keyboard.begin();//开始键盘通讯
  //Mouse.begin();

  boot_time = millis() / 1000;
  check_down_time = millis() / 1000;

  Serial.println(">>> 开启 nb-iot ...");


  delay(10000);  //等待一会，确保网络连接上

  Serial.println(">>> 检查网络连接 ...");
  net_connect_succ = false;
  mqtt_connect_succ = false;

  net_connect_succ = connect_nb();
  if (net_connect_succ)
  {
    Serial.println(">>> MQTT连接 ...");
    mqtt_connect_succ = connect_mqtt();
  }
  mqtt_connect_error_num = 0;
  check_down_time_cnt = 0;
  Serial.println("net_connect_succ:" + String(net_connect_succ) + ",mqtt_connect_succ:" + String(mqtt_connect_succ) );
}


void loop() {
  //英文资料说millis（) 在大约50天后清零，

  if ( millis() / 1000 < check_down_time )
    check_down_time = millis() / 1000;

  if ( millis() / 1000 < boot_time )
    boot_time = millis() / 1000;

  //每12小时自动重启air302
  if ( millis() / 1000 - boot_time > 12 * 3600)
  {
    Serial.println("12 小时定时重启 ...");
    reset_air302();
    net_connect_succ = false;
    mqtt_connect_succ = false;
    delay(60 * 1000);
    boot_time = millis() / 1000 ;
    return;
  }


  //如果setup时网络连接失败，重新再试
  if (net_connect_succ == false)
  {
    delay(5000);
    mqtt_connect_succ = false;
    Serial.println(">>> 检查网络连接 ...");
    net_connect_succ = connect_nb();
    if (net_connect_succ)
    {
      Serial.println(">>> 连接 MQTT...");
      mqtt_connect_succ = connect_mqtt();
    }
    return;
  }


  //每1小时检查air302是否alive, 如果关闭则重启air302
  if ( millis() / 1000 - check_down_time >   3600)
  {
    if (check_waker_air302() == false)
    {
      Serial.println("AT 命令不响应");
      reset_air302();
      net_connect_succ = false;
      mqtt_connect_succ = false;
    }
    else
    {
      if (check_down_time_cnt == 0)
      {
        String ret = send_at("AT+ECDNS=www.baidu.com", "OK", 10);
        Serial.println("ret=" + ret);
      }

      check_down_time_cnt = check_down_time_cnt + 1;
      if (check_down_time_cnt >= 2)
        check_down_time_cnt = 0;
    }

    check_down_time = millis() / 1000 ;
    return ;
  }


  if (net_connect_succ )
  {
    if ( mqtt_connect_succ)
    {
      if (mySerial.available() > 0)
      {
        String mqtt_receive = "";
        char ch;
        while (mySerial.available() > 0)
        {
          ch = mySerial.read();
          mqtt_receive = mqtt_receive + ch;
          delay(10);
        }
        //air302收到的字串带回车换行，去除,简化数据分析
        mqtt_receive.trim();

        Serial.println("get mqtt msg 1:" + mqtt_receive );
        //+ECMTDISC: 0,0 标志表示MQTT中断！且不会自动重连
        if (mqtt_receive.length() == 0)
        {
          Serial.println("收到空串，跳过");
        }
        else if (mqtt_receive.indexOf("OK") > -1)
        {
          Serial.println("收到ok串，跳过");
        }
        else if (mqtt_receive.indexOf("+ECMTDISC: 0,0") > -1)
        {
          Serial.println("收到mqtt中断信号1，重新连接mqtt");
          Serial.println("disconnect mqtt");
          clear_uart(10000);
          mqtt_connect_succ = false;
        }
        else if (mqtt_receive.indexOf("+ECMTSTAT: 0,1") > -1)
        {
          Serial.println("收到mqtt中断信号2，重新连接mqtt");
          Serial.println("disconnect mqtt");
          clear_uart(10000);
          mqtt_connect_succ = false;
        }
        //延迟收到此字串，不用处理
        else if (mqtt_receive.indexOf("+ECMTPUB: 0,0,0") > -1)
        {
          Serial.println("收到:" + mqtt_receive);
        }
        else if (mqtt_receive.indexOf("+ECMTRECV: 0,") > -1 )
        {

          //+ECMTRECV: 0,987,"/***",hello
          //分解出接收的数据
          splitString(mqtt_receive, ",", buff_split, 4);
          buff_split[0].trim();
          if (buff_split[0] == "+ECMTRECV: 0" && buff_split[2] == "\"" + mqtt_topic + "\"")
          {
            mqtt_receive =  buff_split[3];
            Serial.println("get mqtt msg 2: " + mqtt_receive  );

            if (mqtt_receive == "KEY_RETURN")
            {
              // Keyboard.press(KEY_RETURN);
              // Keyboard.release(KEY_RETURN);
              mySerial_bluetool.print(mqtt_receive);
            }
            else if (mqtt_receive == "MOUSE_CLICK")
            {
              //Mouse.click();
              mySerial_bluetool.print(mqtt_receive);
            }
            else if (mqtt_receive.startsWith("MOUSE_MOVE/"))
            {

              mySerial_bluetool.print(mqtt_receive);
              /*
                splitString(mqtt_receive, "/", buff_split, 3);
                int xDistance = buff_split[1].toInt();
                int yDistance = buff_split[2].toInt();
                // Mouse.move(xDistance, yDistance, 0);
              */
            }
            else
            {
              // Keyboard.print(mqtt_receive);
              mySerial_bluetool.print(mqtt_receive);
            }
            Serial.println("send mqtt resp>>>" );
            String ret = send_at("AT+ECMTPUB=0,0,0,0,\"" + mqtt_topic_resp + "\",\"ok\"", "+ECMTPUB: 0,0,0", 30);
            Serial.println("ret=" + ret);
          }
          else
          {
            Serial.println("skip mqtt message");
          }

        }
        //收到不是以上字串的数据，多半是网络中断
        else
        {
          Serial.println("收到mqtt之外的数据，重新连接mqtt");
          Serial.println("disconnect mqtt 2");
          //无条件断开mqtt
          clear_uart(20000);
          net_connect_succ = false;
          mqtt_connect_succ = false;
        }
        delay(2000);
      }
    }
    else
    {
      //每分钟尝试一次,连接mqtt
      Serial.println(">>> 60秒后重新连接 mqtt");
      delay(60000);
      Serial.println(">>> 连接 mqtt");

      String ret = send_at("AT+ECDNS=www.baidu.com", "OK", 30);
      Serial.println("ret=" + ret);

      mqtt_connect_succ = connect_mqtt();

      if (mqtt_connect_succ)
        mqtt_connect_error_num = 0;
      else
      {
        mqtt_connect_error_num = mqtt_connect_error_num + 1;
        Serial.println("mqtt_connect_error_num=" + String(mqtt_connect_error_num));
      }

      //连续3次MQTT连接失败，重启air302
      if (mqtt_connect_error_num >= 3)
      {
        Serial.println("连接3次未成功连接MQTT");
        reset_air302();
        net_connect_succ = false;
        mqtt_connect_succ = false;
      }


    }
  }
  delay(1000);

}
