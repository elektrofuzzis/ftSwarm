<script lang="ts">
    import type {InputIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";

    export let input: InputIo;
</script>

<SwarmCtrlBase io={input} descriptor={input.subType}>
    {input.value}
</SwarmCtrlBase>