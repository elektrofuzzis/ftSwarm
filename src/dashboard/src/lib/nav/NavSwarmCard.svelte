<script lang="ts">
    import type {Swarm} from "../../swarm";
    import {currentSwarm} from "../../stores";

    interface Props {
        swarm: Swarm;
    }

    let {swarm}: Props = $props();
</script>

<a class="unstyled card" onclick={() => currentSwarm.set(swarm.id)} href="#{swarm.id}">
    <div class={"card__image " + (swarm.id === $currentSwarm ? "card__image--active" : "")}>
        {#if swarm.type === 'ftSwarm'}
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
                <rect width="256" height="256" fill="none"/>
                <rect x="104" y="104" width="48" height="48" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <rect x="48" y="48" width="160" height="160" rx="8" fill="none" stroke="currentColor"
                      stroke-linecap="round" stroke-linejoin="round" stroke-width="16"/>
                <line x1="208" y1="104" x2="232" y2="104" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="208" y1="152" x2="232" y2="152" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="24" y1="104" x2="48" y2="104" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="24" y1="152" x2="48" y2="152" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="152" y1="208" x2="152" y2="232" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="104" y1="208" x2="104" y2="232" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="152" y1="24" x2="152" y2="48" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="104" y1="24" x2="104" y2="48" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
            </svg>
        {:else if swarm.type === 'ftSwarmControl'}
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
                <rect width="256" height="256" fill="none"/>
                <rect x="40" y="152" width="176" height="64" rx="8" fill="none" stroke="currentColor"
                      stroke-linecap="round" stroke-linejoin="round" stroke-width="16"/>
                <line x1="128" y1="152" x2="128" y2="88" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <circle cx="128" cy="56" r="32" fill="none" stroke="currentColor" stroke-linecap="round"
                        stroke-linejoin="round" stroke-width="16"/>
                <line x1="200" y1="120" x2="168" y2="120" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
            </svg>
        {:else}
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256">
                <rect width="256" height="256" fill="none"/>
                <line x1="40" y1="64" x2="216" y2="64" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="40" y1="128" x2="112" y2="128" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <line x1="40" y1="192" x2="128" y2="192" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
                <circle cx="184" cy="144" r="32" fill="none" stroke="currentColor" stroke-linecap="round"
                        stroke-linejoin="round" stroke-width="16"/>
                <line x1="206.63" y1="166.63" x2="232" y2="192" fill="none" stroke="currentColor" stroke-linecap="round"
                      stroke-linejoin="round" stroke-width="16"/>
            </svg>
        {/if}
    </div>

    <div class="swarm-name">
        {swarm.name}
        {#if swarm.type !== "Overview"}
            <span class="swarm-type"><br/>SN: {swarm.serialNumber}</span>
        {/if}
    </div>
</a>

<!--suppress CssUnusedSymbol -->
<style>
    .card {
        border: 1px solid var(--color-secondary);
        border-radius: 4px;
        padding: 8px;
        margin: 0 16px;
        height: 40px;
        width: 150px;

        display: flex;
        flex-direction: row;
        align-items: center;

        cursor: pointer;
    }

    .card__image {
        height: 100%;
        color: var(--color-disabled);
    }

    .card__image--active {
        color: var(--color-primary);
        position: relative;
    }

    .card__image svg {
        height: 100%;
    }

    .swarm-name {
        flex: 1;
        font-size: 16px;
        text-align: center;
        line-height: 16px;
    }

    .swarm-type {
        font-size: 12px;
        color: var(--color-disabled);
    }
</style>