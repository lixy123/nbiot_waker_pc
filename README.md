# wake_by_nb_iot_air302
通过nb_iot唤醒远程PC <br/>
利用badusb技术+物联网nb-iot技术设计的pc唤醒器,也可发命令执行pc休眠等功能<br/>

 <b>一.方案设计:</b><br/>
1.atmega32u4 插入PC usb.<br/>
2.atmega32u4 虚拟串口与air302串口对连, AT命令驱动air302 连接mqtt端<br/>
3.手机安装图形化mqtt指令发送器,发送键盘,鼠标触动数据至mqtt服务端<br/>
4.air302收到mqtt指令后将数据传给atmega32u4,通过usb输出键盘和鼠标模拟操作<br/>
5.键盘和鼠标模拟输出主要目标是唤醒PC,发命令让PC休眠等.<br/>
运行原理：<br/>
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/yuanli.JPG?raw=true' /> 


 <b> 二.硬件</b>  <br/>
  1.CJMCU-Beetle arduino Leonardo USB ATMEGA32U4 Mini Size Development Board <br/>
  2.air302开发板 (或sim7020开发板,AT命令有区别,需调整程序) <br/>
 <br/>
  引脚连接 <br/>
  CJMCU <==> AIR302 <br/>
  5v         5v <br/>
  GND        GND <br/>
  SDA        reset <br/>
  MOSI       TX <br/>
  MISO       RX <br/>

 这是手工DIY的原型板：<br/>

<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/1.jpg?raw=true' /> 
<img src= 'https://github.com/lixy123/nbiot_waker_pc/blob/main/2.jpg?raw=true' /> 

 <b> 三.软件</b>  <br/>
  1.Arduino 1.8.13 打开本代码 <br/>
  2.开发板选择 arduino leonardo <br/>
  3.CJMCU-Beetle插入PC usb <br/>
  4.选择端口，烧录本程序 <br/>

 <b> 四.代码设计细节:</b>  <br/> 
  1.开机检测NB-IOT连接，AT命令连接MQTT <br/>
  2.当收到MQTT的文字，键盘/鼠标输出给宿主机,当前主要功用是唤醒休眠后的PC开机,没做太复杂的功能 <br/>
  3.稳定性检查： <br/>
     A.当收到MQTT服务器中断信息，每60秒后自动重新连接MQTT。 尝试3次仍连接不上MQTT,复位NB-IOT硬件 <br/>
     B.每小时检测AT命令是否能收到反应消息，如无，硬件复位NB-IOT硬件 <br/>
     C.每12时主动触发air302复位一次 <br/>

 <b> 五.用法：</b> <br/>
  1.CJMCU连接好air302(带卡),插入pc的USB口。 <br/>
  2.PC控制面板找到USB的键盘，鼠标设备，设置不节能，允许唤醒。 <br/> 
    将PC进入休眠状态，此CJMCU会保持供电，相当于连接了一个USB的键盘，鼠标设备 <br/>
  3.安卓手机安装MQTT软件IotMTQQPanel, 设置好命令图标，通过图标发送如下协议的MQTT来遥控CJMCU， <br/>
    例如模拟按键盘，点击鼠标，以达到唤醒PC的目的. <br/>
  4.偶尔发现pc用延长线连接CJMCU运行不正常现象,原因是延长线质量不好,降低了电压,电流输出也不够造成.换成直接插PC的USB口正常,优先插用usb3.0口,因为供电更大.

 <b> 六.前景:</b> <br/>
硬件成本约60左右,适合做成钥匙串形式的礼品<br/>
工作与电脑相关度高, 恨不得一天24小时不离电脑的IT背景人士等<br/>





