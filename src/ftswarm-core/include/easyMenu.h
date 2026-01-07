/*
 * easyMenu.h
 *
 * simple menu system
 * 
 * (C) 2026 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once
#define MAXMENUITEMS 99

class Menu {
  private:
    uint8_t  maxMenuItems = 0;
    uint8_t  spacer = 0;
    int8_t   *id = NULL;
    int8_t   maxOption = -1;
    int8_t   maxNumber = 0;
    char     **option = NULL;
    char     delimiter = ' ';
    char     *header = NULL;

  protected:
    char *prompt = NULL;
    void resetPrompt();
    void resetOption();

  private:
    int isValid( char *str );

  public:
    
    Menu( const char *basePrompt, const char *newPrompt, const char *Header, uint8_t maxMenuItems, uint8_t spacer=0, char delimiter = ':' );
    Menu( uint8_t maxMenuItems ):Menu( NULL, NULL, NULL, maxMenuItems ){};
    
    ~Menu();
  
    void   begin( const char *basePrompt, const char *newPrompt, const char *Header, uint8_t spacer, char delimiter = ':' );
    void   start( void );
    bool   add( const char *item, const char *value, int8_t id, char key = '\0', bool staticDelimiter = false );
    bool   add( const char *item, int value, int8_t id, char key = '\0' );
    bool   add( int8_t id, char key = '\0' );
    bool   add( const char *value, int8_t id, char key = '\0' );
    bool   addF( const char *item, float value, int8_t id, char key = '\0' );
    bool   addExit( void );
    int8_t userChoice( void );
};
