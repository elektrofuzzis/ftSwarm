<template>
  <input id="authShown" type="checkbox" hidden ref="authShown">
  <div class="overlay" @click="$refs.authShown.checked = !$refs.authShown.checked">
    <div @click="() => $refs.authShown.checked = !$refs.authShown.checked">
      <div class="key"></div>
      <h2>Login</h2>
      <div class="flex-container">
        <input type="text" @input="(e) => sanitize(e.target)" name="pin" maxlength="4">
        <button id="unlock_btn" disabled @click="(e) => login(e.target)">Unlock with PIN</button>
      </div>
    </div>
  </div>
  <div class="click-to-overlay" @click="popupHandle">
    <span>{{loginlogout}}</span>
  </div>
</template>

<script>
export default {
  name: "Auth",
  data() {
    let loginlogout = localStorage.getItem('pin') != null ? 'Logout' : 'Login'

    return {
      localStorage,
      loginlogout,
      sanitize(elem) {
        let matches = elem.value.match(/\d+/g);
        if (matches)
          elem.value = matches.join("")
        else
          elem.value = ""
        document.querySelector("#unlock_btn").disabled = elem.value.length !== 4
      },
      login(elem) {
        if(elem.disabled) return
        localStorage.setItem("pin", document.querySelector("input[name='pin']").value)
        this.loginlogout = "Logout"
      },
      id(refname){
        return document.querySelector(`#${refname}`)
      },
      popupHandle() {
        if(localStorage.getItem("pin")){
          localStorage.clear()
          this.loginlogout = "Login"
        }
        this.id("authShown").checked = !this.id("authShown").checked
      }
    }
  }
}
</script>

<style scoped>
.click-to-overlay {
  position: absolute;
  right: 5px;
  bottom: 0;
  width: 100px;
  height: 30px;
  background-color: coral;
  border-top-left-radius: 11px;
  border-top-right-radius: 11px;

  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9998;
}

.overlay {
  position: absolute;
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
  background-color: darkorange;
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