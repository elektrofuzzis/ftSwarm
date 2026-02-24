import type {
  FtSwarmVersion,
  SwOSIOType,
  SwOSState,
} from "./generated/genApiEnums";

export interface ApiGeneralIoType {
  name: string;
  IOType: SwOSIOType;
  icon: string;
  active: boolean;
}

export interface ApiOutputIoType extends ApiGeneralIoType {
  speed: number;
  highResolution: boolean;
}

export interface ApiServoOutputType extends ApiGeneralIoType {
  offset: number;
  position: number;
}

export interface ApiDigitalInputType extends ApiGeneralIoType {
  value: boolean;
}

export interface ApiAnalogInputType extends ApiGeneralIoType {
  value: number;
}

export interface ApiFormattedValueInputType extends ApiGeneralIoType {
  value: string;
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
  quaternion: number[];
  acceleration: number[];
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
  controllers: ApiController[];
};
