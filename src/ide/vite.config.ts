import { defineConfig } from "vite";
import solid from "vite-plugin-solid";
import tailwindcss from "@tailwindcss/vite";
import {statsPlugin} from "vite-bundle-explorer/plugin";

export default defineConfig({
  plugins: [solid(), tailwindcss(), statsPlugin()],
  build: {
    target: "esnext",
    minify: "esbuild",
  },
});
