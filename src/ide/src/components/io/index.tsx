import type { Component } from "solid-js";
import { SwOSIOType } from "../../api/generated/genApiEnums";
import type {ApiController, ApiGeneralIoType} from "../../api/apiTypes";
import { IoFrame } from "./IoFrame";
import { Dynamic } from "solid-js/web";
import {type RegistryKey, registryKeyOf} from "../../api/om/optimisticRegistry.ts";
import {DigitalInput} from "./DigitalInput.tsx";
import {JoystickInput} from "./Joystick.tsx";
import {MotorOutput} from "./MotorOutput.tsx";

export type IoCardProps<Io extends ApiGeneralIoType = ApiGeneralIoType> = { io: Io, controller: ApiController };
export type IoCardRendererComponent<Io extends ApiGeneralIoType = ApiGeneralIoType> = Component<IoCardProps<Io>>;

export function registryKeyOfProps<T>(props: IoCardProps, id: T): RegistryKey {
    return registryKeyOf(props.controller, props.io, id)
}

const Unimplemented: IoCardRendererComponent = (_props) => {
  return <span class="text-red-200 bg-red-600/10 rounded-full text-xs uppercase px-2 py-1 border border-red-600/75">Not yet implemented</span>
}

const componentMapper: Record<SwOSIOType, IoCardRendererComponent<any>> = {
  [SwOSIOType.SWOSIO_UNDEF]: Unimplemented,
  [SwOSIOType.SWOSIO_DIGITAL]: DigitalInput,
  [SwOSIOType.SWOSIO_SWITCH]: DigitalInput,
  [SwOSIOType.SWOSIO_REEDSWITCH]: DigitalInput,
  [SwOSIOType.SWOSIO_LIGHTBARRIER]: DigitalInput,
  [SwOSIOType.SWOSIO_BUTTON]: DigitalInput,
  [SwOSIOType.SWOSIO_ANALOG]: Unimplemented,
  [SwOSIOType.SWOSIO_VOLTMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_OHMMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_THERMOMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_LDR]: Unimplemented,
  [SwOSIOType.SWOSIO_JOYSTICK]: JoystickInput,
  [SwOSIOType.SWOSIO_MOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_XSMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_XMMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_TRACTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_ENCODER]: Unimplemented,
  [SwOSIOType.SWOSIO_LAMP]: Unimplemented,
  [SwOSIOType.SWOSIO_VALVE]: Unimplemented,
  [SwOSIOType.SWOSIO_COMPRESSOR]: Unimplemented,
  [SwOSIOType.SWOSIO_BUZZER]: Unimplemented,
  [SwOSIOType.SWOSIO_STEPPER]: Unimplemented,
  [SwOSIOType.SWOSIO_COUNTER]: Unimplemented,
  [SwOSIOType.SWOSIO_ROTARYENCODER]: Unimplemented,
  [SwOSIOType.SWOSIO_FREQUENCYMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_LIDAR]: Unimplemented,
  [SwOSIOType.SWOSIO_CAM]: Unimplemented,
  [SwOSIOType.SWOSIO_SERVO]: Unimplemented,
  [SwOSIOType.SWOSIO_PIXEL]: Unimplemented,
  [SwOSIOType.SWOSIO_OLED]: Unimplemented,
  [SwOSIOType.SWOSIO_I2C]: Unimplemented,
  [SwOSIOType.SWOSIO_GYRO]: Unimplemented,
  [SwOSIOType.SWOSIO_POWER]: Unimplemented,
  [SwOSIOType.SWOSIO_COLORSENSOR]: Unimplemented,
  [SwOSIOType.SWOSIO_TRAILSENSOR]: Unimplemented,
  [SwOSIOType.SWOSIO_ULTRASONIC]: Unimplemented,
  [SwOSIOType.SWOSIO_JOYSTICK_POTI]: Unimplemented,
  [SwOSIOType.SWOSIO_WHEELDRIVE]: Unimplemented,
  [SwOSIOType.SWOSIO_MINIMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_SMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_POWERMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_MMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_RCMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_RCSERVO]: Unimplemented,
  [SwOSIOType.SWOSIO_RCPOTI]: Unimplemented,
  [SwOSIOType.SWOSIO_MAXIOTYPE]: Unimplemented
}

export const IoCard: Component<IoCardProps> = (props) => (
  <IoFrame {...props}>
    <Dynamic component={componentMapper[props.io.IOType]} {...props} />
  </IoFrame>
)
