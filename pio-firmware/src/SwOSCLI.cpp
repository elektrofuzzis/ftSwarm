/*
 * SwOSCLI.cpp
 *
 * ftSwarm Command Line Interface
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include "SwOSCLI.h"
#include "easyKey.h"
#include "SwOS.h"
#include "SwOSSwarm.h"
#include "SwOSFirmware.h"
#include "SwOSCLIParameter.h"

typedef struct {
  char cmd[20];
  int minParams, maxParams;
} IOCmdList_t;

const IOCmdList_t IOCmdList [IOCMD_MAX] = {
  { "subscribe", 1, 1 },
  { "getIOType", 0, 0},
  { "getSensorType", 0, 0},
  { "setSensorType", 2, 2},
  { "getValue", 0, 0},
  { "getVoltage", 0, 0},
  { "getResistance", 0, 0},
  { "getKelvin", 0, 0},
  { "getCelcius", 0, 0},
  { "getFahrenheit", 0, 0},
  { "getToggle", 0, 0},
  { "setActorType", 1, 1},  
  { "getActorType", 0, 0},
  { "setSpeed", 1, 1},
  { "getSpeed", 0, 0},
  { "setMotionType", 0, 0},
  { "getMotionType", 0, 0},
  { "onTrigger", 2, 3},
  { "onTriggerLR", 2, 3},
  { "onTriggerFB", 2, 3},
  { "setPosition", 1, 1},
  { "getPosition", 0, 0},
  { "setOffset", 1, 1},
  { "getOffset", 0, 0},
  { "setColor", 1, 1},
  { "getColor", 0, 0},
  { "setBrightness", 1, 1},
  { "getBrightness", 0, 0},
  { "setRegister", 2, 2},
  { "getRegister", 1, 1}
};

typedef struct {
  char constant[30];
  int  value;
} constantList_t;

const constantList_t constantList[12] = {
  { "FTSWARM_DIGITAL", FTSWARM_DIGITAL }, 
  { "FTSWARM_ANALOG", FTSWARM_ANALOG }, 
  { "FTSWARM_SWITCH", FTSWARM_SWITCH }, 
  { "FTSWARM_REEDSWITCH", FTSWARM_REEDSWITCH }, 
  { "FTSWARM_LIGHTBARRIER", FTSWARM_LIGHTBARRIER }, 
  { "FTSWARM_VOLTMETER", FTSWARM_VOLTMETER }, 
  { "FTSWARM_OHMMETER", FTSWARM_OHMMETER }, 
  { "FTSWARM_THERMOMETER", FTSWARM_THERMOMETER }, 
  { "FTSWARM_LDR", FTSWARM_LDR }, 
  { "FTSWARM_TRAILSENSOR", FTSWARM_TRAILSENSOR }, 
  { "FTSWARM_COLORSENSOR", FTSWARM_COLORSENSOR  }, 
  { "FTSWARM_ULTRASONIC", FTSWARM_ULTRASONIC }
};

SwOSCLI::SwOSCLI() {
  _line[0] = '\0';
  _evalPtr = NULL;
  _start   = NULL;
}

void SwOSCLI::Error( Error_t error, int expected, int found ) {

  int pos = 0;

  // calc error postion
  if ( (_start) && (_line) && ( _start > _line ) ) pos = _start - _line; 

  // print marker
  for (int i=0; i<pos; i++) printf(" ");
  printf( "^ Error: " );

  // print error
  switch ( error ) {
    case ERROR_SYNTAXERROR:             printf("Syntax error.\n"); break;
    case ERROR_DOTEXPECTED:             printf("\".\" expected.\n"); break;  
    case ERROR_LITERALEXPECTED:         printf("Literal expected.\n"); break; 
    case ERROR_IOEXPECTED:              printf("Not a valid IO port or IO port is offline.\n"); break;
    case ERROR_NUMBEREXPECTED:          printf("Number expected.\n"); break;
    case ERROR_LPARANTHESISEXPECTED:    printf("\"(\" expected.\n"); break;
    case ERROR_RPARANTHESISEXPECTED:    printf("\")\" expected.\n"); break;
    case ERROR_UNKOWNCMD:               printf("unkown command\n"); break;
    case ERROR_WRONGNUMBEROFARGUMENTS:  printf("%d parameters expected, %d found.\n", expected, found ); break;
    default:                            printf("Syntax error.\n"); break;
  }
  
}

bool SwOSCLI::isDigit( char ch) {
  return ( (ch >= '0') &&  (ch <= '9' ) );
}

bool SwOSCLI::isAlpha( char ch ) {
  return ( (ch >= 'A') && (ch <= 'Z' ) ) ||
         ( (ch >= 'a') && (ch <= 'z' ) ) ||
         (ch == '_');
}

bool SwOSCLI::isLiteral( char ch) {
  return (isDigit(ch) ) || isAlpha(ch);
}

EvalResult_t SwOSCLI::getNumber( void ) {
  // _evalPtr is already on the first char

  // if the number starts with + or -, the next char needs to be a digit
  if ( ( (*_evalPtr=='+') || (*_evalPtr=='-') ) && 
       ( !isDigit(_evalPtr[1]) ) )
    return EVAL_SYNTAXERROR;

  // now loop until end of digits
  while (isDigit(*++_evalPtr));

  return EVAL_NUMBER;

}

EvalResult_t SwOSCLI::getLiteral( void ) {
  // _evalPtr is already on the first char

  // now loop until end of literals
  while (isLiteral(*_evalPtr)) _evalPtr++;

  return EVAL_LITERAL;

}

EvalResult_t SwOSCLI::getNextToken( char *token ) {

  // kill white spaces
  while (*_evalPtr==' ') _evalPtr++;

  // remember where we started
  _start = _evalPtr;  

  // different behaviour based on first char
  EvalResult_t eval = EVAL_OK;
  switch (*_evalPtr) {
    case '\0':        eval = EVAL_EOL; break; // no _evalPtr++, so we can't skip EOL
    
    case '.':         eval = EVAL_DOT; _evalPtr++; break;
    
    case ',':         eval = EVAL_COMMA; _evalPtr++; break;

    case '(':         eval = EVAL_LPARANTHESIS; _evalPtr++; break;
    
    case ')':         eval = EVAL_RPARANTHESIS; _evalPtr++; break;
    
    case '+':
    case '-':  
    case '0' ... '9': eval = getNumber( ); break;
    
    case 'a' ... 'z':
    case 'A' ... 'Z': eval = getLiteral( ); break;
    
    default:          eval = EVAL_SYNTAXERROR; break;

  }

   // valid data? copy token or clear it
  if ( eval >= EVAL_OK ) {
    strncpy( token, _start, (_evalPtr - _start) );
    token[_evalPtr - _start]='\0';
    
  } else {
    token[0] = '\0';
  }

  // return evaluation result
  return eval;
}

void SwOSCLI::startCLI( bool noEcho ) {

  printf("@@@ ftSwarmOS CLI started\n");
  
  if ( noEcho ) {
    keyboardEcho( false );
    prompt[0] = '\0';
  }

  myOSSwarm.lock();
  myOSSwarm.unsubscribe();
  myOSSwarm.unlock();
  
}

void SwOSCLI::halt( void ) {

  myOSSwarm.halt();

}

void SwOSCLI::help( void ) {

    printf("help - list all commands\n");
    printf("setup - start setup mode\n");
    printf("halt - stop all motors\n");
    printf("exit - end command line interface.\n\n");

    printf("<Alias-Name>.<Command>(<parameter>, ...) or\n");
    printf("<Hostname>.<IO-Name>.<Command>(<parameter>, ...)\n\n");

    printf("Input commands (A1..A6):\n");
    printf("  subscribe( hysteresis )\n" );
    printf("  getIOType()\n");
    printf("  setSensorType( sensorType, normallyOpen )\n");
    printf("  getSensorType()\n");
    printf("  getValue()\n");
    printf("  getVoltage()\n");
    printf("  getResistance()\n");
    printf("  getKelvin()\n");
    printf("  getCelcius()\n");
    printf("  getFahrenheit()\n");
    printf("  getToggle()\n");
    printf("  onTrigger( triggerEvent, actor, p1)\n");
    printf("  onTrigger( triggerEvent, actor)\n\n");
    
    printf("Joystick commands (JOY1..JOY2):\n");
    printf("  subscribe( int hysteresis )\n" );
    printf("  getValue()\n");
    printf("  onTriggerLR( triggerEvent, actor, p1)\n");
    printf("  onTriggerLR( triggerEvent, actor)\n");
    printf("  onTriggerFB( triggerEvent, actor, p1)\n");
    printf("  onTriggerFB( triggerEvent, actor)\n\n");

    printf("Actor commands (M1..M2):\n");
    printf("  getActorType()\n");
    printf("  setActorType( actorType )\n");
    printf("  setSpeed( speed )\n");
    printf("  getSpeed()\n");
    printf("  setMotionType( motionType )\n");
    printf("  getMotionType()\n\n");

    printf("Servo commands (SERVO1..SERVO2):\n");
    printf("  setPosition( position )\n");
    printf("  getPosition()\n");
    printf("  setOffset( position )\n");
    printf("  getOffset()\n\n");

    printf("ftPixel commands (LED1..LED18):\n");
    printf("  setColor( color )\n");
    printf("  getColor()\n");
    printf("  setBrightness( brightness )\n");
    printf("  getBrightness()\n\n");    

    printf("I2C commands:\n");
    printf("  setRegister( register, value )\n");
    printf("  getRegister( register )\n");
    printf("  onTrigger( triggerEvent, actor, p1)\n\n");
}

Cmd_t SwOSCLI::evalSimpleCommand( char *token ) {

  Cmd_t cmd = CMD_UNKONWN;

   // did I found a command?
  if      ( strcmp( token, "help"  ) == 0 ) cmd = CMD_HELP;
  else if ( strcmp( token, "setup" ) == 0 ) cmd = CMD_SETUP;
  else if ( strcmp( token, "startCLI" ) == 0 ) cmd = CMD_STARTCLI;
  else if ( strcmp( token, "halt" ) == 0 ) cmd = CMD_HALT;
  else if ( strcmp( token, "exit" ) == 0 )  cmd = CMD_EXIT;
  
  // command found?
  if ( cmd >= 0 ) {

    // all commands don't have parameters
    if ( getNextToken( token ) != EVAL_EOL ) {
      Error( ERROR_SYNTAXERROR );
      return CMD_ERROR;
    }

    // execute 
    switch (cmd) {
      case CMD_HELP:      help();           break;
      case CMD_SETUP:     mainMenu();       break;
      case CMD_STARTCLI:  startCLI( true ); break;
      case CMD_HALT:      halt();           break;
    }

  }

  return cmd;

}

bool SwOSCLI::eval( void ) {
  // returns false on command exit

  char token[CLIMAXLINE];
  
  // rest variables to run the evaluation
  _evalPtr = _line;

  // every line must start with a literal
  switch ( getNextToken( token ) ) {
    case EVAL_LITERAL: break;         // everything ok
    case EVAL_EOL:     return true;   // just an emoty line, ignore
    default:           Error( ERROR_LITERALEXPECTED ); return true;
  }

  // interpret as a simple command
  switch ( evalSimpleCommand( token ) ) {
    case CMD_EXIT:    return false;   // exit: stop CLI
    case CMD_UNKONWN: break;          // not a command: continue
    default:          return true;    // error, setup, help, startcli, halt: stop evaluation
  }

  // if it's not a command, it should be a host or an io.
  evalIOCommand( token );
 
  return true;

}

void SwOSCLI::executeInputCmd( void ) {

  SwOSInput *io = (SwOSInput *)_io;

  switch ( _cmd ) {
    case IOCMD_getSensorType: myOSSwarm.lock();
                              printf("R: %d\n", io->getSensorType() ); 
                              myOSSwarm.unlock();
                              break;

    case IOCMD_setSensorType: if ( ( _parameter[0].inRange( "sensorType", 0, FTSWARM_MAXSENSOR-1 ) ) && 
                                   ( _parameter[1].inRange( "normallyOpen", 0, 1 ) ) ) {
                                printf("R: ok\n");
                                myOSSwarm.lock();
                                io->setSensorType( (FtSwarmSensor_t)_parameter[0].getValue(), (bool)_parameter[1].getValue(), false );
                                myOSSwarm.unlock();
                              }
                              break;

    case IOCMD_getValue:      myOSSwarm.lock();
                              printf("R: %d\n", io->getValueUI32() );
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getVoltage:    myOSSwarm.lock();
                              printf("R: %f\n", io->getVoltage());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getResistance: myOSSwarm.lock();
                              printf("R: %f\n", io->getResistance());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getKelvin:     myOSSwarm.lock();
                              printf("R: %f\n", io->getKelvin());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getCelcius:    myOSSwarm.lock();
                              printf("R: %f\n", io->getCelcius());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getFahrenheit: myOSSwarm.lock();
                              printf("R: %f\n", io->getFahrenheit());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_getToggle:     myOSSwarm.lock();
                              printf("R: %d\n", io->getToggle());
                              myOSSwarm.unlock();
                              break;

    case IOCMD_onTrigger:     if (( _parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER ) ) &&
                                  ( _parameter[1].isIO() ) &&
                                  ( _parameter[2].isConstant() ) ) {
                                printf("R: ok\n");
                                myOSSwarm.lock();
                                if ( _maxParameter == 2 ) {
                                  // work with a fixed value
                                  io->registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), false, _parameter[2].getValue() );
                                } else {
                                  // work with input's value
                                  io->registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), true, 0 );
                                }

                                myOSSwarm.unlock();
                              }
                              break;

    default:                  myOSSwarm.lock();
                              printf("Error: invalid command.\n");
                              myOSSwarm.unlock();
                              break;
  }
}

void SwOSCLI::executeActorCmd( void ) {

  SwOSActor *io = (SwOSActor *)_io;
  
  switch ( _cmd ) {
    case IOCMD_setActorType:    if (_parameter[0].inRange( "actorType", 0, (int)FTSWARM_MAXACTOR-1 ) ) { 
                                    printf("R: ok\n"); 
                                    myOSSwarm.lock();
                                    io->setActorType( (FtSwarmActor_t) _parameter[0].getValue(), false );
                                    myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getActorType:    myOSSwarm.lock();
                                printf("R: %d\n", (int) io->getActorType() ); 
                                myOSSwarm.unlock();
                                break;

    case IOCMD_setSpeed:        if (_parameter[0].inRange( "speed", -255, 255 ) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setPower( _parameter[0].getValue() );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getSpeed:        myOSSwarm.lock();
                                printf("R: %d\n", io->getPower() ); 
                                myOSSwarm.unlock();
                                break;

    case IOCMD_setMotionType:   if (_parameter[0].inRange( "motionType", 0, FTSWARM_MAXMOTION-1) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setMotionType( (FtSwarmMotion_t) _parameter[0].getValue() );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getMotionType:   myOSSwarm.lock();
                                printf("R: %d\n", (int) io->getMotionType() ); 
                                myOSSwarm.unlock();
                                break;

    default:                    printf("Error: invalid command.\n");
                                break;
  }

}

void SwOSCLI::executeJoystickCmd( void ) {

  SwOSJoystick *io = (SwOSJoystick *)_io;

  switch ( _cmd ) {

    case IOCMD_getValue:      myOSSwarm.lock();
                              int16_t lr, fb;
                              io->getValue( &lr, &fb );
                              myOSSwarm.unlock();
                              break;

    case IOCMD_onTriggerLR:   if (( _parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER ) ) &&
                                  ( _parameter[1].isIO() ) &&
                                  ( _parameter[2].isConstant() ) ) {
                                printf("R: ok\n");
                                myOSSwarm.lock();
                                if ( _maxParameter == 2 ) {
                                  // work with a fixed value
                                  io->triggerLR.registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), false, _parameter[2].getValue() );
                                } else {
                                  // work with input's value
                                  io->triggerLR.registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), true, 0 );
                                }

                                myOSSwarm.unlock();
                              }
                              break;

    case IOCMD_onTriggerFB:   if (( _parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER ) ) &&
                                  ( _parameter[1].isIO() ) &&
                                  ( _parameter[2].isConstant() ) ) {
                                printf("R: ok\n");
                                myOSSwarm.lock();
                                if ( _maxParameter == 2 ) {
                                  // work with a fixed value
                                  io->triggerFB.registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), false, _parameter[2].getValue() );
                                } else {
                                  // work with input's value
                                  io->triggerFB.registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), true, 0 );
                                }

                                myOSSwarm.unlock();
                              }
                              break;

    default:                  myOSSwarm.lock();
                              printf("Error: invalid command.\n");
                              myOSSwarm.unlock();
                              break;
  }

}

void SwOSCLI::executeServoCmd( void ) {

  SwOSServo *io = (SwOSServo *)_io;
  
  switch ( _cmd ) {
    case IOCMD_setPosition:     if (_parameter[0].inRange( "position", -255, 255 ) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setPosition( (int16_t) _parameter[0].getValue(), false );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getPosition:     myOSSwarm.lock();
                                printf("R: %d\n", io->getPosition() ); 
                                myOSSwarm.unlock();
                                break;

    case IOCMD_setOffset:       if (_parameter[0].inRange( "offset", -255, 255 ) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setOffset( (int16_t) _parameter[0].getValue(), false );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getOffset:       myOSSwarm.lock();
                                printf("R: %d\n", io->getOffset() ); 
                                myOSSwarm.unlock();
                                break;

    default:                    printf("Error: invalid command.\n");
                                break;
  }

}

void SwOSCLI::executePixelCmd( void ) {

  SwOSPixel *io = (SwOSPixel *)_io;
  
  switch ( _cmd ) {
    case IOCMD_setBrightness:   if (_parameter[0].inRange( "brightness", 0, 255 ) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setBrightness( (uint8_t) _parameter[0].getValue() );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getBrightness:   myOSSwarm.lock();
                                printf("R: %d\n", io->getBrightness() ); 
                                myOSSwarm.unlock();
                                break;

    case IOCMD_setColor:        printf("R: ok\n");
                                myOSSwarm.lock(); 
                                io->setColor( (uint) _parameter[0].getValue() );
                                myOSSwarm.unlock();
                                break;

    case IOCMD_getColor:        myOSSwarm.lock();
                                printf("R: %u\n", io->getColor() ); 
                                myOSSwarm.unlock();
                                break;

    default:                    printf("Error: invalid command.\n");
                                break;
  }

}

void SwOSCLI::executeI2CCmd( void ) {

  SwOSI2C *io = (SwOSI2C *)_io;
  
  switch ( _cmd ) {
    case IOCMD_setRegister:     if ( (_parameter[0].inRange( "register", 0, MAXI2CREGISTER ) ) &&
                                   (_parameter[1].inRange( "value", 0, 255 ) ) ) { 
                                  printf("R: ok\n");
                                  myOSSwarm.lock(); 
                                  io->setRegister( (uint8_t) _parameter[0].getValue(), (uint8_t) _parameter[1].getValue() );
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_getRegister:     if (_parameter[0].inRange( "register", 0, MAXI2CREGISTER ) ) {
                                  myOSSwarm.lock();
                                  printf("R: %d\n", io->getRegister( (uint8_t) _parameter[0].getValue() ) ); 
                                  myOSSwarm.unlock();
                                }
                                break;

    case IOCMD_onTrigger:     if (( _parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER ) ) &&
                                  ( _parameter[1].isIO() ) &&
                                  ( _parameter[2].isConstant() ) ) {
                                printf("R: ok\n");
                                myOSSwarm.lock();
                                if ( _maxParameter == 2 ) {
                                  // work with a fixed value
                                  io->registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), false, _parameter[2].getValue() );
                                } else {
                                  // work with input's value
                                  io->registerEvent( (FtSwarmTrigger_t)_parameter[0].getValue(), _parameter[1].getIO(), true, 0 );
                                }

                                myOSSwarm.unlock();
                              }


    default:                    printf("Error: invalid command.\n");
                                break;
  }

}

void SwOSCLI::executeIOCommand( void ) {

  if (!_io) return;

  switch (_io->getIOType() ) {
    case FTSWARM_INPUT:    executeInputCmd(); break;
    case FTSWARM_ACTOR:    executeActorCmd(); break;
    case FTSWARM_JOYSTICK: executeJoystickCmd(); break;
    case FTSWARM_SERVO:    executeServoCmd(); break;
    case FTSWARM_PIXEL:    executePixelCmd(); break;
    case FTSWARM_I2C:      executeI2CCmd(); break;
    //case FTSWARM_BUTTON:
    //case FTSWARM_OLED:
    //case FTSWARM_GYRO:
    default:               printf("Error: unsupported IO\n");
                           break;
  }

}

bool SwOSCLI::tokenizeConstant( char *token, int param) {

    for (uint8_t i=0; i<=12; i++ ) {
        if ( strcmp( constantList[i].constant, token ) == 0 ) {
            return true;
        }
    }

    return false;

}

bool SwOSCLI::tokenizeCmd( char *cmd ) {

  for (uint8_t i=0; i<=IOCMD_MAX; i++) {
    if ( strcmp( IOCmdList[i].cmd, cmd ) == 0 ) {
      _cmd = (IOCmd_t) i;
      return true;
    }
  }

  return false;

}

SwOSIO *SwOSCLI::getIO( char *token, char *IOName ) {
  
  SwOSCtrl *ctrl = NULL;
  SwOSIO   *io   = NULL;

  ctrl = (SwOSCtrl *)myOSSwarm.getControler( token );
  if (ctrl) { 
    // it's a controller, now we need the io port
    strcpy( IOName, token);
    if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return NULL; }
    if ( getNextToken( token ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return NULL; }
    io = ctrl->getIO( token );
    strcat(IOName, ".");
    strcat(IOName, token);
  } else {
    io = myOSSwarm.getIO( token );
    strcpy( IOName, token );
  }

  return io;

}

void SwOSCLI::evalIOCommand( char *token ) {
  // token is already the first literal. Could be a host or an IO

  SwOSIO   *paramIO;
  char     cmd[CLIMAXLINE];
  char     IOName[CLIMAXLINE];
  char     paramIOName[CLIMAXLINE];

  // check, if the token is a controller or an io
  _io = getIO( token, IOName );

  // check, if it's an IO
  if (!_io) { Error( ERROR_IOEXPECTED ); return; }

  // now we need another "." and a method
  if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return; }

  // next token should be a command
  if ( getNextToken( cmd ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return; }

  // let's tokenize the cmd
  if ( !tokenizeCmd( cmd ) ) { Error( ERROR_UNKOWNCMD ); return; }

  // optional parameters
  _maxParameter = -1;
  switch ( getNextToken( token ) ) {
    case EVAL_EOL: break;
    case EVAL_LPARANTHESIS: {
      bool cont = true;
      while (cont) {

        // get a parameter
        switch ( getNextToken( token ) ) {
          case EVAL_LITERAL:        paramIO = getIO( token, paramIOName);
                                    _maxParameter++;
                                    _parameter[_maxParameter].setValue(paramIO, paramIOName);
                                    break;
          case EVAL_NUMBER:         _maxParameter++;
                                    _parameter[_maxParameter].setValue(token);
                                    break;
          case EVAL_RPARANTHESIS:   cont=false;
                                    break;
          default:                  Error( ERROR_NUMBEREXPECTED ); 
                                    return;
        }
        
        // if first token was ), stop the loop
        if (!cont) break;

        // now we're looking for , and )
        switch ( getNextToken( token ) ) {
          case EVAL_RPARANTHESIS: cont = false;
                                  break;
          case EVAL_COMMA:        break;
          default:                Error( ERROR_RPARANTHESISEXPECTED ); return;
        }
      }
      break; }
    default:
      Error( ERROR_LPARANTHESISEXPECTED );
      return;
  }

  if ( ( _maxParameter+1 != IOCmdList[ _cmd ].minParams ) &&
       ( _maxParameter+1 != IOCmdList[ _cmd ].maxParams ) ) { 
    Error( ERROR_WRONGNUMBEROFARGUMENTS, IOCmdList[ _cmd ].maxParams, _maxParameter+1 ); return;
  } 

  // nothing should be left
  if ( getNextToken( token ) != EVAL_EOL ) { Error( ERROR_SYNTAXERROR ); return; }

  // subscribe needs non-int-parameters
  if (_cmd==IOCMD_subscribe) {
    myOSSwarm.lock();
    int a = _parameter[0].getValue();
    _io->subscribe( IOName, a);
    myOSSwarm.unlock();
  } else {
    executeIOCommand();
  }

}

void SwOSCLI::run( void ) {

  printf("\n\nftSwarmOS %s\n\n(C) Christian Bergschneider & Stefan Fuss\n", SWOSVERSION );
  printf("\n\nsetup - start configuration menus\nexit - end CLI mode\nhelp - show commands\n\n");

  startCLI( false );

  while (true) {

    // wait on user
    enterString( prompt, _line, CLIMAXLINE-1 );
    
    // eval string
    if (!eval() ) break;
  
  }

}