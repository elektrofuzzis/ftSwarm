import { type Accessor, type Component, createMemo, For } from "solid-js";
import { useParams } from "@solidjs/router";
import { useOMContext } from "../contexts/transport/context.ts";
import { SwOSState } from "../api/generated/genApiEnums.ts";
import {
  IOTypeClasses,
  type ApiController,
  type FtSwarmIo,
} from "../api/apiTypes.ts";
import Unplug from "lucide-solid/icons/unplug";
import { getControllerIcon } from "../api/icons.ts";
import { Dynamic } from "solid-js/web";
import { EditableLabel } from "../components/EditableLabel.tsx";
import { IoCard } from "../components/io";

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
  seq: Accessor<number>;
}>;

const InvalidState: SwarmStatusRenderComponent = ({ controller, seq: _ }) => {
  const icon = () => getControllerIcon(controller().CtrlVersion);

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

const Divider: Component<{ name: string }> = ({ name }) => (
  <div class="mb-2 mt-3 flex items-center gap-2">
    <div class="h-px bg-thm-surface-border-2 flex-1"></div>
    <span class="text-xs font-semibold text-thm-font-muted uppercase tracking-wider">
      {name}
    </span>
    <div class="h-px bg-thm-surface-border-2 flex-1"></div>
  </div>
);

const ControllerDetail: SwarmStatusRenderComponent = ({ controller, seq }) => {
  const ios = () => controller().io;
  const inputs = () =>
    ios().filter((io) => IOTypeClasses[io.IOType] == "input");
  const outputs = () =>
    ios().filter((io) => IOTypeClasses[io.IOType] == "output");
  const specials = () =>
    ios().filter((io) => IOTypeClasses[io.IOType] == "special");

  const categories = () =>
    (
      [
        ["Inputs", inputs],
        ["Outputs", outputs],
        ["Specials", specials],
      ] satisfies [string, () => FtSwarmIo[]][]
    ).filter(([_, fn]) => fn().length > 0);

  return (
    <div class="p-4 w-full">
      <h2 class="text-thm-font-muted tracking-tight">Controller Detail</h2>

      <h2 class="text-2xl font-bold text-thm-font tracking-tight">
        <EditableLabel>{controller().name}</EditableLabel>
      </h2>

      <For each={categories()}>
        {([name, iosFn]) => (
          <>
            <Divider name={name} />
            <div class="w-full grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-4 mb-4">
              <For each={iosFn()}>
                {(value) => <IoCard io={value} controller={controller()} seq={seq} />}
              </For>
            </div>
          </>
        )}
      </For>
    </div>
  );
};

export const SwarmControllerDetailRoute: Component = () => {
  const params = useParams<{ id: string }>();
  const om = useOMContext();
  const serialNumber = () => parseInt(params.id);
  const controller = createMemo(() => om.useController(serialNumber())());

  return (
    <>
      {isInvalidState(controller().state) ? (
        <InvalidState controller={controller} seq={om.lastSequence} />
      ) : (
        <ControllerDetail controller={controller} seq={om.lastSequence} />
      )}
    </>
  );
};
