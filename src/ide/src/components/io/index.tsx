import type { Accessor, Component } from "solid-js";
import { SwOSIOType } from "../../api/generated/genApiEnums";
import type { ApiController, ApiGeneralIoType } from "../../api/apiTypes";
import { IoFrame } from "./IoFrame";
import {
  type RegistryKey,
  registryKeyOf,
} from "../../api/om/optimisticRegistry.ts";
import { DigitalInput } from "./DigitalInput.tsx";
import { JoystickInput } from "./Joystick.tsx";
import { MotorOutput } from "./MotorOutput.tsx";
import type { Sequence, SequencedDatum } from "../../api/om";
import { AnalogInput } from "./AnalogInput.tsx";
import { FormattedValueInput } from "./FormattedValueInput.tsx";
import { BinaryOutput } from "./BinaryOutput.tsx";
import { ServoOutput } from "./ServoOutput.tsx";
import { StepperOutput } from "./StepperOutput.tsx";
import { PixelOutput } from "./PixelOutput.tsx";
import { GyroInput } from "./GyroInput.tsx";

export type IoCardProps<Io extends ApiGeneralIoType = ApiGeneralIoType> = {
  io: Io;
  controller: ApiController;
  seq: Accessor<Sequence>;
};
export type IoCardRendererComponent<
  Io extends ApiGeneralIoType = ApiGeneralIoType,
> = Component<IoCardProps<Io>>;

export function toSequencedDatum<T>(
  props: IoCardProps,
  value: T,
): SequencedDatum<T> {
  return {
    seq: props.seq(),
    data: value,
  };
}

export function sequencedDatumFactory<T>(
  props: IoCardProps,
  accessor: (props: IoCardProps) => T,
): () => SequencedDatum<T> {
  return () => toSequencedDatum(props, accessor(props));
}

export function registryKeyOfProps<T>(props: IoCardProps, id: T): RegistryKey {
  return registryKeyOf(props.controller, props.io, id);
}

const Unimplemented: IoCardRendererComponent = (_props) => {
  return (
    <span class="text-red-200 bg-red-600/10 rounded-full text-xs uppercase px-2 py-1 border border-red-600/75">
      Not yet implemented
    </span>
  );
};

const componentMapper: Record<SwOSIOType, IoCardRendererComponent<any>> = {
  [SwOSIOType.SWOSIO_UNDEF]: Unimplemented,
  [SwOSIOType.SWOSIO_DIGITAL]: DigitalInput,
  [SwOSIOType.SWOSIO_SWITCH]: DigitalInput,
  [SwOSIOType.SWOSIO_REEDSWITCH]: DigitalInput,
  [SwOSIOType.SWOSIO_LIGHTBARRIER]: DigitalInput,
  [SwOSIOType.SWOSIO_BUTTON]: DigitalInput,
  [SwOSIOType.SWOSIO_ANALOG]: AnalogInput,
  [SwOSIOType.SWOSIO_VOLTMETER]: AnalogInput,
  [SwOSIOType.SWOSIO_OHMMETER]: AnalogInput,
  [SwOSIOType.SWOSIO_THERMOMETER]: AnalogInput,
  [SwOSIOType.SWOSIO_LDR]: AnalogInput,
  [SwOSIOType.SWOSIO_JOYSTICK]: JoystickInput,
  [SwOSIOType.SWOSIO_MOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_XSMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_XMMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_TRACTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_ENCODER]: MotorOutput,
  [SwOSIOType.SWOSIO_LAMP]: BinaryOutput,
  [SwOSIOType.SWOSIO_VALVE]: BinaryOutput,
  [SwOSIOType.SWOSIO_COMPRESSOR]: BinaryOutput,
  [SwOSIOType.SWOSIO_BUZZER]: BinaryOutput,
  [SwOSIOType.SWOSIO_STEPPER]: StepperOutput,
  [SwOSIOType.SWOSIO_COUNTER]: FormattedValueInput,
  [SwOSIOType.SWOSIO_ROTARYENCODER]: FormattedValueInput,
  [SwOSIOType.SWOSIO_FREQUENCYMETER]: FormattedValueInput,
  [SwOSIOType.SWOSIO_LIDAR]: FormattedValueInput,
  [SwOSIOType.SWOSIO_CAM]: Unimplemented,
  [SwOSIOType.SWOSIO_SERVO]: ServoOutput,
  [SwOSIOType.SWOSIO_PIXEL]: PixelOutput,
  [SwOSIOType.SWOSIO_OLED]: Unimplemented,
  [SwOSIOType.SWOSIO_I2C]: Unimplemented,
  [SwOSIOType.SWOSIO_GYRO]: GyroInput,
  [SwOSIOType.SWOSIO_POWER]: Unimplemented,
  [SwOSIOType.SWOSIO_COLORSENSOR]: Unimplemented,
  [SwOSIOType.SWOSIO_TRAILSENSOR]: Unimplemented,
  [SwOSIOType.SWOSIO_ULTRASONIC]: FormattedValueInput,
  [SwOSIOType.SWOSIO_JOYSTICK_POTI]: AnalogInput,
  [SwOSIOType.SWOSIO_WHEELDRIVE]: MotorOutput,
  [SwOSIOType.SWOSIO_MINIMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_SMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_POWERMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_MMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_RCMOTOR]: MotorOutput,
  [SwOSIOType.SWOSIO_RCSERVO]: ServoOutput,
  [SwOSIOType.SWOSIO_RCPOTI]: AnalogInput,
};

export const IoCard: Component<IoCardProps> = (props) => (
  <IoFrame {...props}>{componentMapper[props.io.IOType](props)}</IoFrame>
);
