import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiPowerInputType } from "../../api/apiTypes.ts";

export const PowerInput: IoCardRendererComponent<ApiPowerInputType> = (
  props,
) => {
  const isBad = () => props.io.value[0] < 4.6;
  const bgColors = () =>
    isBad()
      ? "bg-thm-error/20 text-thm-error border-thm-error/30"
      : "bg-thm-primary/20 text-thm-primary border-thm-primary/30";
  return (
    <div class="flex items-center justify-between pt-2">
      <span class="text-sm text-zinc-400">Value</span>
      <div
        class={`${bgColors()} flex items-baseline gap-1 rounded-full border px-4 py-1 text-xs font-bold tracking-wide`}
      >
        <span>{props.io.value[0]}</span>
        <span class="text-[10px] font-medium opacity-70">
          {props.io.value[1]}
        </span>
      </div>
    </div>
  );
};
