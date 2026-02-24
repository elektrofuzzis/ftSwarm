import { Show, type ParentComponent } from "solid-js";
import type { ApiGeneralIoType } from "../../api/apiTypes";
import { ChevronDown, Link2 } from "lucide-solid";
/*
<div class="p-4 border border-thm-surface-border-2 rounded-lg">
  <h3 class="text-thm-font-muted text-sm">
    {FtSwarmIoTypeNames[value.IOType]}
  </h3>
  <p class="text-thm-font text-lg font-medium">
    {value.name}: {value.value}
  </p>
</div>

 */
export const IoFrame: ParentComponent<{ io: ApiGeneralIoType }> = (props) => {
  return (
    <div class="border bg-thm-surface-2 border-thm-surface-border-2 rounded-lg p-3 flex flex-col transition-all h-fit">
      <div class="flex items-center justify-between mb-1">
        <div class="relative flex items-center justify-between w-full">
          <select
            value={props.io.IOType}
            class="appearance-none bg-transparent text-[10px] w-full font-bold uppercase tracking-wider text-thm-font-muted hover:text-thm-font pr-4 transition-colors outline-none cursor-pointer z-10"
          >
            <option value={0}>Digital Input</option>
            <option value={4}>Button</option>
            <option value={5}>Analog</option>
            <option value={10}>Joystick</option>
            <option value={11}>Motor</option>
            <option value={25}>Camera</option>
            <option value={26}>Servo</option>
            <option value={30}>Gyro</option>
          </select>
          <ChevronDown
            size={12}
            class="absolute right-0 text-thm-font-muted pointer-events-none"
          />
        </div>
        <Show when={false}>
          <div
            class="flex items-center gap-1 px-1 bg-indigo-500/10 text-indigo-400 rounded-full text-[10px] font-medium border border-indigo-500/20"
            title="Used in Event Programming"
          >
            <Link2 size={10} />
            <span class="hidden sm:inline">Event</span>
          </div>
        </Show>
      </div>

      <div class="flex items-center gap-2 mb-2">
        <span class="text-xs font-mono font-bold text-thm-font-muted bg-thm-surface-1 border border-thm-surface-border-2 px-2 py-1 rounded">
          {props.io.name}
        </span>
        <input
          type="text"
          value={"Label Name"}
          class="bg-transparent text-sm font-semibold text-thm-font transition-colors border-b border-transparent hover:border-thm-surface-border-2 focus:border-thm-primary focus:outline-none px-1 w-full"
          placeholder="Name"
        />
      </div>

      <div class="border-t border-zinc-800/50 pt-2">{props.children}</div>
    </div>
  );
};
