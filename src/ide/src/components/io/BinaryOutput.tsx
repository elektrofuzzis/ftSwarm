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
        }
    )

    const isOn = () => optimisticSpeed() !== 0

    const toggle = () => {
        if (login.interactiveDisabled()) return
        setSpeedEditing(true)
        setOptimisticSpeed(isOn() ? 0 : 100)
        setSpeedEditing(false)
    }

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
            <button
                onClick={toggle}
                disabled={login.interactiveDisabled()}
                class={`px-4 py-1 rounded-full text-xs font-bold tracking-wide border transition-colors ${
                    isOn()
                        ? "bg-thm-primary/20 text-thm-primary border-thm-primary/30"
                        : "bg-zinc-800 text-zinc-500 border-zinc-700"
                } ${login.interactiveDisabled() ? "opacity-50 cursor-not-allowed" : "hover:brightness-110 cursor-pointer"}`}
            >
                {isOn() ? "ON" : "OFF"}
            </button>
        </div>
    )
}
