<template>
  <div class="click-to-overlay" @click="showLoginDialogue">
    <span>{{ loginlogout }}</span>
  </div>
</template>

<script lang="ts">
import Swal from "sweetalert2";
import {obtainAccessToken, isAuthenticated, performTokenMod} from "@/loader/ApiLoader";

export default {
  name: "Auth",
  data() {
    let loginlogout = localStorage.getItem('pin') != null ? 'Logout' : 'Login'

    return {
      localStorage,
      loginlogout,
      showLoginDialogue() {
        if (localStorage.getItem('pin') != null) {
          this.localStorage.removeItem('pin')
          this.loginlogout = 'Login'
          Swal.fire({
            title: 'Logged out',
            showConfirmButton: false,
            timer: 1500
          })
          return
        }

        Swal.fire({
          title: 'Login',
          html: `<input type="text" id="login" name="pin" class="swal2-input" placeholder="PIN" maxlength="4">`,
          confirmButtonText: 'Login',
          allowEnterKey: true,
          focusConfirm: false,
          preConfirm: async () => {
            // @ts-ignore
            const pin = Swal.getPopup().querySelector('#login').value
            if (!pin) {
              Swal.showValidationMessage(`Please enter your PIN`)
              return {pin: undefined}
            }

            // Check if pin is numeric
            if (isNaN(pin)) {
              Swal.showValidationMessage(`PIN must be numeric`)
              return {pin: undefined}
            }

            if (pin.length !== 4) {
              Swal.showValidationMessage(`Please enter a valid PIN`)
              return {pin: undefined}
            }

            // Check pin on backend
            const accessToken = await obtainAccessToken()
            localStorage.setItem("token", String(accessToken))
            localStorage.setItem("pin", pin)
            performTokenMod()
            localStorage.removeItem("pin")

            if (!await isAuthenticated()) {
              Swal.showValidationMessage(`ftSwarm refused PIN`)
              return {pin: undefined}
            }

            return {pin: pin}
          }
        }).then((result) => {
          if (result.value?.pin) {
            this.loginlogout = "Logout"
            localStorage.setItem("pin", result.value.pin)
            Swal.fire({
              title: 'Logged in',
              showConfirmButton: false,
              timer: 1500
            })
          }
        })
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

input {
  background-color: rgba(189, 195, 199, 0.89);
  border: none;
  color: #333;
}

input {
  font-size: 20px;
  width: 80%;
  text-align: center;
}
</style>