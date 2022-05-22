<template>
  <li class="li-led">
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <span class="text-muted">LED</span>
        <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
      </div>
    </div>
    <input :disabled="!loggedin" 
           :value="output['brightness']"
           @input="(e) => {ledBrightness(e, output)}" 
           class="value"
           max="255" min="0"
           type="range">
    <div class="value-output-container">
      <input :disabled="!loggedin" 
             :value="output['color']"
             @input="(e) => {ledColor(e, output)}" 
             type="color">
    </div>
  </li>
</template>

<script lang="ts">

import { FtSwarmLED, apiLedBrightness, apiLedColor } from "../loader/ApiLoader"

export default {
  name: "FtSwarmLED",
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
    ledBrightness(event: any, output: any) {
      let led: FtSwarmLED = {
        id: output.id,
        brightness: event.target.value
      }

      apiLedBrightness(led)
    },
    ledColor(event: any, output: any) {
      let led: FtSwarmLED = {
        id: output.id,
        color: event.target.value
      }

      apiLedColor(led)
    }

  }
}
</script>

<style scoped>
</style>