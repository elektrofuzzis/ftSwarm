import { Loader } from "./io/Loader.tsx";
import { createSignal, createEffect, onCleanup, For, Show } from "solid-js";

interface ColorInputProps {
  value: string;
  isMutating: boolean;
  isThrottled: boolean;
  isEditing: boolean;
  setEditing: (editing: boolean) => void;
  setValue: (value: string) => void;
  disabled: boolean;
}

const PRESET_COLORS = [
  "000000",
  "FFFFFF",
  "EF4444",
  "F97316",
  "FBBF24",
  "22C55E",
  "3B82F6",
  "6366F1",
  "A855F7",
  "EC4899",
];

const hsvToHex = (h: number, s: number, v: number): string => {
  const f = (n: number) => {
    const k = (n + h / 60) % 6;
    return v - v * s * Math.max(0, Math.min(k, 4 - k, 1));
  };
  const r = Math.round(f(5) * 255)
    .toString(16)
    .padStart(2, "0");
  const g = Math.round(f(3) * 255)
    .toString(16)
    .padStart(2, "0");
  const b = Math.round(f(1) * 255)
    .toString(16)
    .padStart(2, "0");
  return `${r}${g}${b}`.toUpperCase();
};

const hexToHsv = (hex: string): { h: number; s: number; v: number } => {
  const r = parseInt(hex.substring(0, 2), 16) / 255;
  const g = parseInt(hex.substring(2, 4), 16) / 255;
  const b = parseInt(hex.substring(4, 6), 16) / 255;
  const max = Math.max(r, g, b);
  const min = Math.min(r, g, b);
  const d = max - min;
  let h = 0;
  const s = max === 0 ? 0 : d / max;
  const v = max;
  if (max !== min) {
    switch (max) {
      case r:
        h = (g - b) / d + (g < b ? 6 : 0);
        break;
      case g:
        h = (b - r) / d + 2;
        break;
      case b:
        h = (r - g) / d + 4;
        break;
    }
    h /= 6;
  }
  return { h: h * 360, s, v };
};

export const ColorInput = (props: ColorInputProps) => {
  const [isOpen, setIsOpen] = createSignal(false);
  let containerRef: HTMLDivElement | undefined;
  let inputRef: HTMLInputElement | undefined;
  let sbPickerRef: HTMLDivElement | undefined;
  let hueSliderRef: HTMLDivElement | undefined;

  const handleDocumentClick = (e: MouseEvent) => {
    if (containerRef && !containerRef.contains(e.target as Node)) {
      setIsOpen(false);
      props.setEditing(false);
    }
  };

  createEffect(() => {
    if (isOpen()) {
      document.addEventListener("click", handleDocumentClick);
    } else {
      document.removeEventListener("click", handleDocumentClick);
    }
  });

  onCleanup(() => {
    document.removeEventListener("click", handleDocumentClick);
  });

  createEffect(() => {
    if (isOpen() && inputRef) {
      inputRef.value = props.value;
    }
  });

  createEffect(() => {
    const currentValue = props.value;
    if (inputRef && document.activeElement !== inputRef) {
      inputRef.value = currentValue;
    }
  });

  const updateColor = (hex: string) => {
    const cleanHex = hex.replace("#", "").toUpperCase();
    if (/^[0-9A-F]{0,6}$/.test(cleanHex)) {
      if (inputRef && document.activeElement === inputRef) {
        inputRef.value = cleanHex;
      }
      if (cleanHex.length === 6) {
        props.setValue(cleanHex);
      }
    }
  };

  const currentHsv = () => {
    const validHex = /^[0-9A-F]{6}$/i.test(props.value)
      ? props.value
      : "000000";
    return hexToHsv(validHex);
  };

  const handleSaturationBrightnessChange = (e: MouseEvent) => {
    if (!sbPickerRef) return;
    const rect = sbPickerRef.getBoundingClientRect();
    const x = Math.max(0, Math.min(e.clientX - rect.left, rect.width));
    const y = Math.max(0, Math.min(e.clientY - rect.top, rect.height));
    const s = x / rect.width;
    const v = 1 - y / rect.height;
    props.setValue(hsvToHex(currentHsv().h, s, v));
  };

  const handleHueChange = (e: MouseEvent) => {
    if (!hueSliderRef) return;
    const rect = hueSliderRef.getBoundingClientRect();
    const x = Math.max(0, Math.min(e.clientX - rect.left, rect.width));
    const h = (x / rect.width) * 360;
    const hsv = currentHsv();
    props.setValue(hsvToHex(h, hsv.s, hsv.v));
  };

  const setupDrag = (
    trackRef: HTMLDivElement | undefined,
    handler: (e: MouseEvent) => void,
  ) => {
    if (!trackRef) return;
    const onMouseMove = (e: MouseEvent) => handler(e);
    const onMouseUp = () => {
      window.removeEventListener("mousemove", onMouseMove);
      window.removeEventListener("mouseup", onMouseUp);
    };
    window.addEventListener("mousemove", onMouseMove);
    window.addEventListener("mouseup", onMouseUp);
  };

  return (
    <div ref={containerRef} class="relative">
      <div class="flex justify-between text-sm">
        <div class="flex items-center gap-2">
          <span class="text-zinc-400">Color</span>
          <Loader
            isMutating={props.isMutating}
            isThrottled={props.isThrottled}
            isEditing={props.isEditing}
          />
        </div>
        <div class="flex items-center gap-2">
          <span class="font-mono text-xs text-zinc-500">#{props.value}</span>
          <button
            type="button"
            disabled={props.disabled}
            onClick={(e) => {
              e.stopPropagation();
              const nextState = !isOpen();
              props.setEditing(nextState);
              setIsOpen(nextState);
            }}
            style={{ "background-color": `#${props.value}` }}
            class="h-6 w-6 cursor-pointer rounded border border-zinc-700 disabled:cursor-not-allowed"
          />
        </div>
      </div>

      <Show when={isOpen()}>
        <div class="absolute right-0 bottom-8 z-50 flex w-48 flex-col gap-3 rounded-lg border border-zinc-800 bg-zinc-900 p-3 shadow-xl">
          <div
            ref={sbPickerRef}
            onMouseDown={(e) => {
              handleSaturationBrightnessChange(e);
              setupDrag(sbPickerRef, handleSaturationBrightnessChange);
            }}
            style={{ "background-color": `hsl(${currentHsv().h}, 100%, 50%)` }}
            class="relative h-28 w-full cursor-crosshair overflow-hidden rounded select-none"
          >
            <div class="absolute inset-0 bg-gradient-to-r from-white to-transparent" />
            <div class="absolute inset-0 bg-gradient-to-t from-black to-transparent" />
            <div
              style={{
                left: `${currentHsv().s * 100}%`,
                top: `${(1 - currentHsv().v) * 100}%`,
              }}
              class="pointer-events-none absolute -mt-1.5 -ml-1.5 h-3 w-3 rounded-full border border-white shadow"
            />
          </div>

          <div
            ref={hueSliderRef}
            onMouseDown={(e) => {
              handleHueChange(e);
              setupDrag(hueSliderRef, handleHueChange);
            }}
            style={{
              background:
                "linear-gradient(to right, #f00 0%, #ff0 17%, #0f0 33%, #0ff 50%, #00f 67%, #f0f 83%, #f00 100%)",
            }}
            class="relative h-3 w-full cursor-pointer rounded select-none"
          >
            <div
              style={{ left: `${(currentHsv().h / 360) * 100}%` }}
              class="pointer-events-none absolute -top-0.5 -ml-0.75 h-4 w-1.5 rounded border border-white bg-zinc-900 shadow"
            />
          </div>

          <div>
            <label class="mb-1 block text-xs text-zinc-400">Hex Value</label>
            <div class="flex items-center gap-1 rounded border border-zinc-700 bg-zinc-800 px-2 py-1">
              <span class="text-xs text-zinc-500">#</span>
              <input
                ref={inputRef}
                type="text"
                maxLength={6}
                onInput={(e) => updateColor(e.currentTarget.value)}
                class="w-full bg-transparent font-mono text-xs text-zinc-200 uppercase outline-none"
              />
            </div>
          </div>

          <div>
            <label class="mb-1 block text-xs text-zinc-400">Presets</label>
            <div class="grid grid-cols-5 gap-1.5">
              <For each={PRESET_COLORS}>
                {(color) => (
                  <button
                    type="button"
                    onClick={() => {
                      updateColor(color);
                      props.setValue(color);
                    }}
                    style={{ "background-color": `#${color}` }}
                    class={`h-6 w-6 cursor-pointer rounded border transition-transform active:scale-95 ${
                      props.value === color
                        ? "border-white"
                        : "border-transparent"
                    }`}
                  />
                )}
              </For>
            </div>
          </div>
        </div>
      </Show>
    </div>
  );
};
