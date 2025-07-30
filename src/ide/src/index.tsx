/* @refresh reload */
import { render } from "solid-js/web";
import "./index.css";
import { Layout } from "./components/Layout";

const root = document.getElementById("root");

render(() => <Layout />, root!);
