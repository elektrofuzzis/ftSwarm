import tokenSwap from './NotSecureSecurityModule';
import swal from "sweetalert2";

export async function obtainAccessToken() {
    const response = await fetch(`/api/getToken`, {
        method: 'GET',
    });
    return Number((await response.json())["token"]);
}

export function performTokenMod() {
    const token: number = parseInt(String(localStorage.getItem('token')), 10);
    const pin: number = parseInt(String(localStorage.getItem('pin')), 10);
    console.log(token, pin);
    const newToken = tokenSwap(token, pin);
    console.log(newToken);
    localStorage.setItem('token', newToken.toString());
}

export async function isAuthenticated() {
    const response = await fetch(`/api/isAuthenticated`, {
        body: JSON.stringify({
            Token: parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: 'POST',
    });

    return response.ok;
}

export type FtSwarmLED = {
    id: string,
    brightness?: Number,
    color?: Number
}

export function apiLedBrightness(led: FtSwarmLED) {
    performTokenMod()
    fetch("/api/led", {
        body: JSON.stringify({
            id: led.id,
            brightness: parseInt(String(led.brightness), 10),
            token:  parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: "POST"
    }).then(async (response)=> {
        return

    }).catch(async () => {
        swal.fire({
            title: "Error in Communication",
            text: "ftSwarm didn't respond or request was blocked",
            toast: true,
            icon: "error"
        })
    })
}

export function apiLedColor(led: FtSwarmLED) {
    performTokenMod()
    fetch("/api/led", {
        body: JSON.stringify({
            id: led.id,
            color: led.color,
            token:  parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: "POST"
    }).then(async (response)=> {
        return
    }).catch(async () => {
        swal.fire({
            title: "Error in Communication",
            text: "ftSwarm didn't respond or request was blocked",
            toast: true,
            icon: "error"
        })
    })
}

export type FtSwarmServo = {
    id: string,
    position?: Number
}

export function apiServoPosition(servo: FtSwarmServo) {
    performTokenMod()
    fetch("/api/servo", {
        body: JSON.stringify({
            id: servo.id,
            position: servo.position,
            token:  parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: "POST"
    }).then(async (response)=> {
        return
    }).catch(async () => {
        swal.fire({
            title: "Error in Communication",
            text: "ftSwarm didn't respond or request was blocked",
            toast: true,
            icon: "error"
        })
    })
}

export type FtSwarmMotor = {
    id: string,
    cmd?: Number,
    power?: Number
}

export function apiMotorCmd(motor: FtSwarmMotor) {
    performTokenMod()
    fetch("/api/actor", {
        body: JSON.stringify({
            id:    motor.id,
            cmd:   motor.cmd,
            token: parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: "POST"
    }).then(async (response)=> {
        return
    }).catch(async () => {
        swal.fire({
            title: "Error in Communication",
            text: "ftSwarm didn't respond or request was blocked",
            toast: true,
            icon: "error"
        })
    })
}

export function apiMotorPower(motor: FtSwarmMotor) {
    performTokenMod()
    fetch("/api/actor", {
        body: JSON.stringify({
            id:    motor.id,
            power: motor.power,
            token: parseInt(String(localStorage.getItem('token')), 10)
        }),
        method: "POST"
    }).then(async (response)=> {
        return;
    }).catch(async () => {
        swal.fire({
            title: "Error in Communication",
            text: "ftSwarm didn't respond or request was blocked",
            toast: true,
            icon: "error"
        })
    })
}
export type FtSwarmIo = {
    name: string;
    id: string;
    type: string;
    icon: string;
    [key: string]: any;
}

export type FtSwarm = {
    name: string;
    id: number;
    serialNumber: number;
    hostinfo: string;
    type: "ftSwarm" | "ftSwarmControl";
    io: FtSwarmIo[];
}

export async function getSwarm(): Promise<FtSwarm[]> {
    const response = await fetch(`/api/getSwarm`, {
        method: 'GET',
    });
    return await response.json();
}
