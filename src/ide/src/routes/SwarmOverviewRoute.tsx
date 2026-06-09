import {createMemo, For, type Component, Show} from "solid-js";
import {useOMContext} from "../contexts/transport/context.ts";
import {IoCard} from "../components/io";
import {type ApiController, type FtSwarmIo} from "../api/apiTypes.ts";
import {getControllerIcon} from "../api/icons.ts";
import {Dynamic} from "solid-js/web";
import {SwOSState} from "../api/generated/genApiEnums.ts";

const state2Bg: Record<SwOSState, string> = {
    [SwOSState.OFFLINE]: "bg-thm-error",
    [SwOSState.BOOTING]: "bg-thm-primary",
    [SwOSState.STARTWIFI]: "bg-thm-primary",
    [SwOSState.RUNNING]: "bg-thm-ok",
    [SwOSState.ERROR]: "bg-thm-error",
    [SwOSState.WAITING]: "bg-thm-primary",
    [SwOSState.IDENTIFY]: "bg-thm-primary",
    [SwOSState.FATAL]: "bg-thm-error",
    [SwOSState.FACTORY1]: "bg-thm-error",
    [SwOSState.FACTORY2]: "bg-thm-error",
};

export const SwarmOverviewRoute: Component = () => {
    const om = useOMContext();

    const controllers = createMemo(() => Object.values(om.controllers));

    const activeIos = createMemo(() => {
        const result: { io: FtSwarmIo; controller: ApiController }[] = [];

        for (const controller of controllers()) {
            for (const io of controller.io) {
                if (io.active) {
                    result.push({io, controller});
                }
            }
        }

        return result;
    });

    return (

        <div class="p-4 w-full">
            <div class="flex items-center gap-2 mb-1">
                <h2 class="text-thm-font-muted tracking-tight pb-2">My Swarm</h2>
            </div>

            <Show
                when={activeIos().length > 0}
                fallback={
                    <div class="flex flex-col gap-8">
                        <div class="p-8 text-center max-w-2xl mx-auto">
                            <div
                                class="bg-thm-surface-3 w-16 h-16 rounded-full flex items-center justify-center mx-auto mb-4 border border-thm-surface-border-2">
                                <div class="text-thm-primary text-2xl">★</div>
                            </div>
                            <h3 class="text-xl font-semibold text-thm-font mb-2">No Active IOs</h3>
                            <p class="text-thm-font-muted mb-6">
                                You haven't marked any IOs as active yet. Go to a controller's detail page and
                                activate some IOs to see them here for quick monitoring.
                            </p>
                        </div>
                    </div>
                }
            >
                <div class="w-full grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-4">
                    <For each={activeIos()}>
                        {({io, controller}) => (
                            <IoCard io={io} controller={controller} seq={om.lastSequence}/>
                        )}
                    </For>
                </div>
            </Show>
            <div>
                <h2 class="text-thm-font-muted tracking-tight pb-2 mt-8">Controller Overview</h2>
                <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
                    <For each={controllers()}>
                        {(controller) => (
                            <a
                                href={`/controller/${controller.serialNumber}`}
                                class="bg-thm-surface-2 border border-thm-surface-border-2 rounded-lg p-4 hover:border-thm-primary/50 transition-all group"
                            >
                                <div class="flex items-center gap-4">
                                    <div
                                        class="p-3 bg-thm-surface-3 rounded-lg border border-thm-surface-border-2 group-hover:border-thm-primary/30 transition-colors">
                                        <Dynamic component={getControllerIcon(controller.CtrlVersion)}
                                                 class="w-6 h-6 text-thm-font-muted group-hover:text-thm-primary transition-colors"/>
                                    </div>
                                    <div class="flex-1 min-w-0">
                                        <div class="flex items-center justify-between gap-2">
                                            <div class="flex items-center gap-2 min-w-0">
                                                <h4 class="font-bold text-thm-font truncate">{controller.name}</h4>
                                                <Show when={om.isKelda() && controllers().indexOf(controller) === 0}>
                                                    <span class="px-1.5 py-0.5 rounded bg-thm-primary/10 border border-thm-primary/20 text-[10px] font-bold text-thm-primary leading-none uppercase tracking-wider">Kelda</span>
                                                </Show>
                                            </div>
                                            <div
                                                class={`w-2 h-2 rounded-full ${state2Bg[controller.state] || 'bg-thm-surface-border-2'}`}/>
                                        </div>
                                        <p class="text-[10px] text-thm-font-muted font-mono uppercase tracking-wider">SN: {controller.serialNumber}</p>
                                    </div>
                                </div>
                                <div
                                    class="mt-4 pt-3 border-t border-thm-surface-border-2/50 flex items-center justify-between text-xs text-thm-font-muted">
                                    <span>{controller.io.length} IO Ports</span>
                                    <span
                                        class="text-thm-primary opacity-0 group-hover:opacity-100 transition-opacity">View Detail →</span>
                                </div>
                            </a>
                        )}
                    </For>
                </div>
            </div>
        </div>
    );
};
