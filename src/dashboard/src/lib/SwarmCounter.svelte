<script lang="ts">
    import type {CounterIo, InputIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";

    export let input: CounterIo;

    let unit = input.icon === "28_frequency.svg" ? "Hz" : "";
</script>

<SwarmCtrlBase io={input} descriptor={input.subType} colspan={1}>
    {input.value}{unit}
</SwarmCtrlBase>