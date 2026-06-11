import { Show, type Component } from "solid-js";
import { useDebug } from "../contexts/DebugContext";
import Bug from "lucide-solid/icons/bug";

export const DebugToggle: Component = () => {
  const { isUnlocked, isOpen, toggleMenu } = useDebug();

  return (
    <Show when={isUnlocked() && !isOpen()}>
      <button
        onClick={toggleMenu}
        class="hover:bg-thm-secondary text-thm-font bg-thm-primary fixed top-4 right-0 z-30 rounded-tl-xl rounded-bl-xl p-2 shadow-lg transition-colors duration-300"
      >
        <Bug class="h-6 w-6" />
      </button>
    </Show>
  );
};
