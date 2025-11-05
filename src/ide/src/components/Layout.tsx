import { useIsRouting } from "@solidjs/router";
import { Sidebar } from "./Sidebar";
import { Surface1 } from "./Surface";
import type { ParentComponent } from "solid-js";
import { DebugMenu } from "./DebugMenu";
import { DebugToggle } from "./DebugToggle";
import { useDebug } from "../contexts/DebugContext";
import { DebugMode } from "../contexts/logtypes";
import { Show } from "solid-js";

const Content: ParentComponent = (props) => {
  const isRouting = useIsRouting();
  return (
    <Surface1
      class={"flex-1 transition-opacity " + (isRouting() ? "opacity-75" : "")}
    >
      {props.children}
    </Surface1>
  );
};

export const Layout: ParentComponent = (props) => {
  const { isOpen, mode } = useDebug();

  return (
    <>
      <div
        class="w-full h-full flex p-4 gap-4"
        classList={{
          "flex-col": isOpen() && mode() === DebugMode.ATTACHED_BOTTOM,
        }}
      >
        <div class="flex flex-1 gap-4" style={{ "min-height": "0" }}>
          <Sidebar />
          <Content>{props.children}</Content>
          <Show when={isOpen() && mode() === DebugMode.ATTACHED_RIGHT}>
            <div class="w-1/3 flex flex-col">
              <DebugMenu />
            </div>
          </Show>
        </div>
        <Show when={isOpen() && mode() === DebugMode.ATTACHED_BOTTOM}>
          <div class="h-1/3 flex flex-col">
            <DebugMenu />
          </div>
        </Show>
      </div>

      <Show when={mode() === DebugMode.SCREEN_RIGHT}>
        <DebugMenu />
      </Show>
      <DebugToggle />
    </>
  );
};
