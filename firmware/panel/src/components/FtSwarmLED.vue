<template>
  <li>
      <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
    <input class="value" type="range" min="-255" max="255" :value="output['power']" :disabled="!loggedin">
    <input type="color" :disabled="!loggedin">
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