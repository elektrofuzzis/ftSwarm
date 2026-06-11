import { createSignal, Show } from "solid-js";
import { Sidebar } from "./Sidebar";
import { Surface1 } from "./Surface";
import { type ParentComponent } from "solid-js";
import { DebugToggle } from "./DebugToggle";
import { LoginOverlay } from "./LoginOverlay";
import { DebugMenu } from "./DebugMenu.tsx";
import { SaveFab } from "./SaveFab.tsx";
import { OverviewBar } from "./OverviewBar.tsx";

const Content: ParentComponent = (props) => {
  return (
    <Surface1 class={"flex-1 transition-opacity"}>{props.children}</Surface1>
  );
};

export const Layout: ParentComponent = (props) => {
  const [sidebarOpen, setSidebarOpen] = createSignal(false);
  const toggleSidebar = () => setSidebarOpen((prev) => !prev);
  const closeSidebar = () => setSidebarOpen(false);

  return (
    <>
      <div class="flex h-full w-full gap-4 p-4">
        <div class="flex flex-1 gap-4" style={{ "min-height": "0" }}>
          <Show when={sidebarOpen()}>
            <div
              class="fixed inset-0 z-30 bg-black/50 lg:hidden"
              onClick={closeSidebar}
            />
          </Show>
          <div
            class="bg-thm-surface-1 border-thm-surface-border-1 fixed inset-y-0 left-0 z-40 w-72 -translate-x-full border-r px-4 transition-transform duration-200 ease-in-out lg:static lg:inset-auto lg:z-auto lg:w-auto lg:translate-x-0 lg:border-none lg:bg-transparent lg:px-0"
            classList={{ "translate-x-0": sidebarOpen() }}
          >
            <Sidebar open={sidebarOpen()} onToggle={closeSidebar} />
          </div>

          <div
            class="flex h-full w-full flex-col gap-4"
            style={{ "min-height": "0" }}
          >
            <OverviewBar onToggleSidebar={toggleSidebar} />
            <Content>{props.children}</Content>
          </div>
        </div>
      </div>
      <LoginOverlay />

      <DebugMenu />
      <DebugToggle />
      <SaveFab />
    </>
  );
};
