<script lang="ts">
    import type {ServoIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";
    import Slider from "../utils/Slider.svelte";
    import {ftSwarm} from "../../api/FtSwarm";

  interface Props {
    input: ServoIo;
  }

  let { input = $bindable() }: Props = $props();
</script>

<SwarmCtrlBase colspan={4} descriptor="Servo" io={input}>
    <div class="container">
        <Slider max={255} min={-255} bind:value={input.position} oninput={() => {
            ftSwarm.debouncedUpdateServo(input.id, input.position);
        }}/>
    </div>
</SwarmCtrlBase>

<style>
  .container {
    display: flex;
    flex-direction: row;
    align-items: center;
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