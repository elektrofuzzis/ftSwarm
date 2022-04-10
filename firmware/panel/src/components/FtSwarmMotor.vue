<template>
  <li>
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <select :disabled="!loggedin">
          <option :selected="output['subType'] === 'XMOTOR'" value="XMOTOR">XMotor</option>
          <option :selected="output['subType'] === 'TRACTORMOTOR'" value="TRACTORMOTOR">Tractor Motor</option>
          <option :selected="output['subType'] === 'ENCODERMOTOR'" value="ENCODERMOTOR">Encoder Motor</option>
          <option :selected="output['subType'] === 'LAMP'" value="LAMP">Lamp</option>
        </select>
        <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
      </div>
    </div>
    <input :disabled="!loggedin" :value="output['power']" class="value" max="255" min="-255" type="range">
    <select :disabled="!loggedin">
      <option :selected="output['motiontype'] === 0" value="0">COAST</option>
      <option :selected="output['motiontype'] === 1" value="1">BRAKE</option>
      <option :selected="output['motiontype'] === 2" value="2">RUN</option>
    </select>
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