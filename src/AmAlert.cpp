#include "AmAlert.h"
#include "ArduinoJson-v7.4.1.h"
#include "AmAlert_CA.h"
#ifdef ARDUINO_UNOWIFIR4
#include <WiFiSSLClient.h>
#else
#include <WiFiClientSecure.h>
#endif

#ifdef ARDUINO_UNOWIFIR4
#define ESP_LOGI(...) ;
#define ESP_LOGE(...) ;
#endif

static const char * TAG = "Am-Alert";

static String urlEncode(const char *msg) ;
static String urlEncode(String msg) ;

AmAlert::AmAlert() {

}

void AmAlert::begin(String token, Client *client) {
    this->token = token;
    this->client = client;
}

void AmAlert::setToken(String token) {
    this->token = token;
}

void AmAlert::setClient(Client *client) {
    this->client = client;
}

bool AmAlert::send(String massage, Am_Alert_Massage_Option_t *option) {
    if (massage.length() <= 0) {
        ESP_LOGE(TAG, "massage can't empty");
        return false;
    }

    bool imageUpload = false;

    // TODO: use user client for Ethernet support
    if (!this->client) {
#ifdef ARDUINO_UNOWIFIR4
        this->client = new WiFiSSLClient();
#else
        this->client = new WiFiClientSecure();
        if (AmAlert_CA) {
            ((WiFiClientSecure *) this->client)->setCACert(AmAlert_CA);
        } else {
            ((WiFiClientSecure *) this->client)->setInsecure(); 
        }
#endif
    }

    int ret = this->client->connect("alert.artronshop.co.th", 443);
    if (ret <= 0) {
        ESP_LOGE(TAG, "connect to LINE server fail code : %d", ret);
        return false;
    }

    // TODO: add base64 encode for upload image
    JsonDocument doc;

    doc["message"] = massage;
    if (option) {
        if (option->image.url.length() > 0) {
            doc["image"] = option->image.url;
        }
        if (option->map.lat && option->map.lng) {
            if (option->map.service == LONGDO_MAP) {
                String map_url = "https://mmmap15.longdo.com/mmmap/snippet/index.php?width=1000&height=1000";
                map_url += "&lat=" + String(option->map.lat, 9);
                map_url += "&long=" + String(option->map.lng, 9);
                map_url += "&zoom=" + String(option->map.zoom);
                map_url += "&pinmark=" + String(option->map.noMaker ? '0' : '1');
                if (option->map.option.length() > 0) {
                    map_url += "&" + option->map.option;
                }
                map_url += "&HD=1";

                ESP_LOGI(TAG, "Map image URL: %s", map_url.c_str());

                doc["image"] = map_url;
            } else if (option->map.service == GOOGLE_MAP) {
                String map_url = "https://maps.googleapis.com/maps/api/staticmap";
                map_url += "?center=" + String(option->map.lat, 9) + "," + String(option->map.lng, 9);
                map_url += "&markers=color:red%7Clabel:U%7C" + String(option->map.lat, 9) + "," + String(option->map.lng, 9);
                map_url += "&zoom=" + String(option->map.zoom);
                map_url += "&size=1000x1000";
                map_url += "&format=jpg";
                map_url += "&key=" + option->map.api_key;
                if (option->map.option.length() > 0) {
                    map_url += "&" + option->map.option;
                }

                ESP_LOGI(TAG, "Map image URL: %s", map_url.c_str());

                doc["image"] = map_url;
            }
        }
        imageUpload = (option->image.data.buffer) && (option->image.data.size > 0);
    }

    this->client->print("POST /api/push HTTP/1.1\r\n");
    this->client->print("Host: alert.artronshop.co.th\r\n");
    this->client->print("Authorization: Bearer " + this->token + "\r\n");
    this->client->print("User-Agent: ESP32\r\n");
    this->client->print("Content-Type: application/json\r\n");
    this->client->print("Content-Length: " + String(measureJsonPretty(doc)) + "\r\n");
    this->client->print("\r\n");
    serializeJsonPretty(doc, *this->client);

    delay(20); // wait server respond

    long timeout = millis() + 30000;
    bool first_line = true;
    int state = 0;
    while(this->client->connected() && (timeout > millis())) {
        if (this->client->available()) {
            if (state == 0) { // Header
                String line = this->client->readStringUntil('\n');
                if (line.endsWith("\r")) {
                    line = line.substring(0, line.length() - 1);
                }
                ESP_LOGI(TAG, "Header: %s", line.c_str());
                if (first_line) {
                    if (sscanf(line.c_str(), "HTTP/%*f %d", &this->status_code) >= 1) {
                        first_line = false;
                    } else {
                        ESP_LOGE(TAG, "invalid first line");
                    }
                } else {
                    // Header
                    if (line.length() == 0) {
                        state = 2;
                    }
                }
            } else if (state == 2) { // Data
                String line = this->client->readStringUntil('\n');
                if (line.endsWith("\r")) {
                    line = line.substring(0, line.length() - 1);
                }
                ESP_LOGI(TAG, "Data: %s", line.c_str());
                if (line.length() == 0) {
                    break;
                }
            }
        }
        delay(10);
    }
    ESP_LOGI(TAG, "END");
    
    this->client->stop();
    delete this->client;
    this->client = NULL;

    return this->status_code == 200;
}

AmAlert Alert;
