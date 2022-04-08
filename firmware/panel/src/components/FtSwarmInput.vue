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
      <span class="name">{{input.name}}</span>
    </div>
    <span class="value">{{input.value.toString().replace("Â", "")}}</span>
    <img alt="Icon" :src="'../assets/' + input.icon" class="on">
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

<style scoped>

li {
  margin: 5px 2px 2px 5px;
  display: flex;
  flex-direction: row;
  justify-content: space-between;
  align-items: center;
  height: 100%;
  width: 90%;
  float: left;
}

.vertical-container {
  display: flex;
  flex-direction: column;
  justify-content: center;
}

select {
  border: none;
  border-radius: 0;
  background-color: #f5f5f5;
  font-size: 0.8em;
}

.name {
  font-size: 1.2em;
  margin-left: 5px;
}

img {
  border-radius: 50%;
  background-color: red;
  height: 30px;
}

.on {
  background-color: green;
}

</style>