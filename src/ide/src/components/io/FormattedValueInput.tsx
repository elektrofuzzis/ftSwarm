import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiFormattedValueInputType } from "../../api/apiTypes.ts";

export const FormattedValueInput: IoCardRendererComponent<
  ApiFormattedValueInputType
> = (props) => {
  return (
    <div class="flex items-center justify-between pt-2">
      <span class="text-sm text-zinc-400">Value</span>
      <div
        class={`bg-thm-primary/20 text-thm-primary border-thm-primary/30 flex items-baseline gap-1 rounded-full border px-4 py-1 text-xs font-bold tracking-wide`}
      >
        <span>
          {Array.isArray(props.io.value) ? props.io.value[0] : props.io.value}
        </span>
        {Array.isArray(props.io.value) && (
          <span class="text-[10px] font-medium opacity-70">
            {props.io.value[1]}
          </span>
        )}
      </div>
    </div>
  );
};
