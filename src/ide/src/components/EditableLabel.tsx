import Edit from "lucide-solid/icons/edit";
import Check from "lucide-solid/icons/check";
import {
  Show,
  type ParentComponent,
  createSignal,
  createEffect,
} from "solid-js";

export const EditableLabel: ParentComponent<{
  allowEdit?: boolean;
  onEdit?: (newValue: string) => void;
  onValidate?: (value: string) => string | null;
  isEditing?: boolean;
  setEditing?: (editing: boolean) => void;
}> = (props) => {
  const [localEditing, setLocalEditing] = createSignal(false);
  const isEditing = () => props.isEditing ?? localEditing();
  const setEditing = (val: boolean) =>
    props.setEditing ? props.setEditing(val) : setLocalEditing(val);

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
    <div class="group relative inline-flex min-h-[1.5em] items-center gap-2">
      <Show
        when={isEditing()}
        fallback={
          <div
            class="flex cursor-pointer items-center gap-2"
            onClick={() => props.allowEdit !== false && setEditing(true)}
          >
            <span>{props.children}</span>
            <Show when={props.allowEdit ?? true}>
              <Edit
                size={18}
                class="text-thm-font-muted opacity-0 transition-colors group-hover:text-white group-hover:opacity-100"
              />
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
            class="bg-thm-surface-3 border-thm-primary text-thm-font focus:ring-thm-primary min-w-[150px] rounded border px-2 py-0.5 focus:ring-1 focus:outline-none"
          />
          <button
            onClick={handleSave}
            class="hover:bg-thm-surface-border-2 rounded p-1 text-emerald-500 transition-colors"
          >
            <Check size={18} />
          </button>
          <button
            onClick={handleCancel}
            class="hover:bg-thm-surface-border-2 text-thm-error rounded p-1 transition-colors"
          >
            <span class="flex size-5 items-center justify-center">&times;</span>
          </button>
        </div>
      </Show>
    </div>
  );
};
