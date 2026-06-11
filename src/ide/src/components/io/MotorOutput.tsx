import { createEffect, on } from "solid-js";
import {
  type IoCardRendererComponent,
  registryKeyOfProps,
  sequencedDatumFactory,
} from "./index.tsx";
import type { ApiOutputIoType } from "../../api/apiTypes.ts";
import {
  useOMContext,
  useTransportContext,
} from "../../contexts/transport/context.ts";
import { transactMessage } from "../../api/transport";
import { apiNameOf, rpcResponseToSeq } from "../../api/util.ts";
import { useLoginContext } from "../../contexts/LoginContext.tsx";
import { SwOSIOType } from "../../api/generated/genApiEnums.ts";
import { Loader } from "./Loader.tsx";

const enum MotorOptimisticStores {
  Speed,
}

export const MotorOutput: IoCardRendererComponent<ApiOutputIoType> = (
  props,
) => {
  const om = useOMContext();
  const transport = useTransportContext();
  const login = useLoginContext();
  const max = () => (props.io.IOType == SwOSIOType.SWOSIO_MOTOR ? 4095 : 100);

  const [
    optimisticSpeed,
    setOptimisticSpeed,
    _0,
    {
      setEditing: setSpeedEditing,
      isMutating: isSpeedMutating,
      isThrottled: isSpeedThrottled,
      isEditing: isSpeedEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, MotorOptimisticStores.Speed),
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

  createEffect(
    on(max, (newMax, oldMax) => {
      if (oldMax === undefined) return;
      setOptimisticSpeed(Math.round((optimisticSpeed() * newMax) / oldMax));
    }),
  );

  return (
    <div class="space-y-3 pt-2">
      <div class="flex justify-between text-sm">
        <div class="flex items-center gap-2">
          <span class="text-zinc-400">Speed</span>
          <Loader
            isMutating={isSpeedMutating()}
            isThrottled={isSpeedThrottled()}
            isEditing={isSpeedEditing()}
          />
        </div>
        <span class="text-zinc-100">{props.io.speed}</span>
      </div>
      <input
        type="range"
        min={-max()}
        max={max()}
        value={optimisticSpeed()}
        disabled={login.interactiveDisabled()}
        onInput={(e) => setOptimisticSpeed(Number(e.currentTarget.value))}
        onFocus={() => setSpeedEditing(true)}
        onBlur={() => setSpeedEditing(false)}
        class="accent-thm-primary w-full"
      />
    </div>
  );
};
