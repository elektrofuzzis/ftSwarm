import { useIsRouting } from "@solidjs/router";
import { Sidebar } from "./Sidebar";
import { Surface1 } from "./Surface";
import { type ParentComponent, lazy } from "solid-js";
import { DebugToggle } from "./DebugToggle";
import { LoginOverlay } from "./LoginOverlay";

const DebugMenu = lazy(() =>
  import("./DebugMenu").then((m) => ({ default: m.DebugMenu })),
);

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
  return (
    <>
      <div class="w-full h-full flex p-4 gap-4">
        <div class="flex flex-1 gap-4" style={{ "min-height": "0" }}>
          <Sidebar />
          <Content>{props.children}</Content>
        </div>
      </div>
      <LoginOverlay />

      <DebugMenu />
      <DebugToggle />
    </>
  );
};
