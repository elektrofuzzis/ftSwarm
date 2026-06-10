import type { Component } from "solid-js";

export const ReadTheDocs: Component = () => {
  return (
    <p class="text-center text-thm-font-muted text-sm p-1">
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
