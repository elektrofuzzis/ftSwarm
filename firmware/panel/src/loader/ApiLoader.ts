export async function obtainAccessToken() {
    const response = await fetch(`/api/token`, {
        method: 'GET',
    });
    return Number(await response.text());
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