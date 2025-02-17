/*
 * SwOSHWCam.h
 *
 * Camera hardware implementation
 * 
 * (C) 2021-25 Christian Bergschneider & Stefan Fuss
 * 
 */

 #include "SwOSHW/SwOSHWCam.h"
 #include "SwOSHW/SwOSHWBaseCtrl.h"
 
 /***************************************************
 *
 *   SwOSCAM - Camera
 *
 ***************************************************/


 SwOSCAM::SwOSCAM(const char *name, SwOSCtrl *ctrl ) : SwOSIO( name, ctrl ) {

  if ( _ctrl->isLocal() ) { 
    _setupLocal(); 
  }

}
/*

esp_err_t xclk_timer_conf2(int ledc_timer, int xclk_freq_hz)
{
    ledc_timer_config_t timer_conf;
    timer_conf.duty_resolution = LEDC_TIMER_1_BIT;
    timer_conf.freq_hz = xclk_freq_hz;
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;

#if ESP_IDF_VERSION_MAJOR >= 4
    timer_conf.clk_cfg = LEDC_AUTO_CLK;
#endif
    timer_conf.timer_num = (ledc_timer_t)ledc_timer;
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed for freq %d, rc=%x", xclk_freq_hz, err);
    }
    return err;
}

esp_err_t camera_enable_out_clock2(const camera_config_t* config)
{
    esp_err_t err = xclk_timer_conf2(config->ledc_timer, config->xclk_freq_hz);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed, rc=%x", err);
        return err;
    }

    // g_ledc_channel = config->ledc_channel;
    ledc_channel_config_t ch_conf;
    ch_conf.gpio_num = config->pin_xclk;
    ch_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_conf.channel = config->ledc_channel;
    ch_conf.intr_type = LEDC_INTR_DISABLE;
    ch_conf.timer_sel = config->ledc_timer;
    ch_conf.duty = 1;
    ch_conf.hpoint = 0;
    err = ledc_channel_config(&ch_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed, rc=%x", err);
        return err;
    }
    return ESP_OK;
}

void blubber() {
  // initialize local HW

  ledc_channel_config_t *ledc_channel = NULL;
  
  // set digital ports _in1 & in2 to output
  gpio_config_t io_conf = {
    .pin_bit_mask = 0,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  io_conf.pin_bit_mask = io_conf.pin_bit_mask | (1ULL << GPIO_NUM_4);
  gpio_config(&io_conf);
  
  // set motor driver off
  gpio_set_level( GPIO_NUM_4, 0 );

  // use Timer 0
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_12_BIT,
    .timer_num        = LEDC_TIMER_3,
    .freq_hz          = 10000,
    .clk_cfg          = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // just prepare led channel, don't register yet
  ledc_channel = (ledc_channel_config_t *) calloc( sizeof( ledc_channel_config_t ), 1 );
  ledc_channel->gpio_num       = GPIO_NUM_4;
  ledc_channel->speed_mode     = LEDC_LOW_SPEED_MODE;
  ledc_channel->channel        = LEDC_CHANNEL_5;
  ledc_channel->intr_type      = LEDC_INTR_DISABLE;
  ledc_channel->timer_sel      = LEDC_TIMER_3;
  ledc_channel->duty           = 0; 
  ledc_channel->hpoint         = 0;
  ledc_channel->flags.output_invert = 1;

  gpio_reset_pin( (gpio_num_t) ledc_channel->gpio_num );
  ledc_channel_config( ledc_channel );
  ESP_ERROR_CHECK( ledc_set_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel, 128 ) );
  ESP_ERROR_CHECK( ledc_update_duty( LEDC_LOW_SPEED_MODE, ledc_channel->channel ) );
  gpio_set_level( GPIO_NUM_4, 1 );

}

*/

void SwOSCAM::_setupLocal() {

  return;

/*
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
config.pin_sccb_sda = SIOD_GPIO_NUM;
config.pin_sccb_scl = SIOC_GPIO_NUM;
config.pin_pwdn = PWDN_GPIO_NUM;
config.pin_reset = RESET_GPIO_NUM;
config.xclk_freq_hz = 20000000;
config.frame_size = FRAMESIZE_UXGA;
config.pixel_format = PIXFORMAT_JPEG; // for streaming
config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
config.fb_location = CAMERA_FB_IN_PSRAM;
config.jpeg_quality = 12;
config.fb_count = 1;
if(config.pixel_format == PIXFORMAT_JPEG){
  if(psramFound()){
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }
} else {
  // Best option for face detection/recognition
  config.frame_size = FRAMESIZE_240X240;
}
*/

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
//config.xclk_freq_hz = 10000;
config.frame_size = FRAMESIZE_SVGA;
config.pixel_format = PIXFORMAT_JPEG; // for streaming
config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
config.fb_location = CAMERA_FB_IN_DRAM;
config.jpeg_quality = 12;
config.fb_count = 1;

// analogWrite( 4, 128 );
// blubber();
// return;

// camera_enable_out_clock2(&config);
// return;

// camera init
esp_err_t err = esp_camera_init(&config);
if (err != ESP_OK) {
  Serial.printf("Camera init failed with error 0x%x", err);
  return;
}

sensor_t * s = esp_camera_sensor_get();
// drop down frame size for higher initial frame rate
if(config.pixel_format == PIXFORMAT_JPEG){
  s->set_framesize(s, FRAMESIZE_QVGA);
}

}

void SwOSCAM::jsonize( JSONize *json, uint8_t id) {

  json->startObject();
  SwOSIO::jsonize(json, id);
  json->variable("url", (char *) "/stream" );
  json->variableUI8( "framesize",  _framesize );
  json->variableUI8( "quality",    _quality );
  json->variableI16( "brightness", _brightness );
  json->variableI16( "contrast",   _contrast );
  json->variableI16( "saturation", _saturation );
  json->variableB( "H-Mirror", _hMirror );
  json->variableB( "v-Flip",   _vFlip );
  json->endObject();

}

void SwOSCAM::setRemote( void ) {
  
}

void SwOSCAM::streaming( bool onOff, bool dontSendToRemote ) {
  
  _streaming = onOff;

  // apply local or remote
  if (_ctrl->isLocal())       1==1;  // ToDo Streaming
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setFramesize( framesize_t framesize, bool dontSendToRemote ) {
  
  _framesize = framesize;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_framesize( esp_camera_sensor_get(), framesize );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setQuality( int quality, bool dontSendToRemote ) {
  
  _quality = quality;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_quality( esp_camera_sensor_get(), quality );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setBrightness( int brightness, bool dontSendToRemote ) {
  
  _brightness = brightness;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_brightness( esp_camera_sensor_get(), brightness );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setContrast( int contrast, bool dontSendToRemote ) {
  
  _contrast = contrast;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_contrast( esp_camera_sensor_get(), contrast );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSaturation( int saturation, bool dontSendToRemote ) {
  
  _saturation = saturation;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_saturation( esp_camera_sensor_get(), saturation );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setSpecialEffect( int specialEffect, bool dontSendToRemote ) {
  
  _specialEffect = specialEffect;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_special_effect( esp_camera_sensor_get(), specialEffect );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setWbMode( int wbMode, bool dontSendToRemote ) {
  
  _wbMode = wbMode;

  // apply local or remote
  if (_ctrl->isLocal())       esp_camera_sensor_get()->set_wb_mode( esp_camera_sensor_get(), wbMode );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setVFlip( bool vFlip, bool dontSendToRemote ) {
  
  _vFlip = vFlip; 

  // apply local or remote
  if (_ctrl->isLocal())   esp_camera_sensor_get()->set_vflip( esp_camera_sensor_get(), vFlip );
  else if (!dontSendToRemote) setRemote();

}

void SwOSCAM::setHMirror( bool hMirror, bool dontSendToRemote ) {
  
  _hMirror = hMirror; 

  // apply local or remote
  if (_ctrl->isLocal())   esp_camera_sensor_get()->set_hmirror( esp_camera_sensor_get(), hMirror );
  else if (!dontSendToRemote) setRemote();

}