<template>
  {{ load() }}
  <auth></auth>
  <nav>
    <FtSwarmIcon v-for="ftswarm in swarm" :current="current" :ft-swarm="ftswarm"
                 @click="jumpTo(ftswarm['id'])"></FtSwarmIcon>
  </nav>
  <svg width="70mm" height="23mm" viewBox="0 0 264.56697 79.370058"
       enable-background="new 0 0 595.276 595.276">

    <g
        id="g42"
        transform="translate(-172.18119,-236.93309)">
      <defs
          id="defs35">

        <rect
            id="SVGID_1_"
            x="283.905"
            y="186.96001"
            transform="matrix(-0.1736,-0.9848,0.9848,-0.1736,113.3257,598.4075)"
            width="47.645"
            height="129.395"/>
      </defs>
      <clipPath
          id="SVGID_2_">
        <use
            xlink:href="#SVGID_1_"
            overflow="visible"
            id="use37"
            x="0"
            y="0"
            width="100%"
            height="100%"/>
      </clipPath>
      <path
          clip-path="url(#SVGID_2_)"
          fill-rule="evenodd"
          clip-rule="evenodd"
          d="m 281.845,252.73 c 9.456,2.114 17.484,5.802 24.233,10.946 7.979,-7.091 17.396,-12.303 27.965,-16.087 10.569,-3.781 22.288,-6.137 34.874,-7.514 -11.133,-0.631 -22.161,-0.638 -33.052,0.707 -10.891,1.343 -21.646,4.041 -32.235,8.814 -0.434,0.197 -1.167,0.293 -1.898,0.302 -0.733,0.008 -1.468,-0.071 -1.906,-0.227 -7.599,-2.709 -15.713,-3.582 -24.219,-3.146 -8.506,0.436 -17.404,2.177 -26.571,4.699 12.465,-1.149 23.352,-0.608 32.809,1.506"
          id="path40"/>
    </g>
  </svg>

  <svg width="70mm" height="23mm" viewBox="0 0 264.56697 79.370058"
       class="small"
       enable-background="new 0 0 595.276 595.276">

    <g
        id="g42"
        transform="translate(-172.18119,-236.93309)">
      <defs
          id="defs35">

        <rect
            id="SVGID_1_"
            x="283.905"
            y="186.96001"
            transform="matrix(-0.1736,-0.9848,0.9848,-0.1736,113.3257,598.4075)"
            width="47.645"
            height="129.395"/>
      </defs>
      <clipPath
          id="SVGID_2_">
        <use
            xlink:href="#SVGID_1_"
            overflow="visible"
            id="use37"
            x="0"
            y="0"
            width="100%"
            height="100%"/>
      </clipPath>
      <path
          clip-path="url(#SVGID_2_)"
          fill-rule="evenodd"
          clip-rule="evenodd"
          d="m 281.845,252.73 c 9.456,2.114 17.484,5.802 24.233,10.946 7.979,-7.091 17.396,-12.303 27.965,-16.087 10.569,-3.781 22.288,-6.137 34.874,-7.514 -11.133,-0.631 -22.161,-0.638 -33.052,0.707 -10.891,1.343 -21.646,4.041 -32.235,8.814 -0.434,0.197 -1.167,0.293 -1.898,0.302 -0.733,0.008 -1.468,-0.071 -1.906,-0.227 -7.599,-2.709 -15.713,-3.582 -24.219,-3.146 -8.506,0.436 -17.404,2.177 -26.571,4.699 12.465,-1.149 23.352,-0.608 32.809,1.506"
          id="path40"/>
    </g>
  </svg>
  <div v-for="ftswarm in swarm" class="container">
    <div>
      <div class="inputs" v-if="current === ftswarm['id']">
        <div class="descriptor">INPUTS</div>
        <ul>
          <div class="wrap-container">
            <FtSwarmInput v-for="input in ftswarm['io'].filter(value => value['type'] === 'INPUT')" :input="input"
                          :loggedin="!!localStorage.getItem('pin')"></FtSwarmInput>
            <FtSwarmButton v-for="input in ftswarm['io'].filter(value => value['type'] === 'BUTTON')" :input="input"
                           :loggedin="!!localStorage.getItem('pin')"></FtSwarmButton>
          </div>
          <div class="center-container">
            <FtSwarmJoystick v-for="input in ftswarm['io'].filter(value => value['type'] === 'JOYSTICK')" :input="input"
                             :loggedin="!!localStorage.getItem('pin')"></FtSwarmJoystick>
          </div>
        </ul>
      </div>
      <div class="outputs" v-if="current === ftswarm['id']">
        <div class="descriptor">OUTPUTS</div>
        <ul>
          <div class="wrap-container">
            <FtSwarmMotor v-for="output in ftswarm['io'].filter(value => value['type'] === 'ACTOR')" :output="output"
                          :loggedin="!!localStorage.getItem('pin')"></FtSwarmMotor>
          </div>
          <div class="wrap-container">
            <FtSwarmLED v-for="output in ftswarm['io'].filter(value => value['type'] === 'LED')" :output="output"
                        :loggedin="!!localStorage.getItem('pin')"></FtSwarmLED>
          </div>
            <FtSwarmServo v-for="output in ftswarm['io'].filter(value => value['type'] === 'SERVO')" :output="output"
                          :loggedin="!!localStorage.getItem('pin')"></FtSwarmServo>
        </ul>
      </div>
    </div>
  </div>
  <div class="footer-container">
    <span>Read the docs at <a href="https://elektrofuzzis.github.io/ftSwarm">https://elektrofuzzis.github.io/ftSwarm</a></span>
    <span>(C) 2022 Christian Bergschneider & Stefan Fuss</span>
  </div>
</template>

<script lang="ts">
import Auth from "@/components/Auth.vue"
import FtSwarmIcon from "@/components/FtSwarmIcon.vue"
import FtSwarmInput from "@/components/FtSwarmInput.vue"
import FtSwarmMotor from "@/components/FtSwarmMotor.vue"
import FtSwarmLED from "@/components/FtSwarmLED.vue"
import FtSwarmServo from "@/components/FtSwarmServo.vue"
import FtSwarmButton from "@/components/FtSwarmButton.vue"
import FtSwarmJoystick from "@/components/FtSwarmJoystick.vue";
import {FtSwarm, getSwarm} from "@/loader/ApiLoader";
import {ref} from "vue";

export default {
  data() {
    let swarm = ref([] as FtSwarm[]);

    return {
      console,
      hasLoaded: false,
      blockReload: false,
      swarm,
      localStorage,
      current: 0,
      load() {
        if (this.hasLoaded) return;
        this.reload()
        setInterval(this.reload, 500);
        this.hasLoaded = true;
        return "";
      },
      reload() {
        if (this.blockReload) return;
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
    FtSwarmServo,
    FtSwarmButton,
    FtSwarmJoystick
  }
}
</script>

<style lang="scss">

html, body {
  margin: 0;
  background-color: #151517;
  z-index: -3;
  font-size: 0.9em;
}

#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  color: #a9a9a9;
  box-sizing: border-box;
  z-index: -2;
}

nav {
  width: 100%;
  position: absolute;
  left: 0;
  right: 0;
  top: 0;
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  overflow-x: auto;
  padding: 3px 0;
  z-index: 100;
}

a {
  color: #9d0000;
}

ul {
  height: 100%;
  width: fit-content;
  display: flex;
  flex-direction: column;
  margin: 0;
}

.inputs, .outputs {
  margin-top: 60px;
  height: max-content;
  width: fit-content;
  overflow: hidden;
}

.descriptor {
  position: absolute;
  font-size: 1.5em;
  font-weight: bold;
  color: #a4a4a4;
  writing-mode: vertical-lr;
  transform: rotate(180deg);
  border-right: 5px solid #a4a4a4;
  margin: 10px;
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  float: left;
}

.container {
  overflow-x: hidden;
  width: 100%;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
  z-index: 10;
}

li {
  margin: 5px 2px 2px 5px;
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
  justify-content: left;
  align-items: center;
  height: 100%;
  width: fit-content;
  float: left;
  z-index: 10;
}

li > * {
  margin: 2px 10px;
}

.li-input {
  width: 18em;
}

.li-button {
  width: 18em;
}

.li-joystick {
  width: 27em;
}

.li-led {
  width: 30em;
}

.li-motor {
  width: 30em;
}

.li-servo {
  width: 30em;
}

.wrap-container {
  display: flex;
  flex-wrap: wrap;
  flex-direction: row;
  justify-content: left;
  z-index: 10;
  max-width: 75em;
}

.center-container {
  width: 35em;
}

.vertical-container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  z-index: 10;
  width: 6em;
}

.footer-container {
  display: flex;
  flex-direction: column;
  justify-content: center;
  text-align: center;
  margin-top: 10px;
  row-gap: 10px;
}

.vertical-joystick-container {
  display: flex;
  flex-direction: column;
  width: 8em;
}

.value-input-container {
  text-align: left;
}

.value-output-container {
  text-align: left;
}

.value-joystick-container {
  width: 10em;
  text-align: left;
}

div {
  z-index: 10;
}

.horizontal-container {
  display: flex;
  flex-direction: row;
  justify-content: space-between;
}

select {
  border: none;
  border-radius: 0;
  background-color: #34343b;
  color: white;
  font-size: 0.8em;
  width: 9em;
}


.text-muted {
  color: #bdc3c7;
}

.name {
  margin-left: 5px;
}

img {
  border-radius: 50%;
  background-color: #9d0000;
  height: 30px;
  margin: 6px;
  padding: 3px;
  color: #a9a9a9 !important;
}

.on {
  background-color: #006900 !important;
}

.id {
  color: #a4a4a4;
  font-size: 0.8em;
  opacity: 0;
}

.name:hover > .id {
  opacity: 1;
}

input[type="range"] {
  -webkit-appearance: none;
  height: 10px;
  border-radius: 5px;
  background-color: #a4a4a4;
  outline: none;
  width: 8em;
}

input[type="color"] {
  border: none;
  border-radius: 0;
  background-color: #d7d7d7;
  outline: none;
  z-index: 2;
}

svg {
  position: fixed;
  bottom: 0%;
  right: 0%;
  width: 100%;
  height: 100%;
  fill: rgba(34, 34, 37, 0.81);
  z-index: 0;
  pointer-events:none;
}

.small {
  transform: scale(0.5) translateX(-25%) translateY(10%);
}

</style>
