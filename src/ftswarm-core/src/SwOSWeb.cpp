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

#include "serialize.h"
#include "SwOSNVS.h"
#include "SwOSHW.h"
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

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)  {

  const char *type = "text/plain";
  if (CHECK_FILE_EXTENSION(filepath, ".html")) {
      type = "text/html";
  } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
      type = "application/javascript";
  } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
      type = "text/css";
  } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
      type = "image/png";
  } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
      type = "image/x-icon";
  } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
      type = "image/svg+xml";
  }
    
  return httpd_resp_set_type(req, type);

}

char *findlast( char *str, char ch) {

  char *result = str;
  char *test   = result;

  while ( *test != '\0' ) {
    if ( *test == ch ) result = test;
      test++;
    }

  if ( *result == ch )  result++;

  return result;

}

bool getAuthorization( httpd_req_t *req, uint16_t *token ) {
  // needs to be called before start building a request's response

  bool result = false;
  char *authBuffer;

  // get the value of Authorisation, expected value is "Bearer <number>"
  size_t len = httpd_req_get_hdr_value_len(req, "Authorization" )+1;
  authBuffer = (char *) malloc( len );
  result = httpd_req_get_hdr_value_str(req, "Authorization", authBuffer, len) == ESP_OK;

  // Authorisation found, check on token
  if ( result && ( strlen( authBuffer ) > 7 ) ) {
    char *tokenStr = authBuffer + 7; // sizeof("Bearer ") == 7
    *token = (uint16_t) atol(tokenStr);
  }

  // cleanup
  free(authBuffer);

  return result;
  
}

esp_err_t indexHandler(httpd_req_t *req ) {
  // reply on /index.html

  httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
  httpd_resp_set_type( req, "text/html" ); 

  httpd_resp_send_chunk(req, sfs_index_html, SFS_index_html_len);

  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;
}

esp_err_t fileHandler(httpd_req_t *req ) {
  // reply on /assets/* or /js/* or /css/*
  
  char *file = findlast( (char *) req->uri, '/' );
  set_content_type_from_file( req, file);
  httpd_resp_set_hdr( req, "Content-Encoding", "gzip" );
  
  uint32_t len;
  const char * x = sfs_get_file( (char *) req->uri, &len);

  // file found?
  if ( x[0] != '\0' ) {
    httpd_resp_send_chunk(req, x, len);
  } else {
    httpd_resp_set_status( req, HTTPD_404 );
  }
  
  httpd_resp_sendstr_chunk(req, NULL);
  return ESP_OK;

}

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
  // reply on /api/getToken

  char buffer[STDIO_BUFFER_SIZE];
  dumpStdIO(buffer, STDIO_BUFFER_SIZE);

  httpd_resp_set_type( req, "application/json; charset=utf-8" );
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

  if ( FTSWARM_HAL_HAS_CAM ) {
  
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

  // /
  httpd_uri_t index = { .uri = "/", .method = HTTP_GET, .handler = &indexHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &index);
    
  // css
  httpd_uri_t cssGet = { .uri = "/css/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &cssGet);

  // js
  httpd_uri_t jsGet = { .uri = "/js/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &jsGet);

  // assets
  httpd_uri_t assetsGet = { .uri = "/assets/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &assetsGet);

  // log
  httpd_uri_t getLog = { .uri = "/api/getLog", .method = HTTP_GET, .handler = &apiGetLogHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &getLog);

  // ws
  httpd_uri_t ws = { .uri = "/ws", .method = HTTP_GET, .handler = &wsHandler, .user_ctx = NULL, .is_websocket  = true };
  httpd_register_uri_handler(UIServer, &ws);

  xTaskCreatePinnedToCore( wsTask, "wsTask", 10000, NULL, 1, NULL, ARDUINO_EVENT_RUNNING_CORE );

  return true;

}