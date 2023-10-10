<script>
    import {swarmApiData} from "../stores";
    import Swal from "sweetalert2";
    import {ftSwarm} from "../api/FtSwarm";

    const logout = () => {
        localStorage.removeItem('token');
        localStorage.removeItem('key');

        Swal.fire({
            title: 'Logged out',
            text: 'You have been logged out',
            icon: 'success',
            confirmButtonText: 'Ok',
            background: 'var(--background-card)',
            color: 'var(--color-text)',
        })
    };

    const login = async () => {
        let answer = await Swal.fire({
            title: 'Login',
            input: 'text',
            inputLabel: 'Swarm-Pin',
            inputPlaceholder: 'Pin',
            showCancelButton: true,
            inputValidator(inputValue) {
                if (!inputValue) {
                    return 'You need to write something!'
                }

                if (isNaN(Number(inputValue))) {
                    return 'You need to write a number!'
                }

                if (inputValue.length !== 4) {
                    return 'You need to write a 4 digit number!'
                }
            },
            background: 'var(--background-card)',
            color: 'var(--color-text)',
        })

        if (!answer.isConfirmed) {
            return;
        }

        let pin = Number(answer.value);

        await ftSwarm.login(pin)
    };
</script>

{#if $swarmApiData.auth.status}
    <div class="login-thumb" on:click={logout}>
        <span>
            LOGOUT
        </span>
    </div>
{:else}
    <div class="login-thumb" on:click={login}>
        <span>
            LOGIN
        </span>
    </div>
{/if}

<style>
  .login-thumb {
    position: fixed;
    bottom: 0;
    right: 64px;
    width: 100px;
    height: 20px;
    padding: 0.5em;
    background: var(--color-secondary);
    border-radius: 8px 8px 0 0;
    cursor: pointer;
  }

  .login-thumb span {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    font-weight: 700;
  }
</style>