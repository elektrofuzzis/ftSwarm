<script lang="ts">
    import type {InputIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";
    import {SENSOR_NAMES} from "../../api/registries.js";

    interface Props {
        input: InputIo;
    }

    let { input }: Props = $props();
</script>

<SwarmCtrlBase io={input} descriptor={SENSOR_NAMES[input.subType]}>
    {input.value}
</SwarmCtrlBase>