import { createEffect, createSignal, Show, type Accessor } from "solid-js";

export function MissedHeartbeatCounter({ count }: { count: Accessor<number> }) {
  const bgWidthPercentTimeoutProgressBar = () =>
    Math.min((count() / 5) * 100, 100);
  const [isShown, setIsShown] = createSignal(false);
  createEffect(() => {
    const shouldShow = count() > 1 && count() < 5;
    if (document.startViewTransition)
      document.startViewTransition(() => {
        setIsShown(shouldShow);
      });
    else setIsShown(shouldShow);
  });
  return (
    <Show when={isShown()}>
      <div class="fixed bottom-4 right-0 z-30 rounded-tl-xl rounded-bl-xl shadow-lg text-thm-font bg-thm-surface-3 border-thm-surface-border-3">
        <div class="relative w-full h-full py-2 px-4 ">
          <div
            class="absolute top-0 left-0 h-full z-40 bg-thm-error rounded-tl-xl rounded-bl-xl transition-all duration-300"
            style={{ width: `${bgWidthPercentTimeoutProgressBar()}%` }}
          />
          <div class="z-50 relative">
            Missed Heartbeats: <span class="font-mono">{count()}/5</span>
          </div>
        </div>
      </div>
    </Show>
  );
}
