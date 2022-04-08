<template>
  <li>
    <div class="vertical-container">
      <span class="text-muted">Servo</span>
      <span class="name">{{ output.name }} <span class="id">{{ output.id }}</span></span>
    </div>
    <input :disabled="!loggedin" :value="output['position']" class="value" max="255" min="-255" style="width: 70%"
           type="range">
    <img :src="'../assets/' + output.icon" alt="Icon">
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