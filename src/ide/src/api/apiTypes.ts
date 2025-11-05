export interface ApiGeneralIoType {
  name: string;
  type: number;
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

export enum ApiCpuVersion {
  FtSwarmV1_0 = 0, // development only
  FtSwarmControlV1_3 = 1,
  FtSwarmV1_15 = 2, // production version with JST connectors
  FtSwarmRSV2_0_Dev = 3, // development only
  FtSwarmRSV2_0_Prod = 4, // production version
  FtSwarmCAM = 5,
  FtSwarmDuino = 6,
  FtSwarmPwdDrive = 7,
  FtSwarmXL = 8,
  FtSwarmRC = 9,
  FtSwarmControlV2_x = 10, // reserved
}

export enum ControllerState {
  Offline = 0,
  Booting = 1, // internal state, for documentation only
  StartingWifi = 2, // internal state, for documentation only
  Running = 3,
  Error = 4,
  WaitingForHardware = 5, // other controllers
  Identify = 6,
  FatalError = 7, // swarm has stopped operation
}

export type ApiController = {
  name: string;
  serialNumber: string;
  type: ApiCpuVersion;
  state: ControllerState;
  io: (ApiGeneralIoType & any)[];
};

export type ApiGetSwarmResponse = {
  name: string;
  kelda: boolean;
  controllers: ApiController[];
};
