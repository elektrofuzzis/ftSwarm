import type { ParentComponent } from "solid-js";
import { Layout } from "./components/Layout";
import { DebugContextProvider } from "./contexts/DebugContext";

export const App: ParentComponent = (props) => {
  return (
    <DebugContextProvider>
      <Layout>{props.children}</Layout>
    </DebugContextProvider>
  );
};
