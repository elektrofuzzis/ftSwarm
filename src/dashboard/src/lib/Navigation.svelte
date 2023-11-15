<script>
    import FtSwarmImg from '../assets/ftSwarm.svg'
    import {swarmApiData} from "../stores";
    import NavSwarmCard from "./NavSwarmCard.svelte";
</script>

<nav>
    <img alt="ftSwarm" src={FtSwarmImg}/>

    <div class="swarmies">
        {#each ($swarmApiData).swarms as swarm}
            <NavSwarmCard {swarm}/>
        {/each}
    </div>
</nav>
<main>
    <slot/>
</main>
<style lang="postcss">
  nav {
    background-color: var(--background-nav);

    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    height: 64px;
    padding: 8px 16px;

    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  nav img {
    height: 48px;
  }

  .swarmies {
    display: flex;
    flex-direction: row;
    overflow-x: auto;
    overflow-y: hidden;
    height: 100%;
  }

  main {
    padding: 88px 16px 16px 16px;
  }
</style>