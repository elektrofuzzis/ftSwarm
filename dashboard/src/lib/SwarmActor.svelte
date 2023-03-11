<script lang="ts">
    import type {ActorIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";
    import Slider from "./Slider.svelte";
    import {ftSwarm} from "../api/FtSwarm";
    import {swarmApiData} from "../stores.ts";

    export let input: ActorIo;
    let disabled: boolean;
    $: disabled = !$swarmApiData.auth.status;
</script>

<SwarmCtrlBase colspan={2} descriptor={input.subType} io={input}>
    <div class="container">
        <select bind:value={input.motiontype} {disabled} on:change={(_) => {
            ftSwarm.debouncedUpdateMotor(input.id, input.motiontype, input.power);
        }}>
            <option value={0}>COAST</option>
            <option value={1}>BRAKE</option>
            <option value={2}>RUN</option>
        </select>
        <Slider max={255} min={-255} bind:value={input.power} oninput={(_) => {
            ftSwarm.debouncedUpdateMotor(input.id, input.motiontype, input.power);
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
</style>