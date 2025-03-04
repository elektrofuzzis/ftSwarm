<script lang="ts">
    import {currentSwarm, swarmApiData} from "../stores";
    import SwarmInput from "./inputs/SwarmInput.svelte";
    import SwarmBtn from "./inputs/SwarmBtn.svelte";
    import SwarmJoystick from "./inputs/SwarmJoystick.svelte";
    import SwarmActor from "./inputs/SwarmActor.svelte";
    import SwarmServo from "./inputs/SwarmServo.svelte";
    import SwarmLED from "./inputs/SwarmLED.svelte";
    import SwarmCounter from "./inputs/SwarmCounter.svelte";
    import {FtSwarmIOType} from "../api/registries";
    import SwarmGyro from "./inputs/SwarmGyro.svelte";

    const currentSwarmIo = $derived.by(() => {
        if ($currentSwarm === -1) return $swarmApiData.swarms.flatMap((swarm) => swarm.io).filter(it => it.active)
        return $swarmApiData.swarms[$currentSwarm].io
    })
</script>

<div class="container">
    <span class="label">Input</span>
    <div class="grid">
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.DIGITALINPUT || io.type === FtSwarmIOType.ANALOGINPUT) as io}
            <SwarmInput input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.COUNTERINPUT) as io}
            <SwarmCounter input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.BUTTON) as io}
            <SwarmBtn input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.JOYSTICK) as io}
            <SwarmJoystick input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.GYRO) as io}
            <SwarmGyro input={io}/>
        {/each}
    </div>
    <span class="label">Output</span>
    <div class="grid">
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.ACTOR) as io}
            <SwarmActor input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.SERVO) as io}
            <SwarmServo input={io}/>
        {/each}
        {#each currentSwarmIo.filter((io) => io.type === FtSwarmIOType.PIXEL) as io}
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