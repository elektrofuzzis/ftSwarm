import { createEffect, createSignal, Show, type Accessor } from "solid-js";

export function MissedHeartbeatCounter({ count }: { count: Accessor<number> }) {
  const bgWidthPercentTimeoutProgressBar = () =>
    Math.min((count() / 5) * 100, 100);
  const [isShown, setIsShown] = createSignal(false);
  createEffect(() => {
    const shouldShow = count() > 1 && count() < 5;
    if (isShown() == shouldShow) return;
    if (document.startViewTransition)
      document.startViewTransition(() => {
        setIsShown(shouldShow);
      });
    else setIsShown(shouldShow);
  });
  return (
    <Show when={isShown()}>
      <div class="text-thm-font bg-thm-surface-3 border-thm-surface-border-3 fixed right-0 bottom-4 z-30 rounded-tl-xl rounded-bl-xl shadow-lg">
        <div class="relative h-full w-full px-4 py-2">
          <div
            class="bg-thm-error absolute top-0 left-0 z-40 h-full rounded-tl-xl rounded-bl-xl transition-all duration-300"
            style={{ width: `${bgWidthPercentTimeoutProgressBar()}%` }}
          />
          <div class="relative z-50">
            Missed Heartbeats: <span class="font-mono">{count()}/5</span>
          </div>
        </div>
      </div>
    </Show>
  );
}
