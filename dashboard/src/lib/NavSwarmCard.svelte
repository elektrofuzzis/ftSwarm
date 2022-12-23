<script lang="ts">
    import type {Swarm} from "../swarm";
    import {currentSwarm} from "../stores.js";

    export let swarm: Swarm;
</script>

<div class="card" on:click={() => currentSwarm.set(swarm.id)}>
    <div class={"card__image " + (swarm.id === $currentSwarm ? "card__image--active" : "")}>
        {#if swarm.type === 'ftSwarm'}
            <svg width="49" height="48" viewBox="0 0 49 48" fill="none" xmlns="http://www.w3.org/2000/svg">
                <path d="M40 44H8C5.79086 44 4 42.2091 4 40V8C4 5.79086 5.79086 4 8 4H40C42.2091 4 44 5.79086 44 8V40C44 42.2091 42.2091 44 40 44Z"
                      fill="currentColor"/>
                <circle cx="9" cy="9" r="2" fill="#363A4F"/>
                <rect x="8" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="13" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="18" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="23" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="28" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="33" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="38" y="45" width="3" height="3" fill="currentColor"/>
                <rect x="7" width="3" height="3" fill="currentColor"/>
                <rect x="12" width="3" height="3" fill="currentColor"/>
                <rect x="17" width="3" height="3" fill="currentColor"/>
                <rect x="22" width="3" height="3" fill="currentColor"/>
                <rect x="27" width="3" height="3" fill="currentColor"/>
                <rect x="32" width="3" height="3" fill="currentColor"/>
                <rect x="37" width="3" height="3" fill="currentColor"/>
                <rect y="40" width="3" height="3" transform="rotate(-90 0 40)" fill="currentColor"/>
                <rect x="0.00109863" y="35" width="3" height="3" transform="rotate(-90 0.00109863 35)"
                      fill="currentColor"/>
                <rect x="0.00213623" y="30" width="3" height="3" transform="rotate(-90 0.00213623 30)"
                      fill="currentColor"/>
                <rect x="0.00323486" y="25" width="3" height="3" transform="rotate(-90 0.00323486 25)"
                      fill="currentColor"/>
                <rect x="0.00427246" y="20" width="3" height="3" transform="rotate(-90 0.00427246 20)"
                      fill="currentColor"/>
                <rect x="0.00537109" y="15" width="3" height="3" transform="rotate(-90 0.00537109 15)"
                      fill="currentColor"/>
                <rect x="0.00646973" y="10" width="3" height="3" transform="rotate(-90 0.00646973 10)"
                      fill="currentColor"/>
                <rect x="45" y="40" width="3" height="3" transform="rotate(-90 45 40)" fill="currentColor"/>
                <rect x="45.0011" y="35" width="3" height="3" transform="rotate(-90 45.0011 35)" fill="currentColor"/>
                <rect x="45.0021" y="30" width="3" height="3" transform="rotate(-90 45.0021 30)" fill="currentColor"/>
                <rect x="45.0032" y="25" width="3" height="3" transform="rotate(-90 45.0032 25)" fill="currentColor"/>
                <rect x="45.0043" y="20" width="3" height="3" transform="rotate(-90 45.0043 20)" fill="currentColor"/>
                <rect x="45.0054" y="15" width="3" height="3" transform="rotate(-90 45.0054 15)" fill="currentColor"/>
                <rect x="45.0065" y="10" width="3" height="3" transform="rotate(-90 45.0065 10)" fill="currentColor"/>
            </svg>
        {:else}
            <svg width="48" height="48" viewBox="0 0 48 48" fill="none" xmlns="http://www.w3.org/2000/svg">
                <path d="M30.2379 0.000382805C27.1473 -0.0309462 24.373 1.86349 23.6957 4.66111C22.9217 7.85839 25.19 10.9838 28.784 11.6723C32.3777 12.3608 35.8907 10.3429 36.6648 7.14566C37.4387 3.94859 35.1705 0.823199 31.5768 0.134711C31.1275 0.0486637 30.6795 0.00488904 30.2381 0.000382805H30.2379ZM25.7101 12.6479L21.7234 29.1167H27.3984L31.1334 13.6872C30.2188 13.7769 29.274 13.7416 28.327 13.5602C27.3798 13.3788 26.5023 13.0648 25.7101 12.648V12.6479ZM17.0636 31.048L15.8576 34.2667H32.1424L30.9364 31.048H17.0636ZM1.92965 36.198V39.4167H5.54774V36.198H1.92965ZM13.2043 36.198L11.9983 39.4167H36.0017L34.7957 36.198H13.2043ZM42.4523 36.198V39.4167H46.0704V36.198H42.4523ZM0 41.3479V48H48V41.3479H0Z"
                      fill="currentColor"/>
            </svg>
        {/if}
    </div>

    <div class="swarm-name">
        {swarm.name}
        <span class="swarm-type"><br/>SN: {swarm.serialNumber}</span>
    </div>
</div>

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