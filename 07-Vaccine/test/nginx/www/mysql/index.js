document.addEventListener('DOMContentLoaded', () => {
        document.getElementById('loginForm').addEventListener('submit', function (e) {
                e.preventDefault()

                const username = document.getElementById('username').value
                const password = document.getElementById('password').value
                console.log("1 " + { username, password })
                console.log("2 " + JSON.stringify({ username, password }))
                fetch('/login', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ username, password })
                })
                        .then(response => response.json())
                        .then(response => {
                                console.log("Server Response:", response)
                                if (response.success) { alert('Logged in!') }
                                else { alert('Invalid username or password') }
                        })
                        .catch(error => { alert('Error:', error) })
        })
})
