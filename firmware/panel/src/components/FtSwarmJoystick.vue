<template>
  <li>
    <div class="vertical-container">
      <span class="text-muted">Joystick</span>
      <span class="name">{{input.name}} <span class="id">{{input.id}}</span></span>
    </div>
    <span class="value">Up-Down: {{ input.valueFb.toString().replace("Â", "") }}%</span>
    <span class="value">Left-Right: {{ input.valueLr.toString().replace("Â", "") }}%</span>
    <span class="value">Button: {{ input.button.toString().replace("Â", "") }}</span>
    <div class="joystickarea">
      <div class="joystick" :style="{left:input.valueLr/2+50+'%',top:(-(input.valueFb))/2+50+'%'}" :class="input.button===1?'on':''"></div>
    </div>
  </li>
</template>

<script lang="ts">
export default {
  name: "FtSwarmJoystick",
  props: {
    input: {
      type: Object,
      required: true
    },
    loggedin: {
      type: Boolean,
      required: true
    }
  }
}
</script>

<style scoped>
.joystickarea {
  width: 100px;
  height: 100px;
  border: 1px solid #ccc;
  border-radius: 50%;
  background-color: #ccc;
  position: relative;
}
.joystick {
  width: 10px;
  height: 10px;
  border-radius: 50%;
  background-color: red;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
}
</style>