import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";

const ip = "172.16.16.49"; // Enter dev-mode IP here

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [svelte()],
  server: {
    proxy: {
      "/api": `http://${ip}:80`,
      "/assets": `http://${ip}:80`,
    },
  },
});
