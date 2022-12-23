<script lang="ts">
    import type {JoystickIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";

    export let input: JoystickIo;
</script>

<SwarmCtrlBase colspan={2} descriptor="Joystick" io={input}>
    <div class="inner">
        <span>X: <span class="w-3chars">{input.valueLr}</span>% Y: <span class="w-3chars">{input.valueFb}</span>%</span>

        <div class="joystick-simulator">
            {#if input.button}
                <div class="joystick-simulator__stick swarm-ctrl__value--on" style="left: {input.valueLr}%; top: {100-input.valueFb}%"></div>
            {:else}
                <div class="joystick-simulator__stick swarm-ctrl__value--off" style="left: {input.valueLr}%; top: {100-input.valueFb}%"></div>
            {/if}
        </div>

    </div>
</SwarmCtrlBase>

<style>
  .inner {
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 16px;
  }

  .w-3chars {
    width: 3ch;
    display: inline-block;
    text-align: right;
  }

  .joystick-simulator {
    position: relative;
    width: 64px;
    height: 64px;
    border-radius: 50%;
    background: var(--background-selection);
  }

  .joystick-simulator__stick {
    position: absolute;
    width: 20px;
    height: 20px;
    border-radius: 50%;
    transform: translate(-50%, -50%);
  }

  .swarm-ctrl__value--on {
    background-color: var(--color-primary);
  }

  .swarm-ctrl__value--off {
    background-color: var(--color-red);
  }
</style>
