<script lang="ts">
    import {currentSwarm, swarmApiData} from "../stores";
    import SwarmInput from "./SwarmInput.svelte";
    import SwarmBtn from "./SwarmBtn.svelte";
    import SwarmJoystick from "./SwarmJoystick.svelte";
    import SwarmActor from "./SwarmActor.svelte";
    import SwarmServo from "./SwarmServo.svelte";
    import SwarmLED from "./SwarmLED.svelte";
    import SwarmCounter from "./SwarmCounter.svelte";
</script>

<div class="container">
    <span class="label">Input</span>
    <div class="grid">
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "DIGITALINPUT" || io.type === "ANALOGINPUT") as io}
            <SwarmInput input={io}/>
        {/each}
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "COUNTER") as io}
            <SwarmCounter input={io}/>
        {/each}
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "BUTTON") as io}
            <SwarmBtn input={io}/>
        {/each}
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "JOYSTICK") as io}
            <SwarmJoystick input={io}/>
        {/each}
    </div>
    <span class="label">Output</span>
    <div class="grid">
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "ACTOR") as io}
            <SwarmActor input={io}/>
        {/each}
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "SERVO") as io}
            <SwarmServo input={io}/>
        {/each}
        {#each $swarmApiData.swarms[$currentSwarm].io.filter((io) => io.type === "LED") as io}
            <SwarmLED input={io}/>
        {/each}
    </div>
</div>


<style lang="postcss">
  .container {
    width: 100%;
    max-width: 1200px;
    margin: 0 auto;
  }

  .label {
    display: block;
    margin-bottom: 8px;
    margin-top: 16px;
    font-size: 1rem;
    font-weight: 600;
    color: var(--color-text-muted);
    text-transform: uppercase;
  }

  .grid {
    display: grid;
    grid-gap: 16px;
    grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
  }
</style>