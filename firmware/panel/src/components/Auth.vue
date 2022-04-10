<template>
  <input id="authShown" ref="authShown" hidden type="checkbox">
  <div class="overlay" @click="$refs.authShown.checked = !$refs.authShown.checked">
    <div @click="() => $refs.authShown.checked = !$refs.authShown.checked">
      <div class="key"></div>
      <h2>Login</h2>
      <div class="flex-container">
        <input maxlength="4" name="pin" type="text" @input="(e) => sanitize(e.target)">
        <button id="unlock_btn" disabled @click="(e) => login(e.target)">Unlock with PIN</button>
      </div>
    </div>
  </div>
  <div class="click-to-overlay" @click="popupHandle">
    <span>{{ loginlogout }}</span>
  </div>
</template>

<script lang="ts">
export default {
  name: "Auth",
  data() {
    let loginlogout = localStorage.getItem('pin') != null ? 'Logout' : 'Login'

    return {
      localStorage,
      loginlogout,
      sanitize(elem: HTMLInputElement) {
        let matches = elem.value.match(/\d+/g);
        if (matches)
          elem.value = matches.join("")
        else
          elem.value = ""
        // @ts-ignore
        document.querySelector("#unlock_btn").disabled = elem.value.length !== 4
      },
      login(elem: HTMLButtonElement) {
        if (elem.disabled) return
        // @ts-ignore
        localStorage.setItem("pin", document.querySelector("input[name='pin']").value)
        this.loginlogout = "Logout"
        // @ts-ignore
        this.id("authShown").checked = false
      },
      id(refname: string) {
        return document.querySelector(`#${refname}`)
      },
      popupHandle() {
        if (localStorage.getItem("pin")) {
          localStorage.clear()
          this.loginlogout = "Login"
        }
        // @ts-ignore
        this.id("authShown").checked = !this.id("authShown").checked
      }
    }
  }
}
</script>

<style scoped>
.click-to-overlay {
  position: fixed;
  right: 5px;
  bottom: 0;
  width: 100px;
  height: 30px;
  background-color: #9d0000;
  border-top-left-radius: 11px;
  border-top-right-radius: 11px;
  color: white;

  cursor: pointer;

  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9998;
}

.overlay {
  position: absolute;
  text-align: center;
  left: 0;
  right: 0;
  top: -100vh;
  bottom: 100vh;

  display: flex;
  align-items: center;
  justify-content: center;
}

.overlay > div {
  width: 200px;
  height: 300px;
  border-radius: 12px;
  background-color: #9d0000;
}

#authShown:checked + .overlay {
  position: absolute;
  left: 0;
  right: 0;
  top: 0;
  bottom: 0;
  z-index: 9999;
}

input {
  font-size: 20px;
  width: 80%;
  text-align: center;
}

.flex-container {
  display: flex;
  height: 100%;
  align-items: center;
  justify-content: space-evenly;
  flex-direction: column;
}
</style>