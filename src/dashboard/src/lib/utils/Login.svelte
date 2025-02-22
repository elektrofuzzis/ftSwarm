<script>
    import {swarmApiData} from "../../stores";
    import Swal from "sweetalert2";
    import {ftSwarm} from "../../api/FtSwarm";

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

        if (!await ftSwarm.login(pin)) {
            Swal.fire({
                title: 'Login failed',
                text: 'Please try again',
                icon: 'error',
                background: 'var(--background-card)',
                color: 'var(--color-text)',
            }).then()
        }
    };
</script>


{#if $swarmApiData.auth.status}
    <button class="unstyled login-thumb" onclick={logout}>
        LOGOUT
    </button>
{:else}
    <button class="unstyled login-thumb" onclick={login}>
        LOGIN
    </button>
{/if}

<style>
    .login-thumb {
        position: fixed;
        bottom: 0;
        right: 64px;
        padding: 0.5em;
        background: var(--color-secondary);
        border-radius: 8px 8px 0 0;
        cursor: pointer;
        font-size: 18px;
        font-weight: 700;
    }
</style>