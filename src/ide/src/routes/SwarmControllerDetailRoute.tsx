import { type Accessor, type Component, createMemo, For } from "solid-js";
import { useParams } from "../util/router";
import {
  useOMContext,
  useTransportContext,
} from "../contexts/transport/context.ts";
import { SwOSState } from "../api/generated/genApiEnums.ts";
import {
  IOTypeClasses,
  type ApiController,
  type FtSwarmIo,
} from "../api/apiTypes.ts";
import Unplug from "lucide-solid/icons/unplug";
import { getControllerIcon } from "../api/icons.ts";
import { EditableLabel } from "../components/EditableLabel.tsx";
import { IoCard } from "../components/io";
import { transactMessage } from "../api/transport";
import { rpcResponseToSeq } from "../api/util.ts";
import { useLoginContext } from "../contexts/LoginContext.tsx";
import { Loader } from "../components/io/Loader.tsx";

const INVALID_STATES: SwOSState[] = [
  SwOSState.OFFLINE,
  SwOSState.BOOTING,
  SwOSState.STARTWIFI,
  SwOSState.ERROR,
  SwOSState.FATAL,
];

const isInvalidState = (state: SwOSState) => INVALID_STATES.includes(state);

type SwarmStatusRenderComponent = Component<{
  controller: Accessor<ApiController>;
  seq: Accessor<number>;
}>;

const InvalidState: SwarmStatusRenderComponent = ({ controller, seq: _ }) => {
  const icon = () => getControllerIcon(controller().CtrlVersion);

  return (
    <div class="flex h-full w-full items-center justify-center">
      <div class="bg-thm-surface-2 border-thm-surface-border-2 mx-auto w-full max-w-md rounded-xl border p-6 shadow-2xl backdrop-blur-sm">
        <div class="bg-thm-surface-1 border-thm-surface-border-1 relative mb-6 flex h-32 items-center justify-center overflow-hidden rounded-lg border">
          <div class="bg-thm-error/7 radial-gradient absolute inset-0 blur-2xl" />
          <div class="text-thm-font-muted relative z-10 flex items-center gap-8">
            <div class="bg-thm-surface-3 border-thm-surface-border-2 rounded-full border p-3 shadow-sm transition-colors">
              {icon()({ class: "w-8 h-8" })}
            </div>

            <div class="flex items-center gap-1">
              <div class="bg-thm-error h-1.5 w-1.5 rounded-full" />
              <div class="animate-dash text-thm-error h-[2px] w-16" />
              <div class="bg-thm-error h-1.5 w-1.5 rounded-full" />
            </div>

            <div class="bg-thm-surface-3 border-thm-surface-border-2 text-thm-error rounded-full border p-3 shadow-sm">
              <Unplug class="h-8 w-8" />
            </div>
          </div>
        </div>

        <div class="space-y-2 text-center">
          <h2 class="text-thm-font text-xl font-bold tracking-tight">
            Controller Disconnected
          </h2>
          <p class="text-thm-font-muted text-sm leading-relaxed">
            Communication with the device could not be established. The
            controller is not responding to polling requests.
          </p>
        </div>
      </div>
    </div>
  );
};

const Divider: Component<{ name: string }> = ({ name }) => (
  <div class="mt-3 mb-2 flex items-center gap-2">
    <div class="bg-thm-surface-border-2 h-px flex-1"></div>
    <span class="text-thm-font-muted text-xs font-semibold tracking-wider uppercase">
      {name}
    </span>
    <div class="bg-thm-surface-border-2 h-px flex-1"></div>
  </div>
);

const ControllerDetail: SwarmStatusRenderComponent = ({ controller, seq }) => {
  const transport = useTransportContext();
  const om = useOMContext();
  const login = useLoginContext();

  const [
    optimisticName,
    setOptimisticName,
    _,
    {
      setEditing: setNameEditing,
      isMutating: isNameMutating,
      isThrottled: isNameThrottled,
      isEditing: isNameEditing,
    },
  ] = om.useBoundStore(
    `ctrl:${controller().serialNumber}:name`,
    () => ({ data: controller().name, seq: seq() }),
    async (newName) => {
      let res = await transactMessage(
        transport,
        `${controller().name}.setAlias("${newName}")`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(res);
    },
  );

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
    <div class="w-full p-4">
      <div class="mb-1 flex items-center gap-2">
        <h2 class="text-thm-font-muted tracking-tight">Controller Detail</h2>
        <Loader
          isMutating={isNameMutating()}
          isThrottled={isNameThrottled()}
          isEditing={isNameEditing()}
        />
      </div>

      <h2 class="text-thm-font text-2xl font-bold tracking-tight">
        <EditableLabel
          allowEdit={!login.interactiveDisabled()}
          onEdit={setOptimisticName}
          isEditing={isNameEditing()}
          setEditing={setNameEditing}
        >
          {optimisticName()}
        </EditableLabel>
      </h2>

      <For each={categories()}>
        {([name, iosFn]) => (
          <>
            <Divider name={name} />
            <div class="mb-4 grid w-full grid-cols-1 gap-4 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4">
              <For each={iosFn()}>
                {(value) => (
                  <IoCard io={value} controller={controller()} seq={seq} />
                )}
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
