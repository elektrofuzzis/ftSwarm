import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiGyroInputType } from "../../api/apiTypes.ts";

export const GyroInput: IoCardRendererComponent<ApiGyroInputType> = (props) => {
  const toDegrees = (rad: number) => Math.round((rad * 180) / Math.PI);

  return (
    <div class="space-y-2 pt-2">
      <div class="grid grid-cols-3 gap-2">
        <div class="rounded border border-zinc-700/50 bg-zinc-800/50 p-2 text-center">
          <div class="mb-1 text-[10px] font-bold text-zinc-500 uppercase">
            Yaw
          </div>
          <div class="font-mono text-sm text-zinc-200">
            {toDegrees(props.io.YawPitchRoll[0])}°
          </div>
        </div>
        <div class="rounded border border-zinc-700/50 bg-zinc-800/50 p-2 text-center">
          <div class="mb-1 text-[10px] font-bold text-zinc-500 uppercase">
            Pitch
          </div>
          <div class="font-mono text-sm text-zinc-200">
            {toDegrees(props.io.YawPitchRoll[1])}°
          </div>
        </div>
        <div class="rounded border border-zinc-700/50 bg-zinc-800/50 p-2 text-center">
          <div class="mb-1 text-[10px] font-bold text-zinc-500 uppercase">
            Roll
          </div>
          <div class="font-mono text-sm text-zinc-200">
            {toDegrees(props.io.YawPitchRoll[2])}°
          </div>
        </div>
      </div>
      <div class="text-right font-mono text-[10px] text-zinc-600">
        {props.io.YawPitchRoll[0].toFixed(3)},{" "}
        {props.io.YawPitchRoll[1].toFixed(3)},{" "}
        {props.io.YawPitchRoll[2].toFixed(3)} rad
      </div>
    </div>
  );
};
