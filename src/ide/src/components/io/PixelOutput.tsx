import type { IoCardRendererComponent } from "./index.tsx";
import type { ApiPixelOutputType } from "../../api/apiTypes.ts";
import {
  useOMContext,
  useTransportContext,
} from "../../contexts/transport/context.ts";
import { registryKeyOfProps, sequencedDatumFactory } from "./index.tsx";
import { apiNameOf, rpcResponseToSeq } from "../../api/util.ts";
import { useLoginContext } from "../../contexts/LoginContext.tsx";
import { Loader } from "./Loader.tsx";
import { transactMessage } from "../../api/transport";
import { ColorInput } from "../ColorInput.tsx";

export const PixelOutput: IoCardRendererComponent<ApiPixelOutputType> = (
  props,
) => {
  const transport = useTransportContext();
  const login = useLoginContext();
  const om = useOMContext();

  const [
    optimisticBrightness,
    setOptimisticBrightness,
    _,
    {
      setEditing: setBrightnessEditing,
      isMutating: isBrightnessMutating,
      isThrottled: isBrightnessThrottled,
      isEditing: isBrightnessEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, "brightness"),
    sequencedDatumFactory(props, (_) => props.io.brightness),
    async (newBrightness) => {
      const result = await transactMessage(
        transport,
        `${apiNameOf(props.io, props.controller)}.setBrightness(${newBrightness})`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(result);
    },
    { needsSave: false },
  );

  const [
    optimisticColor,
    setOptimisticColor,
    __,
    {
      setEditing: setColorEditing,
      isMutating: isColorMutating,
      isThrottled: isColorThrottled,
      isEditing: isColorEditing,
    },
  ] = om.useBoundStore(
    registryKeyOfProps(props, "color"),
    sequencedDatumFactory(props, (_) => props.io.color),
    async (newColor) => {
      const result = await transactMessage(
        transport,
        `${apiNameOf(props.io, props.controller)}.setColor(#${newColor})`,
      ).then((v) => v.unwrapOr(null));
      return rpcResponseToSeq(result);
    },
    { needsSave: false },
  );

  return (
    <div class="space-y-3 pt-2">
      <div class="flex justify-between text-sm">
        <div class="flex items-center gap-2">
          <span class="text-zinc-400">Brightness</span>
          <Loader
            isMutating={isBrightnessMutating()}
            isThrottled={isBrightnessThrottled()}
            isEditing={isBrightnessEditing()}
          />
        </div>
        <span class="text-zinc-100">{optimisticBrightness()}</span>
      </div>
      <input
        type="range"
        min="0"
        max="255"
        value={optimisticBrightness()}
        onInput={(e) => {
          setBrightnessEditing(true);
          setOptimisticBrightness(parseInt(e.target.value));
        }}
        onChange={() => setBrightnessEditing(false)}
        disabled={login.interactiveDisabled()}
        class={`accent-thm-primary h-2 w-full cursor-pointer appearance-none rounded-lg bg-zinc-700 ${login.interactiveDisabled() ? "cursor-not-allowed opacity-50" : ""}`}
      />

      <ColorInput
        value={optimisticColor()}
        isMutating={isColorMutating()}
        isThrottled={isColorThrottled()}
        isEditing={isColorEditing()}
        setEditing={setColorEditing}
        setValue={setOptimisticColor}
        disabled={login.interactiveDisabled()}
      />
    </div>
  );
};
