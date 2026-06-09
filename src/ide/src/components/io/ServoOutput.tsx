import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiServoOutputType} from "../../api/apiTypes.ts";
import {useOMContext, useTransportContext} from "../../contexts/transport/context.ts";
import {registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import {apiNameOf, rpcResponseToSeq} from "../../api/util.ts";
import {useLoginContext} from "../../contexts/LoginContext.tsx";
import {Loader} from "./Loader.tsx";
import {SwOSIOType} from "../../api/generated/genApiEnums.ts";
import {transactMessage} from "../../api/transport";

export const ServoOutput: IoCardRendererComponent<ApiServoOutputType> = (props) => {
    const transport = useTransportContext()
    const login = useLoginContext()
    const om = useOMContext()

    const max = () => props.io.IOType === SwOSIOType.SWOSIO_SERVO ? 255 : 90 // RCSERVO_RESOLUTION is 90

    const [optimisticPosition, setOptimisticPosition, _, {
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
        },
        { needsSave: false }
    )

    const [optimisticOffset, setOptimisticOffset, __, {
        setEditing: setOffsetEditing,
        isMutating: isOffsetMutating,
        isThrottled: isOffsetThrottled,
        isEditing: isOffsetEditing
    }] = om.useBoundStore(
        registryKeyOfProps(props, "offset"),
        sequencedDatumFactory(props, (_) => props.io.offset),
        async (newOffset) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setOffset(${newOffset})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(result)
        },
        { needsSave: false }
    )

    return (
        <div class="space-y-3 pt-2">
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
                type="range"
                min="0"
                max={max()}
                value={optimisticPosition()}
                onInput={(e) => {
                    setPositionEditing(true)
                    setOptimisticPosition(parseInt(e.target.value))
                }}
                onChange={() => setPositionEditing(false)}
                disabled={login.interactiveDisabled()}
                class={`w-full h-2 bg-zinc-700 rounded-lg appearance-none cursor-pointer accent-thm-primary ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : ""}`}
            />

            <div class="flex justify-between text-sm">
                <div class="flex items-center gap-2">
                    <span class="text-zinc-400">Offset</span>
                    <Loader
                        isMutating={isOffsetMutating()}
                        isThrottled={isOffsetThrottled()}
                        isEditing={isOffsetEditing()}
                    />
                </div>
                <span class="text-zinc-100">{optimisticOffset()}</span>
            </div>
            <input
                type="range"
                min="0"
                max={max()}
                value={optimisticOffset()}
                onInput={(e) => {
                    setOffsetEditing(true)
                    setOptimisticOffset(parseInt(e.target.value))
                }}
                onChange={() => setOffsetEditing(false)}
                disabled={login.interactiveDisabled()}
                class={`w-full h-2 bg-zinc-700 rounded-lg appearance-none cursor-pointer accent-thm-primary ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : ""}`}
            />
        </div>
    )
}
