<script lang="ts">
    import {swarmApiData} from "../../stores.ts";

  interface Props {
    min?: number;
    max?: number;
    value?: number;
    oninput: (event: Event) => void;
  }

  let {
    min = 0,
    max = 100,
    value = $bindable(50),
    oninput
  }: Props = $props();

    let disabled: boolean = $derived(!$swarmApiData.auth.status);
    
</script>

<input bind:value max={max} min={min} type="range" {oninput} {disabled} />

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