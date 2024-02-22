#define BLYNK_TEMPLATE_ID           "TMPLYZBgCgmc"
#define BLYNK_DEVICE_NAME           "RC Car"
#define BLYNK_AUTH_TOKEN            "ZQdrb8G-fOGdpSl7F-728eabn2iXfZMO"


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SaadRasheed";
char pass[] = "12345678";

BLYNK_WRITE(V0)
{
  Serial.println(param.asInt());
}

BLYNK_WRITE(V1)
{
  Serial.println(param.asInt());
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}

void loop()
{
  Blynk.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
