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
    <div class="flex h-4 min-w-4 items-center gap-1">
      <Show when={props.isMutating}>
        <RefreshCw size={12} class="text-thm-primary animate-spin" />
      </Show>
      <Show when={!props.isMutating && props.isThrottled}>
        <Clock size={12} class="text-thm-font-muted" />
      </Show>
      <Show when={!props.isMutating && !props.isThrottled && props.isEditing}>
        <div
          class="bg-thm-primary h-1.5 w-1.5 animate-pulse rounded-full"
          title="Editing..."
        />
      </Show>
    </div>
  );
};
