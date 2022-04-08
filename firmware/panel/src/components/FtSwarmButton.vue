<template>
  <li>
    <div class="vertical-container">
      <span class="text-muted">Button</span>
      <span class="name">{{ input.name }} <span class="id">{{ input.id }}</span></span>
    </div>
    <span class="value">{{ input.state.toString().replace("Â", "") }}</span>
    <img :class="input.state===1?'on':''" :src="'../assets/' + input.icon" alt="Icon">
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
