通过nb_iot唤醒远程PC <br/>
利用badusb技术+物联网nb-iot技术设计的pc唤醒器,也可发命令执行pc休眠等功能<br/>

 <b>一.方案设计:</b><br/>
1.本方案包括两套硬件组合。<br/>
2.第1套硬件: NBIOT版本的MQTT转蓝牙透传发送器<br/>
    注：本软件代码用的是这个产品 LilyGo-T-PCIE  <br/>
    https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE <br/>
3.第2套硬件: 蓝牙透传接收器转Badusb<br/>
4.控制端：手机，使用软件IotMTQQPanel作为MQTT客户端 <br/>
5运行原理:<br/>
  A.ESP32利用NBIOT模块连接至mqtt服务器, 随时可接收mqtt文字<br/>
  B.手机通过mqtt软件发送文字至mqtt服务器<br/>
  C.mqtt服务器将文字传送给NBIOT模块 <br/>
  D.NBIOT模块将文字通过串口转发ESP32<br/>
  E.ESP32将文字通过蓝牙透传发送给Badusb<br/>
  F.Badusb分析蓝牙收到文字，发送键盘,鼠标单击信号给PC/笔记本，完成唤醒PC/笔记本等功能<br/>
  
运行原理：<br/>
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/yuanli.JPG?raw=true' /> <br/>

注：先前做过一版CJMCU-Beetle直连NB-IOT设备的PC唤醒器，实测效果不好. Badusb在休眠的PC上经常中断失去连接，疑似NB-IOT瞬间电流较大，因为休眠的PC供电不足导致其重启失效.<br/>

 <b> 二.硬件</b>  <br/> 
  硬件图示： <br/> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/all.jpg?raw=true' />  <br/> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/he.jpg?raw=true' />  <br/> 
 
<b>第1套硬件: </b> NBIOT版本的MQTT转蓝牙透传发送器 <br/>
>组成:<br/>
esp32+sim7020 <br/>
>硬件资料:<br/>
https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE <br/>
注:也可以用普通esp32+sim7020自制,不一定需要用此模块,但引脚号需要调整,没有上面的模块省事.外观简洁. <br/>
>功能：<br/>
通过NBIOT技术连接mqtt服务器，可随时待命接收MQTT客户端发来的的文字。当收到文字后，通过蓝牙将文字发给第2套硬件<br/>
    
<b>第2套硬件: </b>蓝牙透传接收器转Badusb <br/>
>组成:<br/>
  1.CJMCU-Beetle arduino Leonardo USB ATMEGA32U4 Mini Size Development Board <br/>
  2.蓝牙模块 HC-06 (注：使用前清除与其它蓝牙模块的配对记忆,否则连接不上) <br/>
>硬件资料:<br/>
https://www.dtmao.cc/news_show_906322.shtml <br/>
>连线
  CJMCU <==> 透传蓝牙 <br/>
  5v         5v <br/>
  GND        GND <br/>
  MOSI       TX <br/>
  MISO       RX   <br/>

 <b> 三.软件</b>  <br/>
 第1套硬件: <br/>
  1.Arduino 1.8.13 打开代码 esp32_nb_iot_sim7020_pcie <br/>
  2.开发板选择 ESP32 Dev Module <br/>
  3.ESP32 插入PC <br/>
  4.选择端口，烧录本程序 <br/>
  说明：<br/>

  库文件:<br/>
  https://github.com/espressif/arduino-esp32 版本:1.0.6 (注：2.0版本蓝牙库有bug, 蓝牙发送时ESP32会重启)

  代码修改处：<br/>
  此处修改成第2套硬件的蓝牙MAC<br/>
  bt_address[6]  = {0x00, 0x21, 0x13, 0x06, 0x0B, 0xF8}; (注:根据实际蓝牙MAC调整)

  因为mqtt运行原理，代码中此处要修改，一机一用 (注:如不修改,MQTT接收会与他人用的产生冲突)<br/>
  String mqtt_clientid = "clientesp32_s2"; <br/>
  String mqtt_topic = "/clientesp32_s2"; <br/>
  String mqtt_topic_resp = "/clientesp32_s2/resp"; <br/>

 第2套硬件:<br/>
  1.Arduino 1.8.13 打开本代码<br/>
  2.开发板选择 arduino leonardo<br/>
  3.CJMCU-Beetle插入PC usb<br/>
  4.选择端口，烧录本程序<br/>
  说明：<br/>
  目前实现解释器：模拟键盘回车,模拟键盘输出字符串(例：输出密码，输出PC休眠命令)，模拟鼠标单击<br/>
  
 <b> 四.用法：</b> <br/>
  1.ESP32+sim7020 上电 <br/>
  2.CJMCU 插入pc的USB口<br/>
  3.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒 <br/> 
    将PC进入休眠状态，此CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备 <br/>
  4.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU <br/>
    实现通过模拟键盘，鼠标来唤醒PC,休眠PC,或输入密码开机等功能 <br/>
  
 <b> 五.前景:</b> <br/>
硬件成本约200-300左右<br/>
适合人群：
1.工作与电脑相关度高, 一天24小时不敢离开电脑的IT背景人士<br/>
2.通信,网管远程维护人员<br/>

 



