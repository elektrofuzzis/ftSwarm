import type { ParentComponent } from "solid-js";

export const Surface1: ParentComponent<{ class?: String }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-1 border-1 border-thm-surface-border-1 rounded-lg " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};

export const Surface2: ParentComponent<{ class?: String }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-2 border-1 border-thm-surface-border-2 rounded " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};

export const Surface3: ParentComponent<{ class?: String }> = (props) => {
  return (
    <div
      class={
        "bg-thm-surface-3 border-1 border-thm-surface-border-3 rounded " +
        (props.class ?? "")
      }
    >
      {props.children}
    </div>
  );
};
