/*
 * SwOSHWCam.h
 *
 * Camera hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSHW/SWOSHWCam.h"
#include "SwOSHW/SwOSHWBaseCtrl.h"
#include "SwOSLog.h"
 
/***************************************************
 *
 *   SwOSCAM - Camera
 *
 ***************************************************/


 SwOSCAM::SwOSCAM(const char *name, SwOSCtrl *ctrl, uint8_t flags ) : SwOSIO( name, ctrl, SWOSIO_CAM, flags ) {

  if ( ctrl->isLocal() ) { 
    setupLocal(); 
  }

}

void SwOSCAM::setupLocal() {

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_5;
  config.ledc_timer = LEDC_TIMER_3;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOC_GPIO_NUM;
  config.pin_sccb_scl = SIOD_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    SWARM_LOG_ERROR( TRANSLATE( "Camera init failed with error 0x%x", "Fehler 0x%x beim Initialisieren der Kamera." ), err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

}

void SwOSCAM::serialize( Serialize *serialize ) {

  serialize->startObject();
  SwOSIO::serialize( serialize );
  serialize->item( SERIALIZE_LITERAL_URL,        "/stream" );
  serialize->item( SERIALIZE_LITERAL_FRAMESIZE,  framesize );
  serialize->item( SERIALIZE_LITERAL_QUALITY,    quality );
  serialize->item( SERIALIZE_LITERAL_BRIGHTNESS, brightness );
  serialize->item( SERIALIZE_LITERAL_CONTRAST,   contrast );
  serialize->item( SERIALIZE_LITERAL_SATURATION, saturation );
  serialize->item( SERIALIZE_LITERAL_HMIRROR,    hMirror );
  serialize->item( SERIALIZE_LITERAL_VFLIP,      vFlip );
  serialize->endObject();

}

void SwOSCAM::setRemote( void ) {
  
}

void SwOSCAM::setStreaming( bool onOff, bool dontSendToRemote ) {
  
  streaming = onOff;

  // apply local or remote
  if (ctrl->isLocal())       1==1;  // ToDo Streaming
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setFramesize( framesize_t framesize, bool dontSendToRemote ) {
  
  this->framesize = framesize;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_framesize( esp_camera_sensor_get(), this->framesize );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setQuality( int quality, bool dontSendToRemote ) {
  
  this->quality = quality;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_quality( esp_camera_sensor_get(), this->quality );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setBrightness( int brightness, bool dontSendToRemote ) {
  
  this->brightness = brightness;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_brightness( esp_camera_sensor_get(), this->brightness );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setContrast( int contrast, bool dontSendToRemote ) {
  
  this->contrast = contrast;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_contrast( esp_camera_sensor_get(), this->contrast );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSaturation( int saturation, bool dontSendToRemote ) {
  
  this->saturation = saturation;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_saturation( esp_camera_sensor_get(), this->saturation );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSpecialEffect( int specialEffect, bool dontSendToRemote ) {
  
  this->specialEffect = specialEffect;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_special_effect( esp_camera_sensor_get(), this->specialEffect );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setWbMode( int wbMode, bool dontSendToRemote ) {
  
  this->wbMode = wbMode;

  // apply local or remote
  if (ctrl->isLocal())       esp_camera_sensor_get()->set_wb_mode( esp_camera_sensor_get(), this->wbMode );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setVFlip( bool vFlip, bool dontSendToRemote ) {
  
  this->vFlip = vFlip; 

  // apply local or remote
  if (ctrl->isLocal())   esp_camera_sensor_get()->set_vflip( esp_camera_sensor_get(), this->vFlip );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setHMirror( bool hMirror, bool dontSendToRemote ) {
  
  this->hMirror = hMirror; 

  // apply local or remote
  if (ctrl->isLocal())   esp_camera_sensor_get()->set_hmirror( esp_camera_sensor_get(), this->hMirror );
  else if (!dontSendToRemote) setRemote();

}