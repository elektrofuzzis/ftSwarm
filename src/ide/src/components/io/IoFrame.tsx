import {Show, type ParentComponent} from "solid-js";
import {type IoCardProps, registryKeyOfProps, sequencedDatumFactory} from "./index.tsx";
import {useOMContext} from "../../contexts/transport/context.ts";
import logger from "../../util/logger.ts";
import {Dynamic} from "solid-js/web";
import {getIoIcon} from "../../api/icons.ts";
import ChevronDown from "lucide-solid/icons/chevron-down";
import Link2 from "lucide-solid/icons/link-2";

enum IOFrameOptimisticStores {
    ALIAS
}

export const IoFrame: ParentComponent<IoCardProps> = (props) => {
    const om = useOMContext()

    const [optimisticAlias, setOptimisticAlias, _, {setEditing: setAliasEditing}] = om.useBoundStore(
        registryKeyOfProps(props, {kind: "Frame", val: IOFrameOptimisticStores.ALIAS}),
        sequencedDatumFactory(props, (_) => props.io.alias ?? props.io.name),
        async (newAlias) => {
            logger.debug(`Setting alias of ${props.io.name} to ${newAlias}`)
            return undefined
        }
    )

    return (
        <div
            class="border bg-thm-surface-2 border-thm-surface-border-2 rounded-lg p-3 flex flex-col transition-all">
            <div class="flex items-center justify-between mb-1">
                <div class="relative flex items-center justify-between w-full">
                    <span
                        class="text-[10px] font-bold uppercase text-thm-primary tracking-wider pr-2">{props.io.name}</span>
                    <select
                        value={props.io.IOType}
                        class="appearance-none bg-transparent text-[10px] w-full font-bold uppercase tracking-wider text-thm-font-muted hover:text-thm-font pr-4 transition-colors outline-none cursor-pointer z-10"
                        disabled={true}
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
                        <Link2 size={10}/>
                        <span class="hidden sm:inline">Event</span>
                    </div>
                </Show>
            </div>

            <div class="flex items-center gap-2 mb-2">
        <span
            class="text-xs font-mono font-bold text-thm-font-muted py-0.5">
            <Dynamic component={getIoIcon(props.io.IOType)} class="w-5 h-5"/>
        </span>
                <input
                    type="text"
                    value={optimisticAlias()}
                    class="bg-transparent text-sm font-semibold text-thm-font transition-colors border-b border-transparent hover:border-thm-surface-border-2 focus:border-thm-primary focus:outline-none px-1 w-full"
                    placeholder="Name"
                    onInput={(e) => setOptimisticAlias(e.currentTarget.value)}
                    onFocus={() => setAliasEditing(true)}
                    onBlur={() => setAliasEditing(false)}
                />
            </div>

            <div class="border-t border-zinc-800/50 pt-2">{props.children}</div>
        </div>
    );
};
