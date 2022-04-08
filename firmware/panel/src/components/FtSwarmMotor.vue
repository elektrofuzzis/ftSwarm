<template>
  <li>
    <div class="vertical-container">
      <select :disabled="!loggedin">
        <option value="XMOTOR" :selected="output['subType'] === 'XMOTOR'">XMotor</option>
        <option value="TRACTORMOTOR" :selected="output['subType'] === 'TRACTORMOTOR'">Tractor Motor</option>
        <option value="ENCODERMOTOR" :selected="output['subType'] === 'ENCODERMOTOR'">Encoder Motor</option>
        <option value="LAMP" :selected="output['subType'] === 'LAMP'">Lamp</option>
      </select>
      <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
    </div>
    <input class="value" type="range" min="-255" max="255" :value="output['power']" :disabled="!loggedin">
    <select :disabled="!loggedin">
      <option value="0" :selected="output['motiontype'] === 0">COAST</option>
      <option value="1" :selected="output['motiontype'] === 1">BRAKE</option>
      <option value="2" :selected="output['motiontype'] === 2">RUN</option>
    </select>
    <img alt="Icon" :src="'../assets/' + output.icon">
  </li>
</template>

<script lang="ts">
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
  }
}
</script>

<style scoped>
</style>