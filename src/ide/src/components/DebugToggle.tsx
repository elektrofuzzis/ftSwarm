import { Show, type Component } from "solid-js";
import { useDebug } from "../contexts/DebugContext";
import Bug from "lucide-solid/icons/bug";

export const DebugToggle: Component = () => {
  const { isUnlocked, isOpen, toggleMenu } = useDebug();

  return (
    <Show when={isUnlocked() && !isOpen()}>
      <button
        onClick={toggleMenu}
        class="fixed top-4 right-0 z-30 p-2 rounded-tl-xl rounded-bl-xl shadow-lg hover:bg-thm-secondary text-thm-font bg-thm-primary transition-colors duration-300"
      >
        <Bug class="w-6 h-6" />
      </button>
    </Show>
  );
};
