<template>
  <li>
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <span class="text-muted">Button</span>
        <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
      </div>
    </div>
    <input :disabled="!loggedin" :value="output['brightness']" class="value" max="255" min="0" type="range">
    <input :disabled="!loggedin" :value="'#' + output.color" type="color">
  </li>
</template>

<script lang="ts">
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
  }
}
</script>

<style scoped>
</style>