通过nb_iot唤醒远程PC <br/>
利用badusb技术+物联网nb-iot技术设计的pc唤醒器,也可发命令执行pc休眠等功能<br/>

 <b>一.方案设计:</b><br/>
1.本方案包括两套硬件组合。<br/>
2.第1套硬件: esp32+sim7020 <br/>
注：此处代码直接用了这个产品 LilyGo-T-PCIE,  https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE <br/>
    如果想自己用esp32和普通的sim7020模块组合，需要调整代码中用到的引脚. <br/>
3.第2套硬件: atmega32u4芯片的CJMCU-Beetle+ 蓝牙从设备 <br/>
4.控制端：手机，使用软件IotMTQQPanel作为MQTT客户端 <br/>
5运行原理:<br/>
  A.ESP32与sim7020套装连接至mqtt服务器<br/>
  B.手机通过mqtt软件发送文字至mqtt服务器<br/>
  C.mqtt服务器将文字转发sim7020，sim7020通过串口转发ESP32<br/>
  D.ESP32将文字通过蓝牙透传发送给CJMCU<br/>
  E.CJMCU通过蓝牙透传收到文字，发送键盘,鼠标单击信号给PC/笔记本，完成唤醒PC/笔记本等功能<br/>
  
运行原理图：<br/>
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/yuanli.JPG?raw=true' /> <br/>

注：先前做过一版CJMCU-Beetle直接连接NB-IOT设备的PC唤醒器，效果不好，模块USB键盘在PC上失去连接，导致不可用。可能原因是NB-IOT偶尔瞬间峰值电流较大，休眠的电脑供电不足导致其重启失效.<br/>

 <b> 二.硬件</b>  <br/> 
  硬件图示： <br/> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/all.jpg?raw=true' />  <br/> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/he.jpg?raw=true' />  <br/> 
 
<b>第1套硬件: </b> <br/>
>组成:<br/>
esp32+sim7020<br/>
>硬件资料:<br/>
https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE<br/>
>功能：<br/>
通过NBIOT技术连接mqtt服务器，可随时待命接收MQTT客户端发来的的文字。当收到文字后，通过蓝牙将文字发给第2套硬件<br/>
    
<b>第2套硬件: </b> <br/>
>组成:<br/>
  1.CJMCU-Beetle arduino Leonardo USB ATMEGA32U4 Mini Size Development Board <br/>
  2.蓝牙模块 HC-06 <br/>
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

  此处修改成第2套硬件的蓝牙MAC<br/>
  bt_address[6]  = {0x00, 0x21, 0x13, 0x06, 0x0B, 0xF8};<br/>

  因为mqtt运行原理，代码中此处要修改，一机一用: <br/>
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
  1.ESP32+esp32+sim7020 上电 <br/>
  2.CJMCU 插入pc的USB口。<br/>
  3.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒。 <br/> 
    将PC进入休眠状态，此CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备 <br/>
  4.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU， <br/>
    例如模拟按键盘，点击鼠标，以达到唤醒PC的目的. <br/>
  
 <b> 五.前景:</b> <br/>
硬件成本约200左右<br/>
适合人群：工作与电脑相关度高, 一天24小时不敢离开电脑的IT背景人士等<br/>

 



