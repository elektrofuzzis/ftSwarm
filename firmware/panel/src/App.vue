<template>
  <auth></auth>
  <nav>

  </nav>
  <router-view/>
</template>

<script lang="ts">
import {World, System, Component, TagComponent, Types} from "ecsy";
import {FtSwarmCtrl, FtSwarmJST} from "@/ecs/swarmComponents"
import NotSecureSecurityModule from "@/loader/NotSecureSecurityModule";
import Auth from "@/components/Auth.vue"


export default {
  setup() {
  },
  data() {
    return {
      swarm: []
    }
  },
  components: {
    Auth
  }
}
</script>

<style lang="scss">
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
</style>
