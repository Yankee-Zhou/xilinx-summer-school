#include "sea_esp32_qspi.h"
#include "SimpleDHT.h"
#include "string.h"

#include "AWS_IOT.h"
#include "WiFi.h"

AWS_IOT hornbill;

char WIFI_SSID[]="FAST_55CF92";
char WIFI_PASSWORD[]="15090469063";
char HOST_ADDRESS[]="a27dn167878u2a-ats.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "esp32_Client";
char TOPIC_NAME[]= "$aws/things/esp32-iot/shadow/update";


int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0,tick1=0;
char payload[512];
char rcvdPayload[512];
uint8_t data1[4] = {41,42,43,44};
uint8_t data2[32];
void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}


void setup()
{
  Serial.begin(115200);
  SeaTrans.begin();
  //SeaTrans.write(0, data1, 4);
  //SeaTrans.read(0, data2, 4);
  //Serial.printf("%d %d %d %d\r\n",data2[0],data2[1],data2[2],data2[3]);
  delay(2000);

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // wait 5 seconds for connection:
        delay(5000);
    }

    Serial.println("Connected to wifi");

    if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfully");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }

    delay(2000);
  
}

void loop()
{
    
    /*
    if(msgReceived == 1)
    {
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
    }
    */
    int tag = 0;
    int count=0;
    float _data[3]={0,0,0};
    int flag[3]={0,0,0};
    while(1){
        SeaTrans.read(0, data2, 16);
        
        _data[0]=data2[3];
        _data[1]=data2[5];
        _data[2]=data2[7];
        
        for(int i=0;i<3;i++){
          if (_data[i]>50 && _data[i] < 150) {
            if (flag[i] == 0) {
              tag = 1;
              flag[i] = 1;
              break;
            }
            flag[i]= 1;
        }
          else if (_data[0] < 200 && _data[0]>150) {
            if (flag[i] == 1) {
              tag = 1;
              flag[i] = 0;
              break;
            }
            flag[i]= 0;
          }
        }
        
      if (tag == 1){
        count++;
        if(count%2==0){
          Serial.println(count/2);
          sprintf(payload,"{\"key\":\"%d\"}",count/2);//json:{"key":"value"} 字符而不是结尾:\"
          printf("%s\n",payload);
          if(hornbill.publish(TOPIC_NAME,payload) == 0)
          {        
              Serial.println("Publish successfully!");
              //Serial.println(payload);
          }
          else
          {
              Serial.println("Publish failed!");
          }
        }
      }
      tag=0;    
      delay(50);  
  }   
}
