import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiJoystickInputType} from "../../api/apiTypes.ts";

export const JoystickInput: IoCardRendererComponent<ApiJoystickInputType> = (props) => {
    return <div class="space-y-3 pt-2">
        <div class="flex items-center gap-2">
            <div class={`w-3 h-3 rounded-full ${props.io.value ? 'bg-emerald-500' : 'bg-zinc-700'}`}></div>
            <span class="text-xs text-zinc-400">Button State</span>
        </div>
        <div class="flex gap-4">
            <div class="flex-1 space-y-3">
                <div class="space-y-1">
                    <div class="flex justify-between text-xs">
                        <span class="text-zinc-400">L/R Axis</span>
                        <span class="text-zinc-100">{props.io.valueLR}</span>
                    </div>
                    <div class="w-full bg-zinc-800 h-2 rounded-full overflow-hidden">
                        <div
                            class="bg-thm-primary h-full transition-all"
                            style={{width: `${((props.io.valueLR + 100) / 200) * 100}%`}}
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
                            class="bg-thm-primary h-full transition-all"
                            style={{width: `${((props.io.valueFB + 100) / 200) * 100}%`}}
                        ></div>
                    </div>
                </div>
            </div>

            <div class="w-16 h-16 bg-zinc-800 rounded-lg relative flex-shrink-0 border border-zinc-700">
                <div class="absolute inset-0 flex items-center justify-center opacity-20">
                    <div class="w-full h-px bg-zinc-400"></div>
                    <div class="h-full w-px bg-zinc-400 absolute"></div>
                </div>
                <div
                    class={`absolute w-4 h-4 rounded-full shadow-lg transition-all ${props.io.value ? 'bg-emerald-500 scale-110' : 'bg-thm-primary'}`}
                    style={{
                        left: `${((props.io.valueLR + 100) / 200) * 100}%`,
                        top: `${((100 - props.io.valueFB) / 200) * 100}%`,
                        transform: 'translate(-50%, -50%)'
                    }}
                ></div>
            </div>
        </div>
    </div>
}
