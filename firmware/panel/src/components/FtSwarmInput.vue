<template>
  <li class="li-input">
    <div class="horizontal-container">
      <img :src="'../assets/' + input.icon" alt="Icon">
      <div class="vertical-container">
        <select disabled class="text-muted">
          <option :selected="input['sensorType'] === 0" value="0">Digital</option>
          <option :selected="input['sensorType'] === 1" value="1">Analog</option>
          <option :selected="input['sensorType'] === 2" value="2">Switch</option>
          <option :selected="input['sensorType'] === 3" value="3">Reedswitch</option>
          <option :selected="input['sensorType'] === 4" value="4">Light Barrier</option>
          <option :selected="input['sensorType'] === 5" value="5">Voltmeter</option>
          <option :selected="input['sensorType'] === 6" value="6">Ohmmeter</option>
          <option :selected="input['sensorType'] === 7" value="7">Thermometer</option>
          <option :selected="input['sensorType'] === 8" value="8">LDR</option>
          <option :selected="input['sensorType'] === 9" value="9">Trailsensor</option>
          <option :selected="input['sensorType'] === 10" value="10">Ultrasonic</option>
        </select>
        <span class="name">{{ input.name }} <span class="id">{{ input.id }}</span></span>
      </div>
    </div>
    <div class="value-input-container">
      <span class="value">{{ input.value.toString().replace("Â", "") }}</span>
    </div>
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
