import type { ParentComponent, ComponentProps } from "solid-js";
import { splitProps, mergeProps } from "solid-js";

type ButtonVariant = "primary" | "secondary" | "outline" | "ghost" | "danger";

export type ButtonProps = ComponentProps<"button"> & {
  variant?: ButtonVariant;
};

export const Button: ParentComponent<ButtonProps> = (props) => {
  const merged = mergeProps({ variant: "primary" as ButtonVariant }, props);
  const [local, rest] = splitProps(merged, ["children", "class", "variant"]);

  const baseClasses =
    "w-full py-2 px-4 rounded-md font-semibold text-center " +
    "transition-all " +
    "focus:outline-none focus:ring-1 focus:ring-thm-primary focus:ring-offset-2 focus:ring-offset-thm-background " +
    "disabled:opacity-50 disabled:cursor-not-allowed";

  const getVariantClasses = (variant: ButtonVariant) => {
    switch (variant) {
      case "secondary":
        return "text-thm-font bg-thm-secondary hover:brightness-110 active:brightness-90 disabled:hover:brightness-100";

      case "outline":
        return "bg-transparent border border-thm-primary text-thm-primary hover:bg-thm-primary hover:text-thm-font active:bg-thm-primary active:brightness-90";

      case "ghost":
        return "bg-transparent text-thm-font-muted hover:text-thm-font active:text-thm-primary";

      case "danger":
        return "text-thm-font bg-thm-error hover:brightness-110 active:brightness-90 disabled:hover:brightness-100";

      case "primary":
      default:
        return "text-thm-font bg-thm-accent hover:brightness-110 active:brightness-90 disabled:hover:brightness-100";
    }
  };

  const variantClasses = getVariantClasses(local.variant);

  return (
    <button
      class={`${baseClasses} ${variantClasses} ${local.class || ""}`}
      {...rest}
    >
      {local.children}
    </button>
  );
};
