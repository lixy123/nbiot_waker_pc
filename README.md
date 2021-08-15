# wake_by_nb_iot_air302
通过nb_iot唤醒远程PC <br/>
利用badusb技术+物联网nb-iot技术设计的pc唤醒器,也可发命令执行pc休眠等功能<br/>

 <b>一.方案设计:</b><br/>
1.本方案包括两套硬件组合。<br/>
2.第1套硬件: esp32-s2+nb-iot设备+蓝牙主设备。<br/>
  注：用esp32也行，用esp32-s2是因为我手头正好有一块esp32-s2.<br/>
3.第2套硬件: atmega32u4芯片的CJMCU-Beetle+蓝牙从设备。<br/>
4.控制端：手机，使用软件IotMTQQPanel作为MQTT客户端 <br/>
5运行原理:<br/>
  A.ESP32通过串口与air302连接, AT命令驱动air302连接至mqtt服务器<br/>
  B.手机通过mqtt软件发送文字至mqtt服务器<br/>
  C.mqtt服务器将文字转发air302，air302通过串口转发ESP32-s2<br/>
  D.ESP32-s2将文字通过蓝牙透传转发CJMCU<br/>
  E.CJMCU通过蓝牙透传收到文字，发送键盘,鼠标单击信号给PC/笔记本，完成唤醒PC/笔记本等功能<br/>
运行原理图：<br/>
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/yuanli.JPG?raw=true' /> <br/>

注：先前做过一版CJMCU-Beetle直接连接NB-IOT设备的PC唤醒器，效果不好，badusb脱机概率较大，可能原因是NB-IOT偶尔瞬间峰值电流较大，休眠的电脑供电不足导致其重启失效.<br/>

 <b> 二.硬件</b>  <br/> 
<b>第1套硬件: </b> <br/>
  1.ESP32<br/>
  2.air302<br/>
  引脚连接 <br/>
  ESP32-S2 <==> AIR302 <br/>
  5v         5v <br/>
  GND        GND <br/>
  5          reset <br/>
  1          TX <br/>
  2          RX <br/>
  3.蓝牙透传设备 <br/>
  ESP32-S2 <==> 蓝牙透传(HC05,HC08均可，配置好透传) <br/>
  5v/或3.3V  5v <br/>
  GND        GND <br/>
  4          RX <br/>
  3          TX <br/>
  
<b>第2套硬件: </b> <br/>
  1.CJMCU-Beetle arduino Leonardo USB ATMEGA32U4 Mini Size Development Board <br/>
  2.蓝牙透传 <br/>
  CJMCU <==> 透传蓝牙 <br/>
  5v         5v <br/>
  GND        GND <br/>
  MOSI       TX <br/>
  MISO       RX   <br/>

<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/1.jpg?raw=true' /> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/2.jpg?raw=true' /> 

 <b> 三.软件</b>  <br/>
 第1套硬件: <br/>
  1.Arduino 1.8.13 打开本代码 <br/>
  2.开发板选择 ESP32-S2 <br/>
  3. ESP32-S2插入PC usb <br/>
  4.选择端口，烧录本程序 <br/>
  说明：<br/>
  ESP32-S2通过串口连接air302获得了NB-IOT网络连接使用MQTT协议的能力 <br/>
  ESP32-S2通过串口连接蓝牙硬件获得与其它蓝牙硬件透传的能力 <br/>


 第2套硬件:<br/>
  1.Arduino 1.8.13 打开本代码<br/>
  2.开发板选择 arduino leonardo<br/>
  3.CJMCU-Beetle插入PC usb<br/>
  4.选择端口，烧录本程序<br/>
  说明：<br/>
  目前实现解释器：模拟键盘回车,模拟键盘输出字符串(例：输出密码，输出PC休眠命令)，模拟鼠标单击<br/>
  

 <b> 四.用法：</b> <br/>
  1.ESP32连接好air302(带卡),蓝牙透传设备 接供电设备 <br/>
  2.CJMCU 插入pc的USB口。
  3.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒。 <br/> 
    将PC进入休眠状态，此CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备 <br/>
  4.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU， <br/>
    例如模拟按键盘，点击鼠标，以达到唤醒PC的目的. <br/>
  

 <b> 五.前景:</b> <br/>
硬件成本约100左右<br/>
适合人群：工作与电脑相关度高, 一天24小时不敢离开电脑的IT背景人士等<br/>





