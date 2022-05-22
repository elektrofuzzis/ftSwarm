<template>
  <li class="li-motor">
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <select disabled class="text-muted">
          <option :selected="output['subType'] === 'XMOTOR'" value="XMOTOR">XMotor</option>
          <option :selected="output['subType'] === 'XMMOTOR'" value="XMMOTOR">XMMotor</option>
          <option :selected="output['subType'] === 'TRACTORMOTOR'" value="TRACTORMOTOR">Tractor Motor</option>
          <option :selected="output['subType'] === 'ENCODERMOTOR'" value="ENCODERMOTOR">Encoder Motor</option>
          <option :selected="output['subType'] === 'LAMP'" value="LAMP">Lamp</option>
        </select>
        <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
      </div>
    </div>
    <input :disabled="!loggedin" 
           :value="output['power']" 
           @input="(e) => {motorPower(e, output)}" 
           class="value" 
           max="255" min="-255" 
           type="range">
    <div class="value-output-container">
      <select v-if="['XMOTOR', 'XMMOTOR', 'TRACTORMOTOR', 'ENCODERMOTOR' ].includes( output['subType'] )" 
              :disabled="!loggedin"
              @input="(e) => {servoCmd(e, output)}"  
              class="text-muted">
        <option :selected="output['motiontype'] === 0" value="0">COAST</option>
        <option :selected="output['motiontype'] === 1" value="1">BRAKE</option>
        <option :selected="output['motiontype'] === 2" value="2">RUN</option>
      </select>
    </div>
  </li>
</template>

<script lang="ts">

import { FtSwarmMotor, apiMotorCmd, apiMotorPower } from "../loader/ApiLoader"

export default {
  name: "FtSwarmMotor",
  props: {
    output: {
      type: Object,
      required: true
    },
    loggedin: {
      type: Boolean,
      required: true
    }
  },
  methods: {
    motorCmd(event: any, output: any) {
      let motor: FtSwarmMotor = {
        id: output.id,
        cmd: event.target.value
      }

      apiMotorCmd(motor)
    },
    motorPower(event: any, output: any) {
      let motor: FtSwarmMotor = {
        id: output.id,
        power: event.target.value
      }

      apiMotorPower(motor)
    }}
}
</script>

<style scoped>
</style>