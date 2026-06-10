import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiOutputIoType} from "../../api/apiTypes.ts";
import {useOMContext, useTransportContext} from "../../contexts/transport/context.ts";
import {registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import {apiNameOf, rpcResponseToSeq} from "../../api/util.ts";
import {useLoginContext} from "../../contexts/LoginContext.tsx";
import {Loader} from "./Loader.tsx";
import {transactMessage} from "../../api/transport";

export const BinaryOutput: IoCardRendererComponent<ApiOutputIoType> = (props) => {
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
            let res = await transactMessage(transport, `${apiNameOf(props.io, props.controller)}.setSpeed(${newSpeed})`)
                .then((v) => v.unwrapOr(null))
            return rpcResponseToSeq(res)
        },
        { needsSave: false }
    )

    const isOn = () => optimisticSpeed() !== 0

    return (
        <div class="flex items-center justify-between pt-2">
            <div class="flex items-center gap-2">
                <span class="text-sm text-zinc-400">State</span>
                <Loader
                    isMutating={isSpeedMutating()}
                    isThrottled={isSpeedThrottled()}
                    isEditing={isSpeedEditing()}
                />
            </div>
            <div class="flex p-0.5 bg-zinc-800 rounded-lg border border-zinc-700">
                <button
                    onClick={() => {
                        if (login.interactiveDisabled()) return
                        setSpeedEditing(true)
                        setOptimisticSpeed(0)
                        setSpeedEditing(false)
                    }}
                    disabled={login.interactiveDisabled()}
                    class={`px-3 py-1 rounded-md text-[10px] font-bold transition-all ${
                        !isOn()
                            ? "bg-zinc-700 text-zinc-200 shadow-sm"
                            : "text-zinc-500 hover:text-zinc-400"
                    } ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : "cursor-pointer"}`}
                >
                    OFF
                </button>
                <button
                    onClick={() => {
                        if (login.interactiveDisabled()) return
                        setSpeedEditing(true)
                        setOptimisticSpeed(100)
                        setSpeedEditing(false)
                    }}
                    disabled={login.interactiveDisabled()}
                    class={`px-3 py-1 rounded-md text-[10px] font-bold transition-all ${
                        isOn()
                            ? "bg-thm-primary text-white shadow-sm"
                            : "text-zinc-500 hover:text-zinc-400"
                    } ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : "cursor-pointer"}`}
                >
                    ON
                </button>
            </div>
        </div>
    )
}
