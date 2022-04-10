<template>
  <li>
    <div class="horizontal-container">
      <img :class="input.state===1?'on':''" :src="'../assets/' + input.icon" alt="Icon">
      <div class="vertical-container">
        <span class="text-muted">Button</span>
        <span class="name">{{ input.name }} <span class="id">{{ input.id }}</span></span>
      </div>
    </div>
    <span class="value">{{ input.state.toString().replace("Â", "") }}</span>
  </li>
</template>

<script lang="ts">
export default {
  name: "FtSwarmButton",
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
