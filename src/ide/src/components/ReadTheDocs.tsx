import type { Component } from "solid-js";

export const ReadTheDocs: Component = () => {
  return (
    <p class="text-thm-font-muted p-1 text-center text-sm">
      Read the docs at{" "}
      <a href="https://elektrofuzzis.github.io/ftSwarm">
        https://elektrofuzzis.github.io/ftSwarm
      </a>{" "}
      <br />
      &copy; {new Date().getFullYear()} Christian Bergschneider &amp; Stefan
      Fuss
    </p>
  );
};
