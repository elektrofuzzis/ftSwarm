import { Show } from "solid-js";
import RefreshCw from "lucide-solid/icons/refresh-cw";
import Clock from "lucide-solid/icons/clock";

export interface LoaderProps {
  isMutating: boolean;
  isThrottled: boolean;
  isEditing: boolean;
}

export const Loader = (props: LoaderProps) => {
  return (
    <div class="flex items-center gap-1 h-4 min-w-4">
      <Show when={props.isMutating}>
        <RefreshCw size={12} class="text-thm-primary animate-spin" />
      </Show>
      <Show when={!props.isMutating && props.isThrottled}>
        <Clock size={12} class="text-thm-font-muted" />
      </Show>
      <Show when={!props.isMutating && !props.isThrottled && props.isEditing}>
        <div
          class="w-1.5 h-1.5 rounded-full bg-thm-primary animate-pulse"
          title="Editing..."
        />
      </Show>
    </div>
  );
};
