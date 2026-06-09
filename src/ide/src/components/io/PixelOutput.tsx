import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiPixelOutputType} from "../../api/apiTypes.ts";
import {useOMContext, useTransportContext} from "../../contexts/transport/context.ts";
import {registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import {apiNameOf, rpcResponseToSeq} from "../../api/util.ts";
import {useLoginContext} from "../../contexts/LoginContext.tsx";
import {Loader} from "./Loader.tsx";
import {transactMessage} from "../../api/transport";

export const PixelOutput: IoCardRendererComponent<ApiPixelOutputType> = (props) => {
    const transport = useTransportContext()
    const login = useLoginContext()
    const om = useOMContext()

    const [optimisticBrightness, setOptimisticBrightness, _, {
        setEditing: setBrightnessEditing,
        isMutating: isBrightnessMutating,
        isThrottled: isBrightnessThrottled,
        isEditing: isBrightnessEditing
    }] = om.useBoundStore(
        registryKeyOfProps(props, "brightness"),
        sequencedDatumFactory(props, (_) => props.io.brightness),
        async (newBrightness) => {
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setBrightness(${newBrightness})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(result)
        }
    )

    const [optimisticColor, setOptimisticColor, __, {
        setEditing: setColorEditing,
        isMutating: isColorMutating,
        isThrottled: isColorThrottled,
        isEditing: isColorEditing
    }] = om.useBoundStore(
        registryKeyOfProps(props, "color"),
        sequencedDatumFactory(props, (_) => props.io.color),
        async (newColor) => {
            // newColor is hex string like "RRGGBB"
            const r = parseInt(newColor.substring(0, 2), 16)
            const g = parseInt(newColor.substring(2, 4), 16)
            const b = parseInt(newColor.substring(4, 6), 16)
            const result = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setColor(${r},${g},${b})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(result)
        }
    )

    return (
        <div class="space-y-3 pt-2">
            <div class="flex justify-between text-sm">
                <div class="flex items-center gap-2">
                    <span class="text-zinc-400">Brightness</span>
                    <Loader
                        isMutating={isBrightnessMutating()}
                        isThrottled={isBrightnessThrottled()}
                        isEditing={isBrightnessEditing()}
                    />
                </div>
                <span class="text-zinc-100">{optimisticBrightness()}</span>
            </div>
            <input
                type="range"
                min="0"
                max="255"
                value={optimisticBrightness()}
                onInput={(e) => {
                    setBrightnessEditing(true)
                    setOptimisticBrightness(parseInt(e.target.value))
                }}
                onChange={() => setBrightnessEditing(false)}
                disabled={login.interactiveDisabled()}
                class={`w-full h-2 bg-zinc-700 rounded-lg appearance-none cursor-pointer accent-thm-primary ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : ""}`}
            />

            <div class="flex justify-between text-sm">
                <div class="flex items-center gap-2">
                    <span class="text-zinc-400">Color</span>
                    <Loader
                        isMutating={isColorMutating()}
                        isThrottled={isColorThrottled()}
                        isEditing={isColorEditing()}
                    />
                </div>
                <div class="flex items-center gap-2">
                    <span class="text-xs font-mono text-zinc-500">#{optimisticColor()}</span>
                    <input
                        type="color"
                        value={`#${optimisticColor()}`}
                        onInput={(e) => {
                            setColorEditing(true)
                            setOptimisticColor(e.target.value.substring(1).toUpperCase())
                        }}
                        onChange={() => setColorEditing(false)}
                        disabled={login.interactiveDisabled()}
                        class="w-6 h-6 rounded border-0 bg-transparent cursor-pointer disabled:cursor-not-allowed"
                    />
                </div>
            </div>
        </div>
    )
}
