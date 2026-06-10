// GENERATED IO MAPPINGS FROM ftSwarm/src/ftswarm-core/include/SwOS.h
import { SwOSIOClass, SwOSIOType } from "./genApiEnums";

export interface IoTypeInfo {
  type: SwOSIOType;
  name: string;
  ioClass: SwOSIOClass;
  showInApi: boolean;
}

export const ioTypeInfos: Record<number, IoTypeInfo> = {
  [SwOSIOType.SWOSIO_DIGITAL]: { type: SwOSIOType.SWOSIO_DIGITAL, name: "DigitalInput", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_SWITCH]: { type: SwOSIOType.SWOSIO_SWITCH, name: "Switch", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_REEDSWITCH]: { type: SwOSIOType.SWOSIO_REEDSWITCH, name: "Reedswitch", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_LIGHTBARRIER]: { type: SwOSIOType.SWOSIO_LIGHTBARRIER, name: "Lightbarrier", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_BUTTON]: { type: SwOSIOType.SWOSIO_BUTTON, name: "Button", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_ANALOG]: { type: SwOSIOType.SWOSIO_ANALOG, name: "Analog", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_VOLTMETER]: { type: SwOSIOType.SWOSIO_VOLTMETER, name: "Voltmeter", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_OHMMETER]: { type: SwOSIOType.SWOSIO_OHMMETER, name: "Ohmmeter", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_THERMOMETER]: { type: SwOSIOType.SWOSIO_THERMOMETER, name: "Thermometer", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_LDR]: { type: SwOSIOType.SWOSIO_LDR, name: "LDR", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_JOYSTICK]: { type: SwOSIOType.SWOSIO_JOYSTICK, name: "Joystick", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_MOTOR]: { type: SwOSIOType.SWOSIO_MOTOR, name: "Motor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_XSMOTOR]: { type: SwOSIOType.SWOSIO_XSMOTOR, name: "XSMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_XMMOTOR]: { type: SwOSIOType.SWOSIO_XMMOTOR, name: "XMMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_TRACTOR]: { type: SwOSIOType.SWOSIO_TRACTOR, name: "Tractor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_ENCODER]: { type: SwOSIOType.SWOSIO_ENCODER, name: "Encoder", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_LAMP]: { type: SwOSIOType.SWOSIO_LAMP, name: "Lamp", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_VALVE]: { type: SwOSIOType.SWOSIO_VALVE, name: "Valve", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_COMPRESSOR]: { type: SwOSIOType.SWOSIO_COMPRESSOR, name: "Compressor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_BUZZER]: { type: SwOSIOType.SWOSIO_BUZZER, name: "Buzzer", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_STEPPER]: { type: SwOSIOType.SWOSIO_STEPPER, name: "Stepper", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_COUNTER]: { type: SwOSIOType.SWOSIO_COUNTER, name: "Counter", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_ROTARYENCODER]: { type: SwOSIOType.SWOSIO_ROTARYENCODER, name: "Rotaryencoder", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_FREQUENCYMETER]: { type: SwOSIOType.SWOSIO_FREQUENCYMETER, name: "Frequencymeter", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_LIDAR]: { type: SwOSIOType.SWOSIO_LIDAR, name: "Lidar", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_CAM]: { type: SwOSIOType.SWOSIO_CAM, name: "Cam", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: false },
  [SwOSIOType.SWOSIO_SERVO]: { type: SwOSIOType.SWOSIO_SERVO, name: "Servo", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_PIXEL]: { type: SwOSIOType.SWOSIO_PIXEL, name: "Pixel", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_OLED]: { type: SwOSIOType.SWOSIO_OLED, name: "OLED", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: false },
  [SwOSIOType.SWOSIO_I2C]: { type: SwOSIOType.SWOSIO_I2C, name: "I2C", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: false },
  [SwOSIOType.SWOSIO_GYRO]: { type: SwOSIOType.SWOSIO_GYRO, name: "Gyro", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_POWER]: { type: SwOSIOType.SWOSIO_POWER, name: "Powersensor", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: true },
  [SwOSIOType.SWOSIO_COLORSENSOR]: { type: SwOSIOType.SWOSIO_COLORSENSOR, name: "Colorsensor", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_TRAILSENSOR]: { type: SwOSIOType.SWOSIO_TRAILSENSOR, name: "Trailsensor", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_ULTRASONIC]: { type: SwOSIOType.SWOSIO_ULTRASONIC, name: "Ultrasonic", ioClass: SwOSIOClass.SWOSIOCLASS_INPUT, showInApi: true },
  [SwOSIOType.SWOSIO_JOYSTICK_POTI]: { type: SwOSIOType.SWOSIO_JOYSTICK_POTI, name: "JoystickPoti", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: false },
  [SwOSIOType.SWOSIO_WHEELDRIVE]: { type: SwOSIOType.SWOSIO_WHEELDRIVE, name: "WheelDrive", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_MINIMOTOR]: { type: SwOSIOType.SWOSIO_MINIMOTOR, name: "MiniMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_SMOTOR]: { type: SwOSIOType.SWOSIO_SMOTOR, name: "SMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_POWERMOTOR]: { type: SwOSIOType.SWOSIO_POWERMOTOR, name: "PowerMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_MMOTOR]: { type: SwOSIOType.SWOSIO_MMOTOR, name: "MMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_RCMOTOR]: { type: SwOSIOType.SWOSIO_RCMOTOR, name: "RCMotor", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_RCSERVO]: { type: SwOSIOType.SWOSIO_RCSERVO, name: "RCServo", ioClass: SwOSIOClass.SWOSIOCLASS_MOTOR, showInApi: true },
  [SwOSIOType.SWOSIO_RCPOTI]: { type: SwOSIOType.SWOSIO_RCPOTI, name: "RCPoti", ioClass: SwOSIOClass.SWOSIOCLASS_SINGULAR, showInApi: false }
};
