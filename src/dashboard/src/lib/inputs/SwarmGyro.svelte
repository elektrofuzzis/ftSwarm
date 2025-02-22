<script lang="ts">
    import type {GyroIo} from "../../swarm";
    import SwarmCtrlBase from "../utils/SwarmCtrlBase.svelte";

    interface Props {
        input: GyroIo;
    }

    let {input}: Props = $props();

    function toDegrees(angle: number) {
        return angle * 180 / Math.PI;
    }

    let eulerAngles = $derived.by(() => {
        const [q0, q1, q2, q3] = input.Quaternion.split(" ").map(parseFloat)
        let roll = Math.atan2(
            2 * ((q2 * q3) + (q0 * q1)),
            q0 ** 2 - q1 ** 2 - q2 ** 2 + q3 ** 2
        )
        let pitch = Math.asin(2 * ((q1 * q3) - (q0 * q2)))
        let yaw = Math.atan2(
            2 * ((q1 * q2) + (q0 * q3)),
            q0 ** 2 + q1 ** 2 - q2 ** 2 - q3 ** 2
        )
        return [toDegrees(yaw), toDegrees(roll), toDegrees(pitch)]
    })
</script>

<SwarmCtrlBase colspan={2} descriptor="Joystick" io={input}>
    <div class="inner">
        <span>Y, P, R: {#each eulerAngles as angle}{angle.toFixed(0) + "° "}{/each} <br /> A: <span>{input.Acceleration}</span></span>
    </div>
</SwarmCtrlBase>

<style>
    .inner {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 16px;
    }
</style>
