import {type IoCardRendererComponent, registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import type {ApiOutputIoType} from "../../api/apiTypes.ts";
import {useOMContext, useTransportContext} from "../../contexts/transport/context.ts";
import {transactMessage} from "../../api/transport";
import {apiNameOf, rpcResponseToSeq} from "../../api/util.ts";
import {useLoginContext} from "../../contexts/LoginContext.tsx";

enum MotorOptimisticStores {
    Speed,
    HighResolution,
}

export const MotorOutput: IoCardRendererComponent<ApiOutputIoType> = (props) => {
    const om = useOMContext()
    const transport = useTransportContext()
    const login = useLoginContext()

    const [optimisticSpeed, setOptimisticSpeed, _0, {setEditing: setSpeedEditing}] = om.useBoundStore(
        registryKeyOfProps(props, MotorOptimisticStores.Speed),
        sequencedDatumFactory(props, (_) => props.io.speed),
        async (newSpeed) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setSpeed(${newSpeed})`)
                .then((v) => v.unwrapOr(null))

            return rpcResponseToSeq(result)
        }
    )
    const [optimisticHighResolution, setOptimisticHighResolution, _1, {setEditing: setHighResolutionEditing}] = om.useBoundStore(
        registryKeyOfProps(props, MotorOptimisticStores.HighResolution),
        sequencedDatumFactory(props, (_) => props.io.highResolution),
        async (newHighResolution) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setHighResolution(${newHighResolution})`)
                .then((v) => v.unwrapOr(null))

            return rpcResponseToSeq(result)
        }
    )
    return (
        <div class="space-y-3 pt-2">
            <div class="flex justify-between text-sm">
                <span class="text-zinc-400">Speed</span>
                <span class="text-zinc-100">{props.io.speed}</span>
            </div>
            <input
                type="range"
                min={optimisticHighResolution() ? -4096 : -255}
                max={optimisticHighResolution() ? 4096 : 255}
                value={optimisticSpeed()}
                disabled={login.interactiveDisabled()}
                onInput={(e) => setOptimisticSpeed(Number(e.currentTarget.value))}
                onFocus={() => setSpeedEditing(true)}
                onBlur={() => setSpeedEditing(false)}
                class="w-full accent-thm-primary"
            />
            <div class="flex items-center gap-2 mt-2">
                <input
                    type="checkbox"
                    id={`hr-${props.io.name}`}
                    checked={optimisticHighResolution()}
                    disabled={login.interactiveDisabled()}
                    onInput={(e) => setOptimisticHighResolution(e.currentTarget.checked)}
                    onFocus={() => setHighResolutionEditing(true)}
                    onBlur={() => setHighResolutionEditing(false)}
                    class="accent-thm-primary"
                />
                <label for={`hr-${props.io.name}`} class="text-xs text-zinc-400">High Resolution</label>
            </div>
        </div>
    );
};