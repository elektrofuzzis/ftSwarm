import {type Component} from "solid-js";
import {useParams} from "@solidjs/router";
import {useOMContext} from "../contexts/transport/context.ts";

export const SwarmControllerDetailRoute: Component = () => {
    const params = useParams<{id: string}>();
    const om = useOMContext()
    const serialNumber = () => parseInt(params.id);
    const controller = om.useController(serialNumber());

    return <></>;
};
