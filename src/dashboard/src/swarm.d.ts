import {FtSwarmActor, type FtSwarmIOType, FtSwarmSensor} from "./api/registries";

export type BaseIo = {
    name: string;
    id: string;
    type: FtSwarmIOType;
    icon: number;
    active: number;
}

export type InputIo = BaseIo & {
    type: FtSwarmIOType.ANALOGINPUT | FtSwarmIOType.DIGITALINPUT;
    sensorType: number;
    subType: FtSwarmSensor;
    value: string;
}

export type CounterIo = BaseIo & {
    type: FtSwarmIOType.COUNTERINPUT;
    sensorType: number;
    subType: string;
    value: string;
}

export type ButtonIo = BaseIo & {
    type: FtSwarmIOType.BUTTON;
    state: number;
}

export type JoystickIo = BaseIo & {
    type: FtSwarmIOType.JOYSTICK;
    valueLr: number;
    valueFb: number;
    button: number;
}

export type GyroIo = BaseIo & {
    type: FtSwarmIOType.GYRO;
    Quaternion: string;
    Acceleration: string;
}

export type ActorIo = BaseIo & {
    type: FtSwarmIOType.ACTOR;
    subType: FtSwarmActor.TRACTOR | FtSwarmActor.MOTOR | FtSwarmActor.XMMOTOR | FtSwarmActor.ENCODER | FtSwarmActor.LAMP;
    motiontype: number;
    speed: number;
    highResolution: boolean;
}

export type LedIo = BaseIo & {
    type: FtSwarmIOType.PIXEL;
    brightness: number;
    color: string;
}

export type ServoIo = BaseIo & {
    type: FtSwarmIOType.SERVO;
    offset: number;
    position: number;
}

export type AnyIo = InputIo | ActorIo | LedIo | ServoIo | ButtonIo | JoystickIo | CounterIo | GyroIo;

export type Swarm = {
    name: string;
    id: number;
    serialNumber: string;
    hostInfo: string;
    type: "ftSwarm" | "ftSwarmControl" | "Overview"
    io: AnyIo[];
}

export type GetSwarmResponse = {
    swarms: Swarm[];
    auth: {
        provided: boolean;
        status: boolean;
        kelda: number;
    };
}