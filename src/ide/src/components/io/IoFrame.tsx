import { For, type ParentComponent, Show } from "solid-js";
import {
  type IoCardProps,
  registryKeyOfProps,
  sequencedDatumFactory,
} from "./index.tsx";
import {
  useOMContext,
  useTransportContext,
} from "../../contexts/transport/context.ts";
import { getIoIcon } from "../../api/icons.ts";
import ChevronDown from "lucide-solid/icons/chevron-down";
import Link2 from "lucide-solid/icons/link-2";
import { useLoginContext } from "../../contexts/LoginContext.tsx";
import { transactMessage } from "../../api/transport";
import { apiNameOf, rpcResponseToSeq } from "../../api/util.ts";
import { ioTypeInfos } from "../../api/generated/genIoMappings.ts";
import { SwOSIOClass, SwOSIOType } from "../../api/generated/genApiEnums.ts";
import { Loader } from "./Loader.tsx";

const enum IOFrameOptimisticStores {
  ALIAS,
}

export const IoFrame: ParentComponent<IoCardProps> = (props) => {
  const om = useOMContext();
  const transport = useTransportContext();
  const login = useLoginContext();

  const [
    optimisticAlias,
    setOptimisticAlias,
    _,
    {
      setEditing: setAliasEditing,
      isMutating: isAliasMutating,
      isThrottled: isAliasThrottled,
      isEditing: isAliasEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, {
      kind: "Frame",
      val: IOFrameOptimisticStores.ALIAS,
    }),
    sequencedDatumFactory(props, (_) => props.io.alias ?? props.io.name),
    async (newAlias) => {
      const result = await transactMessage(
        transport,
        `${apiNameOf(props.io, props.controller)}.setAlias("${newAlias}")`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(result);
    },
  );

  const currentInfo = () => ioTypeInfos[props.io.IOType];
  const availableTypes = () => {
    const info = currentInfo();
    if (!info) return [];
    if (info.ioClass == SwOSIOClass.SWOSIOCLASS_SINGULAR) return [info];
    return Object.values(ioTypeInfos).filter(
      (i) => i.ioClass === info.ioClass && i.showInApi,
    );
  };

  const onTypeChange = async (e: Event) => {
    const newType = parseInt(
      (e.target as HTMLSelectElement).value,
    ) as SwOSIOType;
    const info = ioTypeInfos[newType];

    let params = `${newType}`;
    // The setIOType command expects a second parameter (normallyOpen) for INPUT class IOs.
    // It does NOT support setIOType for SINGULAR types (Joystick, Stepper, etc.) in their specialized handlers.
    // However, it IS supported in executeActorCmd for MOTOR class types with only 1 parameter.
    if (info && info.ioClass === SwOSIOClass.SWOSIOCLASS_INPUT) {
      params += ",1"; // Default normallyOpen to true
    }

    await transactMessage(
      transport,
      `${apiNameOf(props.io, props.controller)}.setIOType(${params})`,
    );
    om.markNeedsSave();
  };

  return (
    <div class="border bg-thm-surface-2 border-thm-surface-border-2 rounded-lg p-3 flex flex-col transition-all group">
      <div class="flex items-center justify-between mb-1">
        <div class="relative flex items-center justify-between w-full">
          <span class="text-[10px] font-bold uppercase text-thm-primary tracking-wider pr-2">
            {props.io.name}
          </span>
          <select
            value={props.io.IOType}
            class="appearance-none bg-transparent text-[10px] w-full font-bold uppercase tracking-wider text-thm-font-muted hover:text-thm-font pr-4 transition-colors outline-none cursor-pointer z-10"
            disabled={login.interactiveDisabled()}
            onChange={onTypeChange}
          >
            <For each={availableTypes()}>
              {(type) => <option value={type.type}>{type.name}</option>}
            </For>
          </select>
          <ChevronDown
            size={12}
            class="absolute right-0 text-thm-font-muted pointer-events-none"
          />
        </div>
        <div class="opacity-0 group-hover:opacity-100 transition-opacity">
          <Loader
            isMutating={isAliasMutating()}
            isThrottled={isAliasThrottled()}
            isEditing={isAliasEditing()}
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
        <span class="text-xs font-mono font-bold text-thm-font-muted py-0.5">
          {getIoIcon(props.io.IOType)({ class: "w-5 h-5" })}
        </span>
        <input
          type="text"
          value={optimisticAlias()}
          class="bg-transparent text-sm font-semibold text-thm-font transition-colors border-b border-transparent hover:border-thm-surface-border-2 focus:border-thm-primary focus:outline-none px-1 w-full"
          placeholder="Name"
          disabled={login.interactiveDisabled()}
          onInput={(e) => setOptimisticAlias(e.currentTarget.value)}
          onFocus={() => setAliasEditing(true)}
          onBlur={() => setAliasEditing(false)}
        />
      </div>

      <div class="border-t border-zinc-800/50 pt-2">{props.children}</div>
    </div>
  );
};
