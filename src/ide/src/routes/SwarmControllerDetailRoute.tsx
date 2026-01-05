import { type Accessor, type Component, createMemo, For } from "solid-js";
import { useParams } from "@solidjs/router";
import { useOMContext } from "../contexts/transport/context.ts";
import { SwOSState } from "../api/generated/genApiEnums.ts";
import type { ApiController } from "../api/apiTypes.ts";
import { Unplug } from "lucide-solid";
import { getControllerIcon } from "../api/icons.ts";
import { Dynamic } from "solid-js/web";
import { EditableLabel } from "../components/EditableLabel.tsx";
import { FtSwarmIoTypeNames } from "../api/names.ts";

const INVALID_STATES: SwOSState[] = [
  SwOSState.OFFLINE,
  SwOSState.BOOTING,
  SwOSState.STARTWIFI,
  SwOSState.ERROR,
  SwOSState.FATAL,
  SwOSState.MAXSTATE,
];

const isInvalidState = (state: SwOSState) => INVALID_STATES.includes(state);

type SwarmStatusRenderComponent = Component<{
  controller: Accessor<ApiController>;
}>;

const InvalidState: SwarmStatusRenderComponent = ({ controller }) => {
  const icon = () => getControllerIcon(controller().type);

  return (
    <div class="w-full h-full flex items-center justify-center">
      <div class="max-w-md w-full mx-auto p-6 bg-thm-surface-2 border border-thm-surface-border-2 rounded-xl shadow-2xl backdrop-blur-sm">
        <div class="relative flex items-center justify-center h-32 mb-6 bg-thm-surface-1 rounded-lg border border-thm-surface-border-1 overflow-hidden">
          <div class="absolute inset-0 bg-thm-error/7 radial-gradient blur-2xl" />
          <div class="relative z-10 flex items-center gap-8 text-thm-font-muted">
            <div class="p-3 bg-thm-surface-3 rounded-full border border-thm-surface-border-2 shadow-sm transition-colors">
              <Dynamic component={icon()} class="w-8 h-8" />
            </div>

            <div class="flex items-center gap-1">
              <div class="w-1.5 h-1.5 rounded-full bg-thm-error" />
              <div class="w-16 h-[2px] animate-dash text-thm-error" />
              <div class="w-1.5 h-1.5 rounded-full bg-thm-error" />
            </div>

            <div class="p-3 bg-thm-surface-3 rounded-full border border-thm-surface-border-2 shadow-sm text-thm-error">
              <Unplug class="w-8 h-8" />
            </div>
          </div>
        </div>

        <div class="text-center space-y-2">
          <h2 class="text-xl font-bold text-thm-font tracking-tight">
            Controller Disconnected
          </h2>
          <p class="text-sm text-thm-font-muted leading-relaxed">
            Communication with the device could not be established. The
            controller is not responding to polling requests.
          </p>
        </div>
      </div>
    </div>
  );
};

const ControllerDetail: SwarmStatusRenderComponent = ({ controller }) => (
  <div class="p-4 w-full">
    <h2 class="text-thm-font-muted tracking-tight">Controller Detail</h2>

    <h2 class="text-2xl font-bold text-thm-font tracking-tight">
      <EditableLabel>{controller().name}</EditableLabel>
    </h2>

    <div class="w-full grid grid-cols-4 gap-2">
      <For each={controller().io}>
        {(value) => (
          <div class="p-4 border border-thm-surface-border-2 rounded-lg">
            <h3 class="text-thm-font-muted text-sm">
              {FtSwarmIoTypeNames[value.type]}
            </h3>
            <p class="text-thm-font text-lg font-medium">
              {value.name}: {value.value}
            </p>
          </div>
        )}
      </For>
    </div>
  </div>
);

export const SwarmControllerDetailRoute: Component = () => {
  const params = useParams<{ id: string }>();
  const om = useOMContext();
  const serialNumber = () => parseInt(params.id);
  const controller = createMemo(() => om.useController(serialNumber())());

  return (
    <>
      {isInvalidState(controller().state) ? (
        <InvalidState controller={controller} />
      ) : (
        <ControllerDetail controller={controller} />
      )}
    </>
  );
};
