import type { Component } from "solid-js";

const PatternedBackground: Component = () => {
  return (
    <div
      class="fixed inset-0 w-full h-full -z-10 pointer-events-none"
      style={{
        "background-image": `
          linear-gradient(to right, var(--thm-surface-1) 1px, transparent 1px),
          linear-gradient(to bottom, var(--thm-surface-1) 1px, transparent 1px)
        `,
        "background-size": "100px 100px",
        opacity: 0.8,
        "mask-image":
          "radial-gradient(ellipse at center, black 0%, transparent 75%)",
        "-webkit-mask-image":
          "radial-gradient(ellipse at center, black 0%, transparent 75%)",
      }}
    />
  );
};

export default PatternedBackground;
