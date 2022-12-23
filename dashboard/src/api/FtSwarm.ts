import {swarmApiData} from "../stores";

const SWARM_API_BASE = window.location.origin === 'http://127.0.0.1:5173' ? 'http://127.0.0.1:5000/api/' : window.location.origin + '/api/'

class FtSwarm {
    private readonly _apiBase: string
    private isLoaded = false
    private interval: number | undefined

    constructor(apiBase: string = SWARM_API_BASE) {
        this._apiBase = apiBase

        console.info('FtSwarm: Using API base', this._apiBase)
    }

    async request() {
        const req = await fetch(this._apiBase + 'getSwarm')
        const res = await req.json()
        swarmApiData.set(res)
    }

    async load() {
        if (this.isLoaded) return
        console.info('FtSwarm: Loading')
        await this.request()
        await new Promise(resolve => setTimeout(resolve, 250))
        console.info('FtSwarm: Loaded initial data')

        this.isLoaded = true
        this.interval = window.setInterval(() => this.request(), 1000)
    }

    async stop() {
        if (!this.isLoaded) return
        console.info('FtSwarm: Stopping')
        window.clearInterval(this.interval)
        this.isLoaded = false
    }
}

export const ftSwarm = new FtSwarm(SWARM_API_BASE)