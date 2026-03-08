import Edit from "lucide-solid/icons/edit";
import { Show, type ParentComponent } from "solid-js";

export const EditableLabel: ParentComponent<{
  allowEdit?: boolean;
  onEdit?: (newValue: string) => void;
  onValidate?: (value: string) => string | null;
}> = (props) => {
  return (
    <div class="inline-flex gap-2 items-center group">
      <span>{props.children}</span>
      <Show when={props.allowEdit ?? true}>
        <Edit class="text-thm-font-muted group-hover:text-white transition-colors" />
      </Show>
    </div>
  );
};
