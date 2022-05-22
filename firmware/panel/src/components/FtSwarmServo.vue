<template>
  <li class="li-servo">
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <span class="text-muted">Servo</span>
        <span class="name">{{ output.name }}</span>
      </div>
    </div>
    <input :disabled="!loggedin"
           :value="output['position']" 
           @input="(e) => {servoPosition(e, output)}"  
           class="value" 
           max="255" min="-255" style="float: left"
           type="range">
  </li>
</template>

<script lang="ts">

import { FtSwarmServo, apiServoPosition } from "../loader/ApiLoader"

export default {
  name: "FtSwarmServo",
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
    servoPosition(event: any, output: any) {
      let servo: FtSwarmServo = {
        id: output.id,
        position: event.target.value
      }

      apiServoPosition(servo)
    }}
}
</script>

<style scoped>
</style>