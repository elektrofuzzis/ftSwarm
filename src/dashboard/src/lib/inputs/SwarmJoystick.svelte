<script lang="ts">
    import type {JoystickIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";
    import {Spring} from "svelte/motion";

    interface Props {
        input: JoystickIo;
    }

    let {input}: Props = $props();
    let lr = Spring.of(() => input.valueLr)
    let fb = Spring.of(() => input.valueFb)
</script>

<SwarmCtrlBase colspan={2} descriptor="Joystick" io={input}>
    <div class="inner">
        <span>X: <span class="w-3chars">{lr.current.toFixed(0)}</span>% Y: <span class="w-3chars">{fb.current.toFixed(0)}</span>%</span>

        <div class="joystick-simulator">
            <div class="joystick-simulator__stick"
                 class:swarm-ctrl__value--on={input.button}
                 class:swarm-ctrl__value--off={!input.button}
                 style="left: {50+lr.current/2}%; top: {50+fb.current/2}%"></div>
        </div>
    </div>
</SwarmCtrlBase>

<style>
    .inner {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 16px;
    }

    .w-3chars {
        width: 3ch;
        display: inline-block;
        text-align: right;
    }

    .joystick-simulator {
        position: relative;
        width: 64px;
        height: 64px;
        border-radius: 50%;
        background: var(--background-selection);
    }

    .joystick-simulator__stick {
        position: absolute;
        width: 20px;
        height: 20px;
        border-radius: 50%;
        transform: translate(-50%, -50%);
    }

    .swarm-ctrl__value--on {
        background-color: var(--color-primary);
    }

    .swarm-ctrl__value--off {
        background-color: var(--color-red);
    }
</style>
