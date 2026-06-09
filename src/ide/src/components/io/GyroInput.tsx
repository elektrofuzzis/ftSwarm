import type {IoCardRendererComponent} from "./index.tsx";
import type {ApiGyroInputType} from "../../api/apiTypes.ts";

export const GyroInput: IoCardRendererComponent<ApiGyroInputType> = (props) => {
    const toDegrees = (rad: number) => Math.round(rad * 180 / Math.PI)

    return (
        <div class="pt-2 space-y-2">
            <div class="grid grid-cols-3 gap-2">
                <div class="bg-zinc-800/50 p-2 rounded border border-zinc-700/50 text-center">
                    <div class="text-[10px] text-zinc-500 uppercase font-bold mb-1">Yaw</div>
                    <div class="text-sm font-mono text-zinc-200">{toDegrees(props.io.YawPitchRoll[0])}°</div>
                </div>
                <div class="bg-zinc-800/50 p-2 rounded border border-zinc-700/50 text-center">
                    <div class="text-[10px] text-zinc-500 uppercase font-bold mb-1">Pitch</div>
                    <div class="text-sm font-mono text-zinc-200">{toDegrees(props.io.YawPitchRoll[1])}°</div>
                </div>
                <div class="bg-zinc-800/50 p-2 rounded border border-zinc-700/50 text-center">
                    <div class="text-[10px] text-zinc-500 uppercase font-bold mb-1">Roll</div>
                    <div class="text-sm font-mono text-zinc-200">{toDegrees(props.io.YawPitchRoll[2])}°</div>
                </div>
            </div>
            <div class="text-[10px] text-zinc-600 font-mono text-right">
                {props.io.YawPitchRoll[0].toFixed(3)}, {props.io.YawPitchRoll[1].toFixed(3)}, {props.io.YawPitchRoll[2].toFixed(3)} rad
            </div>
        </div>
    )
}
