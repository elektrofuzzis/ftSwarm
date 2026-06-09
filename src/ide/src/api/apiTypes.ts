import {
  type SwOSIOClass,
  SwOSIOType,
  type FtSwarmVersion,
  type SwOSState,
} from "./generated/genApiEnums";

type IOTypeMapping = {
  [K in SwOSIOType]: 'input' | 'output' | 'special';
};

export const IOTypeClasses: IOTypeMapping = {
  [SwOSIOType.SWOSIO_UNDEF]: "special",
  [SwOSIOType.SWOSIO_DIGITAL]: "input",
  [SwOSIOType.SWOSIO_SWITCH]: "input",
  [SwOSIOType.SWOSIO_REEDSWITCH]: "input",
  [SwOSIOType.SWOSIO_LIGHTBARRIER]: "input",
  [SwOSIOType.SWOSIO_BUTTON]: "input",
  [SwOSIOType.SWOSIO_ANALOG]: "input",
  [SwOSIOType.SWOSIO_VOLTMETER]: "input",
  [SwOSIOType.SWOSIO_OHMMETER]: "input",
  [SwOSIOType.SWOSIO_THERMOMETER]: "input",
  [SwOSIOType.SWOSIO_LDR]: "input",
  [SwOSIOType.SWOSIO_JOYSTICK]: "input",
  [SwOSIOType.SWOSIO_MOTOR]: "output",
  [SwOSIOType.SWOSIO_XSMOTOR]: "output",
  [SwOSIOType.SWOSIO_XMMOTOR]: "output",
  [SwOSIOType.SWOSIO_TRACTOR]: "output",
  [SwOSIOType.SWOSIO_ENCODER]: "output",
  [SwOSIOType.SWOSIO_LAMP]: "output",
  [SwOSIOType.SWOSIO_VALVE]: "output",
  [SwOSIOType.SWOSIO_COMPRESSOR]: "output",
  [SwOSIOType.SWOSIO_BUZZER]: "output",
  [SwOSIOType.SWOSIO_STEPPER]: "output",
  [SwOSIOType.SWOSIO_COUNTER]: "input",
  [SwOSIOType.SWOSIO_ROTARYENCODER]: "input",
  [SwOSIOType.SWOSIO_FREQUENCYMETER]: "input",
  [SwOSIOType.SWOSIO_LIDAR]: "input",
  [SwOSIOType.SWOSIO_CAM]: "special",
  [SwOSIOType.SWOSIO_SERVO]: "input",
  [SwOSIOType.SWOSIO_PIXEL]: "output",
  [SwOSIOType.SWOSIO_OLED]: "special",
  [SwOSIOType.SWOSIO_I2C]: "special",
  [SwOSIOType.SWOSIO_GYRO]: "input",
  [SwOSIOType.SWOSIO_POWER]: "input",
  [SwOSIOType.SWOSIO_COLORSENSOR]: "input",
  [SwOSIOType.SWOSIO_TRAILSENSOR]: "input",
  [SwOSIOType.SWOSIO_ULTRASONIC]: "input",
  [SwOSIOType.SWOSIO_JOYSTICK_POTI]: "input",
  [SwOSIOType.SWOSIO_WHEELDRIVE]: "output",
  [SwOSIOType.SWOSIO_MINIMOTOR]: "output",
  [SwOSIOType.SWOSIO_SMOTOR]: "output",
  [SwOSIOType.SWOSIO_POWERMOTOR]: "output",
  [SwOSIOType.SWOSIO_MMOTOR]: "output",
  [SwOSIOType.SWOSIO_RCMOTOR]: "output",
  [SwOSIOType.SWOSIO_RCSERVO]: "output",
  [SwOSIOType.SWOSIO_RCPOTI]: "input",
}

export interface ApiGeneralIoType {
  name: string;
  alias?: string;
  IOType: SwOSIOType;
  UIClass: SwOSIOClass;
  icon: string;
  active: boolean;
}

export interface ApiOutputIoType extends ApiGeneralIoType {
  speed: number;
}

export interface ApiServoOutputType extends ApiGeneralIoType {
  offset: number;
  position: number;
}

export interface ApiStepperOutputType extends ApiGeneralIoType {
  speed: number;
  position: number;
  distance: number;
  homing: boolean;
  running: boolean;
}

export interface ApiDigitalInputType extends ApiGeneralIoType {
  value: boolean;
}

export interface ApiAnalogInputType extends ApiGeneralIoType {
  value: string | [string, string];
}

export interface ApiFormattedValueInputType extends ApiGeneralIoType {
  value: string | [string, string];
}

export interface ApiJoystickInputType extends ApiGeneralIoType {
  value: boolean;
  valueLR: number;
  valueFB: number;
}

export interface ApiCamInputType extends ApiGeneralIoType {
  url: string;
  framesize: number;
  quality: number;
  brightnes: number;
  saturation: number;
  "h-Mirror": boolean;
  "v-Flip": boolean;
}

export interface ApiPixelOutputType extends ApiGeneralIoType {
  brightness: number;
  color: string;
}

export interface ApiGyroInputType extends ApiGeneralIoType {
  YawPitchRoll: number[];
}

export type FtSwarmInput =
  | ApiDigitalInputType
  | ApiAnalogInputType
  | ApiFormattedValueInputType
  | ApiJoystickInputType
  | ApiCamInputType
  | ApiGyroInputType;

export type FtSwarmOutput =
  | ApiOutputIoType
  | ApiServoOutputType
  | ApiStepperOutputType
  | ApiPixelOutputType;

export type FtSwarmIo = FtSwarmInput | FtSwarmOutput;

export type ApiController = {
  name: string;
  serialNumber: string;
  CtrlVersion: FtSwarmVersion;
  state: SwOSState;
  io: FtSwarmIo[];
};

export type ApiGetSwarmResponse = {
  name: string;
  kelda: number;
  sync: number;
  controllers: ApiController[];
};
