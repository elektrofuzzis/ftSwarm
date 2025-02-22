import {writable} from "svelte/store";
import type {GetSwarmResponse} from "./swarm";

export const swarmApiData = writable<GetSwarmResponse>({
    auth: {
        provided: false,
        status: false,
        kelda: 0
    },
    swarms: []
});

export const sendTimeout = writable<number>(0);

export const isLoggedIn = writable(false);

swarmApiData.subscribe((data) => {
    isLoggedIn.set(!!data.auth.status);
});

export const currentSwarm = writable(0);