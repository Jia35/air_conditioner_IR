#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid = "ES715_TP_2.4G";
const char* password = "********";
// config static IP
IPAddress ip(192, 168, 0, 177); // where xx is the desired IP Address
IPAddress gateway(192, 168, 0, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
WiFiServer server(8035);

// 設定IR接腳
IRsend irsend(14);

String key_on = "181E009D";
uint16_t out[131];

void TECO_decode(String key);

void setup()
{
  Serial.begin(115200);
  irsend.begin();
  pinMode(15, INPUT);

  // Connect to WiFi network
	WiFi.begin(ssid, password);
	// Static IP Setup Info Here...
	WiFi.config(ip, gateway, subnet);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
  
	// Start the server
	Serial.println("WiFi connected, using IP address: ");
  Serial.println(WiFi.localIP());
	server.begin();
}

void loop()
{ 
	// Check if a client has connected
	WiFiClient client = server.available();
	if (!client) {
		return;
	}
	// Wait until the client sends some data
	while(!client.available()){
		delay(1);
	}
  
	// Read the first line of the request
	String req = client.readStringUntil('\r');
	client.flush();
	
	// Match the request
	if (req.indexOf("/air/") != -1) {
		int A = req.indexOf("/air/")+5;
		// IR解碼(TECO冷氣)
		TECO_decode(req.substring(A, A+8));
  	irsend.sendRaw(out, sizeof(out)/sizeof(out[0]), 38);
		
		String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
		client.print(s);
	}
	else if (req.indexOf("/online/") != -1) {
		String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
		client.print(s);
	}
	else {
		client.stop();
		return;
	}
  
}


// IR解碼(TECO冷氣)
void TECO_decode(String key) {
	char key_char[8];
	uint16_t key_out[64];
	int H=2300, L=900;

  // 字串轉CharArray
  key.toCharArray(key_char, key.length()+1);
  // 依序解碼(反向)
  for(int i=0;i<strlen(key_char);i++) {
    key_out[i*8+1] = 350; key_out[i*8+3] = 350; key_out[i*8+5] = 350; key_out[i*8+7] = 350;
		//Serial.println(key_char[strlen(key_char)-1-i]);
		
    // 十六進制轉二進制並解碼(反向)
    switch (key_char[strlen(key_char)-1-i]) {
    case '0':
      key_out[i*8] = L; key_out[i*8+2] = L; key_out[i*8+4] = L; key_out[i*8+6] = L;
      break;
    case '1':
      key_out[i*8] = H; key_out[i*8+2] = L; key_out[i*8+4] = L; key_out[i*8+6] = L;
      break;
    case '2':
      key_out[i*8] = L; key_out[i*8+2] = H; key_out[i*8+4] = L; key_out[i*8+6] = L;
      break;
    case '3':
      key_out[i*8] = H; key_out[i*8+2] = H; key_out[i*8+4] = L; key_out[i*8+6] = L;
      break;
    case '4':
      key_out[i*8] = L; key_out[i*8+2] = L; key_out[i*8+4] = H; key_out[i*8+6] = L;
      break;
    case '5':
      key_out[i*8] = H; key_out[i*8+2] = L; key_out[i*8+4] = H; key_out[i*8+6] = L;
      break;
    case '6':
      key_out[i*8] = L; key_out[i*8+2] = H; key_out[i*8+4] = H; key_out[i*8+6] = L;
      break;
    case '7':
      key_out[i*8] = H; key_out[i*8+2] = H; key_out[i*8+4] = H; key_out[i*8+6] = L;
      break;
    case '8':
      key_out[i*8] = L; key_out[i*8+2] = L; key_out[i*8+4] = L; key_out[i*8+6] = H;
      break;
    case '9':
      key_out[i*8] = H; key_out[i*8+2] = L; key_out[i*8+4] = L; key_out[i*8+6] = H;
      break;
    case 'A':
      key_out[i*8] = L; key_out[i*8+2] = H; key_out[i*8+4] = L; key_out[i*8+6] = H;
      break;
    case 'B':
      key_out[i*8] = H; key_out[i*8+2] = H; key_out[i*8+4] = L; key_out[i*8+6] = H;
      break;
    case 'C':
      key_out[i*8] = L; key_out[i*8+2] = L; key_out[i*8+4] = H; key_out[i*8+6] = H;
      break;
    case 'D':
      key_out[i*8] = H; key_out[i*8+2] = L; key_out[i*8+4] = H; key_out[i*8+6] = H;
      break;
    case 'E':
      key_out[i*8] = L; key_out[i*8+2] = H; key_out[i*8+4] = H; key_out[i*8+6] = H;
      break;
    case 'F':
      key_out[i*8] = H; key_out[i*8+2] = H; key_out[i*8+4] = H; key_out[i*8+6] = H;
      break;
    default :
    	break;
    }

	}
	
  // 輸出訊號 = 前置訊號 + 解碼訊號 + 中間訊號 + 解碼訊號
  // 加入前置訊號
	out[0] = 4900;
	for(int i=0; i<64; i++) {
    out[i+1] = key_out[i];
	}
  // 加入中間訊號
	out[65] = 8000;
  for(int i=0; i<64; i++) {
    out[i+66] = key_out[i];
	}

}
