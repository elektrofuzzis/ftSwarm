<script lang="ts">
    import type {LedIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";
    import Slider from "./Slider.svelte";
    import {onDestroy, onMount} from "svelte";
    import {ftSwarm} from "../api/FtSwarm";
    import {swarmApiData} from "../stores.ts";

    export let input: LedIo;
    let inputRef;
    let timeout: NodeJS.Timeout;
    let disabled: boolean;
    $: disabled = !$swarmApiData.auth.status;

    onMount(() => {
        clearTimeout(timeout);
        timeout = setInterval(() => {
            inputRef.value = input.color;
        }, 100);
    });

    onDestroy(() => {
        clearTimeout(timeout);
    });
</script>

<SwarmCtrlBase colspan={2} descriptor="LED" io={input}>
    <div class="container">
        <input bind:this={inputRef} type="color" {disabled} on:input={() => {
            ftSwarm.debouncedUpdateLed(input.id, inputRef.value, input.brightness);
        }}/>
        <Slider bind:value={input.brightness} max={255} min={0} oninput={() => {
            ftSwarm.debouncedUpdateLed(input.id, inputRef.value, input.brightness);
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


  input[type="color"] {
    border: 3px solid var(--background-selection);
    border-radius: 4px;
    background: var(--background-selection);
    font-size: 1em;
    text-align: center;
    color: var(--color-text);
  }
</style>