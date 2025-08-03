import { useIsRouting } from "@solidjs/router";
import { Sidebar } from "./Sidebar";
import { Surface1 } from "./Surface";
import type { ParentComponent } from "solid-js/types/server/rendering.js";

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
    <div class="w-full h-full flex p-4 gap-4">
      <Sidebar />
      <Content>{props.children}</Content>
    </div>
  );
};
