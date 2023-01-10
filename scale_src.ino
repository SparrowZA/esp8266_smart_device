#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "dictionary.h"
#include "network.h"

#define DEBUG_LEVEL = 1

#if (DEBUG_LEVEL == 1)
#define debug(x) is Serial.print(x)
#define debugln(x) is Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


// const char *SERVER_URL = "http://192.168.1.54:8888/";
const String SERVER_URL = "http://192.168.1.56:8888/";
// const String SERVER_URL = "http://192.168.183.47:8888/scale/";
char *SERVER_ID = "";

float calibration_factor = -20050;      // Calibration factor for 150Kg load scale
float offset_factor = 103660;           // Scale offset for zero'ing

String device_id;

const String SSID_1 = "Martin_Router_King";
const String PASSWORD_1 = "IHaveAStream";
const String SSID_2 = "LAN Grabs";
const String PASSWORD_2 = "NeverAgainShallWe912";
const String SSID_3 = "Are you jus?";
const String PASSWORD_3 = "Geffers2201";

WiFiClient client;
HTTPClient http_client;

void setup(void){
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("setup");
    
    // TODO: Return the IP address bro, not an empty string ******
    String ip_address = connectToNetwork();
    // String ip_address = WiFi.localIP();
    Serial.println(ip_address);

    String tmp_url = SERVER_URL + "register/?ip=" + ip_address;
    Serial.println(tmp_url);

    if (send_http_get(SERVER_URL + "status/")){
        delay(100);
        device_id = register_device(ip_address);
        Serial.print("Device ID: " + device_id);
        // save_device_id(device_id);
    }    
}

//================================================================
// MAIN LOOP
//================================================================

void loop(){
    // test_print("IP Address! 666.666.666.666");

    // char *test_char = "This is initialised.";
    // *test_char = "This is.";
    // test_print(test_char);
    
    // String test_str = "This is a string test";
    // // char *test_char2 = ;
    // // test_print(string_to_char(test_str));
    // // char *test_char2 = test_str.c_str();
    // test_print(test_str.c_str());
    

    Serial.println("Main loop.");
    String data = "name=John&age=20";

    String post_url = SERVER_URL + device_id + "/";
    Serial.println("POST: " + post_url);
    http_client.begin(client, SERVER_URL);
    http_client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http_client.POST(data);
    String content = http_client.getString();
    http_client.end();

    Serial.println(content);
    delay(2000);
}

//================================================================
// MISC FUNCTIONS
//================================================================

const char * string_to_char(String message){
    int str_length = message.length() + 1;
    char char_array[str_length];
    message.toCharArray(char_array, str_length);
    return char_array;
}

void test_print(const char *message){
    Serial.println(message);
}

String connectToNetwork(){
    int network_cnt;

    NetworkConn connections[3] = {
        {"Martin_Router_King", "IHaveAStream"},
        {"LAN Grabs", "NeverAgainShallWe912"}, 
        {"Are you jus?", "Geffers2201"}
    };
    
    // Search for the Access point and return a NetworkConn structure

    network_cnt = WiFi.scanNetworks(false);
    delay(500);
    Serial.println("Number of APs: " + network_cnt);
    for (int i = 0; i < network_cnt; i++){
        for(int j=0; j < (sizeof(connections) / sizeof(connections[0])); j++){

            if(connections[j].ssid.equals(WiFi.SSID(i))){
                // Serial.println(connections[j].ssid + " struct SSID.");
                Serial.println(connections[j].ssid + " network found. Attempting connection...");
                if (connect_to_wifi(connections[j])){
                    return convert_ip_to_string(WiFi.localIP());
                }
                return "";  
            }
        }
    }
    return "";
}

boolean connect_to_wifi(NetworkConn connection){
    Serial.println("Entering Connection function");

    char* chr_ssid = str_to_char_arr(connection.ssid);
    char* chr_pword = str_to_char_arr(connection.password);

    delay(500);

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED) {
        if(retry >= 5){
            Serial.println("Could not connect to WiFi!");
            return false;
        }
        WiFi.begin(chr_ssid, chr_pword);
        int count = 0;
        while (WiFi.status() != WL_CONNECTED) {
            if(count == 8){
                Serial.println("Connection not made. Retrying!");
                break;
            }
            Serial.println("*");
            delay(500);
            count++;
        }
        print_status();
        retry++;
    }

    Serial.println("");
    Serial.println("WiFi Connection Succesful");
    Serial.println("The IP address of the ESP8266 is:");
    Serial.println(WiFi.localIP());
    return true;
}

String convert_ip_to_string(IPAddress ip_address){
    return String(ip_address[0]) + String(".") +
            String(ip_address[1]) + String(".") +
            String(ip_address[2]) + String(".") +
            String(ip_address[3]) + String(".");
}

void print_status(){
    wl_status_t status = WiFi.status();

    switch(status) {
        case WL_CONNECTED:
            Serial.println("WL_CONNECTED");
        case WL_NO_SSID_AVAIL:
            Serial.println("WL_NO_SSID_AVAIL");
        case WL_CONNECT_FAILED:
            Serial.println("WL_CONNECT_FAILED");
        case WL_WRONG_PASSWORD:
            Serial.println("WL_WRONG_PASSWORD");
        case WL_IDLE_STATUS:
            Serial.println("WL_IDLE_STATUS");
        case WL_DISCONNECTED:
            Serial.println("WL_DISCONNECTED");
    }
}

void print_particulars(char* ssid, int length){
    for (int i=0; i < length; i++){
        Serial.print(ssid[i]);
    }
    Serial.println();
}

char* str_to_char_arr(String str){
    Serial.println(str);
    String temp = str;
    char *cstr = new char[temp.length() + 1];
    strcpy(cstr, temp.c_str());
    return cstr;
}

String register_device(String ip_address){
    Serial.println("ip address: " + ip_address);
    String tmp_url = SERVER_URL + "register/?ip=" + ip_address;
    String content = send_http_post(tmp_url);
    String device_id = decode_uuid(content);
    return device_id;
}

String decode_uuid(String post_data){
    String tmp = post_data;
    int colon_idx = 0;
    int quote1_idx = 0;
    int quote2_idx = 0;

    colon_idx = post_data.indexOf(":") ;
    tmp.remove(0, colon_idx);
    Serial.println("no colon: " + tmp);

    quote1_idx = tmp.indexOf("\"");
    Serial.println("Double quote index: " + quote1_idx);
    // tmp = post_data.substring(quote1_idx, post_data.length());
    tmp.remove(0, quote1_idx + 1);
    Serial.println("no first quote:" + tmp);
    
    quote2_idx = tmp.lastIndexOf("\"");
    // tmp = post_data.substring(0, quote2_idx);
    tmp.remove(quote2_idx, tmp.length() - quote2_idx);
    Serial.println("No last quote: " + tmp);
    Serial.println();
    Serial.println();
    delay(3000);
    return tmp;
}

String send_http_post(String url){
    WiFiClient tmp_client;
    HTTPClient tmp_http_client;

    // "http://192.168.1.50:8888/scale/register/?ip" + ip_address
    String content = "";

    for(int i=0; i<5; i++){
        tmp_http_client.begin(tmp_client, url);
        delay(100);

        int http_response = tmp_http_client.GET();
        Serial.println(http_response);
        
        if(http_response == 201){
            content = tmp_http_client.getString();
            // tmp_http_client.end();
            // tmp_client.stop();
            break;
        }
        else{
            Serial.println("An error occured.");
        }
        Serial.println("No response");
        delay(500);
    }

    Serial.println("Content response: " + content);
    
    tmp_http_client.end();
    tmp_client.stop();
    Serial.println("Clients closed");
    return content;
}

boolean send_http_get(String message){
    WiFiClient tmp_client;
    HTTPClient tmp_http_client;
    
    // "http://192.168.1.50:8888/scale/status/"

    for(int i=0; i<5; i++){
        tmp_http_client.begin(tmp_client, message);

        int http_response = tmp_http_client.GET();
        Serial.println(http_response);
        
        if(http_response == 200){
            tmp_http_client.end();
            tmp_client.stop();
            return true;
        }
        else{
            Serial.println("An error occured.");
        }
        Serial.println("No response");
        delay(500);
    }
    tmp_http_client.end();
    tmp_client.stop();
    return false;
}

boolean save_device_id(String device_id){
    
}
