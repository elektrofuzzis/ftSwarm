import type { ParentComponent } from "solid-js";

export const Surface1: ParentComponent<{ class?: string }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-1 border-thm-surface-border-1 rounded-lg border-1 " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};

export const Surface2: ParentComponent<{ class?: string }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-2 border-thm-surface-border-2 rounded border-1 " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};

export const Surface3: ParentComponent<{ class?: string }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-3 border-thm-surface-border-3 rounded border-1 " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};
