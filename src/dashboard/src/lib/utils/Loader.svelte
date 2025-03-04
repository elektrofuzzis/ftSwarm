<div class="loader">
    <div class="circle"></div>
</div>

<style lang="postcss">
  .loader {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: var(--background-selection);
    z-index: 9999;
    display: flex;
    justify-content: center;
    align-items: center;
  }

  .circle {
    width: 50px;
    height: 50px;
    border-radius: 50%;
    border: 5px solid var(--background-selection);
    border-top-color: var(--color-primary);
    animation: spin 1s linear infinite;
  }

  @keyframes spin {
    to {
      transform: rotate(360deg);
    }
  }
</style>