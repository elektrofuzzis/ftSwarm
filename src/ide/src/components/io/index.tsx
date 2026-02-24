import type { Component } from "solid-js";
import { SwOSIOType } from "../../api/generated/genApiEnums";
import type { ApiGeneralIoType } from "../../api/apiTypes";
import { IoFrame } from "./IoFrame";
import { Dynamic } from "solid-js/web";

const Unimplemented: Component = () => {
  return <span class="text-red-200 bg-red-600/10 rounded-full text-xs uppercase px-2 py-1 border border-red-600/75">Not yet implemented</span>
}

const componentMapper: Record<SwOSIOType, Component> = {
  [SwOSIOType.SWOSIO_UNDEF]: Unimplemented,
  [SwOSIOType.SWOSIO_DIGITAL]: Unimplemented,
  [SwOSIOType.SWOSIO_SWITCH]: Unimplemented,
  [SwOSIOType.SWOSIO_REEDSWITCH]: Unimplemented,
  [SwOSIOType.SWOSIO_LIGHTBARRIER]: Unimplemented,
  [SwOSIOType.SWOSIO_BUTTON]: Unimplemented,
  [SwOSIOType.SWOSIO_ANALOG]: Unimplemented,
  [SwOSIOType.SWOSIO_VOLTMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_OHMMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_THERMOMETER]: Unimplemented,
  [SwOSIOType.SWOSIO_LDR]: Unimplemented,
  [SwOSIOType.SWOSIO_JOYSTICK]: Unimplemented,
  [SwOSIOType.SWOSIO_MOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_XSMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_XMMOTOR]: Unimplemented,
  [SwOSIOType.SWOSIO_TRACTOR]: Unimplemented,
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

export const IoCard: Component<{ io: ApiGeneralIoType }> = (props) => (
  <IoFrame io={props.io}>
    <Dynamic component={componentMapper[props.io.IOType]} />
  </IoFrame>
)
