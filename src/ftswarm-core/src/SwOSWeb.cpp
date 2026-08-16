/*
 * SwOSWEB.h
 *
 * fTSwarm builtin WebServer
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include <esp_err.h>
#include <esp_http_server.h>

#include <freertos/task.h>
#include <cstring>

#include "serialize.h"
#include "SwOSNVS.h"
#include "SwOSHW.h"
#include "SwOSHW/SwOSHWLocal.h"
#include "SwOSSwarm.h"
#include "SwOSWeb.h"
#include "sfs_files.h"
#include "SwOSLog.h"
#include "SwOSCLI.h"

#define GETSWARMDELAY 1000

#define SCRATCH_BUFSIZE (10240)
#define HTTPD_401 "401 Unauthorized"

typedef struct http_server_context {
    char base_path[16 + 1];
    char scratch[SCRATCH_BUFSIZE];
} http_server_context_t;

httpd_handle_t UIServer = NULL;
httpd_handle_t streamServer = NULL;
int authenticatedSession = -1;

esp_err_t indexHandler(httpd_req_t *req, httpd_err_code_t err) {
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  httpd_resp_set_type(req, "text/html"); 

  #define CHUNK_SIZE 4096
  size_t remaining = sfs_index_html_gz_len;
  const char *data_ptr = sfs_index_html_gz;

  while (remaining > 0) {
      size_t to_send = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
      
      esp_err_t err = httpd_resp_send_chunk(req, data_ptr, to_send);
      if (err != ESP_OK) {
          SWARM_LOG_ERROR("Failed to send chunk, error: %d", err);
          return err; 
      }
      
      data_ptr += to_send;
      remaining -= to_send;
  }

  esp_err_t final_err = httpd_resp_send_chunk(req, NULL, 0);
  if (final_err != ESP_OK) {
    SWARM_LOG_ERROR("Final chunk termination failed: %d", final_err);
    return final_err;
  }

  return ESP_OK;
}

#if FTSWARM_HAL_OLEDS > 0

static void pngWriteUint32(uint8_t *buf, uint32_t value) {
    buf[0] = (value >> 24) & 0xff;
    buf[1] = (value >> 16) & 0xff;
    buf[2] = (value >> 8) & 0xff;
    buf[3] = value & 0xff;
}

static uint32_t pngCrc32(const uint8_t *data, size_t len) {
    static uint32_t table[256];
    static bool tableReady = false;

    if (!tableReady) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (uint32_t j = 0; j < 8; j++) {
                if (c & 1) c = 0xedb88320UL ^ (c >> 1);
                else c >>= 1;
            }
            table[i] = c;
        }
        tableReady = true;
    }

    uint32_t crc = 0xffffffffUL;
    for (size_t i = 0; i < len; i++) {
        crc = table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ^ 0xffffffffUL;
}

static uint8_t *pngAppendChunk(uint8_t *dst, const char *type, const uint8_t *data, size_t data_len) {
    pngWriteUint32(dst, data_len);
    dst += 4;
    uint8_t *chunkStart = dst;
    memcpy(dst, type, 4);
    dst += 4;
    if (data && data_len) {
        memcpy(dst, data, data_len);
        dst += data_len;
    }
    uint32_t crc = pngCrc32(chunkStart, 4 + data_len);
    pngWriteUint32(dst, crc);
    dst += 4;
    return dst;
}

static esp_err_t sendOledScreenshot(httpd_req_t *req) {
    uint8_t *fb = oled.getDisplayBuffer();
    if (!fb) return ESP_FAIL;

    const uint32_t width = oled.getScreenWidth();
    const uint32_t height = oled.getDisplayHeight();
    const uint32_t outWidth = width + 4;
    const uint32_t outHeight = height + 4;
    const size_t bytes_per_row = outWidth * 3;
    const size_t raw_size = outHeight * (1 + bytes_per_row);
    const size_t zlib_size = 2 + 5 + raw_size + 4;
    const size_t png_size = 8 + 25 + 12 + zlib_size + 12;

    uint8_t *png = (uint8_t *)calloc(1, png_size);
    if (!png) return ESP_ERR_NO_MEM;

    uint8_t *p = png;
    static const uint8_t pngSignature[8] = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
    memcpy(p, pngSignature, sizeof(pngSignature));
    p += sizeof(pngSignature);

    uint8_t ihdr[13];
    pngWriteUint32(ihdr + 0, outWidth);
    pngWriteUint32(ihdr + 4, outHeight);
    ihdr[8] = 8;    // bit depth
    ihdr[9] = 2;    // color type: truecolor RGB
    ihdr[10] = 0;   // compression method
    ihdr[11] = 0;   // filter method
    ihdr[12] = 0;   // interlace method
    p = pngAppendChunk(p, "IHDR", ihdr, sizeof(ihdr));

    uint8_t *zlib = (uint8_t *)calloc(1, zlib_size);
    if (!zlib) { free(png); return ESP_ERR_NO_MEM; }

    uint8_t *q = zlib;
    *q++ = 0x78; // zlib CMF
    *q++ = 0x01; // zlib FLG (no compression, fastest)

    *q++ = 0x01; // DEFLATE block header: final block, no compression

    uint16_t len = (uint16_t)raw_size;
    *q++ = len & 0xff;
    *q++ = (len >> 8) & 0xff;
    *q++ = (~len) & 0xff;
    *q++ = ((~len) >> 8) & 0xff;

    uint32_t a = 1;
    uint32_t b = 0;

    for (uint32_t y = 0; y < outHeight; y++) {
        *q++ = 0;
        a += 0;
        if (a >= 65521) a -= 65521;
        b += a;
        if (b >= 65521) b -= 65521;

        for (uint32_t x = 0; x < outWidth; x++) {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t bcol = 0;
            if (x >= 2 && x < width + 2 && y >= 2 && y < height + 2) {
                uint32_t srcX = x - 2;
                uint32_t srcY = y - 2;
                uint32_t index = srcX + (srcY / 8) * width;
                uint8_t pixel = (fb[index] >> (srcY & 7)) & 1;
                if (pixel) {
                    if (srcY < 16) {
                        r = 255; g = 215; bcol = 0; // gold
                    } else {
                        r = 0; g = 255; bcol = 255; // cyan
                    }
                }
            }
            *q++ = r;
            *q++ = g;
            *q++ = bcol;
            a += r;
            if (a >= 65521) a -= 65521;
            b += a;
            if (b >= 65521) b -= 65521;
            a += g;
            if (a >= 65521) a -= 65521;
            b += a;
            if (b >= 65521) b -= 65521;
            a += bcol;
            if (a >= 65521) a -= 65521;
            b += a;
            if (b >= 65521) b -= 65521;
        }
    }

    uint32_t adler = (b << 16) | a;
    *q++ = (adler >> 24) & 0xff;
    *q++ = (adler >> 16) & 0xff;
    *q++ = (adler >> 8) & 0xff;
    *q++ = adler & 0xff;

    size_t actual_zlib_len = q - zlib;
    p = pngAppendChunk(p, "IDAT", zlib, actual_zlib_len);
    free(zlib);

    p = pngAppendChunk(p, "IEND", NULL, 0);
    size_t total_size = p - png;

    httpd_resp_set_type(req, "image/png");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=\"oled_screenshot.png\"");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_err_t err = httpd_resp_send(req, (const char *)png, total_size);
    free(png);
    return err;
}

static esp_err_t screenshotUriHandler(httpd_req_t *req) {
    return sendOledScreenshot(req);
}

#endif

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    static int64_t last_frame = 0;
    if (!last_frame) {
      last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
      return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true) {
      fb = esp_camera_fb_get();
      
      if (!fb) {
        log_e("Camera capture failed");
        res = ESP_FAIL;
      
      } else {
        _timestamp.tv_sec  = fb->timestamp.tv_sec;
        _timestamp.tv_usec = fb->timestamp.tv_usec;
        
        if (fb->format != PIXFORMAT_JPEG) {    
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            log_e("JPEG compression failed");
            res = ESP_FAIL;
          }

        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }

      }
      
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if (res != ESP_OK ) { SWARM_LOG_ERROR("Stream error 1 %d", res ); }
      }

      
      if (res == ESP_OK) {
        size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res != ESP_OK ) { SWARM_LOG_ERROR("Strem error 2 %d", res ); }
      }
        
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        if (res != ESP_OK ) { SWARM_LOG_ERROR("Stream error 3 %d", res ); }
      }

      if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;

      } else if (_jpg_buf) {
        free(_jpg_buf);
        _jpg_buf = NULL;
      }
      
      if (res != ESP_OK) {
        log_e("Send frame failed");
        break;
      }
      
      /*
      int64_t fr_end = esp_timer_get_time();
      int64_t frame_time = fr_end - last_frame;
      frame_time /= 1000;

      log_i("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)", (uint32_t)(_jpg_buf_len), (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time, avg_frame_time, 1000.0 / avg_frame_time );
      */

    }

    return res;
}

struct async_resp_arg {
    httpd_handle_t handle;
    int fd;
    uint8_t* message;
};

#define MAXWSTASKPAYLOAD 50 * 1024

static void wsTask( void *args ) {

  size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;

  int client_fds[max_clients];
  int client_info;
  httpd_ws_frame_t ws_pkt;

  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type    = HTTPD_WS_TYPE_BINARY;
  ws_pkt.payload = (uint8_t*) calloc( MAXWSTASKPAYLOAD, 1 );
  
  Serialize        serialize( (char*) ws_pkt.payload, MAXWSTASKPAYLOAD, SERIALIZE_RAW );

  while(1) {

    max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    esp_err_t ret = httpd_get_client_list( UIServer, &max_clients, client_fds);

    // anybody connected?
    if ( max_clients != 0 ) {

      serialize.reset();
      myOSSwarm.serialize( &serialize );
      ws_pkt.len = strlen( (char *)  ws_pkt.payload );

      for (int i = 0; i < max_clients; i++) {

        client_info = httpd_ws_get_fd_info( UIServer, client_fds[i]) ;
        
        if ( client_info == HTTPD_WS_CLIENT_WEBSOCKET ) httpd_ws_send_frame_async( UIServer, client_fds[i], &ws_pkt );

      }

    }

    delay( GETSWARMDELAY );

  }

}

static void wsAsyncHandler( void *varg )
{
    httpd_ws_frame_t ws_pkt;
    struct async_resp_arg* arg = (async_resp_arg*) varg;

    char *response;
    SwOSCLI cli;
    bool loggedIn = ( arg->fd == authenticatedSession );

    response = cli.eval( (char*) arg->message, &loggedIn );

    if ( loggedIn ) authenticatedSession = arg->fd;
    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)response;
    ws_pkt.len     = strlen(response);
    ws_pkt.type    = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async( arg->handle, arg->fd, &ws_pkt );
    free(response);
    
    free( arg->message );
    free( arg );
}

static esp_err_t wsHandler(httpd_req_t *req) {

  // http_get to open a connection
  if (req->method == HTTP_GET) return ESP_OK;

  // get frame
  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = NULL;
  memset( &ws_pkt, 0, sizeof( httpd_ws_frame_t ) );
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  
  // any error?
  if (ret != ESP_OK)  {
    SWARM_LOG_ERROR("httpd_ws_recv_frame failed to get frame len with %d", ret);
    return ret;
  }

  // ignore messages without content
  if (!ws_pkt.len) return ESP_OK;

  // allocate a butter to get the message
  buf = (uint8_t *) calloc(1, ws_pkt.len + 1);
  if (buf == NULL) { SWARM_LOG_ERROR( "Failed to calloc memory for buf"); return ESP_ERR_NO_MEM; }

  // catch the message
  ws_pkt.payload = buf;
  ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
  if (ret != ESP_OK) { SWARM_LOG_ERROR( "httpd_ws_recv_frame failed with %d", ret); free(buf); return ret; }

  // accept text only
  if (ws_pkt.type != HTTPD_WS_TYPE_TEXT ) { free(buf); return ESP_ERR_NOT_SUPPORTED; }

  // send all stuff to asyncHandler
  struct async_resp_arg *arg = (async_resp_arg*) malloc( sizeof( struct async_resp_arg ) );
  arg->handle  = req->handle;
  arg->fd      = httpd_req_to_sockfd( req );
  arg->message = buf;
  return httpd_queue_work( req->handle, wsAsyncHandler, arg );

}

static esp_err_t apiGetLogHandler(httpd_req_t *req ) {
  char buffer[STDIO_BUFFER_SIZE];
  dumpStdIO(buffer, STDIO_BUFFER_SIZE);

  httpd_resp_set_type( req, "text/plain; charset=utf-8" );
  httpd_resp_set_status( req, HTTPD_200 );

  httpd_resp_sendstr_chunk( req, buffer);
  
  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;
}

bool SwOSStartWebServer( void ) {

  http_server_context_t *http_context = (http_server_context_t*)calloc(1, sizeof(http_server_context_t));

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = 10;
  config.stack_size = 100000;
  config.core_id = 0;

  esp_err_t x = httpd_start(&UIServer, &config);

  if ( x != ESP_OK) {
      SWARM_LOG_ERROR( "Failed to start web server! %04x", x);
      return false;
  }

  if ( FTSWARM_HAL_CAMS ) {
  
    config.server_port += 1;
    config.ctrl_port += 1;
    esp_err_t x = httpd_start(&streamServer, &config);
    if ( x != ESP_OK) {
      SWARM_LOG_ERROR( "Failed to start streaming server! %04x", x);
      return false;
    }

    // camera stream
    httpd_uri_t camGet = { .uri = "/stream", .method = HTTP_GET, .handler = &stream_handler, .user_ctx = NULL, .is_websocket = true, .handle_ws_control_frames = false, .supported_subprotocol = NULL };
    httpd_register_uri_handler(streamServer, &camGet);

  }

  // log
  httpd_uri_t getLog = { .uri = "/api/getLog", .method = HTTP_GET, .handler = &apiGetLogHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &getLog);

  #if FTSWARM_HAL_OLEDS > 0
  httpd_uri_t getScreenshot = { .uri = "/api/screenshot", .method = HTTP_GET, .handler = &screenshotUriHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &getScreenshot);
  #endif

  // ws
  httpd_uri_t ws = { .uri = "/ws", .method = HTTP_GET, .handler = &wsHandler, .user_ctx = NULL, .is_websocket  = true };
  httpd_register_uri_handler(UIServer, &ws);

  // everything else
  httpd_register_err_handler(UIServer, httpd_err_code_t::HTTPD_404_NOT_FOUND, &indexHandler);

  xTaskCreatePinnedToCore( wsTask, "wsTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

  return true;

}