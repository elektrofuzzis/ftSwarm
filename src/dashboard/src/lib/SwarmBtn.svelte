<script lang="ts">
    import type {ButtonIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";

    export let input: ButtonIo;
</script>

<SwarmCtrlBase descriptor="BUTTON" io={input} colspan={2}>
    {#if input.state}
        <div class="swarm-ctrl__value swarm-ctrl__value--on">1</div>
    {:else}
        <div class="swarm-ctrl__value swarm-ctrl__value--off">0</div>
    {/if}
</SwarmCtrlBase>

<style>
  .swarm-ctrl__value {
    width: 35px;
    height: 35px;
    border-radius: 50%;

    color: #181926;
    font-size: 14px;
    font-weight: 700;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .swarm-ctrl__value--on {
    background-color: var(--color-primary);
  }

  .swarm-ctrl__value--off {
    background-color: var(--color-red);
  }
</style>
