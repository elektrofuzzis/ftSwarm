import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiOutputIoType} from "../../api/apiTypes.ts";

export const MotorOutput: IoCardRendererComponent<ApiOutputIoType> = (props) => (
    <div class="space-y-3 pt-2">
        <div class="flex justify-between text-sm">
            <span class="text-zinc-400">Speed</span>
            <span class="text-zinc-100">{props.io.speed}</span>
        </div>
        <input
            type="range"
            min={props.io.highResolution ? -4096 : -255}
            max={props.io.highResolution ? 4096 : 255}
            value={props.io.speed}
            class="w-full accent-thm-primary"
        />
        <div class="flex items-center gap-2 mt-2">
            <input type="checkbox" id={`hr-${props.io.name}`} checked={props.io.highResolution}
                   class="accent-thm-primary"/>
            <label for={`hr-${props.io.name}`} class="text-xs text-zinc-400">High Resolution</label>
        </div>
    </div>
);