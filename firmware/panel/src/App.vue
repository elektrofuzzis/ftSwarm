<template>
  {{ load() }}
  <auth></auth>
  <nav>
    <FtSwarmIcon v-for="ftswarm in swarm" :current="current" :ft-swarm="ftswarm"
                 @click="jumpTo(ftswarm['id'])"></FtSwarmIcon>
  </nav>
  <div v-for="ftswarm in swarm" class="container">
    <div class="inputs" v-if="current === ftswarm['id']">
      <div class="descriptor">INPUTS</div>
      <ul>
        <FtSwarmInput v-for="input in ftswarm['io'].filter(value => value['type'] === 'INPUT')" :input="input" :loggedin="!!localStorage.getItem('pin')"></FtSwarmInput>
      </ul>
    </div>
    <div class="outputs" v-if="current === ftswarm['id']">
      <div class="descriptor">OUTPUTS</div>
      <ul>
        <FtSwarmMotor v-for="output in ftswarm['io'].filter(value => value['type'] === 'ACTOR')" :output="output" :loggedin="!!localStorage.getItem('pin')"></FtSwarmMotor>
        <FtSwarmLED v-for="output in ftswarm['io'].filter(value => value['type'] === 'LED')" :output="output" :loggedin="!!localStorage.getItem('pin')"></FtSwarmLED>
        <FtSwarmServo v-for="output in ftswarm['io'].filter(value => value['type'] === 'SERVO')" :output="output" :loggedin="!!localStorage.getItem('pin')"></FtSwarmServo>
      </ul>
    </div>
  </div>
</template>

<script lang="ts">
import Auth from "@/components/Auth.vue"
import FtSwarmIcon from "@/components/FtSwarmIcon.vue"
import FtSwarmInput from "@/components/FtSwarmInput.vue"
import FtSwarmMotor from "@/components/FtSwarmMotor.vue"
import FtSwarmLED from "@/components/FtSwarmLED.vue"
import FtSwarmServo from "@/components/FtSwarmServo.vue"
import {FtSwarm, getSwarm} from "@/loader/ApiLoader";
import {ref} from "vue";

export default {
  data() {
    let swarm = ref([] as FtSwarm[]);

    return {
      console,
      hasLoaded: false,
      swarm,
      localStorage,
      current: 0,
      load() {
        if (this.hasLoaded) return;
        this.reload()
        setInterval(this.reload, 5000);
        this.hasLoaded = true;
        return "";
      },
      reload() {
        getSwarm().then(theswarm => {
          swarm.value = theswarm;
        });
      },
      jumpTo(id: number) {
        this.current = id;
      }
    }
  },
  components: {
    Auth,
    FtSwarmIcon,
    FtSwarmInput,
    FtSwarmMotor,
    FtSwarmLED,
    FtSwarmServo
  }
}
</script>

<style lang="scss">
html, body {
  margin: 0;
}

#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  color: #2c3e50;
  box-sizing: border-box;
}

nav {
  width: 100%;
  background-color: aquamarine;
  position: absolute;
  left: 0;
  right: 0;
  top: 0;
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  overflow-y: scroll;
}

ul {
  height: 100%;
  width: 80%;
  float: right;
}

.inputs, .outputs {
  margin-top: 60px;
  width: 100vw;
  height: max-content;
  overflow: hidden;
}

.descriptor {
  position: absolute;
  font-size: 1.5em;
  font-weight: bold;
  color: #2c3e50;
  writing-mode: vertical-lr;
  transform: rotate(180deg);
  border-right: 5px solid #2c3e50;
  margin: 2px;
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  float: left;
}


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

.id {
  color: #a4a4a4;
  font-size: 0.8em;
  opacity: 0;
}

.name:hover>.id {
  opacity: 1;
}

input[type="range"] {
  -webkit-appearance: none;
  height: 10px;
  border-radius: 5px;
  background-color: #a4a4a4;
  outline: none;
}

input[type="color"] {
  border: none;
  border-radius: 0;
  background-color: #d7d7d7;
  outline: none;
}
</style>
