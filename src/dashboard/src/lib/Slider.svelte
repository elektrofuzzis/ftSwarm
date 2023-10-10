<script lang="ts">
    import {swarmApiData} from "../stores.ts";

    export let min: number = 0;
    export let max: number = 100;
    export let value: number = 50;
    export let oninput: (event: Event) => void;

    let disabled: boolean;
    $: disabled = !$swarmApiData.auth.status;
</script>

<input bind:value max={max} min={min} type="range" on:input={oninput} {disabled} />

<style>
  input {
    width: 200px;

    /* style the track */
    -webkit-appearance: none;
    appearance: none;
    background-color: var(--background-selection);
    height: 5px;
    border-radius: 5px;
  }

  /* style the thumb */
  input::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 15px;
    height: 15px;
    background-color: var(--color-primary);
    border-radius: 50%;
    cursor: pointer;
  }

    input::-moz-range-thumb {
        width: 15px;
        height: 15px;
        background-color: var(--color-primary);
        border-radius: 50%;
        cursor: pointer;
    }
</style>