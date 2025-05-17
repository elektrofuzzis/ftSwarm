/*
 * SwOSHWCam.h
 *
 * Camera hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#pragma once

#include "SwOSHWBaseIO.h"

// CAMERA Pins
#define PWDN_GPIO_NUM     GPIO_NUM_NC
#define RESET_GPIO_NUM    GPIO_NUM_NC

#define Y4_GPIO_NUM       GPIO_NUM_14
#define Y3_GPIO_NUM       GPIO_NUM_13
#define Y5_GPIO_NUM       GPIO_NUM_35
#define Y2_GPIO_NUM       GPIO_NUM_7
#define Y6_GPIO_NUM       GPIO_NUM_39
#define PCLK_GPIO_NUM     GPIO_NUM_20
#define Y7_GPIO_NUM       GPIO_NUM_38
#define Y8_GPIO_NUM       GPIO_NUM_37
#define XCLK_GPIO_NUM     GPIO_NUM_4
#define Y9_GPIO_NUM       GPIO_NUM_36
#define HREF_GPIO_NUM     GPIO_NUM_21
#define VSYNC_GPIO_NUM    GPIO_NUM_5
#define SIOC_GPIO_NUM     GPIO_NUM_19
#define SIOD_GPIO_NUM     GPIO_NUM_18

 /***************************************************
 *
 *   SwOSCAM - Camera
 *
 ***************************************************/

 class SwOSCAM : public SwOSIO {
  protected:

    framesize_t framesize  = FRAMESIZE_QVGA;
    int16_t     quality    = 0;
    int16_t     brightness = 0;
    int16_t     contrast   = 0;
    int16_t     saturation = 0;
    int16_t     specialEffect = 0;
    int16_t     wbMode = 0; 
    bool        vFlip = false;
    bool        hMirror = false;
    bool        streaming = false;
    
    // local HW procedures
    virtual void setupLocal(); // initializes local HW
    virtual void setRemote( void );
    
  public:
    // constructor
    SwOSCAM(const char *name, SwOSCtrl *ctrl );

    // administrative stuff
    virtual void jsonize( JSONize *json, uint8_t id);
    virtual bool isCAM( void )   { return true; };

    void setStreaming( bool onOff, bool dontSendToRemote );
    void setFramesize( framesize_t framesize, bool dontSendToRemote );
    void setQuality( int quality, bool dontSendToRemote );
    void setBrightness( int brightness, bool dontSendToRemote );
    void setContrast( int contrast, bool dontSendToRemote );
    void setSaturation( int saturation, bool dontSendToRemote );
    void setSpecialEffect( int specialEffect, bool dontSendToRemote );
    void setWbMode( int wbMode, bool dontSendToRemote );
    void setHMirror( bool hMirror, bool dontSendToRemote );
    void setVFlip( bool vFlip, bool dontSendToRemote );

};