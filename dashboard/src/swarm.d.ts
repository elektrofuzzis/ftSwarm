export type BaseIo = {
    name: string;
    id: string;
    type: string;
    icon: string;
}

export type InputIo = BaseIo & {
    type: "INPUT";
    sensorType: number;
    subType: number;
}

export type ActorIo = BaseIo & {
    type: "ACTOR";
    subType: "TRACTORMOTOR" | "XMOTOR" | "XMMOTOR" | "ENCODERMOTOR" | "LAMP"
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
    io: [InputIo | ActorIo | LedIo | ServoIo];
}

export type GetSwarmResponse = {
    swarms: Swarm[];
    auth: {
        provided: boolean;
        status: boolean;
    };
}