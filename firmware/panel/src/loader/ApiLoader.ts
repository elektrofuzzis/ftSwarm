import tokenSwap from './NotSecureSecurityModule';

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