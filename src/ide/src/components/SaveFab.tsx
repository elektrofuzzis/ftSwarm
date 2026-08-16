import {
  useOMContext,
  useTransportContext,
} from "../contexts/transport/context.ts";
import Save from "lucide-solid/icons/save";
import { transactMessage } from "../api/transport";
import { Show } from "solid-js";

export const SaveFab = () => {
  const transport = useTransportContext();
  const om = useOMContext();

  const onSave = async () => {
    await transactMessage(transport, "swarm.save(0)");
    om.markSaved();
  };

  return (
    <Show when={om.needsSave}>
      <button
        onClick={onSave}
        class="bg-thm-primary group fixed right-8 bottom-8 z-50 flex h-14 w-14 items-center justify-center rounded-full text-white shadow-2xl transition-all hover:scale-110 active:scale-95"
        title="Save changes to NVS"
      >
        <Save class="h-6 w-6" />
        <span class="pointer-events-none absolute right-full mr-4 rounded border border-zinc-700 bg-zinc-800 px-2 py-1 text-xs whitespace-nowrap opacity-0 transition-opacity group-hover:opacity-100">
          Save to NVS
        </span>
      </button>
    </Show>
  );
};
