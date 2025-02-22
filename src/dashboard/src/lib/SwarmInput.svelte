<script lang="ts">
    import type {InputIo} from "../swarm";
    import SwarmCtrlBase from "./SwarmCtrlBase.svelte";

    interface Props {
        input: InputIo;
    }

    let { input }: Props = $props();
</script>

<SwarmCtrlBase io={input} descriptor={input.subType}>
    {input.value}
</SwarmCtrlBase>