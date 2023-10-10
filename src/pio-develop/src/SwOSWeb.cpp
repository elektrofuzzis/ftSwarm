#include "esp_err.h"
/*
 * SwOSWEB.h
 *
 * fTSwarm builtin WebServer
 * 
 * (C) 2021/22 Christian Bergschneider & Stefan Fuss
 * 
 */
 
#include <esp_http_server.h>
#include <esp_log.h>
#include <cJSON.h>
//#include "esp_vfs.h"

#include "jsonize.h"
#include "SwOSNVS.h"
#include "SwOSHW.h"
#include "SwOSSwarm.h"
#include "SwOSWeb.h"
#include "sfs_files.h"

#define SCRATCH_BUFSIZE (10240)
#define HTTPD_401 "401 Unauthorized"

typedef struct http_server_context {
    char base_path[16 + 1];
    char scratch[SCRATCH_BUFSIZE];
} http_server_context_t;

esp_err_t httpd_resp_sendstr_chunk_cr(httpd_req_t *req, const char *line ) {

  char cr[10];

  httpd_resp_sendstr_chunk( req, line );

  sprintf(cr, "\n");
  httpd_resp_sendstr_chunk( req, cr);

  return ESP_OK;
}

static char *getBody(httpd_req_t *req) {

  int total_len = req->content_len;
  int cur_len = 0;
  char *buf = ((http_server_context_t *)(req->user_ctx))->scratch;
  int received = 0;
  
  if (total_len >= SCRATCH_BUFSIZE) {
    // Respond with 500 Internal Server Error 
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
    return NULL;
  }
  
  while (cur_len < total_len) {
    
    received = httpd_req_recv(req, buf + cur_len, total_len);
    
    if (received <= 0) {
      // Respond with 500 Internal Server Error
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
      return NULL;
    }
    
    cur_len += received;
  }
  
  buf[total_len] = '\0';

  return buf;
  
}

esp_err_t sendResponse( httpd_req_t *req, uint16_t status, bool keepOpen = false ) {

  httpd_resp_set_type( req, "application/json; charset=utf-8" );
  
  switch (status) {
    case 200: httpd_resp_set_status( req, HTTPD_200 ); break;
    case 400: httpd_resp_set_status( req, HTTPD_400 ); break;
    case 401: httpd_resp_set_status( req, HTTPD_401 ); break;
    case 404: httpd_resp_set_status( req, HTTPD_404 ); break;
    default:  httpd_resp_set_status( req, HTTPD_500 ); break;
  }

  if (!keepOpen) {
    // reply noting
    httpd_resp_sendstr(req, "{}" ); 
    httpd_resp_sendstr(req, NULL );
  }
  
  return ESP_OK;
}

cJSON *getJSON( httpd_req_t *req ) {
  // test on valid JSON

  // get body
  char *body = getBody(req);
  if ( body == NULL ) {
    ESP_LOGE( LOGFTSWARM, "getJSON: getBody failed");
    httpd_resp_set_status( req, HTTPD_400 );
    return NULL;
  }

  // parse data
  cJSON *root = cJSON_Parse(body);

  // parsing error?
  if ( root == NULL ) { 
    ESP_LOGE( LOGFTSWARM, "invalid JSON string" );
    httpd_resp_set_status( req, HTTPD_400 );
    return NULL;
  }

  return root;

}

bool getParameter( httpd_req_t *req, cJSON * root, const char *parameter, int *value, bool mandatory = true ) {
  // get a number

  cJSON *p = cJSON_GetObjectItem(root, parameter );

  // check if everything is ok
  if ( p != NULL ) {

    if ( p->type == cJSON_Number ) {
      *value = p->valueint;
      return true;

    } else if ( p->type == cJSON_String ) {

      char *ptr = p->valuestring;

      // quoted string?
      if ( ( ptr[0] == '"' ) && ( ptr[strlen(ptr)] == '"' ) ) { ptr[strlen(ptr)] = '\0'; ptr++; }
      if ( ptr[0] == '\0' ) return false;

      // start with #?
      int base = 10;
      if ( ptr[0] == '#' ) { base = 16; ptr++; }

      *value = (int) strtol( ptr, NULL, base );
      return true;

    } else {

      ESP_LOGE( LOGFTSWARM, "%s has wrong type", parameter );
      httpd_resp_set_status( req, HTTPD_400 );
      return false;
    }
  }

  // mandatory
  if ( mandatory ){
    ESP_LOGE( LOGFTSWARM, "Missing parameter %s.", parameter );
    httpd_resp_set_status( req, HTTPD_400 );
    return false;
  }
  
  // optional
  return false;

}

bool getParameter( httpd_req_t *req, cJSON * root, const char *parameter, uint16_t *value, bool mandatory = true ) {
  // get uint16_t
  int v;
  
  if ( getParameter( req, root, parameter, &v, mandatory ) ) {
    *value = (uint16_t) v;
    return true;
  }

  return false;
}


bool getParameter( httpd_req_t *req, cJSON * root, const char *parameter, bool *value, bool mandatory = true ) {
  // get a boolean
  
  int i;

  if ( getParameter( req, root, parameter, &i, mandatory ) ) {
    *value = (i!=0);
    return true;
  }

  return false;

}

bool getParameter( httpd_req_t *req, cJSON * root, const char *parameter, char *value, bool mandatory = true ) {
  // get a string

  cJSON *p = cJSON_GetObjectItem( root, parameter );

  // check if everything is ok
  if ( ( p != NULL ) && ( p->type == cJSON_String ) ) {
    strcpy( value, p->valuestring );
    return true;
  }

  // missing mandatory parameter?
  if ( ( p == NULL ) && ( mandatory ) ) {
    ESP_LOGW( LOGFTSWARM, "Missing parameter %s.", parameter );
    httpd_resp_set_status( req, HTTPD_400 );

  // wrong type?
  } else if ( p != NULL )  {
    ESP_LOGW( LOGFTSWARM, "%s has wrong type", parameter );
    httpd_resp_set_status( req, HTTPD_400 );
  }
  
  return false;    

}


#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{

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

char * findlast( char *str, char ch) {

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

esp_err_t apiGetSwarm(httpd_req_t *req ) {
  // reply on /api/getSwarm

  uint16_t token;
  bool provided = getAuthorization( req, &token);  
  bool status = false;

  sendResponse( req, 200, true );

  JSONize json(req);

  json.startObject();
  
  json.startObject("auth");
  
  myOSSwarm.lock();

  if (provided) {
    status = myOSSwarm.apiPeekIsAuthorized(token);    
  }

  json.variableB("provided", provided);
  json.variableB("status", status);

  json.endObject();
  json.newObject(JSONObject);
  json.text2string((char *)"swarms");
  json.assign();

  myOSSwarm.jsonize(&json);
  myOSSwarm.unlock();

  json.endObject();

  httpd_resp_sendstr_chunk(req, NULL);
  
  return ESP_OK;
}

esp_err_t apiGetToken(httpd_req_t *req ) {
  // reply on /api/getToken

  sendResponse( req, 200, true );

  JSONize json(req);

  myOSSwarm.lock();
  myOSSwarm.getToken(&json);
  myOSSwarm.unlock();
  
  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;
}

esp_err_t apiGetHandler(httpd_req_t *req ) {
  // reply on get /api/*

  // check on sub urls, apiGetSwarm will lock
  if (strcmp( req->uri, "/api/getSwarm" ) == 0 ) { return apiGetSwarm( req ); }
  if (strcmp( req->uri, "/api/getToken" ) == 0 ) { return apiGetToken( req ); }

  // unknown URL: FAIL
  return sendResponse( req, 404, NULL );
}


esp_err_t apiActor( httpd_req_t *req ) {
  // reply on /api/actor

  // check on valid json 
  cJSON *root = getJSON( req ); if ( root == NULL ) return sendResponse( req, 400 );
 
  char id[64];
  int  cmd = 0;
  int  power = 0;
  uint16_t token;
  uint16_t status = 400;

  // mandatory parameters
  bool hasID = getParameter( req, root, "id", id, true );
  bool hasAuthorization = getAuthorization( req, &token );

  if ( !( hasID && hasAuthorization ) ) {
    cJSON_Delete( root );
    return sendResponse( req, 400 );
  }
  
  // optional parameters
  boolean hasCmd = ( req, getParameter( req, root, "cmd", &cmd, false ) );
  boolean hasPower = ( req, getParameter( req, root, "power", &power, false ) );

  // cleanup
  cJSON_Delete( root );

  // let's do it
  myOSSwarm.lock();  
  if ( hasCmd )   status = myOSSwarm.apiActorCmd( token, id, cmd, !hasPower );
  if ( hasPower ) status = myOSSwarm.apiActorPower( token, id, power, true); 
  myOSSwarm.unlock();

  return sendResponse( req, status );

}

esp_err_t apiLED( httpd_req_t *req ) {
  // reply on /api/led

  // check on valid json 
  cJSON *root = getJSON( req ); if ( root == NULL ) return sendResponse( req, 400 );

  char     id[64];
  int      brightness, color;
  uint16_t status = 400;
  uint16_t token;

  // mandatory parameters
  bool hasID = getParameter( req, root, "id", id, true );
  bool hasAuthorization = getAuthorization( req, &token );
  
  if ( !( hasID && hasAuthorization ) ) {
    cJSON_Delete( root );
    return sendResponse( req, 400 );
  }

  // one of both?
  bool hasBrightness = getParameter( req, root, "brightness", &brightness, false );
  bool hasColor      = getParameter( req, root, "color", &color, false );

  // cleanup
  cJSON_Delete(root);

  // let's do it
  myOSSwarm.lock();
  if ( hasBrightness ) status = myOSSwarm.apiLEDBrightness( token, id, brightness, !hasColor );
  if ( hasColor )      status = myOSSwarm.apiLEDColor( token, id, color, true );
  myOSSwarm.unlock();
  
  return sendResponse( req, status );

}

esp_err_t apiServo(httpd_req_t *req ) {
  // reply on /api/servo

  // check on valid json 
  cJSON *root = getJSON( req ); if ( root == NULL ) return sendResponse( req, 400 );

  char     id[64];
  int      offset, position;
  uint16_t token;
  uint16_t status = 400;

  // mandatory parameters
  bool hasID = getParameter( req, root, "id", id, true );
  bool hasAuthorization = getAuthorization( req, &token );

  if ( !( hasID && hasAuthorization ) ) {
    cJSON_Delete( root );
    return sendResponse( req, 400 );
  }

  // optional
  boolean hasOffset   = getParameter( req, root, "offset", &offset, false);
  boolean hasPosition = getParameter( req, root, "position", &position, false);

  // cleanup
  cJSON_Delete(root);
  
  // let's do it
  myOSSwarm.lock();
  if (hasOffset)   status = myOSSwarm.apiServoOffset( token, id, offset, !hasPosition);
  if (hasPosition) status = myOSSwarm.apiServoPosition( token, id, position, true );
  myOSSwarm.unlock();

  return sendResponse( req, status );

}

esp_err_t apiCam(httpd_req_t *req ) {
  // reply on /api/cam

  // check on valid json 
  cJSON *root = getJSON( req ); if ( root == NULL ) return sendResponse( req, 400 );

  char     id[64];
  uint16_t token;
  int      value;
  uint16_t status = 400;

  // mandatory parameters
  bool hasID = getParameter( req, root, "id", id, true );
  bool hasAuthorization = getAuthorization( req, &token );

  if ( !( hasID && hasAuthorization ) ) {
    cJSON_Delete( root );
    return sendResponse( req, 400 );
  }

  // let's do it
  myOSSwarm.lock();
  if ( getParameter( req, root, "streaming", &value, false) )      status = myOSSwarm.apiCAMStreaming( token, id, value>0, false );
  if ( getParameter( req, root, "framesize", &value, false) )      status = myOSSwarm.apiCAMFramesize( token, id, value, false );
  if ( getParameter( req, root, "quality", &value, false) )        status = myOSSwarm.apiCAMQuality( token, id, value, false );
  if ( getParameter( req, root, "brightness", &value, false) )     status = myOSSwarm.apiCAMBrightness( token, id, value, false );
  if ( getParameter( req, root, "contrast", &value, false) )       status = myOSSwarm.apiCAMContrast( token, id, value, false );
  if ( getParameter( req, root, "saturation", &value, false) )     status = myOSSwarm.apiCAMSaturation( token, id, value, false );
  if ( getParameter( req, root, "specialEffect", &value, false) )  status = myOSSwarm.apiCAMSpecialEffect( token, id, value, false );
  if ( getParameter( req, root, "wbMode", &value, false) )         status = myOSSwarm.apiCAMWbMode( token, id, value, false );
  if ( getParameter( req, root, "hMirror", &value, false) )        status = myOSSwarm.apiCAMHMirror( token, id, value, false );
  if ( getParameter( req, root, "vFlip", &value, false) )          status = myOSSwarm.apiCAMVFlip( token, id, value, false );
  myOSSwarm.unlock(); 
  
  // cleanup
  cJSON_Delete(root);
  
  return sendResponse( req, status );

}

esp_err_t apiIsAuthorized( httpd_req_t *req ) {
  // reply on api/isAuthorized

  // check on valid json 
  cJSON *root = getJSON( req ); if ( root == NULL ) return sendResponse( req, 400 );

  // get Token
  uint16_t token;
  bool hasAuthorization = getAuthorization( req, &token );

  // cleanup
  cJSON_Delete(root);

  if (!hasAuthorization ) return sendResponse( req, 400 );

  // cleanup
  cJSON_Delete(root);

  myOSSwarm.lock();
  uint16_t status = myOSSwarm.apiIsAuthorized( token, true );
  myOSSwarm.unlock();

  return sendResponse( req, status );

}

esp_err_t apiPostHandler(httpd_req_t *req ) {
  // reply on post /api

  // check on sub url
  if (strcmp( req->uri, "/api/led" )   == 0 )          return apiLED( req ); 
  if (strcmp( req->uri, "/api/servo" ) == 0 )          return apiServo( req ); 
  if (strcmp( req->uri, "/api/actor")  == 0 )          return apiActor( req ); 
  if (strcmp( req->uri, "/api/cam")  == 0 )            return apiCam( req ); 
  if (strcmp( req->uri, "/api/isAuthenticated") == 0 ) return apiIsAuthorized( req ); 
 
  // unknown URL: FAIL
  return sendResponse( req, 404 );

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
        if (res != ESP_OK ) { printf("Error 1 %d\n", res ); }
      }

      
      if (res == ESP_OK) {
        size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res != ESP_OK ) { printf("Error 2 %d\n", res ); }
      }
        
      if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        if (res != ESP_OK ) { printf("Error 3 %d\n", res ); }
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

bool SwOSStartWebServer( void ) {

  http_server_context_t *http_context = (http_server_context_t*)calloc(1, sizeof(http_server_context_t));

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = 10;
  config.stack_size = 100000;
  config.core_id = 0;

  httpd_handle_t UIServer = NULL;
  httpd_handle_t streamServer = NULL;

  esp_err_t x = httpd_start(&UIServer, &config);

  if ( x != ESP_OK) {
      ESP_LOGE( LOGFTSWARM, "Failed to start web server! %04x", x);
      return false;
  }

  if ( myOSSwarm.Ctrl[0]->getType() == FTSWARMCAM ) {
  
    config.server_port += 1;
    config.ctrl_port += 1;
    esp_err_t x = httpd_start(&streamServer, &config);
    if ( x != ESP_OK) {
      ESP_LOGE( LOGFTSWARM, "Failed to start streaming server! %04x", x);
      return false;
    }

    // camera stream
    httpd_uri_t camGet = { .uri = "/stream", .method = HTTP_GET, .handler = &stream_handler, .user_ctx = NULL, .is_websocket = true, .handle_ws_control_frames = false, .supported_subprotocol = NULL };
    httpd_register_uri_handler(streamServer, &camGet);

  }

  // /
  httpd_uri_t index = { .uri = "/", .method = HTTP_GET, .handler = &indexHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &index);

  // /api/* HTTP_GET
  httpd_uri_t apiGet = { .uri = "/api/*", .method = HTTP_GET, .handler = &apiGetHandler, .user_ctx = http_context };
  httpd_register_uri_handler(UIServer, &apiGet);

  // /api/* HTTP_POST
  httpd_uri_t apiPost = { .uri = "/api/*", .method = HTTP_POST, .handler = &apiPostHandler, .user_ctx = http_context };
  httpd_register_uri_handler(UIServer, &apiPost);
    
  // css
  httpd_uri_t cssGet = { .uri = "/css/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &cssGet);

  // js
  httpd_uri_t jsGet = { .uri = "/js/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &jsGet);

  // assets
  httpd_uri_t assetsGet = { .uri = "/assets/*", .method = HTTP_GET, .handler = &fileHandler, .user_ctx = NULL };
  httpd_register_uri_handler(UIServer, &assetsGet);

  return true;

}
