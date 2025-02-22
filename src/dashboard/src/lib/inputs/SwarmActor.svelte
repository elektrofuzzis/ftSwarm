<script lang="ts">
    import type {ActorIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";
    import Slider from "../utils/Slider.svelte";
    import {ftSwarm} from "../../api/FtSwarm";
    import {swarmApiData} from "../../stores";
    import {ACTOR_NAMES} from "../../api/registries.js";

    interface Props {
        input: ActorIo;
    }

    let {input = $bindable()}: Props = $props();
    let disabled: boolean = $derived(!$swarmApiData.auth.status);
    let bounds = $derived(input.highResolution ? 4095 : 255)
</script>

<SwarmCtrlBase colspan={2} descriptor={ACTOR_NAMES[input.subType]} io={input}>
    <div class="container">
        <select bind:value={input.motiontype} {disabled} onchange={(_) => {
            ftSwarm.debouncedUpdateMotor(input.id, input.motiontype, input.speed);
        }}>
            <option value={0}>COAST</option>
            <option value={1}>BRAKE</option>
            <option value={2}>RUN</option>
        </select>
        <Slider max={bounds} min={-bounds} bind:value={input.speed} oninput={(_) => {
            ftSwarm.debouncedUpdateMotor(input.id, input.motiontype, input.speed
);
        }}/>
    </div>
</SwarmCtrlBase>

<style>
    .container {
        display: flex;
        flex-direction: row;
        align-items: center;
        justify-content: center;
        flex-wrap: wrap;
        gap: 8px;
    }

    select {
        border: none;
        background: none;
        font-size: 1em;
        text-align: center;
        color: var(--color-text);
    }

    option {
        color: initial;
    }
</style>