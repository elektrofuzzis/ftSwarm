<script lang="ts">
    import Navigation from "./lib/nav/Navigation.svelte";
    import Loader from "./lib/utils/Loader.svelte";
    import {ftSwarm} from "./api/FtSwarm";
    import {onDestroy, onMount} from "svelte";
    import Swarm from "./lib/Swarm.svelte";
    import Login from "./lib/utils/Login.svelte";
    import {swarmApiData} from "./stores";

    let swarmLoadingPromise = $state(new Promise(() => {
    }));

    onMount(() => {
        swarmLoadingPromise = ftSwarm.load();
    });

    onDestroy(() => {
        ftSwarm.stop();
    });
</script>

{#await swarmLoadingPromise}
    <Loader/>
{:then _}
    <Navigation>
        <Swarm/>
        {#if $swarmApiData.auth.kelda}
            <Login/>
        {/if}
        <p class="center">
            Read the docs at <a
                href="https://elektrofuzzis.github.io/ftSwarm">https://elektrofuzzis.github.io/ftSwarm</a><br/>
            © 2022 Christian Bergschneider & Stefan Fuss
        </p>
    </Navigation>
{/await}

<style>
    .center {
        margin-top: 8em;
        text-align: center;
    }

    .center a {
        color: var(--color-primary);
    }
</style>