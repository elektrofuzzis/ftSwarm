/*
 * SwOSCLI.h
 *
 * ftSwarm Command Line Interface
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#pragma once

#include "SwOSSwarm.h"
#include "SwOSCLIParameter.h"
#include "SwOSHW.h"

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
  EVAL_LITERAL,
  EVAL_STRING
} EvalResult_t;

typedef enum { 
  CMD_ERROR = -2, 
  CMD_UNKONWN = -1, 
  CMD_HELP, 
  CMD_WHOAMI,
  CMD_UPTIME,
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
  ERROR_WRONGNUMBEROFARGUMENTS,
  ERROR_NOTAUTHENTICATED,
  ERROR_WRONGIOTYPE,
  ERROR_INVALIDCMD,
  ERROR_WRONGPIN,
  ERROR_STRINGEXPECTED,
  ERROR_ALIASNOTNUNIQUE,
  ERROR_PARAMETEREXPECTED,
  ERROR_SSIDEXPECTED,
  ERROR_PSKEXPECTED,
  ERROR_NOTIMPLEMENTEDYET
} Error_t;

class SwOSCLI {
  protected:

    // user input    
    char *in = NULL;

    // response
    char *response = NULL;

    // propmt
    char     prompt[3] = ">";

    // anything needed to analyze the string
    char     *evalPtr  = NULL;
    char     *start    = NULL;
    SwOSIO   *io       = NULL;
    SwOSCtrl *ctrl     = NULL;
    CLICmd_t cmd;
    int      maxParameter;
    SwOSCLIParameter parameter[MAXPARAM];

    // run mode
    bool interactive = false;
    bool exit        = false;

    // standard messages
    void Error( Error_t error, int expected = 0, int found = 0 );
    void OK( void );

    EvalResult_t getNumber( void );
    EvalResult_t getLiteral( void );
    EvalResult_t getString( void );
    EvalResult_t getNextToken( char* token );
    bool getIO( char *token, char *IOName, SwOSCtrl **ctrl, SwOSIO **io);

    bool tokenizeCmd( char *cmd );
    Cmd_t evalSimpleCommand( char *token );  // tests, if token is a simple command
    void evalComplexCommand( char *token, bool *loggedIn );  // evals an IO command, token is already first token 

    void executeInputCmd( void );
    void executeActorCmd( void ); 
    void executeJoystickCmd( void );
    void executeServoCmd( void );
    void executePixelCmd( void );
    void executeI2CCmd( void );
    void executeCANCmd( void );
    void executeIOCommand( void );
    void executeControllerCmd( void );
    void executeSwarmCmd( bool *loggedIn );
    void executeNVSCmd( bool *loggedIn );
    void startCLI( bool noEcho );
    void halt( void );

  public:
    char *eval( char* in, bool *loggedIn );
    void run( void );
};

extern void mainMenu( void );