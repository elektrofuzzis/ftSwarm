/*
 * SwOSCLI.cpp
 *
 * ftSwarm Command Line Interface
 * 
 * (C) 2023 Christian Bergschneider & Stefan Fuss
 * 
 */

#include <ctype.h>

#include "SwOS.h"
#include "SwOSCLI.h"
#include "easyKey.h"
#include "SwOSSwarm.h"
#include "SwOSFirmware.h"
#include "SwOSCLIParameter.h"
#include "SwOSLog.h"

#define CLIMAXLINE 255

typedef struct {
  char cmd[20];
  bool loginNeeded;
  int  minParams, maxParams;
} IOCmdList_t;

const IOCmdList_t IOCmdList [CLICMD_MAX] = {
  { "login", false, 1, 1 },
  { "triggerUserEvent", false, 0, 10 },
  { "show", false, 0, 0 },
  { "getSwarm", false, 1, 1 },
  { "getEvents", false, 1, 1 },
  { "save", true, 1, 1 },
  { "useConfig", true, 1, 1 },
  { "setAlias", true, 1, 1 },
  { "setWifi", true, 3, 3 },
  { "reboot", true, 0, 0 },
  { "setMicrostepMode", true, 1, 1 },
  { "getMicrostepMode", false, 0, 0 },
  { "subscribe", true, 0, 1 },
  { "unsubscribe", true, 0, 0 },
  { "setIOType", true, 1, 2},
  { "getIOType", false, 0, 0},
  { "getValue", false, 0, 0},
  { "getVoltage", false, 0, 0},
  { "getResistance", false, 0, 0},
  { "getKelvin", false, 0, 0},
  { "getCelcius", false, 0, 0},
  { "getFahrenheit", false, 0, 0},
  { "getToggle", false, 0, 0},
  { "setSpeed", true, 1, 1},
  { "getSpeed", false, 0, 0},
  { "setMotionType", true, 0, 0},
  { "getMotionType", false, 0, 0},
  { "onTrigger", true, 2, 3},
  { "onTriggerLR", true, 2, 3},
  { "onTriggerFB", true, 2, 3},
  { "setPosition", true, 1, 1},
  { "getPosition", false, 0, 0},
  { "setOffset", true, 1, 1},
  { "getOffset", false, 0, 0},
  { "setColor", true, 1, 1},
  { "getColor", false, 0, 0},
  { "setBrightness", true, 1, 1},
  { "getBrightness", false, 0, 0},
  { "setRegister", true, 2, 2},
  { "getRegister", false, 1, 1},
  { "setDistance", true, 2, 2},
  { "getDistance", false, 0, 0},
  { "run", true, 0, 0},
  { "isRunning", true, 0, 0},
  { "stop", true, 0, 0},
  { "homing", true, 1, 1},
  { "isHoming", true, 0, 0},
  { "setHomingOffset", true, 1, 1}
};

const char help[] = R"(help   - list all commands
setup  - start setup mode
halt   - stop all motors
whoami - my own hostname
uptime - my own uptime
exit   - end command line interface.

swarm.<Command>(<parameter>, ...) or
<Alias-Name>.<Command>(<parameter>, ...) or
<Hostname>.<Controller-xcommand>(<parameter>, ...) or
<Hostname>.<IO-Name>.<Command>(<parameter>, ...)

Swarm Commands:
  get( format )                 - get swarm info aka getSwarm
  save(scope)                   - save settings of all swarm members to nvs - 0 all, 1 config, 2 alias, 3 events
  useConfig(config)             - use event config

Controller commands:
  show                          - identify controller by blue LEDs
  reboot                        - reboot controller
  save(scope)                   - save settings to nvs - 0 all, 1 config, 2 alias, 3 events
  setWifi(mode, SSID, PSK)      - set wifi settings
  triggerUserEvent(P1,P2,..P10) - Trigger a user remote code.
  setMicroStepMode(mode)        - set Microstep Mode / ftSwarmPwrDrive only
  getMicroStepMode()            - get Microstep Mode / ftSwarmPwrDrive only

Input commands (A1..A6):
  subscribe( hysteresis )
  unsubscribe()
  getIOType()
  setIOType( sensorType, normallyOpen )
  getValue()
  getVoltage()
  getResistance()
  getKelvin()
  getCelcius()
  getFahrenheit()
  getToggle()
  onTrigger( triggerEvent, actor, p1)
  onTrigger( triggerEvent, actor)
    
Joystick commands (JOY1..JOY2):
  subscribe( int hysteresis )
  unsubscribe()
  getValue()
  onTriggerLR( triggerEvent, actor, p1)
  onTriggerLR( triggerEvent, actor)
  onTriggerFB( triggerEvent, actor, p1)
  onTriggerFB( triggerEvent, actor)

DC-Motor commands (M1..M8):
  getIOType()
  setIOType( actorType )
  setSpeed( speed )
  getSpeed()
  setMotionType( motionType )
  getMotionType()

Stepper commands (M1..M4):
  setIOType( Stepper )
  getIOType()
  setSpeed( speed )
  getSpeed()
  setMotionType( type )
  getMotionType()
  setPosition( position )
  getPosition()
  setDistance( steps, relative )
  getDistance()
  run()
  isRunning()
  stop()
  homing( maxsteps )
  isHoming()
  setHomingOffset( steps )
  subscribe( event )

Servo commands (SERVO1..SERVO2):
  setPosition( position )
  getPosition()
  setOffset( position )
  getOffset()

ftPixel commands (LED1..LED18):
  setColor( color )
  getColor()
  setBrightness( brightness )
  getBrightness()    

I2C commands:
  setRegister( register, value )
  getRegister( register )
  onTrigger( triggerEvent, actor, p1)
)";


void SwOSCLI::Error( Error_t error, int expected, int found ) {

  int pos = 0;

  // calc error postion
  if ( (start) && (in) && ( start > in ) ) pos = start - in; 

  const char err[] = "^ Error: ";

  // print error
  switch ( error ) {
    case ERROR_SYNTAXERROR:             sprintf( response, "%*ssyntax error.", pos, err); break;
    case ERROR_DOTEXPECTED:             sprintf( response, "%*s\".\" expected.", pos, err); break;  
    case ERROR_LITERALEXPECTED:         sprintf( response, "%*sliteral expected.", pos, err); break; 
    case ERROR_IOEXPECTED:              sprintf( response, "%*snot a valid IO port or IO port is offline.", pos, err); break;
    case ERROR_NUMBEREXPECTED:          sprintf( response, "%*snumber expected.", pos, err); break;
    case ERROR_LPARANTHESISEXPECTED:    sprintf( response, "%*s\"(\" expected.", pos, err); break;
    case ERROR_RPARANTHESISEXPECTED:    sprintf( response, "%*s\")\" expected.", pos, err); break;
    case ERROR_UNKOWNCMD:               sprintf( response, "%*sunkown command", pos, err); break;
    case ERROR_WRONGNUMBEROFARGUMENTS:  sprintf( response, "%*s%d parameters expected, %d found.", pos, err, expected, found ); break;
    case ERROR_NOTAUTHENTICATED:        sprintf( response, "%*snot authenticated.", pos, err); break;
    case ERROR_WRONGIOTYPE:             sprintf( response, "%*s%d wrong io-type.", pos, err, found); break;
    case ERROR_INVALIDCMD:              sprintf( response, "%*sinvalid command.", pos, err); break;
    case ERROR_WRONGPIN:                sprintf( response, "%*swrong pin.", pos, err); break;
    case ERROR_STRINGEXPECTED:          sprintf( response, "%*sstring expected.", pos, err); break;
    case ERROR_ALIASNOTNUNIQUE:         sprintf( response, "%*salias name must be unique.", pos, err); break;
    case ERROR_PARAMETEREXPECTED:       sprintf( response, "%*sparameter expected.", pos, err); break;
    case ERROR_SSIDEXPECTED:            sprintf( response, "%*sSSID expected.", pos, err); break;
    case ERROR_PSKEXPECTED:             sprintf( response, "%*sPSK expected.", pos, err); break;
    default:                            sprintf( response, "%*sSyntax error.", pos, err); break;
  }
  
}

EvalResult_t SwOSCLI::getNumber( void ) {
  // evalPtr is already on the first char

  int base   = 10;
  int digits = 0;

  switch (evalPtr[0]) {

    case '#': // RGB hex
              evalPtr++; 
              base = 16; 
              break;

    case '-': // Negative
              evalPtr++; 
              break;

    case '+': // Positive
              evalPtr++; 
              break;

    case '0': // hex
              if ( ( evalPtr[1] == 'x' ) || ( evalPtr[1] == 'X' ) ) {
                evalPtr++;
                evalPtr++;
                base = 16;
              }
              break;
  }

  if ( base == 16 ) {
    while ( (*evalPtr != '\0') && ( isxdigit( *evalPtr ) ) ) { evalPtr++; digits++; }

  } else {
    while ( (*evalPtr != '\0') && ( isdigit( *evalPtr ) ) ) { evalPtr++; digits++; }

  }

  if ( digits > 0 ) return EVAL_NUMBER;

  return EVAL_SYNTAXERROR;

}

EvalResult_t SwOSCLI::getLiteral( void ) {
  // evalPtr is already on the first char

  // now loop until end of literals
  while (isalnum(*evalPtr)) evalPtr++;

  return EVAL_LITERAL;

}

EvalResult_t SwOSCLI::getString( void ) {
  // evalPtr is already on the first char

  // move behind "
  if ( *evalPtr == '"' ) evalPtr++;
  else return EVAL_SYNTAXERROR;

  // now loop until end of literals
  while ( ( *evalPtr != '\0' ) && ( *evalPtr != '"' ) ) evalPtr++;

  if ( *evalPtr == '"' ) evalPtr++;
  else return EVAL_SYNTAXERROR;

  return EVAL_STRING;

}

EvalResult_t SwOSCLI::getNextToken( char *token ) {

  // kill white spaces
  while (*evalPtr==' ') evalPtr++;

  // remember where we started
  start = evalPtr;  

  // different behaviour based on first char
  EvalResult_t eval = EVAL_OK;
  switch (*evalPtr) {
    case '\0':        eval = EVAL_EOL; break; // no evalPtr++, so we can't skip EOL
    
    case '.':         eval = EVAL_DOT; evalPtr++; break;
    
    case ',':         eval = EVAL_COMMA; evalPtr++; break;

    case '(':         eval = EVAL_LPARANTHESIS; evalPtr++; break;
    
    case ')':         eval = EVAL_RPARANTHESIS; evalPtr++; break;
    
    case '+':
    case '-':  
    case '#':
    case '0' ... '9': eval = getNumber( ); break;
    
    case 'a' ... 'z':
    case 'A' ... 'Z': eval = getLiteral( ); break;

    case '"':         eval = getString( ); break;
    
    default:          eval = EVAL_SYNTAXERROR; break;

  }

  // valid data? copy token or clear it
  if ( eval >= EVAL_OK ) {
    strncpy( token, start, (evalPtr - start) );
    token[evalPtr - start]='\0';
    
  } else {
    token[0] = '\0';
  }

  // return evaluation result
  return eval;
}

void SwOSCLI::halt( void ) {

  myOSSwarm.halt();

}

Cmd_t SwOSCLI::evalSimpleCommand( char *token ) {

  Cmd_t cmd = CMD_UNKONWN;

   // did I found a command?
  if      ( strcmp( token, "help"  ) == 0 )    cmd = CMD_HELP;
  else if ( strcmp( token, "whoami" ) == 0 )   cmd = CMD_WHOAMI;
  else if ( strcmp( token, "uptime" ) == 0 )   cmd = CMD_UPTIME;
  else if ( strcmp( token, "setup" ) == 0 )    cmd = CMD_SETUP;
  else if ( strcmp( token, "startCLI" ) == 0 ) cmd = CMD_STARTCLI;
  else if ( strcmp( token, "halt" ) == 0 )     cmd = CMD_HALT;
  else if ( strcmp( token, "exit" ) == 0 )     cmd = CMD_EXIT;
  
  // command found?
  if ( cmd >= 0 ) {

    // all commands don't have parameters
    if ( getNextToken( token ) != EVAL_EOL ) {
      Error( ERROR_SYNTAXERROR );
      return CMD_ERROR;
    }

    float          uptime;

    // execute 
    switch (cmd) {
      case CMD_HELP:      free(response);
                          response = (char *) calloc( 1, sizeof( help ) );
                          strcpy(response, help);
                          break;

      case CMD_WHOAMI:    myOSSwarm.Ctrl[0]->lock();
                          sprintf( response, "%s/%s", myOSSwarm.Ctrl[0]->getName(), myOSSwarm.Ctrl[0]->getAlias() );  
                          myOSSwarm.Ctrl[0]->identify();  
                          myOSSwarm.Ctrl[0]->unlock();
                          break;

      case CMD_UPTIME:    uptime = millis()/1000;
                          sprintf( response, "uptime: %.3f s", uptime);
                          break;

      case CMD_SETUP:     if (interactive) mainMenu();
                          else Error( ERROR_INVALIDCMD );              
                          break;

      case CMD_STARTCLI:  if (interactive) startCLI( true ); 
                          else Error( ERROR_INVALIDCMD );              
                          break;

      case CMD_HALT:      halt();
                          break;
    
    }

  }

  return cmd;

}

char *SwOSCLI::eval( char* in, bool *loggedIn ) {
  // returns false on command exit

  this->in       = in;
  this->response = (char *) calloc( 1, CLIMAXLINE );

  strcpy( this->response, "");

  char token[CLIMAXLINE];
  
  // rest variables to run the evaluation
  evalPtr = in;

  // every line must start with a literal
  switch ( getNextToken( token ) ) {

    case EVAL_LITERAL: break;         // everything ok

    case EVAL_EOL:     return response;   // just an empty line, ignore

    default:           Error( ERROR_LITERALEXPECTED ); return response;

  }

  // interpret as a simple command?
  switch ( evalSimpleCommand( token ) ) {

    case CMD_EXIT:    exit = true;
                      if (interactive) return response;   // exit: stop CLI
                      else Error( ERROR_INVALIDCMD );

    case CMD_UNKONWN: break;           // not a simple command: continue

    default:          return response; // error, setup, help, startcli, halt: stop evaluation

  }

  // if it's not a command, it should be a host or an io.
  evalComplexCommand( token, loggedIn );
 
  return response;

}

void SwOSCLI::OK ( void ) {

  sprintf( response, "R: ok" );

}

void SwOSCLI::executeControllerCmd(void ) {

  SwOSCom *userEvent;
  int      p, microStepMode;

  switch ( cmd ) {
    case CLICMD_show:               // show controller
                                    ctrl->lock();
                                    ctrl->identify();  
                                    ctrl->unlock();
                                    break;

    case CLICMD_triggerUserEvent:   // create SwOSCom header
                                    userEvent = new SwOSCom( ctrl->macAddr, ctrl->serialNumber, CMD_USEREVENT );

                                    // copy parameters
                                    for (uint8_t i=0; i<=maxParameter; i++) {
                                      p = parameter[i].getNumber();
                                      memcpy( &userEvent->data.userEventCmd.payload[i*sizeof(int)], &p, sizeof(int) );
                                    }

                                    // send event
                                    if ( ctrl->isLocal()) {

                                      if ( xQueueSend( myOSNetwork.userEvent, userEvent, ESPNOW_MAXDELAY ) != pdTRUE )SWARM_LOG_ERROR( "Can't send data to user event." );
      
                                    } else {
                                      userEvent->data.userEventCmd.trigger = true;
                                      userEvent->send();
                                    }

                                    // cleanup
                                    delete userEvent;

                                    break;

    case CLICMD_setMicrostepMode:   if (parameter[0].inRange( "MicroStepMode", 0, 7, response ) ) {
                                      OK();
                                      if ( ctrl->getType() == FTSWARMPWRDRIVE ) {
                                        ctrl->lock();
                                        ctrl->setMicrostepMode( (uint8_t) parameter[0].getNumber() );
                                        ctrl->unlock();
                                      }
                                    }
                                    break;

    case CLICMD_getMicrostepMode:   if ( ctrl->getType() == FTSWARMPWRDRIVE ) {
                                      ctrl->lock();
                                      microStepMode = ctrl->getMicrostepMode();
                                      ctrl->unlock();
                                      sprintf( response, "R: %d", microStepMode );
                                    } else { sprintf( response, "kein PwrDrive"); }
                                    break;

    case CLICMD_save:               if ( parameter[0].inRange( "scope", 0, 3, response ) ) {
                                      OK();
                                      ctrl->save( parameter[0].getNumber() );
                                    }
                                    break;

    case CLICMD_reboot:             ctrl->reboot();
                                    OK();
                                    break;

    case CLICMD_setWifi:            // wifi mode in range from 0 to 2?
                                    if ( !parameter[0].inRange( "mode", 0, 2, response ) ) {}
                                    // wifi is on, a SSID is needed
                                    else if ( ( parameter[0].getNumber() != wifiOFF ) && ( !parameter[1].isString() ) ) Error( ERROR_SSIDEXPECTED );
                                    // wifi is in client mode, a PSK is needed
                                    else if ( ( parameter[0].getNumber() == wifiClient ) && ( !parameter[2].isString() ) ) Error( ERROR_PSKEXPECTED );
                                    // everything is fine
                                    else {
                                      OK();
                                      ctrl->setWifi( (FtSwarmWifi_t) parameter[0].getNumber(), parameter[1].getString(), parameter[2].getString() );
                                    }
    
                                    break;
    default:                        Error( ERROR_INVALIDCMD );
                                    break;
  }

}

void SwOSCLI::executeInputCmd( void ) {

  SwOSIOType_t newSensorType, newIOType;

  SwOSCtrl     *ctrl = io->getCtrl();
  uint8_t      index = ctrl->getIndex( io );

  switch ( cmd ) {

    case CLICMD_getIOType:      io->lock();
                                sprintf( response, "R: %d", io->getIOType() ); 
                                io->unlock();
                                break;

    case CLICMD_setIOType:      if ( ( parameter[0].inRange( "ioType", 0, SWOSIO_MAXIOTYPE-1, response ) ) && 
                                     ( parameter[1].inRange( "normallyOpen", 0, 1, response ) ) ) {

                                  // which sensor type?
                                  newIOType =  (SwOSIOType_t) parameter[0].getNumber();

                                  if ( ctrl->changeIOType( index, newIOType ) ) {
                                    io = ctrl->io[ index ];
                                    if (io) io->setParameter( parameter[1].getNumber() );
                                    OK();
                                  }

                                } else Error( ERROR_WRONGIOTYPE, 0, newIOType );

                                break;

    case CLICMD_getValue:       io->lock();
                                sprintf( response, "R: %d", ((SwOSInput *)io)->getValueI32() );
                                io->unlock();
                                break;

    case CLICMD_getVoltage:     io->lock();
                                if ( ( io->getIOType() == SWOSIO_VOLTMETER ) | ( io->getIOType() == SWOSIO_POWER ) ) {
                                  sprintf( response, "R: %f", ((SwOSAnalogInput *)io)->getVoltage());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_getResistance: io->lock();
                                if ( io->getIOType() == SWOSIO_OHMMETER ) {
                                  sprintf( response, "R: %f", ((SwOSAnalogInput *)io)->getResistance());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_getKelvin:      io->lock();
                                if ( io->getIOType() == SWOSIO_THERMOMETER ) {
                                  sprintf( response, "R: %f", ((SwOSAnalogInput *)io)->getKelvin());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_getCelcius:     io->lock();
                                if ( io->getIOType() == SWOSIO_THERMOMETER ) {
                                  sprintf( response, "R: %f", ((SwOSAnalogInput *)io)->getCelcius());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_getFahrenheit:  io->lock();
                                if ( io->getIOType() == SWOSIO_THERMOMETER ) {
                                  sprintf( response, "R: %f", ((SwOSAnalogInput *)io)->getFahrenheit());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_getToggle:      io->lock();
                                if ( io->isDigitalInput() ) {
                                  sprintf( response, "R: %f", ((SwOSDigitalInput*)io)->getToggle());
                                } else {
                                  Error( ERROR_WRONGIOTYPE, 0, io->getIOType() );
                                }
                                io->unlock();
                                break;

    case CLICMD_onTrigger:      // TODO better error handling
                                if (( parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER-1, response ) ) &&
                                    ( parameter[1].isIO() ) &&
                                    ( parameter[2].isNumber() ) ) {
                                  OK();
                                  io->lock();
                                  ((SwOSInput *)io)->addEvent( (FtSwarmTrigger_t)parameter[0].getNumber(), parameter[1].getIO(), parameter[2].getNumber() );
                                  io->unlock();
                                }
                                break;

    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executeActorCmd( void ) {

  SwOSMotor    *motor   = (SwOSMotor *)io;
  SwOSStepper  *stepper = (SwOSStepper *)io;
  int          maxspeed;
  bool         ok = true;
  SwOSCtrl     *ctrl = io->getCtrl();
  uint8_t      index = ctrl->getIndex( io );
  SwOSIOType_t newIOType;
  
  switch ( cmd ) {

    case CLICMD_setIOType:      if ( parameter[0].inRange( "ioType", 0, SWOSIO_MAXIOTYPE-1, response ) ) {

                                  // which sensor type?
                                  newIOType =  (SwOSIOType_t) parameter[0].getNumber();

                                  if ( ctrl->changeIOType( index, newIOType ) ) {
                                  
                                    motor = (SwOSMotor *)ctrl->io[ index ];
                                  
                                    if (motor) {
                                      motor->lock();
                                      motor->setSpeed(0);
                                      motor->apply();
                                      motor->unlock();
                                    }

                                    OK();

                                  }

                                } else Error( ERROR_WRONGIOTYPE, 0, newIOType );

                                break;
                              
    case CLICMD_getIOType:      motor->lock();
                                sprintf( response, "R: %d", (int) motor->getIOType() ); 
                                motor->unlock();
                                break;

    case CLICMD_setSpeed:       if (parameter[0].inRange( "speed", -motor->maxSpeed(), motor->maxSpeed(), response ) ) { 
                                  OK();
                                  motor->lock(); 
                                  motor->setSpeed( parameter[0].getNumber() );
                                  motor->apply();
                                  motor->unlock();
                                }
                                break;

    case CLICMD_getSpeed:       motor->lock();
                                sprintf( response, "R: %d", motor->getSpeed() ); 
                                motor->unlock();
                                break;

    case CLICMD_setMotionType:  if (parameter[0].inRange( "motionType", 0, FTSWARM_MAXMOTION-1, response) ) { 
                                  OK();
                                  motor->lock(); 
                                  motor->setMotionType( (FtSwarmMotion_t) parameter[0].getNumber() );
                                  motor->apply();
                                  motor->unlock();
                                }
                                break;

    case CLICMD_getMotionType:  motor->lock();
                                sprintf( response, "R: %d", (int) motor->getMotionType() ); 
                                motor->unlock();
                                break;

    case CLICMD_setDistance:    if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->setDistance( parameter[0].getNumber(), (parameter[1].getNumber() > 0) );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_getDistance:    if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  stepper->lock();
                                  sprintf( response, "R: %d", stepper->getDistance() ); 
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_run:            if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->startStop( true );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_isRunning:      if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  stepper->lock();
                                  sprintf( response, "R: %d",stepper->isRunning() );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_stop:           if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->startStop( false );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_setPosition:    if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->setPosition( parameter[0].getNumber() );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_getPosition:    if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  stepper->lock();
                                  sprintf( response, "R: %d", stepper->getPosition() ); 
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_homing:         if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->homing( parameter[0].getNumber() );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;
                                
    case CLICMD_isHoming:       if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  stepper->lock();
                                  sprintf( response, "R: %d", stepper->isHoming() ); 
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    case CLICMD_setHomingOffset: if ( stepper->getIOType() == SWOSIO_STEPPER ) {
                                  OK();
                                  stepper->lock(); 
                                  stepper->setHomingOffset( parameter[0].getNumber() );
                                  stepper->unlock();
                                } else Error( ERROR_WRONGIOTYPE, 0, stepper->getIOType() );
                                break;

    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executeJoystickCmd( void ) {

  switch ( cmd ) {

    case CLICMD_getValue:     io->lock();
                              int16_t lr, fb;
                              ((SwOSJoystick*)io)->getValue( &lr, &fb );
                              sprintf(response, "R: %d %d", lr, fb ); 
                              io->unlock();
                              break;

    case CLICMD_onTriggerLR:  if (( parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER-1, response ) ) &&
                                  ( parameter[1].isIO() ) &&
                                  ( parameter[2].isNumber() ) ) {
                                OK();
                                io->lock();
                                ((SwOSJoystick*)io)->lr->addEvent( (FtSwarmTrigger_t)parameter[0].getNumber(), parameter[1].getIO(), parameter[2].getNumber()  );
                                io->unlock();
                              }
                              break;

    case CLICMD_onTriggerFB:  if (( parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER-1, response ) ) &&
                                  ( parameter[1].isIO() ) &&
                                  ( parameter[2].isNumber() ) ) {
                                OK();
                                io->lock();
                                ((SwOSJoystick*)io)->fb->addEvent( (FtSwarmTrigger_t)parameter[0].getNumber(), parameter[1].getIO(), parameter[2].getNumber() );
                                io->unlock();
                              }
                              break;

    default:                  Error( ERROR_INVALIDCMD );
                              break;
  }

}

void SwOSCLI::executeServoCmd( void ) {
  
  switch ( cmd ) {
    case CLICMD_setPosition:    if (parameter[0].inRange( "position", -255, 255, response ) ) { 
                                  OK();
                                  io->lock(); 
                                  ((SwOSServo*) io)->setPosition( (int16_t) parameter[0].getNumber() );
                                  io->unlock();
                                }
                                break;

    case CLICMD_getPosition:    io->lock();
                                sprintf( response, "R: %d", ((SwOSServo*) io)->getPosition() ); 
                                io->unlock();
                                break;

    case CLICMD_setOffset:      if (parameter[0].inRange( "offset", -255, 255, response ) ) { 
                                  OK();
                                  io->lock(); 
                                  ((SwOSServo*) io)->setOffset( (int16_t) parameter[0].getNumber() );
                                  io->unlock();
                                }
                                break;

    case CLICMD_getOffset:      io->lock();
                                sprintf( response, "R: %d", ((SwOSServo*) io)->getOffset() ); 
                                io->unlock();
                                break;

    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executePixelCmd( void ) {

  switch ( cmd ) {
    case CLICMD_setBrightness:   if (parameter[0].inRange( "brightness", 0, 255, response ) ) { 
                                  OK();
                                  io->lock(); 
                                  ((SwOSPixel *)io)->setBrightness( (uint8_t) parameter[0].getNumber() );
                                  io->unlock();
                                }
                                break;

    case CLICMD_getBrightness:  io->lock();
                                sprintf( response, "R: %d", ((SwOSPixel *)io)->getBrightness() ); 
                                io->unlock();
                                break;

    case CLICMD_setColor:       OK();
                                io->lock(); 
                                ((SwOSPixel *)io)->setColor( (uint) parameter[0].getNumber() );
                                io->unlock();
                                break;

    case CLICMD_getColor:       io->lock();
                                sprintf( response, "R: #%X", ((SwOSPixel *)io)->getColor() ); 
                                io->unlock();
                                break;

    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executeI2CCmd( void ) {

  switch ( cmd ) {
    case CLICMD_setRegister:    if ( (parameter[0].inRange( "register", 0, MAXI2CREGISTERS-1, response ) ) &&
                                     ( parameter[1].inRange( "value", 0, 255, response ) ) ) { 
                                  OK();
                                  io->lock(); 
                                  ((SwOSI2C *)io)->setRegister( (uint8_t) parameter[0].getNumber(), (uint8_t) parameter[1].getNumber() );
                                  io->unlock();
                                }
                                break;

    case CLICMD_getRegister:    if ( parameter[0].inRange( "register", 0, MAXI2CREGISTERS-1, response ) ) {
                                  io->lock();
                                  sprintf( response, "R: %d", ((SwOSI2C *)io)->getRegister( (uint8_t) parameter[0].getNumber() ) ); 
                                  io->unlock();
                                }
                                break;

    case CLICMD_onTrigger:     if ( ( parameter[0].inRange( "actor", 0, FTSWARM_MAXTRIGGER-1, response ) ) &&
                                  ( parameter[1].isIO() ) &&
                                  ( parameter[2].isNumber() ) ) {
                                  OK();
                                  io->lock();
                                  ((SwOSI2C *)io)->addEvent( (FtSwarmTrigger_t)parameter[0].getNumber(), parameter[1].getIO(), parameter[2].getNumber() );
                                  io->unlock();
                                }


    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executeSwarmCmd( bool *loggedIn ) {

  SerialFormat_t format;
  size_t         size;
  Serialize      *serialize = NULL;
  uint8_t        newConfig;

  switch ( cmd ) {

    case CLICMD_login:          if ( parameter[0].inRange( "pin", 0, 9999, response ) ) {
                                  *loggedIn = ( parameter[0].getNumber() == myOSNetwork.pin );
                                  if (!*loggedIn) Error( ERROR_WRONGPIN );
                                  else OK();
                                }
                                break;

    case CLICMD_getSwarm:       // aka /api/getSwarm
                                if ( parameter[0].inRange( "format", 0, 1, response ) ) {
                                  free( response );
                                  format = (SerialFormat_t)parameter[0].getNumber();
                                  size = myOSSwarm.approxSerialize( format );
                                  response = (char *) calloc( myOSSwarm.maxCtrl+1, size );
                                  serialize = new Serialize( response, size, format );
                                  serialize->write("R: ");
                                  myOSSwarm.serialize( serialize );
                                  delete serialize;
                                }
                                break;

    case CLICMD_getEvents:      if ( parameter[0].inRange( "format", 0, 1, response ) ) {
                                  free( response );
                                  format = (SerialFormat_t)parameter[0].getNumber();
                                  response = (char *) calloc( myOSSwarm.maxCtrl, 10240 );
                                  serialize = new Serialize( response, 10240, format );
                                  serialize->write("R: ");
                                  myOSSwarm.serializeEvents( serialize );
                                  delete serialize;
                                }
                                break;
    
    case CLICMD_save:           if ( parameter[0].inRange( "scope", 0, 3, response ) ) {
                                  OK();
                                  myOSSwarm.save( parameter[0].getNumber() );
                                }
                                break;

    case CLICMD_useConfig:      if ( parameter[0].inRange( "config", 1, MAXEVENTCONFIGS-1, response ) ) {
                                  OK();
                                  newConfig = parameter[0].getNumber()-1;
                                  nvs.activeEventConfig = newConfig;
                                  myOSSwarm.deleteEvents();
                                  myOSSwarm.addEvents( newConfig );
                                }
                                break;

    default:                    Error( ERROR_INVALIDCMD );
                                break;
  }

}

void SwOSCLI::executeIOCommand( void ) {

  // set alias name io or ctrl
  if ( cmd == CLICMD_setAlias ) {

    char *alias = parameter[0].getString();

    if (!alias) { Error( ERROR_STRINGEXPECTED ); return; }

    if (!isValidIdentifier(alias)) { Error( ERROR_LITERALEXPECTED ); return; }

    if (myOSSwarm.getIO(alias)) { Error( ERROR_ALIASNOTNUNIQUE ); return; }

    OK();
    if (io)     { io->setAlias(alias); return; }
    if (ctrl)   { ctrl->setAlias(alias); return; }

  }

  if ( (!io) && (ctrl ) ) {
    // controller cmd?
    executeControllerCmd( );

  } else if (io ) { 
   
    // io cmd?
    switch (io->getIOType() ) {

      case SWOSIO_DIGITAL: 
      case SWOSIO_SWITCH:
      case SWOSIO_REEDSWITCH:
      case SWOSIO_LIGHTBARRIER:
      case SWOSIO_BUTTON:              
      case SWOSIO_ANALOG:
      case SWOSIO_VOLTMETER: 
      case SWOSIO_OHMMETER:
      case SWOSIO_THERMOMETER:
      case SWOSIO_LDR:
      case SWOSIO_COUNTER:
      case SWOSIO_ROTARYENCODER:
      case SWOSIO_JOYSTICK_POTI:
      case SWOSIO_POWER:
      case SWOSIO_FREQUENCYMETER: executeInputCmd(); break;

      case SWOSIO_MOTOR:
      case SWOSIO_XMOTOR:
      case SWOSIO_XMMOTOR: 
      case SWOSIO_TRACTOR:  
      case SWOSIO_ENCODER:
      case SWOSIO_LAMP:
      case SWOSIO_VALVE:
      case SWOSIO_COMPRESSOR:
      case SWOSIO_BUZZER:
      case SWOSIO_STEPPER:        executeActorCmd(); break;

      case SWOSIO_JOYSTICK:       executeJoystickCmd(); break;

      case SWOSIO_SERVO:          executeServoCmd(); break;

      case SWOSIO_PIXEL:          executePixelCmd(); break;

      case SWOSIO_I2C:            executeI2CCmd(); break;
      
      default:                    sprintf( response, "Error: unsupported IO");
                                  break;
    }

  }

}

bool SwOSCLI::tokenizeCmd( char *cmd ) {

  for (uint8_t i=0; i<=CLICMD_MAX; i++) {
    if ( strcmp( IOCmdList[i].cmd, cmd ) == 0 ) {
      this->cmd = (CLICmd_t) i;
      return true;
    }
  }

  return false;

}

bool  SwOSCLI::getIO( char *token, char *IOName, SwOSCtrl **ctrl, SwOSIO **io ) {
  
  SwOSCtrl *xctrl = NULL;
  SwOSIO   *xio   = NULL;
  char     *xrollback;

  xctrl = (SwOSCtrl *)myOSSwarm.getController( token );
  if (xctrl) { 
    // it's a controller, now we need the io port

    // first I need to save the position of evalPtr 
    xrollback = evalPtr;

    // I need to store the original ctrl.io-Text for subscribe
    strcpy( IOName, token);

    // now try to get .<io>
    if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return false; }
    if ( getNextToken( token ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return false; }

    // check if the read literal is an io
    xio = xctrl->getIO( token );
    
    if (xio) {
      // if an io is found, add .io-Name to io
      strcat(IOName, ".");
      strcat(IOName, token);

    } else {
      // if it's not an io, it was a controller: revoke last 2 getNextToken
      evalPtr = xrollback;
    }

  } else {

    // it was an alias name without controller
    xio = myOSSwarm.getIO( token );
    strcpy( IOName, token );
  }

  // copy resultgetIO
  *ctrl = xctrl;
  *io   = xio;

  return true;

}

void SwOSCLI::evalComplexCommand( char *token, bool *loggedIn ) {
  // token is already the first literal. Could be a host or an IO

  SwOSIO   *paramIO;
  SwOSCtrl *paramCtrl;
  char     command[CLIMAXLINE];
  char     IOName[CLIMAXLINE];
  char     paramIOName[CLIMAXLINE];
  bool     swarm = ( strcmp( token, "swarm" ) == 0 );

  // check, if the token is a controller or an io or nvs or swarm
  if ( (!swarm) && (!getIO( token, IOName, &ctrl, &io ) ) ) { Error( ERROR_IOEXPECTED ); return; }

  // unvalid io?
  if ( ( !io ) && ( !ctrl ) && (!swarm) ) { Error( ERROR_IOEXPECTED ); return; }

  // now we need another "." and a method
  if ( getNextToken( token ) != EVAL_DOT ) { Error( ERROR_DOTEXPECTED ); return; }

  // next token should be a command
  if ( getNextToken( command ) != EVAL_LITERAL ) { Error( ERROR_LITERALEXPECTED ); return; }

  // let's tokenize the command
  if ( !tokenizeCmd( command ) ) { Error( ERROR_UNKOWNCMD ); return; }

  // optional parameters
  maxParameter = -1;
  switch ( getNextToken( token ) ) {
    case EVAL_EOL: break;
    case EVAL_LPARANTHESIS: {
      bool cont = true;
      while (cont) {

        // get a parameter
        switch ( getNextToken( token ) ) {

          case EVAL_LITERAL:        if ( ( getIO( token, paramIOName, &paramCtrl, &paramIO ) ) && ( &paramIO ) ) {
                                      maxParameter++;
                                      parameter[maxParameter].setIO( paramIO );
                                    } else {
                                      Error( ERROR_LITERALEXPECTED );
                                      return;
                                    }
                                    break;

          case EVAL_STRING:         maxParameter++;
                                    parameter[maxParameter].setString( token );
                                    break;

          case EVAL_NUMBER:         maxParameter++;
                                    parameter[maxParameter].setNumber( token);
                                    break;

          case EVAL_RPARANTHESIS:   cont=false;
                                    break;

          default:                  if ( *evalPtr == '\0' )
                                      Error( ERROR_RPARANTHESISEXPECTED );
                                    else 
                                      Error( ERROR_PARAMETEREXPECTED ); 
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

  if ( ( maxParameter+1 < IOCmdList[ cmd ].minParams ) ||
       ( maxParameter+1 > IOCmdList[ cmd ].maxParams ) ) { 
    Error( ERROR_WRONGNUMBEROFARGUMENTS, IOCmdList[ cmd ].maxParams, maxParameter+1 ); return;
  } 

  // nothing should be left
  if ( getNextToken( token ) != EVAL_EOL ) { Error( ERROR_SYNTAXERROR ); return; }

  if ( IOCmdList[ cmd ].loginNeeded && (!*loggedIn) ) { Error( ERROR_NOTAUTHENTICATED); return; }

  if ( cmd==CLICMD_subscribe ) {

    // subscribe needs special handling due to non-int-parameters
    int a = parameter[0].getNumber();

    // controller?
    if ( (!io) && (ctrl ) ) {
      ctrl->lock();
      ctrl->subscribe( IOName );
      ctrl->unlock();
    
    // IO?
    } else {
      io->lock();
      io->subscribe( IOName, a);
      io->unlock();
    }

    OK();

  } else if ( cmd==CLICMD_unsubscribe ) {

    // controller?
    if ( (!io) && (ctrl ) ) {
      ctrl->lock();
      ctrl->unsubscribe( IOName );
      ctrl->unlock();
    
    // IO?
    } else {
      io->lock();
      io->unsubscribe( );
      io->unlock();
    }

    OK();

  } else if (swarm) {

    executeSwarmCmd( loggedIn );

  } else {

    // standard cmd
    executeIOCommand();
  
  }

}

void SwOSCLI::startCLI( bool noEcho ) {

  printf("@@@ ftSwarmOS CLI started\n");
  
  if ( noEcho ) {
    keyboardEcho( false );
    prompt[0] = '\0';
  }

  myOSSwarm.unsubscribe();
  
}

void SwOSCLI::run( void ) {

  char cmd[CLIMAXLINE];
  char *out;
  bool loggedIn = true;

  printf("\n\nsetup - starts configuration menu\nexit - end CLI mode\nhelp - show commands\n\n");
  
  interactive = true;
  startCLI( false );

  while (!exit) {

    // wait on user
    enterString( prompt, cmd, CLIMAXLINE-1 );
    
    // eval string
    out = eval( cmd, &loggedIn );

    // print result
    printf( out );
    printf( "\n" );
    free(out);
  
  }

}