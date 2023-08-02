/*
 * SwOSCLI.h
 *
 * ftSwarm Command Line Interface
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOS.h"
#include "SwOSSwarm.h"
#include "SwOSCLIParameter.h"
#include "SwOSHW.h"

#pragma once

#define CLIMAXLINE 255
#define MAXPARAM   20

typedef enum { 
  EVAL_INVALIDCMD = -2,
  EVAL_SYNTAXERROR = -1, 
  EVAL_OK, 
  EVAL_EOL, 
  EVAL_DOT, 
  EVAL_COMMA, 
  EVAL_LPARANTHESIS, 
  EVAL_RPARANTHESIS, 
  EVAL_NUMBER, 
  EVAL_LITERAL
} EvalResult_t;

typedef enum { 
  CMD_ERROR = -2, 
  CMD_UNKONWN = -1, 
  CMD_HELP, 
  CMD_SETUP, 
  CMD_HALT, 
  CMD_STARTCLI, 
  CMD_EXIT 
} Cmd_t;

typedef enum { 
  ERROR_OK = -1, 
  ERROR_SYNTAXERROR, 
  ERROR_DOTEXPECTED, 
  ERROR_LITERALEXPECTED, 
  ERROR_IOEXPECTED,
  ERROR_NUMBEREXPECTED,
  ERROR_LPARANTHESISEXPECTED,
  ERROR_RPARANTHESISEXPECTED,
  ERROR_UNKOWNCMD,
  ERROR_WRONGNUMBEROFARGUMENTS
} Error_t;

class SwOSCLI {
  protected:

    // user input    
    char _line[CLIMAXLINE];
    char prompt[2] = ">";

    char   *_evalPtr;
    char   *_start;
    SwOSIO *_io = NULL;
    IOCmd_t _cmd;
    int     _maxParameter;
    SwOSCLIParameter _parameter[MAXPARAM];

    void Error( Error_t error, int expected = 0, int found = 0 );

    // some simple char tests    
    bool isDigit(char ch);
    bool isAlpha(char ch);
    bool isLiteral(char ch);

    EvalResult_t getNumber( void );
    EvalResult_t getLiteral( void );
    EvalResult_t getNextToken( char* token );
    SwOSIO *getIO( char *token, char *IOName );

    bool tokenizeConstant( char *token, int param); 
    bool tokenizeCmd( char *cmd );
    Cmd_t evalSimpleCommand( char *token );  // tests, if token is a simple command
    void evalIOCommand( char *token );       // evals an IO command, token is already first token 

    void executeInputCmd( void );
    void executeActorCmd( void ); 
    void executeJoystickCmd( void );
    void executeServoCmd( void );
    void executePixelCmd( void );
    void executeI2CCmd( void );
    void executeIOCommand( void );
    bool eval( void );
    void help( void );
    void startCLI( bool noEcho );
    void halt( void );

  public:
    SwOSCLI();
    void run(void);
};