<template>
  <li>
    <div class="vertical-container">
      <select :disabled="!loggedin" >
        <option value="0" :selected="input['sensorType'] === 0">Digital</option>
        <option value="1" :selected="input['sensorType'] === 1">Analog</option>
        <option value="2" :selected="input['sensorType'] === 2">Switch</option>
        <option value="3" :selected="input['sensorType'] === 3">Reedswitch</option>
        <option value="4" :selected="input['sensorType'] === 4">Voltmeter</option>
        <option value="5" :selected="input['sensorType'] === 5">Ohmmeter</option>
        <option value="6" :selected="input['sensorType'] === 6">Thermometer</option>
        <option value="7" :selected="input['sensorType'] === 7">LDR</option>
        <option value="8" :selected="input['sensorType'] === 8">Trailsensor</option>
        <option value="9" :selected="input['sensorType'] === 9">Ultrasonic</option>
      </select>
      <span class="name">{{input.name}} <span class="id">{{input.id}}</span></span>
    </div>
    <span class="value">{{input.value.toString().replace("Â", "")}}</span>
    <img alt="Icon" :src="'../assets/' + input.icon">
  </li>
</template>

<script lang="ts">
export default {
  name: "FtSwarmInput",
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
