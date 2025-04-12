#ifndef __AM_ALERT__
#define __AM_ALERT__

#include <Arduino.h>
#ifdef ARDUINO_UNOWIFIR4
#include <WiFiS3.h>
#else
#include <WiFi.h>
#include "FS.h"
#endif

typedef enum {
    LONGDO_MAP,
    GOOGLE_MAP
} LINE_Notify_MAP_Service_t;

typedef struct {
    // image
    struct {
        // image from internet
        String url;

        // image from flash / RAM
        struct {
            void *buffer = NULL;
            size_t size = 0;
        } data;

        // image form external storage
#ifndef ARDUINO_UNOWIFIR4
        File file;
#endif
    } image;

    // map
    struct {
        LINE_Notify_MAP_Service_t service = LONGDO_MAP;
        float lat = 0;
        float lng = 0;
        int zoom = 20;
        bool noMaker = false;
        String api_key;
        String option;
    } map;
} Am_Alert_Massage_Option_t;


class AmAlert {
    private:
        String token = "";
        Client *client = NULL;
 
    public:
        int status_code = 0;
        
        AmAlert() ;

        void begin(String token = "", Client *client = NULL) ;
        void setToken(String token) ;
        void setClient(Client *client) ;
        bool send(String massage, Am_Alert_Massage_Option_t *option = NULL) ;

};

extern AmAlert Alert;

#endif