<script lang="ts">
    import Navigation from "./lib/Navigation.svelte";
    import Loader from "./lib/Loader.svelte";
    import {ftSwarm} from "./api/FtSwarm";
    import {onDestroy, onMount} from "svelte";

    let swarmLoadingPromise = new Promise(() => {});

    onMount(() => {
        swarmLoadingPromise = ftSwarm.load();
    });

    onDestroy(() => {
        ftSwarm.stop();
    });
</script>

{#await swarmLoadingPromise}
    <Loader />
{:then swarm}
    <Navigation>
        INPUTS
    </Navigation>
{/await}

<style>
</style>