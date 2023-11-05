export type BaseIo = {
    name: string;
    id: string;
    type: string;
    icon: string;
}

export type InputIo = BaseIo & {
    type: "ANALOGINPUT" | "DIGITALINPUT";
    sensorType: number;
    subType: string;
    value: string;
}

export type CounterIo = BaseIo & {
    type: "COUNTER";
    sensorType: number;
    subType: string;
    value: string;
}

export type ButtonIo = BaseIo & {
    type: "BUTTON";
    state: number;
}

export type JoystickIo = BaseIo & {
    type: "JOYSTICK";
    valueLr: number;
    valueFb: number;
    button: number;
}

export type ActorIo = BaseIo & {
    type: "ACTOR";
    subType: "TRACTORMOTOR" | "XMOTOR" | "XMMOTOR" | "ENCODERMOTOR" | "LAMP";
    motiontype: number;
    speed: number;
}

export type LedIo = BaseIo & {
    type: "LED";
    brightness: number;
    color: string;
}

export type ServoIo = BaseIo & {
    type: "SERVO";
    offset: number;
    position: number;
}

export type Swarm = {
    name: string;
    id: number;
    serialNumber: string;
    hostInfo: string;
    type: "ftSwarm" | "ftSwarmControl"
    io: [InputIo | ActorIo | LedIo | ServoIo | ButtonIo | JoystickIo | CounterIo];
}

export type GetSwarmResponse = {
    swarms: Swarm[];
    auth: {
        provided: boolean;
        status: boolean;
    };
}