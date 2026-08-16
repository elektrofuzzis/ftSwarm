import type { ParentComponent } from "solid-js";
import { Layout } from "./components/Layout";

export const App: ParentComponent = (props) => {
  return <Layout>{props.children}</Layout>;
};
