import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiStepperOutputType } from "../../api/apiTypes.ts";
import {
  useOMContext,
  useTransportContext,
} from "../../contexts/transport/context.ts";
import { registryKeyOfProps, sequencedDatumFactory } from "./index.tsx";
import { apiNameOf, rpcResponseToSeq } from "../../api/util.ts";
import { useLoginContext } from "../../contexts/LoginContext.tsx";
import { Loader } from "./Loader.tsx";
import { Show } from "solid-js";
import { transactMessage } from "../../api/transport";

export const StepperOutput: IoCardRendererComponent<ApiStepperOutputType> = (
  props,
) => {
  const transport = useTransportContext();
  const login = useLoginContext();
  const om = useOMContext();

  const [
    optimisticSpeed,
    setOptimisticSpeed,
    _,
    {
      setEditing: setSpeedEditing,
      isMutating: isSpeedMutating,
      isThrottled: isSpeedThrottled,
      isEditing: isSpeedEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, "speed"),
    sequencedDatumFactory(props, (_) => props.io.speed),
    async (newSpeed) => {
      const result = await transactMessage(
        transport,
        `${apiNameOf(props.io, props.controller)}.setSpeed(${newSpeed})`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(result);
    },
    { needsSave: false },
  );

  const [
    optimisticPosition,
    setOptimisticPosition,
    __,
    {
      setEditing: setPositionEditing,
      isMutating: isPositionMutating,
      isThrottled: isPositionThrottled,
      isEditing: isPositionEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, "position"),
    sequencedDatumFactory(props, (_) => props.io.position),
    async (newPosition) => {
      const result = await transactMessage(
        transport,
        `${apiNameOf(props.io, props.controller)}.setPosition(${newPosition})`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(result);
    },
    { needsSave: false },
  );

  return (
    <div class="space-y-4 pt-2">
      <div class="space-y-2">
        <div class="flex justify-between text-sm">
          <div class="flex items-center gap-2">
            <span class="text-zinc-400">Speed</span>
            <Loader
              isMutating={isSpeedMutating()}
              isThrottled={isSpeedThrottled()}
              isEditing={isSpeedEditing()}
            />
          </div>
          <span class="text-zinc-100">{optimisticSpeed()}</span>
        </div>
        <input
          type="range"
          min="0"
          max="10240"
          value={optimisticSpeed()}
          onInput={(e) => {
            setSpeedEditing(true);
            setOptimisticSpeed(parseInt(e.target.value));
          }}
          onChange={() => setSpeedEditing(false)}
          disabled={login.interactiveDisabled()}
          class={`accent-thm-primary h-2 w-full cursor-pointer appearance-none rounded-lg bg-zinc-700 ${login.interactiveDisabled() ? "cursor-not-allowed opacity-50" : ""}`}
        />
      </div>

      <div class="space-y-2">
        <div class="flex justify-between text-sm">
          <div class="flex items-center gap-2">
            <span class="text-zinc-400">Position</span>
            <Loader
              isMutating={isPositionMutating()}
              isThrottled={isPositionThrottled()}
              isEditing={isPositionEditing()}
            />
          </div>
          <span class="text-zinc-100">{optimisticPosition()}</span>
        </div>
        <input
          type="number"
          value={optimisticPosition()}
          onInput={(e) => {
            setPositionEditing(true);
            setOptimisticPosition(parseInt(e.target.value));
          }}
          onChange={() => setPositionEditing(false)}
          disabled={login.interactiveDisabled()}
          class="focus:border-thm-primary w-full rounded border border-zinc-700 bg-zinc-800 px-2 py-1 text-sm text-zinc-100 transition-colors focus:outline-none"
        />
      </div>

      <div class="grid grid-cols-2 gap-2">
        <div class="rounded border border-zinc-700/50 bg-zinc-800/50 p-2">
          <div class="mb-1 text-[10px] font-bold text-zinc-500 uppercase">
            Distance
          </div>
          <div class="font-mono text-sm text-zinc-200">{props.io.distance}</div>
        </div>
        <div class="flex flex-col gap-1">
          <Show when={props.io.homing}>
            <div class="bg-thm-primary/10 text-thm-primary border-thm-primary/20 animate-pulse rounded border px-2 py-1 text-center text-[10px] font-bold">
              HOMING
            </div>
          </Show>
          <Show when={props.io.running}>
            <div class="rounded border border-emerald-500/20 bg-emerald-500/10 px-2 py-1 text-center text-[10px] font-bold text-emerald-500">
              RUNNING
            </div>
          </Show>
        </div>
      </div>
    </div>
  );
};
