<template>
  <div :class="current === getSwarm().id?'box-active':''" class="box">
    <code>{{ getType() }}</code><br/>
    {{ getSwarm().name }}
  </div>
</template>

<script lang="ts">
import {FtSwarm} from "@/loader/ApiLoader";

export default {
  name: "FtSwarmIcon",
  props: ["ftSwarm", "current"],
  methods: {
    getSwarm(): FtSwarm {
      // @ts-ignore
      return this.ftSwarm;
    },
    getType(): string {
      return this.getSwarm().type == "ftSwarm" ? "ftSwarm" : "ftSwarmControl";
    }
  },
}
</script>

<style lang="sass" scoped>
.box
  background-color: #ffa57f
  border-color: #ffa57f
  width: fit-content
  padding: 0.2em 0.4em
  margin: 0 1px
  border-radius: 0.2em
  border-width: 1px
  border-style: solid
  cursor: pointer

.box-active
  border-color: blueviolet

code
  font-family: monospace
  font-size: 0.8em
  font-weight: bold
  color: #724766
  padding: 0.2em 0.4em

</style>