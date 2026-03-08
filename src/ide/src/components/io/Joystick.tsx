import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiJoystickInputType} from "../../api/apiTypes.ts";

export const JoystickInput: IoCardRendererComponent<ApiJoystickInputType> = (props) => {
    return <div class="space-y-3 pt-2">
        <div class="flex items-center gap-2">
            <div class={`w-3 h-3 rounded-full ${props.io.value ? 'bg-emerald-500' : 'bg-zinc-700'}`}></div>
            <span class="text-xs text-zinc-400">Button State</span>
        </div>
        <div class="space-y-1">
            <div class="flex justify-between text-xs">
                <span class="text-zinc-400">L/R Axis</span>
                <span class="text-zinc-100">{props.io.valueLR}</span>
            </div>
            <div class="w-full bg-zinc-800 h-2 rounded-full overflow-hidden">
                <div
                    class="bg-indigo-500 h-full"
                    style={{width: `${((props.io.valueLR + 255) / 510) * 100}%`}}
                ></div>
            </div>
        </div>
        <div class="space-y-1">
            <div class="flex justify-between text-xs">
                <span class="text-zinc-400">F/B Axis</span>
                <span class="text-zinc-100">{props.io.valueFB}</span>
            </div>
            <div class="w-full bg-zinc-800 h-2 rounded-full overflow-hidden">
                <div
                    class="bg-indigo-500 h-full"
                    style={{width: `${((props.io.valueFB + 255) / 510) * 100}%`}}
                ></div>
            </div>
        </div>
    </div>
}