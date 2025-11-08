import { createStore } from "solid-js/store";

class RootObjectModel {
  constructor() {
    const [controllers, setControllers] = createStore([]);
  }
}
