import {Component, TagComponent, Types} from "ecsy";

class IoObject extends Component<any> {
}

IoObject.schema = {
    id: {type: Types.String},
    icon: {type: Types.String},
    name: {type: Types.String}
}

class Actor extends Component<any> {
}

Actor.schema = {
    range_current: {type: Types.Number},
    sub_type: {type: Types.String}
}

class ActorFromZero extends TagComponent {
}

class CoastBrakeRun extends Component<any> {
}

CoastBrakeRun.schema = {
    type: {type: Types.Number}
}

class Color extends Component<any> {
}

Color.schema = {
    color: {type: Types.Number},
    brightness: {type: Types.Number}
}

class Range extends Component<any> {
}

Range.schema = {
    value: {type: Types.Number}
}

class Input extends Component<any> {
}

Input.schema = {
    subtype: {type: Types.String},
    value: {type: Types.String}
}

class Joystick extends Component<any> {
}

Joystick.schema = {
    lr: {type: Types.Number},
    fb: {type: Types.Number},
    btn: {type: Types.Boolean}
}
