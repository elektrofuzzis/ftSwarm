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

bool getParameter( cJSON * root, const char *parameter, int *value ) {

  cJSON *p = cJSON_GetObjectItem(root, parameter );

  if ( p != NULL ) {

    if ( p->type == cJSON_Number ) {
      *value = p->valueint;
    } else {
      ESP_LOGW( LOGFTSWARM, "%s has wrong type", parameter );
      return false;  
    }
    
    ESP_LOGD( LOGFTSWARM, "%s=%d", parameter, *value );
    return true;  
  
  } else {

    ESP_LOGW( LOGFTSWARM, "Missing parameter %s.", parameter );
    return false;
  
  }

}

bool getParameter( cJSON * root, const char *parameter, bool *value ) {

  int i;

  if ( getParameter( root, parameter, &i ) ) {
    *value = (i!=0);
    return true;
  }

  return false;

}

bool getParameter( cJSON * root, const char *parameter, char *value ) {

  cJSON *p = cJSON_GetObjectItem(root, parameter );

  if ( p != NULL ) {    
    strcpy( value, p->valuestring );
    ESP_LOGD( LOGFTSWARM, "%s=%s", parameter, value );
    return true;  
  
  } else {
    ESP_LOGW( LOGFTSWARM, "Missing parameter %s.", parameter );
    return false;
  
  }

}

static char *getBody(httpd_req_t *req)
{

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

cJSON *getJSON( httpd_req_t *req ) {
 
  ESP_LOGD( LOGFTSWARM, "getBody");

  char *body = getBody(req);

  if (body==NULL) {
    ESP_LOGD( LOGFTSWARM, "getBody failed");
    return NULL;
  }

  ESP_LOGD( LOGFTSWARM, "Parse");

  return cJSON_Parse(body);
}

esp_err_t indexHandler(httpd_req_t *req )
{
  char line[512];

  ESP_LOGD( LOGFTSWARM, "/index.html");

  httpd_resp_send_chunk(req, sfs_index_html, sfs_index_len);

  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;
}

esp_err_t stylesHandler(httpd_req_t *req )
{
    ESP_LOGD( LOGFTSWARM, "/static/styles.css");
  
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send_chunk(req, sfs_styles_css, sfs_styles_len);

    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
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

esp_err_t staticHandler(httpd_req_t *req )
{
  ESP_LOGD( LOGFTSWARM, "%s", req->uri);

  char *file = findlast( (char *) req->uri, '/' );
  set_content_type_from_file( req, file);

  uint32_t len;
  const char * x = sfs_get_file( file, &len);

  httpd_resp_send_chunk(req, x, len);
  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;

}

esp_err_t apiGetSwarm(httpd_req_t *req )
{

  JSONize json(req);

  myOSSwarm.lock();
  myOSSwarm.jsonize(&json);
  myOSSwarm.unlock();
  
  httpd_resp_sendstr_chunk(req, NULL);

  return ESP_OK;
}

esp_err_t apiGetHandler(httpd_req_t *req )
{

  // log request
  ESP_LOGD( LOGFTSWARM, "GET %s", req->uri );

  // check on sub urls, apiGetSwarm will lock
  if (strcmp( req->uri, "/api/getSwarm" ) == 0 ) { return apiGetSwarm( req ); }

  // unknown URL: FAIL
  return ESP_FAIL;
}

esp_err_t apiWifi(httpd_req_t *req )
{ // /api/wifi <apmode> <ssid> <pwd>

  cJSON *root = getJSON( req );
  if ( root == NULL ) { 
    ESP_LOGW( LOGFTSWARM, "invalid JSON string" );
    return ESP_FAIL;
  }
  
  esp_err_t xReturn = ESP_OK;

  myOSSwarm.lock();
  if (!getParameter( root, "apmode", &myOSSwarm.nvs.APMode ) ) xReturn = ESP_FAIL;
  if (!getParameter( root, "ssid", myOSSwarm.nvs.wifiSSID ) )  xReturn = ESP_FAIL;
  if (!getParameter( root, "pwd", myOSSwarm.nvs.wifiPwd ) )    xReturn = ESP_FAIL;
  myOSSwarm.unlock();

  cJSON_Delete(root);
  httpd_resp_sendstr(req, "Post control value successfully");

  return xReturn;

}

esp_err_t apiSave(httpd_req_t *req )
{ // /api/save

  myOSSwarm.lock();
  myOSSwarm.nvs.save();
  myOSSwarm.unlock();
  
  httpd_resp_sendstr(req, "Post control value successfully");

  return ESP_OK;

}

esp_err_t apiActor(httpd_req_t *req )
{

  cJSON *root = getJSON( req );
  if ( root == NULL ) { 
    ESP_LOGW( LOGFTSWARM, "invalid JSON string" );
    return ESP_FAIL;
  }

  char id[64];
  int  cmd = 0;
  int  power = 0;
  bool hasCmd   = false;
  bool hasPower = false;

  if (!getParameter( root, "id", id )) return ESP_FAIL;

  // optional parameters
  hasCmd   = (getParameter( root, "cmd", &cmd ));
  hasPower = (getParameter( root, "power", &power ));

  ESP_LOGD( LOGFTSWARM, "apiActor: hasCmd %d cmd %d hasPower %d power %d", hasCmd, cmd, hasPower, power );

  // but at least one
  if ( (!hasCmd) && (!hasPower) ) return ESP_FAIL;

  // let's send
  myOSSwarm.lock();
  if (hasCmd)   myOSSwarm.apiActorCmd(id, cmd);
  if (hasPower) myOSSwarm.apiActorPower(id, power);
  myOSSwarm.unlock();
  
  cJSON_Delete(root);
  httpd_resp_sendstr(req, "Post control value successfully");

  return ESP_OK;

}

esp_err_t apiLED(httpd_req_t *req )
{

  cJSON *root = getJSON( req );
  if ( root == NULL ) { 
    ESP_LOGW( LOGFTSWARM, "invalid JSON string" );
    return ESP_FAIL;
  }

  char id[64];
  int  brightness, color;

  if (!getParameter( root, "id", id )) return ESP_FAIL;
  if (!getParameter( root, "brightness", &brightness )) return ESP_FAIL;
  if (!getParameter( root, "color", &color )) return ESP_FAIL;

  myOSSwarm.lock();
  myOSSwarm.apiLED(id, brightness, color );
  myOSSwarm.unlock();
  
  cJSON_Delete(root);
  httpd_resp_sendstr(req, "Post control value successfully");

  return ESP_OK;

}

esp_err_t apiServo(httpd_req_t *req )
{

  cJSON *root = getJSON( req );
  if ( root == NULL ) { 
    ESP_LOGW( LOGFTSWARM, "invalid JSON string" );
    return ESP_FAIL;
  }

  char id[64];
  int  offset, position;

  if (!getParameter( root, "id", id )) return ESP_FAIL;
  if (!getParameter( root, "offset", &offset )) return ESP_FAIL;
  if (!getParameter( root, "position", &position )) return ESP_FAIL;

  myOSSwarm.lock();
  myOSSwarm.apiServo( id, offset, position );
  myOSSwarm.unlock();

  cJSON_Delete(root);
  httpd_resp_sendstr(req, "Post control value successfully");

  return ESP_OK;

}

esp_err_t apiPostHandler(httpd_req_t *req )
{

  // log request
  ESP_LOGD( LOGFTSWARM, "POST %s", req->uri );

  // check on sub url
  if      (strcmp( req->uri, "/api/led" )   == 0 ) return apiLED( req ); 
  else if (strcmp( req->uri, "/api/servo" ) == 0 ) return apiServo( req ); 
  else if (strcmp( req->uri, "/api/actor")  == 0 ) return apiActor( req ); 
  else if (strcmp( req->uri, "/api/wifi")   == 0 ) return apiWifi( req ); 
  else if (strcmp( req->uri, "/api/save" )  == 0 ) return apiSave( req ); 
  else return ESP_FAIL;

}


bool SwOSStartWebServer( void ) {

  http_server_context_t *http_context = (http_server_context_t*)calloc(1, sizeof(http_server_context_t));

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.max_uri_handlers = 10;
  config.stack_size = 50000;

  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config) != ESP_OK) {
      ESP_LOGE( LOGFTSWARM, "Failed to start web server!");
      return false;
  }

  // /
  httpd_uri_t index = { .uri = "/", .method = HTTP_GET, .handler = &indexHandler, .user_ctx = NULL };
  httpd_register_uri_handler(server, &index);

  // /api/* HTTP_GET
  httpd_uri_t apiGet = { .uri = "/api/*", .method = HTTP_GET, .handler = &apiGetHandler, .user_ctx = NULL };
  httpd_register_uri_handler(server, &apiGet);

  // /api/* HTTP_POST
  httpd_uri_t apiPost = { .uri = "/api/*", .method = HTTP_POST, .handler = &apiPostHandler, .user_ctx = http_context };
  httpd_register_uri_handler(server, &apiPost);

  // styles.css
  httpd_uri_t stylesGet = { .uri = "/styles.css", .method = HTTP_GET, .handler = &stylesHandler, .user_ctx = NULL };
  httpd_register_uri_handler(server, &stylesGet);
    
  // images
  httpd_uri_t staticGet = { .uri = "/static/*", .method = HTTP_GET, .handler = &staticHandler, .user_ctx = NULL };
  httpd_register_uri_handler(server, &staticGet);

  return true;

}
