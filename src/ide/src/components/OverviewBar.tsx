import type { Component } from "solid-js";
import { useOMContext } from "../contexts/transport/context";
import { SwOSState } from "../api/generated/genApiEnums";
import { Surface1 } from "./Surface";
import { StatusCircle } from "./Sidebar";

export const OverviewBar: Component<{
  onToggleSidebar: () => void;
}> = (props) => {
  const swarm = useOMContext();
  const controllers = () => Object.values(swarm.controllers);
  const totalControllers = () => controllers().length;
  const onlineControllers = () =>
    controllers().filter((it) => it.state === SwOSState.RUNNING).length;

  const allOnline = () =>
    totalControllers() > 0 && onlineControllers() === totalControllers();
  const statusClass = () => {
    if (allOnline()) return "bg-thm-ok";
    if (onlineControllers() === 0) return "bg-thm-error";
    return "bg-thm-primary";
  };

  return (
    <div class="flex justify-between lg:hidden">
      <div class="flex items-center gap-2">
        <button class="text-thm-font p-1.5" onClick={props.onToggleSidebar}>
          <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            width="24"
            height="24"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            stroke-linecap="round"
            stroke-linejoin="round"
          >
            <line x1="4" y1="12" x2="20" y2="12"></line>
            <line x1="4" y1="6" x2="20" y2="6"></line>
            <line x1="4" y1="18" x2="20" y2="18"></line>
          </svg>
        </button>

        <span class="inline-block pb-1 font-['Trebuchet_MS',sans-serif] text-2xl leading-none font-bold tracking-[-1.5px] text-[#989898] italic">
          ftSwarm
        </span>
      </div>

      <Surface1 class="ml-4 flex items-center gap-1 px-2 py-px text-sm">
        <StatusCircle class={statusClass()} />
        <span class="text-thm-font-muted pl-1">Online:</span>
        {onlineControllers()}/{totalControllers()}
      </Surface1>
    </div>
  );
};
