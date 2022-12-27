import {writable} from "svelte/store";
import type {GetSwarmResponse} from "./swarm";

export const swarmApiData = writable<GetSwarmResponse>({
    auth: {
        provided: false,
        status: false
    },
    swarms: []
});

export const currentSwarm = writable(0);