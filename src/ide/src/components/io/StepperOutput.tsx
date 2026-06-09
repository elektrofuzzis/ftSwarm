import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiStepperOutputType} from "../../api/apiTypes.ts";
import {useOMContext, useTransportContext} from "../../contexts/transport/context.ts";
import {registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import {apiNameOf, rpcResponseToSeq} from "../../api/util.ts";
import {useLoginContext} from "../../contexts/LoginContext.tsx";
import {Loader} from "./Loader.tsx";
import {Show} from "solid-js";
import {transactMessage} from "../../api/transport";

export const StepperOutput: IoCardRendererComponent<ApiStepperOutputType> = (props) => {
    const transport = useTransportContext()
    const login = useLoginContext()
    const om = useOMContext()

    const [optimisticSpeed, setOptimisticSpeed, _, {
        setEditing: setSpeedEditing,
        isMutating: isSpeedMutating,
        isThrottled: isSpeedThrottled,
        isEditing: isSpeedEditing
    }] = om.useBoundStore(
        registryKeyOfProps(props, "speed"),
        sequencedDatumFactory(props, (_) => props.io.speed),
        async (newSpeed) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setSpeed(${newSpeed})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(result)
        }
    )

    const [optimisticPosition, setOptimisticPosition, __, {
        setEditing: setPositionEditing,
        isMutating: isPositionMutating,
        isThrottled: isPositionThrottled,
        isEditing: isPositionEditing
    }] = om.useBoundStore(
        registryKeyOfProps(props, "position"),
        sequencedDatumFactory(props, (_) => props.io.position),
        async (newPosition) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setPosition(${newPosition})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(result)
        }
    )

    return (
        <div class="space-y-4 pt-2">
            <div class="space-y-2">
                <div class="flex justify-between text-sm">
                    <div class="flex items-center gap-2">
                        <span class="text-zinc-400">Speed</span>
                        <Loader
                            isMutating={isSpeedMutating()}
                            isThrottled={isSpeedThrottled()}
                            isEditing={isSpeedEditing()}
                        />
                    </div>
                    <span class="text-zinc-100">{optimisticSpeed()}</span>
                </div>
                <input
                    type="range"
                    min="0"
                    max="10240"
                    value={optimisticSpeed()}
                    onInput={(e) => {
                        setSpeedEditing(true)
                        setOptimisticSpeed(parseInt(e.target.value))
                    }}
                    onChange={() => setSpeedEditing(false)}
                    disabled={login.interactiveDisabled()}
                    class={`w-full h-2 bg-zinc-700 rounded-lg appearance-none cursor-pointer accent-thm-primary ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : ""}`}
                />
            </div>

            <div class="space-y-2">
                <div class="flex justify-between text-sm">
                    <div class="flex items-center gap-2">
                        <span class="text-zinc-400">Position</span>
                        <Loader
                            isMutating={isPositionMutating()}
                            isThrottled={isPositionThrottled()}
                            isEditing={isPositionEditing()}
                        />
                    </div>
                    <span class="text-zinc-100">{optimisticPosition()}</span>
                </div>
                <input
                    type="number"
                    value={optimisticPosition()}
                    onInput={(e) => {
                        setPositionEditing(true)
                        setOptimisticPosition(parseInt(e.target.value))
                    }}
                    onChange={() => setPositionEditing(false)}
                    disabled={login.interactiveDisabled()}
                    class="w-full px-2 py-1 bg-zinc-800 border border-zinc-700 rounded text-sm text-zinc-100 focus:outline-none focus:border-thm-primary transition-colors"
                />
            </div>

            <div class="grid grid-cols-2 gap-2">
                <div class="bg-zinc-800/50 p-2 rounded border border-zinc-700/50">
                    <div class="text-[10px] text-zinc-500 uppercase font-bold mb-1">Distance</div>
                    <div class="text-sm font-mono text-zinc-200">{props.io.distance}</div>
                </div>
                <div class="flex flex-col gap-1">
                    <Show when={props.io.homing}>
                        <div class="bg-thm-primary/10 text-thm-primary text-[10px] font-bold px-2 py-1 rounded border border-thm-primary/20 text-center animate-pulse">
                            HOMING
                        </div>
                    </Show>
                    <Show when={props.io.running}>
                        <div class="bg-emerald-500/10 text-emerald-500 text-[10px] font-bold px-2 py-1 rounded border border-emerald-500/20 text-center">
                            RUNNING
                        </div>
                    </Show>
                </div>
            </div>
        </div>
    )
}
