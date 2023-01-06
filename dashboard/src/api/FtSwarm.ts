import {isLoggedIn, swarmApiData} from "../stores";
import {nextToken} from "./auth";
import Swal from "sweetalert2";

const SWARM_API_BASE = window.location.origin === 'http://127.0.0.1:5173' ? 'http://127.0.0.1:5000/api/' : window.location.origin + '/api/'
let mouseDown = false;
let lastRequest = 0;

window.addEventListener('mousedown', () => {
    return mouseDown = true;
});
window.addEventListener('mouseup', () => {
    return mouseDown = false;
});

function debounce(func, wait, immediate = false) {
    let timeout;
    let lastCall = 0;
    return function (...args) {
        const context = this;
        const later = function () {
            timeout = null;
            if (!immediate) func.apply(context, args);
        };
        const callNow = immediate && !timeout;
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
        const now = Date.now();
        if (now - lastCall < wait) {
            return;
        }
        lastCall = now;
        if (callNow) func.apply(context, args);
    }
}

class FtSwarm {
    private readonly _apiBase: string
    private isLoaded = false
    private interval: number | undefined

    constructor(apiBase: string = SWARM_API_BASE) {
        this._apiBase = apiBase

        console.info('FtSwarm: Using API base', this._apiBase)
    }

    async fetch(url: string, options: RequestInit = {}) {
        const token = localStorage.getItem('token')
        const headers = new Headers(options.headers)

        if (token) {
            headers.append('Authorization', `Bearer ${token}`)
        }
        try {

            return fetch(this._apiBase + url, {
                ...options,
                headers,
            })
        } catch (e) {
            await Swal.fire({
                title: 'Error',
                text: 'Could not connect to the Swarm API',
                icon: 'error',
                confirmButtonText: 'Ok'
            })
            throw e
        }
    }

    async request() {
        if (mouseDown) {
            await new Promise<void>(resolve => {
                let interval = setInterval(() => {
                    if (!mouseDown) {
                        clearInterval(interval)
                        resolve()
                    }
                }, 250)
            })
        }

        if (Date.now() - lastRequest < 500) {
            return
        }

        const req = await this.fetch('getSwarm')
        const res = await req.json()
        swarmApiData.set(res)
        lastRequest = Date.now()
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

    async obtainAccessToken() {
        const req = await this.fetch('getToken')
        const num = Number((await req.json())["token"])
        localStorage.token = num
        return num
    }

    async fetchWithAuth(url: string, options: RequestInit = {}) {
        let state = false

        isLoggedIn.update((v) => {
            state = v
            return v
        })

        if (!state) {
            await Swal.fire({
                title: 'Error',
                text: 'You are not logged in',
                icon: 'error',
                toast: true,
                position: 'top-end',
                timer: 3000,
                timerProgressBar: true,
                showConfirmButton: false,
                showCancelButton: false
            })
            throw new Error('Not logged in')
        }

        await this.performTokenMod()
        return this.fetch(url, options)
    }

    performTokenMod() {
        const token: number = parseInt(String(localStorage.getItem('token')), 10);
        const pin: number = parseInt(String(localStorage.getItem('pin')), 10);
        console.log("FtSwarm: TokenMod", token, pin);
        const newToken = nextToken(token, pin);
        console.log("FtSwarm: New Token", newToken);
        localStorage.setItem('token', newToken.toString());
    }

    async login(pin: number) {
        await this.obtainAccessToken()
        localStorage.setItem('pin', pin.toString());
    }

    async updateLed(id: string, color: string, brightness: number) {
        console.log("FtSwarm: updateLed", id, color, brightness)
        await this.fetchWithAuth(`led`, {
            method: 'POST',
            body: JSON.stringify({
                id,
                color,
                brightness,
            }),
        })
    }

    async updateServo(id: string, position: number) {
        console.log("FtSwarm: updateServo", id, position)
        await this.fetchWithAuth(`servo`, {
            method: 'POST',
            body: JSON.stringify({
                id,
                position,
            }),
        })
    }

    async updateMotor(id: string, cmd: number, power: number) {
        console.log("FtSwarm: updateMotor", id, cmd, power)
        await this.fetchWithAuth(`actor`, {
            method: 'POST',
            body: JSON.stringify({
                id,
                cmd,
                power,
            }),
        })
    }

    public debouncedUpdateLed = debounce(this.updateLed, 250)
    public debouncedUpdateServo = debounce(this.updateServo, 250)
    public debouncedUpdateMotor = debounce(this.updateMotor, 250)
}

export const ftSwarm = new FtSwarm(SWARM_API_BASE)