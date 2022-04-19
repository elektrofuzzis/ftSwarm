<template>
  <li>
    <div class="horizontal-container">
      <img :src="'../assets/' + output.icon" alt="Icon">
      <div class="vertical-container">
        <span class="text-muted">Servo</span>
        <span class="name">{{ output.name }}</span>
      </div>
    </div>
    <input :disabled="!loggedin" :value="output['position']" class="value" max="255" min="-255" style="float: left"
           type="range">
  </li>
</template>

<script lang="ts">
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
  }
}
</script>

<style scoped>
</style>