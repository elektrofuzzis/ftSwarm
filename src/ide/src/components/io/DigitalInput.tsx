import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiDigitalInputType } from "../../api/apiTypes.ts";

export const DigitalInput: IoCardRendererComponent<ApiDigitalInputType> = (
  props,
) => {
  return (
    <div class="flex items-center justify-between pt-2">
      <span class="text-sm text-zinc-400">State</span>
      <div
        class={`px-4 py-1 rounded-full text-xs font-bold tracking-wide border ${props.io.value ? "bg-thm-ok/20 text-thm-ok border-thm-ok/30" : "bg-zinc-800/50 text-zinc-500 border-zinc-700/50"}`}
      >
        {props.io.value ? "HIGH" : "LOW"}
      </div>
    </div>
  );
};
