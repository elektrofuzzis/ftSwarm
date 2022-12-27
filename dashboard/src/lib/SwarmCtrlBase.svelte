<script lang="ts">
    import type {BaseIo} from "../swarm";

    export let io: BaseIo;
    export let descriptor: string;
    export let colspan: number;
</script>

<div class="card colspan-when-large-enough" style={colspan > 1 ? `grid-column: span ${colspan}` : ''}>
    <div class="card__inner">
        <div class="card__left">
            <img alt="type" src={"/assets/" + io.icon}>

            <div class="card__infos">
                <span class="muted">{descriptor}</span>
                {io.name}
                <span class="muted boo">{io.id}</span>
            </div>
        </div>
        <div class="card__centergroup">
            <slot/>
        </div>
    </div>
</div>

<style lang="postcss">
  .card {
    width: 100%;
    height: 96px;
    background: var(--background-card);
    border-radius: 4px;
  }

  @media (max-width: 768px) {
    .colspan-when-large-enough {
      grid-column: span 1 !important;
    }
  }

  .card__inner {
    padding: 16px;

    display: flex;
    flex-direction: row;
    justify-content: space-between;
    align-items: center;
  }

  .card__left {
    display: flex;
    flex-direction: row;
    align-items: center;
  }

  .card__infos {
    margin-left: 16px;

    display: flex;
    flex-direction: column;
  }

  .muted {
    color: var(--color-text-muted);
    font-size: 0.8em;
  }

  .boo {
    opacity: 0;
    transition: opacity 0.2s;
  }

  .card:hover .boo {
    opacity: 1;
  }

  .card__left img {
    width: 64px;
    height: 64px;
  }
</style>