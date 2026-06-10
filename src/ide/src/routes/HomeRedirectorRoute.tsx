import { Navigate } from "@solidjs/router";

export const HomeRedirectorRoute = () => {
  return <Navigate href="/controller/overview" />;
};
