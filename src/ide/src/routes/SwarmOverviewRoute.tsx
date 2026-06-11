import { createMemo, For, type Component, Show } from "solid-js";
import { useOMContext } from "../contexts/transport/context.ts";
import { IoCard } from "../components/io";
import { type ApiController, type FtSwarmIo } from "../api/apiTypes.ts";
import { getControllerIcon } from "../api/icons.ts";
import { SwOSState } from "../api/generated/genApiEnums.ts";
import { A } from "../util/router.tsx";

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
          result.push({ io, controller });
        }
      }
    }

    return result;
  });

  return (
    <div class="w-full p-4">
      <div class="mb-1 flex items-center gap-2">
        <h2 class="text-thm-font-muted pb-2 tracking-tight">My Swarm</h2>
      </div>

      <Show
        when={activeIos().length > 0}
        fallback={
          <div class="flex flex-col gap-8">
            <div class="mx-auto max-w-2xl p-8 text-center">
              <div class="bg-thm-surface-3 border-thm-surface-border-2 mx-auto mb-4 flex h-16 w-16 items-center justify-center rounded-full border">
                <div class="text-thm-primary text-2xl">★</div>
              </div>
              <h3 class="text-thm-font mb-2 text-xl font-semibold">
                No Active IOs
              </h3>
              <p class="text-thm-font-muted mb-6">
                You haven't marked any IOs as active yet. Once you've used or
                named some IOs, they'll show up here.
              </p>
            </div>
          </div>
        }
      >
        <div class="grid w-full grid-cols-1 gap-4 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4">
          <For each={activeIos()}>
            {({ io, controller }) => (
              <IoCard io={io} controller={controller} seq={om.lastSequence} />
            )}
          </For>
        </div>
      </Show>
      <div>
        <h2 class="text-thm-font-muted mt-8 pb-2 tracking-tight">
          Controller Overview
        </h2>
        <div class="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <For each={controllers()}>
            {(controller) => (
              <A
                href={`/controller/${controller.serialNumber}`}
                class="bg-thm-surface-2 border-thm-surface-border-2 hover:border-thm-primary/50 group rounded-lg border p-4 transition-all"
              >
                <div class="flex items-center gap-4">
                  <div class="bg-thm-surface-3 border-thm-surface-border-2 group-hover:border-thm-primary/30 rounded-lg border p-3 transition-colors">
                    {getControllerIcon(controller.CtrlVersion)({
                      class:
                        "w-6 h-6 text-thm-font-muted group-hover:text-thm-primary transition-colors",
                    })}
                  </div>
                  <div class="min-w-0 flex-1">
                    <div class="flex items-center justify-between gap-2">
                      <div class="flex min-w-0 items-center gap-2">
                        <h4 class="text-thm-font truncate font-bold">
                          {controller.name}
                        </h4>
                        <Show
                          when={
                            om.isKelda() &&
                            controllers().indexOf(controller) === 0
                          }
                        >
                          <span class="bg-thm-primary/10 border-thm-primary/20 text-thm-primary rounded border px-1.5 py-0.5 text-[10px] leading-none font-bold tracking-wider uppercase">
                            Kelda
                          </span>
                        </Show>
                      </div>
                      <div
                        class={`h-2 w-2 rounded-full ${state2Bg[controller.state] || "bg-thm-surface-border-2"}`}
                      />
                    </div>
                    <p class="text-thm-font-muted font-mono text-[10px] tracking-wider uppercase">
                      SN: {controller.serialNumber}
                    </p>
                  </div>
                </div>
                <div class="border-thm-surface-border-2/50 text-thm-font-muted mt-4 flex items-center justify-between border-t pt-3 text-xs">
                  <span>{controller.io.length} IO Ports</span>
                  <span class="text-thm-primary opacity-0 transition-opacity group-hover:opacity-100">
                    View Detail →{/* no icon here for better binary size */}
                  </span>
                </div>
              </A>
            )}
          </For>
        </div>
      </div>
    </div>
  );
};
