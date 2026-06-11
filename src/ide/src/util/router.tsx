import {
  createSignal,
  onMount,
  onCleanup,
  createContext,
  useContext,
  batch,
  type Component,
} from "solid-js";
import { type JSX } from "solid-js/jsx-runtime";
import logger from "./logger";

const [localLocation, setLocalLocation] = createSignal<string>(
  window.location.pathname,
);
const ParamsContext = createContext<Record<string, string>>({});

export function navigate(to: string): void {
  batch(() => {
    logger.info(`Navigating to ${to}`);
    window.history.pushState({}, "", to);
    setLocalLocation(to);
  });
}

export function useParams<T extends Record<string, string>>(): T {
  return useContext(ParamsContext) as T;
}

interface RouteProps {
  path: string;
  component: Component;
}

interface RouterProps {
  root?: (props: { children: JSX.Element }) => JSX.Element;
  routes: RouteProps[];
}

export function Router(props: RouterProps): JSX.Element {
  const handlePopState = (): void => {
    setLocalLocation(window.location.pathname);
  };

  onMount(() => {
    window.addEventListener("popstate", handlePopState);
  });

  onCleanup(() => {
    window.removeEventListener("popstate", handlePopState);
  });

  const matchedRoute = () => {
    const currentPath = localLocation();
    const routes = props.routes;

    for (const route of routes) {
      if (!route.path) continue;

      const paramNames: string[] = [];
      const regexPath = route.path.replace(/:([^/]+)/g, (_, name) => {
        paramNames.push(name);
        return "([^/]+)";
      });

      const match = new RegExp(`^${regexPath}$`).exec(currentPath);

      if (match) {
        const params = paramNames.reduce<Record<string, string>>(
          (acc, name, index) => {
            acc[name] = match[index + 1];
            return acc;
          },
          {},
        );

        const Component = route.component;

        logger.debug(
          `Route path ${route.path} matched ${currentPath} with params ${JSON.stringify(params)}`,
        );

        return (
          <ParamsContext.Provider value={params}>
            <Component />
          </ParamsContext.Provider>
        );
      }
    }
    logger.warn(`No route matched ${currentPath}`);
    return null;
  };

  const RootComponent = props.root;

  return RootComponent ? (
    <RootComponent>{matchedRoute()}</RootComponent>
  ) : (
    <>{matchedRoute()}</>
  );
}

interface AnchorProps extends JSX.AnchorHTMLAttributes<HTMLAnchorElement> {
  href: string;
}

export function A(props: AnchorProps): JSX.Element {
  const handleClick = (e: MouseEvent) => {
    if (
      e.defaultPrevented ||
      e.button !== 0 ||
      e.metaKey ||
      e.altKey ||
      e.ctrlKey ||
      e.shiftKey
    )
      return;
    e.preventDefault();
    navigate(props.href);
  };

  return <a {...props} onClick={handleClick} />;
}

interface NavigateProps {
  href: string;
}

export function Navigate(props: NavigateProps): null {
  onMount(() => {
    setTimeout(() => navigate(props.href), 0);
  });
  return null;
}

export function useLocation() {
  return localLocation;
}
