import {Component, TagComponent, Types} from "ecsy";

export class FtSwarmJST extends Component<any>{}

FtSwarmJST.schema = {
    serial_number: {type: Types.Number},
    swarm_index: {type: Types.Number},
}

export class FtSwarmCtrl extends FtSwarmJST{}

export class Kelda extends TagComponent{}


