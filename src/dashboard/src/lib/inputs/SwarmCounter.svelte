<script lang="ts">
    import type {CounterIo, InputIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";

    interface Props {
        input: CounterIo;
    }

    let { input }: Props = $props();

    let unit = input.icon === "28_frequency.svg" ? "Hz" : "";
</script>

<SwarmCtrlBase io={input} descriptor={input.subType} colspan={1}>
    {input.value}{unit}
</SwarmCtrlBase>