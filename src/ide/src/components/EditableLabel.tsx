import Edit from "lucide-solid/icons/edit";
import Check from "lucide-solid/icons/check";
import X from "lucide-solid/icons/x";
import { Show, type ParentComponent, createSignal, createEffect } from "solid-js";

export const EditableLabel: ParentComponent<{
  allowEdit?: boolean;
  onEdit?: (newValue: string) => void;
  onValidate?: (value: string) => string | null;
  isEditing?: boolean;
  setEditing?: (editing: boolean) => void;
}> = (props) => {
  const [localEditing, setLocalEditing] = createSignal(false);
  const isEditing = () => props.isEditing ?? localEditing();
  const setEditing = (val: boolean) => (props.setEditing ? props.setEditing(val) : setLocalEditing(val));

  const [value, setValue] = createSignal("");
  let inputRef: HTMLInputElement | undefined;

  createEffect(() => {
    if (isEditing()) {
      setValue(String(props.children));
      inputRef?.focus();
      inputRef?.select();
    }
  });

  const handleSave = () => {
    const error = props.onValidate?.(value());
    if (error) return;
    props.onEdit?.(value());
    setEditing(false);
  };

  const handleCancel = () => {
    setEditing(false);
  };

  return (
    <div class="inline-flex gap-2 items-center group relative min-h-[1.5em]">
      <Show
        when={isEditing()}
        fallback={
          <div 
            class="flex items-center gap-2 cursor-pointer"
            onClick={() => props.allowEdit !== false && setEditing(true)}
          >
            <span>{props.children}</span>
            <Show when={props.allowEdit ?? true}>
              <Edit size={18} class="text-thm-font-muted group-hover:text-white transition-colors opacity-0 group-hover:opacity-100" />
            </Show>
          </div>
        }
      >
        <div class="flex items-center gap-2">
          <input
            ref={inputRef}
            type="text"
            value={value()}
            onInput={(e) => setValue(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === "Enter") handleSave();
              if (e.key === "Escape") handleCancel();
            }}
            class="bg-thm-surface-3 border border-thm-primary rounded px-2 py-0.5 text-thm-font focus:outline-none focus:ring-1 focus:ring-thm-primary min-w-[150px]"
          />
          <button
            onClick={handleSave}
            class="p-1 hover:bg-thm-surface-border-2 rounded transition-colors text-emerald-500"
          >
            <Check size={18} />
          </button>
          <button
            onClick={handleCancel}
            class="p-1 hover:bg-thm-surface-border-2 rounded transition-colors text-thm-error"
          >
            <X size={18} />
          </button>
        </div>
      </Show>
    </div>
  );
};
