import {useOMContext, useTransportContext} from "../contexts/transport/context.ts";
import Save from "lucide-solid/icons/save";
import {transactMessage} from "../api/transport";
import {Show} from "solid-js";

export const SaveFab = () => {
    const transport = useTransportContext()
    const om = useOMContext()

    const onSave = async () => {
        await transactMessage(transport, 'swarm.save(0)')
        om.markSaved()
    }

    return (
        <Show when={om.needsSave}>
            <button
                onClick={onSave}
                class="fixed bottom-8 right-8 w-14 h-14 bg-thm-primary text-white rounded-full shadow-2xl flex items-center justify-center hover:scale-110 active:scale-95 transition-all z-50 group"
                title="Save changes to NVS"
            >
                <Save class="w-6 h-6" />
                <span class="absolute right-full mr-4 px-2 py-1 bg-zinc-800 text-xs rounded opacity-0 group-hover:opacity-100 transition-opacity whitespace-nowrap pointer-events-none border border-zinc-700">
                    Save to NVS
                </span>
            </button>
        </Show>
    )
}
